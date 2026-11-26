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

#ifndef _DRV_PSPI_DEV_INIT_H_
#define _DRV_PSPI_DEV_INIT_H_

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <drv_pspi_dev.h>

#ifdef SUPPORT_GPL_SYMBOL
#define PSPI_MODULE_LICENSE() MODULE_LICENSE("GPL v2");
#else
#define PSPI_MODULE_LICENSE() MODULE_LICENSE("Proprietary");
#endif

#define PSPI_TX_DEV_ENTRY_READY
#define PSPI_TX_DEV_ENTRY_STAGE

#define PSPI_RX_SNR_ENTRY_READY(Name) \
ushort pspi_id = 0;\
module_param(pspi_id, ushort, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);\
MODULE_PARM_DESC(pspi_id, "pspi bus");\
ushort pspi_slave = 0;\
module_param(pspi_slave, ushort, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);\
MODULE_PARM_DESC(pspi_slave, "sensor slave address");

#define PSPI_RX_SNR_ENTRY_STAGE(Name,LinearEntry,PrivateDataType) \
static PrivateDataType* g_pData[PSPI_DEV_MAX] = {NULL};\
static int __init Name##_init_driver(void)\
{\
    int i = 0;\
    void* p0 = LinearEntry;\
    for(i = 0; i < PSPI_DEV_MAX; ++i)\
    {\
        if(pspi_id == i)\
        {\
            if(p0)\
            {\
                void* pData = CamOsMemAlloc(sizeof(PrivateDataType));\
                CamOsMemset(pData,0,sizeof(PrivateDataType));\
                DRV_PSPI_SENSOR_Register(i, LinearEntry, pData);\
                DRV_PSPI_SENSOR_BindingSlaveID(i, pspi_slave&0xFF);\
                g_pData[i] = pData;\
            }\
        }\
    }\
    return 0;\
}\
static void __exit Name##_exit_driver(void)\
{\
    int i = 0;\
    for(i = 0; i < PSPI_DEV_MAX; ++i)\
    {\
        if(pspi_id == i)\
        {\
            if(g_pData[i])\
            {\
                DRV_PSPI_SENSOR_UnRegister(i);\
                CamOsMemRelease((void*)g_pData[i]);\
                g_pData[i] = NULL;\
            }\
        }\
    }\
}\
module_init(Name##_init_driver);\
module_exit(Name##_exit_driver);\
MODULE_DESCRIPTION("pspi_device_"#Name);\
MODULE_AUTHOR("Sgs");\
PSPI_MODULE_LICENSE()

#endif

