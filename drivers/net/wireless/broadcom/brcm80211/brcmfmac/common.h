// SPDX-License-Identifier: ISC
/*
 * Copyright (c) 2014 Broadcom Corporation
 */
#ifndef BRCMFMAC_COMMON_H
#define BRCMFMAC_COMMON_H

#include <linux/platform_device.h>
#include "fwil_types.h"

#define BRCMF_FW_ALTPATH_LEN			256

#define BRCMFMAC_COUNTRY_BUF_SZ		4

/**
 * enum brcmf_bus_type - Bus type identifier. Currently SDIO, USB and PCIE are
 *			 supported.
 */
enum brcmf_bus_type {
	BRCMF_BUSTYPE_SDIO,
	BRCMF_BUSTYPE_USB,
	BRCMF_BUSTYPE_PCIE
};


/**
 * struct brcmfmac_sdio_pd - SDIO Device specific platform data.
 *
 * @txglomsz:		SDIO txglom size. Use 0 if default of driver is to be
 *			used.
 * @drive_strength:	is the preferred drive_strength to be used for the SDIO
 *			pins. If 0 then a default value will be used. This is
 *			the target drive strength, the exact drive strength
 *			which will be used depends on the capabilities of the
 *			device.
 * @oob_irq_supported:	does the board have support for OOB interrupts. SDIO
 *			in-band interrupts are relatively slow and for having
 *			less overhead on interrupt processing an out of band
 *			interrupt can be used. If the HW supports this then
 *			enable this by setting this field to true and configure
 *			the oob related fields.
 * @oob_irq_nr,
 * @oob_irq_flags:	the OOB interrupt information. The values are used for
 *			registering the irq using request_irq function.
 * @sd_head_align:	alignment requirement for start of data buffer.
 * @sd_sgentry_align:	length alignment requirement for each sg entry.
 */
struct brcmfmac_sdio_pd {
	int		txglomsz;
	unsigned int	drive_strength;
	bool		oob_irq_supported;
	unsigned int	oob_irq_nr;
	unsigned long	oob_irq_flags;
	unsigned short	sd_head_align;
	unsigned short	sd_sgentry_align;
};

/**
 * struct brcmfmac_pd_cc_entry - Struct for translating user space country code
 *				 (iso3166) to firmware country code and
 *				 revision.
 *
 * @iso3166:	iso3166 alpha 2 country code string.
 * @cc:		firmware country code string.
 * @rev:	firmware country code revision.
 */
struct brcmfmac_pd_cc_entry {
	char	iso3166[BRCMFMAC_COUNTRY_BUF_SZ];
	char	cc[BRCMFMAC_COUNTRY_BUF_SZ];
	s32	rev;
};

/**
 * struct brcmfmac_pd_cc - Struct for translating country codes as set by user
 *			   space to a country code and rev which can be used by
 *			   firmware.
 *
 * @table_size:	number of entries in table (> 0)
 * @table:	array of 1 or more elements with translation information.
 */
struct brcmfmac_pd_cc {
	int				table_size;
	struct brcmfmac_pd_cc_entry	table[];
};


/* Definitions for the module global and device specific settings are defined
 * here. Two structs are used for them. brcmf_mp_global_t and brcmf_mp_device.
 * The mp_global is instantiated once in a global struct and gets initialized
 * by the common_attach function which should be called before any other
 * (module) initiliazation takes place. The device specific settings is part
 * of the drvr struct and should be initialized on every brcmf_attach.
 */

/**
 * struct brcmf_mp_global_t - Global module parameters.
 *
 * @firmware_path: Alternative firmware path.
 */
struct brcmf_mp_global_t {
	char	firmware_path[BRCMF_FW_ALTPATH_LEN];
};

extern struct brcmf_mp_global_t brcmf_mp_global;

/**
 * struct brcmf_mp_device - Device module parameters.
 *
 * @p2p_enable: Legacy P2P0 enable (old wpa_supplicant).
 * @feature_disable: Feature_disable bitmask.
 * @fcmode: FWS flow control.
 * @roamoff: Firmware roaming off?
 * @ignore_probe_fail: Ignore probe failure.
 * @trivial_ccode_map: Assume firmware uses ISO3166 country codes with rev 0
 * @country_codes: If available, pointer to struct for translating country codes
 * @bus: Bus specific platform data. Only SDIO at the mmoment.
 */
struct brcmf_mp_device {
	bool		p2p_enable;
	unsigned int	feature_disable;
	int		fcmode;
	bool		roamoff;
	bool		iapp;
	bool		ignore_probe_fail;
	bool		trivial_ccode_map;
	struct brcmfmac_pd_cc *country_codes;
	const char	*board_type;
	unsigned char	mac[ETH_ALEN];
	const char	*antenna_sku;
	const void	*cal_blob;
	int		cal_size;
	union {
		struct brcmfmac_sdio_pd sdio;
	} bus;
};

void brcmf_c_set_joinpref_default(struct brcmf_if *ifp);

struct brcmf_mp_device *brcmf_get_module_param(struct device *dev,
					       enum brcmf_bus_type bus_type,
					       u32 chip, u32 chiprev);
void brcmf_release_module_param(struct brcmf_mp_device *module_param);

/* Sets dongle media info (drv_version, mac address). */
int brcmf_c_preinit_dcmds(struct brcmf_if *ifp);
int brcmf_c_set_cur_etheraddr(struct brcmf_if *ifp, const u8 *addr);

#ifdef CONFIG_DMI
void brcmf_dmi_probe(struct brcmf_mp_device *settings, u32 chip, u32 chiprev);
#else
static inline void
brcmf_dmi_probe(struct brcmf_mp_device *settings, u32 chip, u32 chiprev) {}
#endif

#ifdef CONFIG_ACPI
void brcmf_acpi_probe(struct device *dev, enum brcmf_bus_type bus_type,
		      struct brcmf_mp_device *settings);
#else
static inline void brcmf_acpi_probe(struct device *dev,
				    enum brcmf_bus_type bus_type,
				    struct brcmf_mp_device *settings) {}
#endif

u8 brcmf_map_prio_to_prec(void *cfg, u8 prio);

u8 brcmf_map_prio_to_aci(void *cfg, u8 prio);

#endif /* BRCMFMAC_COMMON_H */
