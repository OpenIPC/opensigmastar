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

#ifndef __SGS_DT_API_H__
#define __SGS_DT_API_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_DT_STRLEN 256
#ifndef MAX_DT_OBJECT
#define MAX_DT_OBJECT 200
#endif

#ifndef MAX_DT_VERTEX
#define MAX_DT_VERTEX 10
#endif

#ifndef MAX_DT_NUM_CLASS
#define MAX_DT_NUM_CLASS 16
#endif

#ifndef MAX_DT_ROI
#define MAX_DT_ROI 12
#endif

/*******************base*******************/
#ifndef TYPEDEF_DTBOX_T
#define TYPEDEF_DTBOX_T
typedef struct
{
    MI_U32 x;
    MI_U32 y;
    MI_U32 width;
    MI_U32 height;
    MI_U32 class_id;
    MI_FLOAT score;
    MI_U64 pts;
    MI_U64 track_id;
    MI_BOOL moving;
} DtBox_t;
#endif


/*******************count*******************/
typedef struct
{
    MI_U32 x;
    MI_U32 y;
} DtPoint_t;


typedef struct
{
    DtPoint_t point[MAX_DT_VERTEX];
    MI_U32 point_num;
} DtRoi_t;


typedef struct
{
    DtRoi_t roi;
    MI_U32 count;
    MI_FLOAT interval;
    MI_U32 interval_in_count;
    MI_U32 interval_out_count;
    MI_U32 count_cls[MAX_DT_NUM_CLASS];
    MI_S16 num_count_cls;
} CountRoi_t;


/*******************roi alarm*******************/
typedef enum
{
    E_NORMAL_ROI_ALARM = 0, //  正常区域警戒
    E_ILLEGAL_PARKING,      // 非法停车
} RoiAlarmMode_e;

typedef struct
{
    DtRoi_t roi;
    RoiAlarmMode_e roi_model;
    MI_BOOL inbreak;          //入侵
    DtBox_t inbreak_box[MAX_DT_OBJECT];
    MI_U32  inbreak_box_num;
    MI_BOOL target_enter;
    DtBox_t target_enter_box[MAX_DT_OBJECT];
    MI_U32  target_enter_box_num;
    MI_BOOL target_leave;
    DtBox_t target_leave_box[MAX_DT_OBJECT];
    MI_U32  target_leave_box_num;
}ALGO_DtRoiAlarm_t;


typedef struct
{
    DtRoi_t roi;
    MI_BOOL wander;
    DtBox_t target_wander_box[MAX_DT_OBJECT];
    MI_U32  target_wander_num;
    MI_FLOAT interval;
}ALGO_DtRoiWander_t;


typedef struct
{
    DtRoi_t roi;
    MI_BOOL fast_move;
    DtBox_t fast_move_box[MAX_DT_OBJECT];
    MI_U32  fast_move_box_num;
    MI_FLOAT sensitive;
}ALGO_DtRoiFastMove_t;


/*******************line alarm*******************/
typedef struct
{
    DtPoint_t point1;
    DtPoint_t point2;
}DtLine_t;


typedef struct
{
    DtLine_t line;
    MI_BOOL inbreak;     //入侵
    DtBox_t inbreak_box[MAX_DT_OBJECT];
    MI_U32  inbreak_box_num;
    MI_BOOL cross_line;
    DtBox_t cross_line_box[MAX_DT_OBJECT];
    MI_S32  cross_line_orien[MAX_DT_OBJECT];
    MI_S32  cross_line_num;
    MI_FLOAT sensitive; //between(-1.0 - 1.0)
}ALGO_DtLineAlarm_t;


typedef struct
{
    MI_U32 width;
    MI_U32 height;
    MI_IPU_ELEMENT_FORMAT format;
} DtInputAttr_t;


typedef struct
{
    char ipu_firmware_path[MAX_DT_STRLEN];  // ipu_firmware.bin path
    char model_path[MAX_DT_STRLEN];         // detect model path, do not set when use outside detect model
    MI_BOOL create_device;                  // set false to create ipu device outside algo lib
    MI_BOOL destroy_device;                 // set false to destroy ipu device outside algo lib
    void *model_buffer;                     // set it when load model from memory, do not set when use outside detect model
    MI_U32 model_buffer_len;                // set it when load model from memory, do not set when use outside detect model
} DtInit_t;


typedef struct
{
    void *p_vir_addr;
    MI_PHY phy_addr;
    MI_U32 buf_size;
    MI_U64 pts;
} DtInput_t;


typedef struct
{
    MI_S32 disp_width;               // default=1920
    MI_S32 disp_height;              // default=1080

    // detect
    MI_FLOAT conf_threshold;         // default=0.5

    // track
    MI_BOOL ignore_static_objects;   // default=false
    MI_BOOL stable_bbox;             // default=false
    MI_FLOAT stable_iou_threshold;   // default=0.65
    MI_FLOAT static_iou_threshold;   // default=0.85
    MI_S32 ignore_frame_number;      // default=0
    MI_FLOAT ignore_static_time;     // default=0
    MI_BOOL use_detect_bbox;         // default=false
    MI_BOOL strict_mode;             // default=false
    MI_S32 dynamic_to_static_count_threshold; // default=3
} DtParams_t;



// base
MI_S32 ALGO_DT_CreateHandle(void **handle);
MI_S32 ALGO_DT_InitHandle(void *handle, const DtInit_t *init_info);
MI_S32 ALGO_DT_GetInputAttr(void *handle, DtInputAttr_t *input_attr);
MI_S32 ALGO_DT_DeinitHandle(void *handle);
MI_S32 ALGO_DT_ReleaseHandle(void *handle);
MI_S32 ALGO_DT_SetParams(void *handle, const DtParams_t *params);

// detect
MI_S32 ALGO_DT_Detect(void *handle, const DtInput_t *algo_input, DtBox_t bboxes[MAX_DT_OBJECT], MI_S32 *num_bboxes);

// track
MI_S32 ALGO_DT_Track(void *handle, DtBox_t bboxes[MAX_DT_OBJECT], MI_S32 *num_bboxes);

// count
MI_S32 ALGO_DT_Count(void *handle, CountRoi_t rois[MAX_DT_ROI], MI_S32 num_rois, DtBox_t bboxes_input[MAX_DT_OBJECT], MI_S32 num_bboxes_input);

// alarm
MI_S32 ALGO_DT_RoiAlarmDetect(void* handle, ALGO_DtRoiAlarm_t rois[MAX_DT_ROI], MI_S32 num_rois, DtBox_t bboxes_input[MAX_DT_OBJECT], MI_S32 num_bboxes_input);
MI_S32 ALGO_DT_RoiWanderDetect(void* handle, ALGO_DtRoiWander_t rois[MAX_DT_ROI], MI_S32 num_rois, DtBox_t bboxes_input[MAX_DT_OBJECT], MI_S32 num_bboxes_input);
MI_S32 ALGO_DT_RoiFastMovingDetect(void* handle, ALGO_DtRoiFastMove_t rois[MAX_DT_ROI], MI_S32 num_rois, DtBox_t bboxes_input[MAX_DT_OBJECT], MI_S32 num_bboxes_input);
MI_S32 ALGO_DT_LineAlarmDetect(void* handle, ALGO_DtLineAlarm_t lines[MAX_DT_ROI], MI_S32 num_lines, DtBox_t bboxes_input[MAX_DT_OBJECT], MI_S32 num_bboxes_input);

#ifdef __cplusplus
}
#endif

#endif