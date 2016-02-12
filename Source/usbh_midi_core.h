#ifndef __USBH_MIDI_CORE_H_
#define __USBH_MIDI_CORE_H_

#include <usbd_core.h>
#include <usbd_def.h>
#include <usbd_desc.h>
#include <usbd_req.h>
#include <usbd_conf.h>
#include <usbh_core.h>
#include <usbh_conf.h>
#include <usb_otg.h>
#include <usb_dcd_int.h>
#include <usb_hcd_int.h>
#include <usb_regs.h>

#ifndef MBOX_DONT_USE_USB_HOST
extern const USBH_Usr_cb_TypeDef USBH_USR_Callbacks; // implemented in usbh_midi_core.c
#endif

#endif  // __USBH_MIDI_CORE_H_
