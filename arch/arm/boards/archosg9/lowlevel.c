/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <common.h>
#include <io.h>
#include <init.h>
#include <mach/omap4-mux.h>
#include <mach/omap4-silicon.h>
#include <mach/omap4-clock.h>
#include <mach/syslib.h>
#include <asm/barebox-arm.h>
#include <asm/barebox-arm-head.h>
#include "mux.h"

#define TPS62361_VSEL0_GPIO    7

static const struct ddr_regs ddr_regs_400_mhz_2cs = {
	.tim1         = 0x10EB0662,
	.tim2         = 0x20370DD2,
	.tim3         = 0x00B1C33F,
	.phy_ctrl_1   = 0x849FF408,
	.ref_ctrl     = 0x00000618,
	.config_init  = 0x80000EB9,
	.config_final = 0x80001AB9,
	.zq_config    = 0xD00B3215,
	.mr1          = 0x83,
	.mr2          = 0x4
};

static noinline void archosg9_init_lowlevel(void)
{
	struct dpll_param core = OMAP4_CORE_DPLL_PARAM_19M2_DDR400;
	struct dpll_param mpu = OMAP4_MPU_DPLL_PARAM_19M2_MPU600;
	struct dpll_param iva = OMAP4_IVA_DPLL_PARAM_19M2;
	struct dpll_param per = OMAP4_PER_DPLL_PARAM_19M2;
	struct dpll_param abe = OMAP4_ABE_DPLL_PARAM_19M2;
	struct dpll_param usb = OMAP4_USB_DPLL_PARAM_19M2;

	writel(CM_SYS_CLKSEL_19M2, CM_SYS_CLKSEL);

	/* Configure all DPLL's at 100% OPP */
	omap4_configure_mpu_dpll(&mpu);
	omap4_configure_iva_dpll(&iva);
	omap4_configure_per_dpll(&per);
	omap4_configure_abe_dpll(&abe);
	omap4_configure_usb_dpll(&usb);

	/* Enable all clocks */
	omap4_enable_all_clocks();

	set_muxconf_regs();

	omap4_ddr_init(&ddr_regs_400_mhz_2cs, &core);

	/* Set VCORE1 = 1.3 V, VCORE2 = VCORE3 = 1.21V */
	omap4_scale_vcores(TPS62361_VSEL0_GPIO);
	board_init_lowlevel_return();
}

void __naked __bare_init reset(void)
{
	u32 r;

	common_reset();

	r = 0x4030D000;
	__asm__ __volatile__("mov sp, %0" : : "r"(r));

	archosg9_init_lowlevel();
}