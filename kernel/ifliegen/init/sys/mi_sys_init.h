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

#ifndef __MI_SYS_INIT_H__
#define __MI_SYS_INIT_H__

#include <mi_common.h>
#include <mi_common_internal.h>
#include <mi_common_datatype.h>
#include <mi_sys.h>

#if defined(CONFIG_ENABLE_MI_NORMAL_STR)
void MI_SYS_SetFreezable(void);
MI_S32 MI_SYS_TryToFreeze(void);
MI_S32 MI_SYS_IMPL_DeInitPmNotify(void);
MI_S32 MI_SYS_IMPL_InitPmNotify(void);
#endif

MI_U32 MI_SYS_FreeReservedArea(ss_phys_addr_t start, ss_phys_addr_t end, int poison, const char *s);
MI_S32 MI_SYS_GetProcessName(MI_U8 *szName, MI_U32 u32Length);
void *MI_SYS_Buf_GetCurUserMapCtx(void);
void MI_SYS_ShowAllTask(void);
char *parese_Cmdline(char *str);
MI_BOOL MI_SYS_Vma_Is_Va_Invalid(void *va, MI_U32 u32Size);
MI_BOOL MI_SYS_Is_KernVa_Invalid(void *va, MI_U32 u32Size);
void *MI_SYS_VMA_DoGetTrueVa(void *vir_addr);
MI_BOOL MI_SYS_Pass_IsUnmapVmallocAddr(void *pVirtAddr);
void *MI_SYS_FileGet(MI_S32 s32Fd);
void *MI_SYS_FilePrivGet(void *filp);
void MI_SYS_FilePut(void *filp);

#endif
