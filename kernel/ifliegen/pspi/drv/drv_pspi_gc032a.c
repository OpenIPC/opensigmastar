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

#include <drv_pspi_dev.h>
#include <drv_pspi_dev_init.h>

#define PSPI_SNR_IIC_BUS   0
#define PSPI_SNR_IIC_FMT   IIC_FMT_A8D8
#define PSPI_SNR_IIC_ADDR  0x6E
#define PSPI_SNR_IIC_SPEED 200000
#define PSPI_SNR_MCLK_FREQ 24000000
#define PSPI_SNR_DATA_PREC PSPI_DATAPRECISION_8

static struct {
    struct _senout{
        s32 width, height, min_fps, max_fps;
    }senout;

    struct _senstr{
        const char* strResDesc;
    }senstr;

}gc032a_linear[] = {
    { {240, 320, 0, 30}, {"240x320@30fps"}},
};

typedef struct {
    const IIC_ARRAY_t *pTableHDR;
    const IIC_ARRAY_t *pTableLinear;
    u32              u32ParamSize;
} SensorParams;

const static IIC_ARRAY_t PSPI_SENSOR_InitTable[] =
{
    {0x12, 0x80},
    {0x11, 0xb0},
    {0x1b, 0x80},
    {0x6b, 0x01},
    {0x12, 0x20},
    {0x3a, 0x00},
    {0x15, 0x10},
    {0x62, 0x82},
    {0x08, 0xa0},
    {0x06, 0x68},
    {0x2b, 0x20},
    {0x27, 0x97},
    {0x17, 0x01},
    {0x18, 0x79},
    {0x19, 0x00},
    {0x1a, 0xa0},
    {0x03, 0x00},
    {0x13, 0x00},
    {0x01, 0x13},
    {0x02, 0x20},
    {0x87, 0x16},
    {0x8c, 0x01},
    {0x8d, 0xcc},
    {0x13, 0x07},
    {0x33, 0x10},
    {0x34, 0x1d},
    {0x35, 0x46},
    {0x36, 0x40},
    {0x37, 0xa4},
    {0x38, 0x7c},
    {0x65, 0x46},
    {0x66, 0x46},
    {0x6e, 0x20},
    {0x9b, 0xa4},
    {0x9c, 0x7c},
    {0xbc, 0x0c},
    {0xbd, 0xa4},
    {0xbe, 0x7c},
    {0x20, 0x09},
    {0x09, 0x03},
    {0x72, 0x2f},
    {0x73, 0x2f},
    {0x74, 0xa7},
    {0x75, 0x12},
    {0x79, 0x8d},
    {0x7a, 0x00},
    {0x7e, 0xfa},
    {0x70, 0x0f},
    {0x7c, 0x84},
    {0x7d, 0xba},
    {0x5b, 0xc2},
    {0x76, 0x90},
    {0x7b, 0x55},
    {0x71, 0x46},
    {0x77, 0xdd},
    {0x13, 0x0f},
    {0x8a, 0x10},
    {0x8b, 0x20},
    {0x8e, 0x21},
    {0x8f, 0x40},
    {0x94, 0x41},
    {0x95, 0x7e},
    {0x96, 0x7f},
    {0x97, 0xf3},
    {0x13, 0x07},
    {0x24, 0x58},
    {0x97, 0x48},
    {0x25, 0x08},
    {0x94, 0xb5},
    {0x95, 0xc0},
    {0x80, 0xf6},
    {0x81, 0xe0},
    {0x82, 0x1b},
    {0x83, 0x37},
    {0x84, 0x39},
    {0x85, 0x58},
    {0x86, 0x77},
    {0x89, 0x74},
    {0x8a, 0x5c},
    {0x8b, 0x4c},
    {0x39, 0x98},
    {0x3f, 0x98},
    {0x90, 0xa0},
    {0x91, 0xe0},
    {0x40, 0x20},
    {0x41, 0x28},
    {0x42, 0x26},
    {0x43, 0x25},
    {0x44, 0x1f},
    {0x45, 0x1a},
    {0x46, 0x16},
    {0x47, 0x12},
    {0x48, 0x0f},
    {0x49, 0x0d},
    {0x4b, 0x0b},
    {0x4c, 0x0a},
    {0x4e, 0x08},
    {0x4f, 0x06},
    {0x50, 0x06},
    {0x5a, 0x56},
    {0x51, 0x1b},
    {0x52, 0x04},
    {0x53, 0x4a},
    {0x54, 0x26},
    {0x57, 0x75},
    {0x58, 0x2b},
    {0x5a, 0xd6},
    {0x51, 0x28},
    {0x52, 0x1e},
    {0x53, 0x9e},
    {0x54, 0x70},
    {0x57, 0x50},
    {0x58, 0x07},
    {0x5c, 0x28},
    {0xb0, 0xe0},
    {0xb1, 0xc0},
    {0xb2, 0xb0},
    {0xb3, 0x4f},
    {0xb4, 0x63},
    {0xb4, 0xe3},
    {0xb1, 0xf0},
    {0xb2, 0xa0},
    {0x55, 0x00},
    {0x56, 0x40},
    {0x96, 0x50},
    {0x9a, 0x30},
    {0x6a, 0x81},
    {0x23, 0x33},
    {0xa0, 0xd0},
    {0xa1, 0x31},
    {0xa6, 0x04},
    {0xa2, 0x0f},
    {0xa3, 0x2b},
    {0xa4, 0x0f},
    {0xa5, 0x2b},
    {0xa7, 0x9a},
    {0xa8, 0x1c},
    {0xa9, 0x11},
    {0xaa, 0x16},
    {0xab, 0x16},
    {0xac, 0x3c},
    {0xad, 0xf0},
    {0xae, 0x57},
    {0xc6, 0xaa},
    {0xd2, 0x78},
    {0xd0, 0xb4},
    {0xd1, 0x00},
    {0xc8, 0x10},
    {0xc9, 0x12},
    {0xd3, 0x09},
    {0xd4, 0x2a},
    {0xee, 0x4c},
    {0x7e, 0xfa},
    {0x74, 0xa7},
    {0x78, 0x4e},
    {0x60, 0xe7},
    {0x61, 0xc8},
    {0x6d, 0x70},
    {0x1e, 0x39},
    {0x98, 0x1a},
    {0x69, 0x20},
    {0xf1, 0x28},
    {0x13, 0x05},
    {0x6a, 0x81},
    {0x23, 0x00},
    {0x01, 0x08},
    {0x02, 0x08},
};

static int PSPI_SENSOR_Init(PSPI_SNR_CFG_t *PspiSnrCfg)
{
    u32 i      = 0;
    int s32Ret = 0;

    for(i = 0; i < ARRAY_SIZE(PSPI_SENSOR_InitTable); i++)
    {
        if(PSPI_SENSOR_InitTable[i].u16Reg==0xffff)
        {
            CamOsUsSleep(PSPI_SENSOR_InitTable[i].u16Data);
        }
        else
        {
            s32Ret = PspiSnrCfg->pstI2cCtrl->IIC_TX(PspiSnrCfg->pstI2cCtrl, &PspiSnrCfg->stI2cCfg, PSPI_SENSOR_InitTable[i].u16Reg, PSPI_SENSOR_InitTable[i].u16Data);
            if (s32Ret)
            {
                CamOsPrintf("[%s] init error\n", __FUNCTION__);
                return s32Ret;
            }
        }
    }

    return 0;
}
static int PSPI_SENSOR_Release(PSPI_SNR_CFG_t *PspiSnrCfg)
{
    return 0;
}

int PSPI_SENSOR_InitHandleLinear(PSPI_SNR_CFG_t* PspiSnrCfg)
{
    int             res     = 0 ;
    SensorParams   *pParams = NULL;
    PSPI_SNR_CFG_t *pCfg    = PspiSnrCfg;

    if (!pCfg)
    {
        CamOsPrintf("[%s] Fail to get pspi sensor configuration\n", __FUNCTION__);
        return -1;
    }

    if (pCfg->pPrivData == NULL)
    {
        CamOsPrintf("[%s] Fail to get pspi sensor data\n", __FUNCTION__);
        return -1;
    }

    pCfg->stI2cCfg.u8Bus        = PSPI_SNR_IIC_BUS;
    pCfg->stI2cCfg.eFmt         = PSPI_SNR_IIC_FMT;
    pCfg->stI2cCfg.u16SlaveAddr = PSPI_SNR_IIC_ADDR;
    pCfg->stI2cCfg.u32Speed     = PSPI_SNR_IIC_SPEED;

    pCfg->u32MclkFreq = PSPI_SNR_MCLK_FREQ;
    pCfg->eDataPrec   = PSPI_SNR_DATA_PREC;
    pCfg->eDataFmt    = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;

    pCfg->stPspiSnrAttr.u16MaxFps        = gc032a_linear[res].senout.max_fps;
    pCfg->stPspiSnrAttr.u16MinFps        = gc032a_linear[res].senout.min_fps;
    pCfg->stPspiSnrAttr.u16OutputWidth   = gc032a_linear[res].senout.width;
    pCfg->stPspiSnrAttr.u16OutputHeight  = gc032a_linear[res].senout.height;
    CamOsStrncpy(pCfg->stPspiSnrAttr.au8StrResDesc, gc032a_linear[res].senstr.strResDesc,CamOsStrlen(gc032a_linear[res].senstr.strResDesc));

    pCfg->PSPI_SNR_INIT        = PSPI_SENSOR_Init;
    pCfg->PSPI_SNR_RELEASE     = PSPI_SENSOR_Release;
    pCfg->PSPI_SNR_POWER_ON    = NULL;
    pCfg->PSPI_SNR_POWER_OFF   = NULL;
    pCfg->PSPI_SNR_GET_FPS     = NULL;
    pCfg->PSPI_SNR_SET_FPS     = NULL;
    pCfg->PSPI_SNR_GET_AEUSECS = NULL;
    pCfg->PSPI_SNR_SET_AEUSECS = NULL;

    pParams               = (SensorParams *)pCfg->pPrivData;
    pParams->pTableLinear = PSPI_SENSOR_InitTable;
    pParams->u32ParamSize = ARRAY_SIZE(PSPI_SENSOR_InitTable);

    return 0;
}


PSPI_RX_SNR_ENTRY_READY(gc032a);


PSPI_RX_SNR_ENTRY_STAGE(gc032a, PSPI_SENSOR_InitHandleLinear,       SensorParams);

