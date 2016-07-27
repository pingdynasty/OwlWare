#ifndef __MIDI_CONTROL_H
#define __MIDI_CONTROL_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
 extern "C" {
#endif

   void midi_receive_usb_buffer(uint8_t *buffer, uint16_t length);
   void midi_send_usb_buffer(uint8_t* buffer, uint16_t length);
   bool midi_device_connected();
   void midi_set_input_channel(int8_t ch);
   void midi_set_output_channel(int8_t ch);

#ifdef __cplusplus
}
#endif

#endif /* __MIDI_CONTROL_H */
