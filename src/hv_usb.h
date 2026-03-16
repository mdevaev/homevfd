#pragma once

#include "tusb.h"

#include "hv_types.h"


typedef void (*hv_usb_read_f)(const u8 *, uz);

extern bool hv_g_usb_has_updev; // Есть канальная связь с USB
extern bool hv_g_usb_has_uplink; // Есть TTY-связь по USB


void hv_usb_init(void);
void hv_usb_close(void);
void hv_usb_task(void);

bool hv_usb_write_available(void);
void hv_usb_write(const void *msg, uz len);
void hv_usb_read(hv_usb_read_f cb);
