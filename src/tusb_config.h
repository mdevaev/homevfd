#pragma once


//--------------------------------------------------------------------
// Common config
//--------------------------------------------------------------------

#define CFG_TUSB_OS			OPT_OS_PICO

// Enable device stack
#define CFG_TUD_ENABLED		1

// CFG_TUSB_DEBUG is defined by compiler in DEBUG build
//#define CFG_TUSB_DEBUG	100

// USB DMA on some MCUs can only access a specific SRAM region with restriction on alignment.
// Tinyusb use follows macros to declare transferring memory so that they can be put
// into those specific section.
//   - CFG_TUSB_MEM SECTION : __attribute__ (( section(".usb_ram") ))
//   - CFG_TUSB_MEM_ALIGN   : __attribute__ ((aligned(4)))
#ifndef CFG_TUSB_MEM_SECTION
#	define CFG_TUSB_MEM_SECTION
#endif


#ifndef CFG_TUSB_MEM_ALIGN
#	define CFG_TUSB_MEM_ALIGN __attribute__((aligned(4)))
#endif


// Это здесь просто для удобства, не делать меньше 1024
#define HV_USB_MAX_CMD_SIZE	2048


//--------------------------------------------------------------------
// Device config
//--------------------------------------------------------------------

#ifndef CFG_TUD_ENDPOINT0_SIZE
#	define CFG_TUD_ENDPOINT0_SIZE 64
#endif

#define CFG_TUD_CDC				1

// CDC FIFO size of TX and RX
#define CFG_TUD_CDC_RX_BUFSIZE	(HV_USB_MAX_CMD_SIZE * 4)
#define CFG_TUD_CDC_TX_BUFSIZE	(HV_USB_MAX_CMD_SIZE * 4)

// CDC Endpoint transfer buffer size, more is faster
#define CFG_TUD_CDC_EP_BUFSIZE	64
