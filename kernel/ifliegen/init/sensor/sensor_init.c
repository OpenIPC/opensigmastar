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
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/kdev_t.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/miscdevice.h>
#include <asm/string.h>
#include <asm/uaccess.h>

#include <ms_msys.h>

#include <cam_sysfs.h>
#include <cam_clkgen.h>
#include <cam_os_wrapper.h>
#include <drv_ss_cus_sensor.h>
#include <mi_common_internal.h>

void DrvSensorIfUt(const char *pCmdStr);
s32  HAL_SENSOR_SetI2CMaster(u32 nGroup, CUS_SENIF_BUS eBusIf, u32 nI2CMaster, u32 nNumLane);
void DRV_SENSOR_IfCtxInit(void);
void DRV_SENSOR_IfInitRxPadAttr(void);
s32  DRV_SENSOR_IfRegBankAddrMap(void);
void DRV_SENSOR_IfDev2SnrPadCtxInit(void);
s32  DRV_SENSOR_IfInitCommon(void);
s32  DRV_SENSOR_IfDeinitCommon(void);


static struct device_node *pstDevNode = NULL;

int DRV_SENSOR_OfPropertyReadU32(const char *propname, u32 *out_value)
{
    if (pstDevNode == NULL)
    {
        printk("[%s %d]ERROR: SNR pstDevNode is NULL!\n", __func__, __LINE__);
        return -1;
    }
    else
    {
        return CamofPropertyReadU32(pstDevNode, propname, out_value);
    }
}

int DRV_SENSOR_OfPropertyReadU32Array(const char *propname, u32 *out_values, size_t sz)
{
    if (pstDevNode == NULL)
    {
        printk("[%s %d]ERROR: SNR pstDevNode is NULL!\n", __func__, __LINE__);
        return -1;
    }
    else
    {
        return CamOfPropertyReadU32Array(pstDevNode, propname, out_values, sz);
    }
}

void *DRV_SENSOR_IfModuleGetDev(void)
{
    return (void *)pstDevNode;
}

static int _SENSORIF_MODULE_Open(struct inode *inode, struct file *fp)
{
    return 0;
}

static int _SENSORIF_MODULE_Release(struct inode *inode, struct file *fp)
{
    return 0;
}

static ssize_t _SENSORIF_MODULE_FRead(struct file *fp, char __user *buf, size_t size, loff_t *ppos)
{
    return 0;
}

static ssize_t _SENSORIF_MODULE_FWrite(struct file *fp, const char __user *buf, size_t size, loff_t *ppos)
{
    char  cmd[128];
    char *pKBuf = NULL;
    int res = 0;

    pKBuf = vmalloc(size);
    if(pKBuf == NULL)
    {
        goto SENSORIF_WRITE_EXIT;
    }
    if (!copy_from_user((void *)pKBuf, (void __user *)buf, size))
    {
        res = sscanf(pKBuf, "%s", cmd);
        if(res != 1)
        {
            printk("[%s %d]Error: Failed to read the command!\n", __func__, __LINE__);
            goto SENSORIF_WRITE_EXIT;
        }
        CamOsPrintf("[%s] CMD : %s\n", __FUNCTION__, cmd);
    }
    else
    {
        CamOsPrintf("[%s] copy user data failed!!!\n", __FUNCTION__);
        goto SENSORIF_WRITE_EXIT;
    }

    DrvSensorIfUt(pKBuf); // TODO: how to delete

SENSORIF_WRITE_EXIT:
    if (pKBuf)
    {
        vfree(pKBuf);
        pKBuf = NULL;
    }

    return size;
}

static long _SENSORIF_MODULE_Ioctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
    return 0;
}

struct file_operations g_stSensorIfFOps = {
    .owner          = THIS_MODULE,
    .open           = _SENSORIF_MODULE_Open,
    .release        = _SENSORIF_MODULE_Release,
    .read           = _SENSORIF_MODULE_FRead,
    .write          = _SENSORIF_MODULE_FWrite,
    .unlocked_ioctl = _SENSORIF_MODULE_Ioctl,
};

void SensorWARN_ON(void)
{
    WARN_ON(1);
}

void *DRV_CSI_SensorIoremap(u32 addr, u32 size)
{
    return ioremap(addr, size);
}

void DRV_CSI_SensorIoUnmap(void *addr)
{
    iounmap(addr);
}

int DRV_SENSOR_SensorifRequestIrq(unsigned int virq, void *isr, int tri_type, const char *name, void *dev)
{
    return request_irq(virq, (irq_handler_t)isr, tri_type, name, dev);
}

void DRV_SENSOR_SensorifFreeIrq(unsigned int virq, void *dev)
{
    free_irq(virq, dev);
}

typedef struct
{
    struct miscdevice stSensorIfDev;
    dev_t             g_stDevNo;   // for dynamic major number allocation
    dev_t             dev;         // for dynamic major number allocation
} SensorIfData_t;

int SENSORIF_MODULE_SgsSensorIfProbe(struct platform_device *pdev)
{
    SensorIfData_t    *pdata;
    struct miscdevice *pSensorIfDev;
    u32                proval;
    char               aBufName[48];
    u32                nSnrCnt;

    DRV_SENSOR_IfCtxInit();
    DRV_SENSOR_IfRegBankAddrMap();

    // init Rx pad info
    pstDevNode = pdev->dev.of_node;
    DRV_SENSOR_IfInitRxPadAttr();

    pdata = CamOsMemAlloc(sizeof(SensorIfData_t));
    if (!pdata)
    {
        printk("[%s] mem alloc failed !\n", __FUNCTION__);
        return -1;
    }
    CamOsMemset(pdata, 0, sizeof(SensorIfData_t));
    pdev->dev.platform_data = (void *)pdata;

    pSensorIfDev         = &pdata->stSensorIfDev;
    pSensorIfDev->minor  = MISC_DYNAMIC_MINOR;
    pSensorIfDev->name   = "sensorif";
    pSensorIfDev->fops   = &g_stSensorIfFOps;
    pSensorIfDev->parent = &pdev->dev;
    misc_register(pSensorIfDev);

    DRV_SENSOR_IfInitCommon();

    DRV_SENSOR_IfDev2SnrPadCtxInit();

    // skip frame status init
    // DRV_SENSOR_IF_SkipframeInit();

    return 0;
}

int SENSORIF_MODULE_SgsSensorIfRemove(struct platform_device *pdev)
{
    SensorIfData_t    *pdata        = dev_get_platdata(&pdev->dev);
    struct miscdevice *pSensorIfDev = &pdata->stSensorIfDev;

    /*replace device_remove_file -> CamDeviceRemoveFile*/

    DRV_SENSOR_IfDeinitCommon();

    misc_deregister(pSensorIfDev);

    CamOsMemRelease(pdata);
    return 0;
}

/* Match table for of_platform binding */
static const struct of_device_id G_ST_SGS_SENSOR_IF_MATCH[] = {
    {.compatible = "sgs,sensorif", 0},
    {},
};
MODULE_DEVICE_TABLE(of, G_ST_SGS_SENSOR_IF_MATCH);

static struct platform_driver g_stSgsSensorIfDriver = {
    .probe  = SENSORIF_MODULE_SgsSensorIfProbe,
    .remove = SENSORIF_MODULE_SgsSensorIfRemove,
    .driver =
        {
            .name           = "sgs,sensorif",
            .owner          = THIS_MODULE,
            .of_match_table = G_ST_SGS_SENSOR_IF_MATCH,
        },
};

int MI_SENSOR_IOCTL_SgsSensorIfInitDriver(void)
{
    return CamPlatformDriverRegister(&g_stSgsSensorIfDriver);
}

void MI_SENSOR_IOCTL_SgsSensorIfExitDriver(void)
{
    CamPlatformDriverUnregister(&g_stSgsSensorIfDriver);
}

#pragma message("compile mi and mhal")
DECLEAR_MODULE_INIT_EXIT

MI_MODULE_LICENSE();

