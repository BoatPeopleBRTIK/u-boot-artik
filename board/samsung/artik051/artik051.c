#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/power.h>
#include <asm/arch/pinmux.h>
#include <dm/pinctrl.h>
#include <watchdog.h>
#include <bootcount.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
#ifdef CONFIG_HW_WATCHDOG
	unsigned int mask;
	struct exynos0200_power *pmu =
		(struct exynos0200_power *) samsung_get_base_power();

	hw_watchdog_init();

	/* PMU ignores reset signal from WDT, so we need to clear the mask */
	mask = readl(&pmu->mask_wdt_reset_request);
	mask &= ~(1 << 23);
	writel(mask, &pmu->mask_wdt_reset_request);
#endif /* CONFIG_HW_WATCHDOG */

	/* GPIO for debug UART */
	if (exynos_pinmux_config(PERIPH_ID_UART4, PINMUX_FLAG_NONE))
		debug("UART4 not configured.\n");

	/* SRAM size */
	gd->ram_size = CONFIG_SYS_SRAM_SIZE;

	return 0;
}
#endif /* CONFIG_BOARD_EARLY_INIT_F */

int dram_init(void)
{
	return 0;
}

int board_init(void)
{
	return 0;
}

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
#ifndef CONFIG_SYS_NO_FLASH
	/* set protection on BL1 and BL2 region so that we do not touch it */
	flash_protect(FLAG_PROTECT_SET,
			CONFIG_SYS_FLASH_BASE,
			CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET +
							CONFIG_ENV_SIZE - 1,
			&flash_info[0]);
#endif

	return 0;
}
#endif /* CONFIG_MISC_INIT_R */

void relocate_vectors(void)
{
}

#ifdef CONFIG_SYS_BOOTCOUNT_ADDR
#if !defined(CONFIG_BOOTCOUNT_RAM) && !defined(CONFIG_BOOTCOUNT_ENV)
void bootcount_store(ulong a)
{
	void *reg = (void *)CONFIG_SYS_BOOTCOUNT_ADDR;

#if defined(CONFIG_SYS_BOOTCOUNT_SINGLEWORD)
	raw_bootcount_store(reg, (BOOTCOUNT_MAGIC & 0xffff0000) | a);
#else
	raw_bootcount_store(reg, a);
	raw_bootcount_store(reg + 4, BOOTCOUNT_MAGIC);
#endif /* defined(CONFIG_SYS_BOOTCOUNT_SINGLEWORD */
}

ulong bootcount_load(void)
{
	struct exynos0200_power *pmu =
		(struct exynos0200_power *) samsung_get_base_power();
	void *reg = (void *)CONFIG_SYS_BOOTCOUNT_ADDR;

	/* other than that watchdog barked */
	if ((readl(&pmu->rst_stat) & (1 << 23)) == 0)
		return 0;

#if defined(CONFIG_SYS_BOOTCOUNT_SINGLEWORD)
	u32 tmp = raw_bootcount_load(reg);

	if ((tmp & 0xffff0000) != (BOOTCOUNT_MAGIC & 0xffff0000))
		return 0;
	else
		return (tmp & 0x0000ffff);
#else
	if (raw_bootcount_load(reg + 4) != BOOTCOUNT_MAGIC)
		return 0;
	else
		return raw_bootcount_load(reg);
#endif /* defined(CONFIG_SYS_BOOTCOUNT_SINGLEWORD) */
}
#endif
#endif /* CONFIG_SYS_BOOTCOUNT_ADDR */