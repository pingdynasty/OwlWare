#include "eepromcontrol.h"
#include <string.h> /* for memcpy */
#include "stm32f4xx.h"

void eeprom_lock(){
  FLASH_Lock();
}

void eeprom_unlock(){
  FLASH_Unlock();
  /* clear pending flags - important! */
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 
}

uint8_t eeprom_read_byte(uint32_t address){
  while(FLASH_GetStatus() == FLASH_BUSY);
  return *(uint8_t*)address;
}

int eeprom_read_block(uint32_t address, uint8_t* data, uint32_t size){
  while(FLASH_GetStatus() == FLASH_BUSY);
  memcpy(data, (const void*)address, size);
  return FLASH_GetStatus();
}

__attribute__ ((section (".coderam")))
int eeprom_write_block(uint32_t address, uint8_t* data, uint32_t size){
  FLASH_Status status = FLASH_COMPLETE;
  uint32_t* ptr = (uint32_t*)data;
  for(uint32_t i=0; i<size && status == FLASH_COMPLETE; i+=4)
    status = FLASH_ProgramWord(address+i, *ptr++);
  return status == FLASH_COMPLETE ? 0 : -1;
}

__attribute__ ((section (".coderam")))
int eeprom_erase(uint32_t address){
  if(address < ADDR_FLASH_SECTOR_1)
    return -1;  // protect boot sector
    /* FLASH_EraseSector(FLASH_Sector_0, VoltageRange_3); */
  else if(address < ADDR_FLASH_SECTOR_2)
    FLASH_EraseSector(FLASH_Sector_1, VoltageRange_3);
  else if(address < ADDR_FLASH_SECTOR_3)
    FLASH_EraseSector(FLASH_Sector_2, VoltageRange_3);
  else if(address < ADDR_FLASH_SECTOR_4)
    FLASH_EraseSector(FLASH_Sector_3, VoltageRange_3);
  else if(address < ADDR_FLASH_SECTOR_5)
    FLASH_EraseSector(FLASH_Sector_4, VoltageRange_3);
  else if(address < ADDR_FLASH_SECTOR_6)
    FLASH_EraseSector(FLASH_Sector_5, VoltageRange_3);
  else if(address < ADDR_FLASH_SECTOR_7)
    FLASH_EraseSector(FLASH_Sector_6, VoltageRange_3);
  else if(address < ADDR_FLASH_SECTOR_8)
    FLASH_EraseSector(FLASH_Sector_7, VoltageRange_3);
  else if(address < ADDR_FLASH_SECTOR_9)
    FLASH_EraseSector(FLASH_Sector_8, VoltageRange_3);
  else if(address < ADDR_FLASH_SECTOR_10)
    FLASH_EraseSector(FLASH_Sector_9, VoltageRange_3);
  else if(address < ADDR_FLASH_SECTOR_11)
    FLASH_EraseSector(FLASH_Sector_10, VoltageRange_3);
  else if(address < 0x08100000)
    FLASH_EraseSector(FLASH_Sector_11, VoltageRange_3);
  else
    return -1;
  return 0;
}
