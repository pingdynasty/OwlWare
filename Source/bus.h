#ifndef __BUS_H
#define __BUS_H

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

#define BUS_STATUS_IDLE        0x00
#define BUS_STATUS_DISCO       0x01
#define BUS_STATUS_ENUM        0x02
#define BUS_STATUS_IDENT       0x04
#define BUS_STATUS_CONNECTED   0x08
#define BUS_STATUS_ERROR       0xff

#define BUS_CMD_CONFIGURE_IO   0x10

   void bus_setup();
   int bus_status();
   void bus_process();
   void bus_set_midi_channel(uint8_t ch);
   uint8_t* bus_deviceid();
   /* outgoing: send message over digital bus */
   void bus_tx_midi(uint8_t* frame);
   void bus_tx_parameter(uint8_t pid, int16_t value);
   void bus_tx_command(uint8_t cmd, int16_t data);
   void bus_tx_message(const char* msg);
   void bus_tx_data(const uint8_t* data, uint16_t size);
   /* incoming: callback when message received on digital bus */
   void bus_rx_parameter(uint8_t pid, int16_t value);
   void bus_rx_command(uint8_t cmd, int16_t data);
   void bus_rx_message(const char* msg); 
   void bus_rx_data(const uint8_t* data, uint16_t size); 
   /* error callbacks */
   void bus_tx_error(const char* reason);
   void bus_rx_error(const char* reason);

#ifdef __cplusplus
}
#endif

#endif /* __BUS_H */
