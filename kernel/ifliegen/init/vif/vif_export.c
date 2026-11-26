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
// #include "drv_ms_isp_general_vif.h"

struct mi_vif_InternalApis_t;
// mi_sys_internal

// s32 DrvVifIsp3aStatisticConfig(void* handle, u8 sel, ISP_Vif3aStatData_t Cus3aStatsInfo, u32 nVifGroup){return -1;};
extern struct mi_vif_InternalApis_t g_miVifInternalApis;
EXPORT_SYMBOL(g_miVifInternalApis);
// EXPORT_SYMBOL(DrvVifIsp3aStatisticConfig);


