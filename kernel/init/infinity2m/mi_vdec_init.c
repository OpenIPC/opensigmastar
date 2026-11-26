/*
 * mi_vdec: the i2m NVR SDK's mi_vdec.ko (2020-03), relinked for this kernel.
 * The code is SigmaStar's, unchanged (kernel/obj/infinity2m/mi_vdec.o, made by
 * scripts/ko2obj.sh); this only calls its entry points. Its exports and
 * module parameters come with it. The licence is the vendor module's own.
 * Its decoder firmware paths and a debug command are rewritten
 * (scripts/restring.py, kernel/obj/infinity2m/restring.txt): it loads
 * /lib/firmware/chagall-{normal,latency}.bin, not /config/vdec_fw/.
 */
#include <linux/module.h>

extern int _i2m_mi_vdec_init(void);
extern void _i2m_mi_vdec_exit(void);

static int __init mi_vdec_mod_init(void)
{
	return _i2m_mi_vdec_init();
}

static void __exit mi_vdec_mod_exit(void)
{
	_i2m_mi_vdec_exit();
}

module_init(mi_vdec_mod_init);
module_exit(mi_vdec_mod_exit);
MODULE_LICENSE("Proprietary");
