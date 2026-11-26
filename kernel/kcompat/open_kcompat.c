// SPDX-License-Identifier: GPL-2.0
/*
 * open_kcompat: what a kernel newer than 4.9 no longer gives the infinity2m
 * vendor modules (i2m NVR SDK, built against 4.9.84), where a function can
 * give it back. Built only with KCOMPAT=1 (kernel/kcompat/kcompat.kbuild);
 * written for and built against 5.10 (SigmaStar's Iford kernel, 5.10.117).
 *
 * Two kinds of entry point:
 *  - symbols the newer kernel no longer exports at all (do_gettimeofday,
 *    vfs_read, idr_find_slowpath, ...): exported here under their own name,
 *    each only when the target's Module.symvers lacks it (KC_NEED_<sym>);
 *  - symbols the newer kernel still exports with another prototype or
 *    meaning (init_timer_key, vfs_getattr, every allocator taking a gfp_t,
 *    proc_create_data, cdev_init, ...): the vendor objects' references are
 *    renamed to kcompat_<sym> (kernel/kcompat/renames-5.10.txt) and land
 *    here, which calls the kernel's own with the arguments it now expects.
 *
 * What no function can give back -- the vendor code reading and writing
 * members of kernel structures that moved (struct file, seq_file, fb_info,
 * task_struct, platform_device, ...) -- is listed by scripts/kabi-check.py;
 * see docs/kabi.md. This module does not make the vendor modules work on
 * 5.10, it closes the gaps that can be closed this way.
 *
 * 4.9 layouts below are those recorded in kernel/kabi/infinity2m.json.
 */
/* 5.10 has these as static inlines; the exported 4.9 ones below get the
 * names back after the headers */
#define idr_init kc_header_idr_init
#define lock_fb_info kc_header_lock_fb_info
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/gfp.h>
#include <linux/fs.h>
#include <linux/uio.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/proc_fs.h>
#include <linux/idr.h>
#include <linux/io.h>
#include <linux/fb.h>
#include <linux/sched/debug.h>
#include <linux/stat.h>
#include <linux/namei.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/timekeeping.h>
#include <linux/completion.h>
#include <linux/scatterlist.h>
#include <linux/topology.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/vmalloc.h>
#undef idr_init
#undef lock_fb_info

#define KC_EXPORT(sym) EXPORT_SYMBOL(sym)

/* ------------------------------------------------------------------ */
/* gfp_t: every __GFP_ bit above ___GFP_FS moved between 4.9 and 5.10.
 * GFP_KERNEL as the vendor code passes it (0x24000c0) is, read as 5.10
 * bits, IO|FS|ACCOUNT and no reclaim; __GFP_ZERO (0x8000) is NOFAIL. */

static const struct { u32 old; gfp_t now; } gfp_bits[] = {
	{ 0x01, __GFP_DMA }, { 0x02, __GFP_HIGHMEM }, { 0x04, __GFP_DMA32 },
	{ 0x08, __GFP_MOVABLE }, { 0x10, __GFP_RECLAIMABLE }, { 0x20, __GFP_HIGH },
	{ 0x40, __GFP_IO }, { 0x80, __GFP_FS }, /* 0x100 __GFP_COLD: gone */
	{ 0x200, __GFP_NOWARN }, { 0x400, __GFP_RETRY_MAYFAIL }, /* was REPEAT */
	{ 0x800, __GFP_NOFAIL }, { 0x1000, __GFP_NORETRY },
	{ 0x2000, __GFP_MEMALLOC }, { 0x4000, __GFP_COMP }, { 0x8000, __GFP_ZERO },
	{ 0x10000, __GFP_NOMEMALLOC }, { 0x20000, __GFP_HARDWALL },
	{ 0x40000, __GFP_THISNODE }, { 0x80000, __GFP_ATOMIC },
	{ 0x100000, __GFP_ACCOUNT }, /* 0x200000 __GFP_NOTRACK: gone */
	{ 0x400000, __GFP_DIRECT_RECLAIM }, /* 0x800000 __GFP_OTHER_NODE: gone */
	{ 0x1000000, __GFP_WRITE }, { 0x2000000, __GFP_KSWAPD_RECLAIM },
};

static gfp_t gfp49(u32 old)
{
	gfp_t now = 0;
	int i;

	for (i = 0; i < ARRAY_SIZE(gfp_bits); i++)
		if (old & gfp_bits[i].old)
			now |= gfp_bits[i].now;
	return now;
}

void *kcompat___kmalloc(size_t size, u32 flags)
{
	return __kmalloc(size, gfp49(flags));
}
EXPORT_SYMBOL(kcompat___kmalloc);

/* the inline kmalloc() of a constant size: kmalloc_caches[KMALLOC_NORMAL]
 * is where the 4.9 one-dimensional array was, so the cache is right */
void *kcompat_kmem_cache_alloc(struct kmem_cache *s, u32 flags)
{
	return kmem_cache_alloc(s, gfp49(flags));
}
EXPORT_SYMBOL(kcompat_kmem_cache_alloc);

void *kcompat_kmemdup(const void *src, size_t len, u32 flags)
{
	return kmemdup(src, len, gfp49(flags));
}
EXPORT_SYMBOL(kcompat_kmemdup);

char *kcompat_kstrdup(const char *s, u32 flags)
{
	return kstrdup(s, gfp49(flags));
}
EXPORT_SYMBOL(kcompat_kstrdup);

int kcompat_sg_alloc_table(struct sg_table *t, unsigned int nents, u32 flags)
{
	return sg_alloc_table(t, nents, gfp49(flags));
}
EXPORT_SYMBOL(kcompat_sg_alloc_table);

/* 4.9: (gfp, order, struct zonelist *, nodemask); the inline
 * __alloc_pages_node() picked the zonelist out of contig_page_data */
struct page *kcompat___alloc_pages_nodemask(u32 flags, unsigned int order,
					    void *zonelist, nodemask_t *nodemask)
{
	return __alloc_pages_nodemask(gfp49(flags), order, numa_node_id(), nodemask);
}
EXPORT_SYMBOL(kcompat___alloc_pages_nodemask);

/* ------------------------------------------------------------------ */
/* time */

struct kc_timeval49 { s32 tv_sec; s32 tv_usec; };
struct kc_timespec49 { s32 tv_sec; s32 tv_nsec; };

#ifdef KC_NEED_do_gettimeofday
void do_gettimeofday(struct kc_timeval49 *tv)
{
	struct timespec64 ts;

	ktime_get_real_ts64(&ts);
	tv->tv_sec = ts.tv_sec;
	tv->tv_usec = ts.tv_nsec / NSEC_PER_USEC;
}
KC_EXPORT(do_gettimeofday);
#endif

#ifdef KC_NEED_getnstimeofday64
void getnstimeofday64(struct timespec64 *ts)
{
	ktime_get_real_ts64(ts);
}
KC_EXPORT(getnstimeofday64);
#endif

/* SigmaStar's CamOsTimespec_t was two u32 in the i2m kernel, two s64 in
 * Iford's: the vendor code hands 8 bytes of stack */
struct kc_camos_ts49 { u32 nSec; u32 nNanoSec; };

void kcompat_CamOsGetMonotonicTime(struct kc_camos_ts49 *t)
{
	struct timespec64 ts;

	ktime_get_raw_ts64(&ts);	/* getrawmonotonic() in the i2m CamOs */
	t->nSec = ts.tv_sec;
	t->nNanoSec = ts.tv_nsec;
}
EXPORT_SYMBOL(kcompat_CamOsGetMonotonicTime);

void kcompat_CamOsGetTimeOfDay(struct kc_camos_ts49 *t)
{
	struct timespec64 ts;

	ktime_get_real_ts64(&ts);
	t->nSec = ts.tv_sec;
	t->nNanoSec = ts.tv_nsec / NSEC_PER_USEC * NSEC_PER_USEC;
}
EXPORT_SYMBOL(kcompat_CamOsGetTimeOfDay);

/* CamOsTcond (gone from Iford's CamOs): the i2m implementation is a
 * completion behind an init magic, in the vendor's 80-byte CamOsTcond_t */
#define KC_TCOND_MAGIC 0x4b43544e
struct kc_tcond { u32 magic; struct completion c; };
struct kc_tcond_storage { u32 nPriv[20]; };

#ifdef KC_NEED_CamOsTcondInit
int CamOsTcondInit(struct kc_tcond_storage *p)
{
	struct kc_tcond *t = (struct kc_tcond *)p;

	BUILD_BUG_ON(sizeof(struct kc_tcond) > sizeof(struct kc_tcond_storage));
	if (!t)
		return 2;		/* CAM_OS_PARAM_ERR */
	init_completion(&t->c);
	t->magic = KC_TCOND_MAGIC;
	return 0;			/* CAM_OS_OK */
}
KC_EXPORT(CamOsTcondInit);

int CamOsTcondDeinit(struct kc_tcond_storage *p)
{
	struct kc_tcond *t = (struct kc_tcond *)p;

	if (!t)
		return 2;
	if (t->magic != KC_TCOND_MAGIC)
		return 1;		/* CAM_OS_FAIL */
	t->magic = 0;
	return 0;
}
KC_EXPORT(CamOsTcondDeinit);

void CamOsTcondSignal(struct kc_tcond_storage *p)
{
	struct kc_tcond *t = (struct kc_tcond *)p;

	if (t && t->magic == KC_TCOND_MAGIC)
		complete(&t->c);
}
KC_EXPORT(CamOsTcondSignal);

void CamOsTcondSignalAll(struct kc_tcond_storage *p)
{
	struct kc_tcond *t = (struct kc_tcond *)p;

	if (t && t->magic == KC_TCOND_MAGIC)
		complete_all(&t->c);
}
KC_EXPORT(CamOsTcondSignalAll);

int CamOsTcondTimedWait(struct kc_tcond_storage *p, u32 msec)
{
	struct kc_tcond *t = (struct kc_tcond *)p;

	if (!t)
		return 2;
	if (t->magic != KC_TCOND_MAGIC)
		return 1;
	reinit_completion(&t->c);
	if (!wait_for_completion_timeout(&t->c, msecs_to_jiffies(msec)))
		return 3;		/* CAM_OS_TIMEOUT */
	return 0;
}
KC_EXPORT(CamOsTcondTimedWait);
#endif

/* ------------------------------------------------------------------ */
/* timers: 4.9 setup_timer()/INIT_DELAYED_WORK() call init_timer_key(timer,
 * flags, name, key) and then store ->function at +12 and ->data at +16
 * inline. In 5.10 +16 is ->flags, so the work pointer lands there; it is
 * put right before the kernel looks at the timer. */

void kcompat_init_timer_key(struct timer_list *t, unsigned int flags,
			    const char *name, struct lock_class_key *key)
{
	init_timer_key(t, NULL, flags, name, key);
}
EXPORT_SYMBOL(kcompat_init_timer_key);

static void kc_fix_dwork(struct delayed_work *dw)
{
	/* never queued: ->flags still holds what the vendor wrote as ->data */
	if (dw->timer.flags == (u32)(unsigned long)dw)
		dw->timer.flags = TIMER_IRQSAFE | raw_smp_processor_id();
}

bool kcompat_queue_delayed_work_on(int cpu, struct workqueue_struct *wq,
				   struct delayed_work *dw, unsigned long delay)
{
	kc_fix_dwork(dw);
	return queue_delayed_work_on(cpu, wq, dw, delay);
}
EXPORT_SYMBOL(kcompat_queue_delayed_work_on);

bool kcompat_cancel_delayed_work(struct delayed_work *dw)
{
	kc_fix_dwork(dw);
	return cancel_delayed_work(dw);
}
EXPORT_SYMBOL(kcompat_cancel_delayed_work);

/* ------------------------------------------------------------------ */
/* idr: 4.9's is a layer tree whose lookup is inline (idr_find() reads
 * ->hint, then calls idr_find_slowpath()); 5.10's is an xarray. The
 * vendor's struct idr (40 bytes) keeps hint NULL, so every lookup comes
 * here, and a 5.10 idr allocated behind ->top. */

struct kc_idr49 {
	void *hint;
	struct idr *top;
	int layers, cur;
	u32 lock[4];
	int id_free_cnt;
	void *id_free;
};

#ifdef KC_NEED_idr_init
void idr_init(struct kc_idr49 *idp)
{
	memset(idp, 0, sizeof(*idp));
	idp->top = kmalloc(sizeof(*idp->top), GFP_KERNEL);
	if (idp->top)
		idr_init_base(idp->top, 0);
}
KC_EXPORT(idr_init);
#endif

#ifdef KC_NEED_idr_find_slowpath
void *idr_find_slowpath(struct kc_idr49 *idp, int id)
{
	return idp->top ? idr_find(idp->top, id) : NULL;
}
KC_EXPORT(idr_find_slowpath);
#endif

int kcompat_idr_alloc(struct kc_idr49 *idp, void *ptr, int start, int end, u32 flags)
{
	if (!idp->top)
		return -ENOMEM;
	return idr_alloc(idp->top, ptr, start, end, gfp49(flags));
}
EXPORT_SYMBOL(kcompat_idr_alloc);

void *kcompat_idr_get_next(struct kc_idr49 *idp, int *nextid)
{
	return idp->top ? idr_get_next(idp->top, nextid) : NULL;
}
EXPORT_SYMBOL(kcompat_idr_get_next);

void kcompat_idr_destroy(struct kc_idr49 *idp)
{
	if (idp->top) {
		idr_destroy(idp->top);
		kfree(idp->top);
		idp->top = NULL;
	}
}
EXPORT_SYMBOL(kcompat_idr_destroy);

/* ------------------------------------------------------------------ */
/* file_operations: 5.10 added iopoll after write_iter, mmap_supported_flags
 * after mmap and dropped clone/dedupe_file_range, so every member from
 * iterate on moved. The vendor's tables are copied member by member into
 * 5.10 ones where the kernel takes them (cdev_init, __register_chrdev,
 * proc_create_data, which takes a struct proc_ops since 5.6). */

struct kc_fops49 {
	struct module *owner;
	loff_t (*llseek)(struct file *, loff_t, int);
	ssize_t (*read)(struct file *, char __user *, size_t, loff_t *);
	ssize_t (*write)(struct file *, const char __user *, size_t, loff_t *);
	ssize_t (*read_iter)(struct kiocb *, struct iov_iter *);
	ssize_t (*write_iter)(struct kiocb *, struct iov_iter *);
	int (*iterate)(struct file *, struct dir_context *);
	int (*iterate_shared)(struct file *, struct dir_context *);
	unsigned int (*poll)(struct file *, struct poll_table_struct *);
	long (*unlocked_ioctl)(struct file *, unsigned int, unsigned long);
	long (*compat_ioctl)(struct file *, unsigned int, unsigned long);
	int (*mmap)(struct file *, struct vm_area_struct *);
	int (*open)(struct inode *, struct file *);
	int (*flush)(struct file *, fl_owner_t id);
	int (*release)(struct inode *, struct file *);
	int (*fsync)(struct file *, loff_t, loff_t, int datasync);
	int (*fasync)(int, struct file *, int);
	int (*lock)(struct file *, int, struct file_lock *);
	void *sendpage, *get_unmapped_area, *check_flags, *flock, *splice_write,
	     *splice_read, *setlease, *fallocate, *show_fdinfo, *copy_file_range,
	     *clone_file_range, *dedupe_file_range;
};

struct kc_table {
	struct list_head list;
	const void *from;
	union {
		struct file_operations fops;
		struct proc_ops pops;
	};
	bool proc;
};

static LIST_HEAD(kc_tables);
static DEFINE_MUTEX(kc_tables_lock);

static void *kc_translate(const struct kc_fops49 *f, bool proc)
{
	struct kc_table *t;

	if (!f)
		return NULL;
	mutex_lock(&kc_tables_lock);
	list_for_each_entry(t, &kc_tables, list)
		if (t->from == f && t->proc == proc)
			goto out;
	t = kzalloc(sizeof(*t), GFP_KERNEL);
	if (!t)
		goto out;
	t->from = f;
	t->proc = proc;
	if (proc) {
		t->pops.proc_open = f->open;
		t->pops.proc_read = f->read;
		t->pops.proc_read_iter = f->read_iter;
		t->pops.proc_write = f->write;
		t->pops.proc_lseek = f->llseek;
		t->pops.proc_release = f->release;
		t->pops.proc_poll = (__poll_t (*)(struct file *, struct poll_table_struct *))f->poll;
		t->pops.proc_ioctl = f->unlocked_ioctl;
#ifdef CONFIG_COMPAT
		t->pops.proc_compat_ioctl = f->compat_ioctl;
#endif
		t->pops.proc_mmap = f->mmap;
	} else {
		t->fops.owner = f->owner;
		t->fops.llseek = f->llseek;
		t->fops.read = f->read;
		t->fops.write = f->write;
		t->fops.read_iter = f->read_iter;
		t->fops.write_iter = f->write_iter;
		t->fops.iterate = f->iterate;
		t->fops.iterate_shared = f->iterate_shared;
		t->fops.poll = (__poll_t (*)(struct file *, struct poll_table_struct *))f->poll;
		t->fops.unlocked_ioctl = f->unlocked_ioctl;
		t->fops.compat_ioctl = f->compat_ioctl;
		t->fops.mmap = f->mmap;
		t->fops.open = f->open;
		t->fops.flush = f->flush;
		t->fops.release = f->release;
		t->fops.fsync = f->fsync;
		t->fops.fasync = f->fasync;
		t->fops.lock = f->lock;
		if (f->sendpage || f->get_unmapped_area || f->splice_read || f->splice_write ||
		    f->setlease || f->fallocate || f->copy_file_range || f->clone_file_range)
			pr_warn("open_kcompat: %ps: file operations not carried over\n", f);
	}
	list_add(&t->list, &kc_tables);
out:
	mutex_unlock(&kc_tables_lock);
	if (!t)
		return NULL;
	return proc ? (void *)&t->pops : (void *)&t->fops;
}

void kcompat_cdev_init(struct cdev *cdev, const struct kc_fops49 *f)
{
	cdev_init(cdev, kc_translate(f, false));
}
EXPORT_SYMBOL(kcompat_cdev_init);

int kcompat___register_chrdev(unsigned int major, unsigned int baseminor,
			      unsigned int count, const char *name,
			      const struct kc_fops49 *f)
{
	return __register_chrdev(major, baseminor, count, name, kc_translate(f, false));
}
EXPORT_SYMBOL(kcompat___register_chrdev);

struct proc_dir_entry *kcompat_proc_create_data(const char *name, umode_t mode,
						struct proc_dir_entry *parent,
						const struct kc_fops49 *f, void *data)
{
	return proc_create_data(name, mode, parent, kc_translate(f, true), data);
}
EXPORT_SYMBOL(kcompat_proc_create_data);

/* ------------------------------------------------------------------ */
/* files */

/* SigmaStar's 5.10 puts kernel_read/kernel_write (and filp_open) in a
 * symbol namespace (KC_VFS_NS, from Module.symvers) */
#ifdef KC_VFS_NS
MODULE_IMPORT_NS(KC_VFS_NS);
#endif

/* the vendor code reads files after set_fs(KERNEL_DS) (inline, addr_limit
 * is where it was), with a kernel buffer; vfs_read/vfs_write are no longer
 * exported, kernel_read/kernel_write do that */
#ifdef KC_NEED_vfs_read
ssize_t vfs_read(struct file *f, char __user *buf, size_t count, loff_t *pos)
{
	struct iovec iov = { .iov_base = buf, .iov_len = count };
	struct kiocb kiocb;
	struct iov_iter iter;
	ssize_t ret;

	if (uaccess_kernel())
		return kernel_read(f, (void __force *)buf, count, pos);
	if (f->f_op->read)
		return f->f_op->read(f, buf, count, pos);
	if (!f->f_op->read_iter)
		return -EINVAL;
	init_sync_kiocb(&kiocb, f);
	kiocb.ki_pos = pos ? *pos : 0;
	iov_iter_init(&iter, READ, &iov, 1, count);
	ret = call_read_iter(f, &kiocb, &iter);
	if (pos)
		*pos = kiocb.ki_pos;
	return ret;
}
KC_EXPORT(vfs_read);
#endif

#ifdef KC_NEED_vfs_write
ssize_t vfs_write(struct file *f, const char __user *buf, size_t count, loff_t *pos)
{
	struct iovec iov = { .iov_base = (void __user *)buf, .iov_len = count };
	struct kiocb kiocb;
	struct iov_iter iter;
	ssize_t ret;

	if (uaccess_kernel())
		return kernel_write(f, (const void __force *)buf, count, pos);
	if (f->f_op->write)
		return f->f_op->write(f, buf, count, pos);
	if (!f->f_op->write_iter)
		return -EINVAL;
	init_sync_kiocb(&kiocb, f);
	kiocb.ki_pos = pos ? *pos : 0;
	iov_iter_init(&iter, WRITE, &iov, 1, count);
	ret = call_write_iter(f, &kiocb, &iter);
	if (pos)
		*pos = kiocb.ki_pos;
	return ret;
}
KC_EXPORT(vfs_write);
#endif

/* 4.9 struct kstat, 80 bytes (5.10's is 144: on the vendor's stack) */
struct kc_kstat49 {
	u64 ino;
	u32 dev;
	u16 mode;
	u32 nlink;
	u32 uid, gid;
	u32 rdev;
	s64 size;
	struct kc_timespec49 atime, mtime, ctime;
	unsigned long blksize;
	unsigned long long blocks;
};

int kcompat_vfs_getattr(const struct path *path, struct kc_kstat49 *st)
{
	struct kstat k;
	int ret;

	BUILD_BUG_ON(sizeof(struct kc_kstat49) != 80);
	ret = vfs_getattr(path, &k, STATX_BASIC_STATS, AT_STATX_SYNC_AS_STAT);
	if (ret)
		return ret;
	st->ino = k.ino;
	st->dev = k.dev;
	st->mode = k.mode;
	st->nlink = k.nlink;
	st->uid = from_kuid(&init_user_ns, k.uid);
	st->gid = from_kgid(&init_user_ns, k.gid);
	st->rdev = k.rdev;
	st->size = k.size;
	st->atime.tv_sec = k.atime.tv_sec;
	st->atime.tv_nsec = k.atime.tv_nsec;
	st->mtime.tv_sec = k.mtime.tv_sec;
	st->mtime.tv_nsec = k.mtime.tv_nsec;
	st->ctime.tv_sec = k.ctime.tv_sec;
	st->ctime.tv_nsec = k.ctime.tv_nsec;
	st->blksize = k.blksize;
	st->blocks = k.blocks;
	return 0;
}
EXPORT_SYMBOL(kcompat_vfs_getattr);

/* ------------------------------------------------------------------ */
/* the rest */

#ifdef KC_NEED_CamOsPhyMemUnMap
void CamOsPhyMemUnMap(void *virt)
{
	vunmap(virt);		/* as the i2m CamOs does */
}
KC_EXPORT(CamOsPhyMemUnMap);
#endif

#ifdef KC_NEED_ioremap_cached
void __iomem *ioremap_cached(phys_addr_t phys, size_t size)
{
	return ioremap_cache(phys, size);
}
KC_EXPORT(ioremap_cached);
#endif

#ifdef KC_NEED___memzero
void __memzero(void *p, size_t n)
{
	memset(p, 0, n);
}
KC_EXPORT(__memzero);
#endif

#ifdef KC_NEED_warn_slowpath_null
void warn_slowpath_null(const char *file, int line)
{
	pr_warn("WARNING: at %s:%d\n", file, line);
	dump_stack();
	add_taint(TAINT_WARN, LOCKDEP_STILL_OK);
}
KC_EXPORT(warn_slowpath_null);
#endif

/* 4.9 returned 1 (5.10's is an inline returning nothing) */
#ifdef KC_NEED_lock_fb_info
int lock_fb_info(struct fb_info *info)
{
	mutex_lock(&info->lock);
	return 1;
}
KC_EXPORT(lock_fb_info);
#endif

int kcompat_unregister_framebuffer(struct fb_info *info)
{
	unregister_framebuffer(info);
	return 0;
}
EXPORT_SYMBOL(kcompat_unregister_framebuffer);

/* 5.10 takes a log level third; the vendor's r2 is whatever it was */
void kcompat_show_stack(struct task_struct *task, unsigned long *sp)
{
	show_stack(task, sp, KERN_DEFAULT);
}
EXPORT_SYMBOL(kcompat_show_stack);

/* Iford's CamOsThreadAttrb_t gained a 8-byte Sched member: the vendor's
 * 12-byte attribute block is copied into a zeroed one */
struct kc_thread_attr49 { u32 nPriority, nStackSize; char *szName; };
struct kc_thread_attr { u32 nPriority, nStackSize; char *szName; u32 sched[2]; };
extern int CamOsThreadCreate(void *thread, struct kc_thread_attr *attr,
			     void *(*fn)(void *), void *arg);

int kcompat_CamOsThreadCreate(void *thread, struct kc_thread_attr49 *attr,
			      void *(*fn)(void *), void *arg)
{
	struct kc_thread_attr a = { 0 };

	if (!attr)
		return CamOsThreadCreate(thread, NULL, fn, arg);
	a.nPriority = attr->nPriority;
	a.nStackSize = attr->nStackSize;
	a.szName = attr->szName;
	return CamOsThreadCreate(thread, &a, fn, arg);
}
EXPORT_SYMBOL(kcompat_CamOsThreadCreate);

static void __exit kcompat_exit(void)
{
	struct kc_table *t, *n;

	list_for_each_entry_safe(t, n, &kc_tables, list)
		kfree(t);
}
module_exit(kcompat_exit);

MODULE_DESCRIPTION("4.9 kernel interfaces for the relinked infinity2m vendor modules");
MODULE_LICENSE("GPL");
