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


#ifndef __SGS_FR_API_H__
#define __SGS_FR_API_H__

#ifdef __cplusplus
extern "C"
{
#endif


#define MAX_FR_STRLEN 256
#define MAX_FR_OBJECT 50
#define FR_POINT_LEN 5
#define FEATURE_LEN 512

/*********************base**********************/
typedef enum{
    FR_MODE_FACE_DET=0x01,
    FR_MODE_FACE_RECOG=0x02,
    FR_MODE_FACE_ATTR=0x04,
    FR_MODE_FACE_EMOTION=0x08,
    FR_MODE_FACE_QUALITY=0x10,
}FrMode_e;


/*********************detect********************/
typedef enum{
    FR_DETECT_MODE_DET_MAX_FACE=0,   // 表示检测取最大人脸
    FR_DETECT_MODE_DET_ALL_FACE,     // 表示检测，取所有人脸
    FR_DETECT_MODE_TRACK_MAX_FACE,   // 表示视频输入，取最大脸，加入跟踪id
    FR_DETECT_MODE_TRACK_ALL_FACE,   // 表示视频输入，取所有脸，加入跟踪id
}FrDetectMode_e;

typedef struct FrPoint_
{
    MI_FLOAT x;
    MI_FLOAT y;
}FrPoint_t;

typedef struct FrBox_
{
    MI_FLOAT x1;
    MI_FLOAT y1;
    MI_FLOAT x2;
    MI_FLOAT y2;
    MI_FLOAT score;
    FrPoint_t landmark[FR_POINT_LEN];
    MI_U64 face_id;                    // track id
    MI_BOOL befiltered;
    MI_FLOAT move_sensitive;   // 越小表示人脸运动幅度越大
}FrBox_t;


/*********************attr**********************/
typedef struct FrAttr_t_
{
    MI_BOOL   eye_glass;
    MI_BOOL   male;
    MI_BOOL   mask;
    MI_S32    age;
    MI_BOOL   mustache;
    MI_S32    emotion;
}FrAttr_t;


/*********************face quality***************/
typedef struct FaceQualityAngle_
{
    MI_FLOAT pitch;
    MI_FLOAT yaw;
    MI_FLOAT roll;
    MI_FLOAT quality;
}FaceQualityAngle_t;


typedef struct
{
    MI_U32 width;
    MI_U32 height;
    MI_IPU_ELEMENT_FORMAT format;
} FrInputAttr_t;


typedef struct
{
    char ipu_firmware_path[MAX_FR_STRLEN];       // ipu_firmware.bin path
    char det_model_path[MAX_FR_STRLEN];          // detect model path, set when chose FR_MODE_FACE_DET mode
    char feature_model_path[MAX_FR_STRLEN];      // feature extraction model path, set when chose FR_MODE_FACE_RECOG mode
    char cos_model_path[MAX_FR_STRLEN];          // feature comparasion model path, set when chose FR_MODE_FACE_RECOG mode
    char attr_model_path[MAX_FR_STRLEN];         // face attr model path, set when chose FR_MODE_FACE_ATTR mode
    char emo_model_path[MAX_FR_STRLEN];          // face emotion model path, set when chose FR_MODE_FACE_EMOTION mode
    char quality_model_path[MAX_FR_STRLEN];     // face angle model path, set when chose FR_MODE_FACE_QUALITY mode
    MI_BOOL create_device;                       // set false to create ipu device outside algo lib
    MI_BOOL destroy_device;                      // set false to destroy ipu device outside algo lib
    MI_U8 fr_mode;
} FrInit_t;


typedef struct
{
    void *p_vir_addr;
    MI_PHY phy_addr;
    MI_U32 buf_size;
    MI_U64 pts;
} FrInput_t;


typedef struct
{
    MI_S32 disp_width;
    MI_S32 disp_height;

    // detect
    MI_FLOAT det_threshold;
    MI_S32  box_min_size;
    MI_FLOAT face_angle_ratio;
    MI_FLOAT eye_distance;

    // attr, emotion
    MI_FLOAT attr_threshold;

    // track
    MI_FLOAT motion_sensitive;

    // face quality
    MI_FLOAT face_quality_threshold;
} FrParams_t;



MI_S32 ALGO_FR_CreateHandle(void **handle);
MI_S32 ALGO_FR_InitHandle(void *handle, const FrInit_t *init_info);
MI_S32 ALGO_FR_GetInputAttr(void *handle, FrInputAttr_t *det_input_attr, FrInputAttr_t *attr_input_attr, FrInputAttr_t *quality_input_attr);
MI_S32 ALGO_FR_SetParams(void *handle, const FrParams_t *params);
MI_S32 ALGO_FR_DeinitHandle(void *handle);
MI_S32 ALGO_FR_ReleaseHandle(void *handle);

MI_S32 ALGO_FR_Detect(void *handle, const FrInput_t *algo_input, const FrDetectMode_e *detect_mode, FrBox_t bboxes[MAX_FR_OBJECT], MI_S32 *num_bboxes);
MI_S32 ALGO_FR_Align(void *handle, MI_U8* image_data, MI_S32 type, FrBox_t *detect_box, MI_U8* out_data);
MI_S32 ALGO_FR_Attr(void *handle, const FrInput_t *algo_input, FrAttr_t *results);
MI_S32 ALGO_FR_FaceQuality(void *handle, const FrInput_t *algo_input, FrBox_t *detect_box, FaceQualityAngle_t *angle);
MI_S32 ALGO_FR_FeatureExtract(void *handle, MI_U8* image_data, MI_S16* out_feature);
MI_S32 ALGO_FR_FeatureCompare(MI_S16* feature1, MI_S16* feature2, MI_S32 length, MI_FLOAT* similarity);
MI_S32 ALGO_FR_BatchFeatureCompare(void *handle, MI_S16* single_feature, MI_S16* batch_features, MI_S32 batch, MI_FLOAT* similarity);
MI_S32 ALGO_FR_BatchFeatureCompareV2(void *handle, const FrInput_t *single_feature, const FrInput_t *batch_features, MI_S32 batch, MI_FLOAT* similarity);



#ifdef __cplusplus
}
#endif

#endif