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
#include <linux/scatterlist.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <mi_sys.h>
#include <mi_common_datatype.h>
#include <mi_common.h>
#include <mi_common_internal.h>
#include <mi_common_datatype.h>
#include <mi_sys.h>
#include <cam_os_wrapper.h>
#include "sg_init.h"
#include "common_print.h"
#include <linux/mman.h>

#define ALIGN_UP(val, alignment) ((((val) + (alignment)-1) / (alignment)) * (alignment))

MI_SYS_SG_SgTableHanle MI_SYS_SG_AllocSgTable(MI_S32 size)
{
    MI_S32 ret;
    struct sg_table *sg_table;

    sg_table = CamOsMemAlloc(sizeof(struct sg_table));
    if (!sg_table)
    {
        DBG_ERR(" fail\n");
        return NULL;
    }

    ret = sg_alloc_table(sg_table, size, GFP_KERNEL);
    if (CAM_OS_UNLIKELY(ret))
    {
        DBG_ERR(" fail\n");
        CamOsMemRelease(sg_table);
        return NULL;
    }
    return sg_table;
}

void MI_SYS_SG_FreeSgTable(MI_SYS_SG_SgTableHanle sgt)
{
    struct sg_table *sg_table = sgt;
    sg_free_table(sg_table);
    CamOsMemRelease(sg_table);
}

MI_SYS_SG_SgTableHanle MI_SYS_SG_DupSgTable(MI_SYS_SG_SgTableHanle src)
{
    int i;
    MI_SYS_SG_SgTableHanle dest;
    struct sg_table *src_table = (struct sg_table *)src, *dest_table = NULL;
    struct scatterlist *sg, *new_sg;

    dest = MI_SYS_SG_AllocSgTable(src_table->orig_nents);
    if (!dest)
    {
        return NULL;
    }
    dest_table = (struct sg_table *)dest;

    new_sg = dest_table->sgl;
    for_each_sgtable_sg(src_table, sg, i)
    {
        sg_set_page(new_sg, sg_page(sg), sg->length, sg->offset);
        new_sg = sg_next(new_sg);
    }

    return dest;
}


void *MI_SYS_SG_GetSgList(MI_SYS_SG_SgTableHanle sgt)
{
    struct sg_table *sg_table = sgt;
    return sg_table->sgl;
}

void *MI_SYS_SG_SetPageStep(void *sg, MI_PHY cpuBusAddr, MI_S32 length)
{
    sg_set_page(sg, pfn_to_page(__phys_to_pfn(cpuBusAddr)), length, offset_in_page(cpuBusAddr));
    sg = sg_next(sg);

    DBG_INFO("sg_set_page cpuBusAddr %llx, miuAddr:%llx, page:%p length:%x offset_in_page:%lx\n",
            cpuBusAddr, CamOsMemPhysToMiu(cpuBusAddr),
            pfn_to_page(__phys_to_pfn(cpuBusAddr)), length, offset_in_page(cpuBusAddr));
    return sg;
}

/**
 * MI_SYS_SG_RemapVmaOnly: remap continuous physical addr to user
 * no need sg_table, call linux api remap_pfn_range directly.
 * unmap: MI_SYS_Vma_Munmap/MI_SYS_Vma_UserUnmap.
 */

/*
 * The caller must write-lock current->mm->mmap_lock.
 * If call this api in mmap internal callback function.
 * write-lock current->mm->mmap_lock has done, no need again.
 */
void * MI_SYS_SG_RemapVmaOnly(MI_SYS_VMA_HANDLE handle, MI_PHY u64PhyAddr, MI_U32 u32Size, MI_BOOL bCache)
{
    MI_S32 ret;
    unsigned long remap_addr = 0;
    struct vm_area_struct *vma = (struct vm_area_struct *)handle;

    BUG_ON(!vma);
    if (offset_in_page(u64PhyAddr))
    {
        DBG_ERR("phyadd error 0x%llx", u64PhyAddr);
        return NULL;
    }

    if (!current->mm)
    {
        DBG_ERR("current->mm not exist, return NULL\n");
        return NULL;
    }

    if (bCache)
    {
        vma->vm_page_prot = vm_get_page_prot(vma->vm_flags);
    }
    else
    {
        vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
    }

    BUG_ON(u32Size < vma->vm_end - vma->vm_start);
    ret = io_remap_pfn_range(vma, vma->vm_start, __phys_to_pfn(CamOsMemMiuToPhys(u64PhyAddr)) + vma->vm_pgoff,
                                vma->vm_end - vma->vm_start, vma->vm_page_prot);
    DBG_INFO("io_remap_pfn_range (%lx %llx %lx)\n", vma->vm_start, u64PhyAddr, vma->vm_end - vma->vm_start);

    if (ret)
    {
        DBG_ERR("io_remap_pfn_range fail\n");
        remap_addr = 0;
    }
    else
    {
        remap_addr = vma->vm_start;
    }

    return (void *)remap_addr;
}

/**
 * MI_SYS_SG_MapSgTableToUser: mmap discontinuous physical addr to user
 * mode virtual addr using sg_table.
 * unmap: MI_SYS_Vma_Munmap/MI_SYS_Vma_UserUnmap.
 */
void * MI_SYS_SG_MapSgTableToUser(MI_SYS_VMA_HANDLE handle, MI_SYS_SG_SgTableHanle sgt, MI_BOOL bCache)
{
    int i=0;
    unsigned long populate;
    struct scatterlist *sg;
    unsigned long total_len=0;
    struct vm_area_struct *vma = NULL;
    unsigned long virt_addr = 0;
    unsigned long remap_addr = 0;
    unsigned long map_offset = 0;
    unsigned long remainder = 0;
    struct sg_table *sg_table = sgt;

    BUG_ON(!sg_table);

    if (!current->mm)
    {
        DBG_ERR("current->mm not exist, return NULL\n");
        return NULL;
    }

    for_each_sg(sg_table->sgl, sg, sg_table->nents, i)
    {
        BUG_ON(i != 0 && sg->offset != 0);
        if (i == 0 && sg->offset != 0)
        {
            // first nents, allow offset != 0, else nents, offset always == 0.
            map_offset = sg->offset;
        }
        total_len += sg->offset + sg->length;
        DBG_INFO("%lx, 0x%x 0x%x %d\n", total_len, sg->length, sg->offset, i);
    }
    DBG_INFO("total_len: %lx\n", total_len);

    if(!handle)
    {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,8,0)
        down_write(&current->mm->mmap_lock);
#else
        down_write(&current->mm->mmap_sem);
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,9,0)
        virt_addr = do_mmap(NULL, 0, total_len, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_NORESERVE, 0, &populate, NULL);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0)
        virt_addr = do_mmap_pgoff(NULL, 0, total_len, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_NORESERVE, 0, &populate, NULL);
#else
        virt_addr = do_mmap_pgoff(NULL, 0, total_len, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_NORESERVE, 0, &populate);
#endif

        if(IS_ERR_VALUE(virt_addr) || virt_addr == 0)
        {
            goto RET_BUSY;
        }
        vma = find_vma(current->mm , virt_addr);
        if (!vma || vma->vm_start > virt_addr)
        {
            goto RET_BUSY;
        }
    }
    else
    {
        /*
         * The caller must write-lock current->mm->mmap_lock.
         * If call this api in mmap internal callback function.
         * write-lock current->mm->mmap_lock has done, no need again.
         */
        vma = (struct vm_area_struct *)handle;
        virt_addr = vma->vm_start;
    }
    DBG_INFO("vma->vm_start=%lx, vma->vm_end=%lx, virt_addr=%lx, total_len=%lx\n",
            vma->vm_start, vma->vm_end, virt_addr, total_len);

    BUG_ON(vma->vm_start != virt_addr);
    BUG_ON(vma->vm_end - vma->vm_start > ALIGN_UP(total_len, PAGE_SIZE));

    remap_addr = virt_addr;
    remainder = vma->vm_end - vma->vm_start;

    for_each_sg(sg_table->sgl, sg, sg_table->nents, i)
    {
        struct page *page = sg_page(sg);
        MI_S32 ret;
        unsigned long len = sg->offset + sg->length;
        len = min(len, remainder);

        BUG_ON(i != 0 && sg->offset != 0);

        vma->vm_pgoff = 0;
        if (bCache)
        {
            vma->vm_page_prot = vm_get_page_prot(vma->vm_flags);
        }
        else
        {
            vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
        }

        ret = io_remap_pfn_range(vma, remap_addr, page_to_pfn(page), len,
                              vma->vm_page_prot);
        DBG_INFO("io_remap_pfn_range (%lx %lx %x %x)\n", remap_addr, len, sg->length, sg->offset);

        if (ret)
        {
            DBG_ERR("io_remap_pfn_range fail\n");
            goto RET_BUSY;
        }

        remap_addr += len;
        remainder -= len;

        if(!remainder)
        {
            break;
        }
    }

    BUG_ON(total_len < remap_addr - virt_addr);

    // vma->vm_ops = NULL;
    vma->vm_flags = (vma->vm_flags | VM_DONTCOPY);
    vma->vm_private_data = NULL;

    remap_addr = vma->vm_start + map_offset;
    //MI_SYS_BUG_ON(remap_addr % sizeof(long) != 0);

    if (!handle)
    {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,8,0)
        up_write(&current->mm->mmap_lock);
#else
        up_write(&current->mm->mmap_sem);
#endif
    }


    DBG_INFO("remap_addr=%lx, total_len=%lx\n",remap_addr, total_len);
    return (void *)remap_addr;

RET_BUSY:
    DBG_ERR("[thread:%s pid:%d] virt_addr:%lx, vma:%px, total_len:%lx, maybe mmap leak!!!\n",
            current->comm, current->pid, virt_addr, vma, total_len);

    // virt_addr is mmap by self, need munmap if do fail.
    if(!handle)
    {
        if (virt_addr && !IS_ERR_VALUE((unsigned long)(virt_addr)))
        {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0)
            do_munmap(current->mm, (unsigned long)virt_addr, total_len, NULL);
#else
            do_munmap(current->mm, (unsigned long)virt_addr, total_len);
#endif
        }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,8,0)
        up_write(&current->mm->mmap_lock);
#else
        up_write(&current->mm->mmap_sem);
#endif
    }

    return NULL;
}

/**
 * MI_SYS_SG_MapSgTableToKernl: mmap discontinuous physical addr to kernel
 * mode virtual addr using sg_table.
 * unmap: MI_SYS_Vma_UnVmap/MI_SYS_Vma_VunmapKern.
 */
void * MI_SYS_SG_MapSgTableToKernl(MI_SYS_SG_SgTableHanle sgt, MI_BOOL bCache)
{
    struct scatterlist *sg;
    int page_count=0;
    struct page **pages;
    int i, j;
    void *kern_map_ptr = NULL;
    unsigned long map_offset = 0;
    pgprot_t pg_prot;
    int                 page_total = 0;
    struct sg_table *sg_table = sgt;

    BUG_ON(!sg_table);

    if (bCache)
        pg_prot = PAGE_KERNEL;
    else
        pg_prot = pgprot_writecombine(PAGE_KERNEL);

    for_each_sg(sg_table->sgl, sg, sg_table->nents, i)
    {
        BUG_ON(i != 0 && sg->offset != 0);
        if (i == 0 && sg->offset != 0)
        {
            // first nents, allow offset != 0, else nents, offset always == 0.
            map_offset = sg->offset;
        }

        page_count += PAGE_ALIGN(sg->offset + sg->length) / PAGE_SIZE;
        DBG_INFO("page_count:%d, sg->length:0x%x sg->offset:0x%x i:%d\n",
                page_count, sg->length, sg->offset, i);
    }
    BUG_ON(!page_count);
    DBG_INFO("page_count:%d\n", page_count);

    pages = vmalloc(sizeof(struct page*)*page_count);
    if(!pages)
    {
        DBG_ERR("vmalloc fail\n");
        return NULL;
    }

    for_each_sg(sg_table->sgl, sg, sg_table->nents, i)
    {
        BUG_ON(i != 0 && sg->offset != 0);
        page_count = PAGE_ALIGN(sg->offset + sg->length) / PAGE_SIZE;
        for( j = 0; j < page_count; j++)
        {
            pages[page_total + j] =  sg_page(sg) + j;
        }
        page_total += page_count;
    }

    kern_map_ptr = vmap(pages, page_total, VM_MAP, pg_prot);
    DBG_INFO("kern_map_ptr:%lx, return_addr:%lx\n",
            (unsigned long)kern_map_ptr, (unsigned long)kern_map_ptr + map_offset);
    vfree(pages);
    kern_map_ptr = (void*)((unsigned long)kern_map_ptr + map_offset);
    BUG_ON((unsigned long)kern_map_ptr % sizeof(long) != 0);
    return kern_map_ptr;
}


