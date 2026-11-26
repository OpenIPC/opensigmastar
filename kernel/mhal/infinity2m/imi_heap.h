/*
 * The IMI heap allocator inside the vendor mhal (imi_heap.c, not in the
 * boot release), as its DWARF declares it. SigmaStar's disp OS layer calls
 * it for the rotation buffers.
 */
#ifndef OPEN_IMI_HEAP_H
#define OPEN_IMI_HEAP_H

typedef enum {
	IMI_HEAP_OK = 0,
	IMI_HEAP_NO_FIT_SZIE = -1,
	IMI_HEAP_REACH_MAX_ALLOCATE_CNT = -2,
	IMI_HEAP_CREATE_NODE_FAIL = -3,
	IMI_HEAP_FREE_INVALID_ADDR = -4,
} ImiHeapRet_e;

ImiHeapRet_e ImiHeapMalloc(u32 u32Bank, u32 u32Size, void **ppAddr);
ImiHeapRet_e ImiHeapFree(u32 u32Bank, void *pAddr);
u32 ImiHeapQueryMaxFree(u32 u32Bank);

#endif
