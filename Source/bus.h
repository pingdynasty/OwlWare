#ifndef __BUS_H
#define __BUS_H

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

   void bus_setup();

  /* outgoing: send message over digital bus */
  void bus_tx_parameter(uint8_t pid, int16_t value);
  /* incoming: callback when message received on digital bus */
  void bus_rx_parameter(uint8_t pid, int16_t value);
  void bus_tx_button(uint8_t bid, int16_t value);
  void bus_rx_button(uint8_t bid, int16_t value);
  void bus_tx_error(const char* reason);
  void bus_rx_error(const char* reason);

#ifdef __cplusplus
}
#endif

#endif /* __BUS_H */
