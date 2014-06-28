#if defined( USE_USB_CDC )

// boards\RFM96W-BB\STM32_USB-FS-Device_Driver

#include "..\src\usb_core.c"
#include "..\src\usb_init.c"
#include "..\src\usb_int.c"
#include "..\src\usb_mem.c"
#include "..\src\usb_regs.c"
#include "..\src\usb_sil.c"

// boards\RFM96W-BB\usb
#include "..\src\usb_desc.c"
#include "..\src\usb_istr.c"
#include "..\src\usb_prop.c"
#include "..\src\usb_pwr.c"

#include "usb-cdc-board.c"
#include "uart-usb-board.c"

#endif
