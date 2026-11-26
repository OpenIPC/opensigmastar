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

//-------------------------------------------------------------------------------------------------
//  Header Files
//-------------------------------------------------------------------------------------------------

#ifndef _MI_VGE_H_
#define _MI_VGE_H_

#include "mi_common.h"
#include "mi_vge_datatype.h"

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Macros
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Enum
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Variable
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  local function  prototypes
//-------------------------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#endif

    MI_S32 MI_VGE_CreateDev(MI_VGE_DEV devId, MI_VGE_DeviceAttr_t *pstDevAtr);
    MI_S32 MI_VGE_DestroyDev(MI_VGE_DEV devId);
    MI_S32 MI_VGE_StartJob(MI_VGE_HANDLE *hHandle);
    MI_S32 MI_VGE_EndJob(MI_VGE_HANDLE hHandle);
    MI_S32 MI_VGE_CancelJob(MI_VGE_HANDLE hHandle);
    MI_S32 MI_VGE_AddLineTask(MI_VGE_HANDLE hHandle, const MI_VGE_Task_t *pstTask, const MI_VGE_Line_t pstLine[],
                              MI_U32 u32LineCnt);
    MI_S32 MI_VGE_AddCoverTask(MI_VGE_HANDLE hHandle, const MI_VGE_Task_t *pstTask, const MI_VGE_Cover_t pstCover[],
                               MI_U32 u32CoverCnt);
    MI_S32 MI_VGE_AddOsdTask(MI_VGE_HANDLE hHandle, const MI_VGE_Task_t *pstTask, const MI_VGE_Osd_t pstOsd[],
                             MI_U32 u32OsdCnt);
    MI_S32 MI_VGE_AddMosaicTask(MI_VGE_HANDLE hHandle, const MI_VGE_Task_t *pstTask, const MI_VGE_Mosaic_t pstMosaic[],
                                MI_U32 u32MosaicCnt);
    MI_S32 MI_VGE_AddFaceFrameTask(MI_VGE_HANDLE hHandle, const MI_VGE_Task_t *pstTask,
                                   const MI_VGE_FaceFrame_t pstFaceFrame[], MI_U32 u32FaceFrameCnt);

#ifdef __cplusplus
}
#endif

#endif
