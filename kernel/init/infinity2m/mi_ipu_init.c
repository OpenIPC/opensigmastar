/*
 * mi_ipu: the i2m NVR SDK's mi_ipu.ko (2020-03), relinked for this kernel.
 * The code is SigmaStar's, unchanged (kernel/obj/infinity2m/mi_ipu.o, made by
 * scripts/ko2obj.sh); this only calls its entry points. Its exports and
 * module parameters come with it. The licence is the vendor module's own.
 */
#include <linux/module.h>

extern int _i2m_mi_ipu_init(void);
extern void _i2m_mi_ipu_exit(void);

static int __init mi_ipu_mod_init(void)
{
	return _i2m_mi_ipu_init();
}

static void __exit mi_ipu_mod_exit(void)
{
	_i2m_mi_ipu_exit();
}

module_init(mi_ipu_mod_init);
module_exit(mi_ipu_mod_exit);
MODULE_LICENSE("GPL");
