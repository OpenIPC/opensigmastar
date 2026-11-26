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
#include "drv_ss_cus_sensor.h"
#include "drv_ss_cus_vcm.h"
#include <cam_os_wrapper.h>

// mi_sensor_internal
struct mi_sensor_internal_apis_t;
extern struct mi_sensor_internal_apis_t g_mi_sensor_internal_apis;
#ifdef CONFIG_ENABLE_SUPPORT_USER_SPACE_SENSOR_DRIVER
struct SnrMiscFuncCtrl;
extern struct SnrMiscFuncCtrl gSnrMiscFuncCtrl;
#endif

s32 DRV_SENSOR_IfRegisterSensorI2CSlaveId(u32 nSNRPadID, u32 Slaveid);
s32 DRV_SENSOR_IfRegisterPlaneI2CSlaveId(u32 nSNRPadID, u32 nPlaneID, u32 Slaveid);
s32 DRV_SENSOR_IfSensorHandleVer(u32 version_major, u32 version_minor);
s32 DRV_SENSOR_IfSensorIFVer(u32 version_major, u32 version_minor);
s32 DRV_SENSOR_IfSensorI2CVer(u32 version_major, u32 version_minor);
s32 DRV_SENSOR_IfRegisterSensorDriver(u32 nSNRPadID, SensorInitHandle pfnSensorInitHandle);
s32 DRV_SENSOR_IfRegisterPlaneDriver(u32 nSNRPadID, u32 nPlaneID, SensorInitHandle pfnSensorInitHandle);
s32 DRV_SENSOR_IfRegisterSensorDriverEx(u32 nSNRPadID, SensorInitHandle pfnSensorInitHandle, void *pPrivateData);
s32 DRV_SENSOR_IfRegisterPlaneDriverEx(u32 nSNRPadID, u32 nPlaneID, SensorInitHandle pfnSensorInitHandle);
s32 DRV_SENSOR_IfRelease(u32 nSNRPadID);
u64 EXT_log_2(u32 value);
u64 intlog10(u32 value);
u32 round_float(u32 x, u32 piont_offset);
u64 base2_exp_float_pow(u64 x);
s32 DRV_SENSOR_IfRegisterVcmDriver(u32 nSNRPadID, VcmInitHandle pfnVcmInitHandle);
s32 DRV_SENSOR_IfReleaseVcmDriver(u32 nCamID);

#ifndef COMBINE_MI_MODULE
EXPORT_SYMBOL(g_mi_sensor_internal_apis);
#endif
EXPORT_SYMBOL(DRV_SENSOR_IfRegisterSensorI2CSlaveId);     //called by sensor drvier
EXPORT_SYMBOL(DRV_SENSOR_IfRegisterPlaneI2CSlaveId);
EXPORT_SYMBOL(DRV_SENSOR_IfSensorHandleVer);
EXPORT_SYMBOL(DRV_SENSOR_IfSensorIFVer);
EXPORT_SYMBOL(DRV_SENSOR_IfSensorI2CVer);
EXPORT_SYMBOL(DRV_SENSOR_IfRegisterSensorDriver);
EXPORT_SYMBOL(DRV_SENSOR_IfRegisterPlaneDriver);
EXPORT_SYMBOL(DRV_SENSOR_IfRegisterSensorDriverEx);
EXPORT_SYMBOL(DRV_SENSOR_IfRegisterPlaneDriverEx);
EXPORT_SYMBOL(DRV_SENSOR_IfRelease);

EXPORT_SYMBOL(EXT_log_2);
EXPORT_SYMBOL(intlog10);
EXPORT_SYMBOL(round_float);
EXPORT_SYMBOL(base2_exp_float_pow);
EXPORT_SYMBOL(DRV_SENSOR_IfRegisterVcmDriver);
EXPORT_SYMBOL(DRV_SENSOR_IfReleaseVcmDriver);
#ifdef CONFIG_ENABLE_SUPPORT_USER_SPACE_SENSOR_DRIVER
EXPORT_SYMBOL(gSnrMiscFuncCtrl);
#endif
