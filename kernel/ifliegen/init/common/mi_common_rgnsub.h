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
#ifndef _REGIONSUBSTRACT_H_
#define _REGIONSUBSTRACT_H_

#include "cam_os_wrapper.h"
#include "mi_common_datatype.h"

typedef int           BOOL;
typedef unsigned char BYTE;
typedef void *        PBLOCKHEAP;

typedef struct MI_COMMON_RGNSUB_Handle_s
{
    CamOsMemCache_t rgnsubCache;
} MI_COMMON_RGNSUB_Handle_t;

typedef struct MI_COMMON_RGNSUB_Rect_s
{
    MI_S32 left;
    MI_S32 top;
    MI_S32 right;
    MI_S32 bottom;
} MI_COMMON_RGNSUB_Rect_t;

typedef MI_COMMON_RGNSUB_Rect_t *PRECT;

typedef struct MI_COMMON_RGNSUB_CliPrect_s
{
    MI_COMMON_RGNSUB_Rect_t             rc;
    struct MI_COMMON_RGNSUB_CliPrect_s *next;
    struct MI_COMMON_RGNSUB_CliPrect_s *prev;
} MI_COMMON_RGNSUB_CliPrect_t;

typedef MI_COMMON_RGNSUB_CliPrect_t *PCLIPRECT;

#define NULLREGION    0x00
#define SIMPLEREGION  0x01
#define COMPLEXREGION 0x02

typedef struct MI_COMMON_RGNSUB_CliPrgn_s
{
    BYTE                    type;
    BYTE                    reserved[3];
    MI_COMMON_RGNSUB_Rect_t rcBound;
    PCLIPRECT               head;
    PCLIPRECT               tail;
    PBLOCKHEAP              heap;
} MI_COMMON_RGNSUB_CliPrgn_t;

typedef MI_COMMON_RGNSUB_CliPrgn_t *PCLIPRGN;

#define region_for_each_rect(rect, rgn)                                                       \
    for (rect = ((rgn)->head) ? (&((rgn)->head->rc)) : NULL; rect;                            \
         rect = (CAM_OS_CONTAINER_OF(rect, MI_COMMON_RGNSUB_CliPrect_t, rc)->next)            \
                    ? &(CAM_OS_CONTAINER_OF(rect, MI_COMMON_RGNSUB_CliPrect_t, rc)->next->rc) \
                    : NULL)

/**
 * Dumps the region for debugging purposes.
 *
 * This function is used to debug the contents of a region.
 *
 * @param region The region to be dumped
 */
void MI_COMMON_RGNSUB_DbgDumpRgn(MI_COMMON_RGNSUB_CliPrgn_t *region);

/**
 * Checks if a clipping region is empty.
 *
 * Determines whether the specified clipping region contains no rectangles.
 *
 * @param pRgn The clipping region to check
 * @return TRUE if the region is empty, FALSE otherwise
 */
BOOL MI_COMMON_RGNSUB_IsEmptyClipRgn(const MI_COMMON_RGNSUB_CliPrgn_t *pRgn);

/**
 * Initializes a clipping region.
 *
 * Prepares the clipping region for use by initializing its internal state and associating it with a memory heap.
 *
 * @param pRgn The clipping region to initialize
 * @param heap The memory heap to be used by the region
 */
void MI_COMMON_RGNSUB_InitClipRgn(PCLIPRGN pRgn, PBLOCKHEAP heap);

/**
 * Empties a clipping region.
 *
 * Clears all rectangles from the specified clipping region, releasing any associated resources.
 *
 * @param handle The handle to the region subsystem
 * @param pRgn The clipping region to be emptied
 */
void MI_COMMON_RGNSUB_EmptyClipRgn(MI_COMMON_RGNSUB_Handle_t *handle, PCLIPRGN pRgn);

/**
 * Adds a rectangle to a clipping region.
 *
 * Appends the specified rectangle to the list of rectangles in the clipping region.
 *
 * @param handle The handle to the region subsystem
 * @param region The clipping region to which the rectangle will be added
 * @param rect The rectangle to be added to the region
 * @return TRUE if the rectangle was successfully added, FALSE otherwise
 */
BOOL MI_COMMON_RGNSUB_AddClipRect(MI_COMMON_RGNSUB_Handle_t *handle, PCLIPRGN region,
                                  const MI_COMMON_RGNSUB_Rect_t *rect);

/**
 * Subtracts one region from another.
 *
 * Performs region subtraction where the result is stored in the destination region.
 *
 * @param handle The handle to the region subsystem
 * @param rgnD The destination region where the result will be stored
 * @param rgnM The region to subtract from
 * @param rgnS The region being subtracted
 * @return TRUE if the subtraction was successful, FALSE otherwise
 */
BOOL MI_COMMON_RGNSUB_SubtractRgn(MI_COMMON_RGNSUB_Handle_t *handle, MI_COMMON_RGNSUB_CliPrgn_t *rgnD,
                                  const MI_COMMON_RGNSUB_CliPrgn_t *rgnM, const MI_COMMON_RGNSUB_CliPrgn_t *rgnS);

/**
 * Initializes the region subsystem.
 *
 * Performs any necessary initialization for the region subsystem.
 *
 * @param handle The handle to the region subsystem
 * @return 0 on success, non-zero on failure
 */
int MI_COMMON_RGNSUB_InitRgn(MI_COMMON_RGNSUB_Handle_t *handle);

/**
 * Deinitializes the region subsystem.
 *
 * Releases any resources and performs cleanup for the region subsystem.
 *
 * @param handle The handle to the region subsystem
 * @return 0 on success, non-zero on failure
 */
int MI_COMMON_RGNSUB_DeinitRgn(MI_COMMON_RGNSUB_Handle_t *handle);

#endif
