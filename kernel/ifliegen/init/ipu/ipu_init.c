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

#include <linux/moduleparam.h>
#include <linux/of_platform.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/pm_opp.h>
#include <linux/platform_device.h>
#include <linux/kobject.h>

#include "cam_sysfs.h"
#include "cam_os_wrapper.h"
#include "ipu_init.h"


extern int g_perfMonitor;
extern int g_freqExpe;
extern int g_preemptASAI;
module_param(g_perfMonitor,    int, 0644);
module_param(g_freqExpe,    int, 0644);
module_param(g_preemptASAI,    int, 0644);

int MI_IPU_ParseIrq(unsigned int *pu32Virq, int len)
{
    int devid;

    struct device_node *dev_node = NULL;
    struct platform_device *pdev= NULL;

    dev_node = of_find_compatible_node(NULL, NULL, "sgs,dla");
    if (!dev_node) {
        CamOsPrintf("fail to find dla device tree node\n");
        return -1;
    }
    pdev = of_find_device_by_node(dev_node);
    if (!pdev) {
        CamOsPrintf("fail to find dla device node\n");
        return -1;
    }

    for (devid = 0; devid < len; devid++) {
        pu32Virq[devid] = CamIrqOfParseAndMap(pdev->dev.of_node, devid);
    }

    return 0;
}

MI_S32 MI_IPU_Parse_Freq_Vol_Table(char *pCompatible, MI_IPU_Freq_Vol_t **ppstIpuOppTb, unsigned int *pTbLen)
{
#if (defined(CONFIG_SGS_VOLTAGE_VID) || defined(CONFIG_SGS_VOLTAGE_IDAC))
    MI_S32 opp_cnt, loop;
    MI_U32 freq_khz, voltage_uv;
    struct device_node *dev_node = NULL;
    struct property *prop = NULL;
    int nr;
    const __be32 *value;
    MI_IPU_Freq_Vol_t *ipu_opp_tb;

    dev_node = of_find_compatible_node(NULL, NULL, pCompatible);
    if (!dev_node) {
        CamOsPrintf("fail to find ipu device tree node %s\n", pCompatible);
        return -1;
    }

    prop = of_find_property(dev_node, "operating-points", NULL);
    if (!prop) {
        CamOsPrintf("fail to find ipu operating-points\n");
        return -1;
    }

    if (!prop->value) {
        CamOsPrintf("fail to get ipu operating-points value\n");
        return -1;
    }

    nr = prop->length / sizeof(unsigned int);
    if (nr % 2) {
        CamOsPrintf("Invalid OPP table\n");
        return -1;
    }
    opp_cnt = nr / 2;
    if (!opp_cnt) {
        CamOsPrintf("Invalid OPP table length\n");
        return -1;
    }

    ipu_opp_tb = CamOsMemAlloc(sizeof(*ipu_opp_tb)*opp_cnt);
    if (!ipu_opp_tb) {
        CamOsPrintf("fail to allocate memory for ipu opp table\n");
        return -1;
    }

    /*
    *Each OPP is a set of tuples consisting of frequency ad voltage like <kHz uV>
    */
    value = (unsigned int *)prop->value;
    for (loop = 0; loop < opp_cnt; loop++) {
        freq_khz = be32_to_cpup(value++);
        voltage_uv = be32_to_cpup(value++);
        ipu_opp_tb[loop].freq_mhz = freq_khz / 1000;
        ipu_opp_tb[loop].voltage_mv = voltage_uv / 1000;
    }

    *ppstIpuOppTb = ipu_opp_tb;
    *pTbLen = opp_cnt;

    return 0;
#else
    return -1;
#endif
}

int MI_IPU_GetPllClk(void **ppPllClk)
{
#if 0 //(defined CONFIG_SGS_CHIP_IFORD)
    void *pll_clk = NULL;
    void *pll_clk_parent = NULL;
    void *pll_clk_pparent = NULL;
#else
    struct clk *pll_clk = NULL;
#endif

    struct device_node *dev_node = NULL;

    dev_node = of_find_compatible_node(NULL, NULL, "sgs,dla");
    if (!dev_node) {
        CamOsPrintf("fail to find ipu pll device node\n");
        return -1;
    }
    pll_clk = of_clk_get(dev_node, 0);
    if (IS_ERR(pll_clk))
    {
        CamOsPrintf("fail to get ipu pll clk\n");
        return -1;
    }

#if 0 //(defined CONFIG_SGS_CHIP_IFORD)
    // FIXME: select clock index by VDD
    idx = 1; //upll_320M
    pll_clk_parent = drv_clk_get_parent(pll_clk);
    pll_clk_pparent = drv_clk_get_parent_by_index(pll_clk_parent, idx);
    drv_clk_set_parent(pll_clk_parent, pll_clk_pparent);

#if 0
    for(idx = 0; idx < drv_clk_get_num_parents(pll_clk_parent); idx++)
    {
        pll_clk_pparent = drv_clk_get_parent_by_index(pll_clk_parent, idx);
        CamOsPrintf("IPU clock parent[%d] Freq: %dHz\n", idx, drv_clk_get_rate(pll_clk_pparent));
    }
#endif
    *ppPllClk = pll_clk;
#else
    *ppPllClk = (void *)pll_clk;
#endif
    return 0;
}

static MI_IPU_Common_Callback_t stCommonCallback;
int MI_IPU_Scaling_Freq(unsigned int freq_mhz)
{
    return stCommonCallback.scaling_freq(freq_mhz);
}
EXPORT_SYMBOL(MI_IPU_Scaling_Freq);

int MI_IPU_LinuxInit(MI_IPU_Common_Callback_t *pstCommonCallback)
{
    if (pstCommonCallback) {
        stCommonCallback = *pstCommonCallback;
    }

    return 0;
}

void MI_IPU_Iounmap_Wrapper(void *addr)
{
    iounmap(addr);
}

__attribute__((weak)) void *MI_IPU_GetInternalApis(void)
{
    return NULL;
}
extern void *MI_IPU_GetInternalApis(void);
EXPORT_SYMBOL(MI_IPU_GetInternalApis);

MI_S32 MI_IPU_Invoke_ByKernel(MI_U32 u32ChnId, MI_IPU_BatchInvokeParam_t *pstInvokeParam,
    MI_IPU_RuntimeInfo_t *pstRuntimeInfo)
{
    if (!stCommonCallback.invoke_by_kernel)
    {
        CamOsPrintf("MI_IPU_Invoke_ByKernel callback is not registered\n");
        return -1;
    }

    return stCommonCallback.invoke_by_kernel(u32ChnId, pstInvokeParam, pstRuntimeInfo);
}
EXPORT_SYMBOL(MI_IPU_Invoke_ByKernel);

DECLEAR_MODULE_INIT_EXIT
MI_MODULE_LICENSE();
MODULE_AUTHOR("Sgs");

