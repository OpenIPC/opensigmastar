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
#ifdef __cplusplus
extern "C" {
#endif

#ifndef AUDIOSRCPROCESS_H_
#define AUDIOSRCPROCESS_H_
#define SRC_API_VERSION                     {'1','3'}

typedef enum {
    ALGO_SRC_RET_SUCCESS                = 0x00000000,
    ALGO_SRC_RET_INIT_ERROR             = 0x10000601,
    ALGO_SRC_RET_INVALID_MODE           = 0x10000602,
    ALGO_SRC_RET_INVALID_HANDLE         = 0x10000603,
    ALGO_SRC_RET_INVALID_CHANNEL        = 0x10000604,
    ALGO_SRC_RET_INVALID_POINT_NUMBER   = 0x10000605,
    ALGO_SRC_RET_INVALID_SAMPLE_RATE    = 0x10000606,
    ALGO_SRC_RET_API_CONFLICT           = 0x10000607,
    ALGO_SRC_RET_INVALID_CALLING        = 0x10000608,
    ALGO_SRC_RET_INVALID_JSONFILE       = 0x10000609,
    ALGO_SRC_RET_FAILED                  = 0x10000610
} ALGO_SRC_RET;

typedef enum{
    SRC_8k_to_16k   = 0,
    SRC_8k_to_32k   = 1,
    SRC_8k_to_48k   = 2,
    SRC_16k_to_8k   = 3,
    SRC_16k_to_32k  = 4,
    SRC_16k_to_48k  = 5,
    SRC_32k_to_8k   = 6,
    SRC_32k_to_16k  = 7,
    SRC_32k_to_48k  = 8,
    SRC_48k_to_8k   = 9,
    SRC_48k_to_16k  = 10,
    SRC_48k_to_32k  = 11,
    SRC_24k_to_8k   = 12,
    SRC_24k_to_16k  = 13,
    SRC_24k_to_32k  = 14,
    SRC_24k_to_48k  = 15,
    SRC_8k_to_24k   = 16,
    SRC_16k_to_24k  = 17,
    SRC_32k_to_24k  = 18,
    SRC_48k_to_24k  = 19
}SrcConversionMode;

typedef enum{
    SRATE_8K  =  8,
    SRATE_16K =  16,
    SRATE_32K =  32,
    SRATE_48K =  48,
    SRATE_24K =  24
}SrcInSrate;

typedef enum{
    ORDER_LOW   =   0,
    ORDER_HIGH  =   1
}SrcFilterOrder;

typedef void* SRC_HANDLE;

typedef struct{
    SrcInSrate WaveIn_srate;
    SrcConversionMode mode;
    SrcFilterOrder order;
    unsigned int channel;
    unsigned int point_number;
}SRCStructProcess;

unsigned int IaaSrc_GetBufferSize(SrcConversionMode mode);
SRC_HANDLE IaaSrc_Init(char *workingBufferAddress, SRCStructProcess *src_struct);
ALGO_SRC_RET IaaSrc_Run(SRC_HANDLE handle, short *audio_input, short *audio_output, int* output_size);
ALGO_SRC_RET IaaSrc_Release(SRC_HANDLE handle);
unsigned int IaaSrc_GetJsonFileSize(char* jsonfile);
ALGO_SRC_RET IaaSrc_StructReadFromJson(SRCStructProcess* src_struct, char* jsonBuffer, char* jsonfile, unsigned int buffSize);
#endif /* AUDIOSRCPROCESS_H_ */

#ifdef __cplusplus
}
#endif
