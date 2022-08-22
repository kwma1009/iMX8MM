/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2020 PHYTEC Messtechnik GmbH
 * Author: Teresa Remmet <t.remmet@phytec.de>
 */

#ifndef _PHYTEC_IMX8M_DDR_H
#define _PHYTEC_IMX8M_DDR_H

#define PHYTEC_MAX_OPTIONS	17
#define PHYTEC_IMX8MQ_SOM	66
#define PHYTEC_IMX8MM_SOM	69
#define PHYTEC_IMX8MP_SOM	70

#define PHYTEC_EEPROM_INVAL	0xff

enum {
	PHYTEC_API_REV0 = 0,
	PHYTEC_API_REV1,
	PHYTEC_API_REV2,
};

static const char * const phytec_som_type_str[] = {
	"PCM",
	"PCL",
	"KSM",
	"KSP",
	"PCM-KSP",
	"PCM-KSM",
	"PCL-KSP",
	"PCL-KSM",
};

struct phytec_api0_data {
	u8 pcb_rev;		/* PCB revision of SoM */
	u8 som_type;		/* SoM type */
	u8 ksp_no;		/* KSP no */
	char opt[16];		/* SoM options */
	u8 mac[6];		/* MAC address (optional) */
	u8 pad[5];		/* padding */
	u8 cksum;		/* checksum */
} __attribute__ ((__packed__));

struct phytec_api2_data {
	u8 pcb_rev;		/* PCB revision of SoM */
	u8 pcb_sub_opt_rev;	/* PCB subrevision and opt revision */
	u8 som_type;		/* SoM type */
	u8 som_no;		/* SoM number */
	u8 ksp_no;		/* KSP information */
	char opt[PHYTEC_MAX_OPTIONS]; /* SoM options */
	char bom_rev[2];	/* BOM revision */
	u8 mac[6];		/* MAC address (optional) */
	u8 crc8;		/* checksum */
} __attribute__ ((__packed__));

struct phytec_eeprom_data {
	u8 api_rev;
	union {
		struct phytec_api0_data data_api0;
		struct phytec_api2_data data_api2;
	} data;
} __attribute__ ((__packed__));

int _phytec_eeprom_data_setup(struct phytec_eeprom_data *data,
			      int bus_num, int addr, int addr_fallback);
int _phytec_eeprom_data_init(struct phytec_eeprom_data *data,
			     int bus_num, int addr);

char * __maybe_unused _phytec_get_imx8m_opt(struct phytec_eeprom_data *data);
u8 __maybe_unused _phytec_get_imx8m_ddr_size(struct phytec_eeprom_data *data);
u8 __maybe_unused _phytec_get_imx8m_spi(struct phytec_eeprom_data *data);
u8 __maybe_unused _phytec_get_imx8m_eth(struct phytec_eeprom_data *data);
void __maybe_unused _phytec_print_som_info(struct phytec_eeprom_data *data);

#define phytec_eeprom_data_setup(bus_num, addr, addr_fallback) \
	(_phytec_eeprom_data_setup(NULL, bus_num, addr, addr_fallback))
#define phytec_eeprom_data_init(bus_num, addr) \
	(_phytec_eeprom_data_init(NULL, bus_num, addr))

#define phytec_get_imx8m_opt() _phytec_get_imx8m_opt(NULL)
#define phytec_get_imx8m_ddr_size() _phytec_get_imx8m_ddr_size(NULL)
#define phytec_get_imx8m_spi() _phytec_get_imx8m_spi(NULL)
#define phytec_get_imx8m_eth() _phytec_get_imx8m_eth(NULL)
#define phytec_print_som_info() _phytec_print_som_info(NULL)

#endif
