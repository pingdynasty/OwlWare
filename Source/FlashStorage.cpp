#include "FlashStorage.h"
#include "device.h"
#include <string.h>
#include "message.h"
#include "eepromcontrol.h"

void FlashStorage::init(){
  uint32_t offset = 0;
  StorageBlock block;
  count = 0;
  do{
    block = createBlock(EEPROM_PAGE_BEGIN, offset);
    blocks[count++] = block;
    offset += block.getBlockSize();
  }while(!block.isFree() && count+1 < STORAGE_MAX_BLOCKS && offset < EEPROM_PAGE_END);
  // fills at least one (possibly empty) block into list
}

#if 0
void FlashStorage::recover(){
  StorageBlock block = getLastBlock();
  if(!block.isFree() && !block.isValidSize()){
    // count backwards to see how much free space (0xff words) there is
    uint32_t* top = (uint32_t*)block.getData();
    uint32_t* ptr = (uint32_t*)(EEPROM_PAGE_END);
    uint32_t free = 0;
    while(--ptr > top && *ptr == 0xffffffff)
      free += 4;
    uint32_t size = (ptr-top)*4;
    // write size to last block, and update magick to deleted
    block.setSize(size);
    block.setDeleted();
    // add empty block to end
    if(count+1 < STORAGE_MAX_BLOCKS)
      blocks[count++] = StorageBlock(ptr+4); // createBlock(ptr+4);
  }
}
#endif

StorageBlock FlashStorage::append(void* data, uint32_t size){
  StorageBlock last = getLastBlock();
  if(last.isFree()){
    last.write(data, size);
    if(count < STORAGE_MAX_BLOCKS)
      blocks[count++] = StorageBlock((uint32_t*)last.getData()+last.getDataSize());
    return last;
  }else{
    if(last.isValidSize()){
      if(count < STORAGE_MAX_BLOCKS){
	// create a new block
	blocks[count++] = StorageBlock((uint32_t*)last.getData()+last.getDataSize());
	return append(data, size);
      }else{
	error(FLASH_ERROR, "No more blocks available");
      }
    }else{
      error(FLASH_ERROR, "Invalid non-empty block");
      // invalid non-empty block
      // probably have to rewrite everything
      // todo!
      /* erase(); */
      /* recover(); */
    }
  }
  // return getLastBlock();
  return StorageBlock();
}

uint8_t FlashStorage::getBlocksVerified(){
  uint8_t nof = 0;
  for(uint8_t i=0; i<count; ++i)
    if(blocks[i].verify())
      nof++;
  return nof;
}

uint32_t FlashStorage::getTotalUsedSize(){
  // returns bytes used by written and deleted blocks
  uint32_t size = 0;
  for(uint8_t i=0; i<count; ++i)
    if(blocks[i].isValidSize())
      size += blocks[i].getBlockSize();
  return size;
}

uint32_t FlashStorage::getDeletedSize(){
  uint32_t size = 0;
  for(uint8_t i=0; i<count; ++i)
    if(blocks[i].isDeleted())
      size += blocks[i].getBlockSize();
  return size;
}

// erase entire allocated FLASH memory
void FlashStorage::erase(){
  uint32_t page = EEPROM_PAGE_BEGIN;
  eeprom_unlock();
  while(page < EEPROM_PAGE_END){
    eeprom_erase(page);
    page += EEPROM_PAGE_SIZE;
  }
  eeprom_lock();
  init();
}

void FlashStorage::defrag(void* buffer, uint32_t size){
  ASSERT(size >= getWrittenSize(), "Insufficient space for full defrag");
  uint8_t* ptr = (uint8_t*)buffer;
  if(getDeletedSize() > 0 && getWrittenSize() > 0){
    uint32_t offset = 0;
    for(uint8_t i=0; i<count && offset<size; ++i){
      if(blocks[i].verify()){
	memcpy(ptr+offset, blocks[i].getBlock(), blocks[i].getBlockSize());
	offset += blocks[i].getBlockSize();
      }
    }
    erase();
    eeprom_unlock();
    eeprom_write_block(EEPROM_PAGE_BEGIN, buffer, offset);
    eeprom_lock();
    init();
  }
}

StorageBlock FlashStorage::createBlock(uint32_t page, uint32_t offset){
  /* if(page+offset+4 >= EEPROM_PAGE_END */
  /*    || page < EEPROM_PAGE_BEGIN) */
  /*   return StorageBlock();       */
  StorageBlock block((uint32_t*)(page+offset));
  return block;
}

FlashStorage storage;
