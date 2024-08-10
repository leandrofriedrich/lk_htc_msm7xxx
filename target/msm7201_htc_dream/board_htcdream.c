/*
 * board_htcdream.c
 * provides support for the HTC Dream board for LK bootloader
 *
 * Copyright (C) 2011 Alexander Tarasikov <alexander.tarasikov@gmail.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <array.h>
#include <bootreason.h>
#include <debug.h>
#include <reg.h>
#include <target.h>
#include <dev/battery/ds2746.h>
#include <dev/fbcon.h>
#include <dev/flash.h>
#include <dev/gpio.h>
#include <dev/gpio_keys.h>
#include <dev/keys.h>
#include <dev/udc.h>
#include <lib/ptable.h>
#include <platform/clock.h>
#include <platform/timer.h>
#include <platform/msm_gpio.h>
#include <platform/msm_i2c.h>
#include <platform/hsusb.h>
#include <target/dynboard.h>
#include <target/msm7200a_hsusb.h>
#include <target/dex_vreg.h>
#include <target/dex_comm.h>
#include <target/htcdream.h>


static struct fbcon_config *fb_config;
/******************************************************************************
 * LCD
 *****************************************************************************/
extern struct fbcon_config *lcdc_init(void);

static void htcdream_display_init(void)
{

#if DISPLAY_TYPE_MDDI
	fb_config = mddi_init();
	ASSERT(fb_config);
	fbcon_setup(fb_config);
#endif

#if DISPLAY_TYPE_LCDC
	fb_config = lcdc_init();
	ASSERT(fb_config);
	fbcon_setup(fb_config);
#endif
}

static void htcdream_display_exit(void) {
	fbcon_teardown();
}

/******************************************************************************
 * LEDs
 *****************************************************************************/
static void htcdream_set_key_light(int brightness) {
	unsigned char buffer[3] = {};

	//Front key light
	buffer[0] = 0x14;
	buffer[1] = brightness ? 0x80 : 0;
	buffer[2] = brightness;
	msm_i2c_write(0x66, buffer, 3);
}

static void htcdream_set_color_leds(bool red, bool green, bool blue) {
	unsigned char buffer[2] = {};
	buffer[0] = 0x20;
	buffer[1] = red | (green << 1) | (blue << 2);
	msm_i2c_write(0x66, buffer, 2);
}

/******************************************************************************
 * USB
 *****************************************************************************/
static void htcdream_usb_disable(void)
{
	gpio_set(KOVS100_USB_RESET_PHY, 0);
	gpio_set(KOVS100_USB_POWER_PHY, 0);
}

static void htcdream_usb_enable(void)
{
	gpio_set(0x54, 1);
	gpio_set(KOVS100_BT_ROUTER, 0);

	gpio_set(KOVS100_USB_POWER_PHY, 1);
	gpio_set(KOVS100_USB_RESET_PHY, 0);
	mdelay(3);
	gpio_set(KOVS100_USB_RESET_PHY, 1);
	mdelay(3);
}

static void msm_hsusb_set_state(int state)
{
	if (state) {
		htcdream_usb_enable();
		msm7200a_ulpi_config(1);
	} else {
		msm7200a_ulpi_config(0);
		htcdream_usb_disable();
	}
}

static void htcdream_set_charger(enum psy_charger_state state) {
	if (state == CHG_OFF) {
		gpio_set(KOVS100_N_CHG_ENABLE, 1);
		gpio_set(KOVS100_N_CHG_INHIBIT, 0);
		gpio_set(KOVS100_CHG_HIGH, 0);
	}
	else {
		gpio_set(KOVS100_N_CHG_ENABLE, 0);
		gpio_set(KOVS100_N_CHG_INHIBIT, 1);
		gpio_set(KOVS100_CHG_HIGH, 1);
	}
}

static bool htcdream_usb_online(void) {
	return dex_get_vbus_state();
}

static bool htcdream_ac_online(void) {
	return 0;
}

static bool htcdream_want_charging(void) {
	uint32_t voltage = ds2746_read_voltage_mv(0x36);
	return voltage < DS2746_HIGH_VOLTAGE;
}

static void htcdream_wait_for_charge(void) {
	uint32_t voltage;
	int current;
	uint8_t no_charger_cycles = 0;
	bool power = false;
	do {
		gpio_set(KOVS100_N_CHG_ENABLE, 1);
		mdelay(10);
		voltage = ds2746_read_voltage_mv(0x36);
		current = ds2746_read_current_ma(0x36, 1500);
		
		power = htcdream_usb_online();
		if (power) {
			htcdream_set_charger(CHG_USB_HIGH);
			no_charger_cycles = 0;
			htcdream_set_color_leds(0, 1, 0);
		}
		else {
			no_charger_cycles++;
			htcdream_set_charger(CHG_OFF);
			htcdream_set_color_leds(1, 0, 0);

			//If no charger connected for 6 seconds and we're low on battery
			if (no_charger_cycles > 12) {
				target_shutdown();
			}
		}
		printf("[BAT] voltage=%d current=%d\n", voltage, current);
		mdelay(500);
		//ok, don't drop charge but instead increase minimum voltage to
		//compensate for incorrect reading
	} while ((power && (voltage < 3700)) || (!power && (voltage < 3600)));
	if (get_boot_reason() == BOOT_CHARGING)
		reboot_device(BOOT_WARM);
}

static struct pda_power_supply htcdream_power_supply = {
	.is_ac_online = htcdream_ac_online,
	.is_usb_online = htcdream_usb_online,
	.set_charger_state = htcdream_set_charger,
	.want_charging = htcdream_want_charging,
};

static struct msm_hsusb_pdata htcdream_hsusb_pdata = {
	.set_ulpi_state = msm_hsusb_set_state,
	.power_supply = &htcdream_power_supply,
};

static struct udc_device htcdream_udc_device = {
	.vendor_id = 0x18d1,
	.product_id = 0xD00D,
	.version_id = 0x0100,
	.manufacturer = "Sony Ericsson",
	.product = "Xperia X1",
};

static void htcdream_usb_init(void) {
	msm_hsusb_init(&htcdream_hsusb_pdata);
	int ret = udc_init(&htcdream_udc_device);
	dprintf(VDEBUG, "udc_init done with ret=%d\n", ret);
}

/******************************************************************************
 * GPIO Keys
 *****************************************************************************/
static struct gpio_key htcdream_gpio_keys[] = {
	{KOVS100_POWER_KEY, KEY_POWER, true, "Power"},
	{KOVS100_CAM_FULL_KEY, KEY_DOWN, true, "Camera full press"},
	{KOVS100_CAM_HALF_KEY, KEY_UP, true, "Camera half press"},
};

static struct gpio_keys_pdata htcdream_gpio_keys_pdata = {
	.keys = htcdream_gpio_keys,
	.nkeys = ARRAY_SIZE(htcdream_gpio_keys),
};

static void htcdream_gpio_keys_init(void) {
	for (unsigned i = 0; i < ARRAY_SIZE(htcdream_gpio_keys); i++) {
		msm_gpio_config(MSM_GPIO_CFG(htcdream_gpio_keys[i].gpio,
				0, MSM_GPIO_CFG_INPUT,
				MSM_GPIO_CFG_NO_PULL, MSM_GPIO_CFG_2MA), 0);
	}
	gpio_keys_init(&htcdream_gpio_keys_pdata);
}
/******************************************************************************
 * NAND
 *****************************************************************************/
/* CE starts at 0x02820000
 * block size is 0x20000
 * offset is the size of all the data we don't want to mess with,
 * namely, bootloader and calibration
 * the last 16 mbytes are used for nand block markers etc so don't
 * use anything after 0x1fe00000
 *
 * TODO: make a fake mbr, specify LK as XIP
 * and the rest as imgfs or uldr and correct partitions offsets
 *
 */

#define HTCdream_RESERVED_SECTORS 321
#define HTCdream_FLASH_SIZE 0x1000

#define HTCdream_FLASH_OFFSET (HTCdream_RESERVED_SECTORS + 1)

#define HTCdream_FLASH_RECOVERY_START 0
#define HTCdream_FLASH_RECOVERY_SIZE 0x50

#define HTCdream_FLASH_MISC_START (HTCdream_FLASH_RECOVERY_START + \
	HTCdream_FLASH_RECOVERY_SIZE)
#define HTCdream_FLASH_MISC_SIZE 5

#define HTCdream_FLASH_BOOT_START (HTCdream_FLASH_MISC_START + \
	HTCdream_FLASH_MISC_SIZE)
#define HTCdream_FLASH_BOOT_SIZE 0x50

#define HTCdream_FLASH_SYS_START (HTCdream_FLASH_BOOT_START + \
	HTCdream_FLASH_BOOT_SIZE)
#define HTCdream_FLASH_SYS_SIZE 0x488

#define HTCdream_FLASH_DATA_START (HTCdream_FLASH_SYS_START + \
	HTCdream_FLASH_SYS_SIZE)
#define HTCdream_FLASH_DATA_SIZE 0x908

#define HTCdream_FLASH_CACHE_SIZE 0x80
#define HTCdream_FLASH_CACHE_START (HTCdream_FLASH_SIZE - \
	(HTCdream_FLASH_OFFSET + HTCdream_FLASH_CACHE_SIZE))

#if HTCdream_FLASH_CACHE_START < HTCdream_FLASH_OFFSET
	#error Cache partition starts in protected zone
#endif

#if (HTCdream_FLASH_DATA_START + HTCdream_FLASH_DATA_SIZE) \
	> HTCdream_FLASH_CACHE_START
	#error Incorrect flash layout. Verify partition sizes manually
#endif

static struct ptable flash_ptable;
static struct ptentry htcdream_part_list[] = {
	{
		.start = HTCdream_FLASH_RECOVERY_START,
		.length = HTCdream_FLASH_RECOVERY_SIZE,
		.name = "recovery",
		.type = TYPE_APPS_PARTITION,
		.perm = PERM_WRITEABLE,
	},
	{
		.start = HTCdream_FLASH_MISC_START,
		.length = HTCdream_FLASH_MISC_SIZE,
		.name = "misc",
		.type = TYPE_APPS_PARTITION,
		.perm = PERM_WRITEABLE,
	},
	{
		.start = HTCdream_FLASH_BOOT_START,
		.length = HTCdream_FLASH_BOOT_SIZE,
		.name = "boot",
		.type = TYPE_APPS_PARTITION,
		.perm = PERM_WRITEABLE,
	},
	{
		.start = HTCdream_FLASH_SYS_START,
		.length = HTCdream_FLASH_SYS_SIZE,
		.name = "system",
		.type = TYPE_APPS_PARTITION,
		.perm = PERM_WRITEABLE,
	},
	{
		.start = HTCdream_FLASH_DATA_START,
		.length = HTCdream_FLASH_DATA_SIZE,
		.name = "userdata",
		.type = TYPE_APPS_PARTITION,
		.perm = PERM_WRITEABLE,
	},
	{
		.start = HTCdream_FLASH_CACHE_START,
		.length = HTCdream_FLASH_CACHE_SIZE,
		.name = "cache",
		.type = TYPE_APPS_PARTITION,
		.perm = PERM_WRITEABLE,
	},
};

static void htcdream_nand_init(void) {
	struct flash_info *flash_info;
	int nparts = ARRAY_SIZE(htcdream_part_list);
	unsigned offset = HTCdream_FLASH_OFFSET;
	ptable_init(&flash_ptable);
	flash_init();
	flash_info = flash_get_info();
	if (!flash_info) {
		dprintf(CRITICAL, "%s: error initializing flash");
		return;
	}

	for (int i = 0; i < nparts; i++) {
		struct ptentry *ptn = &htcdream_part_list[i];
		unsigned len = ptn->length;

		if ((len == 0) && (i == nparts - 1)) {
			len = flash_info->num_blocks - offset - ptn->start;
		}
		ptable_add(&flash_ptable, ptn->name, offset + ptn->start,
			len, ptn->flags, ptn->type, ptn->perm);
	}

	ptable_dump(&flash_ptable);
	flash_set_ptable(&flash_ptable);
}
/******************************************************************************
 * Exports
 *****************************************************************************/
static void htcdream_early_init(void) {
	htcdream_display_init();
}

static void htcdream_init(void) {
	if (get_boot_reason() == BOOT_CHARGING) {
		htcdream_set_color_leds(1, 1, 0);
	}
	htcdream_wait_for_charge();

	htcdream_set_key_light(160);
	htcdream_set_color_leds(1, 0, 1);
	//set half rate to reduce flicker
	clk_set_rate(MDP_CLK, 109 * 1000 * 1000);
	htcdream_usb_init();
	htcdream_nand_init();
	htcdream_gpio_keys_init();
	printf("\n\n===========================\n\n");
	printf(" press  CAMERA  for  RECOVERY\n");
	printf(" press  POWER  for  FASTBOOT\n");
	printf("\n===========================\n\n");
}

static void htcdream_exit(void) {
	htcdream_set_color_leds(0, 1, 1);
//	htcdream_display_exit();
}

struct msm7k_board htcdream_board = {
	.early_init = htcdream_early_init,
	.init = htcdream_init,
	.exit = htcdream_exit,
	.cmdline = "fbcon=rotate:2"
	" smd_rpcrouter.hot_boot=1"
	" init=/init physkeyboard=kovsq"
	" force_cdma=0 hwrotation=180 lcd.density=240"
	" msmvkeyb_toggle=off",
};
