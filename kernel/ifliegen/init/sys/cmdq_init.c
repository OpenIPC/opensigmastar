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
#include <asm/neon.h>
#include <linux/module.h>

#include "mi_common_datatype.h"

#include <linux/moduleparam.h>
#include <linux/stat.h>
#include <linux/sched.h>
#include <linux/of_platform.h>
#include "cam_sysfs.h"
#include "drv_clock.h"
#include "cam_device_wrapper.h"


/////////////// cmdq ////////////////
// FIXME: use interface to get answer
#define NUMBER_OF_CMDQ_HW 12
int DrvCmdqGetIrqNum(int nDrvId)
{
    struct device_node *devNode;
    struct platform_device *pdev;
    char compatibleName[16];
    int irq = 0;
    if(nDrvId>=NUMBER_OF_CMDQ_HW )
    {
        pr_info("[CMDQ] Error Id: %d\n", nDrvId);
        return -1;
    }
    CamOsSprintf(compatibleName, "sgs,cmdq%d", nDrvId); //"sgs,cmdq0"
    devNode = of_find_compatible_node(NULL, NULL, compatibleName);

    //pr_err("%s , %s\n" , __func__ , compatibleName);

    if (!devNode) {
        //pr_err("%s , LINE = %d \n" , __func__ , __LINE__ );
        return -1;
    }
            //pr_err("%s , LINE = %d \n" , __func__ , __LINE__ );
    pdev = of_find_device_by_node(devNode);
    if (!pdev) {
        of_node_put(devNode);
           // pr_err("%s , LINE = %d \n" , __func__ , __LINE__ );
        return -1;
    }
    irq = CamIrqOfParseAndMap(pdev->dev.of_node, 0);
    pr_info("[CMDQ%d] Virtual IRQ: %d\n", nDrvId, irq);
    //pr_err("[CMDQ%d] Virtual IRQ: %d\n", nDrvId, irq);
    //pr_err("%s , LINE = %d \n" , __func__ , __LINE__ );
    return irq;
}

/* return Mhz unit*/
int DrvCmdqGetClk(int nDrvId)
{
    int nClkRate_MHz = 0;
    struct device_node *devNode;
    char compatibleName[16];
    void* pClkHandle = NULL;

    if(nDrvId>=NUMBER_OF_CMDQ_HW )
    {
        pr_info("[CMDQ] Error Id: %d\n", nDrvId);
        return -1;
    }

    CamOsSprintf(compatibleName, "sgs,cmdq%d", nDrvId); //"sgs,cmdq0"
    devNode = of_find_compatible_node(NULL, NULL, compatibleName);
    if (NULL == devNode)
    {
        pr_info("[CMDQ%d] device node isn't exist\n", nDrvId);
        return -1;
    }

    pClkHandle = drv_clk_get(devNode, 0);
    if (!pClkHandle)
    {
        pr_info("[CMDQ%d] Get Clk fail\n", nDrvId);
        return -1;
    }
    nClkRate_MHz = drv_clk_get_rate(pClkHandle) / 1000000;
    drv_clk_put(pClkHandle);

    pr_info("[CMDQ%d] CLK: %d MHz\n", nDrvId, nClkRate_MHz);
    return nClkRate_MHz;
}
