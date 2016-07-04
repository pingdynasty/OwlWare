#ifndef __EEPROM_CONTROL_H
#define __EEPROM_CONTROL_H

#include <stdint.h>

/* Base address of the Flash sectors */
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbyte */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbyte */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbyte */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbyte */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbyte */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbyte */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbyte */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbyte */
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) /* Base @ of Sector 8, 128 Kbyte */
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) /* Base @ of Sector 9, 128 Kbyte */
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) /* Base @ of Sector 10, 128 Kbyte */
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbyte */

#ifdef __cplusplus
 extern "C" {
#endif

   void eeprom_lock();
   void eeprom_unlock();
   void* eeprom_read(uint32_t address);
   uint8_t eeprom_read_byte(uint32_t address);
   uint32_t eeprom_read_word(uint32_t address);
   int eeprom_read_block(uint32_t address, void* data, uint32_t size);
   int eeprom_write_block(uint32_t address, void* data, uint32_t size);
   int eeprom_write_word(uint32_t address, uint32_t data);
   int eeprom_write_byte(uint32_t address, uint8_t data);
   int eeprom_erase(uint32_t address);

   int eeprom_wait();
   int eeprom_get_status();
   int eeprom_erase_sector(uint32_t sector);

#ifdef __cplusplus
}
#endif

#endif /* __EEPROM_CONTROL_H */
