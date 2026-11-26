/*
 * mi_sys: the i2m NVR SDK's mi_sys.ko (2020-03), relinked for this kernel.
 * The code is SigmaStar's, unchanged (kernel/obj/infinity2m/mi_sys.o, made by
 * scripts/ko2obj.sh); this only calls its entry points. Its exports and
 * module parameters come with it. The licence is the vendor module's own.
 * A debug command that called an SDK tool in /config is rewritten to do
 * nothing (scripts/restring.py, kernel/obj/infinity2m/restring.txt). While
 * it loads it runs <default_config_path>/config_tool (memory map, panel
 * timings, PQ) and waits for it; default_config_path defaults to /config.
 */
#include <linux/module.h>

extern int _i2m_mi_sys_init(void);
extern void _i2m_mi_sys_exit(void);

static int __init mi_sys_mod_init(void)
{
	return _i2m_mi_sys_init();
}

static void __exit mi_sys_mod_exit(void)
{
	_i2m_mi_sys_exit();
}

module_init(mi_sys_mod_init);
module_exit(mi_sys_mod_exit);
MODULE_LICENSE("Proprietary");
