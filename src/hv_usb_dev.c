#include <string.h>

#include "pico/unique_id.h"

#include "tusb.h"
#if TUD_OPT_HIGH_SPEED
#	error "High-Speed is not supported"
#endif

#include "hv_const.h"
#include "hv_types.h"


bool hv_g_usb_has_updev = false;


void tud_mount_cb(void) {
	hv_g_usb_has_updev = true;
}

void tud_umount_cb(void) {
	hv_g_usb_has_updev = false;
}

void tud_suspend_cb(bool remote_wakeup_enabled) {
	(void)remote_wakeup_enabled;
	hv_g_usb_has_updev = false;
}

void tud_resume_cb(void) {
	hv_g_usb_has_updev = true;
}

const u8 *tud_descriptor_device_cb(void) {
	// Invoked when received GET DEVICE DESCRIPTOR
	// Application return pointer to descriptor

	static const tusb_desc_device_t desc = {
		.bLength			= sizeof(tusb_desc_device_t),
		.bDescriptorType	= TUSB_DESC_DEVICE,
		.bcdUSB				= 0x0200,

		// Use Interface Association Descriptor (IAD) for CDC
		// As required by USB Specs IAD's subclass must be common class (2) and protocol must be IAD (1)
		.bDeviceClass		= TUSB_CLASS_MISC,
		.bDeviceSubClass	= MISC_SUBCLASS_COMMON,
		.bDeviceProtocol	= MISC_PROTOCOL_IAD,

		.bMaxPacketSize0	= CFG_TUD_ENDPOINT0_SIZE,

		.idVendor			= HV_USB_VID,
		.idProduct			= HV_USB_PID,
		.bcdDevice			= 0x0100,

		.iManufacturer		= 1,
		.iProduct			= 2,
		.iSerialNumber		= 3,

		.bNumConfigurations	= 1,
	};
	return (const u8 *)&desc;
}

const u8 *tud_descriptor_configuration_cb(u8 index) {
	// Invoked when received GET CONFIGURATION DESCRIPTOR.
	// Application return pointer to descriptor.
	// Descriptor contents must exist long enough for transfer to complete.

	(void)index;

	enum {num_cdc = 0, num_cdc_data, num_total};
	static const u8 desc[] = { // Full-speed config
		TUD_CONFIG_DESCRIPTOR(
			1,		// Config number
			num_total,// Interface count
			0,		// String index
			(TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN), // Total length
			0,		// Attribute
			100		// Power in mA
		),
		TUD_CDC_DESCRIPTOR(
			num_cdc,// Interface number
			4,		// String index
			0x81,	// EPNUM_CDC_NOTIF - EP notification address
			8,		// EP notification size
			0x02,	// EPNUM_CDC_OUT - EP OUT data address
			0x82,	// EPNUM_CDC_IN - EP IN data address
			64		// EP size
		),
	};
	return desc;
}

const u16 *tud_descriptor_string_cb(u8 index, u16 lang_id) {
	// Invoked when received GET STRING DESCRIPTOR request.
	// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete.

	(void)lang_id;

	static u16 desc_str[32];
	uz desc_str_len;

	if (index == 0) {
		desc_str[1] = 0x0409; // Supported language is English (0x0409)
		desc_str_len = 1;
	} else {
		char str[32];
		switch (index) {
			case 1: strcpy(str, HV_USB_MFC); break; // Manufacturer
			case 2: strcpy(str, HV_USB_PROD); break; // Product
			case 3: pico_get_unique_board_id_string(str, 32); break;
			case 4: strcpy(str, HV_USB_PROD " CDC"); break; // CDC interface
			default: return NULL;
		}
		desc_str_len = strlen(str);
		for (uz i = 0; i < desc_str_len; ++i) {
			desc_str[i + 1] = str[i]; // Convert ASCII string into UTF-16
		}
	}

	// First byte is length (including header), second byte is string type
	desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * desc_str_len + 2);
	return desc_str;
}
