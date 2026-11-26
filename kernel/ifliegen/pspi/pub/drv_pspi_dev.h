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

#ifndef _DRV_PSPI_DEV_H_
#define _DRV_PSPI_DEV_H_

#include <cam_os_wrapper.h>
#include <mi_sys_datatype.h>

#define PSPI_DEV_MAX 9

typedef enum {
    PSPI_CLK_POL_POS = 0, /**< High active */
    PSPI_CLK_POL_NEG = 1, /**< Low active */
} PSPI_CLK_POL;

typedef enum
{
    PSPI_SENSOR_MASTER_MODE = 0,
    PSPI_SENSOR_SLAVE_MODE  = 1,
}PSPI_SENSOR_MODE;

typedef enum {
    PSPI_DATAPRECISION_8  = 0, /**< raw data precision is 8bits */
    PSPI_DATAPRECISION_10 = 1, /**< raw data precision is 10bits */
    PSPI_DATAPRECISION_16 = 2, /**< raw data precision is 16bits */
    PSPI_DATAPRECISION_12 = 3, /**< raw data precision is 12bits */
    PSPI_DATAPRECISION_14 = 4, /**< raw data precision is 14bits */
} PSPI_DATAPRECISION;

typedef enum {
    IIC_FMT_A8D8,   /**< 8 bits Address, 8 bits Data */
    IIC_FMT_A16D8,  /**< 16 bits Address 8 bits Data */
    IIC_FMT_A8D16,  /**< 8 bits Address 16 bits Data */
    IIC_FMT_A16D16, /**< 16 bits Address 16 bits Data */
    IIC_FMT_END
} IIC_DATA_FMT;

typedef struct IIC_ARRAY{
    u16 u16Reg;
    u16 u16Data;
} IIC_ARRAY_t;

typedef struct IIC_CONFIG{
    IIC_DATA_FMT eFmt;
    u8           u8Bus;
    u32          u32Speed;
    u16          u16SlaveAddr;
}IIC_CFG_t;

typedef struct IIC_CONTROL{
    void *pData;
    u32  u32PspiID;
    int (*IIC_OPEN)(struct IIC_CONTROL* pstI2cCtrl, IIC_CFG_t *pstI2cCfg);
    int (*IIC_CLOSE)(struct IIC_CONTROL* pstI2cCtrl);
    int (*IIC_TX)(struct IIC_CONTROL* pstI2cCtrl , IIC_CFG_t *pstI2cCfg, u16 u16Reg, u16 u16Data);
    int (*IIC_RX)(struct IIC_CONTROL* pstI2cCtrl, IIC_CFG_t *pstI2cCfg, u16 u16Reg, volatile u16 *pData);
}IIC_CTRL_t;

typedef struct PSPI_SNR_CONTROL
{
    void* pData;
    int (*MClock)(u32 u32PspiID, u8 u8OnOff, u32 u32Mclk);
    int (*PowerOff)(u32 u32PspiID, PSPI_CLK_POL ePol);
    int (*Reset)(u32 u32PspiID, PSPI_CLK_POL ePol);
    int (*SetSkipFrame)(u32 u32PspiID, u32 u32Fps, u32 u32Cnt);
}PSPI_SNR_CTRL_t;

typedef struct PSPI_SNR_ATTRIBUTE{
    u16 u16Width;
    u16 u16Height;
    u16 u16MaxFps;
    u16 u16MinFps;
    u16 u16CropStartX;
    u16 u16CropStartY;
    u16 u16OutputWidth;
    u16 u16OutputHeight;
    u16 u16MinFrameLengthLine;
    u16 u16RowTime;
    u8  au8StrResDesc[32];
}PSPI_SNR_ATTR_t;

typedef struct PSPI_SNR_CONFIG{
    IIC_CFG_t            stI2cCfg;
    IIC_CTRL_t          *pstI2cCtrl;
    void                *pPrivData;
    u32                  u32MclkFreq;
    PSPI_DATAPRECISION   eDataPrec;
    PSPI_SENSOR_MODE     eSnrMode;
    MI_SYS_PixelFormat_e eDataFmt;
    PSPI_SNR_CTRL_t     *pstPspiSnrCtrl;
    PSPI_SNR_ATTR_t      stPspiSnrAttr;

    int (*PSPI_SNR_POWER_ON)(struct PSPI_SNR_CONFIG* pstPspiCfg, u32 u32PspiID);
    int (*PSPI_SNR_POWER_OFF)(struct PSPI_SNR_CONFIG* pstPspiCfg, u32 u32PspiID);
    int (*PSPI_SNR_INIT)(struct PSPI_SNR_CONFIG* pstPspiCfg);
    int (*PSPI_SNR_RELEASE)(struct PSPI_SNR_CONFIG* pstPspiCfg);
    int (*PSPI_SNR_SUSPEND)(struct PSPI_SNR_CONFIG* pstPspiCfg);
    int (*PSPI_SNR_RESUME)(struct PSPI_SNR_CONFIG* pstPspiCfg);
    int (*PSPI_SNR_GET_AEUSECS)(struct PSPI_SNR_CONFIG* pstPspiCfg, u32 *u32Us);
    int (*PSPI_SNR_SET_AEUSECS)(struct PSPI_SNR_CONFIG* pstPspiCfg, u32 u32Us);
    int (*PSPI_SNR_GET_FPS)(struct PSPI_SNR_CONFIG* pstPspiCfg);
    int (*PSPI_SNR_SET_FPS)(struct PSPI_SNR_CONFIG* pstPspiCfg, u32 u32Fps);
}PSPI_SNR_CFG_t;

typedef int (*PspiSnrInitHandle)(PSPI_SNR_CFG_t* pstPspiCfg);

extern int  DRV_PSPI_SENSOR_Register(u32 u32PspiID, PspiSnrInitHandle pfnPspiSnrInitHandle, void *pPrivData);
extern int  DRV_PSPI_SENSOR_BindingSlaveID(u32 u32PspiID, u32 U32SlaveID);
extern void DRV_PSPI_SENSOR_UnRegister(u32 u32PspiID);

#endif

