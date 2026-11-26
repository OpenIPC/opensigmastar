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


#ifndef __SGS_LPR_API_H__
#define __SGS_LPR_API_H__

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef MAX_LPR_OBJECT
#define MAX_LPR_OBJECT 10
#endif

#ifndef MAX_LPR_STRLEN
#define MAX_LPR_STRLEN 256
#endif

#ifndef MAX_LP_RESULT_LEN
#define MAX_LP_RESULT_LEN 20
#endif

#ifndef NUM_LPR_KEYPOINTS
#define NUM_LPR_KEYPOINTS 4
#endif



typedef struct
{
    char ipu_firmware_path[MAX_LPR_STRLEN];     // ipu_firmware.bin path
    char det_model[MAX_LPR_STRLEN];             // det model path
    char rec_model[MAX_LPR_STRLEN];             // rec model path
    char cls_model[MAX_LPR_STRLEN];             // cls model path

    MI_BOOL create_device;                      // set false to handle ipu device outside algo lib
    MI_BOOL destroy_device;                     // set false to handle ipu device outside algo lib
} LprInitInfo_t;


typedef struct
{
    MI_U32 width;
    MI_U32 height;
    MI_IPU_ELEMENT_FORMAT format;
}LprInputAttr_t;


typedef struct
{
    MI_S32 disp_width;
    MI_S32 disp_height;

    MI_FLOAT det_threshold;                     // det confidence
    MI_FLOAT rec_threshold;                     // rec confidence
    MI_FLOAT cls_threshold;                     // cls confidence

    MI_FLOAT quality_threshold;                 // quality threshold
    MI_FLOAT horizon_angle_threshold;
    MI_FLOAT vertical_angle_threshold;
    MI_FLOAT plate_area_threshold;
    MI_FLOAT keypoints_distance_threshold;
}LprParams_t;


typedef struct
{
    void *p_vir_addr;
    MI_PHY phy_addr;
    MI_U32 buf_size;
    MI_U64 pts;
    MI_U16 width;
    MI_U16 height;
    MI_BOOL filter;
} LprInput_t;


typedef struct
{
    MI_U32 x;
    MI_U32 y;
    MI_U32 width;
    MI_U32 height;
    MI_S32 cls;
    MI_FLOAT det_score;
    MI_U64 pts;
    MI_FLOAT keypts[NUM_LPR_KEYPOINTS][2];
    MI_U64 track_id;
} LprBox_t;


typedef struct
{
    MI_FLOAT quality_score;
    MI_FLOAT horizon_angle;
    MI_FLOAT vertical_angle;
    MI_FLOAT plate_area;
    MI_FLOAT keypoints_distance;
} LprQuality_t;


typedef enum
{
    LP_COLOR_BLUE=0,
    LP_COLOR_GREEN,
    LP_COLOR_YELLOW,
    LP_COLOR_WHITE,
    LP_COLOR_BLACK
}LpColor_e;


typedef enum
{
    LP_TYPE_NORM=0,
    LP_TYPE_POLICE,
    LP_TYPE_ARMY,
    LP_TYPE_HONGKONG,
    LP_TYPE_MACAO,
    LP_TYPE_EMBASSY
}LpType_e;

typedef struct
{
    MI_FLOAT rec_score;
    char lp_number[MAX_LP_RESULT_LEN];

    MI_FLOAT cls_score;
    LpColor_e lp_color;
    LpType_e lp_type;
} LprInfo_t;


typedef struct
{
    LprBox_t box;
    LprQuality_t quality;
    LprInfo_t info;
}LprResult_t;


MI_S32 ALGO_LPR_CreateHandle(void **handle);
MI_S32 ALGO_LPR_InitHandle(void *handle, LprInitInfo_t *lpr_init_info);
MI_S32 ALGO_LPR_GetInputAttr(void *handle, LprInputAttr_t *input_attr);
MI_S32 ALGO_LPR_SetParams(void *handle, const LprParams_t *params);
MI_S32 ALGO_LPR_Detect(void *handle, const LprInput_t *algo_input, LprResult_t lp_result[MAX_LPR_OBJECT], MI_S32 *lp_num);
MI_S32 ALGO_LPR_Recognize(void *handle, const LprInput_t *algo_input, LprInputAttr_t *input_attr, LprResult_t lp_result[MAX_LPR_OBJECT], MI_S32 *lp_num);
MI_S32 ALGO_LPR_DeinitHandle(void *handle);
MI_S32 ALGO_LPR_ReleaseHandle(void *handle);

#ifdef __cplusplus
}
#endif

#endif
