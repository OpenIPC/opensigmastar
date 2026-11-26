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

#ifndef __SGS_SED_API_H__
#define __SGS_SED_API_H__

#ifdef __cplusplus
extern "C"
{
#endif
#define MAX_SED_STRLEN (256)
#define MAX_SED_EVENT_NUM (2)

typedef struct
{
    char ipu_firmware_path[MAX_SED_STRLEN]; // ipu_firmware.bin path
    char model_path[MAX_SED_STRLEN];        // detect model path
    MI_BOOL create_device;                  // set false to create ipu device outside algo lib
    MI_BOOL destroy_device;                 // set false to destroy ipu device outside algo lib
    void *model_buffer;                     // set it when load model from memory
    MI_U32 model_buffer_len;                // set it when load model from memory
} SedInit_t;

typedef struct
{
    MI_S32 frame_len;
} SedInputAttr_t;


typedef struct
{
    MI_S32 sample_rate;
    MI_S32 bit_width;
    void *buffer;
    MI_U32 buffer_len;
} SedInput_t;

typedef struct
{
    MI_S32 smooth_length;                         // output result smooth len; default 0
    MI_FLOAT vad_threshold;                       // vad threshold; default -45
    MI_FLOAT event_threshold[MAX_SED_EVENT_NUM];  // event score threshold; default [0.5, 0.5]
    MI_U32 min_trigger_times[MAX_SED_EVENT_NUM];  // return a positive event when detected more than min_trigger_times, default 1
    MI_FLOAT lsd_threshold;                       // large sound detection rms threshold(unit: dB)  default 0 means lsd is disabled
} SedParams_t;

typedef struct
{
    MI_BOOL is_valid;
    MI_S32 event_index;
    MI_FLOAT event_score;
} SedOutput_t;


typedef struct
{
    MI_BOOL is_large_sound;
    MI_FLOAT max_rms;
} SedLsdOutput_t;

MI_S32 ALGO_SED_CreateHandle(void **handle);
MI_S32 ALGO_SED_InitHandle(void *handle, const SedInit_t *init_info);
MI_S32 ALGO_SED_SetParams(void *handle, const SedParams_t *params);
MI_S32 ALGO_SED_GetParams(void *handle, SedParams_t *params);
MI_S32 ALGO_SED_GetInputAttr(void *handle, SedInputAttr_t *input_attr);
MI_S32 ALGO_SED_Run(void *handle, const SedInput_t *input, SedOutput_t *output);
MI_S32 ALGO_SED_Lsd(void *handle, const SedInput_t *input, SedLsdOutput_t* output);
MI_S32 ALGO_SED_DeinitHandle(void *handle);
MI_S32 ALGO_SED_ReleaseHandle(void *handle);
#ifdef __cplusplus
}
#endif

#endif