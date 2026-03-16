#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/watchdog.h"

#include "tusb.h"

#include "hv_const.h"
#include "hv_types.h"
#include "hv_cmd.h"
#include "hv_usb.h"
#include "hv_vfd.h"


#ifdef HV_DEV_BUILD
#	warning "===== Building the DEV firmware ====="
#endif


static void _send_ack(u16 rid) {
	hv_cmd_out_ack_s cmd;
	hv_cmd_header_init(&cmd.header, rid, HV_CMD_OP_ACK);
	hv_usb_write(&cmd, sizeof(cmd));
}

static void _send_nak(u16 rid, u8 reason) {
	hv_cmd_out_nak_s cmd = {.reason = reason};
	hv_cmd_header_init(&cmd.header, rid, HV_CMD_OP_NAK);
	hv_usb_write(&cmd, sizeof(cmd));
}

#define SEND_ACK { _send_ack(cmd->header.rid); }

static void _run_cmd_bootloader(const hv_cmd_in_bootloader_s *cmd) {
	(void)cmd;
	hv_usb_close();
	sleep_ms(20);
	reset_usb_boot(0, 0);
}

static void _run_cmd_reboot(const hv_cmd_in_reboot_s *cmd) {
	(void)cmd;
	hv_usb_close();
	sleep_ms(10);
	watchdog_enable(1, false);
	while (true);
}

static void _run_cmd_set_grid_status(const hv_cmd_in_set_grid_status_s *cmd) {
	hv_vfd_set_grid_status(cmd->status);
	SEND_ACK;
}

static void _run_cmd_set_bat_status(const hv_cmd_in_set_bat_status_s *cmd) {
	hv_vfd_set_bat_status(cmd->status);
	SEND_ACK;
}

static void _run_cmd_set_bat_level(const hv_cmd_in_set_bat_level_s *cmd) {
	hv_vfd_set_bat_level(cmd->level);
	SEND_ACK;
}

static void _run_cmd_set_water_status(const hv_cmd_in_set_water_status_s *cmd) {
	hv_vfd_set_water_status(cmd->status);
	SEND_ACK;
}

static void _run_cmd_set_water_level(const hv_cmd_in_set_water_level_s *cmd) {
	hv_vfd_set_water_level(cmd->level);
	SEND_ACK;
}

static void _run_cmd_set_pixmap(const hv_cmd_in_set_pixmap_s *cmd) {
	hv_vfd_set_pixmap(cmd->data);
	SEND_ACK;
}

#undef SEND_ACK

static void _handle_read(const u8 *msg, uz len) {
	u16 rid = 0;
	if (len >= sizeof(hv_cmd_header_s)) {
		const hv_cmd_header_s *const header = (const hv_cmd_header_s*)msg;
		switch (header->op) {
#			define CMD(x_op, x_in, x_func) case x_op: { \
					if (len >= sizeof(x_in)) { x_func((const x_in*)msg); return; } \
				} break;
			CMD(HV_CMD_OP_BOOTLOADER,		hv_cmd_in_bootloader_s,			_run_cmd_bootloader);
			CMD(HV_CMD_OP_REBOOT,			hv_cmd_in_reboot_s,				_run_cmd_reboot);
			CMD(HV_CMD_OP_SET_GRID_STATUS,	hv_cmd_in_set_grid_status_s,	_run_cmd_set_grid_status);
			CMD(HV_CMD_OP_SET_BAT_STATUS,	hv_cmd_in_set_bat_status_s,		_run_cmd_set_bat_status);
			CMD(HV_CMD_OP_SET_BAT_LEVEL,	hv_cmd_in_set_bat_level_s,		_run_cmd_set_bat_level);
			CMD(HV_CMD_OP_SET_WATER_STATUS,	hv_cmd_in_set_water_status_s,	_run_cmd_set_water_status);
			CMD(HV_CMD_OP_SET_WATER_LEVEL,	hv_cmd_in_set_water_level_s,	_run_cmd_set_water_level);
			CMD(HV_CMD_OP_SET_PIXMAP,		hv_cmd_in_set_pixmap_s,			_run_cmd_set_pixmap);
#			undef CMD
		}
		rid = header->rid;
	}
	_send_nak(rid, HV_CMD_NAK_INVALID_COMMAND);
}

int main(void) {
	// Для USB частота должна быть множителем от 12MHz
	// Это пришло из PIO-USB свича, но я оставляю как есть на всякий случай
	set_sys_clock_khz(120000, true);

	// Заводим вачдог на пять секунд, не позволяем ему паузиться при отладке
	watchdog_enable(5000, false);

	hv_vfd_init();
	hv_usb_init();

	while (true) {
		watchdog_update();
		hv_usb_task();
		hv_vfd_task();

		if (hv_usb_write_available()) {
			hv_usb_read(_handle_read);
		}
	}
	return 0;
}
