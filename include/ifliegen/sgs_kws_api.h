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
#pragma once
#ifdef __cplusplus
extern "C"
{
#endif
#ifndef MAX_KWS_STRLEN
#define MAX_KWS_STRLEN 256
#endif // !MAX_KWS_STRLEN

#ifndef MAX_KWS_NUM
#define MAX_KWS_NUM 16
#endif // !MAX_KWS_NUM

typedef void* KWS_HANDLE;
typedef struct KwsInit_
{
    MI_U8 ipu_firmware_bin[MAX_KWS_STRLEN];
    MI_U8 kws_model_path[MAX_KWS_STRLEN];
    MI_U8 units_path[MAX_KWS_STRLEN];
    MI_U32 beam_size;
    MI_U32 variable_size;
    MI_BOOL create_device;
    MI_BOOL destroy_device;
}KwsInit_t;
typedef struct
{
    MI_FLOAT threshold[MAX_KWS_NUM];
    MI_BOOL strict[MAX_KWS_NUM];
    MI_U8 key_words[MAX_KWS_NUM][MAX_KWS_STRLEN];
    MI_U32 words_num;
}KwsParams_t;
typedef struct
{
    void* p_vir_addr;
    MI_S32 sample_rate; //only support 16000
    MI_S32 bit_width;  //only support 16
    MI_S32 channel;    //only support 1
    MI_U16 length;
}KwsInput_t;
typedef struct
{
    MI_FLOAT score;
    MI_U8 centence[MAX_KWS_STRLEN];
    MI_U32 id;
}KwsResult_t;
MI_S32 ALGO_KWS_CreateHandle(KWS_HANDLE* handle);
MI_S32 ALGO_KWS_InitHandle(KWS_HANDLE handle, const KwsInit_t init_info);
MI_S32 ALGO_KWS_GetInputLength(KWS_HANDLE handle, MI_S32* input_length);
MI_S32 ALGO_KWS_SetParams(KWS_HANDLE handle, const KwsParams_t* params);
MI_S32 ALGO_KWS_Run(KWS_HANDLE handle, KwsInput_t* srcData, KwsResult_t* results);
MI_S32 ALGO_KWS_DeInitHandle(KWS_HANDLE handle);
MI_S32 ALGO_KWS_ReleaseHandle(KWS_HANDLE handle);

#ifdef __cplusplus
}
#endif
