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
#include <linux/of_platform.h>
#include <linux/of_irq.h>
#include <linux/delay.h>

#include "cam_sysfs.h"
#include "mi_common.h"
#include "mi_common_internal.h"

extern int g_eventCmd;
extern int g_cmdqtimeoutGap;

module_param(g_eventCmd, int, S_IRUGO | S_IWUSR);
module_param(g_cmdqtimeoutGap, int, S_IRUGO | S_IWUSR);

DECLEAR_MODULE_INIT_EXIT
MODULE_LICENSE("Proprietary");
MODULE_AUTHOR("Sgs");
