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

#ifndef _DISP_INIT_H_
#define _DISP_INIT_H_

void    DISP_MODULE_DeviceInit(void);
void    DISP_MODULE_DeviceDeInit(void);
MI_U32* DRV_DISP_OS_GpioPadModeToPadIndex(MI_U32 u32PadMode);
MI_U8   DRV_DISP_OS_GpioDrvSet(MI_U8 u8GpioIdx, MI_U8 u8Level);
MI_U8   DRV_DISP_OS_GpioDrvGet(MI_U8 u8GpioIdx, MI_U8 *pu8Level);
MI_U8   DRV_DISP_OS_SetLpllClkConfig(MI_U64 u64Dclk, MI_U8 bDsi, MI_U8 u8LpllId);
MI_U8   DRV_DISP_OS_SetPixelClkConfig(MI_U32 u32Dev, MI_U32 u32ParentClkIndex);

#endif
