
#include "eepromcontrol.h"
#include <string.h> /* for memcpy */
#include "stm32f4xx.h"

/* __attribute__((__section__(".eeprom"), used)) uint16_t eeprom_junk[8192]; // 0x8029e5c */

#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbytes */

   void* eeprom_get_address(uint32_t address){
     return (void*)(ADDR_FLASH_SECTOR_11 + address);
   }

   uint8_t eeprom_read_byte(uint32_t address){
     while(FLASH_GetStatus() == FLASH_BUSY);
     return *(uint8_t*)eeprom_get_address(address);
   }

   int eeprom_read_block(uint32_t address, uint8_t* data, uint32_t size){
     address = (uint32_t)eeprom_get_address(address);
     while(FLASH_GetStatus() == FLASH_BUSY);
/*      for(int i=0; i<size; ++i) */
/*        data[i] = *(uint8_t*)(address+i); */
     memcpy(data, (uint8_t*)address, size);
     return FLASH_GetStatus();
   }

__attribute__ ((section (".coderam")))
   int eeprom_write_block(uint32_t address, uint8_t* data, uint32_t size){
     address = (uint32_t)eeprom_get_address(address);
     FLASH_Unlock();
     FLASH_Status status = FLASH_COMPLETE;
     for(uint32_t i=0; i<size && status == FLASH_COMPLETE; ++i)
       status = FLASH_ProgramByte(address+i, data[i]);
     FLASH_Lock();
     return status;
   }

__attribute__ ((section (".coderam")))
   int eeprom_erase(){
     FLASH_Unlock();
     FLASH_Status status = FLASH_EraseSector(FLASH_Sector_11, VoltageRange_3);
     FLASH_Lock();
   }
