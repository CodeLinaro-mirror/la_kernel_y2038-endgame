/* SPDX-License-Identifier: GPL-2.0-only */
/****************************************************************************
 * Driver for Solarflare network controllers and boards
 * Copyright 2007-2010 Solarflare Communications Inc.
 */

#ifndef EF4_PHY_H
#define EF4_PHY_H

/****************************************************************************
 * 10Xpress (SFX7101) PHY
 */
extern const struct ef4_phy_operations falcon_sfx7101_phy_ops;

void tenxpress_set_id_led(struct ef4_nic *efx, enum ef4_led_mode mode);

/****************************************************************************
 * AMCC/Quake QT202x PHYs
 */
extern const struct ef4_phy_operations falcon_qt202x_phy_ops;

/* These PHYs provide various H/W control states for LEDs */
#define QUAKE_LED_LINK_INVAL	(0)
#define QUAKE_LED_LINK_STAT	(1)
#define QUAKE_LED_LINK_ACT	(2)
#define QUAKE_LED_LINK_ACTSTAT	(3)
#define QUAKE_LED_OFF		(4)
#define QUAKE_LED_ON		(5)
#define QUAKE_LED_LINK_INPUT	(6)	/* Pin is an input. */
/* What link the LED tracks */
#define QUAKE_LED_TXLINK	(0)
#define QUAKE_LED_RXLINK	(8)

void falcon_qt202x_set_led(struct ef4_nic *p, int led, int state);

/****************************************************************************
* Transwitch CX4 retimer
*/
extern const struct ef4_phy_operations falcon_txc_phy_ops;

#define TXC_GPIO_DIR_INPUT	0
#define TXC_GPIO_DIR_OUTPUT	1

void falcon_txc_set_gpio_dir(struct ef4_nic *efx, int pin, int dir);
void falcon_txc_set_gpio_val(struct ef4_nic *efx, int pin, int val);

extern void
mdio45_ethtool_ksettings_get_npage(const struct mdio_if_info *mdio,
				   struct ethtool_link_ksettings *cmd,
				   u32 npage_adv, u32 npage_lpa);

/**
 * mdio45_ethtool_ksettings_get - get settings for ETHTOOL_GLINKSETTINGS
 * @mdio: MDIO interface
 * @cmd: Ethtool request structure
 *
 * Since the CSRs for auto-negotiation using next pages are not fully
 * standardised, this function does not attempt to decode them.  Use
 * mdio45_ethtool_ksettings_get_npage() to specify advertisement bits
 * from next pages.
 */
static inline void
mdio45_ethtool_ksettings_get(const struct mdio_if_info *mdio,
			     struct ethtool_link_ksettings *cmd)
{
	mdio45_ethtool_ksettings_get_npage(mdio, cmd, 0, 0);
}

#endif
