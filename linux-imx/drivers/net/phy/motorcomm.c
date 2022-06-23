// SPDX-License-Identifier: GPL-2.0+
/*
 * motorcomm.c: Motorcomm 8511/8521 PHY driver.
 *
 * Author: Frank <Frank.Sae@motor-comm.com>
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/phy.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>

// clang-format off
#define YTPHY_KERNEL_DRIVER_VERSION		"1.0.47"
#define YT8521_DRIVER_ENAME			"YT8521 Gigabit Ethernet"

#define PHY_ID_YT8511		0x0000010a
#define PHY_ID_YT8521				0x0000011A
#define YT_PHY_ID_MASK				0x00000FFF

/* Specific Function Control Register */
#define YTPHY_SPECIFIC_FUNCTION_CONTROL_REG	0x10

/* 2b00 Manual MDI configuration
 * 2b01 Manual MDIX configuration
 * 2b10 Reserved
 * 2b11 Enable automatic crossover for all modes  *default*
 */
#define YTPHY_SFCR_MDI_CROSSOVER_MODE_MASK	(BIT(6) | BIT(5))
#define YTPHY_SFCR_CROSSOVER_EN			BIT(3)
#define YTPHY_SFCR_SQE_TEST_EN			BIT(2)
#define YTPHY_SFCR_POLARITY_REVERSAL_EN		BIT(1)
#define YTPHY_SFCR_JABBER_DIS			BIT(0)

/* Specific Status Register */
#define YTPHY_SPECIFIC_STATUS_REG		0x11
#define YTPHY_SSR_SPEED_MODE_BIT		14

/* 2b00 10 Mbps
 * 2b01 100 Mbps
 * 2b10 1000 Mbps
 * 2b11 Reserved
 */
#define YTPHY_SSR_SPEED_MODE_MASK		(BIT(15) | BIT(14))
#define YTPHY_SSR_SPEED_10M			0x0
#define YTPHY_SSR_SPEED_100M			0x1
#define YTPHY_SSR_SPEED_1000M			0x2
#define YTPHY_SSR_DUPLEX_BIT			13
#define YTPHY_SSR_DUPLEX			BIT(13)
#define YTPHY_SSR_PAGE_RECEIVED			BIT(12)
#define YTPHY_SSR_SPEED_DUPLEX_RESOLVED		BIT(11)
#define YTPHY_SSR_LINK				BIT(10)
#define YTPHY_SSR_MDIX_CROSSOVER		BIT(6)
#define YTPHY_SSR_DOWNGRADE			BIT(5)
#define YTPHY_SSR_TRANSMIT_PAUSE		BIT(3)
#define YTPHY_SSR_RECEIVE_PAUSE			BIT(2)
#define YTPHY_SSR_POLARITY			BIT(1)
#define YTPHY_SSR_JABBER			BIT(0)

/* Interrupt enable Register */
#define YTPHY_INTERRUPT_ENABLE_REG		0x12
#define YTPHY_IER_AUTONEG_ERR			BIT(15)
#define YTPHY_IER_SPEED_CHANGED			BIT(14)
#define YTPHY_IER_DUPLEX_CHANGED		BIT(13)
#define YTPHY_IER_PAGE_RECEIVED			BIT(12)
#define YTPHY_IER_LINK_FAILED			BIT(11)
#define YTPHY_IER_LINK_SUCCESSED		BIT(10)
#define YTPHY_IER_WOL				BIT(6)
#define YTPHY_IER_WIRESPEED_DOWNGRADE		BIT(5)
#define YTPHY_IER_SERDES_LINK_FAILED		BIT(3)
#define YTPHY_IER_SERDES_LINK_SUCCESSED		BIT(2)
#define YTPHY_IER_POLARITY_CHANGED		BIT(1)
#define YTPHY_IER_JABBER_HAPPENED		BIT(0)

/* Interrupt Status Register */
#define YTPHY_INTERRUPT_STATUS_REG		0x13
#define YTPHY_ISR_AUTONEG_ERR			BIT(15)
#define YTPHY_ISR_SPEED_CHANGED			BIT(14)
#define YTPHY_ISR_DUPLEX_CHANGED		BIT(13)
#define YTPHY_ISR_PAGE_RECEIVED			BIT(12)
#define YTPHY_ISR_LINK_FAILED			BIT(11)
#define YTPHY_ISR_LINK_SUCCESSED		BIT(10)
#define YTPHY_ISR_WOL				BIT(6)
#define YTPHY_ISR_WIRESPEED_DOWNGRADE		BIT(5)
#define YTPHY_ISR_SERDES_LINK_FAILED		BIT(3)
#define YTPHY_ISR_SERDES_LINK_SUCCESSED		BIT(2)
#define YTPHY_ISR_POLARITY_CHANGED		BIT(1)
#define YTPHY_ISR_JABBER_HAPPENED		BIT(0)

/* Speed Auto Downgrade Control Register */
#define YTPHY_SPEED_AUTO_DOWNGRADE_CONTROL_REG	0x14
#define YTPHY_SADCR_SPEED_DOWNGRADE_EN		BIT(5)

/* If these bits are set to 3, the PHY attempts five times ( 3(set value) +
 * additional 2) before downgrading, default 0x3
 */
#define YTPHY_SADCR_SPEED_RETRY_LIMIT		(0x3 << 2)

/* Rx Error Counter Register */
#define YTPHY_RX_ERROR_COUNTER_REG		0x15

/* Extended Register's Address Offset Register */
#define YTPHY_PAGE_SELECT			0x1E

/* Extended Register's Data Register */
#define YTPHY_PAGE_DATA				0x1F

#define YT8511_PAGE_SELECT	0x1e
#define YT8511_PAGE		0x1f
#define YT8511_EXT_CLK_GATE	0x0c
#define YT8511_EXT_DELAY_DRIVE	0x0d
#define YT8511_EXT_SLEEP_CTRL	0x27

/* 2b00 25m from pll
 * 2b01 25m from xtl *default*
 * 2b10 62.m from pll
 * 2b11 125m from pll
 */
#define YT8511_CLK_125M		(BIT(2) | BIT(1))
#define YT8511_PLLON_SLP	BIT(14)

/* RX Delay enabled = 1.8ns 1000T, 8ns 10/100T */
#define YT8511_DELAY_RX		BIT(0)

/* TX Gig-E Delay is bits 7:4, default 0x5
 * TX Fast-E Delay is bits 15:12, default 0xf
 * Delay = 150ps * N - 250ps
 * On = 2000ps, off = 50ps
 */
#define YT8511_DELAY_GE_TX_EN	(0xf << 4)
#define YT8511_DELAY_GE_TX_DIS	(0x2 << 4)
#define YT8511_DELAY_FE_TX_EN	(0xf << 12)
#define YT8511_DELAY_FE_TX_DIS	(0x2 << 12)

/* Extended register is different from MMD Register and MII Register.
 * We can use ytphy_read_ext/ytphy_write_ext/ytphy_modify_ext function to
 * operate extended register.
 * Extended Register  start
 */

#define YT8521_EXTREG_SLEEP_CONTROL1_REG	0x27
#define YT8521_ESC1R_SLEEP_SW			BIT(15)
#define YT8521_ESC1R_PLLON_SLP			BIT(14)

#define YT8521_REG_SPACE_SELECT_REG		0xA000
#define YT8521_RSSR_FIBER_SPACE			BIT(1)
#define YT8521_RSSR_UTP_SPACE			0x0

#define YT8521_CHIP_CONFIG_REG			0xA001
#define YT8521_CCR_SW_RST			BIT(15)

/* 8 working modes:
 * 3b000 UTP_TO_RGMII  *default*
 * 3b001 FIBER_TO_RGMII
 * 3b010 UTP_FIBER_TO_RGMII
 * 3b011 UTP_TO_SGMII
 * 3b100 SGPHY_TO_RGMAC
 * 3b101 SGMAC_TO_RGPHY
 * 3b110 UTP_TO_FIBER_AUTO
 * 3b111 UTP_TO_FIBER_FORCE
 */
#define YT8521_CCR_MODE_SEL_MASK		(BIT(2) | BIT(1) | BIT(0))

/* 3 Phy register space pooling modes */
#define YT8521_MODE_FIBER			0x1
#define YT8521_MODE_UTP				0x2
#define YT8521_MODE_POLL			0x3

#define YT8521_RGMII_CONFIG1_REG		0xA003

/* TX Gig-E Delay is bits 3:0, default 0x1
 * TX Fast-E Delay is bits 7:4, default 0xf
 * RX Delay is bits 13:10, default 0x0
 * Delay = 150ps * N
 * On = 2250ps, off = 0ps
 */
#define YT8521_RC1R_RX_DELAY_MASK		(0xF << 10)
#define YT8521_RC1R_RX_DELAY_EN			(0xF << 10)
#define YT8521_RC1R_RX_DELAY_DIS		(0x0 << 10)
#define YT8521_RC1R_FE_TX_DELAY_MASK		(0xF << 4)
#define YT8521_RC1R_FE_TX_DELAY_EN		(0xF << 4)
#define YT8521_RC1R_FE_TX_DELAY_DIS		(0x0 << 4)
#define YT8521_RC1R_GE_TX_DELAY_MASK		(0xF << 0)
#define YT8521_RC1R_GE_TX_DELAY_EN		(0xF << 0)
#define YT8521_RC1R_GE_TX_DELAY_DIS		(0x0 << 0)

#define YTPHY_WOL_MACADDR2_REG			0xA007
#define YTPHY_WOL_MACADDR1_REG			0xA008
#define YTPHY_WOL_MACADDR0_REG			0xA009

#define YTPHY_WOL_CONFIG_REG			0xA00A
#define YTPHY_WCR_INTR_SEL			BIT(6)
#define YTPHY_WCR_ENABLE			BIT(3)

/* 2b00 84ms
 * 2b01 168ms  *default*
 * 2b10 336ms
 * 2b11 672ms
 */
#define YTPHY_WCR_PUSEL_WIDTH_MASK		(BIT(2) | BIT(1))
#define YTPHY_WCR_PUSEL_WIDTH_672MS		(BIT(2) | BIT(1))

/* 1b0 Interrupt and WOL events is level triggerd and active LOW  *default*
 * 1b1 Interrupt and WOL events is pulse triggered and active LOW
 */
#define YTPHY_WCR_TYPE_PULSE			BIT(0)

#define YTPHY_SYNC_CLK_CONFIG_REG		0xA012

/* 1b1 output 125m clock  *default*
 * 1b0 output 25m clock
 */
#define YTPHY_SCCR_Clk_125M			BIT(3)
/* Extended Register  end */

// clang-format on

/**
 * ytphy_read_ext() - read a PHY's extended register
 * @phydev: a pointer to a &struct phy_device
 * @regnum: register number to read
 *
 * returns the value of regnum reg or negative error code
 */
static int ytphy_read_ext(struct phy_device *phydev, u16 regnum)
{
	int ret;

	ret = phy_write(phydev, YTPHY_PAGE_SELECT, regnum);
	if (ret < 0)
		return ret;

	ret = phy_read(phydev, YTPHY_PAGE_DATA);
	netdev_info(phydev->attached_dev, "%s, regnum 0x%04x (ret 0x%04x)\n",	__func__, regnum, ret);

	return ret;
}

/**
 * ytphy_write_ext() - write a PHY's extended register
 * @phydev: a pointer to a &struct phy_device
 * @regnum: register number to write
 * @val: value to write to @regnum
 *
 * returns 0 or negative error code
 */
static int ytphy_write_ext(struct phy_device *phydev, u16 regnum, u16 val)
{
	int ret;
	netdev_info(phydev->attached_dev, "%s, regnum 0x%04x (val 0x%04x)\n",	__func__, regnum, val);

	ret = phy_write(phydev, YTPHY_PAGE_SELECT, regnum);
	if (ret < 0)
		return ret;
	
	ret = phy_write(phydev, YTPHY_PAGE_DATA, val);
	ytphy_read_ext(phydev, regnum);	
	return ret;
}

/**
 * ytphy_modify_ext() - bits modify a PHY's extended register
 * @phydev: a pointer to a &struct phy_device
 * @regnum: register number to write
 * @mask: bit mask of bits to clear
 * @set: bit mask of bits to set
 *
 * NOTE: Convenience function which allows a PHY‘s extended register to be
 * modified as new register value = (old register value & ~mask) | set
 *
 * returns 0 or negative error code
 */
static int ytphy_modify_ext(struct phy_device *phydev, u16 regnum, u16 mask,
			    u16 set)
{
	u16 val;
	int ret;
	netdev_info(phydev->attached_dev, "%s, regnum 0x%04x\n",	__func__, regnum);

	ret = ytphy_read_ext(phydev, regnum);
	if (ret < 0)
		return ret;

	val = ret & 0xffff;
	val &= ~mask;
	val |= set;

	ret = ytphy_write_ext(phydev, regnum, val);
	ytphy_read_ext(phydev, regnum);
	return ret;
}

/**
 * ytphy_read_page() - read reg page
 * @phydev: a pointer to a &struct phy_device
 *
 * returns current reg space of yt8521 (YT8521_RSSR_FIBER_SPACE /
 * YT8521_RSSR_UTP_SPACE) or negative errno code
 */
static int ytphy_read_page(struct phy_device *phydev)
{
	int ret;

	ret = ytphy_read_ext(phydev, YT8521_REG_SPACE_SELECT_REG);
	if (ret < 0)
		return ret;

	if (ret & YT8521_RSSR_FIBER_SPACE)
		return YT8521_RSSR_FIBER_SPACE;

	return YT8521_RSSR_UTP_SPACE;
};

/**
 * ytphy_write_page() - write reg page
 * @phydev: a pointer to a &struct phy_device
 * @page: The reg page(YT8521_RSSR_FIBER_SPACE/YT8521_RSSR_UTP_SPACE) to write.
 *
 * returns 0 or negative errno code
 */
static int ytphy_write_page(struct phy_device *phydev, int page)
{
	int mask = YT8521_RSSR_FIBER_SPACE;
	int set = YT8521_RSSR_UTP_SPACE;

	if (page == YT8521_RSSR_FIBER_SPACE)
		set = YT8521_RSSR_FIBER_SPACE;

	return ytphy_modify_ext(phydev, YT8521_REG_SPACE_SELECT_REG, mask, set);
};

/**
 * ytphy_get_wol() - report whether wake-on-lan is enabled
 * @phydev: a pointer to a &struct phy_device
 * @wol: a pointer to a &struct ethtool_wolinfo
 *
 */
static void ytphy_get_wol(struct phy_device *phydev,
			  struct ethtool_wolinfo *wol)
{
	int val = 0;

	wol->supported = WAKE_MAGIC;
	wol->wolopts = 0;

	val = ytphy_read_ext(phydev, YTPHY_WOL_CONFIG_REG);
	if (val < 0)
		return;

	if (val & YTPHY_WCR_ENABLE)
		wol->wolopts |= WAKE_MAGIC;
}

/**
 * ytphy_set_wol() - turn wake-on-lan on or off
 * @phydev: a pointer to a &struct phy_device
 * @wol: a pointer to a &struct ethtool_wolinfo
 *
 * returns 0 or negative errno code
 */
static int ytphy_set_wol(struct phy_device *phydev, struct ethtool_wolinfo *wol)
{
	int ret;
	u16 mask;
	u16 val;
	u8 i;
	int curr_reg_space;
	const u8 *mac_addr;
	struct net_device *p_attached_dev;
	static const u16 mac_addr_reg[] = {
		YTPHY_WOL_MACADDR2_REG,
		YTPHY_WOL_MACADDR1_REG,
		YTPHY_WOL_MACADDR0_REG,
	};

	curr_reg_space = ytphy_read_page(phydev);
	if (curr_reg_space < 0)
		return curr_reg_space;

	if (wol->wolopts & WAKE_MAGIC) {
		p_attached_dev = phydev->attached_dev;
		if (!p_attached_dev)
			return -ENODEV;

		mac_addr = (const u8 *)p_attached_dev->dev_addr;
		if (!is_valid_ether_addr(mac_addr))
			return -EINVAL;

		/* switch to UTP reg space */
		ret = ytphy_write_page(phydev, YT8521_RSSR_UTP_SPACE);
		if (ret < 0)
			goto err_restore_page;

		/* Store the device address for the magic packet */
		for (i = 0; i < 3; i++) {
			ret = ytphy_write_ext(phydev, mac_addr_reg[i],
					      ((mac_addr[i * 2] << 8)) |
						      (mac_addr[i * 2 + 1]));
			if (ret < 0)
				goto err_restore_page;
		}

		/* Enable the WOL feature */
		val = 0;
		mask = YTPHY_WCR_PUSEL_WIDTH_MASK | YTPHY_WCR_INTR_SEL;
		val |= YTPHY_WCR_ENABLE | YTPHY_WCR_INTR_SEL;
		val |= YTPHY_WCR_TYPE_PULSE | YTPHY_WCR_PUSEL_WIDTH_672MS;
		ret = ytphy_modify_ext(phydev, YTPHY_WOL_CONFIG_REG, mask, val);
		if (ret < 0)
			goto err_restore_page;

		/* Enable the WOL feature interrupt */
		ret = phy_set_bits(phydev, YTPHY_INTERRUPT_ENABLE_REG,
				   YTPHY_IER_WOL);
		if (ret < 0)
			goto err_restore_page;

	} else {
		/* switch to UTP reg space */
		ret = ytphy_write_page(phydev, YT8521_RSSR_UTP_SPACE);
		if (ret < 0)
			goto err_restore_page;

		/* Disable the WOL feature */
		val = 0;
		mask = YTPHY_WCR_ENABLE | YTPHY_WCR_INTR_SEL;
		ret = ytphy_modify_ext(phydev, YTPHY_WOL_CONFIG_REG, mask, val);
		if (ret < 0)
			goto err_restore_page;
	}

err_restore_page:
	/* Recover to previous register space page */
	return ytphy_write_page(phydev, curr_reg_space);
}

// clang-format off

static int yt8511_read_page(struct phy_device *phydev)
{
	return __phy_read(phydev, YT8511_PAGE_SELECT);
};

static int yt8511_write_page(struct phy_device *phydev, int page)
{
	return __phy_write(phydev, YT8511_PAGE_SELECT, page);
};

static int yt8511_config_init(struct phy_device *phydev)
{
	int oldpage, ret = 0;
	unsigned int ge, fe;

	oldpage = phy_select_page(phydev, YT8511_EXT_CLK_GATE);
	if (oldpage < 0)
		goto err_restore_page;

	/* set rgmii delay mode */
	switch (phydev->interface) {
	case PHY_INTERFACE_MODE_RGMII:
		ge = YT8511_DELAY_GE_TX_DIS;
		fe = YT8511_DELAY_FE_TX_DIS;
		break;
	case PHY_INTERFACE_MODE_RGMII_RXID:
		ge = YT8511_DELAY_RX | YT8511_DELAY_GE_TX_DIS;
		fe = YT8511_DELAY_FE_TX_DIS;
		break;
	case PHY_INTERFACE_MODE_RGMII_TXID:
		ge = YT8511_DELAY_GE_TX_EN;
		fe = YT8511_DELAY_FE_TX_EN;
		break;
	case PHY_INTERFACE_MODE_RGMII_ID:
		ge = YT8511_DELAY_RX | YT8511_DELAY_GE_TX_EN;
		fe = YT8511_DELAY_FE_TX_EN;
		break;
	default: /* do not support other modes */
		ret = -EOPNOTSUPP;
		goto err_restore_page;
	}

	ret = __phy_modify(phydev, YT8511_PAGE, (YT8511_DELAY_RX | YT8511_DELAY_GE_TX_EN), ge);
	if (ret < 0)
		goto err_restore_page;

	/* set clock mode to 125mhz */
	ret = __phy_modify(phydev, YT8511_PAGE, 0, YT8511_CLK_125M);
	if (ret < 0)
		goto err_restore_page;

	/* fast ethernet delay is in a separate page */
	ret = __phy_write(phydev, YT8511_PAGE_SELECT, YT8511_EXT_DELAY_DRIVE);
	if (ret < 0)
		goto err_restore_page;

	ret = __phy_modify(phydev, YT8511_PAGE, YT8511_DELAY_FE_TX_EN, fe);
	if (ret < 0)
		goto err_restore_page;

	/* leave pll enabled in sleep */
	ret = __phy_write(phydev, YT8511_PAGE_SELECT, YT8511_EXT_SLEEP_CTRL);
	if (ret < 0)
		goto err_restore_page;

	ret = __phy_modify(phydev, YT8511_PAGE, 0, YT8511_PLLON_SLP);
	if (ret < 0)
		goto err_restore_page;

err_restore_page:
	return phy_restore_page(phydev, oldpage, ret);
}

// clang-format on

/**
 * yt8521_get_mode() - get the mode of yt8521(pooling mode)
 * @phydev: a pointer to a &struct phy_device
 *
 * returns current mode of yt8521 (YT8521_MODE_FIBER / YT8521_MODE_UTP /
 * YT8521_MODE_POLL) or negative errno code
 */
static int yt8521_get_mode(struct phy_device *phydev)
{
	int val = 0;

	val = ytphy_read_ext(phydev, YT8521_CHIP_CONFIG_REG);
	if (val < 0)
		return val;

	switch (val & YT8521_CCR_MODE_SEL_MASK) {
	case 1:
	case 4:
	case 5:
		return YT8521_MODE_FIBER;
	case 2:
	case 6:
	case 7:
		return YT8521_MODE_POLL;
	case 3:
	case 0:
		return YT8521_MODE_UTP;
	default: /* do not support other modes */
		return -EOPNOTSUPP;
	}
}

/**
 * yt8521_adjust_status() - adjust speed and duplex according to is_utp, then
 * update speed and duplex to phydev
 *
 * @phydev: a pointer to a &struct phy_device
 * @status: yt8521 status read from YTPHY_SPECIFIC_STATUS_REG
 * @is_utp: false(yt8521 work in fiber mode) or true(yt8521 work in utp mode)
 *
 * NOTE: there is no 10M speed mode in fiber mode, so need adjust.
 *
 * returns 0
 */
static int yt8521_adjust_status(struct phy_device *phydev, int status,
				bool is_utp)
{
	int speed_mode, duplex;
	int speed = SPEED_UNKNOWN;

	if (is_utp)
		duplex = (status & YTPHY_SSR_DUPLEX) >> YTPHY_SSR_DUPLEX_BIT;
	else
		duplex = 1;

	speed_mode = (status & YTPHY_SSR_SPEED_MODE_MASK) >>
		     YTPHY_SSR_SPEED_MODE_BIT;
	switch (speed_mode) {
	case YTPHY_SSR_SPEED_10M:
		if (is_utp)
			speed = SPEED_10;
		break;
	case YTPHY_SSR_SPEED_100M:
		speed = SPEED_100;
		break;
	case YTPHY_SSR_SPEED_1000M:
		speed = SPEED_1000;
		break;
	default:
		speed = SPEED_UNKNOWN;
		break;
	}

	phydev->speed = speed;
	phydev->duplex = duplex;
	phydev->port = is_utp ? PORT_TP : PORT_FIBRE;

	return 0;
}
/**
 * yt8521_read_status() -  determines the negotiated speed and duplex
 * @phydev: a pointer to a &struct phy_device
 *
 * returns 0 or negative errno code
 */
static int yt8521_read_status(struct phy_device *phydev)
{
	int ret;
	int val;
	int yt8521_fiber_latch_val;
	int yt8521_fiber_curr_val;
	int link;
	int link_utp = 0, link_fiber = 0;
	int yt8521_mode;

	yt8521_mode = yt8521_get_mode(phydev);
	if (yt8521_mode < 0)
		return yt8521_mode;

	/* YT8521 has two reg space (utp/fiber) and three work mode (utp/fiber/poll),
	 * each reg space has YTPHY_SPECIFIC_STATUS_REG. The mode must be set
	 * before we can read the state (YTPHY_SPECIFIC_STATUS_REG). Pool mode
	 * combines utp and faber mode ,so we need do both.
	 */

	if ((yt8521_mode == YT8521_MODE_UTP) ||
	    (yt8521_mode == YT8521_MODE_POLL)) {
		ret = ytphy_write_page(phydev, YT8521_RSSR_UTP_SPACE);
		if (ret < 0)
			return ret;

		ret = genphy_read_status(phydev);
		if (ret)
			return ret;

		val = phy_read(phydev, YTPHY_SPECIFIC_STATUS_REG);
		if (val < 0)
			return val;

		link = val & YTPHY_SSR_LINK;
		if (link) {
			link_utp = 1;
			ret = yt8521_adjust_status(phydev, val, true);
			if (ret < 0)
				return ret;
		} else {
			link_utp = 0;
		}
	}

	if ((yt8521_mode == YT8521_MODE_FIBER) ||
	    (yt8521_mode == YT8521_MODE_POLL)) {
		ret = ytphy_write_page(phydev, YT8521_RSSR_FIBER_SPACE);
		if (ret < 0)
			return ret;

		ret = genphy_read_status(phydev);
		if (ret)
			return ret;

		val = phy_read(phydev, YTPHY_SPECIFIC_STATUS_REG);
		if (val < 0)
			return val;

		/* When PHY is in fiber mode, speed transferred
		 * from 1000Mbps to 100Mbps, there is not link down
		 * from 0x11, so we need check reg 1 to identify such case.
		 */
		yt8521_fiber_latch_val = phy_read(phydev, MII_BMSR);
		if (yt8521_fiber_latch_val < 0)
			return yt8521_fiber_latch_val;

		yt8521_fiber_curr_val = phy_read(phydev, MII_BMSR);
		if (yt8521_fiber_curr_val < 0)
			return yt8521_fiber_curr_val;

		link = val & YTPHY_SSR_LINK;
		if (link && yt8521_fiber_latch_val != yt8521_fiber_curr_val) {
			link = 0;

			netdev_info(
				phydev->attached_dev,
				"%s, phy addr: %d, fiber link down detect, latch = %04x, curr = %04x\n",
				__func__, phydev->mdio.addr,
				yt8521_fiber_latch_val, yt8521_fiber_curr_val);
		}
		if (link) {
			link_fiber = 1;
			ret = yt8521_adjust_status(phydev, val, false);
			if (ret < 0)
				return ret;
		} else {
			link_fiber = 0;
		}
	}

	if (link_utp || link_fiber) {
		if (phydev->link == 0)
			phydev->link = 1;
	} else {
		phydev->link = 0;
	}

	/* set utp reg space as default reg space */
	return ytphy_write_page(phydev, YT8521_RSSR_UTP_SPACE);
}

/**
 * yt8521_modify_BMCR - bits modify a PHY's BMCR register according to the
 * current mode
 * @phydev: the phy_device struct
 * @mask: bit mask of bits to clear
 * @set: bit mask of bits to set
 *
 * NOTE: Convenience function which allows a PHY‘s BMCR register to be
 * modified as new register value = (old register value & ~mask) | set.
 * YT8521 has two space (utp/fiber) and three mode (utp/fiber/poll), each space
 * has MII_BMCR. pool mode combines utp and faber,so need do both.
 *
 * returns 0 or negative errno code
 */
static int yt8521_modify_BMCR(struct phy_device *phydev, u16 mask, u16 set)
{
	int ret;
	int yt8521_mode;

	yt8521_mode = yt8521_get_mode(phydev);
	if (yt8521_mode < 0)
		return yt8521_mode;

	if ((yt8521_mode == YT8521_MODE_UTP) ||
	    (yt8521_mode == YT8521_MODE_POLL)) {
		ret = ytphy_write_page(phydev, YT8521_RSSR_UTP_SPACE);
		if (ret < 0)
			return ret;

		ret = phy_modify(phydev, MII_BMCR, mask, set);
		if (ret < 0)
			return ret;
	}

	if ((yt8521_mode == YT8521_MODE_FIBER) ||
	    (yt8521_mode == YT8521_MODE_POLL)) {
		ret = ytphy_write_page(phydev, YT8521_RSSR_FIBER_SPACE);
		if (ret < 0)
			return ret;

		ret = phy_modify(phydev, MII_BMCR, mask, set);
		if (ret < 0)
			return ret;

		/* set utp reg apace as default reg apace*/
		ret = ytphy_write_page(phydev, YT8521_RSSR_UTP_SPACE);
		if (ret < 0)
			return ret;
	}
	return 0;
}

/**
 * yt8521_soft_reset() - called to issue a PHY software reset
 * @phydev: a pointer to a &struct phy_device
 *
 * returns 0 or negative errno code
 */
int yt8521_soft_reset(struct phy_device *phydev)
{
	return yt8521_modify_BMCR(phydev, 0, BMCR_RESET);
}

/**
 * yt8521_suspend() - suspend the hardware
 * @phydev: a pointer to a &struct phy_device
 *
 * returns 0 or negative errno code
 */
int yt8521_suspend(struct phy_device *phydev)
{
	int val;
	int wol_enabled;

	val = ytphy_read_ext(phydev, YTPHY_WOL_CONFIG_REG);
	if (val < 0)
		return val;

	wol_enabled = (val & YTPHY_WCR_ENABLE);
	if (wol_enabled)
		val = BMCR_ISOLATE;
	else
		val = BMCR_PDOWN;

	return yt8521_modify_BMCR(phydev, 0, val);
}

/**
 * yt8521_resume() - resume the hardware
 * @phydev: a pointer to a &struct phy_device
 *
 * returns 0 or negative errno code
 */
int yt8521_resume(struct phy_device *phydev)
{
	int ret;
	/* disable auto sleep */
	ret = ytphy_modify_ext(phydev, YT8521_EXTREG_SLEEP_CONTROL1_REG,
			       YT8521_ESC1R_SLEEP_SW, 0);
	if (ret < 0)
		return ret;

	return yt8521_modify_BMCR(phydev, BMCR_ISOLATE | BMCR_PDOWN, 0);
}

/**
 * yt8521_config_init() - called to initialize the PHY
 * @phydev: a pointer to a &struct phy_device
 *
+ * returns 0 or negative errno code
 */
static int yt8521_config_init(struct phy_device *phydev)
{
	int ret;
	u16 val = 0;
	int hw_strap_mode;

	/* Explicitly switch to UTP reg space, just to be sure */
	ret = ytphy_write_page(phydev, YT8521_RSSR_UTP_SPACE);
	if (ret < 0)
		return ret;

	ret = phy_read(phydev, MII_PHYSID2);
	if (ret < 0)
		return ret;

	if (phydev->interface == PHY_INTERFACE_MODE_RGMII_ID ||
	    phydev->interface == PHY_INTERFACE_MODE_RGMII_RXID)
		val |= (YT8521_RC1R_RX_DELAY_EN);

	if (phydev->interface == PHY_INTERFACE_MODE_RGMII_ID ||
	    phydev->interface == PHY_INTERFACE_MODE_RGMII_TXID)
		val |= (YT8521_RC1R_GE_TX_DELAY_EN |
			YT8521_RC1R_FE_TX_DELAY_EN);

	ret = ytphy_modify_ext(phydev, YT8521_RGMII_CONFIG1_REG,
			       (YT8521_RC1R_RX_DELAY_MASK |
				YT8521_RC1R_FE_TX_DELAY_MASK |
				YT8521_RC1R_GE_TX_DELAY_MASK),
			       val);
	if (ret < 0)
		return ret;

	/* disable auto sleep */
	ret = ytphy_modify_ext(phydev, YT8521_EXTREG_SLEEP_CONTROL1_REG,
			       YT8521_ESC1R_SLEEP_SW, 0);
	if (ret < 0)
		return ret;

	/* enable RXC clock when no wire plug */
	//ret = ytphy_modify_ext(phydev, 0xc, BIT(12), 0);
	if (ret < 0)
		return ret;

	hw_strap_mode = ytphy_read_ext(phydev, YT8521_CHIP_CONFIG_REG);
	if (hw_strap_mode < 0)
		return hw_strap_mode;

	netdev_info(
		phydev->attached_dev,
		"%s done, phy addr: %d, strap mode = %ld, polling mode = %d\n",
		__func__, phydev->mdio.addr,
		hw_strap_mode & YT8521_CCR_MODE_SEL_MASK,
		yt8521_get_mode(phydev));

	return ret;
}

/**
 * yt8521_aneg_done() - determines the auto negotiation result
 * @phydev: a pointer to a &struct phy_device
 *
 * NOTE: for fiber mode, when speed is 100M, there is no definition for
 * autonegotiation, and this function handles this case and return
 * 1 per linux kernel's polling.
 *
 * returns 0(no link)or 1( fiber or utp link) or negative errno code
 */
int yt8521_aneg_done(struct phy_device *phydev)
{
	int link_fiber = 0, link_utp = 0;
	int ret;

	/* Explicitly switch to FIBER reg space*/
	ret = ytphy_write_page(phydev, YT8521_RSSR_FIBER_SPACE);
	if (ret < 0)
		return ret;

	ret = ytphy_write_page(phydev, YTPHY_SPECIFIC_STATUS_REG);
	if (ret < 0)
		return ret;

	link_fiber = !!(ret & YTPHY_SSR_LINK);

	/* Explicitly switch to UTP reg space */
	ret = ytphy_write_page(phydev, YT8521_RSSR_UTP_SPACE);
	if (ret < 0)
		return ret;

	if (!link_fiber) {
		ret = ytphy_write_page(phydev, YTPHY_SPECIFIC_STATUS_REG);
		if (ret < 0)
			return ret;

		link_fiber = !!(ret & YTPHY_SSR_LINK);
	}

	return !!(link_fiber | link_utp);
}

// clang-format off

static struct phy_driver motorcomm_phy_drvs[] = {
	{
		PHY_ID_MATCH_EXACT(PHY_ID_YT8511),
		.name		= "YT8511 Gigabit Ethernet",
		.config_init	= yt8511_config_init,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
		.read_page	= yt8511_read_page,
		.write_page	= yt8511_write_page,
	},
	{
		.phy_id		= PHY_ID_YT8521,
		.name		= YT8521_DRIVER_ENAME,
		.phy_id_mask	= YT_PHY_ID_MASK,
		.read_page	= ytphy_read_page,
		.write_page	= ytphy_write_page,
		.get_wol	= ytphy_get_wol,
		.set_wol	= ytphy_set_wol,
		.config_aneg	= genphy_config_aneg,
		.soft_reset	= yt8521_soft_reset,
		.aneg_done	= yt8521_aneg_done,
		.config_init	= yt8521_config_init,
		.read_status	= yt8521_read_status,
		.suspend	= yt8521_suspend,
		.resume		= yt8521_resume,
	},
};

module_phy_driver(motorcomm_phy_drvs);

MODULE_DESCRIPTION("Motorcomm 8511/8521 PHY driver");
MODULE_AUTHOR("Frank");
MODULE_LICENSE("GPL");
MODULE_VERSION(YTPHY_KERNEL_DRIVER_VERSION);

static const struct mdio_device_id __maybe_unused motorcomm_tbl[] = {
	{ PHY_ID_MATCH_EXACT(PHY_ID_YT8511) },
	{ PHY_ID_MATCH_EXACT(PHY_ID_YT8521) },
	{ /* sentinal */ }
};

MODULE_DEVICE_TABLE(mdio, motorcomm_tbl);
