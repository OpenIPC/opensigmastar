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
#ifndef _SG_INIT_H_
#define _SG_INIT_H_

#include "mi_common_datatype.h"

typedef void* MI_SYS_VMA_HANDLE;
typedef void* MI_SYS_SG_SgTableHanle;

MI_SYS_SG_SgTableHanle MI_SYS_SG_AllocSgTable(MI_S32 size);
void MI_SYS_SG_FreeSgTable(MI_SYS_SG_SgTableHanle sgt);
MI_SYS_SG_SgTableHanle MI_SYS_SG_DupSgTable(MI_SYS_SG_SgTableHanle src);
void* MI_SYS_SG_GetSgList(MI_SYS_SG_SgTableHanle sgt);
void* MI_SYS_SG_SetPageStep(void *sgl, MI_PHY cpuBusAddr, MI_S32 length);
void* MI_SYS_SG_RemapVmaOnly(MI_SYS_VMA_HANDLE handle, MI_PHY u64PhyAddr, MI_U32 u32Size, MI_BOOL bCache);
void* MI_SYS_SG_MapSgTableToUser(MI_SYS_VMA_HANDLE handle, MI_SYS_SG_SgTableHanle sgt, MI_BOOL bCache);
void* MI_SYS_SG_MapSgTableToKernl(MI_SYS_SG_SgTableHanle sgt, MI_BOOL bCache);


#endif  /* ifndef _SG_INIT_H_ */


