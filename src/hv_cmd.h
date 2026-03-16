#pragma once

#include "hv_const.h"
#include "hv_types.h"


enum {
	HV_CMD_OP_NAK				= 0,
	HV_CMD_OP_ACK,				// 1
	HV_CMD_OP_BOOTLOADER,		// 2
	HV_CMD_OP_REBOOT,			// 3
	//HV_CMD_OP_STATE,			// 4
	HV_CMD_OP_SET_GRID_STATUS	= 5,
	HV_CMD_OP_SET_BAT_STATUS,	// 6
	HV_CMD_OP_SET_BAT_LEVEL,	// 7
	HV_CMD_OP_SET_WATER_STATUS,	// 8
	HV_CMD_OP_SET_WATER_LEVEL,	// 9
	HV_CMD_OP_SET_PIXMAP,		// 10
};

enum {
	HV_CMD_NAK_INVALID_COMMAND	= 0,
};

#define STRUCT_PACKED struct __attribute__((__packed__))

typedef STRUCT_PACKED {
	u8		proto;		// Версия протокола
	u16		rid;		// Айди запроса
	u8		op;			// Код операции
} hv_cmd_header_s;

#define MAKE_CMD(x_name, ...) \
	typedef STRUCT_PACKED { \
		hv_cmd_header_s header; \
		__VA_ARGS__; \
	} x_name

MAKE_CMD(hv_cmd_out_nak_s, u8 reason);
MAKE_CMD(hv_cmd_out_ack_s);

MAKE_CMD(hv_cmd_in_bootloader_s);
MAKE_CMD(hv_cmd_in_reboot_s);

MAKE_CMD(hv_cmd_in_set_grid_status_s, u8 status);
MAKE_CMD(hv_cmd_in_set_bat_status_s, u8 status);
MAKE_CMD(hv_cmd_in_set_bat_level_s, u8 level);
MAKE_CMD(hv_cmd_in_set_water_status_s, u8 status);
MAKE_CMD(hv_cmd_in_set_water_level_s, u8 level);
MAKE_CMD(hv_cmd_in_set_pixmap_s, u8 data[HV_COLS * HV_PAGES]);

#undef MAKE_CMD
#undef STRUCT_PACKED

inline void hv_cmd_header_init(hv_cmd_header_s *header, u16 rid, u8 op) {
	header->proto = 0;
	header->rid = rid;
	header->op = op;
}
