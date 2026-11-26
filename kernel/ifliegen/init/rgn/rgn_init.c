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
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include "cam_sysfs.h"
#include "cam_os_wrapper.h"
#include "mi_common_internal.h"

#define _RGN_INIT_C_

#define DRV_RGN_DEVICE_NAME  "rgn"

static struct platform_device g_stDrvRgnPlatformDevice = {
    .name = DRV_RGN_DEVICE_NAME,
    .id   = 0,
    .dev  = {
        .of_node = NULL,
    },
};

extern bool g_bSupportOsdBlending;
extern void *MHAL_RGN_GetInternalApis(void);
extern void *mi_rgn_GetInternalApis(void);
extern MI_U32 MHAL_RGN_GetGopNumByType(MI_U32 eTargetType);
extern MI_U32 MHAL_RGN_GetTargetMax(void);
extern MI_S32 MHAL_RGN_SetIrqNum(MI_U32 eTargetType, MI_U8 u8Id, MI_U32 u32IrqNum);

#ifdef __KERNEL__
#if defined(CONFIG_MI_RGN_SUPPORT_OVERLAY_BLENDING)
#if (CONFIG_MI_RGN_SUPPORT_OVERLAY_BLENDING==0)
module_param_named(b_support_osd_blending, g_bSupportOsdBlending, bool, S_IRUGO | S_IWUSR);
#endif
#else
module_param_named(b_support_osd_blending, g_bSupportOsdBlending, bool, S_IRUGO | S_IWUSR);
#endif
#endif

#ifndef COMBINE_MI_MODULE
EXPORT_SYMBOL(MHAL_RGN_GetInternalApis);
EXPORT_SYMBOL(mi_rgn_GetInternalApis);
#endif

void DRV_RGN_MoudleDeviceInit(void)
{
    MI_U32 u32IrqId = 0, u32Index = 0;
    MI_U8  i, j;

    if (g_stDrvRgnPlatformDevice.dev.of_node == NULL)
    {
        g_stDrvRgnPlatformDevice.dev.of_node = of_find_compatible_node(NULL, NULL, "sgs,rgn");
    }
    if (g_stDrvRgnPlatformDevice.dev.of_node == NULL)
    {
        //CamOsPrintf("[RGN INIT] Get Device mode Fail!!\n");
        return;
    }

    for (i = 0; i < MHAL_RGN_GetTargetMax(); i++)
    {
        for (j = 0; j < MHAL_RGN_GetGopNumByType(i); j++)
        {
            u32IrqId = CamOfIrqToResource(g_stDrvRgnPlatformDevice.dev.of_node, u32Index, NULL);
            if (!u32IrqId)
            {
                CamOsPrintf("[RGN MODULE] Can't Get GOP_IRQ\n");
                return;
            }
            if (0 != MHAL_RGN_SetIrqNum(i, j, u32IrqId))
            {
                continue;
            }
            u32Index++;
        }
    }
}

DECLEAR_MODULE_INIT_EXIT
MI_MODULE_LICENSE();
MODULE_AUTHOR("malloc.peng");
