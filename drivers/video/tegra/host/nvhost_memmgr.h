/*
 * drivers/video/tegra/host/nvhost_memmgr.h
 *
 * Tegra Graphics Host Memory Management Abstraction header
 *
 * Copyright (c) 2012-2014, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _NVHOST_MEM_MGR_H_
#define _NVHOST_MEM_MGR_H_

#include <linux/dma-mapping.h>
#include <linux/scatterlist.h>

struct nvhost_chip_support;
struct mem_mgr;
struct mem_handle;
struct platform_device;
struct device;
struct nvhost_allocator;

enum mem_mgr_flag {
	mem_mgr_flag_uncacheable = 0,
	mem_mgr_flag_write_combine = 1,
};

enum mem_rw_flag {
	mem_flag_none = 0,
	mem_flag_read_only = 1,
	mem_flag_write_only = 2,
};

enum mem_mgr_type {
	mem_mgr_type_nvmap = 0,
	mem_mgr_type_dmabuf = 1,
};

#define MEMMGR_TYPE_MASK	0x0
#define MEMMGR_ID_MASK		(~MEMMGR_TYPE_MASK)

int nvhost_memmgr_init(struct nvhost_chip_support *chip);
struct mem_mgr *nvhost_memmgr_alloc_mgr(void);
void nvhost_memmgr_put_mgr(struct mem_mgr *);
struct mem_mgr *nvhost_memmgr_get_mgr(struct mem_mgr *);
struct mem_mgr *nvhost_memmgr_get_mgr_file(int fd);
struct mem_handle *nvhost_memmgr_get(struct mem_mgr *,
		ulong id, struct platform_device *dev);
void nvhost_memmgr_put(struct mem_mgr *mgr, struct mem_handle *handle);
struct sg_table *nvhost_memmgr_pin(struct mem_mgr *,
		struct mem_handle *handle,
		struct device *dev,
		int rw_flag);
void nvhost_memmgr_unpin(struct mem_mgr *mgr,
		struct mem_handle *handle, struct device *dev,
		struct sg_table *sgt);
void *nvhost_memmgr_mmap(struct mem_handle *handle);
void nvhost_memmgr_munmap(struct mem_handle *handle, void *addr);
void *nvhost_memmgr_kmap(struct mem_handle *handle, unsigned int pagenum);
void nvhost_memmgr_kunmap(struct mem_handle *handle, unsigned int pagenum,
		void *addr);
static inline int nvhost_memmgr_type(ulong id) { return id & MEMMGR_TYPE_MASK; }
static inline int nvhost_memmgr_id(ulong id) { return id & MEMMGR_ID_MASK; }

#ifdef CONFIG_TEGRA_IOMMU_SMMU
int nvhost_memmgr_smmu_map(struct sg_table *sgt, size_t size,
			   struct device *dev);
void nvhost_memmgr_smmu_unmap(struct sg_table *sgt, size_t size,
			   struct device *dev);
#endif

/*
 * Return IOVA for buffers allocated from IOVMM heap and have IOVA assigned.
 * Otherwise assume the buffer was allocated from carveout and return physical
 * address.
 */
static inline dma_addr_t nvhost_memmgr_dma_addr(struct sg_table *sgt)
{
	return sg_dma_address(sgt->sgl) && sg_dma_address(sgt->sgl) != DMA_ERROR_CODE ?
		sg_dma_address(sgt->sgl) :
		sg_phys(sgt->sgl);
}

#endif
