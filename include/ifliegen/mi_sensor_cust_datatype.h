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

#ifndef _MI_SENSOR_CUST_DATATYPE_H_
#define _MI_SENSOR_CUST_DATATYPE_H_

#include "mi_sensor_datatype.h"

typedef enum
{
    E_MI_CSI_CMD_INIT = 0,
    E_MI_CSI_CMD_DEINIT,
    E_MI_CSI_CMD_SET_CLK,
    E_MI_CSI_CMD_SET_LANE,
    E_MI_CSI_CMD_SET_LONG_PKT,
    E_MI_CSI_CMD_SET_HDR_MODE,
    E_MI_CSI_CMD_SET_PATTERN_GEN,
    E_MI_CSI_CMD_SET_CLK_DATA_SKIP,
    E_MI_CSI_CMD_SET_YUV_ORDER_SWAP,
    E_MI_CSI_CMD_MAX,
} MI_SNR_CsiCmd_e;

typedef enum {
    E_MI_CSI_MAC_CLK_DISABLE = -1, /**< Disable MIPI clock*/
    E_MI_CSI_MAC_CLK_432M    = 0,
    E_MI_CSI_MAC_CLK_384M    = 1,
    E_MI_CSI_MAC_CLK_345M    = 2,
    E_MI_CSI_MAC_CLK_320M    = 3,
    E_MI_CSI_MAC_CLK_288M    = 4,
    E_MI_CSI_MAC_CLK_240M    = 5,
    E_MI_CSI_MAC_CLK_216M    = 6,
    E_MI_CSI_MAC_CLK_172M    = 7,
    E_MI_CSI_MAC_CLK_144M    = 8,
} MI_SNR_CsiMacClk_e;

typedef enum {
    E_MI_SNR_MCLK_ID_27MHZ = 0,
    E_MI_SNR_MCLK_ID_21P6MHZ,
    E_MI_SNR_MCLK_ID_12MHZ,
    E_MI_SNR_MCLK_ID_5P4MHZ,
    E_MI_SNR_MCLK_ID_36MHZ,
    E_MI_SNR_MCLK_ID_54MHZ,
    E_MI_SNR_MCLK_ID_43P2MHZ,
    E_MI_SNR_MCLK_ID_61P7MHZ,
    E_MI_SNR_MCLK_ID_72MHZ,
    E_MI_SNR_MCLK_ID_48MHZ,
    E_MI_SNR_MCLK_ID_24MHZ,
    E_MI_SNR_MCLK_ID_37P125MHZ,
    E_MI_SNR_MCLK_ID_MAX,
} MI_SNR_MclkId_e;

typedef struct
{
    MI_U8              u8SnrPad;
    MI_U8              u8LaneNum;      //data_lane_num
    MI_U8              u8LaneEn;       //1:enable, 0:disable
    MI_U8              u8HdrEn;        //1:enable, 0:disable
    MI_U32             u32CtlCfg0_15;
    MI_U32             u32CtlCfg16_31;
    MI_U32             u32CtlCfg32_47;
    MI_SNR_HDRType_e   eCsiHdrMode;
    MI_SNR_CsiMacClk_e eClkValId;
} MI_SNR_CsiInit_t;

typedef struct
{
    MI_U8 u8SnrPad;
} MI_SNR_CsiDeInit_t;

typedef struct
{
    MI_U8  u8SnrPad;
    MI_U32 u32RxClkSkipNs;
    MI_U32 u32RxDataSkipNs;
} MI_SNR_CsiClkDataSkip_t;

typedef struct
{
    MI_U8  u8SnrPad;
    MI_U8  u8UserDef;  //0:Disable, 1:PDAF, 2:Gyro
    MI_U32 u32Width;   //sensor PDAF data width
    MI_U32 u32Height;  //sensor PDAF data height
    MI_U32 u32PktBit;  //Current User-Define packet bits num
} MI_SNR_CsiUserDefDataType_t;

typedef struct
{
    MI_U8              u8SnrPad;
    MI_SNR_CsiMacClk_e eClkValId;
} MI_SNR_CsiClk_t;

typedef struct
{
    MI_U8 u8SnrPad;
    MI_U8 u8LaneNum;  //data_lane_num
    MI_U8 u8LaneEn;   //1:enable, 0:disable
} MI_SNR_CsiLane_t;

typedef struct
{
    MI_U8  u8SnrPad;
    MI_U32 u32CtlCfg0_15;
    MI_U32 u32CtlCfg16_31;
    MI_U32 u32CtlCfg32_47;  //default:0, 0x1C00, 0
} MI_SNR_CsiLongPktType_t;

typedef struct
{
    MI_U8            u8SnrPad;
    MI_U8            u8HdrEn;    //1:enable, 0:disable
    MI_SNR_HDRType_e eCsiHdrMode;
} MI_SNR_CsiHdrModeType_t;

typedef enum
{
    E_MI_CSI_PAT_DATA_PREC_RAW10 = 0,
    E_MI_CSI_PAT_DATA_PREC_RAW12 = 1,
    E_MI_CSI_PAT_DATA_PREC_MAX
} MI_SNR_CsiPatDataPrec_e;

typedef struct
{
    MI_U8                   u8SnrPad;
    MI_U8                   u8En;
    MI_U8                   u8LaneNum;
    MI_U16                  u16Height;
    MI_U16                  u16Width;
    MI_U16                  u16Hblank;
    MI_U16                  u16Vblank;
    MI_SNR_CsiPatDataPrec_e u16DataPrec;
} MI_SNR_CsiPatGen_t;

typedef struct
{
    MI_U8  u8SnrPad;
    MI_U32 u32rxClkSkipNs;
    MI_U32 u32rxDataSkipNs;
} MI_SNR_CsiSkip_t;

typedef enum
{
    E_MI_CSI_YUV422_8BIT_OUT_YUYV = 0,
    E_MI_CSI_YUV422_8BIT_OUT_UYVY = 1,
    E_MI_CSI_YUV422_8BIT_OUT_MAX
} MI_SNR_CsiYcSwap_e;

typedef struct
{
    MI_U8              u8SnrPad;
    MI_SNR_CsiYcSwap_e eYcSwap;
} MI_SNR_CsiYcSwap_t;

#define IOCTL_MI_CSI_CMD_INIT               CAM_OS_IOW('i', E_MI_CSI_CMD_INIT, MI_SNR_CsiInit_t)
#define IOCTL_MI_CSI_CMD_DEINIT             CAM_OS_IOW('i', E_MI_CSI_CMD_DEINIT, MI_SNR_CsiDeInit_t)
#define IOCTL_MI_CSI_CMD_SET_CLK            CAM_OS_IOW('i', E_MI_CSI_CMD_SET_CLK, MI_SNR_CsiClk_t)
#define IOCTL_MI_CSI_CMD_SET_LANE           CAM_OS_IOW('i', E_MI_CSI_CMD_SET_LANE, MI_SNR_CsiLane_t)
#define IOCTL_MI_CSI_CMD_SET_LONG_PKT       CAM_OS_IOW('i', E_MI_CSI_CMD_SET_LONG_PKT, MI_SNR_CsiLongPktType_t)
#define IOCTL_MI_CSI_CMD_SET_HDR_MODE       CAM_OS_IOW('i', E_MI_CSI_CMD_SET_HDR_MODE, MI_SNR_CsiHdrModeType_t)
#define IOCTL_MI_CSI_CMD_SET_PATTERN_GEN    CAM_OS_IOW('i', E_MI_CSI_CMD_SET_PATTERN_GEN, MI_SNR_CsiPatGen_t)
#define IOCTL_MI_CSI_CMD_SET_CLK_DATA_SKIP  CAM_OS_IOW('i', E_MI_CSI_CMD_SET_CLK_DATA_SKIP, MI_SNR_CsiSkip_t)
#define IOCTL_MI_CSI_CMD_SET_YUV_ORDER_SWAP CAM_OS_IOW('i', E_MI_CSI_CMD_SET_YUV_ORDER_SWAP, MI_SNR_CsiYcSwap_t)

#endif
