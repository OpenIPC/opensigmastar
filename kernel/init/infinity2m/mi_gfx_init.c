/*
 * mi_gfx: the i2m NVR SDK's mi_gfx.ko (2020-03), relinked for this kernel.
 * The code is SigmaStar's, unchanged (kernel/obj/infinity2m/mi_gfx.o, made by
 * scripts/ko2obj.sh); this only calls its entry points. Its exports and
 * module parameters come with it. The licence is the vendor module's own.
 */
#include <linux/module.h>

extern int _i2m_mi_gfx_init(void);
extern void _i2m_mi_gfx_exit(void);

static int __init mi_gfx_mod_init(void)
{
	return _i2m_mi_gfx_init();
}

static void __exit mi_gfx_mod_exit(void)
{
	_i2m_mi_gfx_exit();
}

module_init(mi_gfx_mod_init);
module_exit(mi_gfx_mod_exit);
MODULE_LICENSE("Proprietary");
