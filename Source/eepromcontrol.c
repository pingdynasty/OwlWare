
#include "eepromcontrol.h"
#include <string.h> /* for memcpy */
#include "stm32f4xx.h"

/* __attribute__((__section__(".eeprom"), used)) uint16_t eeprom_junk[8192];  */

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

#define EEPROM_FLASH_ADDRESS    ADDR_FLASH_SECTOR_1
#define EEPROM_FLASH_SECTOR     FLASH_Sector_1

void eeprom_lock(){
  FLASH_Lock();
}

void eeprom_unlock(){
  FLASH_Unlock();
  /* clear pending flags - important! */
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 
}

void* eeprom_get_address(uint32_t address){
  return (void*)(EEPROM_FLASH_ADDRESS + address);
}

uint8_t eeprom_read_byte(uint32_t address){
  while(FLASH_GetStatus() == FLASH_BUSY);
  return *(uint8_t*)eeprom_get_address(address);
}

int eeprom_read_block(uint32_t address, uint8_t* data, uint32_t size){
  address += EEPROM_FLASH_ADDRESS;
  /* address = (uint32_t)eeprom_get_address(address); */
  while(FLASH_GetStatus() == FLASH_BUSY);
  /*      for(int i=0; i<size; ++i) */
  /*        data[i] = *(uint8_t*)(address+i); */
  memcpy(data, (const void*)address, size);
  return FLASH_GetStatus();
}

int eeprom_write_block(uint32_t address, uint8_t* data, uint32_t size){
  /* address = (uint32_t)eeprom_get_address(address); */
  address += EEPROM_FLASH_ADDRESS;
  FLASH_Status status = FLASH_COMPLETE;
  uint32_t* ptr = (uint32_t*)data;
  for(uint16_t i=0; i<size && status == FLASH_COMPLETE; i+=4)
    status = FLASH_ProgramWord(address+i, *ptr++);
  return status;
}

int eeprom_erase(){
  return FLASH_EraseSector(EEPROM_FLASH_SECTOR, VoltageRange_3);
}
