# Iford: MI kernel modules

Iford is SigmaStar's SSC029A, SSC029B and SSC029C (2× Cortex-A32, Linux
5.10). Its SDK, `Iford_IMD00V2.1.0` (2024-09-24), ships every MI kernel
module as a prebuilt relocatable object plus that module's init and export
glue in source, and links the two into `mi_<module>.ko` against the kernel
being built (`sdk/linux`: `makefile`, `link_modules.mk`,
`init/<module>/cfg.mk`). `kernel/iford.kbuild` does the same link in plain
Kbuild, with nothing from the SDK's `project/` tree at build time.

## What is built

`CHIPARCH=iford` builds 20 modules, named as the SDK names them
(`MOD_PREFIX` `mi_`), so they replace the SDK's `mi_*.ko` one for one,
including the `insmod` lines of its `demo.sh`:

`mi_common` `mi_sys` `mi_sensor` `mi_rgn` `mi_disp` `mi_fb` `mi_dummy`
`mi_aio` `mi_vif` `mi_isp` `mi_scl` `mi_ive` `mi_ai` `mi_ao` `mi_ldc`
`mi_shadow` `mi_venc` `mi_vdisp` `mi_ipu` `mi_debug`

That is the set the SDK's `ipc` product builds: its defconfig
`ipc_iford.nor.uclibc-9.1.0-squashfs.ssz029c.128.qfn128_ddr3_demo_aov`
resolved through the project's Kconfig enables these `interface_<x>`, and
the SDK's prebuilt NOR image of that product has exactly these 20 in its
`miservice` partition. The SDK also has init sources for `jpd` and `nir`
but no object for either (both are `disable` for this product), so they
are not here. `fb_dualos.o` is only for the dual-OS (`DUAL_OS = on`)
builds and is not used.

| Path | What | From the SDK |
|------|------|--------------|
| `kernel/iford/obj/<module>.o_shipped` | the prebuilt objects without their debug sections (`objcopy --strip-debug`: symbols, code and relocations unchanged, 35 MB to 3.7 MB) | `sdk/linux/obj/uclibc/9.1.0/<module>.o` |
| `kernel/iford/init/<module>/` | init and export glue, SigmaStar's source | `sdk/linux/init/<module>/` (without `cfg.mk`) |
| `kernel/iford/include/` | sensor driver interface the glue includes | `sdk/driver/SensorDriver/drv/pub/` |
| `include/iford/` | MI headers for applications and for the glue | `project/release/include/` (as shipped, with its `isp/iford` and `ipu/iford`) |

The `_shipped` suffix is Kbuild's own way to carry binaries: the build
copies each to `<module>.o`, and `make clean` leaves the original alone.

What `iford.kbuild` reproduces, per module, from `link_modules.mk` and the
`cfg.mk` files as `current.configs` resolves them for that product:

- objects: the vendor object plus `INIT_FILES`; of the KAPI export files
  only `fb_api_export.c` (`ENABLE_MI_KAPI = y`, `mi_fb_kapi = yes`, every
  other `mi_<x>_kapi` is `no` or unset);
- include path: the module's own init directory and those of its
  `DEP_MODULE`, then the sensor driver headers, the MI headers, and the
  kernel's `drivers/sstar/include` and `drivers/sstar/include/iford`;
- flags: `-g -Wall -Wpointer-sign -Wno-unused-variable -Wno-unused-result
  -pipe -include linux/version.h -DALKAID_SDK -DEXTRA_MODULE_NAME=<module>
  -DCONFIG_MI_<MODULE>=1 -DCONFIG_ENABLE_MI_NORMAL_STR`, and from the
  `cfg.mk` files `-DMI_COMMON_RGNSUB` (common), `-DMI_FB_NORMAL` (fb),
  `-DUSE_IPU_OPP_TABLE=0` (ipu), `-D__DISABLE_MHAL_ISR_REGISTER__
  -D__I6F_ISP__` (isp), `-D__ENABLE_SENSOR_UT__ -D_CSI_DBG_ISR_ERR_`
  (sensor).

`-Werror` is left out: it is the SDK's, and makes the build depend on the
compiler's warning set. The SDK's own `tr ... <<<` needs bash as `/bin/sh`
to produce `-DCONFIG_MI_<MODULE>=1`; `disp_init.c` tests it. Here it is
written out.

### Which objects

The SDK has two sets of objects, `glibc/11.1.0` and `uclibc/9.1.0`. The C
library does not enter a kernel object; the compiler does. Their defined
symbols are the same, but every uClibc-set object references
`__stack_chk_guard` and no glibc-set object does: the gcc 11 kernels are
built with the GCC plugin `arm_ssp_per_task` (`CONFIG_STACKPROTECTOR_PER_TASK`,
the canary per task), the gcc 9.1 kernels without it (one global
`__stack_chk_guard`). The set has to match the kernel, not the userland.
`isp`, `venc` (gcc 11 adds `memmove`) and `ldc` (`memcpy`) differ in one
more import each.

This repository carries the `uclibc/9.1.0` set: the image tested below
(the SDK's NOR `ipc` image, kernel built by
`arm-sigmastar-linux-uclibcgnueabihf-9.1.0-gcc`) runs those objects, and
its prebuilt `mi_*.ko` reference `__stack_chk_guard`. Build the kernel
without GCC plugins (`# CONFIG_GCC_PLUGINS is not set`) so that the kernel
exports it. A kernel with `CONFIG_STACKPROTECTOR_PER_TASK=y` needs the
`glibc/11.1.0` set instead; it is not here.

## Build

There is no OpenIPC kernel for Iford yet. The modules were built against
the SDK's kernel (`kernel-Iford_IMD00V2.1.0.tar.gz`, 5.10.117) configured
as the SDK does for that product, `iford_ssz029c_s01b_nor_demo_defconfig`,
with the GCC plugins off as above:

```sh
# the SDK kernel calls `python`; a python3 named python on PATH will do
make -f Makefile ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabihf- \
     iford_ssz029c_s01b_nor_demo_defconfig
scripts/config --disable GCC_PLUGINS
make -f Makefile ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabihf- olddefconfig
make -f Makefile ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabihf- -j$(nproc) vmlinux modules
```

(`-f Makefile`: the tree's lowercase `makefile` is SigmaStar's wrapper.
Its `uImage` step fails on Python 3; the modules do not need it.) Then:

```sh
make -C kernel KDIR=<kernel> CHIPARCH=iford \
     ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabihf-
```

and strip as the SDK does for its image (`STRIP_OPTION = --strip-debug`):
`arm-none-linux-gnueabihf-strip --strip-debug kernel/mi_*.ko`.

Toolchain: the SDK's own (crosstool-NG gcc 9.1.0 uClibc, or SigmaStar gcc
11.1.0 glibc) was not available. Used: Arm GNU Toolchain 10.3-2021.07
(`arm-none-linux-gnueabihf-`, gcc 10.3.1) for the kernel and the modules;
the kernel builds with it without changes, under its own `-Werror`. The
modules also build, with the same symbols, sections and modinfo, with
OpenIPC's buildroot gcc 13.3.0 (`arm-openipc-linux-gnueabihf-`) against
that kernel. The SDK kernel itself does not build with gcc 13.3 as is: its
global `-Werror` stops on gcc 13's new warnings (`dangling-pointer`,
`int-to-pointer-cast` in `kernel/workqueue.c`).

## Verified

**Against the SDK's own link.** The SDK's `sdk/linux` makefile, run with
the same kernel and compiler (`bash` as the shell, `current.configs`
generated from that defconfig), builds the same 20 modules. Every init
object is the same code as ours (`objdump -d -r` identical); `sys_init.o`
differs only in its `__FILE__` string, the path.

**Against the SDK's prebuilt modules** (the image's
`miservice/modules/5.10/mi_*.ko`), all 20:

- exported symbols: identical (113 in all: common 45, fb 27, sensor 17,
  sys 12, disp 5, rgn 2, aio, ipu, isp, venc, vif 1);
- undefined symbols: identical;
- `.modinfo`: identical (`vermagic=5.10.117 SMP preempt mod_unload ARMv7
  thumb2 p2v8`, license, author, parameters) except `depends`. Ours lists
  every module whose symbols are used, e.g. `mi_ai` `mi_common,mi_aio,mi_sys`
  where the SDK's has `mi_common`, `mi_dummy` `mi_common,mi_sys` where the
  SDK's has none. The SDK links each module in its own Kbuild run and sees
  other modules only through `KBUILD_EXTRA_SYMBOLS` of its `DEP_MODULE`,
  which do not exist yet when the modules build in alphabetical order.
  `depends` matters to `modprobe` only;
- sections: the same names; `.gnu.linkonce.this_module` (`struct module`)
  0x1C0 bytes in both. Sizes differ only in the init code (gcc 10.3 for
  9.1, e.g. `mi_sys` `.text` +16 bytes), the `__FILE__` strings and
  `.comment`; after `strip --strip-debug` the files are a little
  smaller than SigmaStar's, by 40 to 184 bytes.

**In QEMU** (qemu-bench `ssc029`, its unmodified build), the SDK's NOR image
of that product (`qemu-boot/sstar-iford-image.py`), booted twice: as
shipped, and with the 20 `mi_*.ko` in `miservice` replaced by these
(squashfs repacked, xz, 128 KiB blocks). Each time: `demo.sh` loads all
modules at boot, `sstar_emac.ko` with a static address, then the SDK's
amigos `preview` (ipc-rtos uClibc build, from an SD card) with its stock
`pad0-venc-file.json` (sensor → VIF → ISP → SCL → VENC → RTSP), and
`ffmpeg -rtsp_transport tcp` on `rtsp://10.0.2.15/video0` for 40 s of wall
clock.

| | SDK modules | these modules |
|---|---|---|
| `lsmod`: `mi_*` live | 20 | 20 |
| `module [<x>] init` lines in dmesg | all 20 modules | the same |
| dmesg, distinct lines | | the same set, except addresses, timings, VENC register dumps, and one `alloc_contig_range: PFNs busy` in the SDK run |
| oops, BUG | none | none |
| RTSP stream | H.264 3840×2160, 120 frames | H.264 3840×2160, 143 frames |
| decoded frames (`framemd5`) | all `8dc5314b…` | all `8dc5314b…` |

The frame counts follow the host's load, not the modules. Both runs log
the same `_MI_SYS_Mma_MapVpaToPa ... PA Slice count ... less than require
count` / `Get Buffer Faild` (4K buffers in this image's 64 MiB MMA heap)
and the same amigos `FILE` errors (its output path is not writable); these
are the SDK's, with either set of modules.

**Not verified:** hardware; unloading (`rmmod`); the parts the machine
does not model (IPU, audio, display, AE in this setup); the
`CONFIG_STACKPROTECTOR_PER_TASK` (glibc 11.1.0) variant; other Iford
products (`ipc-rtos` dual-OS, `usbcam`), whose module set and options
differ.
