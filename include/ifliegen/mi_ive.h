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

#ifndef _MI_IVE_H_
#define _MI_IVE_H_

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#include "mi_ive_datatype.h"

/*****************************************************************************
*   Prototype    : MI_IVE_Create_handle
*   Description  : Create IVE handle index
*   Parameters   : Null
*   Return Value : Handle ID.
*****************************************************************************/
MI_IVE_HANDLE MI_IVE_Create_Handle(void);

/*****************************************************************************
*   Prototype    : MI_IVE_Create
*   Description  : Create IVE handle
*   Parameters   : MI_IVE_HANDLE            hHandle         Handle ID of a task
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_Create(MI_IVE_HANDLE hHandle);

/*****************************************************************************
*   Prototype    : MI_IVE_Destroy
*   Description  : Destroy IVE handle
*   Parameters   : MI_IVE_HANDLE            hHandle         Handle ID of a task
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_Destroy(MI_IVE_HANDLE hHandle);

/*****************************************************************************
*   Prototype    : MI_IVE_Filter
*   Description  : 5x5 template filter.
*   Parameters   : MI_IVE_HANDLE            hHandle         Handle ID of a task
*                  MI_IVE_SrcImage_t       *pstSrc          Input source data.
*                  MI_IVE_DstImage_t       *pstDst          Output result.
*                  MI_IVE_FilterCtrl_t     *pstFltCtrl      Control parameters of filter
*                  MI_BOOL                  bInstant        Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_Filter(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrc,
    MI_IVE_DstImage_t *pstDst, MI_IVE_FilterCtrl_t *pstFltCtrl,MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_Csc
*   Description  : Color space conversion.
*   Parameters   : MI_IVE_HANDLE         hHandle          Handle ID of a task
*                  MI_IVE_SrcImage_t    *pstSrc           Input source data:
*                  MI_IVE_DstImage_t    *pstDst           Output result:
*                  MI_IVE_CscCtrl_t     *pstCscCtrl       Control parameters for CSC
*                  MI_BOOL               bInstant         Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_Csc(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrc,
    MI_IVE_DstImage_t *pstDst, MI_IVE_CscCtrl_t *pstCscCtrl, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_FilterAndCsc
*   Description  : A composite task of template filter plus color space conversion.
*   Parameters   : MI_IVE_HANDLE               hHandle        Handle ID of a task.
*                  MI_IVE_SrcImage_t          *pstSrc         Input source data.
*                  MI_IVE_DstImage_t          *pstDst         Output result.
*                  MI_IVE_FilterAndCscCtrl_t  *pstFltCscCtrl  Control parameters.
*                  MI_BOOL                     bInstant       Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_FilterAndCsc(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrc,
        MI_IVE_DstImage_t *pstDst, MI_IVE_FilterAndCscCtrl_t *pstFltCscCtrl, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_Sobel
*   Description  : SOBEL is used to extract the gradient information.
*   Parameters   : MI_IVE_HANDLE           hHandle       Handle ID of a task
*                  MI_IVE_SrcImage_t      *pstSrc        Input source data.
*                  MI_IVE_DstImage_t      *pstDstH       The (horizontal) result of input image filtered by the input mask;
*                  MI_IVE_DstImage_t      *pstDstV       The (vertical) result of input image filtered by the transposed mask;
*                  MI_IVE_SobelCtrl_t     *pstSobelCtrl  Control parameters
*                  MI_BOOL                 bInstant      Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_Sobel(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrc,
        MI_IVE_DstImage_t *pstDstH, MI_IVE_DstImage_t *pstDstV,
        MI_IVE_SobelCtrl_t *pstSobelCtrl, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_MagAndAng
*   Description  : MagAndAng is used to extract the edge information.
*   Parameters   : MI_IVE_HANDLE           hHandle           Handle ID of a task
*                  MI_IVE_SRC_INFO_S      *pstSrc            Input source data.
*                  MI_IVE_MemInfo_t       *pstDstMag         Output magnitude.
*                  MI_IVE_MemInfo_t       *pstDstAng         Output angle.
*                                                            If the output mode is set to magnitude only,
*                                                            this item can be set to null.
*                  MI_IVE_MagAndAngCtrl_t *pstMagAndAngCtrl  Control parameters
*                  MI_BOOL                 bInstant          Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_MagAndAng(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrc,
        MI_IVE_DstImage_t *pstDstMag, MI_IVE_DstImage_t *pstDstAng,
        MI_IVE_MagAndAngCtrl_t *pstMagAndAngCtrl, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_Dilate
*   Description  : 5x5 template dilate.
*   Parameters   : MI_IVE_HANDLE             hHandle          Handle ID of a task
*                  MI_IVE_SrcImage_t        *pstSrc           Input binary image, which consists of 0 or 255;
*                  MI_IVE_DstImage_t        *pstDst           Output result.
*                  MI_IVE_DilateCtrl_t      *pstDilateCtrl    Control parameters.
*                  MI_BOOL                   bInstant         Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_Dilate(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrc,
    MI_IVE_DstImage_t *pstDst, MI_IVE_DilateCtrl_t *pstDilateCtrl,MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_Erode
*   Parameters   : 5x5 template erode.
*   Input        : MI_IVE_HANDLE               hHandle       Handle ID of a task
*                  MI_IVE_SrcImage_t          *pstSrc        Input binary image, which consists of 0 or 255;
*                  MI_IVE_DstImage_t          *pstDst        Output result.
*                  MI_IVE_ErodeCtrl_t         *pstErodeCtrl  Control parameters
*                  MI_BOOL                     bInstant      Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_Erode(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrc,
    MI_IVE_DstImage_t *pstDst, MI_IVE_ErodeCtrl_t *pstErodeCtrl,MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_Thresh
*   Description  : Thresh operation to the input image.
*   Parameters   : MI_IVE_HANDLE           hHandle       Handle ID of a task
*                  MI_IVE_SrcImage_t      *pstSrc        Input source data.
*                  MI_IVE_DstImage_t      *pstDst        Output result
*                  MI_IVE_ThreshCtrl_t    *pstThrCtrl    Control parameters
*                  MI_BOOL                 bInstant      Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_Thresh(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrc,
    MI_IVE_DstImage_t *pstDst, MI_IVE_ThreshCtrl_t *pstThrCtrl, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_And
*   Description  : Binary images' And operation.
*   Parameters   : MI_IVE_HANDLE          hHandle       Handle ID of a task
*                  MI_IVE_SrcImage_t     *pstSrc1       The input source1.
*                  MI_IVE_SrcImage_t     *pstSrc2       The input source2.
*                  MI_IVE_DstImage_t     *pstDst        Output result of " src1 & src2 ".
*                  MI_BOOL                bInstant      Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_And(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrc1,
    MI_IVE_SrcImage_t *pstSrc2, MI_IVE_DstImage_t *pstDst, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_Sub
*   Description  : Two gray images' Sub operation.
*   Parameters   : MI_IVE_HANDLE          hHandle     Handle ID of a task
*                  MI_IVE_SrcImage_t     *pstSrc1     Minuend of the input source.
*                  MI_IVE_SrcImage_t     *pstSrc2     Subtrahend of the input source.
*                  MI_IVE_DstImage_t     *pstDst      Output result of src1 minus src2
*                  MI_IVE_SubCtrl_t      *pstSubCtrl  Control parameter
*                  MI_BOOL                bInstant    Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_Sub(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrc1,
    MI_IVE_SrcImage_t *pstSrc2, MI_IVE_DstImage_t *pstDst, MI_IVE_SubCtrl_t *pstSubCtrl, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_Or
*   Description  : Two binary images' Or operation.
*   Parameters   : MI_IVE_HANDLE          hHandle    Handle ID of a task
*                  MI_IVE_SrcImage_t     *pstSrc1    Input source1.
*                  MI_IVE_SrcImage_t     *pstSrc2    Input source2.
*                  MI_IVE_DstImage_t     *pstDst     Output result src1 or src2
*                  MI_BOOL                bInstant   Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_Or(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrc1,
    MI_IVE_SrcImage_t *pstSrc2, MI_IVE_DstImage_t *pstDst, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_INTEG
*   Description  : Calculate the input gray image's integral image.
*   Parameters   : MI_IVE_HANDLE           hHandle        Handle ID of a task
*                  MI_IVE_SrcImage_t      *pstSrc         Input source data.
*                  MI_IVE_DstImage_t      *pstDst         Output result.
*                  MI_IVE_IntegCtrl_t     *pstIntegCtrl   Integ Control
*                  MI_BOOL                 bInstant       Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_Integ(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrc,
    MI_IVE_DstImage_t *pstDst, MI_IVE_IntegCtrl_t *pstIntegCtrl, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_Hist
*   Description  : Calculate the input gray image's histogram.
*   Parameters   : MI_IVE_HANDLE        hHandle      Handle ID of a task
*                  MI_IVE_SrcImage_t   *pstSrc       Input source data.
*                  MI_IVE_DstMemInfo_t *pstDst       Output result.
*                  MI_BOOL              bInstant     Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_Hist(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrc,
    MI_IVE_DstMemInfo_t *pstDst, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_ThreshS16
*   Description  : S16 image's THRESH operation.
*   Parameters   : MI_IVE_HANDLE            hHandle        Handle ID of a task
*                  MI_IVE_SrcImage_t       *pstSrc         Input source data.
*                  MI_IVE_DstImage_t       *pstDst         Output result.
*                  MI_IVE_ThreshS16Ctrl_t  *pstThrS16Ctrl  Control parameters
*                  MI_BOOL                  bInstant       Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_ThreshS16(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrc,
    MI_IVE_DstImage_t *pstDst, MI_IVE_ThreshS16Ctrl_t *pstThrS16Ctrl, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_ThreshU16
*   Description  : U16 image's THRESH operation.
*   Parameters   : MI_IVE_HANDLE            hHandle        Handle ID of a task
*                  MI_IVE_SrcImage_t       *pstSrc         Input source data.
*                  MI_IVE_DstImage_t       *pstDst         Output result
*                  MI_IVE_ThreshU16Ctrl_t  *pstThrU16Ctrl  Control parameters
*                  MI_BOOL                  bInstant       Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_ThreshU16(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrc,
    MI_IVE_DstImage_t *pstDst, MI_IVE_ThreshU16Ctrl_t *pstThrU16Ctrl, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_16BitTo8Bit
*   Description  : Scale the input 16bit data to the output 8bit data.
*   Parameters   : MI_IVE_HANDLE               hHandle              Handle ID of a task
*                  MI_IVE_SrcImage_t          *pstSrc               Input source data.
*                  MI_IVE_DstImage_t          *pstDst               Output result
*                  MI_IVE_16BITTO8BIT_CTRL_S  *pst16BitTo8BitCtrl   control parameter
*                  MI_BOOL                     bInstant             Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_16BitTo8Bit(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrc,
    MI_IVE_DstImage_t *pstDst, MI_IVE_16bitTo8BitCtrl_t *pst16BitTo8BitCtrl, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_OrdStatFilter
*   Description  : Order Statistic Filter. It can be used as median\max\min value filter.
*   Parameters   : MI_IVE_HANDLE             hHandle             Handle ID of a task
*                  MI_IVE_SrcImage_t        *pstSrc              Input source data.
*                  MI_IVE_DstImage_t        *pstDst              Output result
*                  MI_IVE_OrdStatFilter_t   *pstOrdStatFltCtrl   Control parameter
*                  MI_BOOL                   bInstant            Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_OrdStatFilter(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrc,
    MI_IVE_DstImage_t *pstDst, MI_IVE_OrdStatFilter_t *pstOrdStatFltCtrl, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_Map
*   Description  : Map a image to another through a lookup table.
*   Parameters   : MI_IVE_HANDLE          hHandle         Handle ID of a task
*                  MI_IVE_SrcImage_t     *pstSrc          Input source.
*                  MI_IVE_SrcMemInfo_t   *pstMap          Input lookup table.
*                  MI_IVE_DstImage_t     *pstDst          Output result.
*                  MI_BOOL                bInstant        Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_Map(MI_IVE_HANDLE hHandle,MI_IVE_SrcImage_t *pstSrc,
    MI_IVE_SrcMemInfo_t *pstMap, MI_IVE_DstImage_t *pstDst,MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_EqualizeHist
*   Description  : Enhance the input image's contrast through histogram equalization.
*   Parameters   : MI_IVE_HANDLE                hHandle              Handle ID of a task
*                  MI_IVE_SrcImage_t           *pstSrc               Input source.
*                  MI_IVE_DstImage_t           *pstDst               Output result.
*                  MI_IVE_EQUALIZEHIST_CTRL_S  *pstEqualizeHistCtrl  EqualizeHist control parameter.
*                  MI_BOOL                      bInstant             Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_EqualizeHist(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrc,
    MI_IVE_DstImage_t *pstDst, MI_IVE_EqualizeHistCtrl_t *pstEqualizeHistCtrl, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_Add
*   Description  : Two gray images' Add operation.
*   Parameters   : MI_IVE_HANDLE          hHandle      Handle ID of a task
*                  MI_IVE_SrcImage_t     *pstSrc1      Augend of the input source.
*                  MI_IVE_SrcImage_t     *pstSrc2      Addend of the input source.
*                  MI_IVE_DstImage_t     *pstDst       Output result of src1 plus src2
*                  MI_IVE_AddCtrl_t      *pstAddCtrl   Control parameter
*                  MI_BOOL                bInstant     Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_Add(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrc1,
    MI_IVE_SrcImage_t *pstSrc2, MI_IVE_DstImage_t *pstDst, MI_IVE_AddCtrl_t *pstAddCtrl, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_Xor
*   Description  : Two binary images' Xor operation.
*   Parameters   : MI_IVE_HANDLE           hHandle    Handle ID of a task
*                  MI_IVE_SrcImage_t      *pstSrc1    The input source1.
*                  MI_IVE_SrcImage_t      *pstSrc2    The input source2.
*                  MI_IVE_DstImage_t      *pstDst     Output result
*                  MI_BOOL                 bInstant   Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_Xor(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrc1,
    MI_IVE_SrcImage_t *pstSrc2, MI_IVE_DstImage_t *pstDst, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_Ncc
*   Description  : Calculate two gray images' NCC (Normalized Cross Correlation).
*   Parameters   : MI_IVE_HANDLE           hHandle     Handle ID of a task
*                  MI_IVE_SrcImage_t      *pstSrc1     Input source1.
*                  MI_IVE_SrcImage_t      *pstSrc2     Input source2.
*                  MI_IVE_DstMemInfo_t    *pstDst      Output result
*                  MI_BOOL                 bInstant    Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_Ncc(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrc1,
    MI_IVE_SrcImage_t *pstSrc2, MI_IVE_DstMemInfo_t *pstDst, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_Ccl
*   Description  : Connected Component Labeling.
*   Parameters   : MI_IVE_HANDLE          hHandle         Handle ID of a task
*                  MI_IVE_SrcImage_t     *pstSrcDst       Input source.
*                  MI_IVE_DstMemInfo_t   *pstBlob         Output result of detected region.
*                  MI_IVE_CclCtrl_t      *pstCclCtrl      CCL control parameter
*                  MI_BOOL                bInstant        Reserved
*   Return Value : MI_SUCCESS: Success;   Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_Ccl(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrcDst,
    MI_IVE_DstMemInfo_t *pstBlob, MI_IVE_CclCtrl_t *pstCclCtrl, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_Gmm
*   Description  : Separate foreground and background using GMM(Gaussian Mixture Model) method;
*   Parameters   : MI_IVE_HANDLE          hHandle      Handle ID of a task
*                  MI_IVE_SrcImage_t     *pstSrc       Input source.
*                  MI_IVE_DstImage_t     *pstFg        Output foreground (Binary) image.
*                  MI_IVE_DstImage_t     *pstBg        Output background image.
*                  MI_IVE_MemInfo_t      *pstModel     Model data.
*                  MI_IVE_GmmCtrl_t      *pstGmmCtrl   Control parameter.
*                  MI_BOOL                bInstant     Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_Gmm(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrc, MI_IVE_DstImage_t *pstFg,
    MI_IVE_DstImage_t *pstBg, MI_IVE_MemInfo_t *pstModel, MI_IVE_GmmCtrl_t *pstGmmCtrl, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_CannyHysEdge
*   Description  : The first part of canny Edge detection. Including step: gradient calculation,
*                  magnitude and angle calculation, hysteresis threshold, NMS(Non-Maximum Suppression)
*   Parameters   : MI_IVE_HANDLE              hHandle              Handle ID of a task
*                  MI_IVE_SrcImage_t         *pstSrc               Input source.
*                  MI_IVE_DstImage_t         *pstEdge              Output result.
*                  MI_IVE_DstMemInfo_t       *pstStack             OutPut stack for CannyEdge
*                  MI_IVE_CannyHysEdgeCtrl_t *pstCannyHysEdgeCtrl  Control parameter.
*                  MI_BOOL                    bInstant             Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_CannyHysEdge(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrc, MI_IVE_DstImage_t *pstEdge,
       MI_IVE_DstMemInfo_t *pstStack, MI_IVE_CannyHysEdgeCtrl_t *pstCannyHysEdgeCtrl, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_CannyEdge
*   Description  : The second part of canny Edge detection: trace strong edge by weak edge.
*   Parameters   : MI_IVE_SrcImage_t  *pstEdge        Input and Output source.
*                  MI_IVE_MemInfo_t   *pstStack       stack for CannyEdge
*                  MI_BOOL             bInstant       Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_CannyEdge(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstEdge, MI_IVE_MemInfo_t *pstStack, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_Lbp
*   Description  : LBP calculation using the original method and a extensional method.
*   Parameters   : MI_IVE_HANDLE         hHandle        Handle ID of a task
*                  MI_IVE_SrcImage_t    *pstSrc         Input source.
*                  MI_IVE_DstImage_t    *pstDst         Output result
*                  MI_IVE_LbpCtrrl_t    *pstLbpCtrl     Control parameter
*                  MI_BOOL               bInstant       Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_Lbp(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrc1, MI_IVE_SrcImage_t *pstSrc2,
    MI_IVE_DstImage_t *pstDst, MI_IVE_LbpCtrrl_t *pstLbpCtrl, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_NormGrad
*   Description  : Gradient calculation and the output is normalized to S8.
*   Parameters   : MI_IVE_HANDLE            hHandle          Handle ID of a task
*                  MI_IVE_SrcImage_t       *pstSrc           Input source data
*                  MI_IVE_DstImage_t       *pstDstH          The (horizontal) result of input image filtered by the input mask;
*                  MI_IVE_DstImage_t       *pstDstV          The (vertical) result  of input image filtered by the transposed mask;
*                  MI_IVE_DstImage_t       *pstDstHV         Output the horizontal and vertical component in single image in package format.
*                  MI_IVE_NormGradCtrl_t   *pstNormGradCtrl  Control parameter
*                  MI_BOOL                  bInstant         Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_NormGrad(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrc,
    MI_IVE_DstImage_t *pstDstH, MI_IVE_DstImage_t *pstDstV, MI_IVE_DstImage_t *pstDstHV,
    MI_IVE_NormGradCtrl_t *pstNormGradCtrl, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_LkOpticalFlow
*   Description  : Calculate LK-Optical Flow in single-layer of the pyramid.
*   Parameters   : MI_IVE_HANDLE                hHandle             Handle ID of a task
*                  MI_IVE_SrcImage_t           *pstSrcPre           Pre-frame input source.
*                  MI_IVE_SrcImage_t           *pstSrcCur           Cur-frame input source. Of the same size and format with pstPreSrc.
*                  MI_IVE_SrcMemInfo_t         *pstPoint            Intresting points coordinates in the cur-layer
*                                                                   for LKOpticalFlow tracking.
*                  MI_IVE_MemInfo_t            *pstMv               Accumlative movements of the interesting points in pre-layers
*                                                                   or init 0s for the first-layer as input.                                                                init 0s .
*                                                                   Movements of the interesting points being tracked in cur-layer
*                                                                   as output.
*                  MI_IVE_LKOPTICALFLOW_CTRL_S *pstLkOptiFlowCtrl   Control parameters.
*                  MI_BOOL                      bInstant            Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
****************************************************************************/
MI_S32 MI_IVE_LkOpticalFlow(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrcPre, MI_IVE_SrcImage_t *pstSrcCur,
    MI_IVE_SrcMemInfo_t *pstPoint, MI_IVE_MemInfo_t *pstMv, MI_IVE_LkOpticalFlowCtrl_t *pstLkOptiFlowCtrl, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_Sad
*   Description  : Sum of absolute differences.
*   Parameters   : MI_IVE_HANDLE          hHandle      Handle ID of a task
*                  MI_IVE_SrcImage_t     *pstSrc1      The input source1.
*                  MI_IVE_SrcImage_t     *pstSrc2      The input source2.
*                  MI_IVE_DstImage_t     *pstSad       Output result of sad value.
*                  MI_IVE_DstImage_t     *pstThr       Output result of thresh.
*                  MI_IVE_SadCtrl_t      *pstSadCtrl   Control parameter
*                  MI_BOOL                bInstant     Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_Sad(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrc1,
    MI_IVE_SrcImage_t *pstSrc2, MI_IVE_DstImage_t *pstSad, MI_IVE_DstImage_t *pstThr,
    MI_IVE_SadCtrl_t *pstSadCtrl, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_Resize
*   Description  : Resize image
*   Parameters   : MI_IVE_HANDLE          hHandle          Handle ID of a task
*                  MI_IVE_SrcImage_t     *pstSrc           Input source.
*                  MI_IVE_DstImage_t     *pstDst           Output result
*                  MVE_IVE_ResizeCtrl_t  *pstResizeCtrl    Control parameters.
*                  MI_BOOL                bInstant         Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_Resize(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrc,
    MI_IVE_DstImage_t *pstDst, MVE_IVE_ResizeCtrl_t *pstResizeCtrl, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_Bernsen
*   Description  : A Bernsen thresholding task for the 3x3 and 5x5 windows.
*   Parameters   : MI_IVE_HANDLE             hHandle             Handle ID of a task
*                  MI_IVE_SrcImage_t        *pstSrc              Input source.
*                  MI_IVE_DstImage_t        *pstDst              Output result
*                  MVE_IVE_BernsenCtrl_t    *pstBernsenCtrl      Control parameters.
*                  MI_BOOL                   bInstant            Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_Bernsen(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrc,
    MI_IVE_DstImage_t *pstDst,MVE_IVE_BernsenCtrl_t *pstBernsenCtrl,MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_AdpThresh
*   Description  : An adaptive thresholding task.
*   Parameters   : MI_IVE_HANDLE               hHandle            Handle ID of a task
*                  MI_IVE_SrcImage_t          *pstSrc             Input source.
*                  MI_IVE_SrcImage_t          *pstInteg           Input integral image of source.
*                  MI_IVE_DstImage_t          *pstDst             Output result
*                  MVE_IVE_AdpThreshCtrl_t    *pstAdpThrCtrl      Control parameters.
*                  MI_BOOL                     bInstant           Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_AdpThresh(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrc,
    MI_IVE_SrcImage_t *pstInteg, MI_IVE_DstImage_t *pstDst, MVE_IVE_AdpThreshCtrl_t *pstAdpThrCtrl, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_LineFilterHor
*   Description  : A horizontal density filter task for binary images
*   Parameters   : MI_IVE_HANDLE                   hHandle               Handle ID of a task
*                  MI_IVE_SrcImage_t              *pstSrcDst             Input and output source.
*                  MVE_IVE_LineFilterHorCtrl_t    *pstLineFilterHorCtrl  Control parameters.
*                  MI_BOOL                         bInstant              Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_LineFilterHor(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrcDst,
    MVE_IVE_LineFilterHorCtrl_t *pstLineFilterHorCtrl, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_LineFilterVer
*   Description  : A vertical density filter task for binary images.
*   Parameters   : MI_IVE_HANDLE                   hHandle               Handle ID of a task
*                  MI_IVE_SrcImage_t              *pstSrcDst             Input and output source.
*                  MVE_IVE_LineFilterVerCtrl_t    *pstLineFilterVerCtrl  Control parameters.
*                  MI_BOOL                         bInstant              Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_LineFilterVer(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrcDst,
    MVE_IVE_LineFilterVerCtrl_t *pstLineFilterVerCtrl, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_NoiseRemoveHor
*   Description  : A horizontal noise removal task for binary images.
*   Parameters   : MI_IVE_HANDLE                  hHandle                Handle ID of a task
*                  MI_IVE_SrcImage_t             *pstSrcDst              Input and output source.
*                  MVE_IVE_NoiseRemoveHorCtrl_t  *pstNoiseRemoveHorCtrl  Control parameters.
*                  MI_BOOL                        bInstant               Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_NoiseRemoveHor(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrcDst,
    MVE_IVE_NoiseRemoveHorCtrl_t *pstNoiseRemoveHorCtrl, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_NoiseRemoveVer
*   Description  : A vertical noise removal task for binary images.
*   Parameters   : MI_IVE_HANDLE                  hHandle                Handle ID of a task
*                  MI_IVE_SrcImage_t             *pstSrcDst              Input and output source.
*                  MVE_IVE_NoiseRemoveVerCtrl_t  *pstNoiseRemoveVerCtrl  Control parameters.
*                  MI_BOOL                        bInstant               Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_NoiseRemoveVer(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrcDst,
    MVE_IVE_NoiseRemoveVerCtrl_t *pstNoiseRemoveVerCtrl, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_Acc
*   Description  : An accumulation task for two gray-scale images.
*   Parameters   : MI_IVE_HANDLE       hHandle     Handle ID of a task
*                  MI_IVE_SrcImage_t  *pstSrc0     First input source.
*                  MI_IVE_SrcImage_t  *pstSrc1     Second input source.
*                  MVE_IVE_AccCtrl_t  *pstAccCtrl  Control parameters.
*                  MI_BOOL             bInstant    Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_Acc(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrc0,
    MI_IVE_SrcImage_t *pstSrc1, MI_IVE_DstImage_t *pstDst, MVE_IVE_AccCtrl_t *pstAccCtrl, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_BAT
*   Description  : Execute the horizontal or vertical alternating time for binary images.
*   Parameters   : MI_IVE_HANDLE         hHandle   Handle ID of a task
*                  MI_IVE_SrcImage_t    *pstSrc    Input source.
*                  MI_IVE_DstMemInfo_t  *pstDstH   First output.
*                  MI_IVE_DstMemInfo_t  *pstDstV   Second output.
*                  MVE_IVE_BatCtrl_t    *pstCtrl   Control parameters.
*                  MI_BOOL               bInstant  Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_BAT(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrc,
    MI_IVE_DstMemInfo_t *pstDstH, MI_IVE_DstMemInfo_t *pstDstV, MVE_IVE_BatCtrl_t *pstCtrl, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_Matrix_Transform
*   Description  : Matrix transform operation.
*   Parameters   : MI_IVE_HANDLE           hHandle           Handle ID of a task
*                  MI_IVE_SrcImage_t      *pstSrc1           Input source 1.
*                  MI_IVE_SrcImage_t      *pstSrc2           Input source 2.
*                  MI_IVE_SrcImage_t      *pstSrc3           Input source 3.
*                  MI_IVE_DstImage_t      *pstDst1           Output result 1
*                  MI_IVE_DstImage_t      *pstDst2           Output result 2
*                  MI_IVE_DstImage_t      *pstDst3           Output result 3
*                  MI_IVE_MatrTranfCtrl_t *pstMatrTranfCtrl  Control parameter
*                  MI_BOOL                 bInstant          Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_Matrix_Transform(MI_IVE_HANDLE hHandle,
    MI_IVE_SrcImage_t *pstSrc1, MI_IVE_SrcImage_t *pstSrc2, MI_IVE_SrcImage_t *pstSrc3,
    MI_IVE_DstImage_t *pstDst1, MI_IVE_DstImage_t *pstDst2, MI_IVE_DstImage_t *pstDst3,
    MI_IVE_MatrTranfCtrl_t *pstMatrTranfCtrl, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_Image_Dot
*   Description  : Execute the operation of dot product.
*   Parameters   : MI_IVE_HANDLE          hHandle      Handle ID of a task
*                  MI_IVE_SrcImage_t     *pstSrc1      Input source 1.
*                  MI_IVE_SrcImage_t     *pstSrc2      Input source 2.
*                  MI_IVE_DstImage_t     *pstDst       Output result.
*                  MI_BOOL                bInstant     Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_Image_Dot(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrc1, MI_IVE_SrcImage_t *pstSrc2,
    MI_IVE_DstImage_t *pstDst, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_AlphaBlending
*   Description  : Execute an independent weighted addition calculation task against two grayscale images.
*   Parameters   : MI_IVE_HANDLE          hHandle      Handle ID of a task
*                  MI_IVE_SrcImage_t     *pstSrc1      Input source 1.
*                  MI_IVE_SrcImage_t     *pstSrc2      Input source 2.
*                  MI_IVE_SrcImage_t     *pstAlpha     Input alpha table.
*                  MI_IVE_DstImage_t     *pstDst1      Output result
*                  MI_BOOL                bInstant     Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_AlphaBlending(MI_IVE_HANDLE hHandle,
     MI_IVE_SrcImage_t *pstSrc1, MI_IVE_SrcImage_t *pstSrc2, MI_IVE_SrcImage_t *pstAlpha,
     MI_IVE_DstImage_t *pstDst, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_Shift_Detector
*   Description  : Execute the operation of object tracking.
*   Parameters   : MI_IVE_HANDLE               hHandle   Handle ID of a task
*                  MI_IVE_SrcImage_t          *pstSrc1   Input source 1.
*                  MI_IVE_SrcImage_t          *pstSrc2   Input source 2.
*                  MI_IVE_DstImage_t          *pstDstX   Output result 1.
*                  MI_IVE_DstImage_t          *pstDstY   Output result 2.
*                  MI_IVE_SHIFT_DETECT_CTRL_t *pstCtrl   Control parameters.
*                  MI_BOOL                     bInstant  Reserved
*   Return Value : MI_SUCCESS: Success;Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_Shift_Detector(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrc1, MI_IVE_SrcImage_t *pstSrc2, MI_IVE_DstImage_t *pstDstX, MI_IVE_DstImage_t *pstDstY, MI_IVE_SHIFT_DETECT_CTRL_t *pstCtrl, MI_BOOL bInstant);

/*****************************************************************************
*   Prototype    : MI_IVE_BGBlur
*   Description  : Used to do background blur/mosaic/blur+mosaic on the basis of input mask of Y/UV component.
*   Parameters   : MI_IVE_HANDLE          hHandle         Handle ID of a task.
*                  MI_IVE_SrcImage_t     *pstSrcYMask     Input mask of Y component.
*                  MI_IVE_SrcImage_t     *pstSrcUvMask    Input mask of UV component.
*                  MI_IVE_SrcImage_t     *pstSrcOri       Input original image.
*                  MI_IVE_SrcImage_t     *pstSrcRepBg     Input small original image.
*                  MI_IVE_DstImage_t     *pstDst          Output result.
*                  MI_IVE_BgBlurCtrl_t   *pstCtrl         Control parameters.
*   Return Value : MI_SUCCESS: Success;   Error codes:    Failure.
*****************************************************************************/
MI_S32 MI_IVE_BGBlur(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrcYMask, MI_IVE_SrcImage_t *pstSrcUvMask, MI_IVE_SrcImage_t *pstSrcOri, MI_IVE_SrcImage_t *pstSrcRepBg, MI_IVE_DstImage_t *pstDst, MI_IVE_BgBlurCtrl_t *pstCtrl);

/*****************************************************************************
*   Prototype    : MI_IVE_MAU
*   Description  : Used to do matrix arithmetic calculation.
*   Parameters   : MI_IVE_HANDLE                hHandle        Handle ID of a task.
*                  MI_IVE_SrcImage_t           *pstLeftMat     Left input matrix.
*                  MI_IVE_DstImage_t           *pstRightMat    Right input matrix.
*                  MI_IVE_SrcImage_t           *pstLeftIdxMat  Left index matrix.
*                  MI_IVE_SrcImage_t           *pstRightIdxMat Right index matrix.
*                  MI_IVE_DstImage_t           *pstDst         The output of left matrix*right matrix.
*                  MI_IVE_DstImage_t           *pstSortMat     The sorted output of left matrix*right matrix.
*                  MI_IVE_DstImage_t           *pstSortIdxMat  The sort index of output.
*                  MI_IVE_MAUCtrl_t            *pstCtrl        Control parameters.
*   Return Value : MI_SUCCESS: Success;         Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_MAU(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstLeftMat, MI_IVE_SrcImage_t *pstRightMat, MI_IVE_SrcImage_t *pstLeftIdxMat,
                    MI_IVE_SrcImage_t *pstRightIdxMat, MI_IVE_DstImage_t *pstDst, MI_IVE_DstImage_t *pstSortMat, MI_IVE_DstImage_t *pstSortIdxMat,
                    MI_IVE_MAUCtrl_t *pstCtrl);

/*****************************************************************************
*   Prototype    : MI_IVE_LkOpticalFlowPyr
*   Description  : Calculate LK-Optical Flow in multi-layer of the pyramid.
*   Parameters   : MI_IVE_HANDLE                hHandle          Handle ID of a task.
*                  MI_IVE_SrcImage_t           *pstSrcPre        Previous input image.
*                  MI_IVE_SrcImage_t           *pstSrcCur        Current input image.
*                  MI_IVE_SrcMemInfo_t         *pstCornerDataPre The corner data of the previous image.
*                  MI_IVE_MemInfo_t            *pstCornerDataCur The init flow and output corner data of the current image.
*                  MI_IVE_DstMemInfo_t         *pstStatus        The trace result.
*                  MI_IVE_DstMemInfo_t         *pstErr           The trace error.
*                  MI_IVE_LkOPFPyrCtrl_t       *pstCtrl          Control parameters.
*   Return Value : MI_SUCCESS: Success;         Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_LkOpticalFlowPyr(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrcPre, MI_IVE_SrcImage_t *pstSrcCur, MI_IVE_SrcMemInfo_t *pstCornerDataPre,
                               MI_IVE_MemInfo_t *pstCornerDataCur, MI_IVE_DstMemInfo_t *pstStatus, MI_IVE_DstMemInfo_t *pstErr, MI_IVE_LkOPFPyrCtrl_t *pstCtrl);

/*****************************************************************************
*   Prototype    : MI_IVE_FAST
*   Description  : Used to do FAST algo and get corner data.
*   Parameters   : MI_IVE_HANDLE                hHandle       Handle ID of a task.
*                  MI_IVE_SrcImage_t           *pstSrc        Input source image.
*                  MI_IVE_MemInfo_t            *pstCornerData Output corner data.
*                  MI_IVE_MemInfo_t            *pstScore      The score value of the corner data.
*                  MI_IVE_FASTCtrl_t           *pstCtrl       Control parameters.
*   Return Value : MI_SUCCESS: Success;         Error codes: Failure.
*****************************************************************************/
MI_S32 MI_IVE_FAST(MI_IVE_HANDLE hHandle, MI_IVE_SrcImage_t *pstSrc, MI_IVE_MemInfo_t *pstCornerData, MI_IVE_MemInfo_t *pstScore, MI_IVE_FASTCtrl_t *pstCtrl);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif/*_MI_IVE_H_*/
