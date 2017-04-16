/*
 * Copyright (c) 2012, 2013, Oracle and/or its affiliates. All rights reserved.
 */

/*
 * Copyright (c) 2008-2010, 2013, Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *    Zou Nan hai <nanhai.zou@intel.com>
 *    Xiang Hai hao<haihao.xiang@intel.com>
 *
 */

#ifndef _INTEL_RINGBUFFER_H_
#define _INTEL_RINGBUFFER_H_

/*
 * Gen2 BSpec "1. Programming Environment" / 1.4.4.6 "Ring Buffer Use"
 * Gen3 BSpec "vol1c Memory Interface Functions" / 2.3.4.5 "Ring Buffer Use"
 * Gen4+ BSpec "vol1c Memory Interface and Command Stream" / 5.3.4.5 "Ring Buffer Use"
 *
 * "If the Ring Buffer Head Pointer and the Tail Pointer are on the same
 * cacheline, the Head Pointer must not be greater than the Tail
 * Pointer."
 */
#define I915_RING_FREE_SPACE 64

struct  intel_hw_status_page {
	void		*page_addr;
	unsigned int	gfx_addr;
	struct		drm_i915_gem_object *obj;
};

#define I915_READ_TAIL(engine) I915_READ(RING_TAIL((engine)->mmio_base))
#define I915_WRITE_TAIL(engine, val) I915_WRITE(RING_TAIL((engine)->mmio_base), val)

#define I915_READ_START(engine) I915_READ(RING_START((engine)->mmio_base))
#define I915_WRITE_START(engine, val) I915_WRITE(RING_START((engine)->mmio_base), val)

#define I915_READ_HEAD(engine)  I915_READ(RING_HEAD((engine)->mmio_base))
#define I915_WRITE_HEAD(engine, val) I915_WRITE(RING_HEAD((engine)->mmio_base), val)

#define I915_READ_CTL(engine) I915_READ(RING_CTL((engine)->mmio_base))
#define I915_WRITE_CTL(engine, val) I915_WRITE(RING_CTL((engine)->mmio_base), val)

#define I915_READ_IMR(engine) I915_READ(RING_IMR((engine)->mmio_base))
#define I915_WRITE_IMR(engine, val) I915_WRITE(RING_IMR((engine)->mmio_base), val)

#define I915_READ_NOPID(engine) I915_READ(RING_NOPID((engine)->mmio_base))
#define I915_READ_SYNC_0(engine) I915_READ(RING_SYNC_0((engine)->mmio_base))
#define I915_READ_SYNC_1(engine) I915_READ(RING_SYNC_1((engine)->mmio_base))

enum intel_engine_hangcheck_action { wait, active, kick, hung };

struct intel_engine_hangcheck {
	bool deadlock;
	u32 seqno;
	u32 acthd;
	int score;
	enum intel_engine_hangcheck_action action;
};

struct  intel_ring {
	const char	*name;
	enum intel_ring_id {
		RCS = 0x0,
		VCS,
		BCS,
		VECS,
	} id;
#define I915_NUM_RINGS 4
	u32		mmio_base;
	void		*vaddr;
	struct		drm_device *dev;
	struct		drm_i915_gem_object *obj;

	u32		head;
	u32		tail;
	int		space;
	int		size;
	int		effective_size;
	struct intel_hw_status_page status_page;

	/** We track the position of the requests in the ring buffer, and
	 * when each is retired we increment last_retired_head as the GPU
	 * must have finished processing the request and so we know we
	 * can advance the ringbuffer up to that position.
	 *
	 * last_retired_head is set to -1 after the value is consumed so
	 * we can detect new retirements.
	 */
	u32		last_retired_head;

	struct {
		u32	gt; /*  protected by dev_priv->irq_lock */
		u32	pm; /*  protected by dev_priv->rps.lock (sucks) */
	} irq_refcount;
	u32		irq_enable_mask;	/* bitmask to enable ring interrupt */
	u32		trace_irq_seqno;
	u32		sync_seqno[I915_NUM_RINGS-1];
	bool		(*irq_get)(struct intel_ring *ring);
	void		(*irq_put)(struct intel_ring *ring);

	int		(*init)(struct intel_ring *ring);

	void		(*write_tail)(struct intel_ring *ring,
				      u32 value);
	int		(*flush)(struct intel_ring *ring,
				  u32	invalidate_domains,
				  u32	flush_domains);
	int		(*add_request)(struct intel_ring *ring);
	/* Some chipsets are not quite as coherent as advertised and need
	 * an expensive kick to force a true read of the up-to-date seqno.
	 * However, the up-to-date seqno is not always required and the last
	 * seen value is good enough. Note that the seqno will always be
	 * monotonic, even if not coherent.
	 */
	u32		(*get_seqno)(struct intel_ring *ring,
				     bool lazy_coherency);
	void		(*set_seqno)(struct intel_ring *ring,
				     u32 seqno);
	int		(*dispatch_execbuffer)(struct intel_ring *ring,
					       u32 offset, u32 length,
					       unsigned flags);
#define I915_DISPATCH_SECURE 0x1
#define I915_DISPATCH_PINNED 0x2
	void		(*cleanup)(struct intel_ring *ring);
	int		(*sync_to)(struct intel_ring *ring,
				   struct intel_ring *to,
				   u32 seqno);

	/* our mbox written by others */
	u32		semaphore_register[I915_NUM_RINGS];
	/* mboxes this ring signals to */
	u32		signal_mbox[I915_NUM_RINGS];

	/**
	 * List of objects currently involved in rendering from the
	 * ringbuffer.
	 *
	 * Includes buffers having the contents of their GPU caches
	 * flushed, not necessarily primitives.  last_rendering_seqno
	 * represents when the rendering involved will be completed.
	 *
	 * A reference is held on the buffer while on this list.
	 */
	struct list_head active_list;

	/**
	 * List of breadcrumbs associated with GPU requests currently
	 * outstanding.
	 */
	struct list_head request_list;

	/**
	 * Do we have some not yet emitted requests outstanding?
	 */
	u32 outstanding_lazy_request;
	bool gpu_caches_dirty;
	bool fbc_dirty;

	wait_queue_head_t irq_queue;
	drm_local_map_t map;

	/**
	 * Do an explicit TLB flush before MI_SET_CONTEXT
	 */
	bool itlb_before_ctx_switch;
	struct i915_hw_context *default_context;
	struct i915_hw_context *last_context;

	struct intel_engine_hangcheck hangcheck;

	void *private;
};

static inline bool
intel_ring_initialized(struct intel_ring *ring)
{
	return ring->obj != NULL;
}

static inline unsigned
intel_ring_flag(struct intel_ring *ring)
{
	return 1 << ring->id;
}

static inline u32
intel_ring_sync_index(struct intel_ring *ring,
		      struct intel_ring *other)
{
	int idx;

	/*
	 * cs -> 0 = vcs, 1 = bcs
	 * vcs -> 0 = bcs, 1 = cs,
	 * bcs -> 0 = cs, 1 = vcs.
	 */

	idx = (other - ring) - 1;
	if (idx < 0)
		idx += I915_NUM_RINGS;

	return idx;
}

static inline u32
intel_read_status_page(struct intel_ring *ring,
		int reg)
{
	u32 *regs = ring->status_page.page_addr;
	return regs[reg];
}

static inline void
intel_write_status_page(struct intel_ring *ring,
			int reg, u32 value)
{
	u32 *regs = ring->status_page.page_addr;
	regs[reg] = value;
}

/**
 * Reads a dword out of the status page, which is written to from the command
 * queue by automatic updates, MI_REPORT_HEAD, MI_STORE_DATA_INDEX, or
 * MI_STORE_DATA_IMM.
 *
 * The following dwords have a reserved meaning:
 * 0x00: ISR copy, updated when an ISR bit not set in the HWSTAM changes.
 * 0x04: ring 0 head pointer
 * 0x05: ring 1 head pointer (915-class)
 * 0x06: ring 2 head pointer (915-class)
 * 0x10-0x1b: Context status DWords (GM45)
 * 0x1f: Last written status offset. (GM45)
 * 0x20-0x2f: Reserved (Gen6+)
 *
 * The area from dword 0x30 to 0x3ff is available for driver usage.
 */
#define I915_GEM_HWS_INDEX		0x30
#define I915_GEM_HWS_INDEX_ADDR (I915_GEM_HWS_INDEX << MI_STORE_DWORD_INDEX_SHIFT)
#define I915_GEM_HWS_SCRATCH_INDEX	0x40
#define I915_GEM_HWS_SCRATCH_ADDR (I915_GEM_HWS_SCRATCH_INDEX << MI_STORE_DWORD_INDEX_SHIFT)

void intel_cleanup_ring_buffer(struct intel_ring *ring);

int intel_wait_ring_buffer(struct intel_ring *ring, int n);
int intel_wait_ring_idle(struct intel_ring *ring);

int intel_ring_begin(struct intel_ring *ring, int n);

static inline void intel_ring_emit(struct intel_ring *ring,
				   u32 data)
{
	unsigned int *virt = (unsigned int *)((intptr_t)ring->vaddr + ring->tail);
	*virt = data;
	ring->tail += 4;
}

void intel_ring_advance(struct intel_ring *ring);
int intel_ring_idle(struct intel_ring *ring);
void intel_ring_init_seqno(struct intel_ring *ring, u32 seqno);
int intel_ring_flush_all_caches(struct intel_ring *ring);
int intel_ring_invalidate_all_caches(struct intel_ring *ring);

int intel_init_render_ring_buffer(struct drm_device *dev);
int intel_init_bsd_ring_buffer(struct drm_device *dev);
int intel_init_blt_ring_buffer(struct drm_device *dev);
int intel_init_vebox_ring_buffer(struct drm_device *dev);

u32 intel_ring_get_active_head(struct intel_ring *ring);
void intel_ring_setup_status_page(struct intel_ring *ring);

static inline u32 intel_ring_get_tail(struct intel_ring *ring)
{
	return ring->tail;
}

static inline u32 intel_ring_get_seqno(struct intel_ring *ring)
{
	BUG_ON(ring->outstanding_lazy_request == 0);
	return ring->outstanding_lazy_request;
}

static inline void i915_trace_irq_get(struct intel_ring *ring, u32 seqno)
{
	if (ring->trace_irq_seqno == 0 && ring->irq_get(ring))
		ring->trace_irq_seqno = seqno;
}

/* DRI warts */
int intel_render_ring_init_dri(struct drm_device *dev, u64 start, u32 size);

#endif /* _INTEL_RINGBUFFER_H_ */
