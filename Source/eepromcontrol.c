
#include "eepromcontrol.h"
#include <string.h> /* for memcpy */
#include "stm32f4xx.h"

/* __attribute__((__section__(".eeprom"), used)) uint16_t eeprom_junk[8192]; // 0x8029e5c */

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
