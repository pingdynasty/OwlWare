#ifndef __USB_MIDI_CORE_H_
#define __USB_MIDI_CORE_H_

#include "usbd_ioreq.h"
#include "usbd_req.h"
#include "usbd_desc.h"

#define MIDI_MAX_PACKET_SIZE			      64
#define USB_MIDI_DESC_SIZ                            0x09
#define MIDI_CONFIG_DESC_SIZE                        101 /* was 109 */
#define MIDI_DESCRIPTOR_TYPE                         0x21
#define MIDI_IN_FRAME_INTERVAL			      1 /* number of frames between MIDI IN transfers */
#define MIDI_REQ_GET_CUR                             0x81
#define MIDI_REQ_SET_CUR                             0x01
#define MIDI_OUT_STREAMING_CTRL                      0x02

extern USBD_Class_cb_TypeDef  MIDI_device_callbacks;

#endif  // __USB_MIDI_CORE_H_
