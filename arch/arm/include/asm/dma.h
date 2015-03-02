/*
 * Copyright (C) 2012 by Marc Kleine-Budde <mkl@pengutronix.de>
 *
 * This file is released under the GPLv2
 *
 */

#include <common.h>

#define dma_alloc dma_alloc
static inline void *dma_alloc(size_t size)
{
	return xmemalign(64, ALIGN(size, 64));
}

#ifndef CONFIG_MMU
static inline void *dma_alloc_coherent(size_t size)
{
	return xmemalign(4096, size);
}

static inline void dma_free_coherent(void *mem, size_t size)
{
	free(mem);
}

static inline void dma_sync_single_for_cpu(unsigned long address, size_t size,
					   enum dma_data_direction dir)
{
}

static inline void dma_sync_single_for_device(unsigned long address, size_t size,
					      enum dma_data_direction dir)
{
}
#endif
