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

//==============================================================================
//
//  File        : AudioAnrProcess.h
//  Description : Audio Noise reduction function
//  Author      : Ray-BR.chen
//  Revision    : 1.0
//
//==============================================================================

#ifdef __cplusplus
extern "C" {
#endif

#ifndef AUDIOANRPROCESS_H_
#define AUDIOANRPROCESS_H_
typedef void* ANR_HANDLE;
typedef void* ANR_BF_HANDLE;

#define _NR_BAND_NUM                    (7)
#define ANR_API_VERSION                 {'1','8'}
//==============================================================================
//
//                              ENUMERATIONS
//
//==============================================================================

typedef enum {
    ALGO_ANR_RET_SUCCESS,
    ALGO_ANR_RET_INIT_ERROR,
    ALGO_ANR_RET_INVALID_HANDLE,
    ALGO_ANR_RET_INVALID_SAMPLE_RATE,
    ALGO_ANR_RET_INVALID_POINT_NUMBER,
    ALGO_ANR_RET_INVALID_CHANNEL,
    ALGO_ANR_RET_INVALID_ENABLE,
    ALGO_ANR_RET_INVALID_MODE,
    ALGO_ANR_RET_INVALID_INTENSITY,
    ALGO_ANR_RET_INVALID_SMOOTH_LEVEL,
    ALGO_ANR_RET_INVALID_CONVERGE_SPEED,
    ALGO_ANR_RET_API_CONFLICT,
    ALGO_ANR_RET_INVALID_CALLING,
    ALGO_ANR_RET_INVALID_FILTER_MODE,
    ALGO_ANR_RET_FAILED
}ALGO_ANR_RET;

typedef enum {
    IAA_ANR_SAMPLE_RATE_8000  =  8000 ,
    IAA_ANR_SAMPLE_RATE_16000 = 16000 ,
    IAA_ANR_SAMPLE_RATE_32000 = 32000 ,
    IAA_ANR_SAMPLE_RATE_48000 = 48000 ,
}IAA_ANR_SAMPLE_RATE;

typedef enum {
    NR_SPEED_LOW,
    NR_SPEED_MID,
    NR_SPEED_HIGH
} NR_CONVERGE_SPEED;

/*ANR config structure*/

typedef struct{
    unsigned int anr_enable;
    unsigned int user_mode;
    int anr_intensity_band[_NR_BAND_NUM-1];
    int anr_intensity[_NR_BAND_NUM];
    unsigned int anr_smooth_level; //range 0-10
    NR_CONVERGE_SPEED anr_converge_speed; //0 1 2 higher the speed more fast
    int anr_filter_mode; // 0~5
}AudioAnrConfig;

/*ANR init structure*/
typedef struct {
    unsigned int point_number;
    unsigned int channel;
    IAA_ANR_SAMPLE_RATE sample_rate;
}AudioAnrInit;

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================
unsigned int IaaAnr_GetBufferSize(void);
ANR_HANDLE IaaAnr_Init(char* working_buffer_address, AudioAnrInit *anr_init);
ALGO_ANR_RET IaaAnr_Config(ANR_HANDLE handle, AudioAnrConfig *anr_config);
ALGO_ANR_RET IaaAnr_Run(ANR_HANDLE handle, short* pss_audio_in);
ALGO_ANR_RET IaaAnr_Free(ANR_HANDLE handle);
ANR_HANDLE IaaAnr_Reset(char* working_buffer_address, AudioAnrInit *anr_init);
ALGO_ANR_RET IaaAnr_GetConfig(ANR_HANDLE handle, AudioAnrInit *anr_init, AudioAnrConfig *anr_config);
ALGO_ANR_RET IaaAnr_EnhanceNNBFMode(ANR_HANDLE handle, ANR_BF_HANDLE handle2, int is_enable);
ALGO_ANR_RET IaaAnr_SetHandleId(ANR_HANDLE handle, int id);
unsigned int IaaAnr_GetJsonFileSize(char* jsonfile);
ALGO_ANR_RET IaaAnr_ReadJson(ANR_HANDLE handle, char* jsonBuffer, char* jsonfile, unsigned int buffSize);
ALGO_ANR_RET IaaAnr_EnableComfortNoise(ANR_HANDLE handle, int enable, int dB);
ALGO_ANR_RET IaaAnr_EnableAutoSmooth(ANR_HANDLE handle, int enable);
ALGO_ANR_RET IaaAnr_InitReadFromJson(AudioAnrInit *anr_init, char* jsonBuffer, char* jsonfile, unsigned int buffSize);
ALGO_ANR_RET IaaAnr_ConfigReadFromJson(AudioAnrConfig *anr_config, char* jsonBuffer, char* jsonfile, unsigned int buffSize);
ALGO_ANR_RET IaaAnr_NrEstimateReferAec(ANR_HANDLE handle, short* aec_in);
#endif// #ifndef _AUDIOANRPROCESS_H_


#ifdef __cplusplus
}
#endif
