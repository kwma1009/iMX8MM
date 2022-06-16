/* SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2020 PHYTEC Messtechnik GmbH
 * Author: Teresa Remmet <t.remmet@phytec.de>
 */

#ifndef __PHYCORE_IMX8MP_H
#define __PHYCORE_IMX8MP_H

#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>

#include "phycore_rauc_env.h"
#include "phycore_fitimage_env.h"

#define CONFIG_SYS_BOOTM_LEN		SZ_64M

#define CONFIG_SPL_MAX_SIZE		(152 * SZ_1K)
#define CONFIG_SYS_MONITOR_LEN		SZ_512K
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	0x300
#define CONFIG_SYS_UBOOT_BASE \
		(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

#ifdef CONFIG_SPL_BUILD
#define CONFIG_SPL_LDSCRIPT		"arch/arm/cpu/armv8/u-boot-spl.lds"
#define CONFIG_SPL_STACK		0x960000
#define CONFIG_SPL_BSS_START_ADDR	0x98FC00
#define CONFIG_SPL_BSS_MAX_SIZE		SZ_1K
#define CONFIG_SYS_SPL_MALLOC_START	0x42200000
#define CONFIG_SYS_SPL_MALLOC_SIZE	SZ_512K

#define CONFIG_SPL_ABORT_ON_RAW_IMAGE

#define CONFIG_POWER
#define CONFIG_POWER_I2C
#define CONFIG_POWER_PCA9450

#define CONFIG_SYS_I2C

#endif

#define CONFIG_EXTRA_ENV_SETTINGS \
	"image=Image\0" \
	"console=ttymxc0\0" \
	"fdt_addr=0x48000000\0" \
	"fdto_addr=0x49000000\0" \
	"bootenv_addr=0x49100000\0" \
	"fdt_file=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"ip_dyn=yes\0" \
	"bootenv=bootenv.txt\0" \
	"mmc_load_bootenv=fatload mmc ${mmcdev}:${mmcpart} ${bootenv_addr} ${bootenv}\0" \
	"mmcdev=" __stringify(CONFIG_SYS_MMC_ENV_DEV) "\0" \
	"mmcpart=" __stringify(CONFIG_SYS_MMC_IMG_LOAD_PART) "\0" \
	"mmcroot=2\0" \
	"mmcautodetect=yes\0" \
	"mmcargs=setenv bootargs console=${console},${baudrate} " \
		"root=/dev/mmcblk${mmcdev}p${mmcroot} fsck.repair=yes rootwait rw\0" \
	"loadimage=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${image}\0" \
	"loadfdt=fatload mmc ${mmcdev}:${mmcpart} ${fdt_addr} ${fdt_file}\0" \
	"mmc_load_overlay=fatload mmc ${mmcdev}:${mmcpart} ${fdto_addr} ${overlay}\0" \
	"mmc_apply_overlays=fdt address ${fdt_addr}; "  \
		"if test ${no_overlays} = 0; then " \
			"for overlay in $overlays; " \
			"do; " \
				"if run mmc_load_overlay; then " \
					"fdt resize ${filesize}; " \
					"fdt apply ${fdto_addr}; " \
				"fi; " \
			"done;" \
		"fi;\0 " \
	"mmcboot=echo Booting from mmc ...; " \
		"if run mmc_load_bootenv; then " \
			"env import -t ${bootenv_addr} ${filesize}; " \
		"fi; " \
		"run mmcargs; " \
		"run fit_test_and_run_boot; " \
		"if run loadfdt; then " \
			"run mmc_apply_overlays; " \
			"booti ${loadaddr} - ${fdt_addr}; " \
		"else " \
			"echo WARN: Cannot load the DT; " \
		"fi;\0 " \
	"nfsroot=/nfs\0" \
	"netargs=setenv bootargs console=${console},${baudrate} root=/dev/nfs ip=${nfsip} " \
		"nfsroot=${serverip}:${nfsroot},v3,tcp\0" \
	"net_load_bootenv=${get_cmd} ${bootenv_addr} ${bootenv}\0" \
	"net_load_overlay=${get_cmd} ${fdto_addr} ${overlay}\0" \
	"net_apply_overlays=fdt address ${fdt_addr}; " \
			"if test ${no_overlays} = 0; then " \
			"for overlay in $overlays; " \
			"do; " \
				"if run net_load_overlay; then " \
					"fdt resize ${filesize}; " \
					"fdt apply ${fdto_addr}; " \
				"fi; " \
			"done;" \
		"fi;\0 " \
	"netboot=echo Booting from net ...; " \
		"if test ${ip_dyn} = yes; then " \
			"setenv nfsip dhcp; " \
			"setenv get_cmd dhcp; " \
		"else " \
			"setenv nfsip ${ipaddr}:${serverip}::${netmask}::eth0:on; " \
			"setenv get_cmd tftp; " \
		"fi; " \
		"if run net_load_bootenv; then " \
			"env import -t ${bootenv_addr} ${filesize}; " \
		"fi; " \
		"run netargs; " \
		"${get_cmd} ${loadaddr} ${image}; " \
		"if ${get_cmd} ${fdt_addr} ${fdt_file}; then " \
			"run net_apply_overlays; " \
			"booti ${loadaddr} - ${fdt_addr}; " \
		"else " \
			"echo WARN: Cannot load the DT; " \
		"fi;\0" \
	"raucdev=2\0" \
	PHYCORE_RAUC_ENV_BOOTLOGIC \
	PHYCORE_FITIMAGE_ENV_BOOTLOGIC

#ifdef CONFIG_IMX_HAB
#define BOOTCOMMAND_APPEND "reset;"
#else
#define BOOTCOMMAND_APPEND ""
#endif

#define CONFIG_BOOTCOMMAND \
	"mmc dev ${mmcdev}; if mmc rescan; then " \
		"env exists dofitboot || setenv dofitboot 0;" \
		"env exists doraucboot || setenv doraucboot 0 && saveenv;" \
		"if test ${doraucboot} = 1; then " \
			"run raucboot; " \
		"elif run loadimage; then " \
			"run mmcboot; " \
		"else run netboot; " \
		"fi; " \
	"fi; " \
	BOOTCOMMAND_APPEND

#ifdef CONFIG_ENV_WRITEABLE_LIST
/* Set environment flag validation to a list of env vars that must be writable */
#define CONFIG_ENV_FLAGS_LIST_STATIC RAUC_REQUIRED_WRITABLE_ENV_FLAGS",dofitboot:dw"
#endif

/* Link Definitions */
#define CONFIG_LOADADDR			0x40480000
#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

#define CONFIG_SYS_INIT_RAM_ADDR	0x40000000
#define CONFIG_SYS_INIT_RAM_SIZE	SZ_512K
#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#define CONFIG_MMCROOT			"/dev/mmcblk2p2"  /* USDHC3 */

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		SZ_32M
#define CONFIG_SYS_SDRAM_BASE		0x40000000

#define PHYS_SDRAM			0x40000000
#define PHYS_SDRAM_SIZE			0x80000000

/* UART */
#define CONFIG_MXC_UART_BASE		UART1_BASE_ADDR

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		SZ_2K
#define CONFIG_SYS_MAXARGS		64
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

/* USDHC */
#define CONFIG_FSL_USDHC
#define CONFIG_SYS_FSL_USDHC_NUM	2
#define CONFIG_SYS_FSL_ESDHC_ADDR       0
#define CONFIG_SYS_MMC_IMG_LOAD_PART	1

/* I2C */
#define CONFIG_SYS_I2C_SPEED		100000

#endif /* __PHYCORE_IMX8MP_H */
