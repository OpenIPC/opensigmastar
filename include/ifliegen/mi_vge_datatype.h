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

//-------------------------------------------------------------------------------------------------
//  Header Files
//-------------------------------------------------------------------------------------------------

#ifndef _MI_VGE_DATATYPE_H_
#define _MI_VGE_DATATYPE_H_

#include "mi_sys_datatype.h"

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Macros
//-------------------------------------------------------------------------------------------------

#define MI_VGE_MAX_PALETTE_TABLE_NUM 256
#define MI_VGE_POLY_VERTEX_MAX       6
#define MI_VGE_MAX_LINE              32
#define MI_VGE_MAX_COVER             8
#define MI_VGE_MAX_OSD               4
#define MI_VGE_MAX_MOSAIC            8
#define MI_VGE_MAX_FACEFRAME         8

#define MI_VGE_RET_SUCCESS       0
#define MI_ERR_VGE_NOT_INIT      MI_DEF_ERR(E_MI_MODULE_ID_VGE, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_VGE_NOT_INIT)
#define MI_ERR_VGE_DEV_INVALID   MI_DEF_ERR(E_MI_MODULE_ID_VGE, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_VGE_INVALID_DEVID)
#define MI_ERR_VGE_OUT_OF_RANGE  MI_DEF_ERR(E_MI_MODULE_ID_VGE, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_VGE_OUT_OF_RANGE)
#define MI_ERR_VGE_PARAM_INVALID MI_DEF_ERR(E_MI_MODULE_ID_VGE, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_VGE_ILLEGAL_PARAM)
#define MI_ERR_VGE_TIMEOUT       MI_DEF_ERR(E_MI_MODULE_ID_VGE, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_VGE_TIMEOUT)
#define MI_ERR_VGE_NULL_PTR      MI_DEF_ERR(E_MI_MODULE_ID_VGE, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_VGE_NULL_PTR)
#define MI_ERR_VGE_HAS_CREATED   MI_DEF_ERR(E_MI_MODULE_ID_VGE, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_VGE_HAS_CREATED)
#define MI_ERR_VGE_HAS_DESTROYED MI_DEF_ERR(E_MI_MODULE_ID_VGE, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_VGE_HAS_DESTROYED)
#define MI_ERR_VGE_DEV_BUSY      MI_DEF_ERR(E_MI_MODULE_ID_VGE, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_VGE_DEV_BUSY)
#define MI_ERR_VGE_FAILED        MI_DEF_ERR(E_MI_MODULE_ID_VGE, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_VGE_FAILED)

//-------------------------------------------------------------------------------------------------
//  Local Enum
//-------------------------------------------------------------------------------------------------

typedef enum
{
    E_MI_ERR_VGE_NOT_INIT      = 1,
    E_MI_ERR_VGE_NOT_CREATE    = 2,
    E_MI_ERR_VGE_INVALID_DEVID = 3,
    E_MI_ERR_VGE_OUT_OF_RANGE  = 4,
    E_MI_ERR_VGE_ILLEGAL_PARAM = 5,
    E_MI_ERR_VGE_TIMEOUT       = 6,
    E_MI_ERR_VGE_NULL_PTR      = 7,
    E_MI_ERR_VGE_HAS_CREATED   = 8,
    E_MI_ERR_VGE_HAS_DESTROYED = 9,
    E_MI_ERR_VGE_DEV_BUSY      = 10,
    E_MI_ERR_VGE_FAILED        = 11,
    E_MI_ERR_VGE_MAX
} MI_VGE_ErrCode_e;

typedef enum
{
    E_MI_VGE_COVER_MODE_RECT = 0,
    E_MI_VGE_COVER_MODE_POLY,
    E_MI_VGE_COVER_MODE_MAP,
    E_MI_VGE_COVER_MODE_MAX
} MI_VGE_CoverMode_e;

typedef enum
{
    E_MI_VGE_BLOCK_SIZE_4 = 0,
    E_MI_VGE_BLOCK_SIZE_8,
    E_MI_VGE_BLOCK_SIZE_16,
    E_MI_VGE_BLOCK_SIZE_32,
    E_MI_VGE_BLOCK_SIZE_64,
    E_MI_VGE_BLOCK_SIZE_128,
    E_MI_VGE_BLOCK_SIZE_MAX
} MI_VGE_BlockSize_e;

typedef enum
{
    E_MI_VGE_MOSAIC_MODE_RECT = 0,
    E_MI_VGE_MOSAIC_MODE_POLY,
    E_MI_VGE_MOSAIC_MODE_MAP,
    E_MI_VGE_MOSAIC_MODE_MAX
} MI_VGE_MosaicMode_e;

typedef enum
{
    E_MI_VGE_ALPHA_CONSTANT = 0,
    E_MI_VGE_ALPHA_PIXEL,
    E_MI_VGE_ALPHA_MAX
} MI_VGE_AlphaMode_e;

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------
typedef MI_S32 MI_VGE_HANDLE;

typedef struct MI_VGE_Point_s
{
    MI_S32 s32X;
    MI_S32 s32Y;
} MI_VGE_Point_t;

typedef struct MI_VGE_PaletteElement_s
{
    MI_U8 u8Alpha;
    MI_U8 u8Red;
    MI_U8 u8Green;
    MI_U8 u8Blue;
} MI_VGE_PaletteElement_t;

typedef struct MI_VGE_DeviceAttr_s
{
    MI_VGE_PaletteElement_t astElement[MI_VGE_MAX_PALETTE_TABLE_NUM];
} MI_VGE_DeviceAttr_t;

typedef struct MI_VGE_FrameInfo_s
{
    MI_SYS_PixelFormat_e enPixelFormat;
    MI_U32               u32Width;
    MI_U32               u32Height;
    MI_PHY               phyAddr[3];
    MI_U32               u32Stride[3];
    MI_U32               u32BufSize;
} MI_VGE_FrameInfo_t;

typedef struct MI_VGE_Task_s
{
    MI_VGE_FrameInfo_t stInputImg;
    MI_VGE_FrameInfo_t stOutputImg;
} MI_VGE_Task_t;

typedef struct MI_VGE_Line_s
{
    MI_VGE_Point_t stStartPoint;
    MI_VGE_Point_t stEndPoint;
    MI_U8          u8LineWidth;
    MI_U32         u32Color;
} MI_VGE_Line_t;

typedef struct MI_VGE_Rect_s
{
    MI_S32 s32X;
    MI_S32 s32Y;
    MI_U32 u32Width;
    MI_U32 u32Height;
} MI_VGE_Rect_t;

typedef struct MI_VGE_OsdArgb1555Alpha_s
{
    MI_U8 u8BgAlpha;
    MI_U8 u8FgAlpha;
} MI_VGE_OsdArgb1555Alpha_t;

typedef union
{
    MI_VGE_OsdArgb1555Alpha_t stArgb1555Alpha;
    MI_U8                     u8ConstantAlpha;
} MI_VGE_AlphaModePara_u;

typedef struct MI_VGE_OsdAlphaAttr_s
{
    MI_VGE_AlphaMode_e     enAlphaMode;
    MI_VGE_AlphaModePara_u stAlphaPara;
} MI_VGE_OsdAlphaAttr_t;

typedef struct MI_VGE_Osd_s
{
    MI_VGE_Rect_t           stOsdRect;
    MI_VGE_OsdAlphaAttr_t   stOsdAlphaAttr;
    MI_SYS_PixelFormat_e    enPixelFormat;
    MI_PHY                  phyAddr;
    MI_U32                  u32Stride;
    MI_U32                  u32BufSize;
    MI_U8                   u8PaletteIdx;
} MI_VGE_Osd_t;

typedef struct MI_VGE_Map_s
{
    MI_U8* u8MapAddr;
    MI_PHY phyAddr;
    MI_U32 u32Stride;
    MI_U32 u32Width;
    MI_U32 u32Height;
} MI_VGE_Map_t;

typedef struct MI_VGE_Polygon_s
{
    MI_U8          u8VertexNum;
    MI_VGE_Point_t astCoord[MI_VGE_POLY_VERTEX_MAX];
} MI_VGE_Polygon_t;

typedef struct MI_VGE_Cover_s
{
    MI_VGE_CoverMode_e enCoverMode;
    MI_BOOL            bHollow;
    MI_U8              u8Thickness;
    union
    {
        MI_VGE_Rect_t    stRectAttr;
        MI_VGE_Polygon_t stPolygonAttr;
        MI_VGE_Map_t     stMapAttr;
    };
    MI_U32 u32Color;

} MI_VGE_Cover_t;

typedef struct MI_VGE_Mosaic_s
{
    MI_VGE_BlockSize_e  enBlkSize;
    MI_VGE_MosaicMode_e enMosaicMode;
    union
    {
        MI_VGE_Rect_t    stRectAttr;
        MI_VGE_Polygon_t stPolygonAttr;
        MI_VGE_Map_t     stMapAttr;
    };
} MI_VGE_Mosaic_t;

typedef struct MI_VGE_FaceFrame_s
{
    MI_VGE_Rect_t stRectAttr;
    MI_U32        u32Color;
    MI_U8         u8Thickness;
} MI_VGE_FaceFrame_t;

//-------------------------------------------------------------------------------------------------
// Variable
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  local function  prototypes
//-------------------------------------------------------------------------------------------------

#endif
