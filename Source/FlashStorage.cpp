#include "FlashStorage.h"
#include "device.h"
#include <string.h>
#include "message.h"
#include "stm32f4xx.h"

bool StorageBlock::write(void* data, uint32_t size){
  if((uint32_t)header+4+size >= EEPROM_PAGE_END)
    return false;
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

void FlashStorage::append(void* data, uint32_t size){
  StorageBlock block = getLastBlock();
  if(block.isFree()){
    block.write(data, size);
  }else{
    if(block.isValidSize()){
      if(count < STORAGE_MAX_BLOCKS){
	blocks[count++] = createBlock(EEPROM_PAGE_BEGIN, block.getBlockSize());
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
}

// erase entire allocated FLASH memory
void FlashStorage::erase(){
  uint32_t page = EEPROM_PAGE_BEGIN;
  int sector = FLASH_Sector_0;
  while(page < EEPROM_PAGE_END){
    FLASH_EraseSector(sector++, VoltageRange_3);
    // FLASH_ErasePage(page);
    page += EEPROM_PAGE_SIZE;
  }
}

StorageBlock FlashStorage::createBlock(uint32_t page, uint32_t offset){
  /* if(page+offset+4 >= EEPROM_PAGE_END */
  /*    || page < EEPROM_PAGE_BEGIN) */
  /*   return StorageBlock();       */
  StorageBlock block((uint32_t*)(page+offset));
  return block;
  /* if(block.isValidSize() */
  /* switch(block.getMagick()){ */
  /* case 0xcf: */
  /*   // written, valid block */
  /*   return block; */
  /* case 0xc0: */
  /*   // deleted block */
  /*   if(block.isValidSize()) */
  /* 	return createBlock(page, offset+block.getSize()+4); */
  /*   break; */
  /* case 0xff: */
  /*   // empty space */
  /*   break; */
  /* default: */
  /*   // invalid */
  /*   break; */
  /* } */
  /* return block; */
}

