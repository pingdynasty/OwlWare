#ifndef __FlashStorage_h__
#define __FlashStorage_h__

#include <inttypes.h>
#include <device.h>
#include "StorageBlock.h"

class FlashStorage {
private:
  StorageBlock blocks[STORAGE_MAX_BLOCKS];
  uint8_t count;
public:
  FlashStorage() : count(0){}
  void init();
  uint8_t getBlocksTotal(){
    return count;
  }
  uint8_t getBlocksWritten(){
    uint8_t nof = 0;
    for(uint8_t i=0; i<count; ++i)
      if(blocks[i].isWritten())
	nof++;
    return nof;
  }
  uint32_t getTotalUsedSize(){
    // returns bytes used by written and deleted blocks
    uint32_t size = 0;
    for(uint8_t i=0; i<count; ++i)
      size += blocks[i].getBlockSize();
    return size;
  }
  uint32_t getDeletedSize(){
    uint32_t size = 0;
    for(uint8_t i=0; i<count; ++i)
      if(blocks[i].isDeleted())
	size += blocks[i].getBlockSize();
    return size;
  }
  uint32_t getTotalAllocatedSize(){
    return EEPROM_PAGE_END - EEPROM_PAGE_BEGIN;
  }
  uint32_t getFreeSize(){
    return getTotalAllocatedSize() - getTotalUsedSize();
  }
  void recover();
  StorageBlock append(void* data, uint32_t size);
  // erase entire allocated FLASH memory
  void erase();
  StorageBlock getLastBlock(){
    return blocks[count > 0 ? count-1 : 0];
  }
  StorageBlock getBlock(uint8_t index){
    if(index < count)
      return blocks[index];
    else
      return getLastBlock();
  }
private:
  StorageBlock createBlock(uint32_t page, uint32_t offset);
};

extern FlashStorage storage;

#endif // __FlashStorage_h__
