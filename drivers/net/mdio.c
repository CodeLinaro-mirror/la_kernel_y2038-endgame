// SPDX-License-Identifier: GPL-2.0-only
/*
 * mdio.c: Generic support for MDIO-compatible transceivers
 * Copyright 2006-2009 Solarflare Communications Inc.
 */

#include <linux/kernel.h>
#include <linux/capability.h>
#include <linux/errno.h>
#include <linux/ethtool.h>
#include <linux/mdio.h>
#include <linux/module.h>

MODULE_DESCRIPTION("Generic support for MDIO-compatible transceivers");
MODULE_AUTHOR("Copyright 2006-2009 Solarflare Communications Inc.");
MODULE_LICENSE("GPL");

/**
 * mdio45_probe - probe for an MDIO (clause 45) device
 * @mdio: MDIO interface
 * @prtad: Expected PHY address
 *
 * This sets @prtad and @mmds in the MDIO interface if successful.
 * Returns 0 on success, negative on error.
 */
int mdio45_probe(struct mdio_if_info *mdio, int prtad)
{
	int mmd, stat2, devs1, devs2;

	/* Assume PHY must have at least one of PMA/PMD, WIS, PCS, PHY
	 * XS or DTE XS; give up if none is present. */
	for (mmd = 1; mmd <= 5; mmd++) {
		/* Is this MMD present? */
		stat2 = mdio->mdio_read(mdio->dev, prtad, mmd, MDIO_STAT2);
		if (stat2 < 0 ||
		    (stat2 & MDIO_STAT2_DEVPRST) != MDIO_STAT2_DEVPRST_VAL)
			continue;

		/* It should tell us about all the other MMDs */
		devs1 = mdio->mdio_read(mdio->dev, prtad, mmd, MDIO_DEVS1);
		devs2 = mdio->mdio_read(mdio->dev, prtad, mmd, MDIO_DEVS2);
		if (devs1 < 0 || devs2 < 0)
			continue;

		mdio->prtad = prtad;
		mdio->mmds = devs1 | (devs2 << 16);
		return 0;
	}

	return -ENODEV;
}
EXPORT_SYMBOL(mdio45_probe);

/**
 * mdio_mii_ioctl - MII ioctl interface for MDIO (clause 22 or 45) PHYs
 * @mdio: MDIO interface
 * @mii_data: MII ioctl data structure
 * @cmd: MII ioctl command
 *
 * Returns 0 on success, negative on error.
 */
int mdio_mii_ioctl(const struct mdio_if_info *mdio,
		   struct mii_ioctl_data *mii_data, int cmd)
{
	int prtad, devad;
	u16 addr = mii_data->reg_num;

	/* Validate/convert cmd to one of SIOC{G,S}MIIREG */
	switch (cmd) {
	case SIOCGMIIPHY:
		if (mdio->prtad == MDIO_PRTAD_NONE)
			return -EOPNOTSUPP;
		mii_data->phy_id = mdio->prtad;
		cmd = SIOCGMIIREG;
		break;
	case SIOCGMIIREG:
	case SIOCSMIIREG:
		break;
	default:
		return -EOPNOTSUPP;
	}

	/* Validate/convert phy_id */
	if ((mdio->mode_support & MDIO_SUPPORTS_C45) &&
	    mdio_phy_id_is_c45(mii_data->phy_id)) {
		prtad = mdio_phy_id_prtad(mii_data->phy_id);
		devad = mdio_phy_id_devad(mii_data->phy_id);
	} else if ((mdio->mode_support & MDIO_SUPPORTS_C22) &&
		   mii_data->phy_id < 0x20) {
		prtad = mii_data->phy_id;
		devad = MDIO_DEVAD_NONE;
		addr &= 0x1f;
	} else if ((mdio->mode_support & MDIO_EMULATE_C22) &&
		   mdio->prtad != MDIO_PRTAD_NONE &&
		   mii_data->phy_id == mdio->prtad) {
		/* Remap commonly-used MII registers. */
		prtad = mdio->prtad;
		switch (addr) {
		case MII_BMCR:
		case MII_BMSR:
		case MII_PHYSID1:
		case MII_PHYSID2:
			devad = __ffs(mdio->mmds);
			break;
		case MII_ADVERTISE:
		case MII_LPA:
			if (!(mdio->mmds & MDIO_DEVS_AN))
				return -EINVAL;
			devad = MDIO_MMD_AN;
			if (addr == MII_ADVERTISE)
				addr = MDIO_AN_ADVERTISE;
			else
				addr = MDIO_AN_LPA;
			break;
		default:
			return -EINVAL;
		}
	} else {
		return -EINVAL;
	}

	if (cmd == SIOCGMIIREG) {
		int rc = mdio->mdio_read(mdio->dev, prtad, devad, addr);
		if (rc < 0)
			return rc;
		mii_data->val_out = rc;
		return 0;
	} else {
		return mdio->mdio_write(mdio->dev, prtad, devad, addr,
					mii_data->val_in);
	}
}
EXPORT_SYMBOL(mdio_mii_ioctl);
