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

#ifndef __IPU_INIT_H__
#define __IPU_INIT_H__

#include "mi_common_internal.h"
#include "mi_ipu_datatype.h"

typedef struct MI_IPU_Freq_Vol_s {
    unsigned int freq_mhz;
    unsigned int voltage_mv;
} MI_IPU_Freq_Vol_t;

typedef int (*scaling_freq_func)(unsigned int);
typedef int (*invoke_by_kernel_func)(MI_U32 , MI_IPU_BatchInvokeParam_t *, MI_IPU_RuntimeInfo_t *);

typedef struct MI_IPU_Common_Callback_s {
    scaling_freq_func scaling_freq;
    invoke_by_kernel_func invoke_by_kernel;
} MI_IPU_Common_Callback_t;

int MI_IPU_ParseIrq(unsigned int *pu32Virq, int len);
int MI_IPU_GetPllClk(void **ppPllClk);
int MI_IPU_LinuxInit(MI_IPU_Common_Callback_t *pstCommonCallback);
MI_S32 MI_IPU_Parse_Freq_Vol_Table(char *pCompatible, MI_IPU_Freq_Vol_t **ppstIpuOppTb, unsigned int *pTbLen);
void MI_IPU_Iounmap_Wrapper(void *addr);

#endif
