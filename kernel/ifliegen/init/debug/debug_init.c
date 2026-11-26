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

#include <linux/module.h>
#include <linux/mm.h>
#include "mi_common.h"
#include "mi_common_internal.h"

MI_U32 MI_Debug_FreeReservedArea(ss_phys_addr_t start, ss_phys_addr_t end, int poison, char *s)
{
    if (!pfn_valid(__phys_to_pfn(start)))
    {
        CamOsPrintf("free reserved fail, it maybe caused by mma_memblock_remove=1 in bootargs\n");
        return -1;
    }

    return free_reserved_area(__va(start), __va(end), poison, s);
}

DECLEAR_MODULE_INIT_EXIT
MODULE_LICENSE("Proprietary");
MODULE_AUTHOR("Sgs");
