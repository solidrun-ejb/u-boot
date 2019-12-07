/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 SolidRun ltd.
 */

#ifndef __LX2_CEX7_H
#define __LX2_CEX7_H

#include <linux/sizes.h>
#include "lx2160a_common.h"

#define CONFIG_SUPPORT_EMMC_BOOT
#undef CONFIG_SYS_MMC_ENV_DEV
#define CONFIG_SYS_MMC_ENV_DEV          1
#undef CONFIG_SYS_MMC_ENV_PART
#define CONFIG_SYS_MMC_ENV_PART         1
#define CONFIG_ENV_SIZE                 0x2000          /* 8KB */
#undef CONFIG_ENV_OFFSET
#define CONFIG_ENV_OFFSET               (SZ_4M - CONFIG_ENV_SIZE)
#undef CONFIG_ENV_ADDR

/*#define CONFIG_SYS_FSL_ESDHC_USE_PIO*/
/* VID */

#define I2C_MUX_CH_VOL_MONITOR		0x2
/* Voltage monitor on channel 2*/
#define I2C_VOL_MONITOR_ADDR		0x5c
#define I2C_VOL_MONITOR_BUS_V_OFFSET	0x2
#define I2C_VOL_MONITOR_BUS_V_OVF	0x1
#define I2C_VOL_MONITOR_BUS_V_SHIFT	3
#define CONFIG_VID_FLS_ENV		"lx2160acex7_vdd_mv"
#define CONFIG_VID

/* The lowest and highest voltage allowed*/
#define VDD_MV_MIN			700
#define VDD_MV_MAX			855

/* PM Bus commands code for LTC3882*/
#define PMBUS_CMD_PAGE                  0x0
#define PMBUS_CMD_READ_VOUT             0x8B
#define PMBUS_CMD_PAGE_PLUS_WRITE       0x05
#define PMBUS_CMD_VOUT_COMMAND          0x21
#define PWM_CHANNEL0                    0x0

#define CONFIG_VOL_MONITOR_LTC3882_SET
#define CONFIG_VOL_MONITOR_LTC3882_READ

/* RTC */
#define CONFIG_SYS_RTC_BUS_NUM		4

/* MAC/PHY configuration */
#if defined(CONFIG_FSL_MC_ENET)
#define CONFIG_MII
#define CONFIG_ETHPRIME		"DPMAC17@rgmii-id"

#define RGMII_PHY_ADDR1		0x01

#endif

/* EMC2301 */
#define I2C_MUX_CH_EMC2301	0x01
#define I2C_EMC2301_ADDR	0x2f
#define I2C_EMC2301_CMD		0x40
#define I2C_EMC2301_PWM		0x80

/* EEPROM */
#undef CONFIG_ID_EEPROM /* Fixme */
#define CONFIG_SYS_I2C_EEPROM_NXID
#define CONFIG_SYS_EEPROM_BUS_NUM	           0
#define CONFIG_SYS_I2C_EEPROM_ADDR	           0x57
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	    1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS     3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS 5

/* Initial environment variables */
#define CONFIG_EXTRA_ENV_SETTINGS		\
	EXTRA_ENV_SETTINGS			\
	"lx2160acex7_vdd_mv=800\0"		\
	"BOARD=lx2160acex7\0"			\
	"xspi_bootcmd=echo Trying load from flexspi..;"		\
		"sf probe 0:0 && sf read $load_addr "		\
		"$kernel_start $kernel_size ; env exists secureboot &&"	\
		"sf read $kernelheader_addr_r $kernelheader_start "	\
		"$kernelheader_size && esbc_validate ${kernelheader_addr_r}; "\
		" bootm $load_addr#$BOARD\0"			\
	"sd_bootcmd=echo Trying load from sd card..;"		\
		"mmcinfo; mmc read $load_addr "			\
		"$kernel_addr_sd $kernel_size_sd ;"		\
		"env exists secureboot && mmc read $kernelheader_addr_r "\
		"$kernelhdr_addr_sd $kernelhdr_size_sd "	\
		" && esbc_validate ${kernelheader_addr_r};"	\
		"bootm $load_addr#$BOARD\0"

#include <asm/fsl_secure_boot.h>

#endif /* __LX2_CEX7_H */
