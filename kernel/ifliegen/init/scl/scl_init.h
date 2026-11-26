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

#ifndef __HAL_SCL_INIT_H__
#define __HAL_SCL_INIT_H__

typedef enum
{
    E_HAL_SCL_COMMON_INMODE_ISP = 0,
    E_HAL_SCL_COMMON_INMODE_VIF,
    E_HAL_SCL_COMMON_INMODE_RDMA,
    E_HAL_SCL_COMMON_INMODE_RDMA1,
    E_HAL_SCL_COMMON_INMODE_ROT,
    E_HAL_SCL_COMMON_INMODE_VDEC,
    E_HAL_SCL_COMMON_INMODE_MVOP,
    E_HAL_SCL_COMMON_INMODE_MAX,
} HAL_SCL_COMMON_InModeType_e;

void * HAL_SCL_MODULE_GetDevNodeHandle(void);
void HAL_SCL_MODULE_MatchIrqID(void *pDevNodeHandle,MI_U32 *pu32IrqID);
#endif //
