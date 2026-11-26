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


#ifndef __SGS_DET_API_H__
#define __SGS_DET_API_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_DET_OBJECT 200
#define MAX_DET_STRLEN 256
#define MAX_DET_CLASSES 10

typedef struct
{
    MI_U32 width;
    MI_U32 height;
    MI_IPU_ELEMENT_FORMAT format;
} DetInputAttr_t;

typedef struct
{
    MI_U32 x;
    MI_U32 y;
    MI_U32 width;
    MI_U32 height;
    MI_U32 class_id;
    MI_FLOAT score;
    MI_U64 pts;
} DetBox_t;

typedef struct
{
    char ipu_firmware_path[MAX_DET_STRLEN]; // ipu_firmware.bin path
    char model_path[MAX_DET_STRLEN];        // detect model path
    MI_BOOL create_device;                  // set false to create ipu device outside algo lib
    MI_BOOL destroy_device;                 // set false to destroy ipu device outside algo lib
    void *model_buffer;                     // set it when load model from memory
    MI_U32 model_buffer_len;                // set it when load model from memory
} DetInit_t;

typedef struct
{
    void *p_vir_addr;
    MI_PHY phy_addr;
    MI_U32 buf_size;
    MI_U64 pts;
    MI_U16 width;
    MI_U16 height;
} DetInput_t;


typedef enum
{
    E_SCORE_DESCENDING = 0,
    E_AREA_DESCENDING,
} DetSortType_e;

typedef struct
{
    MI_S32 class_indexs[MAX_DET_CLASSES];
    MI_S32 class_num;
    DetSortType_e sort_type;
}DetSortInput_t;


typedef struct
{
    MI_FLOAT conf_threshold;         // default=0.5
    MI_S32 disp_width;               // default=1920
    MI_S32 disp_height;              // default=1080
    MI_BOOL ignore_static_objects;   // default=false
    MI_FLOAT static_sensitive;       // default=0.85
    MI_BOOL stable_bbox;             // default=false
    MI_FLOAT stable_sensitive;       // default=0.63
    MI_S32 ignore_frame_number;      // [0-5] ignore the id first number box of detect,default=0
    MI_BOOL strict_mode;             // default=false
} DetParams_t;



MI_S32 ALGO_DET_CreateHandle(void **handle);
MI_S32 ALGO_DET_InitHandle(void *handle, const DetInit_t *init_info);
MI_S32 ALGO_DET_SetParams(void *handle, const DetParams_t *params);
MI_S32 ALGO_DET_GetInputAttr(void *handle, DetInputAttr_t *input_attr);
MI_S32 ALGO_DET_Run(void *handle, const DetInput_t *algo_input, DetBox_t bboxes[MAX_DET_OBJECT], MI_S32 *num_bboxes);
MI_S32 ALGO_DET_SortResult(void *handle, const DetSortInput_t *sort_input, DetBox_t bboxes[MAX_DET_OBJECT], MI_S32 *num_bboxes);
MI_S32 ALGO_DET_DeinitHandle(void *handle);
MI_S32 ALGO_DET_ReleaseHandle(void *handle);
#ifdef __cplusplus
}
#endif

#endif