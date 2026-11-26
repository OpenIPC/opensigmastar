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
#ifndef __MI_AQ_DATATYPE_H__
#define __MI_AQ_DATATYPE_H__

#include "mi_common_datatype.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
// Data type definition
//=============================================================================
typedef enum
{
    E_MI_AQ_HANDLE_ANR = 0,
    E_MI_AQ_HANDLE_AGC,
    E_MI_AQ_HANDLE_EQ,
    E_MI_AQ_HANDLE_APC,
    E_MI_AQ_HANDLE_AEC,
    E_MI_AQ_HANDLE_DH,
    E_MI_AQ_HANDLE_BF,
} MI_AQ_HANDLE_TYPE_e;

typedef struct MI_AQ_HandleAttr_s
{
    void *handle;
    MI_U32 id;
    MI_AQ_HANDLE_TYPE_e type;
} MI_AQ_HandleAttr_t;

#ifdef __cplusplus
}
#endif

#endif //__MI_AI_DATATYPE_H__
