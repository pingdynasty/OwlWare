#include "midicontrol.h"
#include "armcontrol.h"
#include "owlcontrol.h"
#include "usbd_conf.h"
#include "usbd_audio_core.h"
#include "MidiReader.hpp"
#include <string.h>

extern USB_OTG_CORE_HANDLE           USB_OTG_dev;
/* These are external variables imported from USB-MIDI core to be used for IN
   transfer management. It is extremely similar in structure to USB CDC-class
   devices since both use bulk endpoints to transfer serial data. */
extern uint8_t  APP_Rx_Buffer []; /* Write MIDI received data in this buffer.
                                     These data will be sent over USB IN endpoint
                                     in the MIDI core functions. */
extern volatile uint32_t APP_Rx_ptr_in;    /* Increment this pointer or roll it back to
                                     start address when writing received data
                                     in the buffer APP_Rx_Buffer. */
extern volatile uint32_t APP_Rx_ptr_out;   /* This pointer is used by the MIDI driver to
                                     manage data it's sent. We need to keep track
                                     of it to ensure we don't write over data that
                                     has yet to be sent. */

/* status flag that is set when the USB device is connected */
extern uint8_t usbd_usr_device_status;
MidiReader handler;

bool midi_device_connected(){
  return usbd_usr_device_status > 0x02;
}

void midi_receive_usb_buffer(uint8_t *buffer, uint16_t length){    
  for(int i=1; i<length; ++i){
    // skip every 4th byte
    if(i & 0x3){
      MidiReaderStatus status = handler.read(buffer[i]);
      if(status == ERROR_STATUS){
	handler.clear();
	// todo: error message
	return; // discard the rest of the message
      }
    }
  }
}

// void midi_send_short_message(uint8_t* msg, uint16_t length) {
//   uint8_t packet[4];
//   // assert_param(length < sizeof(packet));
//   switch(length){
//   case 3:
//     packet[3] = msg[2];
//     // deliberate fall-through
//   case 2:
//     packet[2] = msg[1];
//     // deliberate fall-through
//   case 1:
//     packet[1] = msg[0];
//   }
//   packet[0] = msg[0]>>4; // todo: set cable id
//   midi_send_usb_buffer(packet, sizeof(packet));
// }

void midi_send_usb_buffer(uint8_t* buffer, uint16_t length) {
  /* Add a MIDI message to the USB buffer. These need to be written in
   * discrete chunks of 4 bytes, because the host can produce unexpected behaviour
   * if a USB transaction comes in which contains only part of a MIDI packet (e.g. if
   * a start-of-frame request happens to fall within the loop of copying the packet
   * byte-by-byte). One consequence of this is that the USB buffer size must be a multiple
   * of 4 (the packet size).
   */
  /* Check if device is online */
  if(USB_OTG_dev.dev.device_status != USB_OTG_CONFIGURED)
    return;
  /* If the buffer is completely full, wait until the USB peripheral clears
   * it to continue. */
  while((APP_RX_DATA_SIZE + APP_Rx_ptr_out - APP_Rx_ptr_in) % APP_RX_DATA_SIZE == length);
  memcpy(&APP_Rx_Buffer[APP_Rx_ptr_in], buffer, length);
  APP_Rx_ptr_in += length;
  if(APP_Rx_ptr_in >= APP_RX_DATA_SIZE)
    APP_Rx_ptr_in = 0;
}
