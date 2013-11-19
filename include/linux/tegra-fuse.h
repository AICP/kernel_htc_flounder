/*
 * Copyright (C) 2010 Google, Inc.
 * Copyright (c) 2010-2013, NVIDIA CORPORATION. All rights reserved.
 *
 * Author:
 *	Colin Cross <ccross@android.com>
 *	Sumit Sharma <sumsharma@nvidia.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/tegra-soc.h>
#include <linux/sysfs.h>
#include <linux/platform_device.h>

#ifndef __TEGRA_FUSE_H
#define __TEGRA_FUSE_H

#define FUSE_SKU_USB_CALIB_0	0x1f0

unsigned long long tegra_chip_uid(void);
void tegra_init_fuse(void);
bool tegra_spare_fuse(int bit);

u32 tegra_fuse_readl(unsigned long offset);
void tegra_fuse_writel(u32 val, unsigned long offset);

#ifdef CONFIG_ARCH_TEGRA_2x_SOC
static inline int tegra_fuse_get_revision(u32 *rev)
{
	return -ENOENT;
}

static inline int tegra_fuse_get_tsensor_calibration_data(u32 *calib)
{
	return -ENOENT;
}
static inline int tegra_fuse_get_tsensor_spare_bits(u32 *spare_bits)
{
	return -ENOENT;
}
int tegra_fuse_get_priv(char *priv);
#else
int tegra_fuse_get_revision(u32 *rev);
int tegra_fuse_get_tsensor_calibration_data(u32 *calib);
int tegra_fuse_get_tsensor_spare_bits(u32 *spare_bits);
static inline int tegra_fuse_get_priv(char *priv)
{
	return -ENOENT;
}
#endif

#ifdef CONFIG_ARCH_TEGRA_2x_SOC
void tegra20_init_speedo_data(void);
#else
static inline void tegra20_init_speedo_data(void) {}
#endif

#ifdef CONFIG_ARCH_TEGRA_3x_SOC
void tegra30_init_speedo_data(void);
#else
static inline void tegra30_init_speedo_data(void) {}
#endif

/* PMC and CLK control registers required to determine the
   FUSE Programming cycles */
extern u32 tegra_read_pmc_reg(int offset);
extern u32 tegra_read_clk_ctrl_reg(int offset);

extern int (*tegra_fuse_regulator_en)(int);
int tegra_soc_speedo_id(void);
void tegra_init_speedo_data(void);
int tegra_cpu_process_id(void);
int tegra_core_process_id(void);
int tegra_gpu_process_id(void);
int tegra_get_age(void);

#ifndef CONFIG_ARCH_TEGRA_2x_SOC
int tegra_package_id(void);
int tegra_cpu_speedo_id(void);
int tegra_cpu_speedo_mv(void);
int tegra_cpu_speedo_value(void);
int tegra_core_speedo_mv(void);
int tegra_gpu_speedo_id(void);
int tegra_get_sku_override(void);
int tegra_get_cpu_iddq_value(void);
#else
static inline int tegra_package_id(void) { return -1; }
static inline int tegra_cpu_speedo_id(void) { return 0; }
static inline int tegra_cpu_speedo_value(void) { return 1777; }
static inline int tegra_cpu_speedo_mv(void) { return 1000; }
static inline int tegra_core_speedo_mv(void) { return 1200; }
static inline int tegra_get_cpu_iddq_value(void) { return 0; }
#endif /* CONFIG_ARCH_TEGRA_2x_SOC */

#ifdef CONFIG_ARCH_TEGRA_12x_SOC
int tegra_cpu_speedo_0_value(void);
int tegra_cpu_speedo_1_value(void);
int tegra_soc_speedo_0_value(void);
int tegra_soc_speedo_1_value(void);
int tegra_soc_speedo_2_value(void);
int tegra_get_soc_iddq_value(void);
int tegra_get_gpu_iddq_value(void);
int tegra_gpu_speedo_value(void);
#endif

#ifdef CONFIG_ARCH_TEGRA_14x_SOC
int tegra_core_speedo_value(void);
#endif

int tegra_fuse_get_revision(u32 *rev);
int tegra_fuse_get_tsensor_calibration_data(u32 *calib);
int tegra_fuse_get_tsensor_spare_bits(u32 *spare_bits);
#if defined(CONFIG_ARCH_TEGRA_11x_SOC) || defined(CONFIG_ARCH_TEGRA_14x_SOC) \
	|| defined(CONFIG_ARCH_TEGRA_12x_SOC)
int tegra_fuse_get_tsensor_calib(int index, u32 *calib);
int tegra_fuse_calib_base_get_cp(u32 *base_cp, s32 *shifted_cp);
int tegra_fuse_calib_base_get_ft(u32 *base_ft, s32 *shifted_ft);
#endif

#endif /* TEGRA_FUSE_H */
