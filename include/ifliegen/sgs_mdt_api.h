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


#ifndef __SGS_MDT_API_H__
#define __SGS_MDT_API_H__

#ifdef __cplusplus
extern "C"
{
#endif


#define MAX_NUM_MDT_OBJS (1)


typedef enum
{
    E_MDT_STAGE_INIT = 0,
    E_MDT_STAGE_DETECT,
    E_MDT_STAGE_TRACK,
} MdtStage_e;


typedef struct
{
    MI_U32 x;
    MI_U32 y;
    MI_U32 width;
    MI_U32 height;
    MI_FLOAT score;
    MI_U64 track_id;
    MI_U64 pts;
} MdtBox_t;


typedef struct
{
    MI_S32 ive_handle;
    MI_U32 in_width;
    MI_U32 in_height;
    MI_U32 in_stride;
} MdtInit_t;


typedef struct
{
    void *p_vir_addr;
    MI_PHY phy_addr;
    MI_U32 buf_size;
    MI_U64 pts;
} MdtInput_t;


typedef struct
{
    MI_FLOAT track_threshold;   // 0.0 ~ 1.0 default 0.3
    MI_U8 detect_threshold;     // 0 ~ 255, default 8
    MI_FLOAT min_width;         // 0.0 ~ 1.0 default 0.06
    MI_FLOAT min_height;        // 0.0 ~ 1.0 default 0.1
    MI_FLOAT max_width;         // 0.0 ~ 1.0 default 0.3
    MI_FLOAT max_height;        // 0.0 ~ 1.0 default 1.0
    MI_FLOAT max_frame_gap;     // 0.0 ms ~ INF ms default 100ms
} MdtParams_t;



MI_S32 ALGO_MDT_CreateHandle(void **handle);
MI_S32 ALGO_MDT_InitHandle(void *handle, const MdtInit_t *init_info);
MI_S32 ALGO_MDT_SetParams(void *handle, const MdtParams_t *params);
MI_S32 ALGO_MDT_DeinitHandle(void *handle);
MI_S32 ALGO_MDT_ReleaseHandle(void *handle);

MI_S32 ALGO_MDT_Run(void *handle, const MdtInput_t* prev_frame, const MdtInput_t* current_frame, MdtBox_t boxes[MAX_NUM_MDT_OBJS], MI_S32 *num_boxes);
MI_S32 ALGO_MDT_Track(void *handle, const MdtInput_t* current_frame, MdtBox_t boxes[MAX_NUM_MDT_OBJS], MI_S32 *num_boxes, MdtStage_e *mdt_stage);
MI_S32 ALGO_MDT_GetMdMask(void *handle, const MdtInput_t *prev_frame, const MdtInput_t *current_frame, MI_U8* mask, MI_S32 mask_w, MI_S32 mask_h);


#ifdef __cplusplus
}
#endif

#endif