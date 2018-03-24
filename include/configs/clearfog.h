/*
 * Copyright (C) 2015 Stefan Roese <sr@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _CONFIG_CLEARFOG_H
#define _CONFIG_CLEARFOG_H

/*
 * High Level Configuration Options (easy to change)
 */
#include <linux/sizes.h>

#define CONFIG_DISPLAY_BOARDINFO_LATE

/*
 * TEXT_BASE needs to be below 16MiB, since this area is scrubbed
 * for DDR ECC byte filling in the SPL before loading the main
 * U-Boot into it.
 */
#define	CONFIG_SYS_TEXT_BASE	0x00800000
#define CONFIG_SYS_TCLK		250000000	/* 250MHz */

#ifndef CONFIG_ENV_IS_ON_SELECTED_BOOT_DEVICE
#define CONFIG_MVEBU_SPL_BOOT_DEVICE_SDHC
#define CONFIG_ENV_IS_IN_MMC
#endif

/*
 * Commands configuration
 */

/* SPI NOR flash default params, used by sf commands */
#define CONFIG_SF_DEFAULT_BUS		1

/*
 * SDIO/MMC Card Configuration
 */
#define CONFIG_SYS_MMC_BASE		MVEBU_SDIO_BASE

/* Partition support */

/* Additional FS support/configuration */
#define CONFIG_SUPPORT_VFAT

/* USB/EHCI configuration */
#define CONFIG_EHCI_IS_TDI

#define CONFIG_ENV_MIN_ENTRIES		128

/* Environment Organization */
#define CONFIG_ENV_SECT_SIZE		0x200
#define CONFIG_ENV_SIZE			(8 * 1024)
#define CONFIG_ENV_OFFSET		(SZ_1M - CONFIG_ENV_SIZE)
#define CONFIG_ENV_ADDR			CONFIG_ENV_OFFSET

#ifdef CONFIG_ENV_IS_IN_MMC
#ifdef CONFIG_MVEBU_SPL_BOOT_DEVICE_MMC
#define CONFIG_SYS_MMC_ENV_DEV          0
#define CONFIG_SYS_MMC_ENV_PART         1
#else
#define CONFIG_SYS_MMC_ENV_DEV          0
#endif
#endif

#if defined(CONFIG_CMD_SCSI) || defined(CONFIG_CMD_BUBT) || defined(CONFIG_ENV_IS_IN_SATA)
#define CONFIG_SYS_SATA_ENV_DEV		0
#endif

#ifdef CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_SPI_BUS		CONFIG_SF_DEFAULT_BUS
#endif

#ifdef CONFIG_CMD_MMC
#define CONFIG_SUPPORT_EMMC_BOOT
#endif

#ifdef CONFIG_CMD_NET
#define CONFIG_TFTP_TSIZE
#endif

#define CONFIG_PHY_MARVELL		/* there is a marvell phy */
#define PHY_ANEG_TIMEOUT	8000	/* PHY needs a longer aneg time */
#define CONFIG_OVERWRITE_ETHADDR_ONCE
#define NET_SEQ_MACADDR_FROM_ENV
#define NET_RANDOM_ETHADDR_TO_ENV
#define FDT_SET_MACADDR_FROM_ENV

/* PCIe support */
#ifndef CONFIG_SPL_BUILD
#define CONFIG_PCI_MVEBU
#define CONFIG_PCI_SCAN_SHOW
#endif

/* SATA support */
#ifdef CONFIG_SCSI
#define CONFIG_LIBATA
#define CONFIG_SCSI_AHCI
#define CONFIG_SCSI_AHCI_PLAT
#define CONFIG_SYS_SCSI_MAX_SCSI_ID	1
#define CONFIG_SYS_SCSI_MAX_LUN		1
#define CONFIG_SYS_SCSI_MAX_DEVICE	(CONFIG_SYS_SCSI_MAX_SCSI_ID * \
					CONFIG_SYS_SCSI_MAX_LUN)
#endif

#define CONFIG_SYS_ALT_MEMTEST

/* Keep device tree and initrd in lower memory so the kernel can access them */
#define RELOCATION_LIMITS_ENV_SETTINGS	\
	"fdt_high=0x10000000\0"		\
	"initrd_high=0x10000000\0"

/* Defines for SPL */
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_SIZE			(140 << 10)
#define CONFIG_SPL_TEXT_BASE		0x40000030
#define CONFIG_SPL_MAX_SIZE		(CONFIG_SPL_SIZE - 0x0030)

#define CONFIG_SPL_BSS_START_ADDR	(0x40000000 + CONFIG_SPL_SIZE)
#define CONFIG_SPL_BSS_MAX_SIZE		(16 << 10)

#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_MALLOC_SIMPLE
#endif

#define CONFIG_SPL_STACK		(0x40000000 + ((192 - 16) << 10))
#define CONFIG_SPL_BOOTROM_SAVE		(CONFIG_SPL_STACK + 4)

#ifdef CONFIG_MVEBU_SPL_BOOT_DEVICE_SPI
/* SPL related SPI defines */
#define CONFIG_SPL_SPI_LOAD
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x20000
#define CONFIG_SYS_U_BOOT_OFFS		CONFIG_SYS_SPI_U_BOOT_OFFS
#endif

#if defined(CONFIG_MVEBU_SPL_BOOT_DEVICE_MMC) || defined(CONFIG_MVEBU_SPL_BOOT_DEVICE_SDHC)
/* SPL related MMC defines */
#define CONFIG_SYS_MMC_U_BOOT_OFFS		(160 << 10)
#define CONFIG_SYS_U_BOOT_OFFS			CONFIG_SYS_MMC_U_BOOT_OFFS
#ifdef CONFIG_SPL_BUILD
#define CONFIG_FIXED_SDHCI_ALIGNED_BUFFER	0x00180000	/* in SDRAM */
#endif
#endif

#if defined(CONFIG_MVEBU_SPL_BOOT_DEVICE_SDHC)
#undef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR
#ifdef CONFIG_CMD_GPT
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR 0x162
#else
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR 0x141
#endif
#endif

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

/* Include the common distro boot environment */
#ifndef CONFIG_SPL_BUILD
#include <config_distro_defaults.h>

#ifdef CONFIG_MMC
#define BOOT_TARGET_DEVICES_MMC(func) func(MMC, mmc, 0)
#else
#define BOOT_TARGET_DEVICES_MMC(func)
#endif

#if defined(CONFIG_CMD_PXE)
#define BOOT_TARGET_DEVICES_PXE(func) func(PXE, pxe, na)
#else
#define BOOT_TARGET_DEVICES_PXE(func)
#endif

#if defined(CONFIG_CMD_SCSI)
#define BOOT_TARGET_DEVICES_SCSI(func) func(SCSI, scsi, 0)
#else
#define BOOT_TARGET_DEVICES_SCSI(func)
#endif

#ifdef CONFIG_USB_STORAGE
#define BOOT_TARGET_DEVICES_USB(func) func(USB, usb, 0)
#else
#define BOOT_TARGET_DEVICES_USB(func)
#endif

#if defined(CONFIG_CMD_DHCP)
#define BOOT_TARGET_DEVICES_DHCP(func) func(DHCP, dhcp, na)
#else
#define BOOT_TARGET_DEVICES_DHCP(func)
#endif

#ifdef CONFIG_MVEBU_SPL_BOOT_DEVICE_SATA
#define BOOT_TARGET_DEVICES(func) \
	BOOT_TARGET_DEVICES_SCSI(func) \
	BOOT_TARGET_DEVICES_MMC(func) \
	BOOT_TARGET_DEVICES_USB(func) \
	BOOT_TARGET_DEVICES_PXE(func) \
	BOOT_TARGET_DEVICES_DHCP(func)
#else
#define BOOT_TARGET_DEVICES(func) \
	BOOT_TARGET_DEVICES_MMC(func) \
	BOOT_TARGET_DEVICES_USB(func) \
	BOOT_TARGET_DEVICES_SCSI(func) \
	BOOT_TARGET_DEVICES_PXE(func) \
	BOOT_TARGET_DEVICES_DHCP(func)
#endif

#define KERNEL_ADDR_R	__stringify(0x800000)
#define FDT_ADDR_R	__stringify(0x100000)
#define RAMDISK_ADDR_R	__stringify(0x1800000)
#define SCRIPT_ADDR_R	__stringify(0x200000)
#define PXEFILE_ADDR_R	__stringify(0x300000)

#define LOAD_ADDRESS_ENV_SETTINGS \
	"kernel_addr_r=" KERNEL_ADDR_R "\0" \
	"fdt_addr_r=" FDT_ADDR_R "\0" \
	"ramdisk_addr_r=" RAMDISK_ADDR_R "\0" \
	"scriptaddr=" SCRIPT_ADDR_R "\0" \
	"pxefile_addr_r=" PXEFILE_ADDR_R "\0"

#include <config_distro_bootcmd.h>

#ifdef CONFIG_TARGET_CLEARFOG_BASE
#define DEFAULT_DEVICE_TREE "armada-388-clearfog-base.dtb"
#else
#define DEFAULT_DEVICE_TREE "armada-388-clearfog.dtb"
#endif

#define CONFIG_EXTRA_ENV_SETTINGS \
	RELOCATION_LIMITS_ENV_SETTINGS \
	LOAD_ADDRESS_ENV_SETTINGS \
	"fdtfile=" DEFAULT_DEVICE_TREE "\0" \
	"console=ttyS0,115200\0" \
	BOOTENV

#endif /* CONFIG_SPL_BUILD */

#endif /* _CONFIG_CLEARFOG_H */
