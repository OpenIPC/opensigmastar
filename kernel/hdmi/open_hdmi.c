// SPDX-License-Identifier: GPL-2.0
/*
 * open_hdmi - switch the SSR621Q HDMI transmitter on at the timing the
 * display (MI_DISP) scans out, through the MhalHdmitx API of open_hdmitx.ko.
 * It does what the vendor's mi_hdmi does on MI_HDMI_SetAttr + Start, which
 * the i2m NVR SDK (2020) does not ship.
 *
 *   /sys/kernel/open_hdmi/output   write 1080p60 | 1080p50 | 720p60 |
 *                                      720p50 | 1024x768p60 | 1280x1024p60 |
 *                                      off; read: current
 *   /sys/kernel/open_hdmi/hpd      1 when a sink is plugged in
 *   /sys/kernel/open_hdmi/edid     the sink's EDID block 0 (binary)
 *
 * Module parameters: output= switches on at load, color=yuv444|rgb444
 * (what goes out; the display feeds YUV444).
 */
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/string.h>
#include <linux/sysfs.h>

#include "MsTypes.h"
#include "mhal_hdmitx_datatype.h"
#include "mhal_hdmitx.h"

static char *output = "";
module_param(output, charp, 0444);
MODULE_PARM_DESC(output, "timing to switch on at load, e.g. 1080p60");
static char *color = "yuv444";
module_param(color, charp, 0444);
MODULE_PARM_DESC(color, "output colour: yuv444 or rgb444");

static const struct {
	const char *name;
	MhaHdmitxTimingResType_e res;
} timings[] = {
	{ "1080p60", E_MHAL_HDMITX_RES_1920X1080P_60HZ },
	{ "1080p50", E_MHAL_HDMITX_RES_1920X1080P_50HZ },
	{ "1080p30", E_MHAL_HDMITX_RES_1920X1080P_30HZ },
	{ "720p60", E_MHAL_HDMITX_RES_1280X720P_60HZ },
	{ "720p50", E_MHAL_HDMITX_RES_1280X720P_50HZ },
	{ "1024x768p60", E_MHAL_HDMITX_RES_1024X768P_60HZ },
	{ "1280x1024p60", E_MHAL_HDMITX_RES_1280X1024P_60HZ },
	{ "1440x900p60", E_MHAL_HDMITX_RES_1440X900P_60HZ },
	{ "2160p30", E_MHAL_HDMITX_RES_3840X2160P_30HZ },
};

static DEFINE_MUTEX(lock);
static void *ctx;
static int cur = -1;            /* index into timings, -1 off */
static struct kobject *kobj;

static int hdmi_off(void)
{
	MhalHdmitxSignalConfig_t sig = { .bEn = false };
	MhalHdmitxMuteConfig_t mute = {
		.bMute = true,
		.enType = E_MHAL_HDMITX_MUTE_VIDEO | E_MHAL_HDMITX_MUTE_AUDIO |
			  E_MHAL_HDMITX_MUTE_AVMUTE,
	};

	if (!ctx)
		return 0;
	MhalHdmitxSetMute(ctx, &mute);
	MhalHdmitxSetSignal(ctx, &sig);
	cur = -1;
	return 0;
}

static int hdmi_on(int t)
{
	MhalHdmitxAttrConfig_t attr;
	MhalHdmitxSignalConfig_t sig = { .bEn = true };
	MhalHdmitxMuteConfig_t mute = {
		.bMute = false,
		.enType = E_MHAL_HDMITX_MUTE_VIDEO | E_MHAL_HDMITX_MUTE_AUDIO |
			  E_MHAL_HDMITX_MUTE_AVMUTE,
	};
	MhalHdmitxRet_e r;

	if (!ctx) {
		r = MhalHdmitxCreateInstance(&ctx, 0);
		if (r != E_MHAL_HDMITX_RET_SUCCESS) {
			pr_err("open_hdmi: create instance: %d\n", r);
			ctx = NULL;
			return -EIO;
		}
	}

	memset(&attr, 0, sizeof(attr));
	attr.bVideoEn = true;
	attr.enInColor = E_MHAL_HDMITX_COLOR_YUV444;
	attr.enOutColor = strcmp(color, "rgb444") ? E_MHAL_HDMITX_COLOR_YUV444 :
						    E_MHAL_HDMITX_COLOR_RGB444;
	attr.enColorDepth = E_MHAL_HDMITX_CD_24_BITS;
	attr.enOutputMode = E_MHAL_HDMITX_OUTPUT_MODE_HDMI;
	attr.enTiming = timings[t].res;
	attr.bAudioEn = false;
	attr.enAudioFreq = E_MHAL_HDMITX_AUDIO_FREQ_48K;
	attr.enAudioCh = E_MHAL_HDMITX_AUDIO_CH_2;
	attr.enAudioCode = E_MHAL_HDMITX_AUDIO_CODING_PCM;
	attr.enAudioFmt = E_MHAL_HDMITX_AUDIO_FORMAT_PCM;

	MhalHdmitxSetAttrBegin(ctx);
	r = MhalHdmitxSetAttr(ctx, &attr);
	MhalHdmitxSetAttrEnd(ctx);
	if (r != E_MHAL_HDMITX_RET_SUCCESS && r != E_MHAL_HDMITX_RET_HPDERR) {
		pr_err("open_hdmi: set attr %s: %d\n", timings[t].name, r);
		return -EIO;
	}
	MhalHdmitxSetSignal(ctx, &sig);
	MhalHdmitxSetMute(ctx, &mute);
	cur = t;
	pr_info("open_hdmi: %s %s%s\n", timings[t].name, color,
		r == E_MHAL_HDMITX_RET_HPDERR ? " (no sink yet)" : "");
	return 0;
}

static int set_output(const char *s)
{
	size_t n = strcspn(s, "\n");
	int i, ret = -EINVAL;

	mutex_lock(&lock);
	if (n == 3 && !strncmp(s, "off", 3)) {
		ret = hdmi_off();
	} else {
		for (i = 0; i < ARRAY_SIZE(timings); i++) {
			if (strlen(timings[i].name) == n &&
			    !strncmp(s, timings[i].name, n)) {
				ret = hdmi_on(i);
				break;
			}
		}
	}
	mutex_unlock(&lock);
	return ret;
}

static ssize_t output_show(struct kobject *k, struct kobj_attribute *a,
			   char *buf)
{
	return sprintf(buf, "%s\n", cur < 0 ? "off" : timings[cur].name);
}

static ssize_t output_store(struct kobject *k, struct kobj_attribute *a,
			    const char *buf, size_t len)
{
	int ret = set_output(buf);

	return ret < 0 ? ret : len;
}

static ssize_t hpd_show(struct kobject *k, struct kobj_attribute *a,
			char *buf)
{
	MhalHdmitxSinkInfoConfig_t si;
	int hpd = 0;

	mutex_lock(&lock);
	if (!ctx && MhalHdmitxCreateInstance(&ctx, 0) != E_MHAL_HDMITX_RET_SUCCESS)
		ctx = NULL;
	if (ctx) {
		memset(&si, 0, sizeof(si));
		si.enType = E_MHAL_HDMITX_SINK_INFO_HPD_STATUS;
		if (MhalHdmitxGetSinkInfo(ctx, &si) == E_MHAL_HDMITX_RET_SUCCESS)
			hpd = si.stInfoUnit.stHpdStatus.bHpd;
	}
	mutex_unlock(&lock);
	return sprintf(buf, "%d\n", hpd);
}

static ssize_t edid_show(struct kobject *k, struct kobj_attribute *a,
			 char *buf)
{
	MhalHdmitxSinkInfoConfig_t si;
	ssize_t n = 0;

	mutex_lock(&lock);
	if (ctx) {
		memset(&si, 0, sizeof(si));
		si.enType = E_MHAL_HDMITX_SINK_INFO_EDID_DATA;
		si.stInfoUnit.stEdidData.u8BlockId = 0;
		if (MhalHdmitxGetSinkInfo(ctx, &si) == E_MHAL_HDMITX_RET_SUCCESS) {
			memcpy(buf, si.stInfoUnit.stEdidData.au8EdidData, 128);
			n = 128;
		}
	}
	mutex_unlock(&lock);
	return n;
}

static struct kobj_attribute output_attr = __ATTR_RW(output);
static struct kobj_attribute hpd_attr = __ATTR_RO(hpd);
static struct kobj_attribute edid_attr = __ATTR_RO(edid);

static struct attribute *attrs[] = {
	&output_attr.attr, &hpd_attr.attr, &edid_attr.attr, NULL,
};

static const struct attribute_group group = { .attrs = attrs };

static int __init open_hdmi_init(void)
{
	int ret;

	kobj = kobject_create_and_add("open_hdmi", kernel_kobj);
	if (!kobj)
		return -ENOMEM;
	ret = sysfs_create_group(kobj, &group);
	if (ret) {
		kobject_put(kobj);
		return ret;
	}
	if (output && *output && set_output(output) < 0)
		pr_warn("open_hdmi: output=%s failed\n", output);
	return 0;
}

static void __exit open_hdmi_exit(void)
{
	mutex_lock(&lock);
	hdmi_off();
	if (ctx)
		MhalHdmitxDestroyInstance(ctx);
	ctx = NULL;
	mutex_unlock(&lock);
	sysfs_remove_group(kobj, &group);
	kobject_put(kobj);
}

module_init(open_hdmi_init);
module_exit(open_hdmi_exit);
MODULE_DESCRIPTION("HDMI output on through the SigmaStar MhalHdmitx API");
MODULE_LICENSE("GPL");
