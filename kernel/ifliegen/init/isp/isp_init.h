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

#ifndef __ISP_INIT_H__
#define __ISP_INIT_H__

void DRV_ISP_ClkInit(u32 dev, s32 clkIdx);
void DRV_ISP_ClkDeInit(u32 dev);

int DRV_ISP_ModuleDriverInit(void);
int DRV_ISP_ModuleDriverExit(void);

#endif
