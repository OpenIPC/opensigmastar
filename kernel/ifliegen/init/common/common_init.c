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
#include <asm/neon.h>
#include <linux/module.h>

#include "mi_common_datatype.h"
#include "mi_common_internal.h"

#include <linux/moduleparam.h>
#include <linux/stat.h>
#include <linux/sched.h>
#include <linux/of_platform.h>
#include <cam_proc_wrapper.h>
#include "cam_sysfs.h"
#include "cam_clkgen.h"
#include "cam_device_wrapper.h"

// TODO: to avoid include cjson.h
#include "mi_common_cjson.h"
#ifdef CONFIG_MI_COMMON_RGNSUB
#include "mi_common_rgnsub.h"
#endif
#ifdef CONFIG_ARM64
#include <asm/debug-monitors.h>
#endif

typedef enum
{
    E_MI_COMMON_RESOURCE_LEVEL_0,
    E_MI_COMMON_RESOURCE_LEVEL_1,
    E_MI_COMMON_RESOURCE_LEVEL_2,
    E_MI_COMMON_RESOURCE_LEVEL_3,
    E_MI_COMMON_RESOURCE_LEVEL_4,
    E_MI_COMMON_RESOURCE_LEVEL_5,
    E_MI_COMMON_RESOURCE_LEVEL_MAX,
} MI_COMMON_ProcessLevel_e;

typedef enum
{
    E_MI_COMMON_RESOURCE_KEY_INIT,
    E_MI_COMMON_RESOURCE_KEY_DEV,
    E_MI_COMMON_RESOURCE_KEY_CHN,
    E_MI_COMMON_RESOURCE_KEY_PORT,
    E_MI_COMMON_RESOURCE_KEY_HANDLE,
    E_MI_COMMON_RESOURCE_KEY_ID,
    E_MI_COMMON_RESOURCE_KEY_GROUP,
    E_MI_COMMON_RESOURCE_KEY_LAYER,
    E_MI_COMMON_RESOURCE_KEY_ATTACH,
    E_MI_COMMON_RESOURCE_KEY_START,
    E_MI_COMMON_RESOURCE_KEY_BIND,
    E_MI_COMMON_RESOURCE_KEY_INPUT_GETBUF,
    E_MI_COMMON_RESOURCE_KEY_INPUT_USERPIC,
    E_MI_COMMON_RESOURCE_KEY_OUTPUT_GETBUF,
    E_MI_COMMON_RESOURCE_KEY_MMA_ALLOCBUF,
    E_MI_COMMON_RESOURCE_KEY_PRIVPOOL_CONFIGBUF,
    E_MI_COMMON_RESOURCE_KEY_PRIVPOOL_ALLOCBUF,
    E_MI_COMMON_RESOURCE_KEY_MAX,
} MI_COMMON_ProcessKeyType_e;

typedef struct MI_COMMON_PROCESS_ResourceInfo_s
{
    MI_ModuleId_e               eModuleId;
    MI_COMMON_ProcessLevel_e    eProcessLevel;
    MI_U64                      u64Key;
    void                        *pPrivateData;
    MI_U32                      u32DataSize;
    MI_S32                      s32Pid;
} MI_COMMON_PROCESS_ResourceInfo_t;

#if !defined(TRUE) && !defined(FALSE)
/// definition for TRUE
#define TRUE 1
/// definition for FALSE
#define FALSE 0
#endif

struct MI_DEVICE_Object_s;
typedef struct MI_DEVICE_Object_s MI_DEVICE_Object_t;
typedef MI_S32 (*MI_COMMON_PROCESS_EARLY_FREE_CB)(MI_ModuleId_e eModId, MI_COMMON_ProcessKeyType_e eKeyType,
                                                  void *pPrivData);

extern char *gp_jsonPath;
extern char  g_modParamPath[128];
extern char  g_configJsonPath[128];

module_param_string(g_modParamPath, g_modParamPath, sizeof(g_modParamPath), 0644);
module_param(gp_jsonPath, charp, S_IRUGO);
module_param_string(g_configJsonPath, g_configJsonPath, sizeof(g_configJsonPath), 0644);

void MI_DEVICE_VersionShow(CamProcSeqFile_t *m, void *pData);
void MI_DEVICE_DebugShow(CamProcSeqFile_t *m, void *pData);
void MI_DEVICE_DebugWrite(char *pBuf, int nLen, void *pData);
void MI_DEVICE_NumberShow(CamProcSeqFile_t *m, void *pData);
void MI_DEVICE_NumberWrite(char *pBuf, int nLen, void *pData);
CamProcEntry_t *MI_DEVICE_GetProcModuleDir(void);
CamProcEntry_t *MI_DEVICE_GetProcHalDir_Tag(int module_id);
void MI_DEVICE_SetProcHalDir_Tag(int module_id, CamProcEntry_t * pEntry);

struct CamClass *MI_DEVICE_GetClass(void);
MI_S32 MI_DEVICE_RemoteWrapper(unsigned short socid, MI_ModuleId_e module_id, int cmd, void *arg, int len);
int MI_DEVICE_Register(MI_DEVICE_Object_t *device);
void MI_DEVICE_Unregister(MI_DEVICE_Object_t *device);
int core_module_init(void);
void core_module_exit(void);
void MI_DEVICE_NotifyMiSysExit(void);
void MI_COMMON_ModuleIdToPrefixName(MI_ModuleId_e eModuleId, char *prefix_name);
cJSON *      MI_COMMON_MODPARAM_GetCjsonObjectItem(const char *const modid, const char *const val_name);
MI_U32 MI_COMMON_PROCESS_GetResKeyVal(MI_U8* u8String);
MI_S32 MI_COMMON_PROCESS_FindResourcePidByKey(MI_ModuleId_e eModId, MI_U64 u64Key);
MI_S32 MI_COMMON_PROCESS_CheckResourceExist(MI_ModuleId_e eModId, MI_U64 u64Key, MI_S32 s32Pid);
MI_S32 MI_COMMON_PROCESS_AddResource(MI_COMMON_PROCESS_ResourceInfo_t *pstResourceInfo);
MI_S32 MI_COMMON_PROCESS_DelResource(MI_COMMON_PROCESS_ResourceInfo_t *pstResourceInfo);
MI_S32 MI_COMMON_PROCESS_ExitFull(MI_ModuleId_e eModId, MI_S32 s32Pid,
                                  MI_S32 (*mi_common_FreeResourceEarly)(MI_COMMON_PROCESS_ResourceInfo_t *),
                                  MI_S32 (*mi_common_FreeResource)(MI_COMMON_PROCESS_ResourceInfo_t *));
MI_S32 MI_COMMON_PROCESS_Exit(MI_ModuleId_e eModId, MI_S32 s32Pid,
                                  MI_S32 (*mi_common_FreeResource)(MI_COMMON_PROCESS_ResourceInfo_t *));                       
void MI_COMMON_PROCESS_RegisterEarlyFreeCb(MI_COMMON_PROCESS_EARLY_FREE_CB pfEarlyFreeCb);
void MI_COMMON_MODPARAM_GETVALUE(const char *pModuleId, const char *pname, MI_COMMON_ModParamType_e eType, void *pval);
void MI_COMMON_MODPARAM_GETSTRING(const char *pModuleId, const char *pname, MI_U16 u16Size, void *pval);
void MI_COMMON_MODPARAM_GETARRAY(const char *pModuleId, const char *pname, MI_COMMON_ModParamType_e eType,
                                         MI_U16 u16Size, void *pval, MI_U32 *p32RealSize);

// common cjson interface
char *MI_COMMON_CJSON_GetConfigJsonPath(void);
cJSON *MI_COMMON_CJSON_GetCustomerCjson(const char *path);
void MI_COMMON_CJSON_FreeCustomerCjson(const char *path);
//MI_COMMON_CJSON_GetObjectItem

#ifdef CONFIG_MI_COMMON_CJSON_WRITE
CJSON_PUBLIC(cJSON *) MI_COMMON_CJSON_CreateObject(void);
CJSON_PUBLIC(cJSON *) MI_COMMON_CJSON_CreateString(const char *string);
CJSON_PUBLIC(cJSON *) MI_COMMON_CJSON_CreateArray(void);
CJSON_PUBLIC(cJSON *) MI_COMMON_CJSON_CreateIntArray(const int *numbers, int count);
CJSON_PUBLIC(cJSON *) MI_COMMON_CJSON_CreateStringArray(const char **strings, int count);
CJSON_PUBLIC(cJSON*) MI_COMMON_CJSON_AddBoolToObject(cJSON * const object, const char * const name, const cJSON_bool boolean);
CJSON_PUBLIC(cJSON*) MI_COMMON_CJSON_AddNumberToObject(cJSON * const object, const char * const name, const int number);
CJSON_PUBLIC(cJSON*) MI_COMMON_CJSON_AddStringToObject(cJSON * const object, const char * const name, const char * const string);
CJSON_PUBLIC(void) MI_COMMON_CJSON_AddItemToArray(cJSON *array, cJSON *item);
CJSON_PUBLIC(void) MI_COMMON_CJSON_AddItemToObject(cJSON *object, const char *string, cJSON *item);
CJSON_PUBLIC(cJSON_bool) MI_COMMON_CJSON_PrintPreallocated(cJSON *item, char *buf, const int len, const cJSON_bool fmt);
#endif

MI_BOOL MI_DEVICE_IsCurrentExiting(void)
{
    return current->flags & PF_EXITING;
}

int MI_DEVICE_IsMmExist(void)
{
    return current->mm != NULL ? TRUE : FALSE;
}

int MI_DEVICE_GetPid(void)
{
    return current->pid;
}

int MI_DEVICE_GetTgid(void)
{
    return current->tgid;
}

#ifdef CONFIG_HAVE_HW_BREAKPOINT
#ifdef CONFIG_ARM64
static void _MI_COMMON_HwBreakPointHandler(struct perf_event *pstPerfEvt, struct perf_sample_data *pstPerfData, struct pt_regs *pstRegs)
{
    //note!must not call printk() here,it can cause irq trigger contiously.
    hw_breakpoint_set_single_step(pstPerfEvt, 2);
}
static int _MI_COMMON_SingleStepHandler(struct pt_regs *regs, unsigned int esr)
{
    dump_stack();
    return 0;
}
static struct step_hook g_DbgStepHook = {
    .flags = DBG_SS_BEFORE_BP,
    .fn    = _MI_COMMON_SingleStepHandler,
};
#else
static void _MI_COMMON_HwBreakPointHandler(struct perf_event *pstPerfEvt, struct perf_sample_data *pstPerfData, struct pt_regs *pstRegs)
{
    dump_stack();
}
#endif

void *MI_COMMON_EnableBreakpoint(void *pBpAddr, MI_U64 u64BpLen, MI_COMMON_HwBreakpointType_e eBpType)
{
    struct perf_event_attr       stEvtAttr;
    struct perf_event *__percpu *pstEvt = NULL;

    hw_breakpoint_init(&stEvtAttr);

    stEvtAttr.bp_addr = (MI_VIRT)pBpAddr;

    if (u64BpLen < HW_BREAKPOINT_LEN_1 || u64BpLen > HW_BREAKPOINT_LEN_8)
    {
        CamOsPrintf("error hw_breakpoint len %llu, must between [%u, %u]\n", u64BpLen, HW_BREAKPOINT_LEN_1, HW_BREAKPOINT_LEN_8);
        return NULL;
    }
    stEvtAttr.bp_len  = u64BpLen;

    switch(eBpType)
    {
        case E_MI_HW_BREAKPOINT_EMPTY:
            stEvtAttr.bp_type = HW_BREAKPOINT_EMPTY;
            break;
        case E_MI_HW_BREAKPOINT_R:
            stEvtAttr.bp_type = HW_BREAKPOINT_R;
            break;
        case E_MI_HW_BREAKPOINT_W:
            stEvtAttr.bp_type = HW_BREAKPOINT_W;
            break;
        case E_MI_HW_BREAKPOINT_RW:
            stEvtAttr.bp_type = HW_BREAKPOINT_RW;
            break;
        case E_MI_HW_BREAKPOINT_X:
            stEvtAttr.bp_type = HW_BREAKPOINT_X;
            break;
        case E_MI_HW_BREAKPOINT_INVALID:
            stEvtAttr.bp_type = HW_BREAKPOINT_INVALID;
            break;
        default:
            CamOsPrintf("error hw_breakpoint type %x\n", eBpType);
            return NULL;
    }
#ifdef CONFIG_ARM64
    CamRegisterKernelStepHook(&g_DbgStepHook);
#endif
    pstEvt = CamRegisterWideHwBreakpoint(&stEvtAttr, _MI_COMMON_HwBreakPointHandler, NULL);
    if (IS_ERR((void __force *)pstEvt))
    {
        CamOsPrintf("error:Breakpoint registration failed\n");
        pstEvt = NULL;
    }

    return (void *)pstEvt;
}

void MI_COMMON_DisableBreakpoint(void *vBpHandle)
{
    struct perf_event *__percpu *pstEvt = vBpHandle;
    if (IS_ERR((void __force *)pstEvt))
    {
        CamOsPrintf("error:sample_hbp is NULL\n");
        return ;
    }
    CamUnRegisterWideHwBreakpoint(pstEvt);
#ifdef CONFIG_ARM64
    CamUnRegisterKernelStepHook(&g_DbgStepHook);
#endif
}
#endif /* CONFIG_HAVE_HW_BREAKPOINT */

extern void recode_timestamp(int mark, const char *name);

void MI_COMMON_RecodeTimeStamp(int mark, const char *name)
{
#ifdef CONFIG_SGS_PROFILING_TIME
    recode_timestamp(mark, name);
#else
    UNUSED(mark);
    UNUSED(name);
#endif
}

DECLEAR_MODULE_INIT_EXIT_EXTRA(core_module_init, core_module_exit);

EXPORT_SYMBOL(MI_DEVICE_DebugShow);
EXPORT_SYMBOL(MI_DEVICE_VersionShow);
EXPORT_SYMBOL(MI_DEVICE_DebugWrite);
EXPORT_SYMBOL(MI_DEVICE_NumberShow);
EXPORT_SYMBOL(MI_DEVICE_NumberWrite);
EXPORT_SYMBOL(MI_DEVICE_GetProcModuleDir);
EXPORT_SYMBOL(MI_DEVICE_GetProcHalDir_Tag);
EXPORT_SYMBOL(MI_DEVICE_SetProcHalDir_Tag);
EXPORT_SYMBOL(MI_COMMON_ModuleIdToPrefixName);
EXPORT_SYMBOL(MI_DEVICE_Register);
EXPORT_SYMBOL(MI_DEVICE_Unregister);
EXPORT_SYMBOL(MI_COMMON_PROCESS_AddResource);
EXPORT_SYMBOL(MI_COMMON_PROCESS_DelResource);
EXPORT_SYMBOL(MI_COMMON_PROCESS_ExitFull);
EXPORT_SYMBOL(MI_COMMON_PROCESS_Exit);

#ifndef COMBINE_MI_MODULE
#ifdef CONFIG_HAVE_HW_BREAKPOINT
EXPORT_SYMBOL(MI_COMMON_EnableBreakpoint);
EXPORT_SYMBOL(MI_COMMON_DisableBreakpoint);
#endif
EXPORT_SYMBOL(MI_COMMON_RecodeTimeStamp);
EXPORT_SYMBOL(MI_DEVICE_NotifyMiSysExit);
EXPORT_SYMBOL(MI_DEVICE_IsMmExist);
EXPORT_SYMBOL(MI_DEVICE_GetTgid);
EXPORT_SYMBOL(MI_DEVICE_GetClass);
EXPORT_SYMBOL(MI_DEVICE_RemoteWrapper);
EXPORT_SYMBOL(MI_COMMON_MODPARAM_GetCjsonObjectItem);
EXPORT_SYMBOL(MI_COMMON_MODPARAM_GETVALUE);
EXPORT_SYMBOL(MI_COMMON_MODPARAM_GETSTRING);
EXPORT_SYMBOL(MI_COMMON_MODPARAM_GETARRAY);
EXPORT_SYMBOL(MI_COMMON_PROCESS_GetResKeyVal);
EXPORT_SYMBOL(MI_COMMON_PROCESS_FindResourcePidByKey);
EXPORT_SYMBOL(MI_COMMON_PROCESS_CheckResourceExist);
EXPORT_SYMBOL(MI_COMMON_PROCESS_RegisterEarlyFreeCb);

// common cjson
EXPORT_SYMBOL(MI_COMMON_CJSON_GetConfigJsonPath);
EXPORT_SYMBOL(MI_COMMON_CJSON_GetCustomerCjson);
EXPORT_SYMBOL(MI_COMMON_CJSON_FreeCustomerCjson);
EXPORT_SYMBOL(MI_COMMON_CJSON_Load);
EXPORT_SYMBOL(MI_COMMON_CJSON_Parse);
EXPORT_SYMBOL(MI_COMMON_CJSON_HasObjectItem);
EXPORT_SYMBOL(MI_COMMON_CJSON_GetObjectItem);
EXPORT_SYMBOL(MI_COMMON_CJSON_GetArraySize);
EXPORT_SYMBOL(MI_COMMON_CJSON_GetArrayItem);
EXPORT_SYMBOL(MI_COMMON_CJSON_Delete);
EXPORT_SYMBOL(MI_COMMON_CJSON_Print);
#ifdef CONFIG_MI_COMMON_CJSON_WRITE
EXPORT_SYMBOL(MI_COMMON_CJSON_CreateObject);
EXPORT_SYMBOL(MI_COMMON_CJSON_CreateString);
EXPORT_SYMBOL(MI_COMMON_CJSON_CreateArray);
EXPORT_SYMBOL(MI_COMMON_CJSON_CreateIntArray);
EXPORT_SYMBOL(MI_COMMON_CJSON_CreateStringArray);
EXPORT_SYMBOL(MI_COMMON_CJSON_AddBoolToObject);
EXPORT_SYMBOL(MI_COMMON_CJSON_AddNumberToObject);
EXPORT_SYMBOL(MI_COMMON_CJSON_AddStringToObject);
EXPORT_SYMBOL(MI_COMMON_CJSON_AddItemToArray);
EXPORT_SYMBOL(MI_COMMON_CJSON_AddItemToObject);
EXPORT_SYMBOL(MI_COMMON_CJSON_PrintPreallocated);
#endif
// rgn sub

#ifdef CONFIG_ENABLE_SAFE_ARITHMETIC
EXPORT_SYMBOL(MI_COMMON_WarnOn);
#endif

#ifdef CONFIG_MI_COMMON_RGNSUB
EXPORT_SYMBOL(MI_COMMON_RGNSUB_IsEmptyClipRgn);
EXPORT_SYMBOL(MI_COMMON_RGNSUB_EmptyClipRgn);
EXPORT_SYMBOL(MI_COMMON_RGNSUB_InitClipRgn);
EXPORT_SYMBOL(MI_COMMON_RGNSUB_SubtractRgn);
EXPORT_SYMBOL(MI_COMMON_RGNSUB_AddClipRect);
EXPORT_SYMBOL(MI_COMMON_RGNSUB_DbgDumpRgn);
EXPORT_SYMBOL(MI_COMMON_RGNSUB_InitRgn);
EXPORT_SYMBOL(MI_COMMON_RGNSUB_DeinitRgn);
#endif
#endif
MI_MODULE_LICENSE();


