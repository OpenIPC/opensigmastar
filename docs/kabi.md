# Kernel ABI of the relinked infinity2m modules

`open_mhal.ko`, `open_mi_*.ko` and `open_fbdev.ko` are SigmaStar's code
from the i2m NVR SDK (2020-03), compiled against that SDK's 4.9.84 kernel
headers. Relinking gives them the running kernel's vermagic and
`struct module`, but everything else the compiler took from those headers
is still in the code:

- the layout of every kernel structure the vendor code allocates,
  initialises at build time (`DEFINE_MUTEX`, a `file_operations` table, a
  `platform_driver`) or dereferences (`file->private_data`,
  `vma->vm_pgoff`, `info->var.xres`, `current->mm`);
- inline header code: `kmalloc()` of a constant size indexing
  `kmalloc_caches[]`, `current` (sp masked with `THREAD_SIZE`),
  `set_fs()` writing `thread_info->addr_limit`, `copy_from_user()` with the
  PAN domain switch, `msecs_to_jiffies()` of constants (`HZ`), `idr_find()`;
- constants: `GFP_*` bits, `PAGE_OFFSET`, `VM_*`, `O_*`, pte bits;
- the kernel functions it calls, by name, with the argument list it was
  built for.

A kernel built from the same source with another configuration, or a newer
kernel, can differ in any of these, and nothing at load time notices.
`scripts/kabi-check.py` compares them.

## The checker

```sh
make -C kernel kabi-check KDIR=<kernel build tree> CHIPARCH=infinity2m \
     ARCH=arm CROSS_COMPILE=arm-openipc-linux-gnueabihf-
```

or `scripts/kabi-check.py --cross <prefix> check --json kernel/kabi/infinity2m.json --kdir <tree>`
(`--all` also lists the harmless differences, `--save res.json` writes the
result, `--dwarf vmlinux` reads a vmlinux built with `CONFIG_DEBUG_INFO`
instead of building the probe). Exit status 1 when something the vendor
code depends on differs. It needs Python 3 and the toolchain's binutils
(`readelf`, `objdump`), no Python packages.

The target tree only has to be configured and prepared
(`make modules_prepare`); a built one (with `Module.symvers`) also gets
its exports checked. Its `CONFIG_DEBUG_INFO` does not matter: the checker
builds a probe object against the tree with `-g` of its own. The probe
includes the headers the vendor code was compiled with (`__has_include`,
a header that does not compile there is moved last, then left out), takes
`__typeof__(&sym)` of every kernel symbol the modules import, and holds an
enum of the constants and `IS_ENABLED()` options below; its DWARF gives the
target's layouts, prototypes and values. About 5 seconds.

### What it knows about the vendor modules: `kernel/kabi/infinity2m.json`

Made once by `scripts/kabi-check.py extract` from the SDK's original `.ko`
files, which still carry their DWARF (the `.o_shipped` here do not):

- **473 kernel types** the vendor code reaches (454 from the kernel's
  `include/` and `arch/`, 19 from its `drivers/sstar/include`: CamOs and
  msys structures; SigmaStar's own MI/MHAL types are left out): size,
  members with offset, size and type (bitfields to the bit, anonymous
  unions flattened), enumerators, typedefs; where each is defined and the
  `CONFIG_` options its definition tests.
- **How the vendor code uses each**, from the DWARF of the vendor functions
  and variables: `storage` (an object of that type in vendor memory:
  global, static, stack, or embedded in a vendor structure), `import` (a
  kernel variable it reads: `init_task`, `registered_fb`, `mem_map`,
  `contig_page_data`, ...), `inline` (a kernel header function inlined into
  vendor code works on it), `deref` (the vendor code holds a pointer to it),
  and the pointer distance from those. The report ranks by this: **[A]**
  objects in vendor memory (112 types), **[B]** used directly (32),
  **[C]**/**[D]** one or more pointers away (197, 132).
- **Which members it touches** (38 types; 429 instructions, and the
  members vendor static initialisers set): a dataflow pass over
  the Thumb-2 disassembly of every vendor function. A register holds a
  pointer to a kernel structure when it comes from a parameter (typed by the
  DWARF), from a kernel function returning one, from a kernel variable,
  from `current` (`sp` masked to the `thread_info`; the masks found say
  `THREAD_SIZE` 8192), or from loading a pointer member; loads and stores
  through it are member accesses. State is merged by intersection over the
  basic blocks, so it finds fewer accesses than the code makes (pointers
  spilled to the stack, register-indexed accesses), not more. Statically
  initialised vendor objects of kernel types (from the ELF symbol table,
  their non-zero and relocated words) count as writes to those members.
- the **318 symbols** the modules import from the kernel (183 kernel, 135
  SigmaStar: CamOs, MIU/MMU, msys, padmux...), with the prototype the SDK
  headers declare (297 are declared there);
- the **constants** and options of the SDK kernel (`THREAD_SIZE`,
  `PAGE_OFFSET`, `HZ`, `KMALLOC_*`, `GFP_*`, `VM_*`, pte bits: 65, and 73
  options);
- the **13 inline `kmalloc()` sites** and the `kmalloc_caches[]` index each
  uses (6, 7, 9, 12: 64 to 4096 bytes), from the literal pool the code loads
  `kmalloc_caches` from and the load that follows;
- the 105 kernel inline functions compiled into the vendor code, and the
  licence of each module (an import that is `EXPORT_SYMBOL_GPL` in the
  target is a problem for the proprietary ones).

The types were checked against the SDK kernel itself: the probe built
against OpenIPC/linux `sigmastar-infinity2m` (the SDK's kernel source; its
headers are those of the SDK's `kernel.tgz` plus two new files) gives all
473 exactly as the vendor DWARF has them, **with `CONFIG_LBDAF` off**. The
SDK's prebuilt modules were not built with its `ssc010a` defconfigs (which
have `CONFIG_LBDAF=y`) but with a configuration like
`infinity2m_ssc011a_s01a_defconfig` (`# CONFIG_LBDAF is not set`).
`kernel/kabi/infinity2m.json` holds the vendor DWARF's layouts, and the
constants and prototypes of that tree (OpenIPC firmware's
`infinity2m-ssc010a.config` minus `LBDAF`).

### How a difference is judged

- `!` breaks the vendor code: a member it reads or writes moved, changed
  size or is gone; an [A] type grew (the kernel writes past the vendor's
  object) or is gone; the kernel calls a vendor function pointer with
  another argument list; an import is not exported, is GPL-only for a
  proprietary module, or takes its arguments differently; a constant the
  inline code baked in changed; a `kmalloc_caches[]` index has no cache.
- `?` changed where the vendor code may care but no access to the changed
  members was found: [B]–[D] types whose layout moved, [A] types that
  shrank (the vendor's object still holds the kernel's), pointer arguments
  to renamed or changed structures. To review.
- not shown (`--all` shows them): members appended to types the vendor
  code only reaches by pointer, return values the vendor code never had.

## The OpenIPC kernel with the firmware's config

OpenIPC/linux `sigmastar-infinity2m` with firmware's
`infinity2m-ssc010a.config`, as built for the firmware:

| | |
|---|---|
| types | 460 of 473 identical; 13 differ, all through `CONFIG_LBDAF=y` (`sector_t`, `blkcnt_t`: `struct inode` from `i_blocks` on, `bio`, `request`, `gendisk`, `hd_struct`, quota): `?` |
| members the vendor code touches in those | `inode->i_rdev` (offset 44) and `inode->i_size` (48), both before the first moved member (104) |
| exports | all 318 exported; every declared prototype identical; no GPL-only import in a proprietary module |
| constants, options | all identical |
| `kmalloc_caches` | `[14]`, indexes 6, 7, 9 and 12 hold the same size classes |

So the current configuration is ABI-identical for everything the blobs are
seen to use; the one difference is `CONFIG_LBDAF`, which the firmware wants
for disks over 2 TiB and which reaches no member the vendor code touches.
With `# CONFIG_LBDAF is not set` all 473 types are identical.

The same with `CONFIG_DEBUG_INFO=y` (and `DWARF4`, not `REDUCED`) gives a
byte-identical result. Reading the types from that build's `vmlinux` DWARF
instead of the probe (`--dwarf`, 20 s) gives the same 13 differences; the
two CamOs list types the kernel itself never uses are not in the vmlinux
DWARF at all, which is why the probe is the default.

### Options that must stay as they are

Each option below was changed alone in the firmware config, the kernel
built (`make all`), checked, and the relinked modules built against it
(modpost):

| Option (firmware value) | Change | Result |
|---|---|---|
| `PREEMPT=y` | `PREEMPT_NONE` or `PREEMPT_VOLUNTARY` | `!` `task_struct`: `mm`, `pid`, `tgid`, `real_parent`, `pids` move (9 places in `mi_sys`, `mi_common`) |
| `SMP=y` | off | `!` `arch_spinlock_t` empties (vendor spinlocks), `task_struct` |
| `DEBUG_SPINLOCK=y` | off | `!` spinlocks 16 → 4 bytes: `file` (`f_flags`, `f_pos`, `private_data`), `seq_file.private`, `fb_info` (28 members), `platform_device`; static `mutex`, `wait_queue_head`, `semaphore` initialisers (magic `0xdead4ead`) |
| `DEBUG_MUTEXES=y` | off | `!` the vendor's static `DEFINE_MUTEX`es (fbdev, mi_ai, ...) set `magic` at +32, which becomes `osq` |
| `DEBUG_LOCK_ALLOC`, `PROVE_LOCKING` (off) | on | the kernel does not build (CamOs: `CamOsMutex_t size define not enough`); `!` every lock type |
| `TIMER_STATS` (off) | on | the kernel does not build (`CamOsTimer_t size define not enough`); `!` `timer_list`, `delayed_work` |
| `ARM_LPAE` (off) | on | the kernel does not build (`drivers/sstar/cpu/memory.c`); `!` `phys_addr_t`, `resource`, `vm_area_struct`, pte bits, 6 prototypes |
| `PM` (off) | on | the kernel does not link (`mach-sstar/infinity2m/pm.c`); `!` `device`, `dev_pm_info`, `platform_device.dev.of_node` |
| `VMSPLIT_3G` | `VMSPLIT_2G` | `!` `PAGE_OFFSET`: mi_sys tests kernel addresses against `0xc0000000` (`mi_sys_buf_map_kern_va`, `mi_sys_buf_mgr_flush_inv_cache`) |
| `HZ_100` | `HZ_1000` | `!` `HZ` |
| `HIGHMEM` (off) | on | `!` the inline `lowmem_page_address()`/`page_address()`; harmless only as long as all RAM is low memory |
| `CC_STACKPROTECTOR_REGULAR` | none | `!` `__stack_chk_guard`/`__stack_chk_fail` not exported (modpost: undefined), `task_struct` |
| `SECURITY` (off) | on | `!` `file.private_data`, `inode.i_rdev`/`i_size` move |
| `SCHEDSTATS` (off) | on | `!` `task_struct` |
| `DEBUG_SG` (off) | on | `!` `scatterlist` (`page_link`, `length`) |
| `FB_TILEBLITTING` (off) | on | `!` `fb_info` (`screen_buffer`, `pseudo_palette`, `par`) |

Free to change (no `!`; only `?` in types the vendor code is not seen to
touch): `LBDAF` (as above), `NR_CPUS` (4 → 8), `DEBUG_ATOMIC_SLEEP`,
`SLAB` instead of `SLUB` (the four `kmalloc_caches[]` indexes keep their
size classes), `SLUB_DEBUG`, `MEMCG`, `CGROUPS` off, `CPUSETS`,
`CPU_SW_DOMAIN_PAN` off (the vendor's inline user copies then open a domain
that is already open), `THUMB2_KERNEL` off, `FB_BACKLIGHT`, `SECCOMP`,
`FTRACE`/`FUNCTION_TRACER`, `AUDIT`/`AUDITSYSCALL`, `FAIR_GROUP_SCHED`,
`CMA` off. `FB_DEFERRED_IO` and `ARM_PATCH_PHYS_VIRT` did not change
(the first is only selected by drivers, the second needs `EMBEDDED`).

`CONFIG_DEBUG_INFO` is free. Any other option: run the check.

## Newer kernels

No 4.x-to-5.x/6.x path exists for the closed parts of this SDK; what
follows measures how far the relinked blobs are from two newer kernels.

- **SigmaStar's Iford SDK kernel, 5.10.117** (`kernel-Iford_IMD00V2.1.0`,
  `iford_ssz029c_s01b_nor_demo_defconfig`, GCC plugins off, built with Arm
  GNU Toolchain 10.3): the same vendor's kernel line and `drivers/sstar`
  (CamOs) for the next generation, with nearly the same debug options as
  the i2m config (`PREEMPT`, `SMP`, `THUMB2_KERNEL`, `DEBUG_SPINLOCK`,
  `DEBUG_MUTEXES`, `HZ=100`, `CPU_SW_DOMAIN_PAN`), so what differs is
  mostly the version.
- **mainline 7.3** (`multi_v7_defconfig` with `ARCH_MSTARV7`, the upstream
  SSD20x/infinity2m support, `THUMB2_KERNEL`, `PREEMPT`, `DEBUG_SPINLOCK`,
  `DEBUG_MUTEXES`, `HZ=100`; multi_v7 keeps `HIGHMEM`): no SigmaStar
  drivers at all.

| | Iford 5.10.117 | mainline 7.3 |
|---|---|---|
| types identical | 273 of 473 | 215 of 473 |
| `!` types | 19 (15 [A], 4 [B]) | 39 (32 [A], 7 [B]) |
| `?` types | 175 | 211 |
| vendor instructions reading or writing a moved member | 138: `fb_info` 87, `file` 20, `platform_device` 12, `task_struct` 9, `seq_file` 6, `CamOsTimespec_t` 4 | 260: `fb_info` 87, `thread_info` 68 (`addr_limit` 62: `set_fs()` is gone), `file` 23, `vm_area_struct` 18, `task_struct` 12, `platform_device` 12, `seq_file` 6, `inode` 2, and 32 in CamOs types mainline does not have |
| moved members set by vendor static initialisers | 104 (in 35 `file_operations` tables, 11 `DEFINE_MUTEX`es, 3 `platform_device`s, ...) | 1206 (most of them the 496 `__ksymtab` entries) |
| kernel imports not exported | 11 | 33 |
| kernel imports with other arguments (`!`) | 8 | 4 (+ the 33) |
| SigmaStar imports not exported | 26 (of 135) | 135 (all) |
| baked constants changed | `GFP_KERNEL`, `GFP_ATOMIC`, `__GFP_ZERO`, `__GFP_NOWARN` | the same, `THREAD_INFO_IN_TASK`, `STACKPROTECTOR_PER_TASK`, `HIGHMEM` |
| `kmalloc_caches` | `[2][14]`: index `[0][i]` is the old `[i]`, the four used indexes hold the same sizes | `[3][14]`, likewise |

On 5.10, in detail:

- **Gone exports** (kernel): `do_gettimeofday`, `getnstimeofday64`,
  `vfs_read`, `vfs_write`, `idr_init`, `idr_find_slowpath` (the inline
  `idr_find()` of the 4.9 layer tree), `ioremap_cached`, `lock_fb_info`
  (an inline now), `warn_slowpath_null`, `__memzero`, `__mm_populate`.
  SigmaStar: `CamOsTcond*` (5), `CamOsPhyMemUnMap`, and the i2m drivers'
  `MDrv_MIU_*`, `MDrv_MMU_*`, `MDrv_GPIO_*`, `mdrv_padmux_*`.
- **Other arguments**: `init_timer_key` (takes the function now),
  `__alloc_pages_nodemask` (a node id instead of the inline-computed
  zonelist), `do_mmap` (no `vm_flags`, an `uf` list), `do_munmap` (`uf`),
  `vfs_getattr` (two more), `show_stack` (a log level: the vendor's `r2` is
  whatever it was), `unregister_framebuffer` (void); `proc_create_data`
  takes a `struct proc_ops` since 5.6 where the vendor passes a 4.9
  `file_operations`. `delayed_work_timer_fn` changed too, but the vendor
  code only stores its address in its timers, which 5.10 then calls right.
- **GFP bits**: every `__GFP_` bit above `__GFP_FS` moved. The vendor's
  `GFP_KERNEL` (`0x24000c0`) reads in 5.10 as IO|FS|ACCOUNT without
  reclaim, its `__GFP_ZERO` (`0x8000`) as `__GFP_NOFAIL`: `kzalloc()`
  returns memory that is not zeroed.
- **Tables the vendor builds and the kernel reads**: `file_operations`
  (from `iterate` on every member moved; 35 vendor tables), `kernel_symbol`
  (12 bytes since 5.4, the vendor's `__ksymtab` has 8-byte entries),
  `vm_operations_struct` (`fault` takes only the `vm_fault`), `platform_driver`
  and `device_driver` (mhal's 4 drivers), static `mutex` initialisers (11)
  (`count` gone, `owner` first), `timer_list`/`delayed_work` (`data` gone,
  `flags` at its place), `kstat` (80 → 144 bytes, on the vendor's stack).
- **Kernel objects the vendor code dereferences**: `struct file`
  (`f_flags`, `f_pos`, `private_data`: 20 instructions in mi_sys and
  mi_common), `seq_file.private` (6, the vendor's `/proc` show functions),
  `fb_info` (every `var.*` and `fix.*` member fbdev sets, `fbops`, `par`,
  `screen_buffer`, `pseudo_palette`: 87 instructions, all 4 bytes further),
  `task_struct` (`mm`, `pid`, `tgid`, `real_parent`, `pids`: 9),
  `platform_device` (`dev.driver_data`, `dev.of_node`: 12, mhal's and
  mi_ipu's probe functions). CamOs changed too: `CamOsTimespec_t` is two
  `s64` (the vendor passes 8 bytes of stack for 16),
  `CamOsThreadAttrb_t` gained 8 bytes; its mutex, semaphore, spinlock,
  timer and cache handles are pointers now, which fit the vendor's larger
  opaque storage.

### What a shim closes: `KCOMPAT=1`

As openhisilicon's `osal_v2_shim` keeps old OSAL names alive on a newer
OSAL, `kernel/kcompat/` gives the vendor objects back what a function can
give back. Written for and built against the Iford 5.10.117 kernel:

```sh
make -C kernel KDIR=<5.10 tree> CHIPARCH=infinity2m KCOMPAT=1 ARCH=arm CROSS_COMPILE=...
```

- `scripts/kcompat-obj.sh` renames, in each vendor object, the references
  to kernel functions whose prototype or meaning changed to `kcompat_<sym>`
  (`kernel/kcompat/renames-5.10.txt`: 21), removes the vendor's 8-byte
  `__ksymtab` and exports the same symbols again with this kernel's
  `EXPORT_SYMBOL` (generated `<mod>_exports.c`), and imports the symbol
  namespaces the vendor's imports are in (SigmaStar's 5.10 puts
  `filp_open`, `kernel_read` in `VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver`).
- `open_kcompat.ko` (`kernel/kcompat/open_kcompat.c`) provides:
  - under their 4.9 names, each only when the target's `Module.symvers`
    lacks it: `do_gettimeofday`, `getnstimeofday64`, `vfs_read`,
    `vfs_write` (`kernel_read`/`kernel_write` after the vendor's
    `set_fs(KERNEL_DS)`, an `iov_iter` read otherwise), `idr_init` and
    `idr_find_slowpath` (a 5.10 idr behind the vendor's 40-byte 4.9
    `struct idr`, whose `hint` stays NULL so the inline `idr_find()` always
    calls out), `ioremap_cached`, `__memzero`, `warn_slowpath_null`,
    `lock_fb_info`, `CamOsTcond*` (a completion in the vendor's 80 bytes, as
    the i2m CamOs has it), `CamOsPhyMemUnMap`;
  - as `kcompat_*`: GFP translation for `__kmalloc`, `kmem_cache_alloc`
    (the inline `kmalloc()`), `kmemdup`, `kstrdup`, `sg_alloc_table`,
    `idr_alloc` and `__alloc_pages_nodemask`; `file_operations` copied
    member by member into 5.10 tables for `cdev_init`, `__register_chrdev`
    and, as `proc_ops`, `proc_create_data`; `init_timer_key` with the old
    arguments, and `queue_delayed_work_on`/`cancel_delayed_work` that put
    back the `flags` the vendor's inline `timer->data` store overwrote;
    `vfs_getattr` into a 4.9 `kstat`; `show_stack`, `unregister_framebuffer`;
    `CamOsGetMonotonicTime`/`CamOsGetTimeOfDay` into 8 bytes,
    `CamOsThreadCreate` with a zeroed 20-byte attribute block.

Built against that kernel, all 14 modules build. modpost's undefined
symbols go from 37 (11 kernel, 26 SigmaStar), with 8 modules refused for
an unimported namespace and 21 calls that link but pass 4.9 arguments, to
21:
`__mm_populate` and the 20 i2m hardware-driver exports (`MDrv_MIU_*`,
`MDrv_MMU_*`, `MDrv_GPIO_*`, `mdrv_padmux_*`, `clientId_KernelProtectToName`,
`u8_MiuWhiteListNum`), which belong to an i2m port of `drivers/sstar`, not
to the kernel version. `KCOMPAT=1` refuses kernels other than 5.10; the
normal build is unchanged by it (byte-identical modules).

### What no shim closes

The vendor code itself reads and writes members that moved; no function is
called there to intercept. Rewriting those instructions is conceivable (the
analysis knows each one), but only where the new offset fits the
instruction's encoding (a 16-bit Thumb `ldr` reaches 124), and only for the
accesses the analysis finds: it misses pointers that pass through the
stack, so a patched module would keep unpatched accesses. Not attempted.
With the evidence above, on 5.10:

- `fbdev`: every `fb_info` member it uses is 4 bytes further on (87
  instructions): it cannot work.
- `mi_sys`, `mi_common`: `file->private_data` (16 instructions, the MI
  device handle of every open), `seq_file->private` in every `/proc` file,
  `current->mm` (`task_struct` +88) for the user mappings, which go
  through `do_mmap`/`do_munmap` (other arguments, not shimmed); mi_sys's
  `vm_operations_struct.fault` takes other arguments.
- `mhal`: its platform drivers are 4.9 `platform_driver` tables, and its
  probe functions read `pdev->dev.of_node` at 208 (now 476).
- 11 modules have a static `DEFINE_MUTEX`, initialised for 4.9: 5.10
  reads its `count` of 1 as `owner` with the waiters flag, and finds its
  wait list 4 bytes further on, where the 4.9 initialiser left
  `wait_list.prev` and a NULL.

Mainline 7.3 adds, among others, `set_fs()` gone (62 instructions write
`thread_info->addr_limit`), `THREAD_INFO_IN_TASK` (`current` is no longer
`thread_info->task`), the per-task stack canary (the vendor reads the global
`__stack_chk_guard`, no longer exported), and no CamOs at all.

**Verdict.** On kernels from this 4.9 tree the blobs are usable as long as
the options in the table above are kept; the checker shows when they are
not. Beyond 4.9, exports and prototypes can be shimmed (done for 5.10) but
the vendor code depends on structure layouts that changed in 4.11–5.10
(`task_struct`, `file`, `fb_info`, `seq_file`, `platform_device`,
`file_operations`, `mutex`), in code no shim reaches: the relinked SDK
modules do not move to 5.10 or later. A newer kernel for infinity2m needs
these modules from source, or SigmaStar objects built for that kernel.

There is no 5.10 kernel for infinity2m to run them on. As far as it goes
without i2m hardware: in qemu-bench's `ssc029` (Iford) machine, booting
the Iford SDK's own NOR image (SigmaStar's prebuilt 5.10.117 kernel) and
unloading its MI modules, the `KCOMPAT=1` builds of `open_kcompat.ko` and
`open_mi_common.ko` load, `open_mi_common`'s init runs, and both unload,
without a kernel message. `mi_common` creates nothing until another MI
module registers a device through it, and the others need `mhal`, which
drives i2m hardware; none of the vendor accesses listed above was reached
in that test.

## Regenerating the vendor file

Only to change what is extracted; needs the SDK's original `.ko` files:

```sh
# the SDK kernel tree, configured as the vendor modules were built
cp infinity2m-ssc010a.config <tree>/.config     # OpenIPC firmware's
<tree>/scripts/config --file <tree>/.config -d LBDAF
make -C <tree> ARCH=arm CROSS_COMPILE=... olddefconfig modules_prepare
scripts/kabi-check.py --cross arm-openipc-linux-gnueabihf- extract \
    --chiparch infinity2m --sdk-kdir <tree> \
    --about "i2m NVR SDK (2020-03), 4.9.84" \
    --out kernel/kabi/infinity2m.json <sdk>/modules/4.9.84/*.ko
```

`extract` warns about every type where the probe of that tree disagrees
with the vendor DWARF; with `LBDAF` off there are none.
