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
#include "mi_sys_init.h"

#include <linux/module.h>
#include <linux/sched.h>
#include <linux/mm.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
#include <linux/sched/debug.h>
#include <linux/sched/task_stack.h>
#endif
#include <linux/file.h>
#include <asm/pgtable.h>
#include <linux/of_platform.h>

#include <cam_proc_wrapper.h>
#include <cam_sysfs.h>

#include "common_print.h"
#include <linux/freezer.h>
#include <linux/suspend.h>
#include <cam_fs_wrapper.h>

extern pgd_t *msys_get_pgd_offset_k(unsigned long va);

void MI_SYS_SetFreezable(void)
{
#if defined(CONFIG_ENABLE_MI_NORMAL_STR)
    set_freezable();
#endif
}

MI_S32 MI_SYS_TryToFreeze(void)
{
#if defined(CONFIG_ENABLE_MI_NORMAL_STR)
    return CamTryToFreeze();
#else
    return MI_SUCCESS;
#endif
}

#if defined(CONFIG_ENABLE_MI_NORMAL_STR)
extern MI_S32 MI_SYS_PM_StrSuspendPrepare(void);
extern MI_S32 MI_SYS_PM_StrPostSuspend(void);

static MI_S32 MI_SYS_IMPL_PmNotify(struct notifier_block *b, unsigned long v, void *d)
{
    switch (v)
    {
        case PM_SUSPEND_PREPARE:
            if(MI_SUCCESS != MI_SYS_PM_StrSuspendPrepare())
            {
                DBG_ERR("PM notify suspend prepare failed\n");
                return NOTIFY_BAD;
            }
            break;
        case PM_POST_SUSPEND:
            MI_SYS_PM_StrPostSuspend();
            break;
        default:
            return NOTIFY_DONE;
    }

    return NOTIFY_OK;
}
static struct notifier_block mi_pm_notify_block = {
    .notifier_call = MI_SYS_IMPL_PmNotify,
};
#endif

MI_S32 MI_SYS_IMPL_InitPmNotify(void)
{
#if defined(CONFIG_ENABLE_MI_NORMAL_STR)
    return CamRegisterPmNotifier(&mi_pm_notify_block);
#else
    return MI_SUCCESS;
#endif
}

MI_S32 MI_SYS_IMPL_DeInitPmNotify(void)
{
#if defined(CONFIG_ENABLE_MI_NORMAL_STR)
    return CamUnRegisterPmNotifier(&mi_pm_notify_block);
#else
    return MI_SUCCESS;
#endif
}

MI_U32 MI_SYS_FreeReservedArea(ss_phys_addr_t start, ss_phys_addr_t end, int poison, const char *s)
{
    return free_reserved_area(__va(start), __va(end), poison, (char *)s);
}

MI_S32 MI_SYS_GetProcessName(MI_U8 *szName, MI_U32 u32Length)
{
    MI_S32 s32Ret = MI_ERR_SYS_FAILED;

    if(szName)
    {
        task_lock(current);
        CamOsStrncpy((char*)szName, current->comm,  u32Length - 1);
        task_unlock(current);
        s32Ret = MI_SUCCESS;
    }

    return s32Ret;
}

void * MI_SYS_Buf_GetCurUserMapCtx(void)
{
    return CURRENT_CTX;
}

static void MI_SYS_ShowTask(struct task_struct *p)
{
    int           ppid;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0)
    unsigned long state = p->__state;
#else
    unsigned long state = p->state;
#endif

    if (!try_get_task_stack(p))
        return;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
    printk(KERN_INFO "%-15.15s %c", p->comm, task_state_to_char(p));
#else
    printk(KERN_INFO "%-15.15s %lu", p->comm, state);
#endif

    if (state == TASK_RUNNING)
        CamOsPrintf(KERN_CONT "  running task    ");

    ppid = 0;
    if (pid_alive(p))
    {
#ifndef CONFIG_PROVE_RCU
        ppid = task_pid_nr(rcu_dereference(p->real_parent));
#endif
    }

    CamOsPrintf(KERN_CONT "%5d %6d 0x%08lx\n", task_pid_nr(p), ppid, (unsigned long)task_thread_info(p)->flags);

    // print_worker_info(KERN_INFO, p);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
        show_stack(p, NULL, KERN_DEFAULT);
#else
        show_stack(p, NULL);
#endif
    // put_task_stack(p);
}

void MI_SYS_ShowAllTask(void)
{
    struct task_struct *g, *p;

    printk(KERN_INFO "  task                PC stack   pid father\n");

    CamTaskListRwLock(FALSE);
    for_each_process_thread(g, p)
    {
        MI_SYS_ShowTask(p);
    }
    CamTaskListRwUnlock(FALSE);
}

static MI_BOOL MI_SYS_Is_Va_Invalid(void *va, bool bUserVa)
{
    pgd_t *pgd = NULL;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,61)
    p4d_t *p4d = NULL;
#endif
    pud_t *pud = NULL;
    pmd_t *pmd = NULL;
    pte_t *pte = NULL;
    struct task_struct *tsk = current;

    if (bUserVa)
    {
        if (va == NULL || tsk->mm == NULL)
        {
            goto EXIT;
        }
        pgd = pgd_offset(tsk->mm,(MI_VIRT)va); // get Page Global Directory
    }
    else
    {
        if (va == NULL)
        {
            goto EXIT;
        }
        pgd = msys_get_pgd_offset_k((MI_VIRT)va); // get Page Global Directory
    }

    if (pgd_none(*pgd) || pgd_bad(*pgd))
    {
        DBG_INFO("pgd is NULL!\n");
        goto EXIT;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,61)
    /* The additional page table level -- p4d -- is enable to support 5 level paging */
    p4d = p4d_offset(pgd, (MI_VIRT)va); // get p4d
    if (p4d_none(*p4d) || p4d_bad(*p4d))
    {
        DBG_INFO("p4d is NULL!\n");
        goto EXIT;
    }

    pud = pud_offset(p4d,(MI_VIRT)va);
#else
    pud = pud_offset(pgd,(MI_VIRT)va);  // get Page Upper Directory
#endif
    if (pud_none(*pud) || pud_bad(*pud))
    {
        DBG_INFO("pud is NULL!\n");
        goto EXIT;
    }

    pmd = pmd_offset(pud,(MI_VIRT)va);  // get Page Middle Directory
    if (pmd_none(*pmd) || pmd_bad(*pmd))
    {
        DBG_INFO("pmd is NULL!\n");
        goto EXIT;
    }

    if (PageHighMem(pfn_to_page(pmd_val(*pmd) >> PAGE_SHIFT)))
    {
        DBG_INFO("It's high memory!\n");
    }

    pte = pte_offset_map(pmd, (MI_VIRT)va);  // get Page Table Entry

    if (!pte_present(*pte) || pte_none(*pte))
    {
        DBG_INFO("pte is NULL!\n");
        pte_unmap(pte);
        goto EXIT;
    }
    pte_unmap(pte);

    return FALSE;
EXIT:
    DBG_INFO("mm:%p, active_mm:%p, pid:%d, tgid:%d, flags:%d, name:%s\n",
        tsk->mm, tsk->active_mm, tsk->pid, tsk->tgid, (tsk->flags & PF_EXITING), tsk->comm);
    return TRUE;
}

MI_BOOL MI_SYS_Vma_Is_Va_Invalid(void *va, MI_U32 u32Size)
{
    struct task_struct *tsk = current;
    MI_VIRT pVirAddr = (MI_VIRT)va;
    struct vm_area_struct *vma;
    MI_S32 s32Ret = MI_SUCCESS;

    if (MI_SYS_Is_Va_Invalid(va, TRUE))
    {
        goto EXIT;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,8,0)
    down_write(&tsk->mm->mmap_lock);
#else
    down_write(&tsk->mm->mmap_sem);
#endif
    vma    = find_vma(tsk->mm, pVirAddr);
    if (!vma)
    {
        DBG_WRN("vma is null\n");
        s32Ret = MI_ERR_SYS_FAILED;
    }
    else if (vma->vm_end - pVirAddr < u32Size)
    {
        DBG_WRN("flush addr 0x%lx, size 0x%x, vma start addr 0x%lx, vma end 0x%lx\n",
            pVirAddr, u32Size, vma->vm_start, vma->vm_end);
        s32Ret = MI_ERR_SYS_FAILED;
    }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,8,0)
    up_write(&current->mm->mmap_lock);
#else
    up_write(&current->mm->mmap_sem);
#endif
    if(s32Ret != MI_SUCCESS)
    {
        goto EXIT;
    }

    return FALSE;
EXIT:
    DBG_INFO("mm:%p, active_mm:%p, pid:%d, tgid:%d, flags:%d, name:%s\n",
        tsk->mm, tsk->active_mm, tsk->pid, tsk->tgid, (tsk->flags & PF_EXITING), tsk->comm);
    return TRUE;
}

MI_BOOL MI_SYS_Is_KernVa_Invalid(void *va, MI_U32 u32Size)
{
    /* kernel space va >= PAGE_OFFSET */
    if (va < (void*)PAGE_OFFSET)
    {
        DBG_ERR("va:%px is not in kernel range, u32Size:%u\n", va, u32Size);
        return TRUE;
    }

    return MI_SYS_Is_Va_Invalid(va, FALSE);
}

void *MI_SYS_VMA_DoGetTrueVa(void *vir_addr)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
    return untagged_addr(vir_addr);
#else
    return vir_addr;
#endif
}

// FIXME: MI_SYS_Proc_Print may not export
MI_S32 MI_SYS_PROC_Print(int level, int logv, const char *kmsgfmt, const char *color, const char *fmt,
                         void *fP_printMatch, const char *file, ...);
CamProcEntry_t *MI_SYS_PROC_CommonRealGetSelfDir(CamProcEntry_t *proc_dir_entry);

MI_BOOL MI_SYS_Pass_IsUnmapVmallocAddr(void *pVirtAddr)
{
    MI_VIRT virtAddr = (MI_VIRT)pVirtAddr;

    if ((virtAddr >= VMALLOC_START && virtAddr < VMALLOC_END) && !vmalloc_to_page((void *)pVirtAddr))
    {
        DBG_ERR(
            "In %s, output struct pointer unmap, maybe you meet race condition between dev unregister & buf "
            "release!!!\n",
            __FUNCTION__);
        return TRUE;
    }

    return FALSE;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
#define DOWN_WRITE_MM(mm) (down_write(&mm->mmap_lock))
#define UP_WRITE_MM(mm) (up_write(&mm->mmap_lock))
#else
#define DOWN_WRITE_MM(mm) (down_write(&mm->mmap_sem))
#define UP_WRITE_MM(mm) (up_write(&mm->mmap_sem))
#endif

//declare in mi_sys_impl.h
MI_S32 MI_SYS_IMPL_Va2Pa(void *pVirAddr, MI_PHY *pPhyAddr)
{
    MI_S32                 ret = MI_ERR_SYS_UNEXIST;
    unsigned long          pfn;
    unsigned long          addr;
    struct vm_area_struct *vma;

    *pPhyAddr = 0;

    if (current->mm) // if current->mm == NULL ,user app already exit
    {
        DOWN_WRITE_MM(current->mm);

        addr = (unsigned long)(pVirAddr);
        vma  = find_vma(current->mm, addr);
        if (!vma)
        {
            UP_WRITE_MM(current->mm);
            DBG_ERR("not find vma!!\n");
            return MI_ERR_SYS_ILLEGAL_PARAM;
        }

        if(addr < vma->vm_start || addr > vma->vm_end)
        {
            UP_WRITE_MM(current->mm);
            DBG_WRN("vma mismatch\naddr:%lx, start:%lx, end:%lx.\n", addr, vma->vm_start, vma->vm_end);
            return MI_ERR_SYS_ILLEGAL_PARAM;
        }

        ret = follow_pfn(vma, addr, &pfn);
        UP_WRITE_MM(current->mm);

        if (ret)
        {
            DBG_WRN("follow_pfn return:%d!!\n", ret);
            return MI_ERR_SYS_ILLEGAL_PARAM;
        }

        *pPhyAddr = CamOsMemPhysToMiu(__pfn_to_phys(pfn) + (addr & ~PAGE_MASK));

#ifdef MI_SYS_MIU_MMU
        if (*pPhyAddr < MMU_MMAP_START_ADDR && MI_SYS_MMA_IsEnableMMU())
        {
            /* pa turn to vpa. */
            *pPhyAddr = MI_SYS_MMA_Pa2Vpa(*pPhyAddr);
            DBG_INFO("PhyAddr：%llx\n", *pPhyAddr);
        }
#endif

    }

    return ret;
}

typedef enum
{
    E_MI_COMMON_POLL_NOT_READY         = (0x0),
    E_MI_COMMON_FRAME_READY_FOR_READ   = (0x1 << 0),
    E_MI_COMMON_BUFFER_READY_FOR_WRITE = (0x1 << 1),
} mi_sys_PollFlag_e;
typedef struct mi_sys_PollFile_s
{
    mi_sys_PollFlag_e (*fState)(struct mi_sys_PollFile_s *);
    void (*fRelease)(struct mi_sys_PollFile_s *);
    void *private;
    void *private1;
} mi_sys_PollFile_t;
MI_S32 MI_COMMON_WakeUpClient(mi_sys_PollFile_t *pstPollFile);
int MI_SYS_POLL_ReleasePollFd(mi_sys_PollFile_t **ppstPollFile);
MI_S32 MI_SYS_POLL_GetPoll(mi_sys_PollFile_t **ppstPollFile,
                              mi_sys_PollFlag_e (*fState)(mi_sys_PollFile_t *),
                              void (*fRelease)(mi_sys_PollFile_t *));
void *MI_SYS_PROC_FuncName(void)
{
    return __builtin_return_address(0);
}

void MI_SYS_PROC_BugOn(MI_U32 u32Line)
{
    CamOsPrintf("[%ps][%d] BUGON!\n",__builtin_return_address(0), u32Line);
    CamOsPanic("BUG!");
}

void *MI_SYS_FileGet(MI_S32 s32Fd)
{
    return fget(s32Fd);
}

void *MI_SYS_FilePrivGet(void *filp)
{
    struct file *f = filp;
    return f->private_data;
}

void MI_SYS_FilePut(void *filp)
{
    fput(filp);
}

struct mi_sys_InternalApis_s;
typedef struct mi_sys_InternalApis_s mi_sys_InternalApis_t;

// mi_sys_internal
extern mi_sys_InternalApis_t g_miSysInternalApis;

DECLEAR_MODULE_INIT_EXIT

EXPORT_SYMBOL(g_miSysInternalApis);

#if (defined(CONFIG_MI_DBG) && (CONFIG_MI_DBG == 1))
#if (CONFIG_MI_DBG_LEVEL > L_MI_DBG_NONE)
extern MI_S32 MI_SYS_PROC_Print(int level, int logv, const char *kmsgfmt, const char *color, const char *fmt,
                         void *fP_printMatch, const char *file, ...);
extern void *MI_SYS_PROC_FuncName(void);
EXPORT_SYMBOL(MI_SYS_PROC_Print);
EXPORT_SYMBOL(MI_SYS_PROC_FuncName);
#endif
#endif

EXPORT_SYMBOL(MI_SYS_ShowAllTask);
EXPORT_SYMBOL(MI_SYS_PROC_BugOn);
EXPORT_SYMBOL(MI_SYS_POLL_ReleasePollFd);
EXPORT_SYMBOL(MI_SYS_POLL_GetPoll);
#ifndef COMBINE_MI_MODULE
EXPORT_SYMBOL(MI_COMMON_WakeUpClient);
EXPORT_SYMBOL(MI_SYS_PROC_CommonRealGetSelfDir);
#endif

MI_MODULE_LICENSE();
MODULE_AUTHOR("colin.hu <colin.hu>");
