#pragma once

#include "hv_types.h"

typedef enum {
	HV_VFD_STATUS_OFF = 0,
	HV_VFD_STATUS_ON,
	HV_VFD_STATUS_BLINK,
} hv_vfd_status_e;


void hv_vfd_init(void);
void hv_vfd_task(void);

void hv_vfd_set_grid_status(hv_vfd_status_e st);
void hv_vfd_set_bat_status(hv_vfd_status_e st);
void hv_vfd_set_water_status(hv_vfd_status_e st);

void hv_vfd_set_bat_level(u8 level);
void hv_vfd_set_water_level(u8 level);

void hv_vfd_set_pixmap(const u8 *data);
