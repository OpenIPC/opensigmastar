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

#include "cam_sysfs.h"
#include "mi_common.h"
#include "mi_common_internal.h"
#include "common_print.h"

DECLEAR_MODULE_INIT_EXIT

MI_MODULE_LICENSE();
MODULE_AUTHOR("Sgs");
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
MODULE_IMPORT_NS(ANDROID_GKI_VFS_EXPORT_ONLY);
#endif
