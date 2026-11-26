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

#ifndef _MI_ISP_AF_DATATYPE_H_
#define _MI_ISP_AF_DATATYPE_H_

#ifdef __USE_USERSPACE_3A__
#include "mi_common.h"
#endif

/************************************* AF Define the structure declaration START **************************************/
#define MI_ISP_AF_WIN_CNT 256

typedef enum __attribute__((aligned(4)))
{
    E_SS_AF_FALSE = 0,
    E_SS_AF_TRUE  = !E_SS_AF_FALSE,
    E_SS_AF_BOOL_MAX
} MI_ISP_AF_Bool_e;

typedef enum __attribute__((aligned(4)))
{
    E_SS_AF_OP_TYP_AUTO   = 0,
    E_SS_AF_OP_TYP_MANUAL = !E_SS_AF_OP_TYP_AUTO,
    E_SS_AF_OP_TYP_MODE_MAX
} MI_ISP_AF_OpType_e;

typedef enum __attribute__((aligned(4)))
{
    E_SS_AF_STATE_NORMAL = 0,
    E_SS_AF_STATE_PAUSE  = 1,
    E_SS_AF_STATE_MAX
} MI_ISP_AF_SmStateType_e;

typedef struct MI_ISP_AF_QueryInfoType_s
{
    MI_ISP_AF_Bool_e bIsStable;      // 0~1
    MI_U16           u16CurMotorPos; // 0~1023
} MI_ISP_AF_QueryInfoType_t;

typedef enum __attribute__((aligned(4)))
{
    E_SS_AF_ALGO_ONESHOT    = 0,
    E_SS_AF_ALGO_CONTINUOUS = 1
} MI_ISP_AF_AlgoType_e;

typedef enum __attribute__((aligned(4)))
{
    E_SS_AF_ACC_IIRH = 0,
    E_SS_AF_ACC_IIRL = 1,
    E_SS_AF_ACC_SBLV = 2,
    E_SS_AF_ACC_SBLH = 3,
    E_SS_AF_ACC_LUMA = 4,
    E_SS_AF_ACC_MAX  = 0xffffffff
} MI_ISP_AF_AccSelType_e;
typedef struct MI_ISP_AF_AdjustIIIType_s
{
    MI_ISP_AF_AccSelType_e eFDAccSel;           // 0~3
    MI_ISP_AF_AccSelType_e eNoFDAccSel;         // 0~3
    MI_ISP_AF_AccSelType_e eSearchLowLuxAccSel; // 0~3
    MI_U16                 u16BlendLowLuxFilterTh;
    MI_U16                 u16ProtectTh_Low;
    MI_U16                 u16ProtectTh_High;
    MI_U16                 u16StableAWBTh;
    MI_U8                  u8CompensationRatio;
    MI_U8                  u8SearchParamScaleUp;
    MI_U8                  u8LocalBlurDetectAfThUp;
    MI_U8                  u8LocalStableAfTh;
    MI_U8                  u8LocalStableAeTh;
} MI_ISP_AF_AdjustIIIType_t;

typedef struct MI_ISP_AF_FDAEInfoType_s
{
    MI_U8 u8IsStable;
} MI_ISP_AF_FDAEInfoType_t;

typedef struct MI_ISP_AF_AWBInfoType_s
{
    MI_U16 u16CurAWBCT;
} MI_ISP_AF_AWBInfoType_t;

typedef struct MI_ISP_AF_BackUpPositionType_s
{
    MI_ISP_AF_Bool_e bEnable; // 0~1
    MI_U8            u8Step;  // 0~255
} MI_ISP_AF_BackUpPositionType_t;

typedef enum __attribute__((aligned(4)))
{
    E_SS_AF_POS_MIN       = 0,
    E_SS_AF_POS_MAX       = 1,
    E_SS_AF_POS_CUSTOMIZE = 2
} MI_ISP_AF_StartVCMPosModeType_e;

typedef struct MI_ISP_AF_OffsetType_s
{
    MI_ISP_AF_Bool_e bEnable;   // 0~1
    MI_S16           s16Offset; // 0~1023
} MI_ISP_AF_OffsetType_t;

typedef struct MI_ISP_AF_AdjustIIType_s
{
    MI_U16 u16VariationTh;
    MI_U16 u16FocusAfAccCntTh_ByGain[16];
    MI_U16 u16FocusAfAccDiffTh_ByGain[16];
    MI_U8  u8DetectFlatTh;
    MI_U8  u8FlatStep;
    MI_U8  u8AddStep;
    MI_U8  u8DecStep;
    MI_U8  u8LocalMotionAfTh;
    MI_U8  u8LocalMotionAeTh;
    MI_U8  u8LocalBlurDetectAfTh;
    MI_U8  u8LocalBlurDetectAeTh;
    MI_U8  u8AvoidAccJumpTh;
    MI_U8  u8AvoidFlatTh;
} MI_ISP_AF_AdjustIIType_t;

typedef struct MI_ISP_AF_StartVCMPosType_s
{
    MI_ISP_AF_StartVCMPosModeType_e eMode;
    MI_U16                          u16Pos;
} MI_ISP_AF_StartVCMPosType_t;

typedef struct MI_ISP_AF_StatusType_s
{
    MI_U32 u32FocusArea;
    MI_U32 u32FaceArea;
    MI_U32 u32PreFaceArea;
    MI_U32 u32FocusFaceFV;
    MI_U32 u32FaceFV;
    MI_U32 u32PreFaceFV;
    MI_U32 u32FocusFaceLuma;
    MI_U32 u32FaceLuma;
    MI_U32 u32PreFaceLuma;
    MI_U32 u32IIRL_Max;
    MI_U32 u32IIRH_Max;
    MI_U32 u32SBLV_Max;
    MI_U32 u32SBLH_Max;
    MI_U16 u16DetectMotion1;
    MI_U16 u16DetectMotion2;
    MI_U16 u16DetectMotion3;
    MI_U16 u16FDAFTrigger;
    MI_U16 u16FDAFUnStableCnt;
    MI_U16 u16FDAFReTrigger;
    MI_U16 u16PointX;
    MI_U16 u16PointY;
    MI_U16 u16FDAE_MinX;
    MI_U16 u16FDAE_MinY;
    MI_U16 u16FDAE_MaxX;
    MI_U16 u16FDAE_MaxY;
    MI_U8  u8FirstSearchPeakDone;
    MI_U8  u8NoFaceToFace;
    MI_U8  u8NoFaceToFaceMoveInAdvance;
    MI_U8  u8StopUpDatePosition;
    MI_U8  u8FVDetectMotion;
    MI_U8  u8UpdateFDCoor;
    MI_U8  u8UpdateLegacyCoor;
    MI_U8  u8FaceMaybe;
} MI_ISP_AF_StatusType_t;

typedef struct MI_ISP_AF_AFVerInfoType_s
{
    MI_U32 u32Release_date;
    MI_U32 u32ReportID;
    MI_U8  u8TestVer;
    MI_U8  u8Major;
    MI_U8  u8Minor;
    MI_U8  u8Reserve;
} MI_ISP_AF_AFVerInfoType_t;

typedef struct MI_ISP_AF_FDAFInfoType_s
{
    MI_U16 u16MinX;
    MI_U16 u16MinY;
    MI_U16 u16MaxX;
    MI_U16 u16MaxY;
    MI_U16 u16PreMinX;
    MI_U16 u16PreMinY;
    MI_U16 u16PreMaxX;
    MI_U16 u16PreMaxY;
    MI_U8  u8WinWgt[256];   // 0~255
    MI_U8  u8FDStableCntTh; // 0~255
    MI_U8  u8FaceDetection; // 0~1
} MI_ISP_AF_FDAFInfoType_t;

typedef struct MI_ISP_AF_FDAFType_s
{
    MI_ISP_AF_Bool_e bEnable; // 0~1
    MI_U16           u16LegacyAF[4];
    MI_U16           u16FDStableArea[4];
    MI_U8            u8FDSensitivity;          // 0~100
    MI_U8            u8FDYSensitivity;         // 0~100
    MI_U8            u8FDReTriggerSensitivity; // 0~100
    MI_U8            u8FDStableCntTh;          // 0~100
    MI_U8            u8FDAreaStableTh;         // 0~100
    MI_U8            u8FDAccStableTh;          // 0~100
    MI_U8            u8FDYStableTh;            // 0~100
    MI_U8            u8FDCoorXInStableTh;      // 0~100
    MI_U8            u8FDCoorYInStableTh;      // 0~100
    MI_U8            u8FDCoorXOutStableTh;     // 0~100
    MI_U8            u8FDCoorYOutStableTh;     // 0~100
    MI_U8            u8PreFDAccStableTh;       // 0~100
    MI_U8            u8PreFDMotionStableTh;    // 0~100
    MI_U8            u8PreFDYStableTh;         // 0~100
    MI_U8            u8PreFDAreaDetectMotion;  // 0~100
    MI_U8            u8PreFDCoorDetectMotion;  // 0~100
} MI_ISP_AF_FDAFType_t;

typedef struct MI_ISP_AF_DetectFlatZoneType_s
{
    MI_ISP_AF_Bool_e bEnable; // 0~1
    MI_U16           u16Th;
    MI_U16           u16MotorPos; // 0~1023
} MI_ISP_AF_DetectFlatZoneType_t;

typedef struct MI_ISP_AF_AdjustType_s
{
    MI_U16 u16AFTable[5];
    MI_U16 u16SearchMotorStep_ADJ[5];
    MI_U16 u16FocusAfAccCntTh_ADJ[5];
    MI_U16 u16FocusAfAccDiffTh_ADJ[5];
    MI_U8  u8CheckMotionCnt;
    MI_U8  u8MotionAeAccDiffTh;
    MI_U8  u8MotionAeAccCntTh;
    MI_U8  u8MotionAfAccDiffTh;
    MI_U8  u8MotionAfAccCntTh;
    MI_U8  u8TriggerWinWgt[256];
    MI_U8  u8UnStableCntTh;
    MI_U8  u8AEStableCntTh;
    MI_U8  u8AfAccDelay;
} MI_ISP_AF_AdjustType_t;

typedef struct MI_ISP_AF_AttrType_s
{
    MI_ISP_AF_SmStateType_e eState;            // 0~1
    MI_ISP_AF_OpType_e      eType;             // 0~1
    MI_U16                  u16ManualMotorPos; // 0~1023
    MI_ISP_AF_AlgoType_e    eAlgo;             // 0~1
} MI_ISP_AF_AttrType_t;

typedef struct MI_ISP_AF_MotorType_s
{
    MI_U16 u16MinMotorPos;  // 0~1023
    MI_U16 u16MaxMotorPos;  // 0~1023
    MI_U16 u16MinMotorStep; // 0~1023
    MI_U16 u16MaxMotorStep; // 0~1023
} MI_ISP_AF_MotorType_t;

typedef struct MI_ISP_AF_AccWeightType_s
{
    MI_U8            u8WinNumX;                   // 1~16
    MI_U8            u8WinNumY;                   // 1~16
    MI_ISP_AF_Bool_e bEqualWinWgt;                // 0~1
    MI_U8            u8WinWgt[MI_ISP_AF_WIN_CNT]; // 0~255
    MI_ISP_AF_Bool_e bIIRHBlendEn;                // 0~1
    MI_U8            u8IIRHWgt_FirstBlendIIRL;    // 0~255
    MI_U8            u8IIRHWgt_SecondBlendSBLV;   // 0~255
    MI_U8            u8IIRHWgt_ThirdBlendSBLH;    // 0~255
} MI_ISP_AF_AccWeightType_t;

typedef struct MI_ISP_AF_OneShotType_s
{
    MI_ISP_AF_AccSelType_e eAccSel;      // 0~3
    MI_U16                 u16MotorStep; // 0~1023
} MI_ISP_AF_OneShotType_t;

typedef struct MI_ISP_AF_SceneChangeType_s
{
    MI_ISP_AF_AccSelType_e ePreAfAccSel;        // 0~3
    MI_U8                  u8PreAeAccDiffThOft; // 0~100, unit: %
    MI_U8                  u8PreAeAccDiffThSlp; // 0~100, unit: %
    MI_U8                  u8PreAeAccCntThOft;  // 0~100, unit: %
    MI_U8                  u8PreAeAccCntThSlp;  // 0~100, unit: %
    MI_U8                  u8PreAfAccDiffThOft; // 0~100, unit: %
    MI_U8                  u8PreAfAccDiffThSlp; // 0~100, unit: %
    MI_U8                  u8PreAfAccCntThOft;  // 0~100, unit: %
    MI_U8                  u8PreAfAccCntThSlp;  // 0~100, unit: %
    MI_ISP_AF_AccSelType_e eFocusAfAccSel;      // 0~3
    MI_U8                  u8FocusAeAccDiffTh;  // 0~100, unit: %
    MI_U8                  u8FocusAeAccCntTh;   // 0~100, unit: %
    MI_U8                  u8FocusAfAccDiffTh;  // 0~100, unit: %
    MI_U8                  u8FocusAfAccCntTh;   // 0~100, unit: %
    MI_U8                  u8StableCntTh;       // 0~255
} MI_ISP_AF_SceneChangeType_t;

typedef struct MI_ISP_AF_SearchStartType_s
{
    MI_U16                 u16SearchMotorStep;       // 0~1023
    MI_U16                 u16SearchMotorDirByPosTh; // 0~1023
    MI_ISP_AF_AccSelType_e eSearchAccSel;            // 0~3
} MI_ISP_AF_SearchStartType_t;

typedef struct MI_ISP_AF_SearchType_s
{
    MI_U8 u8MinMaxAccRatioPeakThOft;      // 0~100, unit: %
    MI_U8 u8MinMaxAccRatioPeakThSlp;      // 0~100, unit: %
    MI_U8 u8AccDecCntPeakTh;              // 0~255
    MI_U8 u8NowFakeMaxAccRatioPeakTh;     // 0~100, unit: %
    MI_U8 u8AccDecCntWrongDirTh;          // 0~255
    MI_U8 u8NowFakeMaxAccRatioWrongDirTh; // 0~100, unit: %
} MI_ISP_AF_SearchType_t;

/************************************* AF Define the structure declaration END ****************************************/

/************************************* AF Define the structure declaration START **************************************/

typedef struct MI_ISP_AF_VerInfoType_s
{
    MI_U32 u32ReleaseDate;
    MI_U32 u32ReportID;
    MI_U8  u8Major;
    MI_U8  u8Minor;
    MI_U8  u8TestVer;
    MI_U8  u8Reserve;
} MI_ISP_AF_VerInfoType_t;
/************************************* AF Define the structure declaration END ****************************************/

#endif //_MI_ISP_AF_DATATYPE_H_
