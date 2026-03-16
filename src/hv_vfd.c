#include "hv_vfd.h"

#include <string.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "hv_const.h"
#include "hv_types.h"


#define _DATA_PIN	0 // ... 7
#define _ADDR_PIN	8 // ... 17

#define _WRITE_PIN	18
#define _READY_PIN	19
//#define _CLEAR_PIN	20
#define _EN_PIN		21

#define _GRID_PIN	22
#define _WATER_PIN	26

#define _COLS		(HV_COLS + 1)
#define _PAGES		HV_PAGES
#define _END		(_COLS * _PAGES) 


static u8 _buf[_PAGES * _COLS] = {0};
static u16 _pos = 0;

static hv_vfd_status_e _grid_st = HV_VFD_STATUS_OFF;
static hv_vfd_status_e _bat_st = HV_VFD_STATUS_OFF;
static hv_vfd_status_e _water_st = HV_VFD_STATUS_OFF;
static bool _blink = false;

static void _write_grid_on(bool on);
static void _write_bat_on(bool on);
static void _write_water_on(bool on);


void hv_vfd_init(void) {
	for (u8 pin = 0; pin <= 26; ++pin) {
		if (pin == _READY_PIN || (pin > 22 && pin < 26)) {
			continue;
		}
		gpio_init(pin);
		gpio_set_dir(pin, GPIO_OUT);
		gpio_put(pin, false);
	}

	gpio_init(_READY_PIN);
	gpio_set_dir(_READY_PIN, GPIO_IN);

	gpio_put(_WRITE_PIN, true);

	while (_pos < _END) {
		hv_vfd_task();
	}

	gpio_put(_EN_PIN, true);

	_write_grid_on(false); // GPIO
	_write_bat_on(false);
}

void hv_vfd_task(void) {
	static u64 next_ts = 0;
	const u64 now_ts = time_us_64();
	if (next_ts == 0 || now_ts >= next_ts) {
		if (_grid_st == HV_VFD_STATUS_BLINK) {
			_write_grid_on(_blink);
		}
		if (_bat_st == HV_VFD_STATUS_BLINK) {
			_write_bat_on(_blink);
		}
		if (_water_st == HV_VFD_STATUS_BLINK) {
			_write_water_on(_blink);
		}
		_blink = !_blink;
		next_ts = time_us_64() + 100000;
	}

	if (_pos >= _END) {
		return; // Nothing to do
	}
	if (gpio_get(_READY_PIN)) {
		return; // Busy now
	}

	const u32 col = _pos / _PAGES;
	const u32 page = _pos % _PAGES;

	const u32 addr = (page << 7) | col;
	const u32 data = _buf[col * _PAGES + page];
    gpio_put_masked(
        ((u32)0x3FF << _ADDR_PIN) | ((u32)0xFF << _DATA_PIN),
        (addr << _ADDR_PIN) | (data << _DATA_PIN));

	sleep_us(1);
	gpio_put(_WRITE_PIN, false);
	sleep_us(1);
	gpio_put(_WRITE_PIN, true);

	++_pos;
}

static void _write_grid_on(bool on) {
	gpio_put(_GRID_PIN, on);
}

static void _write_bat_on(bool on) {
	const u16 start = _PAGES * 70 + 5;
	_buf[start] = on;
	if (_pos > start) {
		_pos = start;
	}
}

static void _write_water_on(bool on) {
	gpio_put(_WATER_PIN, on);
}

void hv_vfd_set_grid_status(hv_vfd_status_e st) {
	if (st != HV_VFD_STATUS_BLINK) {
		_write_grid_on(st);
	}
	_grid_st = st;
}

void hv_vfd_set_bat_status(hv_vfd_status_e st) {
	if (st != HV_VFD_STATUS_BLINK) {
		_write_bat_on(st);
	}
	_bat_st = st;
}

void hv_vfd_set_water_status(hv_vfd_status_e st) {
	if (st != HV_VFD_STATUS_BLINK) {
		_write_water_on(st);
	}
	_water_st = st;
}

void hv_vfd_set_bat_level(u8 level) {
	const u16 start = _PAGES * 70 + 1;
	_buf[start] = (0xF0 >> level);
	if (_pos > start) {
		_pos = start;
	}
}

void hv_vfd_set_water_level(u8 level) {
	const u16 start = _PAGES * 70;
	_buf[start] = (0xFF00 >> level);
	if (_pos > start) {
		_pos = start;
	}
}

void hv_vfd_set_pixmap(const u8 *data) {
	memcpy(_buf, data, (_COLS - 1) * _PAGES);
	_pos = 0;
}
