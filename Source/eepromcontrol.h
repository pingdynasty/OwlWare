#ifndef __EEPROM_CONTROL_H
#define __EEPROM_CONTROL_H

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

   void eeprom_lock();
   void eeprom_unlock();
   uint8_t eeprom_read_byte(uint32_t address);
   int eeprom_read_block(uint32_t address, uint8_t* data, uint32_t size);
   int eeprom_write_block(uint32_t address, uint8_t* data, uint32_t size);
   int eeprom_erase(uint32_t address);

#ifdef __cplusplus
}
#endif

#endif /* __EEPROM_CONTROL_H */
