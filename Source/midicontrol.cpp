#include "midicontrol.h"
#include "armcontrol.h"
#include "owlcontrol.h"
#include "usbd_conf.h"
#include "usbd_audio_core.h"
#include "MidiReader.h"
#include "DigitalBusReader.h"
#include <string.h>
#include "device.h"

extern MidiReader midireader;
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

bool midi_device_connected(){
#ifdef OWLRACK
  return true; // always send midi on bus
#else
  return usbd_usr_device_status > 0x02;
#endif
}

void midi_receive_usb_buffer(uint8_t *buffer, uint16_t length){
  length /= 4;
  while(length--){
    midireader.readMidiFrame(buffer);
// #ifdef OWLRACK
//     // propagate message through digital bus
//     bus.sendFrame(buffer);
// #endif
    buffer += 4;
  }
}

void midi_send_usb_buffer(uint8_t* buffer, uint16_t length) {
  /* Add a MIDI message to the USB buffer. These need to be written in
   * discrete chunks of 4 bytes, because the host can produce unexpected behaviour
   * if a USB transaction comes in which contains only part of a MIDI packet (e.g. if
   * a start-of-frame request happens to fall within the loop of copying the packet
   * byte-by-byte). One consequence of this is that the USB buffer size must be a multiple
   * of 4 (the packet size).
   */
#ifdef OWLRACK
  // propagate message through digital bus
  bus_tx_midi(buffer);
#endif
  /* Check if device is online */
  if(USB_OTG_dev.dev.device_status != USB_OTG_CONFIGURED)
    return;
#ifdef OWLRACK
  if((APP_RX_DATA_SIZE + APP_Rx_ptr_out - APP_Rx_ptr_in) % APP_RX_DATA_SIZE == length)
    return;
#else
  /* If the buffer is completely full, wait until the USB peripheral clears
   * it to continue. */
  while((APP_RX_DATA_SIZE + APP_Rx_ptr_out - APP_Rx_ptr_in) % APP_RX_DATA_SIZE == length);
#endif
  memcpy(&APP_Rx_Buffer[APP_Rx_ptr_in], buffer, length);
  APP_Rx_ptr_in += length;
  if(APP_Rx_ptr_in >= APP_RX_DATA_SIZE)
    APP_Rx_ptr_in = 0;
}

void midi_set_input_channel(int8_t ch){
  midireader.setInputChannel(ch);
  bus_set_midi_channel(ch);
}
