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


#ifndef _MI_AQSERVER_H
#define _MI_AQSERVER_H

#include "mi_common.h"
#include "mi_aqserver_datatype.h"

#ifdef __cplusplus
extern "C"
{
#endif

MI_S32 MI_AQServer_Open();
MI_S32 MI_AQServer_Close();
MI_S32 MI_AQServer_SetDataPath(const char* path);
MI_S32 MI_AQServer_SetHandle(MI_AQ_HandleAttr_t *pHandleInfo);

#ifdef __cplusplus
}   //end of extern C
#endif

#endif
