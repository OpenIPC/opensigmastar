/* Sgs trade secret */
/* Copyright (c) [2019~2025] Sgs Technology Ltd.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
Sgs and be kept in strict confidence
(Sgs Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of Sgs Confidential
Information is unlawful and strictly prohibited. Sgs hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/debugfs.h>
#include <linux/sysfs.h>
#include <linux/vmalloc.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>

#include <mi_common_internal.h>
#include <ms_msys.h>
#include "cam_sysfs.h"
#include "isp_init.h"
#include "isp_poll.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("sgs");

typedef struct
{
    //struct miscdevice isp_dev;
    //struct  CamDevice *   sysfs_dev;
    int irq_num;
} ISP_ModuleData_t;

ISP_ModuleData_t *pg_ispModuledata;
#define _ISP_MODULE_DATA_PTR(dev) (pg_ispModuledata)  //current only support single dev
static unsigned long g_ispRegVirtBase = 0;  // virt base for riu

#define ISP_MAX_ROOT_PATH_LENGTH (128)
static char g_ispRoot[ISP_MAX_ROOT_PATH_LENGTH];

static struct platform_device *g_ispDevice[1];

char *DRV_ISP_ModuleGetIspRoot(u32 dev)
{
    UNUSED(dev);
    return g_ispRoot;
}

int DRV_ISP_ModuleSetIspRoot(u32 dev, const char* pIspRoot)
{
    UNUSED(dev);
    if (CamOsStrlen(pIspRoot) >= sizeof(g_ispRoot))
    {
        return -1;
    }

    CamOsStrncpy(g_ispRoot, pIspRoot, CamOsStrlen(pIspRoot) + 1);

    return 0;
}

static int _DRV_ISP_ModuleProbe(struct platform_device *pdev)
{
    u32 clkIdx = 0;
    s32 curClkIdx = -1;
    ISP_ModuleData_t *pData = NULL;

    /* module device */
    _ISP_MODULE_DATA_PTR(0) = kzalloc(sizeof(ISP_ModuleData_t), GFP_KERNEL);
    pData = _ISP_MODULE_DATA_PTR(0);
    if (NULL == pData)
    {
        CamOsPrintf("%s, line:%d error!\n", __FUNCTION__, __LINE__);
        return -ENOENT;
    }

    g_ispDevice[0] = pdev;

    /* clk */
    if (0 == CamofPropertyReadU32(pdev->dev.of_node, "clock-frequency-index", &clkIdx))
    {
        // clk init
        curClkIdx = clkIdx;
    }
    DRV_ISP_ClkInit(0, curClkIdx);

    /* interrupt */
    pData->irq_num = CamIrqOfParseAndMap(pdev->dev.of_node, 0);
    CamOsPrintf("[ISP] Request irq num: %d\n", pData->irq_num);

    /* reg base */
    g_ispRegVirtBase = (unsigned long)ioremap(0x1F000000, 0x400000);

    return 0;
}

static int _DRV_ISP_ModuleRemove(struct platform_device *pdev)
{
    ISP_ModuleData_t *pData = _ISP_MODULE_DATA_PTR(0);

    DRV_ISP_ClkDeInit(0);

    if(NULL != pData)
    {
        kfree(pData);
    }

    return 0;
}

static int _DRV_ISP_ModuleSuspend(struct platform_device *pdev, pm_message_t state)
{
    return 0;
}

static int _DRV_ISP_ModuleResume(struct platform_device *pdev)
{
    return 0;
}

static struct of_device_id g_ispModuleDeviceTableIds[] = {{.compatible = "isp"}, {}};
MODULE_DEVICE_TABLE(of, g_ispModuleDeviceTableIds);

static struct platform_driver  g_ispDriver = {
    .probe   = _DRV_ISP_ModuleProbe,
    .remove  = _DRV_ISP_ModuleRemove,
    .suspend = _DRV_ISP_ModuleSuspend,
    .resume  = _DRV_ISP_ModuleResume,
    .driver =
        {
            .name           = "isp",
            .owner          = THIS_MODULE,
            .of_match_table = of_match_ptr(g_ispModuleDeviceTableIds),
        },
};

int DRV_ISP_ModuleDriverInit(void)
{
    int ret = 0;

    ret = CamPlatformDriverRegister(&g_ispDriver);
    if (ret)
    {
        CamOsPrintf("[%s] ISP Driver register fail !!!\n", __FUNCTION__);
        return ret;
    }

    ret = DRV_ISP_PollInit(0);
    if (ret)
    {
        CamOsPrintf("[%s] ISP fe register fail !!!\n", __FUNCTION__);
        return ret;
    }

    ret = DRV_ISP_ModuleSetIspRoot(0, "/config/iqfile");
    if (ret)
    {
         CamOsPrintf("[%s] ISP set root fail !!!\n", __FUNCTION__);
        return ret;
    }

    return 0;
}

int DRV_ISP_ModuleDriverExit(void)
{
    CamPlatformDriverUnregister(&g_ispDriver);

    DRV_ISP_PollDeinit(0);

    return 0;
}

int DRV_ISP_ModuleGetIrqNum(u32 dev)
{
    ISP_ModuleData_t *pData = _ISP_MODULE_DATA_PTR(0);
    UNUSED(dev);

    return pData->irq_num;
}

void *HAL_ISP_SysGetRegBase(void)
{
    return (void*)g_ispRegVirtBase;
}

inline u32 HAL_ISP_SysRegV2R(void *virt)
{
    return (u32)((unsigned long)virt - g_ispRegVirtBase);
}

void *DRV_ISP_ModuleGetClkDev(u32 dev)
{
    struct device_node *pstDevNode = NULL;

    pstDevNode = (g_ispDevice[0] != NULL) ? ((struct platform_device *)g_ispDevice[0])->dev.of_node : NULL;

    return pstDevNode;
}

#pragma message("compile mi-lite")
DECLEAR_MODULE_INIT_EXIT
