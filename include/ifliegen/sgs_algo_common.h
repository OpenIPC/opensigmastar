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

#ifndef __SGS_ALGO_COMMON_H__
#define __SGS_ALGO_COMMON_H__

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    E_PD_PERSON = 0
} Label_PD_Person_e;

typedef enum
{
    E_FD_FACE = 0,
} Label_FD_Face_e;

typedef enum
{
    E_PCN_PERSON = 0,
    E_PCN_BICYCLE,
    E_PCN_CAR,
    E_PCN_MOTOCYCLE,
    E_PCN_BUS,
    E_PCN_TRUCK
} Label_PCN_e;

typedef enum
{
    E_PCD_PERSON,
    E_PCD_CAT,
    E_PCD_DOG
} Label_PCD_e;


typedef enum
{
    E_FSD_FIRE,
    E_FSD_SMOKE
} Label_FSD_e;

typedef enum
{
    E_SD_PERSON = 0,
    E_SD_BICYCLE,
    E_SD_CAR,
    E_SD_MOTOCYCLE,
    E_SD_BUS,
    E_SD_TRUCK,
    E_SD_CAT,
    E_SD_DOG,
    E_SD_HEAD,
    E_SD_FACE
} LABEL_SD_e;


typedef enum
{
    E_SPD_PERSON = 0,
    E_SPD_HEAD,
    E_SPD_FACE
} LABEL_SPD_e;

typedef enum
{
    E_BD_BAG = 0,
} LABEL_BD_e;


#ifdef __cplusplus
}
#endif

#endif