/*
 * Copyright (C) 2013 Sascha Hauer, Pengutronix
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation.
 *
 */

#include <malloc.h>
#include <envfs.h>
#include <environment.h>
#include <bootsource.h>
#include <common.h>
#include <gpio.h>
#include <init.h>
#include <of.h>
#include <mach/bbu.h>
#include <fec.h>

#include <linux/micrel_phy.h>

#include <mach/iomux-mx6.h>
#include <mach/imx6.h>

#define GPIO_2_11_PD_CTL	MX6_PAD_CTL_PUS_100K_DOWN | MX6_PAD_CTL_PUE | MX6_PAD_CTL_PKE | \
				MX6_PAD_CTL_SPEED_MED | MX6_PAD_CTL_DSE_40ohm | MX6_PAD_CTL_HYS

#define MX6Q_PAD_SD4_DAT3__GPIO_2_11_PD (_MX6Q_PAD_SD4_DAT3__GPIO_2_11 | MUX_PAD_CTRL(GPIO_2_11_PD_CTL))
#define MX6DL_PAD_SD4_DAT3__GPIO_2_11 IOMUX_PAD(0x0734, 0x034C, 5, 0x0000, 0, GPIO_2_11_PD_CTL)

#define MX6_PHYFLEX_ERR006282	IMX_GPIO_NR(2, 11)

static void phyflex_err006282_workaround(void)
{
	/*
	 * Boards beginning with 1362.2 have the SD4_DAT3 pin connected
	 * to the CMIC. If this pin isn't toggled within 10s the boards
	 * reset. The pin is unconnected on older boards, so we do not
	 * need a check for older boards before applying this fixup.
	 */

	gpio_direction_output(MX6_PHYFLEX_ERR006282, 0);
	mdelay(2);
	gpio_direction_output(MX6_PHYFLEX_ERR006282, 1);
	mdelay(2);
	gpio_set_value(MX6_PHYFLEX_ERR006282, 0);

	if (cpu_is_mx6q())
		mxc_iomux_v3_setup_pad(MX6Q_PAD_SD4_DAT3__GPIO_2_11_PD);
	else if (cpu_is_mx6dl())
		mxc_iomux_v3_setup_pad(MX6DL_PAD_SD4_DAT3__GPIO_2_11);

	gpio_direction_input(MX6_PHYFLEX_ERR006282);
}

static int phytec_pfla02_init(void)
{
	int ret;
	char *environment_path;

	if (!of_machine_is_compatible("phytec,imx6q-pfla02") &&
			!of_machine_is_compatible("phytec,imx6dl-pfla02") &&
			!of_machine_is_compatible("phytec,imx6s-pfla02"))
		return 0;

	phyflex_err006282_workaround();

	imx6_bbu_nand_register_handler("nand", BBU_HANDLER_FLAG_DEFAULT);

	switch (bootsource_get()) {
	case BOOTSOURCE_MMC:
		environment_path = asprintf("/chosen/environment-sd%d",
					bootsource_get_instance() + 1);
		break;
	case BOOTSOURCE_NAND:
		environment_path = asprintf("/chosen/environment-nand");
		break;
	default:
	case BOOTSOURCE_SPI:
		environment_path = asprintf("/chosen/environment-spinor");
		break;
	}

	ret = of_device_enable_path(environment_path);
	if (ret < 0)
		pr_warn("Failed to enable environment partition '%s' (%d)\n",
			environment_path, ret);

	free(environment_path);

	return 0;
}
device_initcall(phytec_pfla02_init);

static int phytec_pbab0x_init(void)
{
	if (!of_machine_is_compatible("phytec,imx6x-pbab01") &&
		!of_machine_is_compatible("phytec,imx6dl-pbab05") &&
		!of_machine_is_compatible("phytec,imx6q-pbab02"))
		return 0;

	defaultenv_append_directory(defaultenv_phyflex_imx6);

	return 0;
}
device_initcall(phytec_pbab0x_init);
