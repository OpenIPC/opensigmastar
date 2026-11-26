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

## Modules

Two kinds, as in openhisilicon:

- **From source**: built from SigmaStar's own source (from an SDK, boot or
  kernel release) or from this repository's.
- **Relinked**: a vendor module that exists only as a finished `.ko`, turned
  into an object by `scripts/ko2obj.sh` (from openhisilicon) and linked
  again for the kernel at hand with a small init wrapper
  (`kernel/init/<CHIPARCH>/`). The code is SigmaStar's, unchanged. It gets this
  kernel's vermagic and `struct module`, so it follows the OpenIPC kernel
  rather than the SDK's build of it. Each keeps the vendor module's licence.

### infinity2m (NVR)

The whole media stack an SSR621Q NVR loads, in this order:

| Module | Kind | What it is |
|--------|------|------------|
| `open_mhal.ko` | relinked | MHAL: display, GOP/regions, panel, GE, DIVP, CMDQ, audio, JPEG/H.264/H.265 encoder HALs |
| `open_mi_common.ko`, `open_mi_sys.ko` | relinked | MI core, buffers, binding |
| `open_mi_ao.ko`, `open_mi_gfx.ko`, `open_mi_divp.ko`, `open_mi_vdec.ko`, `open_mi_ai.ko`, `open_mi_ipu.ko`, `open_mi_disp.ko`, `open_mi_venc.ko`, `open_mi_panel.ko` | relinked | MI modules (the SDK's `.mods_depend` order); each makes `/dev/mi_<x>` |
| `open_fbdev.ko` | relinked | GOP framebuffer, `/dev/fb0` (`default_config_path_file=/config/fbdev.ini`) |
| `open_hdmitx.ko` | source | HDMI transmitter (below) |
| `open_hdmi.ko` | source | switches HDMI on (below) |

The relinked modules come from the i2m NVR SDK (2020-03), the release the
MI libraries and headers in `include/infinity2m` match. They read their
configuration from `/config` (panel timings, PQ, memory map, decoder
firmware), a path built into them; OpenIPC firmware ships it.

### Modules from source

| Module | CHIPARCH | What it is | Source |
|--------|----------|------------|--------|
| `open_hdmitx.ko` | infinity2m | HDMI transmitter driver: the `sstar,hdmitx` platform driver, exporting the `MhalHdmitx*` API the vendor's `mi_hdmi` calls. The i2m NVR SDK's `mhal.ko` has no HDMI TX. | SigmaStar, `boot_621_release_20200522` `drivers/mstar/hdmitx`, its Linux OS layer (`kernel/hdmitx`) |
| `open_hdmi.ko` | infinity2m | Switches the HDMI output on through that API, from sysfs: `/sys/kernel/open_hdmi/output` (write `1080p60`, `1080p50`, `720p60`, `720p50`, `1024x768p60`, `1280x1024p60`, `1440x900p60` or `off`), `hpd`, `edid`. Parameters `output=` (on at load) and `color=yuv444\|rgb444`. | this repository (`kernel/hdmi`) |

The display itself (MI_DISP) still comes from the SDK's `mhal.ko` and
`mi_disp.ko`. Start the display first, then write the same timing to
`open_hdmi/output`.

## Build

`scripts/ko2obj.sh` made `kernel/obj/<CHIPARCH>/*.o`; it is only needed to
add a module. Against an OpenIPC SigmaStar kernel tree that has been built:

```sh
make -C kernel KDIR=<kernel> CHIPARCH=infinity2m \
     ARCH=arm CROSS_COMPILE=arm-openipc-linux-gnueabihf-
```

OpenIPC firmware builds it as the `sigmastar-opensdk` package.

## Headers

`include/<CHIPARCH>/` holds the MI headers (`mi_*.h`) and `sstarFb.h` that
match the libraries the OpenIPC firmware ships for that family. For
infinity2m they match the i2m NVR SDK (2020-03). That SDK's `libmi_vdec`
has no `stErrHandlePolicy` in `MI_VDEC_VideoAttr_t` (4 bytes, from its
DWARF), so the header leaves it out.

## Status

infinity2m: the whole set builds against OpenIPC/linux `sigmastar-infinity2m`
and runs in qemu-bench's `ssr621q` machine on an OpenIPC firmware image,
where an NVR application decodes four cameras (H.264 and H.265) to the
display with it, as with the SDK's own `.ko` files. The machine does not
model the HDMI transmitter's signal, and **nothing has run on hardware** yet.

Candidates for later: the same boot release carries the Linux OS layers of
`disp`, `panel` and `rgn`. On infinity2m those are inside the SDK's
`mhal.ko`, so rebuilding them means replacing that blob.

## License

The repository is GPL-3.0 (`LICENSE`). `kernel/hdmi` is GPL-2.0, as a
kernel module. The vendor sources under `kernel/hdmitx` and the headers
under `include/` keep SigmaStar's own notices.
