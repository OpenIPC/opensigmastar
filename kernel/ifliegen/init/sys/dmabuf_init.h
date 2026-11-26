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

#ifndef __DMABUF_INIT_H__
#define __DMABUF_INIT_H__

#include "sg_init.h"
#include "cam_os_wrapper.h"

typedef void* MI_SYS_DMABUF_PlaneHandle;
typedef void* MI_SYS_DMABUF_DmaHeapBufHandle;

typedef enum
{
    E_MI_SYS_DMABUF_MMA_HEAP,
    E_MI_SYS_DMABUF_IMI_HEAP
} MI_SYS_DMABUF_DmaHeapType_e;

typedef enum {
    MI_SYS_DMABUF_BIDIRECTIONAL = 0,
    MI_SYS_DMABUF_TO_DEVICE = 1,
    MI_SYS_DMABUF_FROM_DEVICE = 2,
    MI_SYS_DMABUF_NONE = 3,
} MI_SYS_DMABUF_DmaDirection_e;

typedef struct MI_SYS_DMABUF_DmaHeapInfo_s
{
    void *private;
    MI_SYS_DMABUF_DmaHeapType_e eDmaHeapType;
    char *heap_name;
} MI_SYS_DMABUF_DmaHeapInfo_t;

typedef struct MI_SYS_DMABUF_Attachment_s
{
    MI_SYS_SG_SgTableHanle table;
    struct CamOsListHead_t node_in_list;
    MI_BOOL bMapped;
} MI_SYS_DMABUF_Attachment_t;

MI_S32 MI_SYS_DMABUF_ExporterDmaHeapAdd(MI_SYS_DMABUF_DmaHeapInfo_t *pstDmaHeapInfo);
MI_S32 MI_SYS_DMABUF_ExporterDmaHeapPut(MI_SYS_DMABUF_DmaHeapInfo_t *pstDmaHeapInfo);
void MI_SYS_DMABUF_ExporterSyncForCpu(MI_SYS_SG_SgTableHanle sgHandle, MI_U32 direction);
void MI_SYS_DMABUF_ExporterSyncForDevice(MI_SYS_SG_SgTableHanle sgHandle, MI_U32 direction);
MI_BOOL MI_SYS_DMABUF_ExporterIsMmaDmaBufFd(MI_S32 s32Fd);

MI_SYS_DMABUF_PlaneHandle MI_SYS_DMABUF_HeapGetBuf(MI_S32 s32Fd, MI_SYS_DMABUF_DmaDirection_e direct);
MI_S32 MI_SYS_DMABUF_HeapPutBuf(MI_SYS_DMABUF_PlaneHandle handle);
MI_PHY MI_SYS_DMABUF_HeapGetBufAddr(MI_SYS_DMABUF_PlaneHandle handle);
MI_S32 MI_SYS_DMABUF_HeapGetBufLength(MI_SYS_DMABUF_PlaneHandle handle);
MI_SYS_DMABUF_PlaneHandle MI_SYS_DMABUF_HeapGetBufByPhyAddr(MI_PHY phyAddr);
MI_S32 MI_SYS_DMABUF_FenceInit(MI_SYS_DMABUF_PlaneHandle handle);
MI_S32 MI_SYS_DMABUF_FenceDeinit(MI_SYS_DMABUF_PlaneHandle handle);
MI_S32 MI_SYS_DMABUF_FenceSignal(MI_SYS_DMABUF_PlaneHandle handle);
void MI_SYS_DMABUF_FenceRemove(MI_SYS_DMABUF_PlaneHandle handle);

#endif


