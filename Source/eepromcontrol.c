#include "eepromcontrol.h"
#include <string.h> /* for memcpy */
#include "stm32f4xx.h"

#define EEPROM_SECTOR_MASK               ((uint32_t)0xFFFFFF07)

__attribute__ ((section (".coderam")))
int eeprom_get_status(){
  if((FLASH->SR & FLASH_FLAG_BSY) != RESET)
    return FLASH_BUSY;
  else if((FLASH->SR & FLASH_FLAG_WRPERR) != RESET)
    return FLASH_ERROR_WRP;
  else if((FLASH->SR & (uint32_t)0xEF) != RESET)
    return FLASH_ERROR_PROGRAM; 
  else if((FLASH->SR & FLASH_FLAG_OPERR) != RESET)
    return FLASH_ERROR_OPERATION;
  return FLASH_COMPLETE;
}

__attribute__ ((section (".coderam")))
void eeprom_lock(){
  while(eeprom_get_status() == FLASH_BUSY);
  /* Set the LOCK Bit to lock the FLASH Registers access */
  FLASH->CR |= FLASH_CR_LOCK;
}

__attribute__ ((section (".coderam")))
int eeprom_wait(){ 
  volatile FLASH_Status status = FLASH_BUSY;
  /* Wait for the FLASH operation to complete by polling on BUSY flag to be reset.
     Even if the FLASH operation fails, the BUSY flag will be reset and an error
     flag will be set */
  while(status == FLASH_BUSY)
    status = eeprom_get_status();
  /* Return the operation status */
  return status;
}

__attribute__ ((section (".coderam")))
int eeprom_erase_sector(uint32_t sector) {
  /* Wait for last operation to be completed */
  FLASH_Status status = eeprom_wait();
  if(status == FLASH_COMPLETE){ 
    /* if the previous operation is completed, proceed to erase the sector */
    FLASH->CR &= CR_PSIZE_MASK;
    FLASH->CR |= FLASH_PSIZE_WORD;
    FLASH->CR &= EEPROM_SECTOR_MASK;
    FLASH->CR |= FLASH_CR_SER | sector;
    FLASH->CR |= FLASH_CR_STRT;    
    /* Wait for last operation to be completed */
    status = eeprom_wait();
    /* When the erase operation is completed, disable the SER Bit */
    FLASH->CR &= (~FLASH_CR_SER);
    FLASH->CR &= EEPROM_SECTOR_MASK; 
    /* Wait for last operation to be completed */
    status = eeprom_wait();
  }
  /* Return the Erase Status */
  return status;
}

__attribute__ ((section (".coderam")))
int eeprom_write_block(uint32_t address, void* data, uint32_t size){
  volatile uint32_t* src = (uint32_t*)data;
  volatile uint32_t* dest = (volatile uint32_t*)address;
  volatile FLASH_Status status = eeprom_wait();
  for(uint32_t i=0; i<size && status == FLASH_COMPLETE; i+=4){
    /* when the previous operation is completed, proceed to program the new data */
    FLASH->CR &= CR_PSIZE_MASK;
    FLASH->CR |= FLASH_PSIZE_WORD;
    FLASH->CR |= FLASH_CR_PG;
    *dest++ = *src++;
    /* Wait for last operation to be completed */
    status = eeprom_wait();
    /* if the program operation is completed, disable the PG Bit */
    FLASH->CR &= (~FLASH_CR_PG);
    /* Wait for last operation to be completed */
    status = eeprom_wait();
  }
  return status == FLASH_COMPLETE ? 0 : -1;
}

void eeprom_unlock(){
  while(eeprom_get_status() == FLASH_BUSY);
  if((FLASH->CR & FLASH_CR_LOCK) != RESET){
    /* Authorize the FLASH Registers access */
    FLASH->KEYR = FLASH_KEY1;
    FLASH->KEYR = FLASH_KEY2;
  }
  /* clear pending flags - important! */
  FLASH->SR = (FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
	       FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);
}

uint8_t eeprom_read_byte(uint32_t address){
  while(eeprom_get_status() == FLASH_BUSY);
  return *(uint8_t*)address;
}

int eeprom_read_block(uint32_t address, void* data, uint32_t size){
  while(eeprom_get_status() == FLASH_BUSY);
  memcpy(data, (const void*)address, size);
  return eeprom_get_status();
}

int eeprom_erase(uint32_t address){
  int ret = -1;
  if(address < ADDR_FLASH_SECTOR_1)
    ret = -1;  // protect boot sector
    /* eeprom_erase_sector(FLASH_Sector_0, VoltageRange_3); */
  else if(address < ADDR_FLASH_SECTOR_2)
    ret = eeprom_erase_sector(FLASH_Sector_1);
  else if(address < ADDR_FLASH_SECTOR_3)
    ret = eeprom_erase_sector(FLASH_Sector_2);
  else if(address < ADDR_FLASH_SECTOR_4)
    ret = eeprom_erase_sector(FLASH_Sector_3);
  else if(address < ADDR_FLASH_SECTOR_5)
    ret = eeprom_erase_sector(FLASH_Sector_4);
  else if(address < ADDR_FLASH_SECTOR_6)
    ret = eeprom_erase_sector(FLASH_Sector_5);
  else if(address < ADDR_FLASH_SECTOR_7)
    ret = eeprom_erase_sector(FLASH_Sector_6);
  else if(address < ADDR_FLASH_SECTOR_8)
    ret = eeprom_erase_sector(FLASH_Sector_7);
  else if(address < ADDR_FLASH_SECTOR_9)
    ret = eeprom_erase_sector(FLASH_Sector_8);
  else if(address < ADDR_FLASH_SECTOR_10)
    ret = eeprom_erase_sector(FLASH_Sector_9);
  else if(address < ADDR_FLASH_SECTOR_11)
    ret = eeprom_erase_sector(FLASH_Sector_10);
  else if(address < 0x08100000)
    ret = eeprom_erase_sector(FLASH_Sector_11);
  else
    ret = -1;
  return ret;
}
