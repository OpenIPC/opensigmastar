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
#ifndef __COMMON_INIT_H__
#define __COMMON_INIT_H__

void common_kernel_neon_begin(void);
void common_kernel_neon_end(void);

int MI_DEVICE_IsMmExist(void);
int MI_DEVICE_GetTgid(void);
int MI_DEVICE_GetPid(void);

void MI_COMMON_RecodeTimeStamp(int mark, const char *name);

#endif // !__COMMON_INIT_H__