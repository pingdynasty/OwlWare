#include "StorageBlock.h"
#include "device.h"
#include <string.h>
#include "message.h"
#include "eepromcontrol.h"
// #include "stm32f4xx.h"

StorageBlock::StorageBlock() : header(NULL){} // (uint32_t*)EEPROM_PAGE_BEGIN

uint32_t StorageBlock::getBlockSize(){
  uint32_t size = getDataSize() + 4;
  while(size & 0x03)
    size++; // pad to 4 bytes
  return size;
}

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
  eeprom_unlock();
  eeprom_write_word((uint32_t)header, 0xcfffffff); // mark as used (no size)
  eeprom_write_block((uint32_t)header+4, data, size);
  bool status = eeprom_write_word((uint32_t)header, 0xcf000000 | size); // set magick and size
  eeprom_lock();
  if(status != 0){
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

bool StorageBlock::setDeleted(){
  eeprom_unlock();
  bool status = eeprom_write_byte((uint32_t)header+3, 0xc0);
  // FLASH_Status status = FLASH_ProgramByte((uint32_t)header, 0xc0);
  eeprom_lock();
  if(status != 0){
    error(FLASH_ERROR, "Flash delete failed");
    return false;
  }
  return true;
}
