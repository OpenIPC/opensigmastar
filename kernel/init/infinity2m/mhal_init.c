/*
 * mhal: the i2m NVR SDK's mhal.ko (2020-03), relinked for this kernel.
 * The code is SigmaStar's, unchanged (kernel/obj/infinity2m/mhal.o, made by
 * scripts/ko2obj.sh); this only calls its entry points. Its exports and
 * module parameters come with it. The licence is the vendor module's own.
 */
#include <linux/module.h>

extern int _i2m_mhal_init(void);
extern void _i2m_mhal_exit(void);

static int __init mhal_mod_init(void)
{
	return _i2m_mhal_init();
}

static void __exit mhal_mod_exit(void)
{
	_i2m_mhal_exit();
}

module_init(mhal_mod_init);
module_exit(mhal_mod_exit);
MODULE_LICENSE("PROPRIETARY");
