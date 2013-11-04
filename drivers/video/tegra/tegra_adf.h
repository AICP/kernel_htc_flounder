/*
 * Copyright (C) 2013 Google, Inc.
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

#ifndef __TEGRA_ADF_H_
#define __TEGRA_ADF_H_

#include <linux/platform_device.h>
#include <video/tegra_adf.h>
#include <mach/dc.h>

struct tegra_adf_info;

#ifdef CONFIG_ADF_TEGRA
void tegra_adf_process_vblank(struct tegra_adf_info *adf_info,
		ktime_t timestamp);
int tegra_adf_process_bandwidth_renegotiate(struct tegra_adf_info *adf_info,
		u32 used_bw_kbps, u32 available_bw_kbps);

struct tegra_adf_info *tegra_adf_init(struct platform_device *ndev,
		struct tegra_dc *dc, struct tegra_fb_data *fb_data);
void tegra_adf_unregister(struct tegra_adf_info *adf_info);
#else
static inline void tegra_adf_process_vblank(struct tegra_adf_info *adf_info,
		ktime_t timestamp)
{
}

static inline struct tegra_adf_info *tegra_adf_init(
		struct platform_device *ndev, struct tegra_dc *dc,
		struct tegra_fb_data *fb_data)
{
	return ERR_PTR(-ENOENT);
}

static inline int tegra_adf_process_bandwidth_renegotiate(
		struct tegra_adf_info *adf_info, u32 used_bw_kbps,
		u32 available_bw_kbps)
{
	return -ENOENT;
}

static inline void tegra_adf_unregister(struct tegra_adf_info *adf_info)
{
}
#endif /* CONFIG_ADF_TEGRA */

#endif /* __TEGRA_ADF_H_ */
