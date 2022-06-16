// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2018 Enrico Scholz <enrico.scholz@sigma-chemnitz.de>
 * Copyright (C) 2020 PHYTEC Messtechnik GmbH
 * Author: Enrico Scholz <enrico.scholz@sigma-chemnitz.de>
 * Author: Stefan Riedmueller <s.riedmueller@phytec.de>
 */

#include <linux/module.h>
#include <linux/errno.h>
#include <linux/i2c.h>
#include <linux/clk.h>
#include <linux/device.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio/consumer.h>
#include <linux/delay.h>

#include <linux/of_graph.h>

#include <linux/v4l2-mediabus.h>
#include <media/v4l2-async.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-fwnode.h>
#include <media/v4l2-ctrls.h>

#include "vvsensor.h"

#define AR0144_MODEL_ID					0x3000
#define AR0144_Y_ADDR_START				0x3002
#define AR0144_X_ADDR_START				0x3004
#define AR0144_Y_ADRR_END				0x3006
#define AR0144_X_ADRR_END				0x3008
#define AR0144_FRAME_LENGTH_LINES			0x300a
#define AR0144_LINE_LENGTH_PCK				0x300c
#define AR0144_REVISION					0x300e
#define	AR0144_COARSE_INT_TIME				0x3012
#define	AR0144_FINE_INT_TIME				0x3014
#define AR0144_RESET_REGISTER				0x301a
#define		BIT_GROUPED_PARAM_HOLD			BIT(15)
#define		BIT_SMIA_SER_DIS			BIT(12)
#define		BIT_FORCED_PLL_ON			BIT(11)
#define		BIT_RESTART_BAD				BIT(10)
#define		BIT_MASK_BAD				BIT(9)
#define		BIT_GPI_EN				BIT(8)
#define		BIT_PARALLEL_EN				BIT(7)
#define		BIT_DRIVE_PINS				BIT(6)
#define		BIT_LOCK_REG				BIT(3)
#define		BIT_STREAM				BIT(2)
#define		BIT_RESTART				BIT(1)
#define		BIT_RESET				BIT(0)
#define	AR0144_DATA_PEDESTAL				0x301e
#define	AR0144_VT_PIX_CLK_DIV				0x302a
#define	AR0144_VT_SYS_CLK_DIV				0x302c
#define AR0144_PRE_PLL_CLK_DIV				0x302e
#define AR0144_PLL_MUL					0x3030
#define	AR0144_OP_PIX_CLK_DIV				0x3036
#define	AR0144_OP_SYS_CLK_DIV				0x3038
#define AR0144_FRAME_COUNT				0x303a
#define AR0144_FRAME_STATUS				0x303c
#define		BIT_PLL_LOCKED				BIT(3)
#define		BIT_FRAME_START_DURING_GPH		BIT(2)
#define		BIT_STANDBY_STATUS			BIT(1)
#define		BIT_FRAMESYNC				BIT(0)
#define	AR0144_READ_MODE				0x3040
#define		BIT_VERT_FLIP				BIT(15)
#define		BIT_HORIZ_MIRROR			BIT(14)
#define		BIT_RM_COL_BIN				BIT(13)
#define		BIT_RM_ROW_BIN				BIT(12)
#define		BIT_RM_COL_SF_BIN			BIT(9)
#define		BIT_RM_COL_SF_BIN_MONO			BIT(7)
#define		BIT_RM_COL_SUM				BIT(5)
#define	AR0144_GREENR_GAIN				0x3056
#define	AR0144_BLUE_GAIN				0x3058
#define	AR0144_RED_GAIN					0x305a
#define	AR0144_GREENB_GAIN				0x305c
#define	AR0144_GLOBAL_GAIN				0x305e
#define	AR0144_ANALOG_GAIN				0x3060
#define		BIT_COARSE_GAIN(n)			((n) << 4)
#define		BIT_COARSE_GAIN_MASK			GENMASK(6, 4)
#define		BIT_FINE_GAIN(n)			(n)
#define		BIT_FINE_GAIN_MASK			0xf
#define AR0144_SMIA_TEST				0x3064
#define		BIT_EMBEDDED_DATA			BIT(8)
#define		BIT_EMBEDDED_STATS_EN			BIT(7)
#define	AR0144_DATAPATH_SEL				0x306e
#define		BIT_SLEW_RATE_DAT(n)			((n) << 13)
#define		BIT_SLEW_RATE_DAT_MASK			GENMASK(15, 13)
#define		BIT_SLEW_RATE_CLK(n)			((n) << 10)
#define		BIT_SLEW_RATE_CLK_MASK			GENMASK(12, 10)
#define	AR0144_TEST_PATTERN				0x3070
#define	AR0144_TEST_DATA_RED				0x3072
#define	AR0144_TEST_DATA_GREENR				0x3074
#define	AR0144_TEST_DATA_BLUE				0x3076
#define	AR0144_TEST_DATA_GREENB				0x3078
#define	AR0144_X_ODD_INC				0x30a2
#define	AR0144_Y_ODD_INC				0x30a6
#define	AR0144_DIGITAL_TEST				0x30b0
#define		BIT_PLL_BYPASS				BIT(14)
#define		BIT_PIXCLK_ON				BIT(8)
#define		BIT_MONOCHROME_OP			BIT(7)
#define AR0144_TEMPSENS_DATA				0x30b2
#define AR0144_TEMPSENS_CTRL				0x30b4
#define		BIT_RETRIG_THRESHOLD(n)			((n) << 6)
#define		BIT_RETRIG_THRESHOLD_MASK		GENMASK(15, 6)
#define		BIT_TEMP_CLEAR				BIT(5)
#define		BIT_TEMP_START_CONV			BIT(4)
#define		BIT_TEMPSENS_PWRON			BIT(0)
#define AR0144_TEMPSENS_CALIB1				0x30c6
#define AR0144_TEMPSENS_CALIB2				0x30c8

#define AR0144_AECTRL					0x3100
#define		BIT_MIN_ANA_GAIN(n)			((n) << 5)
#define		BIT_MIN_ANA_GAIN_MASK			GENMASK(6, 5)
#define		BIT_AUTO_DG_EN				BIT(4)
#define		BIT_AUTO_AG_EN				BIT(1)
#define		BIT_AE_EN				BIT(0)
#define	AR0144_AE_LUMA_TGT				0x3102
#define AR0144_AE_MAX_EXPOSURE				0x311c
#define	AR0144_AE_MIN_EXPOSURE				0x311e
#define	AR0144_AE_COARSE_INT_TIME			0x3164
#define	AR0144_DELTA_DK_CTRL				0x3180
#define		BIT_DK_SUB_EN				BIT(15)
#define		BIT_DK_EVERY_FRAME			BIT(14)
#define		BIT_DK_RECALC				BIT(13)
#define	AR0144_DATA_FORMAT_BITS				0x31ac
#define		BIT_DATA_FMT_IN(n)			((n) << 8)
#define		BIT_DATA_FMT_OUT(n)			(n)
#define	AR0144_SERIAL_FORMAT				0x31ae
#define		BIT_DUAL_LANE				BIT(1)
#define		BIT_SINGLE_LANE				BIT(0)
#define AR0144_MIPI_TIMING_0				0x31b4
#define		BIT_HS_PREP(n)				((n) << 12)
#define		BIT_HS_ZERO(n)				((n) << 8)
#define		BIT_HS_TRAIL(n)				((n) << 4)
#define		BIT_CLK_TRAIL(n)			(n)
#define AR0144_MIPI_TIMING_1				0x31b6
#define		BIT_CLK_PREP(n)				((n) << 12)
#define		BIT_HS_EXIT(n)				((n) << 6)
#define		BIT_CLK_ZERO(n)				(n)
#define AR0144_MIPI_TIMING_2				0x31b8
#define		BIT_BGAP(n)				((n) << 12)
#define		BIT_CLK_PRE(n)				((n) << 6)
#define		BIT_CLK_POST(n)				(n)
#define AR0144_MIPI_TIMING_3				0x31ba
#define		BIT_LPX(n)				((n) << 7)
#define		BIT_WAKE_UP(n)				(n)
#define AR0144_MIPI_TIMING_4				0x31bc
#define		BIT_CONT_TX_CLK				BIT(15)
#define		BIT_HEAVY_LP_LOAD			BIT(14)
#define		BIT_INIT(n)				(n)
#define AR0144_SER_CTRL_STAT				0x31c6
#define		BIT_FRAMER_TEST_MODE			BIT(7)
#define	AR0144_COMPANDING				0x31d0
#define		BIT_COMPAND_EN				BIT(0)
#define AR0144_SERIAL_TEST				0x31d8
#define	AR0144_PIX_DEF_ID				0x31e0
#define		BIT_PIX_DEF_1D_DDC_EN			BIT(3)
#define AR0144_CUSTOMER_REV				0x31fe

#define	AR0144_FLASH_CTRL				0x3270
#define		BIT_LED_FLASH_EN			BIT(8)
#define		BIT_LED_DELAY(n)			(n)
#define		BIT_LED_DELAY_MASK			0xff

#define AR0144_MIPI_CNTRL				0x3354

#define	AR0144_TP_NO_TESTPATTERN	0
#define	AR0144_TP_SOLIDCOLOR		1
#define	AR0144_TP_FULL_COLOR_BAR	2
#define	AR0144_TP_FADE_TO_GRAY		3
#define	AR0144_TP_WALKING_ONES		256

#define	AR0144_TEST_LANE_0		(0x1 << 8)
#define	AR0144_TEST_LANE_1		(0x2 << 8)
#define	AR0144_TEST_MODE_LP11		(0x1 << 4)

#define AR0144_MAX_MIPI_LINK		384000000ULL
#define AR0144_MAX_PARALLEL_LINK	74250000ULL

#define	AR0144_CSI2_DATA_RAW8		0x2a
#define	AR0144_CSI2_DATA_RAW10		0x2b
#define	AR0144_CSI2_DATA_RAW12		0x2c

#define AR0144_NO_SLEW_RATE		(~0u)

#define AR0144_CHIP_VERSION		0x0356
#define AR0144_DEF_WIDTH		1280
#define AR0144_DEF_HEIGHT		800

#define AR0144_FREQ_MENU_8BIT		0
#define AR0144_FREQ_MENU_10BIT		1
#define AR0144_FREQ_MENU_12BIT		2


enum {
	V4L2_CID_USER_BASE_AR0144		= V4L2_CID_USER_BASE + 0x2500,
	V4L2_CID_X_EXPOSURE_FINE,
	V4L2_CID_X_AUTO_EXPOSURE_MIN,
	V4L2_CID_X_AUTO_EXPOSURE_MAX,
	V4L2_CID_X_AUTO_EXPOSURE_TGT,
	V4L2_CID_X_AUTO_EXPOSURE_CUR,

	V4L2_CID_X_AUTOGAIN_ANALOGUE,
	V4L2_CID_X_AUTOGAIN_DIGITAL,

	V4L2_CID_X_AUTOGAIN_ANALOGUE_MIN,

	V4L2_CID_X_BINNING_ROW,
	V4L2_CID_X_BINNING_COL,
	V4L2_CID_X_COMPANDING,

	V4L2_CID_X_DIGITAL_GAIN_RED,
	V4L2_CID_X_DIGITAL_GAIN_GREENR,
	V4L2_CID_X_DIGITAL_GAIN_BLUE,
	V4L2_CID_X_DIGITAL_GAIN_GREENB,

	V4L2_CID_X_EMBEDDED_DATA,
	V4L2_CID_X_BLACK_LEVEL_AUTO,
	V4L2_CID_X_FLASH_DELAY,
	V4L2_CID_X_DYNAMIC_PIXEL_CORRECTION,
	V4L2_CID_X_TRIGGER_MODE,
};

enum {
	V4L2_X_EMBEDDED_OFF,
	V4L2_X_EMBEDDED_STAT,
	V4L2_X_EMBEDDED_DATA,
	V4L2_X_EMBEDDED_BOTH,
};

enum ar0144_model {
	AR0144_MODEL_UNKNOWN,
	AR0144_MODEL_COLOR,
	AR0144_MODEL_MONOCHROME,
};

struct ar0144_format {
	unsigned int code;
	unsigned int bpp;
};

static const struct ar0144_format ar0144_mono_formats[] = {
	{
		.code	= MEDIA_BUS_FMT_Y8_1X8,
		.bpp	= 8,
	}, {
		.code	= MEDIA_BUS_FMT_Y10_1X10,
		.bpp	= 10,
	}, {
		.code	= MEDIA_BUS_FMT_Y12_1X12,
		.bpp	= 12,
	},
};

static const struct ar0144_format ar0144_col_formats[] = {
	{
		.code	= MEDIA_BUS_FMT_SGRBG8_1X8,
		.bpp	= 8,
	}, {
		.code	= MEDIA_BUS_FMT_SGRBG10_1X10,
		.bpp	= 10,
	}, {
		.code	= MEDIA_BUS_FMT_SGRBG12_1X12,
		.bpp	= 12,
	},
};

struct limit_range {
	unsigned long min;
	unsigned long max;
};

struct ar0144_sensor_limits {
	struct limit_range x;
	struct limit_range y;
	struct limit_range hlen;
	struct limit_range vlen;
	struct limit_range hblank;
	struct limit_range vblank;
	struct limit_range ext_clk;
};

struct ar0144_businfo {
	enum v4l2_mbus_type bus_type;
	unsigned int flags;
	const s64 *link_freqs;

	unsigned int slew_rate_dat;
	unsigned int slew_rate_clk;

	unsigned int num_lanes;
	u16 t_hs_prep;
	u16 t_hs_zero;
	u16 t_hs_trail;
	u16 t_clk_trail;
	u16 t_clk_prep;
	u16 t_hs_exit;
	u16 t_clk_zero;
	u16 t_bgap;
	u16 t_clk_pre;
	u16 t_clk_post;
	u16 t_lpx;
	u16 t_wakeup;
	u16 t_init;
	bool cont_tx_clk;
	bool heavy_lp_load;
};

struct ar0144_pll_config {
	unsigned int pre_pll_div;
	unsigned int pre_pll_mul;
	unsigned int vt_sys_div;
	unsigned int vt_pix_div;
	unsigned int op_sys_div;
	unsigned int op_pix_div;
	unsigned long vco_freq;
	unsigned long pix_freq;
	unsigned long ser_freq;
};

struct ar0144_gains {
	struct v4l2_ctrl *dig_ctrl;
	struct v4l2_ctrl *ana_ctrl;
	struct v4l2_ctrl *red_ctrl;
	struct v4l2_ctrl *greenb_ctrl;
	struct v4l2_ctrl *greenr_ctrl;
	struct v4l2_ctrl *blue_ctrl;
	unsigned int red_clip;
	unsigned int greenb_clip;
	unsigned int greenr_clip;
	unsigned int blue_clip;
};

struct ar0144 {
	struct v4l2_subdev subdev;
	struct device *dev;
	struct v4l2_ctrl_handler ctrls;
	struct media_pad pad;

	struct v4l2_mbus_framefmt fmt;
	struct v4l2_rect crop;
	unsigned int bpp;
	unsigned int w_scale;
	unsigned int h_scale;
	unsigned int vblank;
	unsigned int hblank;
	unsigned int hlen;
	unsigned int vlen;
	bool embedded_data;
	bool embedded_stat;

	struct ar0144_businfo info;
	struct ar0144_pll_config pll[3];
	struct ar0144_sensor_limits limits;
	enum ar0144_model model;

	const struct ar0144_format *formats;
	unsigned int num_fmts;

	struct v4l2_ctrl *exp_ctrl;
	struct ar0144_gains gains;

	struct vvcam_mode_info_s vvcam_mode;
	unsigned int vvcam_cur_mode_index;

	struct clk *extclk;
	struct gpio_desc *reset_gpio;

	struct mutex lock;

	int power_user;
	bool is_streaming;
	bool trigger;
};

static struct vvcam_mode_info_s ar0144_modes [] = {
	{
		.index     = 0,
		.width    = 1280,
		.height   = 720,
		.hdr_mode = SENSOR_MODE_LINEAR,
		.bit_width = 12,
		.data_compress = {
			.enable = 0,
		},
		.bayer_pattern = BAYER_GRBG,
		.ae_info = {
			.def_frm_len_lines     = 742,
			.curr_frm_len_lines    = 742,
			.one_line_exp_time_ns  = 20040,

			.max_integration_line  = 65535,
			.min_integration_line  = 1,

			.max_again             = 16 * 1024,
			.min_again             = 1 * 1024,
			.max_dgain             = 15.9 * 1024,
			.min_dgain             = 1 * 1024,
			.gain_step             = 1,
			.start_exposure        = 2* 800 * 1024,
			.cur_fps               = 60 * 1024,
			.max_fps               = 60 * 1024,
			.min_fps               = 5 * 1024,
			.min_afps              = 5 * 1024,
			.int_update_delay_frm  = 1,
			.gain_update_delay_frm = 1,
		},
		.mipi_info = {
			.mipi_lane = 2,
		},
		.preg_data = NULL,
		.reg_data_count = 0,
	},
};

struct priv_ioctl {
	u32 idx;
	const char * const name;
};

struct priv_ioctl priv_ioctls[] = {
	{ VVSENSORIOC_RESET, "VVSENSORIOC_RESET" },
	{ VVSENSORIOC_S_POWER, "VVSENSORIOC_S_POWER" },
	{ VVSENSORIOC_G_POWER, "VVSENSORIOC_G_POWER" },
	{ VVSENSORIOC_S_CLK, "VVSENSORIOC_S_CLK" },
	{ VVSENSORIOC_G_CLK, "VVSENSORIOC_G_CLK" },
	{ VVSENSORIOC_QUERY, "VVSENSORIOC_QUERY" },
	{ VVSENSORIOC_S_SENSOR_MODE, "VVSENSORIOC_S_SENSOR_MODE" },
	{ VVSENSORIOC_G_SENSOR_MODE, "VVSENSORIOC_G_SENSOR_MODE" },
	{ VVSENSORIOC_READ_REG, "VVSENSORIOC_READ_REG" },
	{ VVSENSORIOC_WRITE_REG, "VVSENSORIOC_WRITE_REG" },
	{ VVSENSORIOC_READ_ARRAY, "VVSENSORIOC_READ_ARRAY" },
	{ VVSENSORIOC_WRITE_ARRAY, "VVSENSORIOC_WRITE_ARRAY" },
	{ VVSENSORIOC_G_NAME, "VVSENSORIOC_G_NAME" },
	{ VVSENSORIOC_G_RESERVE_ID, "VVSENSORIOC_G_RESERVE_ID" },
	{ VVSENSORIOC_G_CHIP_ID, "VVSENSORIOC_G_CHIP_ID" },
	{ VVSENSORIOC_S_INIT, "VVSENSORIOC_S_INIT" },
	{ VVSENSORIOC_S_STREAM, "VVSENSORIOC_S_STREAM" },
	{ VVSENSORIOC_S_LONG_EXP, "VVSENSORIOC_S_LONG_EXP" },
	{ VVSENSORIOC_S_EXP, "VVSENSORIOC_S_EXP" },
	{ VVSENSORIOC_S_VSEXP, "VVSENSORIOC_S_VSEXP" },
	{ VVSENSORIOC_S_LONG_GAIN, "VVSENSORIOC_S_LONG_GAIN" },
	{ VVSENSORIOC_S_GAIN, "VVSENSORIOC_S_GAIN" },
	{ VVSENSORIOC_S_VSGAIN, "VVSENSORIOC_S_VSGAIN" },
	{ VVSENSORIOC_S_FPS, "VVSENSORIOC_S_FPS" },
	{ VVSENSORIOC_G_FPS, "VVSENSORIOC_G_FPS" },
	{ VVSENSORIOC_S_HDR_RADIO, "VVSENSORIOC_S_HDR_RADIO" },
	{ VVSENSORIOC_S_WB, "VVSENSORIOC_S_WB" },
	{ VVSENSORIOC_S_BLC, "VVSENSORIOC_S_BLC" },
	{ VVSENSORIOC_G_EXPAND_CURVE, "VVSENSORIOC_G_EXPAND_CURVE" },
	{ VVSENSORIOC_S_TEST_PATTERN, "VVSENSORIOC_S_TEST_PATTERN" },
	{ VVSENSORIOC_MAX, "VVSENSORIOC_MAX" },
};

static inline struct ar0144 *to_ar0144(struct v4l2_subdev *sd);
static inline int bpp_to_index(unsigned int bpp);
static int ar0144_read(struct ar0144 *sensor, u16 reg, u16 *val);
static int ar0144_write(struct ar0144 *sensor, u16 reg, u16 val);
static int ar0144_s_stream(struct v4l2_subdev *sd, int enable);
static int ar0144_set_selection(struct v4l2_subdev *sd,
				struct v4l2_subdev_pad_config *cfg,
				struct v4l2_subdev_selection *sel);
static int ar0144_set_fmt(struct v4l2_subdev *sd,
			  struct v4l2_subdev_pad_config *cfg,
			  struct v4l2_subdev_format *format);


static void ar0144_vv_querycap(struct ar0144 *sensor, void *args)
{
	struct device *dev = sensor->subdev.dev;
	struct v4l2_capability *cap = (struct v4l2_capability *) args;
	struct i2c_client *i2c = v4l2_get_subdevdata(&sensor->subdev);
	const char *csi_id;
	int ret;

	dev_dbg(dev, "%s\n", __func__);

	ret = of_property_read_string(dev->of_node, "isp-bus-info", &csi_id);
	if (!ret) {
		strscpy((char *)cap->bus_info, csi_id, sizeof(cap->bus_info));
	} else {
		dev_warn(dev, "%s: No isp-bus-info found\n", __func__);
		strcpy((char *)cap->bus_info, "csi0");
	}

	strcpy((char *)cap->driver, "phycam");
	if (i2c->adapter)
		cap->bus_info[VVCAM_CAP_BUS_INFO_I2C_ADAPTER_NR_POS] =
			(__u8)i2c->adapter->nr;
	else
		cap->bus_info[VVCAM_CAP_BUS_INFO_I2C_ADAPTER_NR_POS] = 0xFF;
}

static int ar0144_vv_querymode(struct ar0144 *sensor, void *args)
{
	struct device *dev = sensor->subdev.dev;
	/* TODO: Do we need to fix this into copy_from_user? */
	struct vvcam_mode_info_array_s *array =
		(struct vvcam_mode_info_array_s *) args;
	int copy_ret;

	dev_dbg(dev, "%s\n", __func__);

	array->count = ARRAY_SIZE(ar0144_modes);

	copy_ret = copy_to_user(&array->modes, &ar0144_modes,
				sizeof(ar0144_modes));
	return copy_ret;
}

static int ar0144_vv_get_sensormode(struct ar0144 *sensor, void *args)
{
	struct device *dev = sensor->subdev.dev;
	struct vvcam_ae_info_s *ae_info = &sensor->vvcam_mode.ae_info;
	unsigned long pix_freq;
	unsigned int pixclk_mhz;
	int index;
	int ret;

	dev_dbg(dev, "%s\n", __func__);
	dev_dbg(dev, "%s index: %u\n", __func__, sensor->vvcam_cur_mode_index);

	mutex_lock(&sensor->lock);

	index = bpp_to_index(sensor->bpp);
	pix_freq = sensor->pll[index].pix_freq;
	pixclk_mhz = pix_freq / 1000000;

	ae_info->curr_frm_len_lines = sensor->vlen;
	ae_info->one_line_exp_time_ns = sensor->hlen * 1000 / pixclk_mhz;
	ae_info->cur_fps = div_u64(pix_freq * 1024ULL,
				   sensor->vlen * sensor->hlen);

	ae_info->max_integration_line = sensor->vlen;

	mutex_unlock(&sensor->lock);

	ret = copy_to_user(args, &sensor->vvcam_mode,
			   sizeof(struct vvcam_mode_info_s));
	if (ret)
		return -EIO;

	return 0;
}

static int ar0144_vv_set_sensormode(struct ar0144 *sensor, void *args)
{
	struct device *dev = sensor->subdev.dev;
	struct v4l2_subdev *sd = &sensor->subdev;
	struct v4l2_subdev_pad_config cfg;
	struct v4l2_subdev_selection sel;
	struct v4l2_subdev_format format;
	struct vvcam_mode_info_s mode;
	uint32_t index;
	int bpp;
	int ret;

	dev_dbg(dev, "%s\n", __func__);

	ret = copy_from_user(&mode, args, sizeof(struct vvcam_mode_info_s));
	index = mode.index;

	if (index > ARRAY_SIZE(ar0144_modes) - 1)
		index = 0;

	sel.which = V4L2_SUBDEV_FORMAT_ACTIVE;
	sel.pad = 0;
	sel.target = V4L2_SEL_TGT_CROP;

	sel.r.top = (AR0144_DEF_HEIGHT - ar0144_modes[index].height) / 2 + 4;
	sel.r.left = (AR0144_DEF_WIDTH - ar0144_modes[index].width) / 2 + 4;
	sel.r.width = ar0144_modes[index].width;
	sel.r.height = ar0144_modes[index].height;

	format.which = V4L2_SUBDEV_FORMAT_ACTIVE;
	format.pad = 0;

	bpp = ar0144_modes[index].bit_width;
	format.format.width = ar0144_modes[index].width;
	format.format.height = ar0144_modes[index].height;
	format.format.code = sensor->formats[bpp_to_index(bpp)].code;

	ret = ar0144_set_selection(sd, &cfg, &sel);
	if (ret)
		return ret;

	ret = ar0144_set_fmt(sd, &cfg, &format);
	if (ret)
		return ret;

	memcpy(&sensor->vvcam_mode, &ar0144_modes[index],
	       sizeof(struct vvcam_mode_info_s));
	sensor->vvcam_cur_mode_index = index;

	return 0;
}

static int ar0144_vv_s_stream(struct ar0144 *sensor, void *args)
{
	unsigned int enable = *(int *)args;

	return ar0144_s_stream(&sensor->subdev, enable);
}

static int ar0144_vv_set_exposure(struct ar0144 *sensor, void *args)
{
	struct device *dev = sensor->subdev.dev;
	unsigned int pixclk_mhz;
	uint32_t new_exp = *(uint32_t *) args;
	uint32_t int_time;
	int index;

	mutex_lock(&sensor->lock);

	index = bpp_to_index(sensor->bpp);
	pixclk_mhz = sensor->pll[index].pix_freq / 1000000;

	new_exp = new_exp / 1024;
	int_time = new_exp * pixclk_mhz / sensor->hlen;

	mutex_unlock(&sensor->lock);

	v4l2_ctrl_s_ctrl(sensor->exp_ctrl, int_time);

	dev_dbg(dev, "%s: %u --> %u\n", __func__, new_exp, int_time);

	return 0;
}

static int ar0144_vv_set_gain(struct ar0144 *sensor, void *args)
{
	struct device *dev = sensor->subdev.dev;
	uint32_t new_gain = *(uint32_t *) args;
	uint32_t d_gain, a_gain;

	new_gain = new_gain * 1000 / 1024;

	dev_dbg(dev, "%s: %u\n", __func__, new_gain);

	if (new_gain > 16000) {
		a_gain = 16000;
		d_gain = new_gain * 1000 / a_gain;
		v4l2_ctrl_s_ctrl(sensor->gains.ana_ctrl, a_gain);
		v4l2_ctrl_s_ctrl(sensor->gains.dig_ctrl, d_gain);
	} else {
		v4l2_ctrl_s_ctrl(sensor->gains.ana_ctrl, new_gain);
		v4l2_ctrl_s_ctrl(sensor->gains.dig_ctrl, 1000);
	}

	return 0;
}

static int ar0144_vv_set_wb(struct ar0144 *sensor, void *args)
{
	struct device *dev = sensor->subdev.dev;
	sensor_white_balance_t *wb = (sensor_white_balance_t *) args;
	s32 new_gain;

	new_gain = (wb->r_gain >> 8) * 1000 +
		   (wb->r_gain & 0xff) * 1000 / 256;
	v4l2_ctrl_s_ctrl(sensor->gains.red_ctrl, new_gain);
	dev_dbg(dev, "r_gain: %u --> %u\n", wb->r_gain, new_gain);

	new_gain = (wb->gr_gain >> 8) * 1000 +
		   (wb->gr_gain & 0xff) * 1000 / 256;
	v4l2_ctrl_s_ctrl(sensor->gains.greenr_ctrl, new_gain);
	dev_dbg(dev, "gr_gain: %u --> %u\n", wb->gr_gain, new_gain);

	new_gain = (wb->gb_gain >> 8) * 1000 +
		   (wb->gb_gain & 0xff) * 1000 / 256;
	v4l2_ctrl_s_ctrl(sensor->gains.greenb_ctrl, new_gain);
	dev_dbg(dev, "gb_gain: %u --> %u\n", wb->gb_gain, new_gain);

	new_gain = (wb->b_gain >> 8) * 1000 +
		   (wb->b_gain & 0xff) * 1000 / 256;
	v4l2_ctrl_s_ctrl(sensor->gains.blue_ctrl, new_gain);
	dev_dbg(dev, "b_gain: %u --> %u\n", wb->b_gain, new_gain);

	return 0;
}

static int ar0144_vv_read_reg(struct ar0144 *sensor, void *args)
{
	struct device *dev = sensor->subdev.dev;
	struct vvcam_sccb_data_s reg;
	int ret;

	dev_dbg(dev, "%s\n", __func__);

	ret = copy_from_user(&reg, args, sizeof(struct vvcam_sccb_data_s));
	if (ret)
		return ret;

	ret = ar0144_read(sensor, reg.addr, (u16 *)&reg.data);
	if (ret)
		return ret;

	ret = copy_to_user(args, &reg, sizeof(struct vvcam_sccb_data_s));
	if (ret)
		return ret;

	return 0;
}

static int ar0144_vv_write_reg(struct ar0144 *sensor, void *args)
{
	struct device *dev = sensor->subdev.dev;
	struct vvcam_sccb_data_s reg;
	int ret;

	dev_dbg(dev, "%s\n", __func__);

	ret = copy_from_user(&reg, args, sizeof(struct vvcam_sccb_data_s));
	if (ret)
		return ret;

	ret = ar0144_write(sensor, reg.addr, (u16)reg.data);
	if (ret)
		return ret;

	return 0;
}

static long ar0144_priv_ioctl(struct v4l2_subdev *sd, unsigned int cmd,
			      void *arg)
{
	struct ar0144 *sensor = to_ar0144(sd);
	int ret;
	unsigned int idx;

	if (cmd >= 0x100)
		idx = cmd - 0x100;
	else
		idx = sizeof(priv_ioctls);

	if (idx < sizeof(priv_ioctls))
		dev_dbg(sd->dev, "%s: %s\n", __func__, priv_ioctls[idx].name);
	else
		dev_dbg(sd->dev, "%s: Unknown priv ioctl: 0x%08x\n",
			__func__, cmd);

	switch(cmd) {
	case VIDIOC_QUERYCAP:
		ar0144_vv_querycap(sensor, arg);
		break;
	case VVSENSORIOC_QUERY:
		ret = ar0144_vv_querymode(sensor, arg);
		if (ret)
			return -EIO;
		break;
	case VVSENSORIOC_G_SENSOR_MODE:
		ret = ar0144_vv_get_sensormode(sensor, arg);
		if (ret)
			return ret;
		break;
	case VVSENSORIOC_S_SENSOR_MODE:
		ret = ar0144_vv_set_sensormode(sensor, arg);
		if (ret)
			return ret;
		break;
	case VVSENSORIOC_S_STREAM:
		ret = ar0144_vv_s_stream(sensor, arg);
		if (ret)
			return ret;
		break;
	case VVSENSORIOC_S_EXP:
		ret = ar0144_vv_set_exposure(sensor, arg);
		if (ret)
			return -EIO;
		break;
	case VVSENSORIOC_S_GAIN:
		ret = ar0144_vv_set_gain(sensor, arg);
		if (ret)
			return -EIO;
		break;
	case VVSENSORIOC_S_WB:
		ret = ar0144_vv_set_wb(sensor, arg);
		break;
	case VVSENSORIOC_READ_REG:
		ret = ar0144_vv_read_reg(sensor, arg);
		if (ret)
			return ret;
		break;
	case VVSENSORIOC_WRITE_REG:
		ret = ar0144_vv_write_reg(sensor, arg);
		if (ret)
			return ret;
		break;
	default:
		return -ENOTTY;
	};

	return 0;
}

static inline struct ar0144 *to_ar0144(struct v4l2_subdev *sd)
{
	return container_of(sd, struct ar0144, subdev);
}

static inline unsigned int index_to_bpp(int index)
{
	return index * 2 + 8;
}

static inline int bpp_to_index(unsigned int bpp)
{
	return (bpp - 8) / 2;
}

static int ar0144_read(struct ar0144 *sensor, u16 reg, u16 *val)
{
	struct i2c_client *i2c = v4l2_get_subdevdata(&sensor->subdev);
	unsigned char reg_buf[2], read_buf[2];
	int ret;
	u16 result;
	struct i2c_msg xfer[] = {
		[0] = {
			.addr = i2c->addr,
			.flags = 0,
			.len = 2,
			.buf = reg_buf,
		},
		[1] = {
			.addr = i2c->addr,
			.flags = I2C_M_RD,
			.len = 2,
			.buf = read_buf,
		},
	};

	reg_buf[0] = (reg >> 8) & 0xff;
	reg_buf[1] = reg & 0xff;

	ret = i2c_transfer(i2c->adapter, xfer, ARRAY_SIZE(xfer));
	if (ret >= 0 && ret != ARRAY_SIZE(xfer))
		ret = -EIO;

	if (ret < 0) {
		dev_err(&i2c->dev, "Failed to read i2c message (%d)\n", ret);
		return ret;
	}

	result = read_buf[0] << 8;
	result |= read_buf[1];

	*val = result;

	return 0;
}

static int ar0144_write(struct ar0144 *sensor, u16 reg, u16 val)
{
	struct i2c_client *i2c = v4l2_get_subdevdata(&sensor->subdev);
	unsigned char write_buf[4];
	int ret;
	struct i2c_msg xfer[] = {
		[0] = {
			.addr = i2c->addr,
			.flags = 0,
			.len = 4,
			.buf = write_buf,
		},
	};

	write_buf[0] = (reg >> 8) & 0xff;
	write_buf[1] = reg & 0xff;

	write_buf[2] = (val >> 8) & 0xff;
	write_buf[3] = val & 0xff;

	ret = i2c_transfer(i2c->adapter, xfer, ARRAY_SIZE(xfer));
	if (ret >= 0 && ret != ARRAY_SIZE(xfer))
		ret = -EIO;

	if (ret < 0) {
		dev_err(&i2c->dev, "Failed to write i2c message (%d)\n", ret);
		return ret;
	}

	dev_dbg(&i2c->dev, "Wrote i2c message 0x%02x at 0x%02x\n", val, reg);

	return 0;
}

static int ar0144_update_bits(struct ar0144 *sensor, u16 reg,
			      u16 mask, u16 val)
{
	u16 orig, tmp;
	int ret;

	ret = ar0144_read(sensor, reg, &orig);
	if (ret)
		return ret;

	tmp = orig & ~mask;
	tmp |= val & mask;

	if (tmp != orig)
		ret = ar0144_write(sensor, reg, tmp);

	return ret;
}

static int ar0144_set_bits(struct ar0144 *sensor, u16 reg, u16 val)
{
	return ar0144_update_bits(sensor, reg, val, val);
}

static int ar0144_clear_bits(struct ar0144 *sensor, u16 reg, u16 val)
{
	return ar0144_update_bits(sensor, reg, val, 0);
}

static const struct ar0144_format *ar0144_find_format(struct ar0144 *sensor,
						      u32 code)
{
	int i;

	for (i = 0; i < sensor->num_fmts; i++)
		if (sensor->formats[i].code == code)
			return &sensor->formats[i];

	return &sensor->formats[sensor->num_fmts - 1];
}

static int ar0144_start_stream(struct ar0144 *sensor)
{
	return ar0144_update_bits(sensor, AR0144_RESET_REGISTER,
				  BIT_STREAM | BIT_MASK_BAD |
				  BIT_GPI_EN | BIT_FORCED_PLL_ON,
				  BIT_STREAM | BIT_MASK_BAD);
}

static int ar0144_start_trigger(struct ar0144 *sensor)
{
	return ar0144_update_bits(sensor, AR0144_RESET_REGISTER,
				  BIT_STREAM | BIT_MASK_BAD |
				  BIT_GPI_EN | BIT_FORCED_PLL_ON,
				  BIT_MASK_BAD |
				  BIT_GPI_EN | BIT_FORCED_PLL_ON);
}

static int ar0144_enter_standby(struct ar0144 *sensor)
{
	unsigned int timeout = 1000;
	int ret;
	u16 val;

	ret = ar0144_clear_bits(sensor, AR0144_RESET_REGISTER,
				BIT_STREAM | BIT_GPI_EN | BIT_FORCED_PLL_ON);
	if (ret)
		return ret;

	while (timeout) {
		ar0144_read(sensor, AR0144_FRAME_STATUS, &val);

		if (val & BIT_STANDBY_STATUS) {
			dev_dbg(sensor->dev, "reached standby state\n");
			break;
		}

		timeout--;

		if (timeout == 0) {
			dev_warn(sensor->dev,
				  "timeout while trying to enter standby\n");
			break;
		}

		usleep_range(2000, 3000);
	}

	ar0144_read(sensor, AR0144_RESET_REGISTER, &val);

	if ((val & BIT_SMIA_SER_DIS) == 0) {
		/* TODO: Calculate frametime and use it for this wait period */
		msleep(100);
		ret = ar0144_set_bits(sensor, AR0144_RESET_REGISTER,
				      BIT_SMIA_SER_DIS);
		if (ret)
			return ret;
	}

	/* In MIPI mode the sensor might be in LP-11 test mode so make sure
	 * to disable it.
	 */
	if (sensor->info.bus_type == V4L2_MBUS_CSI2_DPHY)
		ret = ar0144_clear_bits(sensor, AR0144_SER_CTRL_STAT,
					BIT_FRAMER_TEST_MODE);

	return ret;
}

static int ar0144_mipi_enter_lp11(struct ar0144 *sensor)
{
	int ret;
	u16 val;

	val = AR0144_TEST_MODE_LP11 | AR0144_TEST_LANE_0;

	if (sensor->info.num_lanes == 2)
		val |= AR0144_TEST_LANE_1;

	ret = ar0144_write(sensor, AR0144_SERIAL_TEST, val);
	if (ret)
		return ret;

	ret = ar0144_set_bits(sensor, AR0144_SER_CTRL_STAT,
			      BIT_FRAMER_TEST_MODE);
	if (ret)
		return ret;

	ret = ar0144_update_bits(sensor, AR0144_RESET_REGISTER,
				 BIT_STREAM | BIT_SMIA_SER_DIS,
				 BIT_STREAM);
	return ret;
}

static void ar0144_reset(struct ar0144 *sensor)
{
	unsigned long ext_freq = clk_get_rate(sensor->extclk);
	unsigned long ext_freq_mhz = ext_freq / 1000000;
	unsigned long wait_usecs;

	if (sensor->reset_gpio) {
		gpiod_set_value_cansleep(sensor->reset_gpio, 1);
		usleep_range(1000, 1100);
		gpiod_set_value_cansleep(sensor->reset_gpio, 0);
	} else {
		ar0144_set_bits(sensor, AR0144_RESET_REGISTER, BIT_RESET);
	}

	wait_usecs = 160000 / ext_freq_mhz;
	usleep_range(wait_usecs, wait_usecs + 1000);
}

static int ar0144_power_on(struct ar0144 *sensor)
{
	/* TODO: Implement it as pm_runtime */
	/* TODO: Enable power, clocks, etc... */
	return 0;
}

static void ar0144_power_off(struct ar0144 *sensor)
{
	/* TODO: Disable power, clocks, etc... */
}

/* V4L2 subdev core ops */
static int ar0144_s_power(struct v4l2_subdev *sd, int on)
{
	struct ar0144 *sensor = to_ar0144(sd);
	int ret = 0;

	dev_dbg(sd->dev, "%s on: %d\n", __func__, on);

	mutex_lock(&sensor->lock);

	if (on) {
		if (sensor->power_user > 0) {
			sensor->power_user++;
			goto out;
		}

		ret = ar0144_power_on(sensor);
		if (ret)
			goto out;

		/* Enable MIPI LP-11 test mode as required by e.g. i.MX 6 */
		if (sensor->info.bus_type == V4L2_MBUS_CSI2_DPHY &&
		    !sensor->is_streaming) {
			ret = ar0144_mipi_enter_lp11(sensor);
			if (ret) {
				ar0144_power_off(sensor);
				goto out;
			}
		}

		sensor->power_user++;

	} else {
		sensor->power_user--;
		if (sensor->power_user < 0) {
			dev_err(sd->dev, "More s_power OFF than ON\n");
			ret = -EINVAL;
			goto out;
		}

		if (sensor->power_user == 0) {
			ar0144_enter_standby(sensor);
			ar0144_power_off(sensor);
		}
	}

out:
	mutex_unlock(&sensor->lock);
	return ret;
}

#ifdef CONFIG_VIDEO_ADV_DEBUG
static int ar0144_s_register(struct v4l2_subdev *sd,
			     const struct v4l2_dbg_register *reg)
{
	struct ar0144 *sensor = to_ar0144(sd);

	return ar0144_write(sensor, reg->reg, reg->val);
}

static int ar0144_g_register(struct v4l2_subdev *sd,
			     struct v4l2_dbg_register *reg)
{
	struct ar0144 *sensor = to_ar0144(sd);

	return ar0144_read(sensor, reg->reg, (u16 *)&reg->val);
}
#endif

static int ar0144_config_pll(struct ar0144 *sensor)
{
	int index = bpp_to_index(sensor->bpp);
	int ret;

	ret = ar0144_write(sensor, AR0144_VT_PIX_CLK_DIV,
			   sensor->pll[index].vt_pix_div);
	if (ret)
		return ret;

	ret = ar0144_write(sensor, AR0144_VT_SYS_CLK_DIV,
			   sensor->pll[index].vt_sys_div);
	if (ret)
		return ret;

	ret = ar0144_write(sensor, AR0144_PRE_PLL_CLK_DIV,
			   sensor->pll[index].pre_pll_div);
	if (ret)
		return ret;

	ret = ar0144_write(sensor, AR0144_PLL_MUL,
			   sensor->pll[index].pre_pll_mul);
	if (ret)
		return ret;

	ret = ar0144_write(sensor, AR0144_OP_PIX_CLK_DIV,
			   sensor->pll[index].op_pix_div);
	if (ret)
		return ret;

	ret = ar0144_write(sensor, AR0144_OP_SYS_CLK_DIV,
			   sensor->pll[index].op_sys_div);
	if (ret)
		return ret;

	/* Wait for PLL to lock */
	usleep_range(1000, 1500);

	return 0;
}

static int ar0144_config_frame(struct ar0144 *sensor)
{
	unsigned int height = sensor->fmt.height * sensor->h_scale;
	unsigned int width = sensor->fmt.width * sensor->w_scale;
	int ret;
	u16 x_end, y_end;

	ret = ar0144_write(sensor, AR0144_Y_ADDR_START, sensor->crop.top);
	if (ret)
		return ret;

	ret = ar0144_write(sensor, AR0144_X_ADDR_START, sensor->crop.left);
	if (ret)
		return ret;

	y_end = sensor->crop.top + height - 1;

	if (sensor->embedded_stat)
		y_end -= 2;
	if (sensor->embedded_data)
		y_end -= 2;

	ret = ar0144_write(sensor, AR0144_Y_ADRR_END, y_end);
	if (ret)
		return ret;

	x_end = sensor->crop.left + width - 1;
	ret = ar0144_write(sensor, AR0144_X_ADRR_END, x_end);
	if (ret)
		return ret;

	ret = ar0144_write(sensor, AR0144_FRAME_LENGTH_LINES, sensor->vlen);
	if (ret)
		return ret;

	ret = ar0144_write(sensor, AR0144_LINE_LENGTH_PCK, sensor->hlen);
	if (ret)
		return ret;

	ret = ar0144_write(sensor, AR0144_X_ODD_INC,
			   (sensor->w_scale << 1) - 1);
	if (ret)
		return ret;

	ret = ar0144_write(sensor, AR0144_Y_ODD_INC,
			   (sensor->h_scale << 1) - 1);
	if (ret)
		return ret;


	/* Enable embedded statistics for Auto Exposure to work
	 * Since it is embedded after the active frame there is no issue
	 * enabling it all the time.
	 */
	ret = ar0144_set_bits(sensor, AR0144_SMIA_TEST, BIT_EMBEDDED_STATS_EN);

	return ret;
}

static int ar0144_config_parallel(struct ar0144 *sensor)
{
	unsigned int slew_rate_dat = sensor->info.slew_rate_dat;
	unsigned int slew_rate_clk = sensor->info.slew_rate_clk;
	u16 val = 0;
	u16 mask = 0;
	int ret;

	if (slew_rate_dat != AR0144_NO_SLEW_RATE) {
		val |= BIT_SLEW_RATE_DAT(slew_rate_dat);
		mask |= BIT_SLEW_RATE_DAT_MASK;
	}

	if (slew_rate_clk != AR0144_NO_SLEW_RATE) {
		val |= BIT_SLEW_RATE_CLK(slew_rate_clk);
		mask |= BIT_SLEW_RATE_CLK_MASK;
	}

	if (mask) {
		ret = ar0144_update_bits(sensor, AR0144_DATAPATH_SEL,
					 mask, val);
		if (ret)
			return ret;
	}

	ret = ar0144_write(sensor, AR0144_DATA_FORMAT_BITS,
			   BIT_DATA_FMT_IN(sensor->bpp) |
			   BIT_DATA_FMT_OUT(sensor->bpp));
	if (ret)
		return ret;

	ret = ar0144_clear_bits(sensor, AR0144_SERIAL_FORMAT,
				BIT_DUAL_LANE | BIT_SINGLE_LANE);
	if (ret)
		return ret;

	ret = ar0144_set_bits(sensor, AR0144_RESET_REGISTER,
			      BIT_SMIA_SER_DIS | BIT_PARALLEL_EN |
			      BIT_DRIVE_PINS);
	if (ret)
		return ret;

	if (sensor->trigger)
		ret = ar0144_start_trigger(sensor);
	else
		ret = ar0144_start_stream(sensor);

	return ret;
}

static int ar0144_config_mipi(struct ar0144 *sensor)
{
	int ret;
	u16 val;

	switch (sensor->bpp) {
	case 8:
		val = AR0144_CSI2_DATA_RAW8;
		break;
	case 10:
		val = AR0144_CSI2_DATA_RAW10;
		break;
	case 12:
		val = AR0144_CSI2_DATA_RAW12;
		break;
	default:
		return -EINVAL;
	}

	ret = ar0144_write(sensor, AR0144_MIPI_CNTRL, val);
	if (ret)
		return ret;

	ret = ar0144_write(sensor, AR0144_MIPI_TIMING_0,
			   BIT_HS_PREP(sensor->info.t_hs_prep) |
			   BIT_HS_ZERO(sensor->info.t_hs_zero) |
			   BIT_HS_TRAIL(sensor->info.t_hs_trail) |
			   BIT_CLK_TRAIL(sensor->info.t_clk_trail));
	if (ret)
		return ret;

	ret = ar0144_write(sensor, AR0144_MIPI_TIMING_1,
			   BIT_CLK_PREP(sensor->info.t_clk_prep) |
			   BIT_HS_EXIT(sensor->info.t_hs_exit) |
			   BIT_CLK_ZERO(sensor->info.t_clk_zero));
	if (ret)
		return ret;

	ret = ar0144_write(sensor, AR0144_MIPI_TIMING_2,
			   BIT_BGAP(sensor->info.t_bgap) |
			   BIT_CLK_PRE(sensor->info.t_clk_pre) |
			   BIT_CLK_POST(sensor->info.t_clk_post));
	if (ret)
		return ret;

	ret = ar0144_write(sensor, AR0144_MIPI_TIMING_3,
			   BIT_LPX(sensor->info.t_lpx) |
			   BIT_WAKE_UP(sensor->info.t_wakeup));
	if (ret)
		return ret;

	val = BIT_INIT(sensor->info.t_init) |
	      (sensor->info.cont_tx_clk ? (u16) BIT_CONT_TX_CLK : 0) |
	      (sensor->info.heavy_lp_load ?
	       (u16) BIT_HEAVY_LP_LOAD : 0);

	ret = ar0144_write(sensor, AR0144_MIPI_TIMING_4, val);
	if (ret)
		return ret;

	ret = ar0144_write(sensor, AR0144_DATA_FORMAT_BITS,
			   BIT_DATA_FMT_IN(sensor->bpp) |
			   BIT_DATA_FMT_OUT(sensor->bpp));
	if (ret)
		return ret;

	if (sensor->info.num_lanes == 1)
		val = BIT_SINGLE_LANE;
	else
		val = BIT_DUAL_LANE;

	ret = ar0144_update_bits(sensor, AR0144_SERIAL_FORMAT,
				 BIT_DUAL_LANE | BIT_SINGLE_LANE, val);
	if (ret)
		return ret;

	ret = ar0144_clear_bits(sensor, AR0144_RESET_REGISTER,
				BIT_PARALLEL_EN | BIT_DRIVE_PINS);
	if (ret)
		return ret;

	if (sensor->trigger)
		ret = ar0144_start_trigger(sensor);
	else
		ret = ar0144_start_stream(sensor);
	if (ret)
		return ret;

	ret = ar0144_clear_bits(sensor, AR0144_RESET_REGISTER,
				BIT_SMIA_SER_DIS);

	return ret;
}

static int ar0144_stream_on(struct ar0144 *sensor)
{
	u16 mono_op;
	int ret;

	/* If the MIPI bus is in use the data and clk lanes are in LP-11 state.
	 * So we have to unset streaming and disable test mode before
	 * configuring the sensor.
	 */
	if (sensor->info.bus_type == V4L2_MBUS_CSI2_DPHY) {
		ret = ar0144_enter_standby(sensor);
		if (ret)
			return ret;
	}

	ret = ar0144_config_pll(sensor);
	if (ret)
		return ret;

	ret = ar0144_config_frame(sensor);
	if (ret)
		return ret;

	mono_op = sensor->model == AR0144_MODEL_MONOCHROME;

	ret = ar0144_update_bits(sensor, AR0144_DIGITAL_TEST, BIT_MONOCHROME_OP,
				 mono_op ? BIT_MONOCHROME_OP : 0);
	if (ret)
		return ret;

	if (sensor->info.bus_type == V4L2_MBUS_PARALLEL)
		ret = ar0144_config_parallel(sensor);
	else
		ret = ar0144_config_mipi(sensor);

	if (ret)
		return ret;

	sensor->is_streaming = true;
	return 0;
}

static int ar0144_stream_off(struct ar0144 *sensor)
{
	int ret;

	ret = ar0144_enter_standby(sensor);
	sensor->is_streaming = false;

	return ret;
}

/* V4L2 subdev video ops */
static int ar0144_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct ar0144 *sensor = to_ar0144(sd);
	int ret = 0;

	dev_dbg(sd->dev, "%s enable: %d\n", __func__, enable);

	mutex_lock(&sensor->lock);

	if (enable && sensor->is_streaming) {
		ret = -EBUSY;
		goto out;
	}

	if (!enable && !sensor->is_streaming)
		goto out;

	if (enable)
		ret = ar0144_stream_on(sensor);
	else
		ret = ar0144_stream_off(sensor);

out:
	mutex_unlock(&sensor->lock);
	return ret;
}

static int ar0144_g_frame_interval(struct v4l2_subdev *sd,
				   struct v4l2_subdev_frame_interval *interval)
{
	struct ar0144 *sensor = to_ar0144(sd);
	unsigned long pix_freq;
	int index;

	mutex_lock(&sensor->lock);

	index = bpp_to_index(sensor->bpp);
	pix_freq = sensor->pll[index].pix_freq;

	interval->interval.numerator = 10;
	interval->interval.denominator = div_u64(pix_freq * 1000ULL,
						 sensor->vlen * sensor->hlen);

	mutex_unlock(&sensor->lock);

	return 0;
}

static struct v4l2_rect *ar0144_get_pad_crop(struct ar0144 *sensor,
					     struct v4l2_subdev_pad_config *cfg,
					     unsigned int pad, u32 which)
{
	switch (which) {
	case V4L2_SUBDEV_FORMAT_TRY:
		return v4l2_subdev_get_try_crop(&sensor->subdev, cfg, pad);
	case V4L2_SUBDEV_FORMAT_ACTIVE:
		return &sensor->crop;
	default:
		return NULL;
	}
}

static struct v4l2_mbus_framefmt *ar0144_get_pad_fmt(struct ar0144 *sensor,
					    struct v4l2_subdev_pad_config *cfg,
					    unsigned int pad, u32 which)
{
	switch (which) {
	case V4L2_SUBDEV_FORMAT_TRY:
		return v4l2_subdev_get_try_format(&sensor->subdev, cfg, pad);
	case V4L2_SUBDEV_FORMAT_ACTIVE:
		return &sensor->fmt;
	default:
		return NULL;
	}
}

static unsigned int ar0144_find_skipfactor(unsigned int input,
					   unsigned int output)
{
	int i;

	/*
	 * We need to determine a matching supported power-of-two skip
	 * factor. If no exact match is found. the next bigger matching
	 * factor is returned.
	 * Supported factors are:
	 * No Skip
	 * Skip 2
	 * Skip 4
	 * Skip 8
	 * Skip 16
	 */

	for (i = 0; i < 4; i++)
		if ((input >> i) <= output)
			break;

	return (1 << i);
}

/* V4L2 subdev pad ops */
static int ar0144_enum_mbus_code(struct v4l2_subdev *sd,
				 struct v4l2_subdev_pad_config *cfg,
				 struct v4l2_subdev_mbus_code_enum *code)
{
	struct ar0144 *sensor = to_ar0144(sd);

	if (code->index < sensor->num_fmts) {
		code->code = sensor->formats[code->index].code;
		return 0;
	} else {
		return -EINVAL;
	}
}

static int ar0144_enum_frame_size(struct v4l2_subdev *sd,
				  struct v4l2_subdev_pad_config *cfg,
				  struct v4l2_subdev_frame_size_enum *fse)
{
	struct ar0144 *sensor = to_ar0144(sd);
	struct v4l2_mbus_framefmt *fmt;
	struct v4l2_rect *crop;
	int ret = 0;

	mutex_lock(&sensor->lock);

	fmt = ar0144_get_pad_fmt(sensor, cfg, fse->pad, fse->which);
	crop = ar0144_get_pad_crop(sensor, cfg, fse->pad, fse->which);

	if (fse->index >= 4 || fse->code != fmt->code) {
		ret = -EINVAL;
		goto out;
	}

	fse->min_width = crop->width / (1u << fse->index);
	fse->max_width = fse->min_width;
	fse->min_height = crop->height / (1u << fse->index);
	fse->max_height = fse->min_height;

	if (fse->min_width <= 1 || fse->min_height <= 1)
		ret = -EINVAL;
out:
	mutex_unlock(&sensor->lock);
	return ret;
}

static unsigned int ar0144_get_hlength(struct ar0144 *sensor)
{
	return clamp_t(unsigned int, sensor->fmt.width + sensor->hblank,
		       sensor->limits.hlen.min, sensor->limits.hlen.max);
}

static unsigned int ar0144_get_vlength(struct ar0144 *sensor)
{
	return clamp_t(unsigned int, sensor->fmt.height + sensor->vblank,
		       sensor->limits.vlen.min, sensor->limits.vlen.max);
}

static int ar0144_set_fmt(struct v4l2_subdev *sd,
			  struct v4l2_subdev_pad_config *cfg,
			  struct v4l2_subdev_format *format)
{
	struct ar0144 *sensor = to_ar0144(sd);
	const struct ar0144_format *sensor_format;
	struct v4l2_mbus_framefmt *fmt;
	struct v4l2_rect *crop;
	unsigned int width, height;
	unsigned int w_scale, h_scale;

	if (sensor->is_streaming && format->which == V4L2_SUBDEV_FORMAT_ACTIVE)
		return -EBUSY;

	mutex_lock(&sensor->lock);

	fmt = ar0144_get_pad_fmt(sensor, cfg, format->pad, format->which);
	crop = ar0144_get_pad_crop(sensor, cfg, format->pad,
				   V4L2_SUBDEV_FORMAT_ACTIVE);

	if (sensor->model == AR0144_MODEL_COLOR)
		fmt->colorspace = V4L2_COLORSPACE_RAW;
	else
		fmt->colorspace = V4L2_COLORSPACE_SRGB;

	fmt->field = V4L2_FIELD_NONE;
	fmt->xfer_func = V4L2_MAP_XFER_FUNC_DEFAULT(fmt->colorspace);
	fmt->ycbcr_enc = V4L2_MAP_YCBCR_ENC_DEFAULT(fmt->colorspace);

	sensor_format = ar0144_find_format(sensor, format->format.code);
	fmt->code = sensor_format->code;

	width = clamp_t(unsigned int, format->format.width,
			1, crop->width);
	height = clamp_t(unsigned int, format->format.height,
			 1, crop->height);

	w_scale = ar0144_find_skipfactor(crop->width, width);
	h_scale = ar0144_find_skipfactor(crop->height, height);

	fmt->width = crop->width / w_scale;
	fmt->height = crop->height / h_scale;

	if (format->which == V4L2_SUBDEV_FORMAT_ACTIVE) {
		sensor->bpp = sensor_format->bpp;
		sensor->w_scale = w_scale;
		sensor->h_scale = h_scale;
		sensor->hlen = ar0144_get_hlength(sensor);
		sensor->vlen = ar0144_get_vlength(sensor);
	}

	format->format = *fmt;

	mutex_unlock(&sensor->lock);
	return 0;
}

static int ar0144_get_fmt(struct v4l2_subdev *sd,
			  struct v4l2_subdev_pad_config *cfg,
			  struct v4l2_subdev_format *format)
{
	struct ar0144 *sensor = to_ar0144(sd);
	struct v4l2_mbus_framefmt *fmt;

	mutex_lock(&sensor->lock);

	fmt = ar0144_get_pad_fmt(sensor, cfg, format->pad, format->which);
	format->format = *fmt;

	mutex_unlock(&sensor->lock);

	return 0;
}

static int ar0144_set_selection(struct v4l2_subdev *sd,
				struct v4l2_subdev_pad_config *cfg,
				struct v4l2_subdev_selection *sel)
{
	struct ar0144 *sensor = to_ar0144(sd);
	struct v4l2_rect *_crop;
	unsigned int max_w, max_h;

	if (sel->target != V4L2_SEL_TGT_CROP)
		return -EINVAL;

	if (sensor->is_streaming &&
	    (sel->r.width != sensor->crop.width ||
	     sel->r.height != sensor->crop.height))
		return -EBUSY;

	mutex_lock(&sensor->lock);

	_crop = ar0144_get_pad_crop(sensor, cfg, sel->pad, sel->which);

	/* Check againts max, min values */
	max_w = sensor->limits.x.max - sensor->limits.x.min - 1;
	max_h = sensor->limits.y.max - sensor->limits.y.min - 1;

	_crop->top = min_t(unsigned int, ALIGN(sel->r.top, 2), max_h);
	_crop->left = min_t(unsigned int, ALIGN(sel->r.left, 2), max_w);
	_crop->width = min_t(unsigned int, sel->r.width, max_w - _crop->left);
	_crop->height = min_t(unsigned int, sel->r.height, max_h - _crop->top);

	if (sensor->is_streaming) {
		/* TODO: Add on the fly cropping change */
		mutex_unlock(&sensor->lock);
		return -EBUSY;
	}

	sel->r = *_crop;

	mutex_unlock(&sensor->lock);

	return 0;
}

static int ar0144_get_selection(struct v4l2_subdev *sd,
				struct v4l2_subdev_pad_config *cfg,
				struct v4l2_subdev_selection *sel)
{
	struct ar0144 *sensor = to_ar0144(sd);
	struct v4l2_rect *_crop;
	unsigned int x_min = sensor->limits.x.min;
	unsigned int y_min = sensor->limits.y.min;
	unsigned int x_max = sensor->limits.x.max;
	unsigned int y_max = sensor->limits.y.max;

	switch (sel->target) {
	case V4L2_SEL_TGT_CROP:
		mutex_lock(&sensor->lock);

		_crop = ar0144_get_pad_crop(sensor, cfg, sel->pad, sel->which);
		sel->r = *_crop;

		mutex_unlock(&sensor->lock);
		break;
	case V4L2_SEL_TGT_CROP_DEFAULT:
		sel->r.left = 4;
		sel->r.top = 4;
		sel->r.width = AR0144_DEF_WIDTH;
		sel->r.height = AR0144_DEF_HEIGHT;
		break;
	case V4L2_SEL_TGT_CROP_BOUNDS:
		sel->r.left = x_min;
		sel->r.top = y_min;
		sel->r.width = (x_max - x_min + 1);
		sel->r.height = (y_max - y_min + 1);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int ar0144_get_mbus_config(struct v4l2_subdev *sd, unsigned int pad,
				  struct v4l2_mbus_config *cfg)
{
	struct ar0144 *sensor = to_ar0144(sd);
	struct ar0144_businfo *info = &sensor->info;

	cfg->flags = info->flags;
	cfg->type = info->bus_type;

	return 0;
}

static const struct v4l2_subdev_core_ops ar0144_subdev_core_ops = {
	.s_power		= ar0144_s_power,
	.ioctl			= ar0144_priv_ioctl,
#ifdef CONFIG_VIDEO_ADV_DEBUG
	.s_register		= ar0144_s_register,
	.g_register		= ar0144_g_register,
#endif
};

static const struct v4l2_subdev_video_ops ar0144_subdev_video_ops = {
	.s_stream		= ar0144_s_stream,
	.g_frame_interval	= ar0144_g_frame_interval,
};

static const struct v4l2_subdev_pad_ops ar0144_subdev_pad_ops = {
	.enum_mbus_code		= ar0144_enum_mbus_code,
	.enum_frame_size	= ar0144_enum_frame_size,
	.set_fmt		= ar0144_set_fmt,
	.get_fmt		= ar0144_get_fmt,
	.set_selection		= ar0144_set_selection,
	.get_selection		= ar0144_get_selection,
	.get_mbus_config	= ar0144_get_mbus_config,
};

static const struct v4l2_subdev_ops ar0144_subdev_ops = {
	.core			= &ar0144_subdev_core_ops,
	.video			= &ar0144_subdev_video_ops,
	.pad			= &ar0144_subdev_pad_ops,
};

static const struct media_entity_operations ar0144_entity_ops = {
	.get_fwnode_pad		= v4l2_subdev_get_fwnode_pad_1_to_1,
};

static int ar0144_set_analogue_gain(struct ar0144 *sensor, unsigned int val)
{
	unsigned int coarse, fine;

	for (coarse = 0; coarse < 5; coarse++)
		if (val < (1u << (coarse + 1)) * 1000)
			break;

	val = val / (1u << coarse);
	fine = 32 * (val - 1000) / val;

	if (fine > 15)
		fine = 15;

	ar0144_update_bits(sensor, AR0144_ANALOG_GAIN,
			   BIT_COARSE_GAIN_MASK | BIT_FINE_GAIN_MASK,
			   BIT_COARSE_GAIN(coarse) | BIT_FINE_GAIN(fine));

	return 1000 * (1u << coarse) * 32 / (32 - fine);
}

unsigned int ar0144_get_min_color_gain(struct ar0144 *sensor)
{
	unsigned int gains[4];
	int min_idx = 0;
	int i;

	gains[0] = sensor->gains.red_ctrl->cur.val;
	gains[1] = sensor->gains.greenr_ctrl->cur.val;
	gains[2] = sensor->gains.greenb_ctrl->cur.val;
	gains[3] = sensor->gains.blue_ctrl->cur.val;

	for (i = 0; i < 4; i++) {
		if (gains[i] < gains[min_idx])
			min_idx = i;
	}

	return gains[min_idx];
}

static int ar0144_set_digital_gain(struct ar0144 *sensor,
				   struct v4l2_ctrl *ctrl)
{
	unsigned int coarse, fine;
	unsigned int gain, gain_min, gain_factor;
	int ret;

	coarse = ctrl->val / 1000;
	fine = (ctrl->val % 1000) * 128 / 1000;

	switch (ctrl->id) {
	case V4L2_CID_DIGITAL_GAIN:
		if (sensor->model == AR0144_MODEL_MONOCHROME) {
			ret = ar0144_write(sensor, AR0144_GLOBAL_GAIN,
					   (coarse << 7) | fine);
			return ret;
		}

		gain_min = ar0144_get_min_color_gain(sensor);
		gain_factor = (ctrl->val * 1000) / gain_min;

		gain = sensor->gains.red_ctrl->cur.val;
		gain += sensor->gains.red_clip;
		gain = (gain * gain_factor) / 1000;

		if (gain > 15999)
			sensor->gains.red_clip = gain - 15999;
		else
			sensor->gains.red_clip = 0;

		gain = clamp_t(unsigned int, gain, 1000, 15999);
		coarse = gain / 1000;
		fine = (gain % 1000) * 128 / 1000;
		ret = ar0144_write(sensor, AR0144_RED_GAIN,
				   (coarse << 7) | fine);
		if (ret)
			return ret;

		sensor->gains.red_ctrl->val = gain;
		sensor->gains.red_ctrl->cur.val = gain;

		gain = sensor->gains.greenr_ctrl->cur.val;
		gain += sensor->gains.greenr_clip;
		gain = (gain * gain_factor) / 1000;

		if (gain > 15999)
			sensor->gains.greenr_clip = gain - 15999;
		else
			sensor->gains.greenr_clip = 0;

		gain = clamp_t(unsigned int, gain, 1000, 15999);
		coarse = gain / 1000;
		fine = (gain % 1000) * 128 / 1000;
		ret = ar0144_write(sensor, AR0144_GREENR_GAIN,
				   (coarse << 7) | fine);
		if (ret)
			return ret;

		sensor->gains.greenr_ctrl->val = gain;
		sensor->gains.greenr_ctrl->cur.val = gain;

		gain = sensor->gains.greenb_ctrl->cur.val;
		gain += sensor->gains.greenb_clip;
		gain = (gain * gain_factor) / 1000;

		if (gain > 15999)
			sensor->gains.greenb_clip = gain - 15999;
		else
			sensor->gains.greenb_clip = 0;


		gain = clamp_t(unsigned int, gain, 1000, 15999);
		coarse = gain / 1000;
		fine = (gain % 1000) * 128 / 1000;
		ret = ar0144_write(sensor, AR0144_GREENB_GAIN,
				   (coarse << 7) | fine);
		if (ret)
			return ret;

		sensor->gains.greenb_ctrl->val = gain;
		sensor->gains.greenb_ctrl->cur.val = gain;

		gain = sensor->gains.blue_ctrl->cur.val;
		gain += sensor->gains.blue_clip;
		gain = (gain * gain_factor) / 1000;

		if (gain > 15999)
			sensor->gains.blue_clip = gain - 15999;
		else
			sensor->gains.blue_clip = 0;

		gain = clamp_t(unsigned int, gain, 1000, 15999);
		coarse = gain / 1000;
		fine = (gain % 1000) * 128 / 1000;
		ret = ar0144_write(sensor, AR0144_BLUE_GAIN,
				   (coarse << 7) | fine);
		if (ret)
			return ret;

		sensor->gains.blue_ctrl->val = gain;
		sensor->gains.blue_ctrl->cur.val = gain;

		break;
	case V4L2_CID_X_DIGITAL_GAIN_RED:
		ret = ar0144_write(sensor, AR0144_RED_GAIN,
				   (coarse << 7) | fine);
		if (!ret)
			sensor->gains.red_clip = 0;
		break;
	case V4L2_CID_X_DIGITAL_GAIN_GREENR:
		ret = ar0144_write(sensor, AR0144_GREENR_GAIN,
				   (coarse << 7) | fine);
		if (!ret)
			sensor->gains.greenr_clip = 0;
		break;
	case V4L2_CID_X_DIGITAL_GAIN_GREENB:
		ret = ar0144_write(sensor, AR0144_GREENB_GAIN,
				   (coarse << 7) | fine);
		if (!ret)
			sensor->gains.greenb_clip = 0;
		break;
	case V4L2_CID_X_DIGITAL_GAIN_BLUE:
		ret = ar0144_write(sensor, AR0144_BLUE_GAIN,
				   (coarse << 7) | fine);
		if (!ret)
			sensor->gains.blue_clip = 0;
		break;
	default: /* Should not end up here, ctrl id was validated by caller */
		return -EINVAL;
	}

	return ret;
}

static int ar0144_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct ar0144 *sensor = ctrl->priv;
	int ret = 0;
	u16 val;

	switch (ctrl->id) {
	case V4L2_CID_VBLANK:
		if (sensor->is_streaming)
			return -EBUSY;

		mutex_lock(&sensor->lock);
		sensor->vblank = ctrl->val;
		sensor->vlen = ar0144_get_vlength(sensor);
		mutex_unlock(&sensor->lock);
		break;
	case V4L2_CID_HBLANK:
		if (sensor->is_streaming)
			return -EBUSY;

		mutex_lock(&sensor->lock);
		sensor->hblank = ctrl->val;
		sensor->hlen = ar0144_get_hlength(sensor);
		mutex_unlock(&sensor->lock);
		break;
	case V4L2_CID_HFLIP:
		ret = ar0144_update_bits(sensor, AR0144_READ_MODE,
					 BIT_HORIZ_MIRROR,
					 ctrl->val ? BIT_HORIZ_MIRROR : 0);
		break;
	case V4L2_CID_VFLIP:
		ret = ar0144_update_bits(sensor, AR0144_READ_MODE,
					 BIT_VERT_FLIP,
					 ctrl->val ? BIT_VERT_FLIP : 0);
		break;
	case V4L2_CID_EXPOSURE:
		/* TODO: implement EXPOSURE -> 100us * conversion and combine
		 * with V4L2_CID_X_EXPOSURE_FINE
		 */
		ret = ar0144_write(sensor, AR0144_COARSE_INT_TIME, ctrl->val);
		break;
	case V4L2_CID_X_EXPOSURE_FINE:
		/* TODO: remove me; see V4L2_CID_EXPOSURE */
		ret = ar0144_write(sensor, AR0144_FINE_INT_TIME, ctrl->val);
		break;
	case V4L2_CID_TEST_PATTERN_RED:
		ret = ar0144_write(sensor, AR0144_TEST_DATA_RED, ctrl->val);
		break;
	case V4L2_CID_TEST_PATTERN_GREENR:
		ret = ar0144_write(sensor, AR0144_TEST_DATA_GREENR, ctrl->val);
		break;
	case V4L2_CID_TEST_PATTERN_BLUE:
		ret = ar0144_write(sensor, AR0144_TEST_DATA_BLUE, ctrl->val);
		break;
	case V4L2_CID_TEST_PATTERN_GREENB:
		ret = ar0144_write(sensor, AR0144_TEST_DATA_GREENB, ctrl->val);
		break;
	case V4L2_CID_EXPOSURE_AUTO:
		val = ctrl->val == V4L2_EXPOSURE_AUTO ? BIT_AE_EN : 0;
		ret = ar0144_update_bits(sensor, AR0144_AECTRL, BIT_AE_EN, val);
		break;
	case V4L2_CID_X_AUTO_EXPOSURE_TGT:
		ret = ar0144_write(sensor, AR0144_AE_LUMA_TGT, ctrl->val);
		break;
	case V4L2_CID_X_AUTO_EXPOSURE_MIN:
		ret = ar0144_write(sensor, AR0144_AE_MIN_EXPOSURE, ctrl->val);
		break;
	case V4L2_CID_X_AUTO_EXPOSURE_MAX:
		ret = ar0144_write(sensor, AR0144_AE_MAX_EXPOSURE, ctrl->val);
		break;
	case V4L2_CID_X_AUTOGAIN_ANALOGUE:
		ret = ar0144_update_bits(sensor, AR0144_AECTRL, BIT_AUTO_AG_EN,
					 ctrl->val ? BIT_AUTO_AG_EN : 0);
		break;
	case V4L2_CID_X_AUTOGAIN_DIGITAL:
		ret = ar0144_update_bits(sensor, AR0144_AECTRL, BIT_AUTO_DG_EN,
					 ctrl->val ? BIT_AUTO_DG_EN : 0);
		break;
	case V4L2_CID_X_AUTOGAIN_ANALOGUE_MIN:
		ret = ar0144_update_bits(sensor, AR0144_AECTRL,
					 BIT_MIN_ANA_GAIN_MASK,
					 BIT_MIN_ANA_GAIN(ctrl->val));
		break;
	case V4L2_CID_X_EMBEDDED_DATA:
		if (sensor->is_streaming)
			return -EBUSY;

		/*
		 * Embedded statistics are always enabled but only shown when
		 * when the corresponding ctrl is set.
		 */
		val = ctrl->val & V4L2_X_EMBEDDED_DATA ?
		       BIT_EMBEDDED_DATA : 0;
		ret = ar0144_update_bits(sensor, AR0144_SMIA_TEST,
					 BIT_EMBEDDED_DATA, val);

		if (ret)
			return ret;

		sensor->embedded_stat = ctrl->val & V4L2_X_EMBEDDED_STAT ?
					true : false;
		sensor->embedded_data = ctrl->val & V4L2_X_EMBEDDED_DATA ?
					true : false;

		break;
	case V4L2_CID_TEST_PATTERN:
		ret = ar0144_write(sensor, AR0144_TEST_PATTERN,
				   ctrl->val < 4 ? ctrl->val : 256);
		if (ret)
			return ret;

		/* This is undocumented but necessary */
		ret = ar0144_update_bits(sensor, 0x3044, 3u << 4, 0);
		break;
	case V4L2_CID_X_BINNING_COL:
		switch (ctrl->val) {
		case 0:
			val = 0;
			break;
		case 1:
			val = BIT_RM_COL_BIN;
			break;
		case 2:
			val = BIT_RM_COL_SUM;
			break;
		}
		ret = ar0144_update_bits(sensor, AR0144_READ_MODE,
					 BIT_RM_COL_BIN | BIT_RM_COL_SUM, val);
		break;
	case V4L2_CID_X_BINNING_ROW:
		ret = ar0144_update_bits(sensor, AR0144_READ_MODE,
					 BIT_RM_ROW_BIN,
					 ctrl->val > 0 ? BIT_RM_ROW_BIN : 0);
		break;
	case V4L2_CID_X_COMPANDING:
		ret = ar0144_update_bits(sensor, AR0144_COMPANDING,
					 BIT_COMPAND_EN,
					 ctrl->val ? BIT_COMPAND_EN : 0);
		break;
	case V4L2_CID_DIGITAL_GAIN:
	case V4L2_CID_X_DIGITAL_GAIN_RED:
	case V4L2_CID_X_DIGITAL_GAIN_GREENR:
	case V4L2_CID_X_DIGITAL_GAIN_BLUE:
	case V4L2_CID_X_DIGITAL_GAIN_GREENB:
		ret = ar0144_set_digital_gain(sensor, ctrl);
		break;
	case V4L2_CID_ANALOGUE_GAIN:
		ctrl->val = ar0144_set_analogue_gain(sensor, ctrl->val);
		break;
	case V4L2_CID_X_BLACK_LEVEL_AUTO:
		ret = ar0144_update_bits(sensor, AR0144_DELTA_DK_CTRL,
					 BIT_DK_SUB_EN,
					 ctrl->val ? BIT_DK_SUB_EN : 0);
		break;
	case V4L2_CID_FLASH_LED_MODE:
		val = ctrl->val ? BIT_LED_FLASH_EN : 0;
		ret = ar0144_update_bits(sensor, AR0144_FLASH_CTRL,
					 BIT_LED_FLASH_EN, val);
		break;
	case V4L2_CID_X_FLASH_DELAY:
		ret = ar0144_update_bits(sensor, AR0144_FLASH_CTRL,
					 BIT_LED_DELAY_MASK,
					 BIT_LED_DELAY(ctrl->val));
		break;
	case V4L2_CID_X_DYNAMIC_PIXEL_CORRECTION:
		val = ctrl->val ? BIT_PIX_DEF_1D_DDC_EN : 0;

		ret = ar0144_update_bits(sensor, AR0144_PIX_DEF_ID,
					 BIT_PIX_DEF_1D_DDC_EN, val);
		break;
	case V4L2_CID_X_TRIGGER_MODE:
		mutex_lock(&sensor->lock);
		sensor->trigger = ctrl->val ? true : false;

		if (!sensor->is_streaming) {
			mutex_unlock(&sensor->lock);
			break;
		}

		if (sensor->trigger)
			ret = ar0144_start_trigger(sensor);
		else
			ret = ar0144_start_stream(sensor);

		mutex_unlock(&sensor->lock);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int ar0144_g_volatile_ctrl(struct v4l2_ctrl *ctrl)
{
	struct ar0144 *sensor = ctrl->priv;
	int index;
	int ret;
	u16 val;

	mutex_lock(&sensor->lock);
	index = bpp_to_index(sensor->bpp);
	mutex_unlock(&sensor->lock);

	switch (ctrl->id) {
	case V4L2_CID_X_AUTO_EXPOSURE_CUR:
		ret = ar0144_read(sensor, AR0144_AE_COARSE_INT_TIME, &val);
		if (ret)
			return ret;

		ctrl->val = val;
		break;
	case V4L2_CID_LINK_FREQ:
		if (sensor->info.bus_type == V4L2_MBUS_PARALLEL)
			break;

		ctrl->val = index;
		break;
	case V4L2_CID_PIXEL_RATE:
		*ctrl->p_new.p_s64 = sensor->pll[index].pix_freq;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static const struct v4l2_ctrl_ops ar0144_ctrl_ops = {
	.s_ctrl			= ar0144_s_ctrl,
	.g_volatile_ctrl	= ar0144_g_volatile_ctrl,
};

static const char * const ar0144_test_pattern_menu[] = {
	"disabled",
	"solid color",
	"color bar",
	"fade to gray",
	"walking 1 (12 bit)"
};

static char const * const ar0144_embdata_menu[] = {
	"disabled",
	"stats",
	"data",
	"both",
};

static const char * const ar0144_binning_menu[] = {
	"none",
	"avg",
	"sum",
};

static const char * const ar0144_ana_gain_min_menu[] = {
	"1x",
	"2x",
	"4x",
	"8x",
};

static const struct v4l2_ctrl_config ar0144_ctrls[] = {
	{
		.ops		= &ar0144_ctrl_ops,
		.id		= V4L2_CID_VBLANK,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.min		= 22,
		.max		= 65535,
		.step		= 1,
		.def		= 22,
	}, {
		.ops		= &ar0144_ctrl_ops,
		.id		= V4L2_CID_HBLANK,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.min		= 208,
		.max		= 65535,
		.step		= 1,
		.def		= 208,
	}, {
		.ops		= &ar0144_ctrl_ops,
		.id		= V4L2_CID_HFLIP,
		.type		= V4L2_CTRL_TYPE_BOOLEAN,
	}, {
		.ops		= &ar0144_ctrl_ops,
		.id		= V4L2_CID_VFLIP,
		.type		= V4L2_CTRL_TYPE_BOOLEAN,
	}, {
		.ops		= &ar0144_ctrl_ops,
		.id		= V4L2_CID_EXPOSURE,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.min		= 0,
		.max		= 65535,
		.step		= 1,
		.def		= 800,
	}, {
		/* TODO: remove me and replace by EXPOSURE -> 100us
		 * conversion
		 */
		.ops		= &ar0144_ctrl_ops,
		.id		= V4L2_CID_X_EXPOSURE_FINE,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Exposure Fine",
		.min		= 0,
		.max		= 65535,
		.step		= 1,
		.def		= 10,
	}, {
		.ops		= &ar0144_ctrl_ops,
		.id		= V4L2_CID_TEST_PATTERN_RED,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.min		= 0,
		.max		= 4095,
		.step		= 1,
	}, {
		.ops		= &ar0144_ctrl_ops,
		.id		= V4L2_CID_TEST_PATTERN_GREENR,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.min		= 0,
		.max		= 4095,
		.step		= 1,
	}, {
		.ops		= &ar0144_ctrl_ops,
		.id		= V4L2_CID_TEST_PATTERN_GREENB,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.min		= 0,
		.max		= 4095,
		.step		= 1,
	}, {
		.ops		= &ar0144_ctrl_ops,
		.id		= V4L2_CID_TEST_PATTERN_BLUE,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.min		= 0,
		.max		= 4095,
		.step		= 1,
	}, {
		.ops		= &ar0144_ctrl_ops,
		.id		= V4L2_CID_X_AUTO_EXPOSURE_TGT,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Auto Exposure Target",
		.min		= 0,
		.max		= 65535,
		.step		= 1,
		.def		= 0x5000,
	}, {
		.ops		= &ar0144_ctrl_ops,
		.id		= V4L2_CID_X_AUTO_EXPOSURE_MIN,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Auto Exposure Min",
		.min		= 0,
		.max		= 65535,
		.step		= 1,
		.def		= 1,
	}, {
		.ops		= &ar0144_ctrl_ops,
		.id		= V4L2_CID_X_AUTO_EXPOSURE_MAX,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Auto Exposure Max",
		.min		= 0,
		.max		= 65535,
		.step		= 1,
		.def		= 800,
	}, {
		.ops		= &ar0144_ctrl_ops,
		.id		= V4L2_CID_X_AUTO_EXPOSURE_CUR,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Auto Exposure Cur",
		.flags		= (V4L2_CTRL_FLAG_READ_ONLY |
				   V4L2_CTRL_FLAG_VOLATILE),
		.min		= 0,
		.max		= 65535,
		.step		= 1,
	}, {
		.ops		= &ar0144_ctrl_ops,
		.id		= V4L2_CID_EXPOSURE_AUTO,
		.type		= V4L2_CTRL_TYPE_MENU,
		.min		= 0,
		.max		= V4L2_EXPOSURE_MANUAL,
		.menu_skip_mask	= ~(BIT(V4L2_EXPOSURE_AUTO) |
				    BIT(V4L2_EXPOSURE_MANUAL)),
		.def		= V4L2_EXPOSURE_AUTO,
	}, {
		.ops		= &ar0144_ctrl_ops,
		.id		= V4L2_CID_X_AUTOGAIN_ANALOGUE,
		.type		= V4L2_CTRL_TYPE_BOOLEAN,
		.name		= "Autogain Analogue",
		.min		= 0,
		.max		= 1,
		.step		= 1,
		.def		= 1,
	}, {
		.ops		= &ar0144_ctrl_ops,
		.id		= V4L2_CID_X_AUTOGAIN_DIGITAL,
		.type		= V4L2_CTRL_TYPE_BOOLEAN,
		.name		= "Autogain Digital",
		.min		= 0,
		.max		= 1,
		.step		= 1,
		.def		= 0,
	}, {
		.ops		= &ar0144_ctrl_ops,
		.id		= V4L2_CID_TEST_PATTERN,
		.type		= V4L2_CTRL_TYPE_MENU,
		.min		= 0,
		.max		= ARRAY_SIZE(ar0144_test_pattern_menu) - 1,
		.qmenu		= ar0144_test_pattern_menu,
	}, {
		.ops		= &ar0144_ctrl_ops,
		.id		= V4L2_CID_X_EMBEDDED_DATA,
		.type		= V4L2_CTRL_TYPE_MENU,
		.flags		= V4L2_CTRL_FLAG_MODIFY_LAYOUT,
		.name		= "Embedded Data",
		.min		= V4L2_X_EMBEDDED_OFF,
		.max		= ARRAY_SIZE(ar0144_embdata_menu) - 1,
		.def		= V4L2_X_EMBEDDED_OFF,
		.qmenu		= ar0144_embdata_menu,
	}, {
		.ops		= &ar0144_ctrl_ops,
		.id		= V4L2_CID_X_BINNING_COL,
		.type		= V4L2_CTRL_TYPE_MENU,
		.name		= "Col Binning",
		.min		= 0,
		.max		= ARRAY_SIZE(ar0144_binning_menu) - 1,
		.qmenu		= ar0144_binning_menu,
	}, {
		.ops		= &ar0144_ctrl_ops,
		.id		= V4L2_CID_X_BINNING_ROW,
		.type		= V4L2_CTRL_TYPE_MENU,
		.name		= "Row Binning",
		.min		= 0,
		/* filter out 'sum' from the menu by omitting last entry */
		.max		= ARRAY_SIZE(ar0144_binning_menu) - 2,
		.qmenu		= ar0144_binning_menu,
	}, {
		.ops		= &ar0144_ctrl_ops,
		.id		= V4L2_CID_X_COMPANDING,
		.type		= V4L2_CTRL_TYPE_BOOLEAN,
		.name		= "Compading",
		.min		= 0,
		.max		= 1,
		.step		= 1,
		.def		= 0,
	}, {
		.ops		= &ar0144_ctrl_ops,
		.id		= V4L2_CID_ANALOGUE_GAIN,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.min		= 1000,
		.step		= 1,
		.max		= 16000,
		.def		= 2000,
	}, {
		.ops		= &ar0144_ctrl_ops,
		.id		= V4L2_CID_DIGITAL_GAIN,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.min		= 1000,
		.step		= 1,
		.max		= 15999,
		.def		= 1000,
	}, {
		.ops		= &ar0144_ctrl_ops,
		.id		= V4L2_CID_X_DIGITAL_GAIN_RED,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Digital Gain Red",
		.min		= 1000,
		.step		= 1,
		.max		= 15999,
		.def		= 1300,
	}, {
		.ops		= &ar0144_ctrl_ops,
		.id		= V4L2_CID_X_DIGITAL_GAIN_GREENR,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Digital Gain Green (Red)",
		.min		= 1000,
		.step		= 1,
		.max		= 15999,
		.def		= 1000,
	}, {
		.ops		= &ar0144_ctrl_ops,
		.id		= V4L2_CID_X_DIGITAL_GAIN_GREENB,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Digital Gain Green (Blue)",
		.min		= 1000,
		.step		= 1,
		.max		= 15999,
		.def		= 1000,
	}, {
		.ops		= &ar0144_ctrl_ops,
		.id		= V4L2_CID_X_DIGITAL_GAIN_BLUE,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Digital Gain Blue",
		.min		= 1000,
		.step		= 1,
		.max		= 15999,
		.def		= 1500,
	}, {
		.ops		= &ar0144_ctrl_ops,
		.id		= V4L2_CID_X_AUTOGAIN_ANALOGUE_MIN,
		.type		= V4L2_CTRL_TYPE_MENU,
		.name		= "Analogue Gain Auto Min",
		.min		= 0,
		.max		= ARRAY_SIZE(ar0144_ana_gain_min_menu) - 1,
		.qmenu		= ar0144_ana_gain_min_menu,
	}, {
		.ops		= &ar0144_ctrl_ops,
		.id		= V4L2_CID_LINK_FREQ,
		.type		= V4L2_CTRL_TYPE_INTEGER_MENU,
		.min		= AR0144_FREQ_MENU_8BIT,
		.max		= AR0144_FREQ_MENU_12BIT,
		.def		= AR0144_FREQ_MENU_12BIT,
	}, {
		.ops		= &ar0144_ctrl_ops,
		.id		= V4L2_CID_PIXEL_RATE,
		.type		= V4L2_CTRL_TYPE_INTEGER64,
		.min		= 0,
		.max		= INT_MAX,
		.step		= 1,
	}, {
		.ops		= &ar0144_ctrl_ops,
		.id		= V4L2_CID_X_BLACK_LEVEL_AUTO,
		.type		= V4L2_CTRL_TYPE_BOOLEAN,
		.name		= "Black Level Correction",
		.min		= 0,
		.max		= 1,
		.step		= 1,
		.def		= 1,
	}, {
		.ops		= &ar0144_ctrl_ops,
		.id		= V4L2_CID_FLASH_LED_MODE,
		.type		= V4L2_CTRL_TYPE_MENU,
		.min		= 0,
		.max		= V4L2_FLASH_LED_MODE_FLASH,
		.menu_skip_mask = BIT(V4L2_FLASH_LED_MODE_TORCH),
		.def		= V4L2_FLASH_LED_MODE_NONE,
	}, {
		.ops		= &ar0144_ctrl_ops,
		.id		= V4L2_CID_X_FLASH_DELAY,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Flash Delay",
		.min		= -128,
		.step		= 1,
		.max		= 127,
		.def		= 0,
	}, {
		.ops		= &ar0144_ctrl_ops,
		.id		= V4L2_CID_X_DYNAMIC_PIXEL_CORRECTION,
		.type		= V4L2_CTRL_TYPE_BOOLEAN,
		.name		= "Dynamic Defect Pixel Correction",
		.min		= 0,
		.max		= 1,
		.step		= 1,
		.def		= 0,
	}, {
		.ops		= &ar0144_ctrl_ops,
		.id		= V4L2_CID_X_TRIGGER_MODE,
		.type		= V4L2_CTRL_TYPE_BOOLEAN,
		.name		= "Trigger Mode",
		.min		= 0,
		.max		= 1,
		.step		= 1,
		.def		= 0,
	},
};

static int ar0144_create_ctrls(struct ar0144 *sensor)
{
	struct v4l2_ctrl_config ctrl_cfg;
	struct v4l2_ctrl *ctrl;
	int i;
	int ret;

	ret = v4l2_ctrl_handler_init(&sensor->ctrls, 10);
	if (ret)
		return ret;

	sensor->subdev.ctrl_handler = &sensor->ctrls;

	for (i = 0; i < ARRAY_SIZE(ar0144_ctrls); i++) {
		ctrl_cfg = ar0144_ctrls[i];

		switch (ctrl_cfg.id) {
		case V4L2_CID_X_DIGITAL_GAIN_RED:
		case V4L2_CID_X_DIGITAL_GAIN_GREENR:
		case V4L2_CID_X_DIGITAL_GAIN_BLUE:
		case V4L2_CID_X_DIGITAL_GAIN_GREENB:
			if (sensor->model == AR0144_MODEL_MONOCHROME)
				continue;

			break;
		case V4L2_CID_X_EMBEDDED_DATA:
			if (sensor->info.bus_type == V4L2_MBUS_CSI2_DPHY)
				continue;

			break;
		case V4L2_CID_HBLANK:
			ctrl_cfg.min = sensor->limits.hblank.min;
			ctrl_cfg.def = ctrl_cfg.min;
			break;
		case V4L2_CID_VBLANK:
			ctrl_cfg.min = sensor->limits.vblank.min;
			ctrl_cfg.def = ctrl_cfg.min;
			break;
		case V4L2_CID_LINK_FREQ:
			ctrl_cfg.qmenu_int = sensor->info.link_freqs;
			break;
		default:
			break;
		}

		ctrl = v4l2_ctrl_new_custom(&sensor->ctrls,
					    &ctrl_cfg, sensor);

		ret = sensor->ctrls.error;
		if (ret) {
			dev_warn(sensor->dev,
				  "failed to register control "
				  "'%s'(0x%x): %d\n",
				  ctrl_cfg.name ? ctrl_cfg.name :
				  v4l2_ctrl_get_name(ctrl_cfg.id),
				  ctrl_cfg.id, ret);
			return ret;
		}

		switch (ctrl->id) {
		case V4L2_CID_PIXEL_RATE:
			ctrl->flags |= V4L2_CTRL_FLAG_READ_ONLY |
				       V4L2_CTRL_FLAG_VOLATILE;
			break;
		case V4L2_CID_LINK_FREQ:
			ctrl->flags |= V4L2_CTRL_FLAG_READ_ONLY |
				       V4L2_CTRL_FLAG_VOLATILE;
			break;
		case V4L2_CID_EXPOSURE:
			sensor->exp_ctrl = ctrl;
			break;
		case V4L2_CID_ANALOGUE_GAIN:
			sensor->gains.ana_ctrl = ctrl;
			break;
		case V4L2_CID_DIGITAL_GAIN:
			ctrl->flags |= V4L2_CTRL_FLAG_EXECUTE_ON_WRITE |
				       V4L2_CTRL_FLAG_UPDATE;
			sensor->gains.dig_ctrl = ctrl;
			break;
		case V4L2_CID_X_DIGITAL_GAIN_RED:
			if (sensor->model == AR0144_MODEL_COLOR)
				sensor->gains.red_ctrl = ctrl;
			break;
		case V4L2_CID_X_DIGITAL_GAIN_GREENB:
			if (sensor->model == AR0144_MODEL_COLOR)
				sensor->gains.greenb_ctrl = ctrl;
			break;
		case V4L2_CID_X_DIGITAL_GAIN_GREENR:
			if (sensor->model == AR0144_MODEL_COLOR)
				sensor->gains.greenr_ctrl = ctrl;
			break;
		case V4L2_CID_X_DIGITAL_GAIN_BLUE:
			if (sensor->model == AR0144_MODEL_COLOR)
				sensor->gains.blue_ctrl = ctrl;
			break;
		default:
			break;
		}
	}

	return 0;
}

static void ar0144_set_defaults(struct ar0144 *sensor)
{
	sensor->limits = (struct ar0144_sensor_limits) {
					/* min		max	 */
		.x			= {0,		1295      },
		.y			= {0,		807       },
		.hlen			= {1488,	65534     },
		.vlen			= {29,		65535     },
		.hblank			= {208,		65535     },
		.vblank			= {22,		65535     },
		.ext_clk		= {6000000,	48000000  },
	};

	sensor->crop.left = 4;
	sensor->crop.top = 4;
	sensor->crop.width = AR0144_DEF_WIDTH;
	sensor->crop.height = AR0144_DEF_HEIGHT;

	sensor->fmt.width = AR0144_DEF_WIDTH;
	sensor->fmt.height = AR0144_DEF_HEIGHT;
	sensor->fmt.field = V4L2_FIELD_NONE;
	sensor->fmt.colorspace = V4L2_COLORSPACE_SRGB;

	if (sensor->model == AR0144_MODEL_MONOCHROME) {
		sensor->formats = ar0144_mono_formats;
		sensor->num_fmts = ARRAY_SIZE(ar0144_mono_formats);
	} else {
		sensor->formats = ar0144_col_formats;
		sensor->num_fmts = ARRAY_SIZE(ar0144_col_formats);
	}

	sensor->fmt.code = sensor->formats[sensor->num_fmts - 1].code;
	sensor->bpp = sensor->formats[sensor->num_fmts - 1].bpp;

	sensor->w_scale = 1;
	sensor->h_scale = 1;
	sensor->hblank = sensor->limits.hblank.min;
	sensor->vblank = sensor->limits.vblank.min;
	sensor->hlen = sensor->limits.hlen.min;
	sensor->vlen = sensor->fmt.height + sensor->vblank;
}

static int ar0144_subdev_registered(struct v4l2_subdev *sd)
{
	struct ar0144 *sensor = to_ar0144(sd);
	int ret;

	ar0144_set_defaults(sensor);

	ret = ar0144_create_ctrls(sensor);
	if (ret)
		return ret;

	v4l2_ctrl_handler_setup(&sensor->ctrls);

	return 0;
}

static const struct v4l2_subdev_internal_ops ar0144_subdev_internal_ops = {
	.registered		= ar0144_subdev_registered,
};

static int ar0144_check_chip_id(struct ar0144 *sensor)
{
	struct device *dev = sensor->dev;
	u16 model_id, customer_rev;
	int ret;

	ret = ar0144_power_on(sensor);
	if (ret) {
		dev_err(dev, "Failed to power on sensor (%d)\n", ret);
		return ret;
	}

	ar0144_reset(sensor);

	ret = ar0144_read(sensor, AR0144_MODEL_ID, &model_id);
	if (ret) {
		dev_err(dev, "Failed to read model ID (%d)\n", ret);
		goto out;
	}

	if (model_id != AR0144_CHIP_VERSION) {
		dev_err(dev, "Wrong chip version: 0x%04x <-> 0x%04x\n",
			model_id, AR0144_CHIP_VERSION);
		ret = -ENOENT;
		goto out;
	}

	ret = ar0144_read(sensor, AR0144_CUSTOMER_REV, &customer_rev);
	if (ret)
		goto out;

	dev_info(dev, "Device ID: 0x%04x customer rev: 0x%04x\n",
		 model_id, customer_rev);

	if (sensor->model == AR0144_MODEL_UNKNOWN) {
		if (customer_rev & BIT(4))
			sensor->model = AR0144_MODEL_COLOR;
		else
			sensor->model = AR0144_MODEL_MONOCHROME;
	}

out:
	ar0144_power_off(sensor);
	return ret;
}

static unsigned long ar0144_clk_mul_div(unsigned long freq,
					unsigned int mul,
					unsigned int div)
{
	uint64_t result;

	if (WARN_ON(div == 0))
		return 0;

	result = freq;
	result *= mul;
	result = div_u64(result, div);

	return result;
}

static int ar0144_calculate_pll(struct ar0144 *sensor, struct device *dev,
				struct ar0144_pll_config *pll,
				unsigned long ext_freq,
				u64 link_freq,
				unsigned int bpp)
{
	unsigned long op_clk;
	unsigned long vco;
	unsigned long pix_clk;
	unsigned long pix_target;
	unsigned long diff, diff_old;
	unsigned int lanes = sensor->info.num_lanes;
	unsigned int div, mul, op_div;
	const struct limit_range div_lim = {.min = 1, .max = 63};
	const struct limit_range mul_lim = {.min = 32, .max = 254};
	const struct limit_range op_div_lim = {.min = 1, .max = 16};
	const struct limit_range pix_lim = {.min = 0, .max = 74250000};
	const struct limit_range vco_lim = {.min = 384000000, .max = 768000000};

	if (sensor->info.bus_type == V4L2_MBUS_PARALLEL)
		pix_target = link_freq;
	else
		pix_target = ar0144_clk_mul_div(link_freq, 2 * lanes, bpp);

	diff_old = pix_target;

	pll->pre_pll_div = 1;
	pll->pre_pll_mul = 0;
	pll->op_sys_div = 1;
	pll->op_pix_div = bpp;
	pll->vt_sys_div = 1;
	pll->vt_pix_div = bpp / lanes;

	div = div_lim.min;
	mul = mul_lim.min;
	op_div = op_div_lim.min;

	while (div <= div_lim.max) {
		if (mul % 2 != 0)
			mul++;

		if (mul > mul_lim.max) {
			mul = mul_lim.min;
			op_div = op_div == 1 ? 2 : op_div + 2;
		}

		if (op_div > op_div_lim.max) {
			op_div = op_div_lim.min;
			div++;
			if (div > div_lim.max)
				break;
		}

		vco = ar0144_clk_mul_div(ext_freq, mul, div);

		if (vco < vco_lim.min || vco > vco_lim.max) {
			mul++;
			continue;
		}

		op_clk = ar0144_clk_mul_div(vco, 1, bpp * op_div);
		pix_clk = op_clk * lanes;

		if (pix_clk > pix_lim.max) {
			mul++;
			continue;
		}

		if (pix_clk > pix_target) {
			mul++;
			continue;
		}

		diff = pix_target - pix_clk;
		if (diff >= diff_old) {
			mul++;
			continue;
		}

		dev_dbg(dev, "%s: vco: %lu pix_clk: %lu\n",
			__func__, vco, pix_clk);
		dev_dbg(dev, "%s op_div: %d pll_div: %d pll_mul: %d\n",
			__func__, op_div, div, mul);

		diff_old = diff;

		pll->pre_pll_div = div;
		pll->pre_pll_mul = mul;
		pll->op_sys_div = op_div;
		pll->vt_sys_div = op_div;
		pll->vco_freq = vco;
		pll->pix_freq = pix_clk;

		if (sensor->info.bus_type == V4L2_MBUS_PARALLEL)
			pll->ser_freq = pix_clk;
		else
			pll->ser_freq = ar0144_clk_mul_div(pix_clk, bpp,
							   2 * lanes);

		mul++;
	}

	if (pll->pre_pll_mul == 0) {
		dev_err(dev, "Unable to find matching pll config\n");
		return -EINVAL;
	}

	dev_dbg(dev, "PLL: bpp: %u VCO: %lu, PIX: %lu, SER: %lu\n",
		bpp, pll->vco_freq, pll->pix_freq, pll->ser_freq);

	return 0;
}

static int ar0144_parse_parallel_props(struct ar0144 *sensor,
				       struct fwnode_handle *ep,
				       struct v4l2_fwnode_endpoint *bus_cfg)
{
	unsigned int tmp;

	sensor->info.flags = bus_cfg->bus.parallel.flags;
	/* Required for PLL calculation */
	sensor->info.num_lanes = 1;

	tmp = AR0144_NO_SLEW_RATE;
	fwnode_property_read_u32(ep, "onsemi,slew-rate-dat", &tmp);
	sensor->info.slew_rate_dat = clamp_t(unsigned int, tmp, 0, 0x7);

	tmp = AR0144_NO_SLEW_RATE;
	fwnode_property_read_u32(ep, "onsemi,slew-rate-clk", &tmp);
	sensor->info.slew_rate_clk = clamp_t(unsigned int, tmp, 0, 0x7);

	return 0;
}

static int ar0144_parse_mipi_props(struct ar0144 *sensor,
				   struct fwnode_handle *ep,
				   struct v4l2_fwnode_endpoint *bus_cfg)
{
	unsigned int tmp;

	sensor->info.num_lanes = bus_cfg->bus.mipi_csi2.num_data_lanes;
	if (sensor->info.num_lanes < 1 || sensor->info.num_lanes > 2) {
		dev_err(sensor->dev, "Wrong number of lanes configured");
		return -EINVAL;
	}

	sensor->info.flags = bus_cfg->bus.mipi_csi2.flags;
	sensor->info.flags |= V4L2_MBUS_CSI2_CHANNEL_0;
	if (sensor->info.num_lanes == 1)
		sensor->info.flags |= V4L2_MBUS_CSI2_1_LANE;
	else
		sensor->info.flags |= V4L2_MBUS_CSI2_2_LANE;

	tmp = 2;
	fwnode_property_read_u32(ep, "onsemi,t-hs-prep", &tmp);
	sensor->info.t_hs_prep = clamp_t(unsigned int, tmp, 0, 0xf);

	tmp = 6;
	fwnode_property_read_u32(ep, "onsemi,t-hs-zero", &tmp);
	sensor->info.t_hs_zero = clamp_t(unsigned int, tmp, 0, 0xf);

	tmp = 6;
	fwnode_property_read_u32(ep, "onsemi,t-hs-trail", &tmp);
	sensor->info.t_hs_trail = clamp_t(unsigned int, tmp, 0, 0xf);

	tmp = 5;
	fwnode_property_read_u32(ep, "onsemi,t-clk-trail", &tmp);
	sensor->info.t_clk_trail = clamp_t(unsigned int, tmp, 0, 0xf);

	tmp = 1;
	fwnode_property_read_u32(ep, "onsemi,t-clk-prep", &tmp);
	sensor->info.t_clk_prep = clamp_t(unsigned int, tmp, 0, 0xf);

	tmp = 4;
	fwnode_property_read_u32(ep, "onsemi,t-hs-exit", &tmp);
	sensor->info.t_hs_exit = clamp_t(unsigned int, tmp, 0, 0x3f);

	tmp = 14;
	fwnode_property_read_u32(ep, "onsemi,t-clk-zero", &tmp);
	sensor->info.t_clk_zero = clamp_t(unsigned int, tmp, 0, 0x3f);

	tmp = 2;
	fwnode_property_read_u32(ep, "onsemi,t-bgap", &tmp);
	sensor->info.t_bgap = clamp_t(unsigned int, tmp, 0, 0xf);

	tmp = 1;
	fwnode_property_read_u32(ep, "onsemi,t-clk-pre", &tmp);
	sensor->info.t_clk_pre = clamp_t(unsigned int, tmp, 0, 0x3f);

	tmp = 7;
	fwnode_property_read_u32(ep, "onsemi,t-clk-post", &tmp);
	sensor->info.t_clk_post = clamp_t(unsigned int, tmp, 0, 0x3f);

	tmp = 2;
	fwnode_property_read_u32(ep, "onsemi,t-lpx", &tmp);
	sensor->info.t_lpx = clamp_t(unsigned int, tmp, 0, 0x3f);

	tmp = 5;
	fwnode_property_read_u32(ep, "onsemi,t-wakeup", &tmp);
	sensor->info.t_wakeup = clamp_t(unsigned int, tmp, 0, 0x7f);

	tmp = 0;
	fwnode_property_read_u32(ep, "onsemi,cont-tx-clk", &tmp);
	sensor->info.cont_tx_clk = tmp ? true : false;

	tmp = 0;
	fwnode_property_read_u32(ep, "onsemi,heavy-lp-load", &tmp);
	sensor->info.heavy_lp_load = tmp ? true : false;

	tmp = 4;
	fwnode_property_read_u32(ep, "onsemi,t-init", &tmp);
	sensor->info.t_init = clamp_t(unsigned int, tmp, 0, 0x7f);

	return 0;
}

static int ar0144_of_probe(struct ar0144 *sensor)
{
	struct device *dev = sensor->dev;
	struct ar0144_businfo *info = &sensor->info;
	struct clk *clk;
	struct gpio_desc *gpio;
	struct fwnode_handle *ep;
	struct v4l2_fwnode_endpoint bus_cfg = {
		.bus_type = V4L2_MBUS_UNKNOWN,
	};
	unsigned long ext_freq;
	u64 *link_freqs;
	int i;
	int ret;

	clk = devm_clk_get(dev, "ext");
	ret = PTR_ERR_OR_ZERO(clk);
	if (ret == -EPROBE_DEFER)
		return ret;
	if (ret < 0) {
		dev_err(dev, "Failed to get external clock (%d)\n", ret);
		return ret;
	}

	sensor->extclk = clk;

	gpio = devm_gpiod_get_optional(dev, "reset", GPIOD_OUT_HIGH);
	ret = PTR_ERR_OR_ZERO(gpio);
	if (ret < 0) {
		dev_err(dev, "Failed to get reset gpio (%d)\n", ret);
		return ret;
	}

	sensor->reset_gpio = gpio;

	ep = fwnode_graph_get_next_endpoint(dev_fwnode(dev), NULL);
	if (!ep)
		return -EINVAL;

	ret = v4l2_fwnode_endpoint_alloc_parse(ep, &bus_cfg);
	if (ret) {
		dev_err(dev, "Failed to parse bus info (%d)\n", ret);
		goto out_put;
	}

	info->bus_type = bus_cfg.bus_type;

	if (bus_cfg.nr_of_link_frequencies != 1) {
		dev_err(dev, "Link frequency required\n");
		ret = -EINVAL;
		goto out_put;
	}

	if (info->bus_type == V4L2_MBUS_PARALLEL &&
	    bus_cfg.link_frequencies[0] > AR0144_MAX_PARALLEL_LINK) {
		dev_err(dev, "Parallel Link frequency exceeds maximum\n");
		ret = -EINVAL;
		goto out_put;
	}

	if (info->bus_type == V4L2_MBUS_CSI2_DPHY &&
	    bus_cfg.link_frequencies[0] > AR0144_MAX_MIPI_LINK) {
		dev_err(dev, "MIPI Link frequency exceeds maximum\n");
		ret = -EINVAL;
		goto out_put;
	}

	switch (info->bus_type) {
	case V4L2_MBUS_PARALLEL:
		ret = ar0144_parse_parallel_props(sensor, ep, &bus_cfg);
		break;
	case V4L2_MBUS_CSI2_DPHY:
		ret = ar0144_parse_mipi_props(sensor, ep, &bus_cfg);
		break;
	default:
		dev_err(dev, "Invalid bus type\n");
		ret = -EINVAL;
	}

	if (ret)
		goto out_put;

	link_freqs = devm_kcalloc(dev, 3, sizeof(*info->link_freqs),
				  GFP_KERNEL);
	if (!link_freqs) {
		ret = -ENOMEM;
		goto out_put;
	}

	ext_freq = clk_get_rate(sensor->extclk);

	for (i = 0; i < 3; i++) {
		ret = ar0144_calculate_pll(sensor, dev, &sensor->pll[i],
					   ext_freq,
					   bus_cfg.link_frequencies[0],
					   index_to_bpp(i));
		if (ret)
			goto out_put;

		link_freqs[i] = sensor->pll[i].ser_freq;;
	}

	info->link_freqs = link_freqs;

out_put:
	v4l2_fwnode_endpoint_free(&bus_cfg);
	fwnode_handle_put(ep);
	return ret;
}

static int ar0144_probe(struct i2c_client *i2c,
			const struct i2c_device_id *did)
{
	struct ar0144 *sensor;
	struct v4l2_subdev *sd;
	int ret;

	sensor = devm_kzalloc(&i2c->dev, sizeof(*sensor), GFP_KERNEL);
	if (!sensor)
		return -ENOMEM;

	sd = &sensor->subdev;
	sensor->model = did->driver_data;
	sensor->dev = &i2c->dev;

	dev_info(sensor->dev, "Probing AR0144 Driver\n");

	ret = ar0144_of_probe(sensor);
	if (ret)
		return ret;

	mutex_init(&sensor->lock);

	v4l2_i2c_subdev_init(sd, i2c, &ar0144_subdev_ops);

	sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	sd->internal_ops = &ar0144_subdev_internal_ops;
	sd->entity.function = MEDIA_ENT_F_CAM_SENSOR;
	sd->entity.ops = &ar0144_entity_ops;

	sensor->pad.flags = MEDIA_PAD_FL_SOURCE;

	ret = media_entity_pads_init(&sd->entity, 1, &sensor->pad);
	if (ret)
		goto out_media;

	ret = ar0144_check_chip_id(sensor);
	if (ret)
		goto out_media;

	ret = v4l2_async_register_subdev(&sensor->subdev);
	if (ret)
		goto out;

	return 0;

out:
	v4l2_ctrl_handler_free(&sensor->ctrls);
out_media:
	media_entity_cleanup(&sd->entity);
	mutex_destroy(&sensor->lock);
	return ret;
}

static int ar0144_remove(struct i2c_client *i2c)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(i2c);
	struct ar0144 *sensor = to_ar0144(sd);

	v4l2_async_unregister_subdev(sd);
	v4l2_ctrl_handler_free(&sensor->ctrls);
	media_entity_cleanup(&sd->entity);
	mutex_destroy(&sensor->lock);

	return 0;
}

static const struct i2c_device_id ar0144_id_table[] = {
	{ "ar0144", AR0144_MODEL_UNKNOWN },
	{ "ar0144c", AR0144_MODEL_COLOR },
	{ "ar0144m", AR0144_MODEL_MONOCHROME },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(i2c, ar0144_id_table);

static const struct of_device_id ar0144_of_match[] = {
	{ .compatible = "onsemi,ar0144" },
	{ .compatible = "onsemi,ar0144c" },
	{ .compatible = "onsemi,ar0144m" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, ar0144_of_match);

static struct i2c_driver ar0144_i2c_driver = {
	.driver	= {
		.name = "ar0144",
		.of_match_table	= of_match_ptr(ar0144_of_match),
	},
	.probe		= ar0144_probe,
	.remove		= ar0144_remove,
	.id_table	= ar0144_id_table,
};
module_i2c_driver(ar0144_i2c_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stefan Riedmueller <s.riedmueller@phytec.de>");
