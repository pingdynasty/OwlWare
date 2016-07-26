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
   bool midi_set_input_channel(uint8_t ch);
   bool midi_set_output_channel(uint8_t ch);

#ifdef __cplusplus
}
#endif

#endif /* __MIDI_CONTROL_H */
