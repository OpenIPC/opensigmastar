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
#if (defined CONFIG_DMABUF_HEAPS)
#include <linux/dma-fence.h>
#include <linux/dma-heap.h>
#include <linux/dma-buf.h>
#include <linux/sched.h>
#include <linux/scatterlist.h>
#include <linux/fdtable.h>
#include <cam_sysfs.h>
#include <mi_common_datatype.h>
#include <mi_common.h>
#include <mi_common_internal.h>
#include <mi_common_datatype.h>
#include <mi_sys.h>
#include "dmabuf_init.h"
#include "common_print.h"
#include <cam_os_wrapper.h>
#include <cam_device_wrapper.h>

extern MI_SYS_DMABUF_DmaHeapBufHandle MI_SYS_DMABUF_ExporterAllocBuf(MI_SYS_DMABUF_DmaHeapInfo_t* pstDmaHeapInfo, MI_U32 len);
extern MI_S32 MI_SYS_DMABUF_ExporterFreeBuf(MI_SYS_DMABUF_DmaHeapBufHandle handle);
extern MI_SYS_DMABUF_Attachment_t* MI_SYS_DMABUF_ExporterAllocAttach(MI_SYS_DMABUF_DmaHeapBufHandle bufHandle, MI_PHY *phyAddr, MI_U32 *u32Size);
extern MI_S32 MI_SYS_DMABUF_ExporterFreeAttach(MI_SYS_DMABUF_DmaHeapBufHandle bufHandle, MI_SYS_DMABUF_Attachment_t *pstBufAttachment);
extern MI_SYS_SG_SgTableHanle MI_SYS_DMABUF_ExporterMap(MI_SYS_DMABUF_Attachment_t *pstBufAttachment);
extern void MI_SYS_DMABUF_ExporterUnMap(MI_SYS_DMABUF_Attachment_t *pstBufAttachment);
extern MI_S32 MI_SYS_DMABUF_ExporterBeginCpuAccess(MI_SYS_DMABUF_DmaHeapBufHandle bufHandle, MI_U32 direction);
extern MI_S32 MI_SYS_DMABUF_ExporterEndCpuAccess(MI_SYS_DMABUF_DmaHeapBufHandle bufHandle, MI_U32 direction);
extern MI_S32 MI_SYS_DMABUF_ExporterMmap(MI_SYS_DMABUF_DmaHeapBufHandle bufHandle, MI_SYS_VMA_HANDLE vma);
extern void* MI_SYS_DMABUF_ExporterVmap(MI_SYS_DMABUF_DmaHeapBufHandle bufHandle);
extern void MI_SYS_DMABUF_ExporterVunmap(MI_SYS_DMABUF_DmaHeapBufHandle bufHandle, void *vaddr);

static struct device *g_pstDmaBufExporterDev;

static int _MI_SYS_DMABUF_ExporterAttachMma(struct dma_buf *dmabuf,
                    struct dma_buf_attachment *attachment)
{
    MI_PHY phyAddr = 0;
    MI_U32 u32Size = 0;
    MI_SYS_DMABUF_Attachment_t *pstBufAttachment;
    struct sg_table *table;

    pstBufAttachment = MI_SYS_DMABUF_ExporterAllocAttach(dmabuf->priv, &phyAddr, &u32Size);
    if (!pstBufAttachment)
    {
        DBG_ERR("attch fail\n");
        return -1;
    }
    attachment->priv = pstBufAttachment;

    table = (struct sg_table *)pstBufAttachment->table;

    sg_dma_address(table->sgl) = phyAddr;
    sg_dma_len(table->sgl) = u32Size;
    table->nents = 1;

    return MI_SUCCESS;
}

static void _MI_SYS_DMABUF_ExporterDetachMma(struct dma_buf *dmabuf,
        struct dma_buf_attachment *attachment)
{
    MI_SYS_DMABUF_ExporterFreeAttach(dmabuf->priv, attachment->priv);
    attachment->priv = NULL;
}

static struct sg_table *_MI_SYS_DMABUF_ExporterMapMma(struct dma_buf_attachment *attachment,
                                                    enum dma_data_direction    direction)
{
    return MI_SYS_DMABUF_ExporterMap(attachment->priv);
}

static void _MI_SYS_DMABUF_ExporterUnmapMma(struct dma_buf_attachment *attachment, struct sg_table *table,
                                          enum dma_data_direction direction)
{
    MI_SYS_DMABUF_ExporterUnMap(attachment->priv);
}

static void _MI_SYS_DMABUF_ExporterReleaseMma(struct dma_buf *dmabuf)
{
    MI_SYS_DMABUF_ExporterFreeBuf(dmabuf->priv);
    dmabuf->priv = NULL;
}

static int _MI_SYS_DMABUF_ExporterBeginCpuAccessMma(struct dma_buf *dmabuf, enum dma_data_direction direction)
{
    return MI_SYS_DMABUF_ExporterBeginCpuAccess(dmabuf->priv, (MI_U32)direction);
}

static int _MI_SYS_DMABUF_ExporterEndCpuAccessMma(struct dma_buf *dmabuf, enum dma_data_direction direction)
{
    return MI_SYS_DMABUF_ExporterEndCpuAccess(dmabuf->priv, (MI_U32)direction);
}

static int _MI_SYS_DMABUF_ExporterMmapMma(struct dma_buf *dmabuf, struct vm_area_struct *vma)
{
    return MI_SYS_DMABUF_ExporterMmap(dmabuf->priv, vma);
}

static void *_MI_SYS_DMABUF_ExporterVmapMma(struct dma_buf *dmabuf)
{
    return MI_SYS_DMABUF_ExporterVmap(dmabuf->priv);
}

static void _MI_SYS_DMABUF_ExporterVunmapMma(struct dma_buf *dmabuf, void *vaddr)
{
    return MI_SYS_DMABUF_ExporterVunmap(dmabuf->priv, vaddr);
}

static struct dma_buf_ops g_miSysExporterMmaDmaBufOps = {
    .attach           = _MI_SYS_DMABUF_ExporterAttachMma,
    .detach           = _MI_SYS_DMABUF_ExporterDetachMma,
    .map_dma_buf      = _MI_SYS_DMABUF_ExporterMapMma,
    .unmap_dma_buf    = _MI_SYS_DMABUF_ExporterUnmapMma,
    .release          = _MI_SYS_DMABUF_ExporterReleaseMma,
    .begin_cpu_access = _MI_SYS_DMABUF_ExporterBeginCpuAccessMma,
    .end_cpu_access   = _MI_SYS_DMABUF_ExporterEndCpuAccessMma,
    .mmap             = _MI_SYS_DMABUF_ExporterMmapMma,
    .vmap             = _MI_SYS_DMABUF_ExporterVmapMma,
    .vunmap           = _MI_SYS_DMABUF_ExporterVunmapMma,
};

static int _MI_SYS_DMABUF_ExporterAttachImi(struct dma_buf *dmabuf,
                    struct dma_buf_attachment *attachment)
{
    MI_PHY phyAddr = 0;
    MI_U32 u32Size = 0;
    MI_SYS_DMABUF_Attachment_t *pstBufAttachment;
    struct sg_table *table;

    pstBufAttachment = MI_SYS_DMABUF_ExporterAllocAttach(dmabuf->priv, &phyAddr, &u32Size);
    if (!pstBufAttachment)
    {
        DBG_ERR("attch fail\n");
        return -1;
    }
    attachment->priv = pstBufAttachment;

    table = (struct sg_table *)pstBufAttachment->table;

    sg_dma_address(table->sgl) = phyAddr;
    sg_dma_len(table->sgl) = u32Size;
    table->nents = 1;

    return MI_SUCCESS;

}

static void _MI_SYS_DMABUF_ExporterDetachImi(struct dma_buf *dmabuf,
        struct dma_buf_attachment *attachment)
{
    MI_SYS_DMABUF_ExporterFreeAttach(dmabuf->priv, attachment->priv);
    attachment->priv = NULL;
}

static struct sg_table *_MI_SYS_DMABUF_ExporterMapImi(struct dma_buf_attachment *attachment,
                                                    enum dma_data_direction    direction)
{
    return MI_SYS_DMABUF_ExporterMap(attachment->priv);
}

static void _MI_SYS_DMABUF_ExporterUnmapImi(struct dma_buf_attachment *attachment, struct sg_table *table,
                                          enum dma_data_direction direction)
{
    MI_SYS_DMABUF_ExporterUnMap(attachment->priv);
}

static void _MI_SYS_DMABUF_ExporterReleaseImi(struct dma_buf *dmabuf)
{
    MI_SYS_DMABUF_ExporterFreeBuf(dmabuf->priv);
    dmabuf->priv = NULL;
}

static struct dma_buf_ops g_miSysExporterImiDmaBufOps = {
    .attach           = _MI_SYS_DMABUF_ExporterAttachImi,
    .detach           = _MI_SYS_DMABUF_ExporterDetachImi,
    .map_dma_buf      = _MI_SYS_DMABUF_ExporterMapImi,
    .unmap_dma_buf    = _MI_SYS_DMABUF_ExporterUnmapImi,
    .release          = _MI_SYS_DMABUF_ExporterReleaseImi,
};

static struct dma_buf * _MI_SYS_DMABUF_ExporterAllocate(struct dma_heap *heap,
        unsigned long len,
        unsigned long fd_flags,
        unsigned long heap_flags)
{
    DEFINE_DMA_BUF_EXPORT_INFO(exp_info);
    MI_SYS_DMABUF_DmaHeapInfo_t *pstDmaHeapInfo = CamDmaHeapGetDrvData(heap);
    struct dma_buf *dmabuf;

    exp_info.flags = O_RDWR;
    exp_info.size  = len;

    if (pstDmaHeapInfo->eDmaHeapType == E_MI_SYS_DMABUF_MMA_HEAP)
    {
        exp_info.ops   = &g_miSysExporterMmaDmaBufOps;
    }
    else
    {
        exp_info.ops   = &g_miSysExporterImiDmaBufOps;
    }
    exp_info.priv = MI_SYS_DMABUF_ExporterAllocBuf(pstDmaHeapInfo, len);
    if (!exp_info.priv)
    {
        DBG_ERR("export dma-buf failed\n");
        return NULL;
    }

    dmabuf = CamDmaBufExport(&exp_info);
    if (CAM_OS_IS_ERR(dmabuf))
    {
        DBG_ERR("export dma-buf failed\n");
        MI_SYS_DMABUF_ExporterFreeBuf((MI_SYS_DMABUF_DmaHeapBufHandle)exp_info.priv);
        return NULL;
    }
    return dmabuf;
}

static long _MI_SYS_DMABUF_ExporterGetPoolSize(struct dma_heap *heap)
{
    return 0;
}

static struct dma_heap_ops g_miSysDmaHeapOps = {
    .allocate = _MI_SYS_DMABUF_ExporterAllocate,
    .get_pool_size = _MI_SYS_DMABUF_ExporterGetPoolSize,
};

static void MI_SYS_DMABUF_ExporterDevRelease(struct device *dev)
{
    CamOsMemRelease(g_pstDmaBufExporterDev);
    g_pstDmaBufExporterDev = NULL;
}

MI_S32 MI_SYS_DMABUF_ExporterDmaHeapAdd(MI_SYS_DMABUF_DmaHeapInfo_t *pstDmaHeapInfo)
{
    struct dma_heap_export_info exp_info;

    exp_info.name = pstDmaHeapInfo->heap_name;
    exp_info.ops = &g_miSysDmaHeapOps;
    exp_info.priv = pstDmaHeapInfo;

    pstDmaHeapInfo->private = CamDmaHeapAdd(&exp_info);
    if(CAM_OS_IS_ERR(pstDmaHeapInfo->private))
    {
        DBG_ERR("create dmabuf exporter failed\n");
        return MI_ERR_SYS_FAILED;
    }

    if (!g_pstDmaBufExporterDev)
    {
        g_pstDmaBufExporterDev = CamOsMemAlloc(sizeof(struct device));
        if (!g_pstDmaBufExporterDev)
        {
            DBG_ERR("create g_pstDmaBufExporterDev fail\n");
            MI_SYS_DMABUF_ExporterDmaHeapPut(pstDmaHeapInfo);
            return MI_ERR_SYS_FAILED;
        }
        CamOsMemset(g_pstDmaBufExporterDev, 0, sizeof(struct device));
        g_pstDmaBufExporterDev->release = MI_SYS_DMABUF_ExporterDevRelease;
        g_pstDmaBufExporterDev->init_name = "mi_sys_dmabuf_dev";
        CamDeviceRegister(g_pstDmaBufExporterDev);
    }

    return MI_SUCCESS;
}

MI_S32 MI_SYS_DMABUF_ExporterDmaHeapPut(MI_SYS_DMABUF_DmaHeapInfo_t *pstDmaHeapInfo)
{
    if(!CAM_OS_IS_ERR(pstDmaHeapInfo->private))
    {
        CamDmaHeapPut((struct dma_heap*)pstDmaHeapInfo->private);
        pstDmaHeapInfo->private = NULL;
    }

    if (g_pstDmaBufExporterDev)
    {
        CamDeviceUnregister(g_pstDmaBufExporterDev);
    }

    return MI_SUCCESS;
}

MI_BOOL MI_SYS_DMABUF_ExporterIsMmaDmaBufFd(MI_S32 s32Fd)
{
    struct dma_buf *dmabuf;
    MI_BOOL ret = FALSE;

    dmabuf = CamDmaBufGet(s32Fd);
    if(CAM_OS_IS_ERR(dmabuf))
    {
        DBG_WRN("invalid dmabuf fd:%d\n", s32Fd);
        return ret;
    }

    DBG_INFO("dmabuf name:%s, exporter name:%s",
                dmabuf->name, dmabuf->exp_name);

    if(dmabuf->ops == &g_miSysExporterMmaDmaBufOps)
    {
        ret = TRUE;
    }
    CamDmaBufPut(dmabuf);

    return ret;
}

void MI_SYS_DMABUF_ExporterSyncForCpu(MI_SYS_SG_SgTableHanle sgHandle, MI_U32 direction)
{
    struct sg_table *table = sgHandle;
    struct scatterlist *sg;
    MI_S32 si;

    for_each_sgtable_sg(table, sg, si)
    {
        dma_sync_single_range_for_cpu(g_pstDmaBufExporterDev, page_to_phys(sg_page(sg)),
                sg->offset, sg->length, direction);
    }
    DBG_INFO("dma sync for cpu, dev name:%s\n", dev_name(g_pstDmaBufExporterDev));
}

void MI_SYS_DMABUF_ExporterSyncForDevice(MI_SYS_SG_SgTableHanle sgHandle, MI_U32 direction)
{
    struct sg_table *table = sgHandle;
    struct scatterlist *sg;
    MI_S32 si;

    for_each_sgtable_sg(table, sg, si)
    {
        dma_sync_single_range_for_device(g_pstDmaBufExporterDev, page_to_phys(sg_page(sg)),
                sg->offset, sg->length, direction);
    }
    DBG_INFO("dma sync for cpu, dev name:%s\n", dev_name(g_pstDmaBufExporterDev));
}

typedef struct MI_SYS_DMABUF_PlaneInfo_s
{
    struct dma_buf              *dmabuf;
    struct dma_buf_attachment   *attachment;
    struct sg_table             *sg_table;
    enum dma_data_direction     direct;
    struct dma_fence            *fence;
    spinlock_t                  fenceLock;
    struct CamOsListHead_t      node;
} MI_SYS_DMABUF_PlaneInfo_t;

static MI_U64 g_u64DmaMask = 0xFFFFFFFFFFFFFFFF;
extern struct CamOsListHead_t g_miSysDmaBufRecList;
extern CamOsMutex_t gp_miSysDmaBufRecListMutex;

MI_SYS_DMABUF_PlaneHandle MI_SYS_DMABUF_HeapGetBuf(MI_S32 s32Fd, MI_SYS_DMABUF_DmaDirection_e direct)
{
    MI_SYS_DMABUF_PlaneInfo_t *pstDmaBufPlaneInfo;
    struct dma_buf              *dmabuf;
    struct dma_buf_attachment   *attachment;
    struct sg_table             *sg_table;

    g_pstDmaBufExporterDev->dma_mask = &g_u64DmaMask;

    dmabuf = CamDmaBufGet(s32Fd);
    if(!dmabuf)
    {
        DBG_ERR("invalid dmabuf fd:%d\n", s32Fd);
        return NULL;
    }

    attachment = CamDmaBufAttach(dmabuf, g_pstDmaBufExporterDev);
    if(!attachment)
    {
        DBG_ERR("failed attach dmabuf\n");
        CamDmaBufPut(dmabuf);
        return NULL;
    }

    sg_table = CamDmaBufMapAttachment(attachment, direct);
    if(!sg_table)
    {
        DBG_ERR("failed map dmabuf\n");
        CamDmaBufDetach(dmabuf, attachment);
        CamDmaBufPut(dmabuf);
        return NULL;
    }

    pstDmaBufPlaneInfo = CamOsMemAlloc(sizeof(MI_SYS_DMABUF_PlaneInfo_t));
    if (!pstDmaBufPlaneInfo)
    {
        DBG_ERR("alloc fail \n");
        CamDmaBufUnmapAttachment(attachment, sg_table, direct);
        CamDmaBufDetach(dmabuf, attachment);
        CamDmaBufPut(dmabuf);
        return NULL;
    }
    pstDmaBufPlaneInfo->dmabuf = dmabuf;
    pstDmaBufPlaneInfo->attachment = attachment;
    pstDmaBufPlaneInfo->sg_table = (MI_SYS_SG_SgTableHanle)sg_table;
    pstDmaBufPlaneInfo->direct = direct;

    CamOsMutexLock(&gp_miSysDmaBufRecListMutex);
    CAM_OS_LIST_ADD(&pstDmaBufPlaneInfo->node, &g_miSysDmaBufRecList);
    CamOsMutexUnlock(&gp_miSysDmaBufRecListMutex);

    return (MI_SYS_DMABUF_PlaneHandle) pstDmaBufPlaneInfo;
}


MI_S32 MI_SYS_DMABUF_HeapPutBuf(MI_SYS_DMABUF_PlaneHandle handle)
{
    MI_SYS_DMABUF_PlaneInfo_t *pstDmaBufPlaneInfo = (MI_SYS_DMABUF_PlaneInfo_t *)handle;
    if (!pstDmaBufPlaneInfo)
    {
        return MI_ERR_SYS_FAILED;
    }
    CamDmaBufUnmapAttachment(pstDmaBufPlaneInfo->attachment, pstDmaBufPlaneInfo->sg_table, pstDmaBufPlaneInfo->direct);
    CamDmaBufDetach(pstDmaBufPlaneInfo->dmabuf, pstDmaBufPlaneInfo->attachment);
    CamDmaBufPut(pstDmaBufPlaneInfo->dmabuf);

    CamOsMutexLock(&gp_miSysDmaBufRecListMutex);
    CAM_OS_LIST_DEL(&pstDmaBufPlaneInfo->node);
    CamOsMutexUnlock(&gp_miSysDmaBufRecListMutex);

    CamOsMemRelease(pstDmaBufPlaneInfo);
    return MI_SUCCESS;
}

MI_PHY MI_SYS_DMABUF_HeapGetBufAddr(MI_SYS_DMABUF_PlaneHandle handle)
{
    MI_SYS_DMABUF_PlaneInfo_t *pstDmaBufPlaneInfo = (MI_SYS_DMABUF_PlaneInfo_t *)handle;
    if (pstDmaBufPlaneInfo)
    {
        return sg_dma_address(pstDmaBufPlaneInfo->sg_table->sgl);
    }
    return 0;
}

MI_S32 MI_SYS_DMABUF_HeapGetBufLength(MI_SYS_DMABUF_PlaneHandle handle)
{
    MI_SYS_DMABUF_PlaneInfo_t *pstDmaBufPlaneInfo = (MI_SYS_DMABUF_PlaneInfo_t *)handle;
    if (pstDmaBufPlaneInfo)
    {
        return sg_dma_len(pstDmaBufPlaneInfo->sg_table->sgl);
    }
    return 0;
}

MI_SYS_DMABUF_PlaneHandle MI_SYS_DMABUF_HeapGetBufByPhyAddr(MI_PHY phyAddr)
{
    MI_SYS_DMABUF_PlaneInfo_t *pstDmaBufPlaneInfo = NULL, *tmp = NULL;

    CamOsMutexLock(&gp_miSysDmaBufRecListMutex);
    CAM_OS_LIST_FOR_EACH_ENTRY_SAFE(pstDmaBufPlaneInfo, tmp, &g_miSysDmaBufRecList, node)
    {
        if (sg_dma_address(pstDmaBufPlaneInfo->sg_table->sgl) == phyAddr)
        {
            CamOsMutexUnlock(&gp_miSysDmaBufRecListMutex);
            return (MI_SYS_DMABUF_PlaneHandle)pstDmaBufPlaneInfo;
        }
    }
    CamOsMutexUnlock(&gp_miSysDmaBufRecListMutex);
    return NULL;
}


static const char *_MI_SYS_DMABUF_HeapFenceName(struct dma_fence *f)
{
    return "mi_sys";
}

static const struct dma_fence_ops gDmaFenceOps = {
    .get_driver_name   = _MI_SYS_DMABUF_HeapFenceName,
    .get_timeline_name = _MI_SYS_DMABUF_HeapFenceName,
};

MI_S32 MI_SYS_DMABUF_FenceInit(MI_SYS_DMABUF_PlaneHandle handle)
{
    MI_SYS_DMABUF_PlaneInfo_t *pstDmaBufPlaneInfo = (MI_SYS_DMABUF_PlaneInfo_t *)handle;
    struct dma_resv  *pstResv = pstDmaBufPlaneInfo->dmabuf->resv;
    struct dma_fence *fence;
    MI_S32            s32Ret;

    fence = CamOsMemAlloc(sizeof(*fence));
    if (!fence)
    {
        DBG_ERR("failed to alloc dma fence\n");
        return MI_ERR_SYS_NOMEM;
    }

    spin_lock_init(&pstDmaBufPlaneInfo->fenceLock);
    dma_fence_init(fence, &gDmaFenceOps, &pstDmaBufPlaneInfo->fenceLock, dma_fence_context_alloc(1), 0);

    s32Ret = CamDmaResvLock(pstResv, NULL);
    if (s32Ret)
    {
        DBG_ERR("failed to lock dma resv\n");
        dma_fence_put(fence);
        return MI_ERR_SYS_FAILED;
    }

    s32Ret = dma_resv_reserve_shared(pstResv, 1);
    if (s32Ret)
    {
        DBG_ERR("dma buf failed to reserve shared\n");
        CamDmaResvUnlock(pstResv);
        dma_fence_put(fence);
        return MI_ERR_SYS_NOMEM;
    }

    dma_resv_add_shared_fence(pstResv, fence);
    pstDmaBufPlaneInfo->fence = fence;
    CamDmaResvUnlock(pstResv);

    return MI_SUCCESS;
}

MI_S32 MI_SYS_DMABUF_FenceDeinit(MI_SYS_DMABUF_PlaneHandle handle)
{
    MI_SYS_DMABUF_PlaneInfo_t *pstDmaBufPlaneInfo = (MI_SYS_DMABUF_PlaneInfo_t *)handle;

    dma_fence_put(pstDmaBufPlaneInfo->fence);
    pstDmaBufPlaneInfo->fence = NULL;
    return MI_SUCCESS;
}

MI_S32 MI_SYS_DMABUF_FenceSignal(MI_SYS_DMABUF_PlaneHandle handle)
{
    MI_SYS_DMABUF_PlaneInfo_t *pstDmaBufPlaneInfo = (MI_SYS_DMABUF_PlaneInfo_t *)handle;

    dma_fence_signal(pstDmaBufPlaneInfo->fence);
    return MI_SUCCESS;
}

/* Remove an unsignaled dma fence */
void MI_SYS_DMABUF_FenceRemove(MI_SYS_DMABUF_PlaneHandle handle)
{
    MI_SYS_DMABUF_PlaneInfo_t *pstDmaBufPlaneInfo = (MI_SYS_DMABUF_PlaneInfo_t *)handle;
    struct dma_resv  *pstResv = pstDmaBufPlaneInfo->dmabuf->resv;
    struct dma_fence *fence   = pstDmaBufPlaneInfo->fence;

    if (fence)
    {
        dma_fence_signal(fence);
    }

    if (!CamDmaResvLock(pstResv, NULL))
    {
        dma_resv_add_shared_fence(pstResv, dma_fence_get_stub());
        CamDmaResvUnlock(pstResv);
    }
    else
    {
        DBG_ERR("failed to lock dma resv\n");
    }
}

#endif

