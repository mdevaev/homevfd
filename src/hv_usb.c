#include "hv_usb.h"

// #include "hardware/structs/usb.h"

#include "tusb.h"

#include "hv_types.h"


#define _ESC	0xF0
#define _BEGIN	0xF1
#define _END	0xF2


bool hv_g_usb_has_uplink = false;


static sz _encode(u8 *enc, const void *msg, uz len);
static bool _append_enc(u8 *enc, uz *pos, const void *data, uz len);
static bool _handle_dec(u8 *dec, sz *pos, bool *esc, hv_usb_read_f cb, u8 ch);


void hv_usb_init(void) {
	tud_init(0);
}

void hv_usb_close(void) {
	tud_disconnect(); // ((usb_hw_t*)hw_clear_alias_untyped(usb_hw))->sie_ctrl = USB_SIE_CTRL_PULLUP_EN_BITS;
	// tud_task();
	//
	// XXX: Если tud_task() вызывать после tud_disconnect(), и посылать сообщения больше 63 байтов,
	// то USB падает в панику "ep 0 out was already available". Похожая проблема обсуждается здесь:
	//   - https://github.com/adafruit/circuitpython/issues/8824
	//   - https://github.com/hathach/tinyusb/pull/2492
	//
	// При этом их фикс в нашем случае не помогает, помогает только не использовать tud_task(),
	// но не понятно, надо ли вообще его использовать после tud_disconnect(). Опять же, в нашем случае
	// соединение закрывается только для перепрошивки устройства, так что на это можно положить болт.
	// В новой версии TinyUSB патч уже включен.
	//
	// На текущей версии TinyUSB tud_disconnect() просто дергает регистр, отключая внутренний pull-up
	// резистор на D+/D-. Если в будущем это изменится, возможно это нужно будет откатить
	// и дергать регистр самим.
}

void hv_usb_task(void) {
	tud_task();

	const bool uplink = tud_cdc_connected();
	if (hv_g_usb_has_uplink != uplink) {
		hv_g_usb_has_uplink = uplink;
	}

	if (!hv_g_usb_has_uplink) {
		//tud_cdc_read_flush();
		tud_cdc_write_clear();
	}
}

bool hv_usb_write_available(void) {
	return (tud_cdc_write_available() >= HV_USB_MAX_CMD_SIZE);
}

void hv_usb_write(const void *msg, uz len) {
	if (!hv_g_usb_has_uplink || !hv_usb_write_available()) {
		return;
	}
	u8 enc[HV_USB_MAX_CMD_SIZE];
	const sz enc_len = _encode(enc, msg, len);
	if (enc_len > 0) {
		tud_cdc_write(enc, enc_len);
		tud_cdc_write_flush();
	}
}

static sz _encode(u8 *enc, const void *msg, uz len) {
	uz pos = 1;
	enc[0] = _BEGIN;
	if (!_append_enc(enc, &pos, msg, len)) {
		return -1;
	}
	if (pos >= HV_USB_MAX_CMD_SIZE) {
		return -1;
	}
	enc[pos] = _END;
	++pos;
	return pos;
}

static bool _append_enc(u8 *enc, uz *pos, const void *data, uz len) {
	for (uz i = 0; i < len; ++i, ++(*pos)) {
		if (*pos >= HV_USB_MAX_CMD_SIZE - 1) { // Одно место резервируется на случай эскейпа
			return false;
		}
		u8 ch = ((u8 *)data)[i];
		if (ch == _ESC || ch == _BEGIN || ch == _END) {
			enc[*pos] = _ESC;
			++(*pos);
			enc[*pos] = ~ch;
		} else {
			enc[*pos] = ch;
		}
	}
	return true;
}

void hv_usb_read(hv_usb_read_f cb) {
	static u8 dec[HV_USB_MAX_CMD_SIZE];
	static sz pos = -1;
	static bool esc = false;
	while (hv_g_usb_has_uplink && tud_cdc_available() > 0) {
		const s32 ch = tud_cdc_read_char();
		if (ch < 0 || !_handle_dec(dec, &pos, &esc, cb, ch)) {
			break;
		}
	}
}

static bool _handle_dec(u8 *dec, sz *pos, bool *esc, hv_usb_read_f cb, u8 ch) {
	if (*pos >= HV_USB_MAX_CMD_SIZE) {
		*pos = -1; // Передача не была начата с _BEGIN
		*esc = false;
	} else if (ch == _BEGIN) {
		*pos = 0;
		*esc = false;
	} else if (ch == _END) {
		if (*pos > 0) { // Обрабатываем непустые сообщения
			cb(dec, *pos);
		}
		*pos = -1;
		*esc = false;
		return false; // По одной команде за раз
	} else if (*pos >= 0 && ch == _ESC) {
		*esc = true;
	} else if (*pos >= 0) {
		if (*esc) {
			ch = ~ch;
			*esc = false;
		}
		dec[*pos] = ch;
		++(*pos);
	}
	return true;
}
