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
#ifndef _MI_SYS_SIDEBAND_DATATYPE_H_
#define _MI_SYS_SIDEBAND_DATATYPE_H_

#include "mi_common.h"

#define MI_SYS_SIDEBAND_DATA_FRAMEID       0x00000001
#define MI_SYS_SIDEBAND_DATA_AEINFO        0x00000002
#define MI_SYS_SIDEBAND_DATA_FRAMESTARTPTS 0x00000003

typedef struct MI_SYS_SidebandData_AeInfo_s {
    MI_U8  u8SnrId;
    MI_U32 u32IspGain;
    MI_U32 u32SnrGain;
    MI_U32 u32Shutter;
} MI_SYS_SidebandData_AeInfo_t;

#endif

