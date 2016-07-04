#include "FlashStorage.h"
#include "device.h"
#include <string.h>
#include "message.h"
#include "stm32f4xx.h"

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

// erase entire allocated FLASH memory
void FlashStorage::erase(){
  uint32_t page = EEPROM_PAGE_BEGIN;
  int sector = FLASH_Sector_0;
  FLASH_Unlock();
  while(page < EEPROM_PAGE_END){
    FLASH_EraseSector(sector++, VoltageRange_3);
    // FLASH_ErasePage(page);
    page += EEPROM_PAGE_SIZE;
  }
  FLASH_Lock();
  init();
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

FlashStorage storage;
