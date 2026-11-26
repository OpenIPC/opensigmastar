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

| Module | CHIPARCH | What it is | Source |
|--------|----------|------------|--------|
| `open_hdmitx.ko` | infinity2m | HDMI transmitter driver: the `sstar,hdmitx` platform driver, exporting the `MhalHdmitx*` API the vendor's `mi_hdmi` calls. The i2m NVR SDK's `mhal.ko` has no HDMI TX. | SigmaStar, `boot_621_release_20200522` `drivers/mstar/hdmitx`, its Linux OS layer (`kernel/hdmitx`) |
| `open_hdmi.ko` | infinity2m | Switches the HDMI output on through that API, from sysfs: `/sys/kernel/open_hdmi/output` (write `1080p60`, `1080p50`, `720p60`, `720p50`, `1024x768p60`, `1280x1024p60`, `1440x900p60` or `off`), `hpd`, `edid`. Parameters `output=` (on at load) and `color=yuv444\|rgb444`. | this repository (`kernel/hdmi`) |

The display itself (MI_DISP) still comes from the SDK's `mhal.ko` and
`mi_disp.ko`. Start the display first, then write the same timing to
`open_hdmi/output`.

## Build

Against an OpenIPC SigmaStar kernel tree that has been built:

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

Built and loaded in qemu-bench's `ssr621q` machine. That machine does not
model the HDMI transmitter, so the output itself is **not verified on
hardware** yet.

Candidates for later: the same boot release carries the Linux OS layers of
`disp`, `panel` and `rgn`. On infinity2m those are inside the SDK's
`mhal.ko`, so rebuilding them means replacing that blob.

## License

The repository is GPL-3.0 (`LICENSE`). `kernel/hdmi` is GPL-2.0, as a
kernel module. The vendor sources under `kernel/hdmitx` and the headers
under `include/` keep SigmaStar's own notices.
