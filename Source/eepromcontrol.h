#ifndef __EEPROM_CONTROL_H
#define __EEPROM_CONTROL_H

#include <string.h> /* for memcpy */
#include "stm32f4xx.h"

#ifdef __cplusplus
 extern "C" {
#endif

/* __attribute__((__section__(".eeprom"), used)) uint16_t eeprom_junk[8192]; // 0x8029e5c */

#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbytes */

   void* eeprom_get_address(uint32_t address){
     return (void*)(ADDR_FLASH_SECTOR_11 + address);
   }

   uint8_t eeprom_read_byte(uint32_t address){
     return *(uint8_t*)eeprom_get_address(address);
   }

   void eeprom_read_block(uint32_t address, uint8_t* data, uint32_t size){
     address = (uint32_t)eeprom_get_address(address);
/*      for(int i=0; i<size; ++i) */
/*        data[i] = *(uint8_t*)(address+i); */
     memcpy(data, (uint8_t*)address, size);
   }

   void eeprom_write_block(uint32_t address, uint8_t* data, uint32_t size){
     address = (uint32_t)eeprom_get_address(address);
     FLASH_Unlock();
     for(uint32_t i=0; i<size; ++i)
       while(FLASH_ProgramByte(address+i, data[i]) == FLASH_BUSY);
     FLASH_Lock();
   }

#ifdef __cplusplus
}
#endif

#endif /* __EEPROM_CONTROL_H */
