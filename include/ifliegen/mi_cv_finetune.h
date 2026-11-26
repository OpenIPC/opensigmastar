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

#ifndef __MI_CV_FINETUNE_H__
#define __MI_CV_FINETUNE_H__

typedef void *MI_CV_FINETUNE_Handle_t;
typedef unsigned long long int MI_CV_FINETUNE_PhyAddr_t;

typedef enum MI_CV_FINETUNE_BufType_e
{
    E_MI_CV_FINETUNE_BUFTYPE_NONCONTINUED = 0x00,
    E_MI_CV_FINETUNE_BUFTYPE_CONTINUED = 0x01
} MI_CV_FINETUNE_BufType_e;

typedef enum MI_CV_FINETUNE_RetCode_e
{
    E_MI_CV_FINETUNE_SUCCESS = 0x00,
    E_MI_CV_FINETUNE_ERR_INVALID_PARA = 0x01,
    E_MI_CV_FINETUNE_ERROR = 0x02,
    E_MI_CV_FINETUNE_ERR_VERSION_ERR = 0x03,
    E_MI_CV_FINETUNE_ERR_PATH_INCORRECT = 0x04
} MI_CV_FINETUNE_RetCode_e;

typedef struct MI_CV_FINETUNE_BufBlock_t
{
    void *pBufAddr;
    unsigned int u32BufSz;
    MI_CV_FINETUNE_BufType_e eBufType;
} MI_CV_FINETUNE_BufBlock_t;

typedef MI_CV_FINETUNE_RetCode_e (*MI_CV_FINETUNE_MemHandler)(MI_CV_FINETUNE_BufBlock_t *pstBufBlock, void *pMemHandle);
typedef struct MI_CV_FINETUNE_CallBack_t
{
    MI_CV_FINETUNE_MemHandler pfnTUNEMalloc;
    MI_CV_FINETUNE_MemHandler pfnTUNEFree;
    MI_CV_FINETUNE_MemHandler pfnTUNEFlush;
    void *pMemHandle;
} MI_CV_FINETUNE_CallBack_t;

typedef enum MI_CV_FINETUNE_Status_e
{
    E_MI_CV_FINETUNE_STATUS_NEED_IMAGE = 0x00,
    E_MI_CV_FINETUNE_STATUS_NEED_TUNE = 0x01,
    E_MI_CV_FINETUNE_STATUS_NONEED_TUNE = 0x02,
    E_MI_CV_FINETUNE_STATUS_NEED_UPDATE = 0x03,
    E_MI_CV_FINETUNE_STATUS_NONEED_UPDATE = 0x04
} MI_CV_FINETUNE_Status_e;

typedef struct
{
    void *pData;
    unsigned int u32Stride;
    unsigned int u32Width;
    unsigned int u32Height;
} MI_CV_FINETUNE_Image_t;

typedef struct MI_CV_FINETUNE_SensorInfo_t
{
    unsigned int u32ISO;
} MI_CV_FINETUNE_SensorInfo_t;

/**
 * @brief init module handle
 *
 * @param pstHandle module handle pointer
 * @param pstCallBack callback function pointer
 * @return MI_CV_FINETUNE_RetCode_e
 */
MI_CV_FINETUNE_RetCode_e MI_CV_FINETUNE_Init(MI_CV_FINETUNE_Handle_t* pstHandle, MI_CV_FINETUNE_CallBack_t* pstCallBack);

/**
 * @brief reset module pamater
 *
 * @param stHandle module handle
 * @param pInJsonBuf json buffer pointer
 * @return MI_CV_FINETUNE_RetCode_e
 */
MI_CV_FINETUNE_RetCode_e MI_CV_FINETUNE_LoadParam(MI_CV_FINETUNE_Handle_t stHandle, void *pInJsonBuf);

/**
 * @brief query stitch status
 *
 * @param stHandle module handle
 * @param pstImg camera image pointer
 * @param pstSensorInfo sensor information
 * @param peStatus module status pointer
 * @return MI_CV_FINETUNE_RetCode_e
 */
MI_CV_FINETUNE_RetCode_e MI_CV_FINETUNE_QueryStatus(MI_CV_FINETUNE_Handle_t stHandle, MI_CV_FINETUNE_Image_t* pstImg,
    MI_CV_FINETUNE_SensorInfo_t* pstSensorInfo, MI_CV_FINETUNE_Status_e* peStatus);

/**
 * @brief run tune process
 *
 * @param stHandle module handle
 * @param pstImg camera image pointer
 * @param pStatus module status pointer
 * @param pOutJsonBuf pointer of output json buffer pointer
 * @return MI_CV_FINETUNE_RetCode_e
 */
MI_CV_FINETUNE_RetCode_e MI_CV_FINETUNE_Process(MI_CV_FINETUNE_Handle_t stHandle, MI_CV_FINETUNE_Image_t* pstImg, MI_CV_FINETUNE_Status_e* peStatus, void **pOutJsonBuf);

/**
 * @brief deinit module handle
 *
 * @param stHandle module handle
 * @return MI_CV_FINETUNE_RetCode_e
 */
MI_CV_FINETUNE_RetCode_e MI_CV_FINETUNE_Deinit(MI_CV_FINETUNE_Handle_t stHandle);

#endif
