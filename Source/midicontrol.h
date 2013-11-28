#ifndef __MIDI_CONTROL_H
#define __MIDI_CONTROL_H

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

   void midi_receive_usb_buffer(uint8_t *buffer, uint16_t length);
   void midi_send_usb_buffer(uint8_t* buffer, uint16_t length);
   void midi_send_short_message(uint8_t* msg, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* __MIDI_CONTROL_H */
