# infinity2m: display, panel and regions from source inside `open_mhal.ko`

The i2m NVR SDK (2020-03) ships `mhal.ko` as one closed module: display
(disp), panel, region/GOP (rgn), GE, DIVP, CMDQ, audio, IMI heap and the
encoder HALs. SigmaStar's SSR621 boot release (`boot_621_release_20200522`)
carries the source of three of those parts, with their Linux OS layers:
`drivers/mstar/disp`, `drivers/mstar/panel`, `drivers/mstar/rgn` (and the
shared `drivers/mstar/common` headers). They are in `kernel/disp`,
`kernel/panel`, `kernel/rgn` and `kernel/common`, as released except for one
header line (below). The rest of `mhal` has no source, so `open_mhal.ko` is
both: the relinked vendor object plus these three parts built from source.

## How the two are combined

1. Kbuild compiles the disp, panel and rgn sources (`kernel/infinity2m.kbuild`,
   `DISP_OBJS`, `PNL_OBJS`, `RGN_OBJS`) and `kernel/mhal/infinity2m/`.
2. `scripts/weaken.sh` takes the vendor object (`obj/infinity2m/mhal.o`, from
   `mhal.o_shipped`), lists every global symbol that both it and a source
   object define, and writes `obj/infinity2m/mhal-open.o` with those symbols
   made weak (`objcopy --weaken-symbol`). This runs at build time, so the
   list always follows the source; with this release it is 910 symbols.
3. `open_mhal.ko` links `mhal-open.o`, the source objects and the init
   wrapper. For each of those symbols the strong definition, the source's,
   wins, and every reference in the module resolves to it:
   - `camdriver_init` (the vendor's module init) calls `_DrvPnlModuleInit`
     and `_DrvDispModuleInit`: both now run the source.
   - the vendor `__ksymtab` entries for `MHAL_DISP_*` (43), `MHAL_RGN_*` (31),
     `MhalPnl*` (18), `DrvDispOs*`, `DrvPnlOsMem*`, `HalRgn*` (102 in all)
     relocate against the symbol, so the exports the vendor `mi_disp`,
     `fbdev` and `mi_panel` import are the source functions.
   - the vendor `__param` entry `geGopOsdFlag` points at the source variable.

   The vendor's own disp/panel/rgn code stays in the module (about 80 KiB of
   its `.text`), no longer reachable.

This works because the vendor object reaches its global functions and data
through relocations against the symbol (`R_ARM_THM_CALL _DrvDispModuleInit`,
`R_ARM_ABS32 MHAL_DISP_DeviceCreateInstance` in `__ksymtab`), not against
`.text` plus an offset. It would not work for a single function inside a
part, since calls to static or inlined functions are resolved by the
compiler; that is why whole parts are replaced.

Details:

- **Exports.** The source's `EXPORT_SYMBOL`s would duplicate the vendor's
  `__ksymtab` entries and modpost would refuse them, so the source objects
  are built with `kernel/mhal/infinity2m/no_export.h` forced first, which
  makes `EXPORT_SYMBOL` empty. The vendor's export table stays the only one;
  it already exports every name the source would (the source exports nothing
  the vendor does not). `drv_rgn_export.c`, nothing but exports, is not built.
- **No second `init_module`.** `drv_disp_module.c` and `drv_pnl_module.c`
  (platform drivers with `module_init`) are not built. In the vendor `mhal`
  they were compiled as built-in code, so their `module_init` became an entry
  in `.initcall6.init`, which a module never runs: the vendor module never
  called them either. `camdriver_init` calls `_Drv{Disp,Pnl}ModuleInit`
  directly, as before. Region code has no init of its own (the SDK's
  `rgn_module.c` init is an empty function in that same dead initcall list).
- **What the source lacks.** `MHAL_DISP_DeviceSetTimeZoneConfig` is exported
  by the SDK's `mhal` and called once by its `mi_disp` (from
  `MI_DISP_IMPL_SetPubAttr`, result ignored), but the boot release has no such
  function: its `MHAL_DISP_DeviceGetTimeZone` reads the zone from the
  display's VSYNC/VDE interrupt flags rather than from configured
  percentages. `kernel/mhal/infinity2m/mhal_disp_compat.c` accepts the call
  and drops the configuration. The disp OS layer calls `ImiHeapMalloc` /
  `ImiHeapFree` from the vendor `mhal`'s IMI heap; `imi_heap.h` declares them
  as the vendor DWARF does. The `open_mhal` parameter `gGopDmaThd` still
  exists (the vendor `__param` entry) but nothing reads it any more: the
  boot release's rgn has no such variable.
- **Build options.** disp is built with `HDMITX_VGA_SUPPORTED`, as the SSR621
  boot configuration does (HDMI and VGA outputs, DAC trimming, HDMI/DAC
  clocks).
- **Include paths** are set per object (`CFLAGS_<name>.o`), for hdmitx too:
  hdmitx and disp/panel/rgn have different `mhal_common.h` and forced
  includes. `open_hdmitx.ko` and `open_hdmi.ko` build to the same code as
  before.

## Checks against the vendor modules

The SDK modules keep their DWARF (`mhal.ko`, `mi_disp.ko`, `fbdev.ko`,
`mi_panel.ko`). gdb `ptype /o` of every type in `mhal_disp_datatype.h`,
`mhal_rgn_datatype.h`, `mhal_pnl_datatype.h`, `mhal_common.h` and
`mhal_cmdq.h` was compared between those modules and the source objects
(offsets, sizes, member names, enumerators), and `ptype` of every exported
function.

- **disp**: all types the same as `mi_disp`, `fbdev` and the vendor `mhal`
  see them; `MHAL_DISP_TimeZoneType_e` only lacks a trailing `_NUM`
  sentinel. All 48 exported prototypes the same.
- **`MHalPixelFormat_e`** (`mhal_common.h`) differed: the SDK has
  `YUV422_UYVY`, `YUV422_YVYU`, `YUV422_VYUY` after `YUV_MST_420`, the boot
  release does not, so every later value (the decoder's tile formats among
  them) would be read 3 lower. `kernel/common/mhal_common.h` has those three
  lines added, marked; this is the only change to the released source.
- **rgn**: all types the same; all 34 exported prototypes the same.
  `MHAL_CMDQ_CmdqInterface_t` (handed in by `MHAL_RGN_SetupCmdQ`) has one more
  trailing member in the SDK (`MHAL_CMDQ_SetWeight`, 100 bytes against 96);
  the source only calls members before it. The `MHAL_CMDQ_EventId_e` values
  differ too, but no disp/panel/rgn source uses them.
- **panel**: all 20 prototypes the same, and all types except two, which are
  longer in the SDK by trailing members: `MhalPnlParamConfig_t` (148 bytes
  against 144, `enPacketType`) and `MhalPnlMipiDsiConfig_t` (76 against 68,
  `u8PolCh0..4`, `enPacketType`). Both are passed by pointer, so the source
  reads and writes only the common prefix; MIPI DSI lane polarity and packet
  type set through `mi_panel` are ignored. An HDMI/VGA NVR has no panel;
  `MhalPnlLinkType_e` gained values at its end.
- **Who calls what.** From the vendor `mhal`'s DWARF, every relocation that
  targets a global of the vendor's disp/panel/rgn compile units was listed
  with the place it comes from. Outside those parts there are only
  `camdriver_init` (`_DrvDispModuleInit`, `_DrvPnlModuleInit`) and the
  `__ksymtab` entries, all now resolving to the source; every weakened symbol
  is defined in one of those compile units (none belongs to GE, DIVP or
  another part), and no vendor code outside them reaches a vendor
  disp/panel/rgn symbol that stayed. The opaque context pointers
  (`void *pDevCtx`, ...) are only passed back by the MI modules.

## Verified in QEMU

qemu-bench's `ssr621q` machine, OpenIPC SSR621Q firmware image with the
modules stripped (`--strip-unneeded`) in `/lib/modules/4.9.84/sigmastar`,
loaded by `load_sigmastar`; vigild with four emulated cameras (three
H.264, one H.265) in a 2x2 view:

- all modules load, no oops; `/dev/mi_{sys,ao,gfx,divp,vdec,ai,disp,panel,poll}`
  and `/dev/fb0` exist; `/sys/kernel/open_hdmi/output` reads `720p60`;
- the screen shows the four decoded cameras and the GOP status UI, as with
  the vendor `mhal`;
- with temporary `printk`s (removed again): `_DrvPnlModuleInit` and
  `_DrvDispModuleInit` ran from the source at load, `MHAL_RGN_GopInit` when
  `fbdev` started, `MHAL_DISP_DeviceCreateInstance` when `fbdev` and
  `mi_disp` opened the device;
- `/proc/kallsyms`: `_DrvDispModuleInit`, `_DrvPnlModuleInit`,
  `MHAL_DISP_DeviceCreateInstance` and `MHAL_RGN_GopInit` lie at offsets
  0x30b21, 0x3ad79, 0x33905 and 0x3ec21 of the module's `.text`, past the
  vendor object's 0x2ebf0 bytes, i.e. in the source objects.

`[_MI_DISP_DevISR][2504]irq flag:0 mask:1` fills the kernel log in that
machine with the vendor `mhal` as much as with this one (the machine's
display interrupt). Panel functions beyond init are not exercised (no
panel). Nothing has run on hardware yet.

## Kernel configuration and version

The relinked parts depend on the kernel's configuration beyond vermagic:
[kabi.md](kabi.md) lists the options that must stay as OpenIPC firmware
sets them, and `make -C kernel kabi-check` checks a kernel tree. The SDK
built these modules with `CONFIG_LBDAF` off; the firmware has it on, which
moves `struct inode` members the vendor code is not seen to use.
