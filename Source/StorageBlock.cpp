#include "StorageBlock.h"
#include "device.h"
#include <string.h>
#include "message.h"
#include "stm32f4xx.h"

StorageBlock::StorageBlock() : header(NULL){} // (uint32_t*)EEPROM_PAGE_BEGIN

bool StorageBlock::isValidSize(){
  return header != NULL && getDataSize() > 0 && ((uint8_t*)header) + getBlockSize() < (uint8_t*)EEPROM_PAGE_END;
    /* getSize() != 0xffffff; // && getSize() != 0x00; */
}

bool StorageBlock::verify(){
  return isValidSize() && isWritten() && !isDeleted();
}

bool StorageBlock::write(void* data, uint32_t size){
  if((uint32_t)header+4+size >= EEPROM_PAGE_END)
    return false;
  FLASH_Unlock();
  FLASH_Status status = FLASH_ProgramWord((uint32_t)header, 0xCFFFFFFF); // mark as used (no size)
  if(status != FLASH_COMPLETE)
    return false;
  uint32_t address = (uint32_t)header+4;
  uint32_t* p32 = (uint32_t*)data;
  for(uint32_t i=0; i<size/4; i++){
    // write one word (4 bytes) at a time
    FLASH_ProgramWord(address, *p32++);
    address += 4;
  }
  // write any remaining bytes
  uint8_t* p8 = (uint8_t*)p32;
  for(uint32_t i=0; i<size%4; i++)
    FLASH_ProgramByte(address++, *p8++);

  status = FLASH_ProgramWord((uint32_t)header, 0xCF000000 | size); // set magick and size
  FLASH_Lock();

  // verify
  if(status != FLASH_COMPLETE){
    error(FLASH_ERROR, "Flash write failed");
    return false;
  }
  if(size != getDataSize()){
    error(FLASH_ERROR, "Size verification failed");
    return false;
  }
  if(memcmp(data, getData(), size) != 0){
    error(FLASH_ERROR, "Data verification failed");
    return false;
  }
  return true;
}
