#include "eepromcontrol.h"
#include <string.h> /* for memcpy */
#include "stm32f4xx.h"

#define EEPROM_SECTOR_MASK               ((uint32_t)0xFFFFFF07)

void eeprom_unlock(){
  if((FLASH->CR & FLASH_CR_LOCK) != RESET){
    /* Authorize the FLASH Registers access */
    FLASH->KEYR = FLASH_KEY1;
    FLASH->KEYR = FLASH_KEY2;
  }
  /* clear pending flags - important! */
  FLASH->SR = (FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
	       FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);
}

__attribute__ ((section (".coderam")))
FLASH_Status eeprom_get_status(void){
  FLASH_Status flashstatus = FLASH_COMPLETE;  
  if((FLASH->SR & FLASH_FLAG_BSY) == FLASH_FLAG_BSY){
    flashstatus = FLASH_BUSY;
  }else{  
    if((FLASH->SR & FLASH_FLAG_WRPERR) != (uint32_t)0x00){ 
      flashstatus = FLASH_ERROR_WRP;
    }else{
      if((FLASH->SR & (uint32_t)0xEF) != (uint32_t)0x00){
	flashstatus = FLASH_ERROR_PROGRAM; 
      }else{
	if((FLASH->SR & FLASH_FLAG_OPERR) != (uint32_t)0x00){
	  flashstatus = FLASH_ERROR_OPERATION;
	}else{
	  flashstatus = FLASH_COMPLETE;
	}
      }
    }
  }
  /* Return the FLASH Status */
  return flashstatus;
}

uint8_t eeprom_read_byte(uint32_t address){
  while(eeprom_get_status() == FLASH_BUSY);
  return *(uint8_t*)address;
}

int eeprom_read_block(uint32_t address, uint8_t* data, uint32_t size){
  while(eeprom_get_status() == FLASH_BUSY);
  memcpy(data, (const void*)address, size);
  return eeprom_get_status();
}

__attribute__ ((section (".coderam")))
void eeprom_lock(){
  /* Set the LOCK Bit to lock the FLASH Registers access */
  FLASH->CR |= FLASH_CR_LOCK;
}

__attribute__ ((section (".coderam")))
FLASH_Status eeprom_wait(void){ 
  __IO FLASH_Status status = FLASH_COMPLETE;   
  /* Check for the FLASH Status */
  status = eeprom_get_status();
  /* Wait for the FLASH operation to complete by polling on BUSY flag to be reset.
     Even if the FLASH operation fails, the BUSY flag will be reset and an error
     flag will be set */
  while(status == FLASH_BUSY)
    status = eeprom_get_status();
  /* Return the operation status */
  return status;
}

__attribute__ ((section (".coderam")))
int eeprom_write_block(uint32_t address, uint8_t* data, uint32_t size){
  uint32_t* ptr = (uint32_t*)data;
  FLASH_Status status = eeprom_wait();  
  for(uint32_t i=0; i<size && status == FLASH_COMPLETE; i+=4){
    /* when the previous operation is completed, proceed to program the new data */
    FLASH->CR &= CR_PSIZE_MASK;
    FLASH->CR |= FLASH_PSIZE_WORD;
    FLASH->CR |= FLASH_CR_PG;
    *(volatile uint32_t*)(address+i) = *ptr++;
    /* Wait for last operation to be completed */
    status = eeprom_wait();
    /* if the program operation is completed, disable the PG Bit */
    FLASH->CR &= (~FLASH_CR_PG);
    /* Wait for last operation to be completed */
    status = eeprom_wait();
  }
    /* status = FLASH_ProgramWord(address+i, *ptr++); */
  return status == FLASH_COMPLETE ? 0 : -1;
}

__attribute__ ((section (".coderam")))
FLASH_Status eeprom_erase_sector(uint32_t FLASH_Sector, uint8_t VoltageRange) {
  uint32_t tmp_psize = 0x0;
  FLASH_Status status = FLASH_COMPLETE;
  if(VoltageRange == VoltageRange_1){
     tmp_psize = FLASH_PSIZE_BYTE;
  }else if(VoltageRange == VoltageRange_2){
    tmp_psize = FLASH_PSIZE_HALF_WORD;
  }else if(VoltageRange == VoltageRange_3){
    tmp_psize = FLASH_PSIZE_WORD;
  }else{
    tmp_psize = FLASH_PSIZE_DOUBLE_WORD;
  }
  /* Wait for last operation to be completed */
  status = eeprom_wait();
  if(status == FLASH_COMPLETE){ 
    /* if the previous operation is completed, proceed to erase the sector */
    FLASH->CR &= CR_PSIZE_MASK;
    FLASH->CR |= tmp_psize;
    FLASH->CR &= EEPROM_SECTOR_MASK;
    FLASH->CR |= FLASH_CR_SER | FLASH_Sector;
    FLASH->CR |= FLASH_CR_STRT;    
    /* Wait for last operation to be completed */
    status = eeprom_wait();    
    /* if the erase operation is completed, disable the SER Bit */
    FLASH->CR &= (~FLASH_CR_SER);
    FLASH->CR &= EEPROM_SECTOR_MASK; 
  }
  /* Return the Erase Status */
  return status;
}

__attribute__ ((section (".coderam")))
int eeprom_erase(uint32_t address){
  int ret = -1;
  if(address < ADDR_FLASH_SECTOR_1)
    ret = -1;  // protect boot sector
    /* eeprom_erase_sector(FLASH_Sector_0, VoltageRange_3); */
  else if(address < ADDR_FLASH_SECTOR_2)
    ret = eeprom_erase_sector(FLASH_Sector_1, VoltageRange_3);
  else if(address < ADDR_FLASH_SECTOR_3)
    ret = eeprom_erase_sector(FLASH_Sector_2, VoltageRange_3);
  else if(address < ADDR_FLASH_SECTOR_4)
    ret = eeprom_erase_sector(FLASH_Sector_3, VoltageRange_3);
  else if(address < ADDR_FLASH_SECTOR_5)
    ret = eeprom_erase_sector(FLASH_Sector_4, VoltageRange_3);
  else if(address < ADDR_FLASH_SECTOR_6)
    ret = eeprom_erase_sector(FLASH_Sector_5, VoltageRange_3);
  else if(address < ADDR_FLASH_SECTOR_7)
    ret = eeprom_erase_sector(FLASH_Sector_6, VoltageRange_3);
  else if(address < ADDR_FLASH_SECTOR_8)
    ret = eeprom_erase_sector(FLASH_Sector_7, VoltageRange_3);
  else if(address < ADDR_FLASH_SECTOR_9)
    ret = eeprom_erase_sector(FLASH_Sector_8, VoltageRange_3);
  else if(address < ADDR_FLASH_SECTOR_10)
    ret = eeprom_erase_sector(FLASH_Sector_9, VoltageRange_3);
  else if(address < ADDR_FLASH_SECTOR_11)
    ret = eeprom_erase_sector(FLASH_Sector_10, VoltageRange_3);
  else if(address < 0x08100000)
    ret = eeprom_erase_sector(FLASH_Sector_11, VoltageRange_3);
  else
    ret = -1;
  return ret;
}
