# Ifliegen (SSC034A, SSC034B)

`CHIPARCH=ifliegen`. Source: SigmaStar's Ifliegen SDK ILSSV01C12
(`sdk-`, `project-`, `kernel-Ifliegen_ILSSV01C12.tar.gz`, built 2025-12-30),
Linux 4.9.84.

## What is built

| Module | From |
|--------|------|
| `mi_common.ko`, `mi_sys.ko`, `mi_rgn.ko`, `mi_sensor.ko`, `mi_vif.ko`, `mi_scl.ko`, `mi_isp.ko`, `mi_venc.ko`, `mi_ive.ko`, `mi_ipu.ko`, `mi_vdf.ko`, `mi_vdisp.ko`, `mi_disp.ko`, `mi_fb.ko`, `mi_aio.ko`, `mi_ai.ko`, `mi_ao.ko`, `mi_dummy.ko`, `mi_debug.ko` | the vendor object `kernel/ifliegen/obj/<x>.o_shipped` relinked with its init and export glue, `kernel/ifliegen/init/<x>/` |
| `bf3901.ko`, `gc032a.ko` | the PSPI sensor drivers, `kernel/ifliegen/pspi/` (SDK `sdk/driver/PspiDriver`), full source |

The MI set is every module the SDK's `sdk/linux` builds for its IPC product
(`ifliegen_sdk_generic_defconfig`, the same for all the `ipc_ifliegen.*`
defconfigs): every `interface_<x>=enable` that has an object in
`sdk/linux/obj`. They are the 19 `mi_*.ko` in the SDK's
`release/chip/ifliegen/ipc/common/glibc/9.1.0/arm/release/modules/4.9.84`,
with the same names, so they replace those files one for one.

Not built, because the SDK has no object for them: `mi_pspi` (the SDK
disables `interface_pspi` for Ifliegen), `mi_gfx`, `mi_vdec`, `mi_jpd`,
`mi_ldc`, `mi_hdmi`, `mi_nir`, `mi_shadow`, `mi_vge`, `mi_dpu`. Their
`init/` sources are in the SDK but not copied here.

The PSPI drivers call `DRV_PSPI_SENSOR_Register`, `_UnRegister` and
`_BindingSlaveID`, which only `mi_pspi.ko` exports (`init/pspi/pspi_export.c`).
They build, but no Ifliegen module set can load them; the SDK's own
`bf3901.ko` fails the same way (see Verification).

## Layout

- `kernel/ifliegen.kbuild`: the build, `link_modules.mk`, each module's
  `cfg.mk` and `pspi.mk` written as Kbuild.
- `kernel/ifliegen/obj/<x>.o_shipped`: the SDK's
  `sdk/linux/obj/glibc/9.1.0/arm/<x>.o`, `objcopy --strip-debug`. Kbuild's
  `_shipped` rule copies each to `<x>.o` at build time, and `make clean`
  leaves it.
- `kernel/ifliegen/init/<x>/`: the SDK's `sdk/linux/init/<x>/*.c, *.h` for
  the 19 modules, unchanged (the files the build does not use, such as
  `dmabuf_init.c` or the `*_api_export.c` of modules whose kernel API export
  is off, are kept with them).
- `kernel/ifliegen/include/`: `project_autoconf.h` (the SDK project's
  `include/generated/autoconf.h`, which `link_modules.mk` force-includes)
  and SensorDriver's `drv/pub` headers.
- `kernel/ifliegen/module.lds`: the SDK's `sdk/linux/module.lds` plus the
  export sections, see below.
- `include/ifliegen/`: the SDK's `project/release/include`, the MI headers
  for applications. The kernel build uses them too, as `link_modules.mk`
  does.

**glibc or uClibc objects.** The SDK ships each object twice,
`obj/glibc/9.1.0` and `obj/uclibc/9.1.0`. Kernel code does not use a C
library, and the two are byte-identical once `.comment` (the compiler's
ident, `GCC: (GNU) 9.1.0` against `GCC: (crosstool-NG 1.24.0) 9.1.0`) and
the DWARF are removed: checked with `objcopy --strip-debug -R .comment` and
`cmp`, all 19. The glibc ones are used. The debug information is 92 % of
their size (46 MB of 49 MB) and is not needed to link, so it is stripped.

## How the SDK links them, and how this does

`sdk/linux/makefile` symlinks `link_modules.mk` into each `init/<x>`, which
builds `mi_<x>.ko` from `../../obj/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/arm/<x>.o`
and the init files, twice: once normally, to get `Module.symvers`, then again
with `LD=clang-13/llvm-ld` and
`-T module_tmp.lds --gc-sections -u init_module -u cleanup_module`, where
`module_tmp.lds` is `module.lds` plus a `KEEP(*(___ksymtab+<sym>))` for each
symbol the first pass exported. The garbage collection drops the parts of the
vendor object the module never reaches, including references to kernel
symbols that do not exist.

Here it is one pass with GNU ld (`ldflags-y` on the `ld -r` of each
`mi_<x>.o`): `module.lds` keeps every `___ksymtab+*` and `___ksymtab_gpl+*`
input section, which is the same set. The compiler flags are
`link_modules.mk`'s (`-ffunction-sections -fdata-sections -DALKAID_SDK
-DEXTRA_MODULE_NAME=<x> -DCONFIG_MI_<X>=1 -DCONFIG_SGS_CHIP_IFLIEGEN=1`,
`linux/version.h` and the project autoconf forced in), with its include
order: the module's own and its `DEP_MODULE` init directories, SensorDriver's
`drv/pub`, `release/include`, then the kernel's `drivers/sgs/include`,
`drivers/sgs_common/include` and `drivers/sgs/include/ifliegen`. The per
module options come from the IPC product's `current.configs`: `MI_FB_NORMAL`
and `MI_FB_VIDEO` for `mi_fb`, which is the only module with its kernel API
export on (`mi_fb_kapi=yes`, so `fb_api_export.c` and `-DKAPI_ENABLE`); no
`MI_SYS_DMABUF` or `MI_SYS_SG_TABLE`; gfx and vdec disabled.

Two changes from the SDK's flags:

- `-Werror` is left out (the sources build without a warning with GCC 10.3).
- `mi_isp`'s `isp_poll.c` includes `isp_cus3a_if.h`, which includes
  `mi_isp_hw_dep_datatype.h`. The SDK release has that in
  `release/include/isp/ifliegen` and its `mi_isp_*` headers in
  `release/include/isp`, and neither directory is on the include path that
  `link_modules.mk` and `init/isp/cfg.mk` set (the SDK's own make was not
  run to see how it gets there); the two are added for `mi_isp` only.

PSPI: `pspi.mk` builds `drv_pspi_<sensor>.ko` and renames the file to
`<sensor>.ko`; the module inside keeps the name `drv_pspi_<sensor>` (that is
what `lsmod` shows for the SDK's file too). The kbuild does the same in its
modpost step: `kernel/ifliegen/pspi/drv/drv_pspi_<sensor>.ko`, copied to
`kernel/<sensor>.ko`.

## Build

There is no OpenIPC kernel for Ifliegen yet. Against the SDK's kernel,
configured as the SDK builds it for the product (for the SPI NAND IPC board,
`ifliegen_ssc034a_s01a_spinand_defconfig`) and built (`make uImage modules`;
the tree needs `python` on the path and an empty
`scripts/sgs_coding_style_sanitize.py`):

```sh
make -C kernel KDIR=<kernel> CHIPARCH=ifliegen ARCH=arm \
     CROSS_COMPILE=arm-none-linux-gnueabihf-
```

Toolchain: Arm GNU Toolchain 10.3-2021.07 `arm-none-linux-gnueabihf` (GCC
10.3.1, binutils 2.36.1), the one the kernel was built with. The SDK names
`arm-linux-gnueabihf-linaro-9.1.0`, which is not public; the vendor objects
are GCC 9.1.0 output and link unchanged. Two builds from clean give
byte-identical `.ko` files.

## Verification

### Against the SDK's prebuilt modules

`release/chip/ifliegen/ipc/common/glibc/9.1.0/arm/release/modules/4.9.84`,
compared with `readelf`, `nm` and `modinfo`:

- **Exported symbols**: the same in all 21 modules.
- **Undefined symbols**: the same in all 21.
- **vermagic**: the same, `4.9.84 SMP preempt mod_unload ARMv7 thumb2 p2v8`;
  `license`, `author`, `description`, `parm` the same.
- **Sections**: the same set of allocated sections in all 21 (the order in
  the file differs: lld against GNU ld).
- **Functions and data**: for every MI module, the same functions and
  objects survive the garbage collection, with the same sizes, except a few
  functions of the init sources, which GCC 10.3 compiles 4 to 48 bytes
  differently from GCC 9.1.0 (`_DRV_ISP_PollFeRead`,
  `DRV_ISP_PollTriggerEvent`, `MI_IPU_Parse_Freq_Vol_Table`, `AudClkEnable`,
  `_MDRV_FB_PanDisplay`), and the numbering of static `__func__` names.

The differences, and why:

| What | Why |
|------|-----|
| `depends` is longer for `mi_sensor`, `mi_scl`, `mi_isp`, `mi_ipu`, `mi_disp`, `mi_fb`, `mi_dummy`, `mi_debug` (e.g. `mi_isp`: `mi_sys,mi_common,mi_sensor,mi_vif` in the SDK's, `+mi_ipu` here) | The SDK's modpost only sees the `Module.symvers` of the module's `DEP_MODULE`; a symbol from another module is left unresolved and its module is not listed. Here all are built in one pass, so `depends` names every module whose symbols are used. The undefined symbols are the same, so the SDK's modules need those modules loaded first too, which its load order (`.mods_depend`) does. |
| `.rodata` up to 10 KB larger | lld merges duplicate strings (`SHF_MERGE`) in `ld -r`; GNU ld does not merge them in a relocatable link. |
| `.ARM.exidx` 8 bytes larger | GNU ld ends the table with an `EXIDX_CANTUNWIND` entry for the end of `.text`. |
| `.text` of `mi_ipu` -4, `mi_aio` -8, `mi_fb` +44 bytes | the init sources, compiled by GCC 10.3. |
| `.comment` | GCC 10.3's ident added to the vendor objects' 9.1.0 one (lld also merges the identical strings). |
| no `.debug_*` | stripped from the objects. |
| `.symtab`, `.strtab` | lld and GNU ld keep different local symbols. |

### QEMU

qemu-bench's `ssc034a` (128 MiB) and `ssc034b` (64 MiB, `--package qfn88`), SPI NAND, booted as in
its `docs/sigmastar.md`: the SDK's IPL, IPL_CUST and TF-A, U-Boot from the
SDK's boot tree (`ifliegen_ssc034a_s01a_spinand_defconfig`), the kernel
above, image from `qemu-boot/sstar-ifliegen-image.py --nand`. The root is a
squashfs (the SDK's glibc busybox, the toolchain's glibc 2.33); a UBIFS
volume on `ubia` carries both module sets (`strip --strip-unneeded`, as the
SDK's image does), the SDK's `modparam_release.json` and `venc_fw` as
`/config`, the SDK's `imx415_mipi.ko`, and `venc_demo` (`sdk/verify/sample_code`
`source/ifliegen/venc/venc_demo`, built with the same GCC 10.3, the SDK's
static `libmi_*.a`).

Each run, from the same image, once with the SDK's modules and once with
these: `insmod` in the order of the SDK's `.mods_depend` (its `demo.sh`
order, `mi_common` with `g_modParamPath=/config/modparam.json`), then
`imx415_mipi.ko chmap=5` and `bf3901.ko`; `venc_demo 0` (H.264) and
`venc_demo 1` (H.265); `rmmod` of everything in reverse order.

| | SDK modules | these |
|---|---|---|
| 19 `mi_*.ko`, `imx415_mipi.ko` | load, `rc=0` | load, `rc=0` |
| `bf3901.ko` | `Unknown symbol DRV_PSPI_SENSOR_Register` (and the other two) | the same |
| `venc_demo 0` | `rc=0`, `venc_demo_case_h264.es` 19748 bytes, md5 `80e08739...` | the same bytes |
| `venc_demo 1` | `rc=0`, `venc_demo_case_h265.es` 22954 bytes, md5 `ce4c3242...` | the same bytes |
| `rmmod` all | no error, nothing left | the same |
| `lsmod` size | | 28 bytes (`mi_aio`) to 9448 (`mi_debug`) larger: the unmerged strings |

The console and `dmesg` of the two runs, with times, PIDs and addresses
masked, are the same line for line: every module's init messages, the
venc driver's (MIU protect, RISC firmware), the demo's, every exit message
on unload. What differs is the order of lines that two threads print at
once (the demo's input and output threads) and the shell echo. The same
holds on `ssc034a` and `ssc034b`, with the same output files on both.

Both runs print `[ven-w]DRV_VENC_TOP_RiscHostFwDeInit 847 wait top riscv
deinit timeout` when the channel is destroyed: the machine's stand-in for
the encoder firmware does not answer the deinit. The demo needs
`/config/venc_fw/ven_para.cfg` (from `project/board/ifliegen/venc_fw`);
without it `MI_VENC_CreateDev` fails with both module sets alike.

## Not verified

- Hardware: nothing has run on a real SSC034A or SSC034B.
- In QEMU only the encoder does work. The other modules load, register
  their devices and unload, the same way with both sets, but no sensor,
  VIF, ISP, SCL, display, framebuffer, audio, IVE, IPU or VDF pipeline was
  run: the machine does not model those blocks far enough.
- The uClibc product: its objects are the same code (see above), its
  modules were not loaded.
- `bf3901.ko`, `gc032a.ko`: build, and fail to load for want of
  `mi_pspi.ko`, as the SDK's do; never run.
- An OpenIPC kernel: there is none for Ifliegen yet. Against another kernel
  config the modules build, but whether the vendor objects match it is up to
  that config (the objects were built for this SDK kernel).
