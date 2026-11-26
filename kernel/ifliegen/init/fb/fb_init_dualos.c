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
#include <linux/fs.h>
#include <linux/platform_device.h>

#include "cam_os_wrapper.h"
#include "cam_device_wrapper.h"
#include "mi_fb.h"

#define ASCII_COLOR_CYAN   "\033[1;36m"
#define ASCII_COLOR_YELLOW "\033[1;33m"
#define ASCII_COLOR_RED    "\033[1;31m"
#define ASCII_COLOR_END    "\033[0m"
#define DBG_INFO(fmt, args...)                                                                                         \
    (                                                                                                                  \
        {                                                                                                              \
            do                                                                                                         \
            {                                                                                                          \
                if (g_debugSwitch)                                                                                     \
                {                                                                                                      \
                    printk(KERN_DEBUG ASCII_COLOR_CYAN "%s[%d] " fmt ASCII_COLOR_END, __FUNCTION__, __LINE__, ##args); \
                }                                                                                                      \
            } while (0);                                                                                               \
        })
#define DBG_WRN(fmt, args...) \
    printk(KERN_WARNING ASCII_COLOR_YELLOW "%s[%d] " fmt ASCII_COLOR_END, __FUNCTION__, __LINE__, ##args);
#define DBG_ERR(fmt, args...) \
    printk(KERN_ERR ASCII_COLOR_RED "%s[%d] " fmt ASCII_COLOR_END, __FUNCTION__, __LINE__, ##args);

#define DEVICE_NAME "sgsfb"                          ///< The device will appear at /dev/sgsfb using this value
#define CLASS_NAME  "fbdev"                            ///< The device class -- this is a character device driver
static int                    g_majorNumber;           ///< Stores the device number -- determined automatically
static struct CamClass *      gp_sgsFbClass  = NULL; ///< The device-driver class struct pointer
static struct CamDevice *     gp_sgsFbDevice = NULL; ///< The device-driver device struct pointer
static struct file_operations g_stOps        = {

};

extern int MDRV_FB_Probe(struct device *dev);
extern int MDRV_FB_Remove(struct device *dev);
static int __init _MDRV_FB_Init(void)
{
    int retval = 0;

    g_majorNumber = register_chrdev(0, DEVICE_NAME, &g_stOps);
    if (g_majorNumber < 0)
    {
        return g_majorNumber;
    }

    gp_sgsFbClass = CamClassCreate(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(gp_sgsFbClass))
    {
        DBG_ERR("Failed to create device class\n");
        retval = PTR_ERR(gp_sgsFbClass);
        goto create_class_failed;
    }
    gp_sgsFbDevice = CamDeviceCreate(gp_sgsFbClass, NULL, MKDEV(g_majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(gp_sgsFbDevice))
    {
        DBG_ERR("Failed to create the device\n");
        retval = PTR_ERR(gp_sgsFbDevice);
        goto create_dev_failed;
    }

    if (MDRV_FB_Probe(CamDeviceGetInternalDevice(gp_sgsFbDevice)))
    {
        DBG_ERR("Failed to probe fb dev\n");
        retval = -EFAULT;
        goto probe_failed;
    }
    CamOsPrintf("module [%s] init\n", "fb");
    return 0;

probe_failed:
    CamDeviceDestroy(gp_sgsFbClass, MKDEV(g_majorNumber, 0));
create_dev_failed:
    CamClassDestroy(gp_sgsFbClass);
create_class_failed:
    unregister_chrdev(g_majorNumber, DEVICE_NAME);
    return retval;
}

static void __exit _MDRV_FB_Exit(void)
{
    CamOsPrintf("module [%s] deinit\n", "fb");
    MDRV_FB_Remove(CamDeviceGetInternalDevice(gp_sgsFbDevice));
    CamDeviceDestroy(gp_sgsFbClass, MKDEV(g_majorNumber, 0));
    CamClassDestroy(gp_sgsFbClass);
    unregister_chrdev(g_majorNumber, DEVICE_NAME);
}

module_init(_MDRV_FB_Init);
module_exit(_MDRV_FB_Exit);

#ifdef NON_COMMERCIAL_FOR_INTERNAL_TEST_ONLY
MODULE_LICENSE("GPL v2");
#else
MODULE_LICENSE("Proprietary");
#endif
