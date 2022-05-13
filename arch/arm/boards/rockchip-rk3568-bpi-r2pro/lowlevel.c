// SPDX-License-Identifier: GPL-2.0-only

#include <common.h>
#include <linux/sizes.h>
#include <asm/barebox-arm-head.h>
#include <asm/barebox-arm.h>
#include <mach/hardware.h>
#include <mach/atf.h>
#include <debug_ll.h>
#include <mach/rockchip.h>

extern char __dtb_rk3568_bpi_r2_pro_start[];
extern char __dtb_rk3568_bpi_r2_pro_v00_start[];

static noinline void rk3568_start(void *fdt_blob_fixed_offset)
{
	void *fdt;

	fdt = fdt_blob_fixed_offset;

	if (current_el() == 3) {
		rk3568_lowlevel_init();
		rk3568_atf_load_bl31(fdt);
		/* not reached */
	}

	barebox_arm_entry(RK3568_DRAM_BOTTOM, 0x80000000 - RK3568_DRAM_BOTTOM, fdt);
}

static void start_rk3568_bpi_r2pro_common(void *fdt_blob_fixed_offset)
{
	/*
	 * Image execution starts at 0x0, but this is used for ATF and
	 * OP-TEE later, so move away from here.
	 */
	if (current_el() == 3)
		relocate_to_adr_full(RK3568_BAREBOX_LOAD_ADDRESS);
	else
		relocate_to_current_adr();

	setup_c();

	rk3568_start(fdt_blob_fixed_offset);
}

ENTRY_FUNCTION(start_rk3568_bpi_r2pro, r0, r1, r2)
{
	/*
	 * set iodomain vccio6 to 1.8V needed for GMAC1 to work.
	 * vccio4 (gmac0/switch) needs to stay at 3v3 (default)
	 */
	//set bit 6 in PMU_GRF_IO_VSEL0 for vccio6 1v8
	writel(RK_SETBITS(BIT(6)), PMU_GRF_IO_VSEL0);
	//clear bit 6 for 3v3 as it was set to 1v8
	writel(RK_CLRBITS(BIT(6)), PMU_GRF_IO_VSEL1);

	start_rk3568_bpi_r2pro_common(__dtb_rk3568_bpi_r2_pro_start);
}

ENTRY_FUNCTION(start_rk3568_bpi_r2pro_v00, r0, r1, r2)
{
	/*
	 * set iodomain vccio4 to 1.8V needed for GMAC0 to work.
	 * vccio6 (gmac1/switch) needs to stay at 3v3 (default)
	 */
	//set bit 4 in PMU_GRF_IO_VSEL0 for vccio4 1v8
	writel(RK_SETBITS(BIT(4)), PMU_GRF_IO_VSEL0);
	//clear bit 4 for 3v3 as it was set to 1v8
	writel(RK_CLRBITS(BIT(4)), PMU_GRF_IO_VSEL1);

	start_rk3568_bpi_r2pro_common(__dtb_rk3568_bpi_r2_pro_v00_start);
}
