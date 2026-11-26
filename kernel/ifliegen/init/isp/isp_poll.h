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
#ifndef _ISP_POLL_H_
#define _ISP_POLL_H_

typedef enum
{
    E_ISP_POLL_EVENT_FRAME,
    E_ISP_POLL_EVENT_REOPEN,
    E_ISP_POLL_EVENT_RESUME,
    E_ISP_POLL_EVENT_ALGO,
    E_ISP_POLL_EVENT_DBG,
    E_ISP_POLL_EVENT_MAX,
}ISP_PollEventType_e;

int DRV_ISP_PollDeinit(u32 dev);
int DRV_ISP_PollInit(u32 dev);
int DRV_ISP_PollTriggerEvent(u32 dev, u32 chn, u32 eventType, u32 data, u8 enable);

#endif//_ISP_POLL_H_