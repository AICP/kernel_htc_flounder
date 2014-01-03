/*
 * arch/arm/mach-tegra/board-flounder.c
 *
 * Copyright (c) 2013, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/ctype.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/serial_8250.h>
#include <linux/i2c.h>
#include <linux/i2c/i2c-hid.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/i2c-tegra.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/platform_data/tegra_usb.h>
#include <linux/spi/spi.h>
#include <linux/spi/rm31080a_ts.h>
#include <linux/maxim_sti.h>
#include <linux/memblock.h>
#include <linux/spi/spi-tegra.h>
#include <linux/nfc/pn544.h>
#include <linux/rfkill-gpio.h>
#include <linux/skbuff.h>
#include <linux/ti_wilink_st.h>
#include <linux/regulator/consumer.h>
#include <linux/smb349-charger.h>
#include <linux/max17048_battery.h>
#include <linux/leds.h>
#include <linux/i2c/at24.h>
#include <linux/of_platform.h>
#include <linux/i2c.h>
#include <linux/i2c-tegra.h>
#include <linux/platform_data/serial-tegra.h>
#include <linux/edp.h>
#include <linux/usb/tegra_usb_phy.h>
#include <linux/mfd/palmas.h>
#include <linux/clk/tegra.h>
#include <media/tegra_dtv.h>
#include <linux/clocksource.h>
#include <linux/irqchip.h>
#include <linux/irqchip/tegra.h>
#include <linux/pci-tegra.h>

#include <mach/irqs.h>
#include <mach/tegra_fiq_debugger.h>

#include <mach/pinmux.h>
#include <mach/pinmux-t12.h>
#include <mach/io_dpd.h>
#include <mach/i2s.h>
#include <mach/isomgr.h>
#include <mach/tegra_asoc_pdata.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <mach/gpio-tegra.h>
#include <mach/tegra_fiq_debugger.h>
#include <mach/xusb.h>
#include <linux/platform_data/tegra_ahci.h>
#include <linux/irqchip/tegra.h>

#include "board.h"
#include "board-flounder.h"
#include "board-common.h"
#include "board-touch-raydium.h"
#include "board-touch-maxim_sti.h"
#include "clock.h"
#include "common.h"
#include "devices.h"
#include "gpio-names.h"
#include "iomap.h"
#include "pm.h"
#include "tegra-board-id.h"
#include "../../../sound/soc/codecs/rt5506.h"
#include "../../../sound/soc/codecs/rt5677.h"
#include "../../../sound/soc/codecs/tfa9895.h"

static struct resource flounder_bluedroid_pm_resources[] = {
	[0] = {
		.name   = "shutdown_gpio",
		.start  = TEGRA_GPIO_PR1,
		.end    = TEGRA_GPIO_PR1,
		.flags  = IORESOURCE_IO,
	},
	[1] = {
		.name = "host_wake",
		.flags  = IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHEDGE,
	},
	[2] = {
		.name = "gpio_ext_wake",
		.start  = TEGRA_GPIO_PEE1,
		.end    = TEGRA_GPIO_PEE1,
		.flags  = IORESOURCE_IO,
	},
	[3] = {
		.name = "gpio_host_wake",
		.start  = TEGRA_GPIO_PU6,
		.end    = TEGRA_GPIO_PU6,
		.flags  = IORESOURCE_IO,
	},
};

static struct platform_device flounder_bluedroid_pm_device = {
	.name = "bluedroid_pm",
	.id             = 0,
	.num_resources  = ARRAY_SIZE(flounder_bluedroid_pm_resources),
	.resource       = flounder_bluedroid_pm_resources,
};

static noinline void __init flounder_setup_bluedroid_pm(void)
{
	flounder_bluedroid_pm_resources[1].start =
		flounder_bluedroid_pm_resources[1].end =
				gpio_to_irq(TEGRA_GPIO_PU6);
	platform_device_register(&flounder_bluedroid_pm_device);
}

static struct tfa9895_platform_data tfa9895_data = {
	.tfa9895_power_enable = TEGRA_GPIO_PX5,
};

struct rt5506_platform_data rt5506_data = {
	.rt5506_enable = TEGRA_GPIO_PX1,
	.rt5506_power_enable = -1,
};
struct rt5677_priv rt5677_data = {
	.vad_clock_en = TEGRA_GPIO_PX3,
};

static struct i2c_board_info __initdata rt5677_board_info = {
	I2C_BOARD_INFO("rt5677", 0x2d),
	.platform_data = &rt5677_data,
};
static struct i2c_board_info __initdata rt5506_board_info = {
	I2C_BOARD_INFO("rt5506", 0x52),
	.platform_data = &rt5506_data,
};

static struct i2c_board_info __initdata tfa9895_board_info = {
	I2C_BOARD_INFO("tfa9895", 0x34),
	.platform_data = &tfa9895_data,
};
static struct i2c_board_info __initdata tfa9895l_board_info = {
	I2C_BOARD_INFO("tfa9895l", 0x35),
};

static __initdata struct tegra_clk_init_table flounder_clk_init_table[] = {
	/* name		parent		rate		enabled */
	{ "pll_m",	NULL,		0,		false},
	{ "hda",	"pll_p",	108000000,	false},
	{ "hda2codec_2x", "pll_p",	48000000,	false},
	{ "pwm",	"pll_p",	3187500,	false},
	{ "i2s1",	"pll_a_out0",	0,		false},
	{ "i2s2",	"pll_a_out0",	0,		false},
	{ "i2s3",	"pll_a_out0",	0,		false},
	{ "i2s4",	"pll_a_out0",	0,		false},
	{ "spdif_out",	"pll_a_out0",	0,		false},
	{ "d_audio",	"clk_m",	12000000,	false},
	{ "dam0",	"clk_m",	12000000,	false},
	{ "dam1",	"clk_m",	12000000,	false},
	{ "dam2",	"clk_m",	12000000,	false},
	{ "audio1",	"i2s1_sync",	0,		false},
	{ "audio2",	"i2s2_sync",	0,		false},
	{ "audio3",	"i2s3_sync",	0,		false},
	{ "vi_sensor",	"pll_p",	150000000,	false},
	{ "vi_sensor2",	"pll_p",	150000000,	false},
	{ "cilab",	"pll_p",	150000000,	false},
	{ "cilcd",	"pll_p",	150000000,	false},
	{ "cile",	"pll_p",	150000000,	false},
	{ "i2c1",	"pll_p",	3200000,	false},
	{ "i2c2",	"pll_p",	3200000,	false},
	{ "i2c3",	"pll_p",	3200000,	false},
	{ "i2c4",	"pll_p",	3200000,	false},
	{ "i2c5",	"pll_p",	3200000,	false},
	{ "sbc1",	"pll_p",	25000000,	false},
	{ "sbc2",	"pll_p",	25000000,	false},
	{ "sbc3",	"pll_p",	25000000,	false},
	{ "sbc4",	"pll_p",	25000000,	false},
	{ "sbc5",	"pll_p",	25000000,	false},
	{ "sbc6",	"pll_p",	25000000,	false},
	{ "uarta",	"pll_p",	408000000,	false},
	{ "uartb",	"pll_p",	408000000,	false},
	{ "uartc",	"pll_p",	408000000,	false},
	{ "uartd",	"pll_p",	408000000,	false},
	{ NULL,		NULL,		0,		0},
};

static void flounder_i2c_init(void)
{
	i2c_register_board_info(1, &rt5677_board_info, 1);
	i2c_register_board_info(1, &rt5506_board_info, 1);
	i2c_register_board_info(1, &tfa9895_board_info, 1);
	i2c_register_board_info(1, &tfa9895l_board_info, 1);
}

#ifndef CONFIG_USE_OF
static struct platform_device *flounder_uart_devices[] __initdata = {
	&tegra_uarta_device,
	&tegra_uartb_device,
	&tegra_uartc_device,
};

static struct tegra_serial_platform_data flounder_uarta_pdata = {
	.dma_req_selector = 8,
	.modem_interrupt = false,
};

static struct tegra_serial_platform_data flounder_uartb_pdata = {
	.dma_req_selector = 9,
	.modem_interrupt = false,
};

static struct tegra_serial_platform_data flounder_uartc_pdata = {
	.dma_req_selector = 10,
	.modem_interrupt = false,
};
#endif
static struct tegra_serial_platform_data flounder_uarta_pdata = {
	.dma_req_selector = 8,
	.modem_interrupt = false,
};

static struct tegra_asoc_platform_data flounder_audio_pdata_rt5677 = {
	.gpio_hp_det = -1,
	.gpio_ldo1_en = TEGRA_GPIO_PK0,
	.gpio_ldo2_en = TEGRA_GPIO_PQ3,
	.gpio_reset = TEGRA_GPIO_PX4,
	.gpio_irq1 = TEGRA_GPIO_PS4,
	.gpio_wakeup = TEGRA_GPIO_PO0,
	.gpio_spkr_en = -1,
	.gpio_spkr_ldo_en = TEGRA_GPIO_PX5,
	.gpio_int_mic_en = TEGRA_GPIO_PV3,
	.gpio_ext_mic_en = TEGRA_GPIO_PS3,
	.gpio_hp_mute = -1,
	.gpio_hp_en = TEGRA_GPIO_PX1,
	.gpio_hp_ldo_en = -1,
	.gpio_codec1 = -1,
	.gpio_codec2 = -1,
	.gpio_codec3 = -1,
	.i2s_param[HIFI_CODEC]       = {
		.audio_port_id = 1,
		.is_i2s_master = 1,
		.i2s_mode = TEGRA_DAIFMT_I2S,
	},
	.i2s_param[SPEAKER]       = {
		.audio_port_id = 2,
		.is_i2s_master = 1,
		.i2s_mode = TEGRA_DAIFMT_I2S,
	},
	.i2s_param[BT_SCO] = {
		.audio_port_id = 3,
		.is_i2s_master = 1,
		.i2s_mode = TEGRA_DAIFMT_DSP_A,
	},
};

static struct tegra_spi_device_controller_data dev_cdata_rt5677 = {
	.rx_clk_tap_delay = 0,
	.tx_clk_tap_delay = 16,
};

struct spi_board_info rt5677_flounder_spi_board[1] = {
	{
	 .modalias = "rt5677_spidev",
	 .bus_num = 4,
	 .chip_select = 0,
	 .max_speed_hz = 12 * 1000 * 1000,
	 .mode = SPI_MODE_0,
	 .controller_data = &dev_cdata_rt5677,
	 },
};

static void flounder_audio_init(void)
{
	flounder_audio_pdata_rt5677.codec_name = "rt5677.1-002d";
	flounder_audio_pdata_rt5677.codec_dai_name = "rt5677-aif1";

	spi_register_board_info(&rt5677_flounder_spi_board[0],
	ARRAY_SIZE(rt5677_flounder_spi_board));
}

static struct platform_device flounder_audio_device_rt5677 = {
	.name = "tegra-snd-rt5677",
	.id = 0,
	.dev = {
		.platform_data = &flounder_audio_pdata_rt5677,
	},
};

static void __init flounder_uart_init(void)
{

#ifndef CONFIG_USE_OF
	tegra_uarta_device.dev.platform_data = &flounder_uarta_pdata;
	tegra_uartb_device.dev.platform_data = &flounder_uartb_pdata;
	tegra_uartc_device.dev.platform_data = &flounder_uartc_pdata;
	platform_add_devices(flounder_uart_devices,
			ARRAY_SIZE(flounder_uart_devices));
#endif
	tegra_uarta_device.dev.platform_data = &flounder_uarta_pdata;
	if (!is_tegra_debug_uartport_hs()) {
		int debug_port_id = uart_console_debug_init(0);
		if (debug_port_id < 0)
			return;

#ifdef CONFIG_TEGRA_FIQ_DEBUGGER
		tegra_serial_debug_init_irq_mode(TEGRA_UARTA_BASE, INT_UARTA, NULL, -1, -1);
#else
		platform_device_register(uart_console_debug_device);
#endif
	} else {
		tegra_uarta_device.dev.platform_data = &flounder_uarta_pdata;
		platform_device_register(&tegra_uarta_device);
	}
}

static struct resource tegra_rtc_resources[] = {
	[0] = {
		.start = TEGRA_RTC_BASE,
		.end = TEGRA_RTC_BASE + TEGRA_RTC_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = INT_RTC,
		.end = INT_RTC,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device tegra_rtc_device = {
	.name = "tegra_rtc",
	.id   = -1,
	.resource = tegra_rtc_resources,
	.num_resources = ARRAY_SIZE(tegra_rtc_resources),
};

static struct platform_device *flounder_devices[] __initdata = {
	&tegra_pmu_device,
	&tegra_rtc_device,
	&tegra_udc_device,
#if defined(CONFIG_TEGRA_WATCHDOG)
	&tegra_wdt0_device,
#endif
#if defined(CONFIG_TEGRA_AVP)
	&tegra_avp_device,
#endif
#if defined(CONFIG_CRYPTO_DEV_TEGRA_SE)
	&tegra12_se_device,
#endif
	&tegra_ahub_device,
	&tegra_dam_device0,
	&tegra_dam_device1,
	&tegra_dam_device2,
	&tegra_i2s_device1,
	&tegra_i2s_device2,
	&tegra_i2s_device3,
	&tegra_i2s_device4,
	&flounder_audio_device_rt5677,
	&tegra_spdif_device,
	&spdif_dit_device,
	&bluetooth_dit_device,
	&tegra_hda_device,
#if defined(CONFIG_CRYPTO_DEV_TEGRA_AES)
	&tegra_aes_device,
#endif
};

static struct tegra_usb_platform_data tegra_udc_pdata = {
	.port_otg = true,
	.has_hostpc = true,
	.unaligned_dma_buf_supported = false,
	.phy_intf = TEGRA_USB_PHY_INTF_UTMI,
	.op_mode = TEGRA_USB_OPMODE_DEVICE,
	.u_data.dev = {
		.vbus_pmu_irq = 0,
		.vbus_gpio = -1,
		.charging_supported = false,
		.remote_wakeup_supported = false,
	},
	.u_cfg.utmi = {
		.hssync_start_delay = 0,
		.elastic_limit = 16,
		.idle_wait_delay = 17,
		.term_range_adj = 6,
		.xcvr_setup = 8,
		.xcvr_lsfslew = 2,
		.xcvr_lsrslew = 2,
		.xcvr_setup_offset = 0,
		.xcvr_use_fuses = 1,
	},
};

static struct tegra_usb_platform_data tegra_ehci1_utmi_pdata = {
	.port_otg = true,
	.has_hostpc = true,
	.unaligned_dma_buf_supported = false,
	.phy_intf = TEGRA_USB_PHY_INTF_UTMI,
	.op_mode = TEGRA_USB_OPMODE_HOST,
	.u_data.host = {
		.vbus_gpio = -1,
		.hot_plug = false,
		.remote_wakeup_supported = true,
		.power_off_on_suspend = true,
	},
	.u_cfg.utmi = {
		.hssync_start_delay = 0,
		.elastic_limit = 16,
		.idle_wait_delay = 17,
		.term_range_adj = 6,
		.xcvr_setup = 15,
		.xcvr_lsfslew = 0,
		.xcvr_lsrslew = 3,
		.xcvr_setup_offset = 0,
		.xcvr_use_fuses = 1,
		.vbus_oc_map = 0x4,
		.xcvr_hsslew_lsb = 2,
	},
};

static struct tegra_usb_platform_data tegra_ehci2_utmi_pdata = {
	.port_otg = false,
	.has_hostpc = true,
	.unaligned_dma_buf_supported = false,
	.phy_intf = TEGRA_USB_PHY_INTF_UTMI,
	.op_mode = TEGRA_USB_OPMODE_HOST,
	.u_data.host = {
		.vbus_gpio = -1,
		.hot_plug = false,
		.remote_wakeup_supported = true,
		.power_off_on_suspend = true,
	},
	.u_cfg.utmi = {
		.hssync_start_delay = 0,
		.elastic_limit = 16,
		.idle_wait_delay = 17,
		.term_range_adj = 6,
		.xcvr_setup = 8,
		.xcvr_lsfslew = 2,
		.xcvr_lsrslew = 2,
		.xcvr_setup_offset = 0,
		.xcvr_use_fuses = 1,
		.vbus_oc_map = 0x5,
	},
};

static struct tegra_usb_platform_data tegra_ehci3_utmi_pdata = {
	.port_otg = false,
	.has_hostpc = true,
	.unaligned_dma_buf_supported = false,
	.phy_intf = TEGRA_USB_PHY_INTF_UTMI,
	.op_mode = TEGRA_USB_OPMODE_HOST,
	.u_data.host = {
		.vbus_gpio = -1,
		.hot_plug = false,
		.remote_wakeup_supported = true,
		.power_off_on_suspend = true,
	},
	.u_cfg.utmi = {
	.hssync_start_delay = 0,
		.elastic_limit = 16,
		.idle_wait_delay = 17,
		.term_range_adj = 6,
		.xcvr_setup = 8,
		.xcvr_lsfslew = 2,
		.xcvr_lsrslew = 2,
		.xcvr_setup_offset = 0,
		.xcvr_use_fuses = 1,
		.vbus_oc_map = 0x5,
	},
};

static struct tegra_usb_otg_data tegra_otg_pdata = {
	.ehci_device = &tegra_ehci1_device,
	.ehci_pdata = &tegra_ehci1_utmi_pdata,
};

static void flounder_usb_init(void)
{
	int usb_port_owner_info = tegra_get_usb_port_owner_info();

	/* Device cable is detected through PMU Interrupt */
	tegra_udc_pdata.support_pmu_vbus = true;
	tegra_ehci1_utmi_pdata.support_pmu_vbus = true;
	tegra_otg_pdata.vbus_extcon_dev_name = "palmas-extcon";
	/* Host cable is detected through PMU Interrupt */
	tegra_udc_pdata.id_det_type = TEGRA_USB_PMU_ID;
	tegra_ehci1_utmi_pdata.id_det_type = TEGRA_USB_PMU_ID;
	tegra_otg_pdata.id_extcon_dev_name = "palmas-extcon";

	if (!(usb_port_owner_info & UTMI1_PORT_OWNER_XUSB)) {
		tegra_otg_pdata.is_xhci = false;
		tegra_udc_pdata.u_data.dev.is_xhci = false;
	} else {
		tegra_otg_pdata.is_xhci = true;
		tegra_udc_pdata.u_data.dev.is_xhci = true;
	}
	tegra_otg_device.dev.platform_data = &tegra_otg_pdata;
	platform_device_register(&tegra_otg_device);
	/* Setup the udc platform data */
	tegra_udc_device.dev.platform_data = &tegra_udc_pdata;

	if (!(usb_port_owner_info & UTMI2_PORT_OWNER_XUSB)) {
		tegra_ehci2_device.dev.platform_data =
			&tegra_ehci2_utmi_pdata;
		platform_device_register(&tegra_ehci2_device);
	}
	if (!(usb_port_owner_info & UTMI2_PORT_OWNER_XUSB)) {
		tegra_ehci3_device.dev.platform_data = &tegra_ehci3_utmi_pdata;
		platform_device_register(&tegra_ehci3_device);
	}
}

static struct tegra_xusb_board_data xusb_bdata = {
	.portmap = TEGRA_XUSB_SS_P0 | TEGRA_XUSB_USB2_P0 | TEGRA_XUSB_SS_P1 |
			TEGRA_XUSB_USB2_P1 | TEGRA_XUSB_USB2_P2,
	.supply = {
		.utmi_vbuses = {
			"usb_vbus0", "usb_vbus1", "usb_vbus2"
		},
		.s3p3v = "hvdd_usb",
		.s1p8v = "avdd_pll_utmip",
		.vddio_hsic = "vddio_hsic",
		.s1p05v = "avddio_usb",
	},

	.hsic[0] = {
		.rx_strobe_trim = 0x1,
		.rx_data_trim = 0x1,
		.tx_rtune_n = 0x8,
		.tx_rtune_p = 0xa,
		.tx_slew_n = 0,
		.tx_slew_p = 0,
		.auto_term_en = true,
		.strb_trim_val = 0x22,
		.pretend_connect = false,
	},
	.uses_external_pmic = false,
};

static void flounder_xusb_init(void)
{
	int usb_port_owner_info = tegra_get_usb_port_owner_info();

	xusb_bdata.lane_owner = (u8) tegra_get_lane_owner_info();

	xusb_bdata.gpio_controls_muxed_ss_lanes = false;

	xusb_bdata.ss_portmap =
		(TEGRA_XUSB_SS_PORT_MAP_USB2_P0 << 0) |
		(TEGRA_XUSB_SS_PORT_MAP_USB2_P2 << 4);

	if (!(usb_port_owner_info & UTMI1_PORT_OWNER_XUSB))
		xusb_bdata.portmap &= ~(TEGRA_XUSB_USB2_P0 |
			TEGRA_XUSB_SS_P0);

	if (!(usb_port_owner_info & UTMI2_PORT_OWNER_XUSB))
		xusb_bdata.portmap &= ~(TEGRA_XUSB_USB2_P1 |
			TEGRA_XUSB_USB2_P2 | TEGRA_XUSB_SS_P1);

	if (usb_port_owner_info & HSIC1_PORT_OWNER_XUSB)
		xusb_bdata.portmap |= TEGRA_XUSB_HSIC_P0;

	if (usb_port_owner_info & HSIC2_PORT_OWNER_XUSB)
		xusb_bdata.portmap |= TEGRA_XUSB_HSIC_P1;

	if (xusb_bdata.portmap)
		tegra_xusb_init(&xusb_bdata);
}

#ifndef CONFIG_USE_OF
static struct platform_device *flounder_spi_devices[] __initdata = {
	&tegra11_spi_device1,
	&tegra11_spi_device4,
};

static struct tegra_spi_platform_data flounder_spi1_pdata = {
	.dma_req_sel		= 15,
	.spi_max_frequency	= 25000000,
	.clock_always_on	= false,
};

static struct tegra_spi_platform_data flounder_spi4_pdata = {
	.dma_req_sel		= 18,
	.spi_max_frequency	= 25000000,
	.clock_always_on	= false,
};

static void __init flounder_spi_init(void)
{
	tegra11_spi_device1.dev.platform_data = &flounder_spi1_pdata;
	tegra11_spi_device4.dev.platform_data = &flounder_spi4_pdata;
	platform_add_devices(flounder_spi_devices,
			ARRAY_SIZE(flounder_spi_devices));
}
#else
static void __init flounder_spi_init(void)
{
}
#endif

#ifdef CONFIG_USE_OF
static struct of_dev_auxdata flounder_auxdata_lookup[] __initdata = {
	OF_DEV_AUXDATA("nvidia,tegra114-spi", 0x7000d400, "spi-tegra114.0",
				NULL),
	OF_DEV_AUXDATA("nvidia,tegra114-spi", 0x7000d600, "spi-tegra114.1",
				NULL),
	OF_DEV_AUXDATA("nvidia,tegra114-spi", 0x7000d800, "spi-tegra114.2",
				NULL),
	OF_DEV_AUXDATA("nvidia,tegra114-spi", 0x7000da00, "spi-tegra114.3",
				NULL),
	OF_DEV_AUXDATA("nvidia,tegra114-spi", 0x7000dc00, "spi-tegra114.4",
				NULL),
	OF_DEV_AUXDATA("nvidia,tegra114-spi", 0x7000de00, "spi-tegra114.5",
				NULL),
	OF_DEV_AUXDATA("nvidia,tegra124-apbdma", 0x60020000, "tegra-apbdma",
				NULL),
	OF_DEV_AUXDATA("nvidia,tegra124-host1x", TEGRA_HOST1X_BASE, "host1x",
		NULL),
	OF_DEV_AUXDATA("nvidia,tegra124-gk20a", 0x538F0000, "gk20a", NULL),
#ifdef CONFIG_ARCH_TEGRA_VIC
	OF_DEV_AUXDATA("nvidia,tegra124-vic", TEGRA_VIC_BASE, "vic03", NULL),
#endif
	OF_DEV_AUXDATA("nvidia,tegra124-msenc", TEGRA_MSENC_BASE, "msenc",
		NULL),
	OF_DEV_AUXDATA("nvidia,tegra124-vi", TEGRA_VI_BASE, "vi", NULL),
	OF_DEV_AUXDATA("nvidia,tegra124-isp", TEGRA_ISP_BASE, "isp", NULL),
	OF_DEV_AUXDATA("nvidia,tegra124-tsec", TEGRA_TSEC_BASE, "tsec", NULL),
	OF_DEV_AUXDATA("nvidia,tegra114-hsuart", 0x70006000, "serial-tegra.0",
				NULL),
	OF_DEV_AUXDATA("nvidia,tegra114-hsuart", 0x70006040, "serial-tegra.1",
				NULL),
	OF_DEV_AUXDATA("nvidia,tegra114-hsuart", 0x70006200, "serial-tegra.2",
				NULL),
	OF_DEV_AUXDATA("nvidia,tegra124-i2c", 0x7000c000, "tegra12-i2c.0",
				NULL),
	OF_DEV_AUXDATA("nvidia,tegra124-i2c", 0x7000c400, "tegra12-i2c.1",
				NULL),
	OF_DEV_AUXDATA("nvidia,tegra124-i2c", 0x7000c500, "tegra12-i2c.2",
				NULL),
	OF_DEV_AUXDATA("nvidia,tegra124-i2c", 0x7000c700, "tegra12-i2c.3",
				NULL),
	OF_DEV_AUXDATA("nvidia,tegra124-i2c", 0x7000d000, "tegra12-i2c.4",
				NULL),
	OF_DEV_AUXDATA("nvidia,tegra124-i2c", 0x7000d100, "tegra12-i2c.5",
				NULL),
	{}
};
#endif

static int __init flounder_touch_init(void)
{
	return 0;
}

static void __init tegra_flounder_early_init(void)
{
	tegra_clk_init_from_table(flounder_clk_init_table);
	tegra_clk_verify_parents();
	tegra_soc_device_init("flounder");
}

static struct tegra_dtv_platform_data flounder_dtv_pdata = {
	.dma_req_selector = 11,
};

static void __init flounder_dtv_init(void)
{
	tegra_dtv_device.dev.platform_data = &flounder_dtv_pdata;
	platform_device_register(&tegra_dtv_device);
}

static struct tegra_io_dpd pexbias_io = {
	.name			= "PEX_BIAS",
	.io_dpd_reg_index	= 0,
	.io_dpd_bit		= 4,
};
static struct tegra_io_dpd pexclk1_io = {
	.name			= "PEX_CLK1",
	.io_dpd_reg_index	= 0,
	.io_dpd_bit		= 5,
};
static struct tegra_io_dpd pexclk2_io = {
	.name			= "PEX_CLK2",
	.io_dpd_reg_index	= 0,
	.io_dpd_bit		= 6,
};

static void __init tegra_flounder_late_init(void)
{
	platform_device_register(&tegra_pinmux_device);
	flounder_pinmux_init();

	tegra_ram_console_init();
	flounder_uart_init();
	flounder_usb_init();
	flounder_xusb_init();
	flounder_i2c_init();
	flounder_spi_init();
	flounder_audio_init();
	platform_add_devices(flounder_devices, ARRAY_SIZE(flounder_devices));
	//tegra_ram_console_debug_init();
	tegra_io_dpd_init();
	flounder_sdhci_init();
	flounder_regulator_init();
	flounder_dtv_init();
	flounder_suspend_init();
	flounder_emc_init();

	flounder_edp_init();
	isomgr_init();
	flounder_touch_init();
	flounder_panel_init();
	flounder_kbc_init();

	/* put PEX pads into DPD mode to save additional power */
	tegra_io_dpd_enable(&pexbias_io);
	tegra_io_dpd_enable(&pexclk1_io);
	tegra_io_dpd_enable(&pexclk2_io);

#ifdef CONFIG_TEGRA_WDT_RECOVERY
	tegra_wdt_recovery_init();
#endif

	flounder_sensors_init();

	flounder_soctherm_init();

	flounder_setup_bluedroid_pm();
	tegra_register_fuse();
}

static void __init flounder_ramconsole_reserve(unsigned long size)
{
	tegra_ram_console_debug_reserve(size);
}

static void __init tegra_flounder_init_early(void)
{
	flounder_rail_alignment_init();
	tegra12x_init_early();
}

static void __init tegra_flounder_dt_init(void)
{
	tegra_flounder_early_init();
#ifdef CONFIG_USE_OF
	of_platform_populate(NULL,
		of_default_bus_match_table, flounder_auxdata_lookup,
		&platform_bus);
#endif

	tegra_flounder_late_init();
}

static void __init tegra_flounder_reserve(void)
{
#if defined(CONFIG_NVMAP_CONVERT_CARVEOUT_TO_IOVMM) || \
		defined(CONFIG_TEGRA_NO_CARVEOUT)
	/* 1920*1200*4*2 = 18432000 bytes */
	tegra_reserve(0, SZ_16M + SZ_2M, SZ_16M);
#else
	tegra_reserve(SZ_1G, SZ_16M + SZ_2M, SZ_4M);
#endif
	flounder_ramconsole_reserve(SZ_1M);
}

static const char * const flounder_dt_board_compat[] = {
	"google,flounder",
	NULL
};

DT_MACHINE_START(FLOUNDER, "flounder")
	.atag_offset	= 0x100,
	.smp		= smp_ops(tegra_smp_ops),
	.map_io		= tegra_map_common_io,
	.reserve	= tegra_flounder_reserve,
	.init_early	= tegra_flounder_init_early,
	.init_irq	= irqchip_init,
	.init_time	= clocksource_of_init,
	.init_machine	= tegra_flounder_dt_init,
	.restart	= tegra_assert_system_reset,
	.dt_compat	= flounder_dt_board_compat,
	.init_late      = tegra_init_late
MACHINE_END
