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
#include <mi_sys.h>
#include <linux/module.h>

MI_S32 MI_SYS_GetChnOutputPortDepth(MI_U16 u16SocId, MI_SYS_ChnPort_t *pstChnPort, MI_U32 *pu32UserFrameDepth,
                                    MI_U32 *pu32BufQueueDepth);
typedef void MI_SYS_RunEnv_e;

extern MI_S32 MI_SYS_GetRunEnvOnce(MI_U16 u16SocId, MI_SYS_RunEnv_e *peRunEnv);

#ifdef KAPI_ENABLE
EXPORT_SYMBOL(MI_SYS_GetRunEnvOnce);
EXPORT_SYMBOL(MI_SYS_Init);
EXPORT_SYMBOL(MI_SYS_Exit);
EXPORT_SYMBOL(MI_SYS_BindChnPort);
EXPORT_SYMBOL(MI_SYS_UnBindChnPort);
EXPORT_SYMBOL(MI_SYS_GetBindbyDest);
EXPORT_SYMBOL(MI_SYS_GetVersion);
EXPORT_SYMBOL(MI_SYS_GetCurPts);
EXPORT_SYMBOL(MI_SYS_InitPtsBase);
EXPORT_SYMBOL(MI_SYS_SyncPts);
EXPORT_SYMBOL(MI_SYS_SetReg);
EXPORT_SYMBOL(MI_SYS_GetReg);
EXPORT_SYMBOL(MI_SYS_Mmap);
EXPORT_SYMBOL(MI_SYS_Munmap);
EXPORT_SYMBOL(MI_SYS_Va2Pa);
EXPORT_SYMBOL(MI_SYS_SetChnMMAConf);
EXPORT_SYMBOL(MI_SYS_GetChnMMAConf);
EXPORT_SYMBOL(MI_SYS_ChnInputPortGetBuf);
EXPORT_SYMBOL(MI_SYS_ChnInputPortPutBuf);
EXPORT_SYMBOL(MI_SYS_ChnOutputPortGetBuf);
EXPORT_SYMBOL(MI_SYS_ChnOutputPortPutBuf);
EXPORT_SYMBOL(MI_SYS_SetChnOutputPortDepth);
EXPORT_SYMBOL(MI_SYS_GetChnOutputPortDepth);
EXPORT_SYMBOL(MI_SYS_ChnInputPortGetBufPa);
EXPORT_SYMBOL(MI_SYS_ChnInputPortPutBufPa);
EXPORT_SYMBOL(MI_SYS_ChnOutputPortGetBufPa);
EXPORT_SYMBOL(MI_SYS_ChnOutputPortPutBufPa);
EXPORT_SYMBOL(MI_SYS_GetFd);
EXPORT_SYMBOL(MI_SYS_CloseFd);
EXPORT_SYMBOL(MI_SYS_ChnPortInjectBuf);
EXPORT_SYMBOL(MI_SYS_MMA_Alloc);
EXPORT_SYMBOL(MI_SYS_MMA_Free);
EXPORT_SYMBOL(MI_SYS_FlushInvCache);
EXPORT_SYMBOL(MI_SYS_ReadUuid);
EXPORT_SYMBOL(MI_SYS_ConfigPrivateMMAPool);
EXPORT_SYMBOL(MI_SYS_MemsetPa);
EXPORT_SYMBOL(MI_SYS_MemcpyPa);
EXPORT_SYMBOL(MI_SYS_MemcpyPaEx);
EXPORT_SYMBOL(MI_SYS_BufFillPa);
EXPORT_SYMBOL(MI_SYS_BufBlitPa);
EXPORT_SYMBOL(MI_SYS_SetChnOutputPortUserFrc);
EXPORT_SYMBOL(MI_SYS_SetChnOutputPortBufExtConf);
EXPORT_SYMBOL(MI_SYS_PrivateDevChnHeapAlloc);
EXPORT_SYMBOL(MI_SYS_PrivateDevChnHeapFree);
EXPORT_SYMBOL(MI_SYS_EnableChnOutputPortLowLatency);
EXPORT_SYMBOL(MI_SYS_DupBuf);
EXPORT_SYMBOL(MI_SYS_ChnInputPortSetUserPicture);
EXPORT_SYMBOL(MI_SYS_EnableUserPicture);
EXPORT_SYMBOL(MI_SYS_DisableUserPicture);
EXPORT_SYMBOL(MI_SYS_SetChnInputPortFrc);
EXPORT_SYMBOL(MI_SYS_QueryDevChnPortState);

#ifdef NON_COMMERCIAL_FOR_INTERNAL_TEST_ONLY
#define MI_MODULE_LICENSE() MODULE_LICENSE("GPL v2");
#else
#define MI_MODULE_LICENSE() MODULE_LICENSE("Proprietary");
#endif

#endif

