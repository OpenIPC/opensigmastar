# OpenSigmaStar

Kernel modules for SigmaStar SoCs built from source, and the headers
applications need to use the vendor's media libraries. Used by
[OpenIPC](https://openipc.org/) firmware, the way
[openhisilicon](https://github.com/OpenIPC/openhisilicon) serves HiSilicon.

SigmaStar ships its media stack (`mhal.ko`, `mi_*.ko`, `libmi_*.so`) as
binaries. Where SigmaStar's own source for a piece turns up, in an SDK, a
boot or kernel release, the piece is rebuilt here for the OpenIPC kernels,
so it can be fixed and does not depend on a blob built for one kernel.

## Supported hardware

| Family | SoCs | CHIPARCH | Kernel |
|--------|------|----------|--------|
| infinity2m | SSD201, SSD202D, SSR621Q | `infinity2m` | [OpenIPC/linux `sigmastar-infinity2m`](https://github.com/OpenIPC/linux/tree/sigmastar-infinity2m) (4.9.84) |
| iford | SSC029A, SSC029B, SSC029C | `iford` | SigmaStar's Iford SDK IMD00V2.1.0 kernel (5.10.117); no OpenIPC kernel yet |
| ifliegen | SSC034A, SSC034B | `ifliegen` | SigmaStar's Ifliegen SDK ILSSV01C12 kernel (4.9.84); no OpenIPC kernel yet |

## SigmaStar families in OpenIPC

Every SigmaStar family, by its chip ID: its codename, the SoCs and boards
known for it, and what OpenIPC has for it. MStar is SigmaStar's former
name, and its parts (MSC313E, MSC316DC, ...) belong here too. The chip ID is the value of the
register at `0x1F003C00`, which the vendor kernels also show in
`/sys/class/mstar/msys/CHIP_ID` (hex) and `/sys/devices/soc0/soc_id`
(decimal); it names the family, not the part.

The code in brackets is the board the device tree or the SDK's config is
named after (`ifliegen_ssc034a_s01a_defconfig`); OpenIPC shows its board in
`/sys/devices/soc0/machine` (`INFINITY6 SSC009A-S01A QFN88`), chosen by the
build, not read from the chip. "Modules here" is the `CHIPARCH` this
repository builds for.

Sources: OpenIPC/firmware defconfigs, OpenIPC/linux `sigmastar-*` branches,
the [OpenIPC/u-boot `sigmastar` release](https://github.com/OpenIPC/u-boot/releases/tag/sigmastar),
the [openipc.org SigmaStar catalogue](https://openipc.org/cameras/vendors/sigmastar),
[ipctool](https://github.com/OpenIPC/ipctool/blob/master/src/hal/sstar.h)
(chip IDs and family names) and its issues
[#18](https://github.com/OpenIPC/ipctool/issues/18) and
[#21](https://github.com/OpenIPC/ipctool/issues/21).

| Family | Codename | Chip ID | SoCs (board) | OpenIPC kernel | Firmware package | Modules here |
|--------|----------|---------|--------------|----------------|------------------|--------------|
| infinity3 | — | 0xC2 (194) | MSC313E, MSC316DC, MSC316DM; and others | [`sigmastar-infinity3`](https://github.com/OpenIPC/linux/tree/sigmastar-infinity3) (3.18.30) | [`sigmastar-osdrv-msc313e`](https://github.com/OpenIPC/firmware/tree/master/general/package/legacy/sigmastar-osdrv-msc313e) | — |
| infinity5 | Pretzel | 0xED (237) | SSC326D, SSC328Q, SSC329Q; and others | — | — | — |
| mercury5 | — | 0xEE (238) | — | — | — | — |
| infinity6 | Macaron | 0xEF (239) | SSC325 (`ssc009a`), SSC325DE (`ssc009b`); SSC323, SSC325D, SSC327, SSC327D, SSC327DE, SSC327Q; and others | [`sigmastar-infinity6`](https://github.com/OpenIPC/linux/tree/sigmastar-infinity6) (4.9.84) | [`sigmastar-osdrv-infinity6`](https://github.com/OpenIPC/firmware/tree/master/general/package/sigmastar-osdrv-infinity6) | — |
| infinity2m | Taiyaki | 0xF0 (240) | SSR621Q (`ssc010a`); SSD201, SSD202D; and others | [`sigmastar-infinity2m`](https://github.com/OpenIPC/linux/tree/sigmastar-infinity2m) (4.9.84) | [`sigmastar-osdrv-infinity2m`](https://github.com/OpenIPC/firmware/tree/master/general/package/sigmastar-osdrv-infinity2m), [`sigmastar-opensdk`](https://github.com/OpenIPC/firmware/tree/master/general/package/sigmastar-opensdk) | `infinity2m` |
| infinity6e | Pudding | 0xF1 (241) | SSC30KD, SSC30KQ, SSC338Q (`ssc012b`); SSC336D, SSC336Q, SSC338D, SSC338G, SSC339G (also marked MSC339G); and others | [`sigmastar-infinity6e`](https://github.com/OpenIPC/linux/tree/sigmastar-infinity6e) (4.9.84) | [`sigmastar-osdrv-infinity6e`](https://github.com/OpenIPC/firmware/tree/master/general/package/sigmastar-osdrv-infinity6e) | — |
| infinity6b0 | Ispahan | 0xF2 (242) | SSC333, SSC335, SSC337 (`ssc009a`); SSC335DE, SSC337DE (`ssc009b`); SSC333DE; and others | [`sigmastar-infinity6b0`](https://github.com/OpenIPC/linux/tree/sigmastar-infinity6b0) (4.9.84) | [`sigmastar-osdrv-infinity6b0`](https://github.com/OpenIPC/firmware/tree/master/general/package/sigmastar-osdrv-infinity6b0) | — |
| mercury6 | Tiramisu | 0xF4 (244) | SSC359G (also marked MSC359G), SSC8836Q-H; and others | — | — | — |
| pioneer3 | Ikayaki | 0xF5 (245) | — | — | — | — |
| infinity6c | Maruko | 0xF9 (249) | SSC377, SSC377D (`ssc027a`); SSC377DE, SSC377QE, SSC378DE, SSC378QE (`ssc027d`); and others | [`sigmastar-infinity6c`](https://github.com/OpenIPC/linux/tree/sigmastar-infinity6c) (5.10.61) | [`sigmastar-osdrv-infinity6c`](https://github.com/OpenIPC/firmware/tree/master/general/package/sigmastar-osdrv-infinity6c) | — |
| infinity6f | Souffle | 0xFB (251) | — | — | — | — |
| infinity6d | Iford | — | SSC029A, SSC029B, SSC029C (SDK board `ssz029c`); and others | — (SDK kernel 5.10.117) | — | `iford` |
| infinity6g (probably) | Ifliegen | — | SSC385 (`ssc034a`, QFN128); `ssc034b` (QFN88); SSC385D, SSC387 (probably); and others | — (SDK kernel 4.9.84) | — | `ifliegen` |
| M6P (Mercury6P) | Mochi | — | — | — | — | — |
| — | Ifado | — | SSC373 (`ssc032b`); and others | — | — | — |
| — | — | — | SSC375 | — | — | — |
| — | Muffin | — | — | — | — | — |
| — | Opera | — | — | — | — | — |
| — | Pcupid | — | — | — | — | — |
| — | Ibopper | — | — | — | — | — |
| — | Ifackel | — | — | — | — | — |
| — | Jaguar1 | — | — | — | — | — |

### Open questions

What the sources leave open or disagree on, to be settled as information
comes in:

- Ifliegen is probably infinity6g; nothing published says so yet.
- SSC385 sits on the `ssc034a` board (Ifliegen). The openipc.org catalogue
  puts SSC385, SSC385D and SSC387 in a family of their own, SSC38X; whether
  SSC38X is Ifliegen is not confirmed.
- The part on the `ssc034b` board (QFN88) is not known.
- SSC029A, SSC029B and SSC029C look like board codes rather than part
  names (the Iford SDK's config is `iford_ssz029c_s01b_nor_demo_defconfig`);
  the Iford part names are not known. The OpenIPC/u-boot `sigmastar` release
  lists SSC029A/B/C and SSC034A/B among its SoCs too.
- SSC375 is in the OpenIPC/u-boot `sigmastar` release; its family is not
  known.
- Mochi is M6P (Mercury6P) as one source recalls it; its chip ID and parts
  are not known.
- Muffin, Opera, Pcupid, Ibopper, Ifackel and Jaguar1: family, chip ID and
  parts not known.
- infinity6c boards: one source gives `ssc027a` as QFN88 and `ssc027b` as
  QFN128; OpenIPC firmware builds its QFN128 parts (DE, QE) for `ssc027d`.
- The board of SSC333DE is not known.
- The openipc.org catalogue says ipctool does not know SSR621Q, but ipctool
  reports it as `SSD20X`; it does not list SSD201, SSD202D, SSC375 or the
  Iford and Ifliegen parts.

## Modules

Two kinds, as in openhisilicon:

- **From source**: built from SigmaStar's own source (from an SDK, boot or
  kernel release) or from this repository's.
- **Relinked**: a vendor module that exists only as a finished `.ko`, turned
  into an object by `scripts/ko2obj.sh` (from openhisilicon) and linked
  again for the kernel at hand with a small init wrapper
  (`kernel/init/<CHIPARCH>/`). The code is SigmaStar's, unchanged (in two
  infinity2m objects a few path strings are rewritten, below). It gets this
  kernel's vermagic and `struct module`, so it follows the OpenIPC kernel
  rather than the SDK's build of it. Each keeps the vendor module's licence.

### infinity2m (NVR)

The whole media stack an SSR621Q NVR loads, in this order:

| Module | Kind | What it is |
|--------|------|------------|
| `open_mhal.ko` | relinked + display/panel/regions from source | MHAL: display, GOP/regions, panel (these three from SigmaStar's source, below), GE, DIVP, CMDQ, audio, JPEG/H.264/H.265 encoder HALs |
| `open_mi_common.ko`, `open_mi_sys.ko` | relinked | MI core, buffers, binding |
| `open_mi_ao.ko`, `open_mi_gfx.ko`, `open_mi_divp.ko`, `open_mi_vdec.ko`, `open_mi_ai.ko`, `open_mi_ipu.ko`, `open_mi_disp.ko`, `open_mi_venc.ko`, `open_mi_panel.ko` | relinked | MI modules (the SDK's `.mods_depend` order); each makes `/dev/mi_<x>` |
| `open_fbdev.ko` | relinked | GOP framebuffer, `/dev/fb0` (`default_config_path_file=<fbdev.ini>`) |
| `open_hdmitx.ko` | source | HDMI transmitter (below) |
| `open_hdmi.ko` | source | switches HDMI on (below) |

The relinked modules come from the i2m NVR SDK (2020-03), the release the
MI libraries and headers in `include/infinity2m` match. What the SDK puts
in `/config` they find elsewhere:

- `open_mi_sys` runs `<default_config_path>/config_tool` while it loads
  (memory map, panel timings, PQ defaults) and waits for it; OpenIPC
  firmware passes `default_config_path=/usr/lib/sigmastar` and builds its own
  `config_tool` from source (sigmastar-osdrv-infinity2m).
- `open_fbdev` reads the file named by `default_config_path_file`.
- `open_mi_vdec` loads its decoder firmware from
  `/lib/firmware/chagall-normal.bin` and `/lib/firmware/chagall-latency.bin`:
  these paths, and two debug commands that called an SDK tool in `/config`,
  are rewritten in the committed objects (`scripts/restring.py`,
  `kernel/obj/infinity2m/restring.txt`, [docs/infinity2m.md](docs/infinity2m.md)).

### iford and ifliegen

These SDKs ship each MI kernel module as an object plus its init and export
glue in source (`sdk/linux`), which they link against the kernel at build
time. The same is done here in plain Kbuild, with the SDK's names, so the
modules are drop-in replacements for the SDK's `mi_*.ko`.

| Modules | CHIPARCH | Notes |
|---------|----------|-------|
| `mi_common`, `mi_sys`, `mi_sensor`, `mi_rgn`, `mi_disp`, `mi_fb`, `mi_dummy`, `mi_aio`, `mi_vif`, `mi_isp`, `mi_scl`, `mi_ive`, `mi_ai`, `mi_ao`, `mi_ldc`, `mi_shadow`, `mi_venc`, `mi_vdisp`, `mi_ipu`, `mi_debug` | iford | Objects from the SDK's `obj/uclibc/9.1.0` set. The kernel must be built with GCC plugins off (those objects use the global stack canary). See [docs/iford.md](docs/iford.md). |
| `mi_common`, `mi_sys`, `mi_rgn`, `mi_sensor`, `mi_vif`, `mi_scl`, `mi_isp`, `mi_venc`, `mi_ive`, `mi_ipu`, `mi_vdf`, `mi_vdisp`, `mi_disp`, `mi_fb`, `mi_aio`, `mi_ai`, `mi_ao`, `mi_dummy`, `mi_debug` | ifliegen | See [docs/ifliegen.md](docs/ifliegen.md). |
| `bf3901.ko`, `gc032a.ko` | ifliegen | PSPI sensor drivers, full source (`sdk/driver/PspiDriver`). They need `mi_pspi.ko`, which the SDK does not build for Ifliegen, so they build but do not load (nor do the SDK's). |

The vendor objects are kept without their debug sections; symbols, code and
relocations are unchanged.

### Modules from source

| Module | CHIPARCH | What it is | Source |
|--------|----------|------------|--------|
| `open_hdmitx.ko` | infinity2m | HDMI transmitter driver: the `sstar,hdmitx` platform driver, exporting the `MhalHdmitx*` API the vendor's `mi_hdmi` calls. The i2m NVR SDK's `mhal.ko` has no HDMI TX. | SigmaStar, `boot_621_release_20200522` `drivers/mstar/hdmitx`, its Linux OS layer (`kernel/hdmitx`) |
| `open_mhal.ko` (part) | infinity2m | Display (`MHAL_DISP_*`), panel (`MhalPnl*`) and regions/GOP (`MHAL_RGN_*`) HALs with their Linux OS layers, linked into the relinked `mhal` in place of the SDK's own copies; see [docs/infinity2m.md](docs/infinity2m.md). | SigmaStar, `boot_621_release_20200522` `drivers/mstar/{disp,panel,rgn,common}` (`kernel/disp`, `kernel/panel`, `kernel/rgn`, `kernel/common`) |
| `open_hdmi.ko` | infinity2m | Switches the HDMI output on through that API, from sysfs: `/sys/kernel/open_hdmi/output` (write `1080p60`, `1080p50`, `720p60`, `720p50`, `1024x768p60`, `1280x1024p60`, `1440x900p60` or `off`), `hpd`, `edid`. Parameters `output=` (on at load) and `color=yuv444\|rgb444`. | this repository (`kernel/hdmi`) |

MI_DISP itself (`mi_disp.ko`) is still the SDK's; the display HAL under it
is now from source. Start the display first, then write the same timing to
`open_hdmi/output`.

## Build

`scripts/ko2obj.sh` made `kernel/obj/<CHIPARCH>/*.o_shipped`, and for
infinity2m `scripts/restring.py` then rewrote the strings listed in
`kernel/obj/infinity2m/restring.txt`; they are only needed to add or
remake a module. `scripts/weaken.sh` runs during the build: it lets
the disp/panel/rgn source objects replace their vendor copies inside
`open_mhal.ko` ([docs/infinity2m.md](docs/infinity2m.md)). Against an OpenIPC SigmaStar kernel tree that has been built:

```sh
make -C kernel KDIR=<kernel> CHIPARCH=infinity2m \
     ARCH=arm CROSS_COMPILE=arm-openipc-linux-gnueabihf-
```

iford and ifliegen build the same way against their SDK kernels; the docs
pages give the kernel configs and toolchains.

OpenIPC firmware builds it as the `sigmastar-opensdk` package.

### Kernel ABI of the relinked modules

A relinked module still has the SDK kernel's structure layouts, inline
code and constants compiled in. `scripts/kabi-check.py` compares a kernel
tree with what the infinity2m vendor modules expect
(`kernel/kabi/infinity2m.json`, from the SDK modules' DWARF and
disassembly):

```sh
make -C kernel kabi-check KDIR=<kernel> CHIPARCH=infinity2m \
     ARCH=arm CROSS_COMPILE=arm-openipc-linux-gnueabihf-
```

OpenIPC firmware's configuration passes; several common options must not
change (`PREEMPT`, `SMP`, `DEBUG_SPINLOCK`, `DEBUG_MUTEXES`, `HZ`,
`VMSPLIT_3G`, ...), and the modules cannot move to 5.10 or later. See
[docs/kabi.md](docs/kabi.md), which also covers `KCOMPAT=1`, a study build
of the modules for a 5.10 kernel with `open_kcompat.ko`.

## Headers

`include/<CHIPARCH>/` holds the MI headers (`mi_*.h`) that match the
family's libraries: for infinity2m with `sstarFb.h`, the ones OpenIPC
firmware ships; for iford and ifliegen, the SDK's `release/include` as
shipped, which their module builds use too. For
infinity2m they match the i2m NVR SDK (2020-03). That SDK's `libmi_vdec`
has no `stErrHandlePolicy` in `MI_VDEC_VideoAttr_t` (4 bytes, from its
DWARF), so the header leaves it out.

## Status

infinity2m: the whole set builds against OpenIPC/linux `sigmastar-infinity2m`
and runs in qemu-bench's `ssr621q` machine on an OpenIPC firmware image,
where an NVR application decodes four cameras (H.264 and H.265) to the
display with it, as with the SDK's own `.ko` files. The machine does not
model the HDMI transmitter's signal, and **nothing has run on hardware** yet.

The display, panel and region parts of `open_mhal.ko` come from source
(boot release, 2020-05) and run the same NVR scene in that machine; how they
are combined with the SDK's `mhal` (2020-03) and what was checked is in
[docs/infinity2m.md](docs/infinity2m.md).

iford and ifliegen build against the SDK kernels and match the SDK's
prebuilt modules (exported and undefined symbols, vermagic, parameters). In
qemu-bench's `ssc029` the Iford set runs the SDK's `preview` (sensor, VIF,
ISP, scaler, H.264) to RTSP with the same decoded frames as SigmaStar's own
modules; in `ssc034a`/`ssc034b` the Ifliegen set gives byte-identical
`venc_demo` H.264 and H.265 output, and loads and unloads cleanly. Not run on
hardware.

## License

The repository is GPL-3.0 (`LICENSE`). `kernel/hdmi` and `kernel/kcompat`
are GPL-2.0, as kernel modules. The vendor sources under `kernel/hdmitx`,
`kernel/disp`, `kernel/panel`, `kernel/rgn` and `kernel/common` and the headers
under `include/` keep SigmaStar's own notices.
