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
#include <linux/miscdevice.h>
#include <linux/workqueue.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <cam_os_wrapper.h>
#include "isp_poll.h"
#include <isp_cus3a_if.h>

typedef struct
{
    u32 status;
    u32 aeEn;
    u32 awbEn;
    u32 afEn;
    u32 reOpen;
    u32 resume;
    u64 dbgCmd;
    u32 mask3a[E_ALGO_TYPE_MAX]; //0:AE , 1:AWB, 2:AF, 3:user snr ae
}ISP_PollCus3aData_t;

typedef struct
{
    CamOsMutex_t lock;
    wait_queue_head_t  waitq;
    ISP_PollCus3aData_t data;
    bool ready;
    bool open;
}ISP_PollPrivate_t;

typedef struct
{
    u16 chn;
    u16 ctrl;
    u32 param;
}ISP_PollDbgParam_t;

ISP_PollPrivate_t *pg_ispPollData[1];

#define _ISP_POLL_DATA_PTR(dev) (pg_ispPollData[0])

static int _DVR_ISP_PollFeOpen(struct inode *inode, struct file *fp)
{
    ISP_PollPrivate_t *pData = _ISP_POLL_DATA_PTR(0);
    if(NULL == pData)
    {
        return -1;
    }

    init_waitqueue_head(&pData->waitq);

    pData->open = 1;

    return 0;
}

static int _DRV_ISP_PollFeRelease(struct inode *inode, struct file *fp)
{
    ISP_PollPrivate_t *pData = _ISP_POLL_DATA_PTR(0);
    pData->open = 0;
    return 0;
}

static ssize_t _DRV_ISP_PollFeRead(struct file *fp, char __user *buf, size_t size, loff_t *ppos)
{
    u32 dataSize = 0;
    ISP_PollPrivate_t *pData = _ISP_POLL_DATA_PTR(0);
    if(NULL == pData)
    {
        return -1;
    }

    CamOsMutexLock(&pData->lock);

    dataSize = sizeof(ISP_PollCus3aData_t);
    if(CamOsCopyToUpperLayer(buf, &pData->data, (size > dataSize  ? dataSize : size)))
    {
        size = 0;
    }

    pData->data.status = 0;
    pData->data.reOpen = 0;
    pData->data.resume = 0;
    pData->data.dbgCmd = 0;
    CamOsMemset(pData->data.mask3a, 0, sizeof(pData->data.mask3a));

    CamOsMutexUnlock(&pData->lock);

    return size;
}

static u32 _DRV_ISP_PollFe(struct file *fp, poll_table *wait)
{
    u32 mask = 0;
    ISP_PollPrivate_t *pData = _ISP_POLL_DATA_PTR(0);
    if(NULL == pData)
    {
        return -1;
    }

    CamOsMutexLock(&pData->lock);

    poll_wait(fp, &pData->waitq, wait);
    if(pData->ready)
    {
        pData->ready = 0;
        mask |= POLLIN | POLLRDNORM;
    }

    CamOsMutexUnlock(&pData->lock);

    return mask;
}

struct file_operations g_ispFeOps = {
    .owner = THIS_MODULE,
    .open = _DVR_ISP_PollFeOpen,
    .release = _DRV_ISP_PollFeRelease,
    .read = _DRV_ISP_PollFeRead,
    .write = 0,
    .poll = _DRV_ISP_PollFe,
};

struct miscdevice g_ispFeDev[1];

int DRV_ISP_PollInit(u32 dev)
{
    ISP_PollPrivate_t *pData = NULL;

    pData = CamOsMemAlloc(sizeof(ISP_PollPrivate_t));
    if(NULL == pData)
    {
        return -1;
    }

    CamOsMemset(pData, 0, sizeof(ISP_PollPrivate_t));

    g_ispFeDev[0].minor = MISC_DYNAMIC_MINOR;
    g_ispFeDev[0].name = "isp_fe";
    g_ispFeDev[0].fops = &g_ispFeOps;
    g_ispFeDev[0].parent = NULL;
    misc_register(&g_ispFeDev[0]);

    CamOsMutexInit(&pData->lock);

    _ISP_POLL_DATA_PTR(0) = pData;

    return 0;
}

int DRV_ISP_PollDeinit(u32 dev)
{
    ISP_PollPrivate_t *pData = _ISP_POLL_DATA_PTR(0);

    CamOsMutexDestroy(&pData->lock);

    CamOsMemRelease(pData);

    misc_deregister(&g_ispFeDev[0]);

    return 0;
}

/**
* set trigger event flag then wake up the poll
* @data: cus3a enable or not by bit,
* bit0-ae, bit1-awb, bit2-af, 0-disable, 1-enable
*/
int DRV_ISP_PollTriggerEvent(u32 dev, u32 chn, u32 eventType, u32 data, u8 enable)
{
    ISP_PollPrivate_t *pData = _ISP_POLL_DATA_PTR(dev);
    u32 n = 0;

    if (MAX_CUST_3A_DEV_NUM <= dev)
        return -1;

    if (MAX_CUST_3A_CHINFO_NUM <= chn)
        return -1;

    if(NULL == pData)
    {
        return -1;
    }

    if(0 == pData->open)
    {
        return -1;
    }

    CamOsMutexLock(&pData->lock);

    switch(eventType)
    {
        case E_ISP_POLL_EVENT_FRAME:
           pData->data.status |= 0x1 << chn;
           for(n=0; n<E_ALGO_TYPE_MAX; ++n)
                pData->data.mask3a[n] |= ((data&(0x1<<n))>>n) << chn;
        break;
        case E_ISP_POLL_EVENT_REOPEN:
            pData->data.reOpen |= 0x1 << chn;
        break;
        case E_ISP_POLL_EVENT_RESUME:
            pData->data.resume |= 0x1 << chn;
        break;
        case E_ISP_POLL_EVENT_ALGO:
            if(data & 0x1)
            {
                if (enable)
                {
                    pData->data.aeEn |= (0x1 << chn);
                }
                else
                {
                    pData->data.aeEn &= ~(0x1 << chn);
                }
            }

            if(data & 0x2)
            {
                if (enable)
                {
                    pData->data.awbEn |= (0x1 << chn);
                }
                else
                {
                    pData->data.awbEn &= ~(0x1 << chn);
                }
            }

            if(data & 0x4)
            {
                if (enable)
                {
                    pData->data.afEn |= (0x1 << chn);
                }
                else
                {
                    pData->data.afEn &= ~(0x1 << chn);
                }
            }
        break;
        case E_ISP_POLL_EVENT_DBG:
            {
                ISP_PollDbgParam_t *pDbgParam;
                pDbgParam = (ISP_PollDbgParam_t*) &pData->data.dbgCmd;
                pDbgParam->chn = chn;
                pDbgParam->ctrl = (u16)(data >> 16); // high word
                pDbgParam->param = (u32)(data & 0xffff); // low word
            }
        break;
    }

    pData->ready = 1;
    wake_up_interruptible_all(&pData->waitq);

    CamOsMutexUnlock(&pData->lock);

    return 0;
}
