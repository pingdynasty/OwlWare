#ifndef __FlashStorage_h__
#define __FlashStorage_h__

#include <inttypes.h>
#include <device.h>

class StorageBlock {
private:
  uint32_t* header;
public:
  StorageBlock() : header((uint32_t*)EEPROM_PAGE_BEGIN){}
  StorageBlock(uint32_t* h) : header(h){}
  uint8_t getMagick(){
    // upper 8 bits
    return (*header) >> 24;
  }
  uint32_t getDataSize(){
    // lower 24 bits
    uint32_t size = (*header) & 0x00ffffff;
    if(size == 0x00ffffff)
      return 0;
    return size;
  }
  uint32_t getBlockSize(){
    return getDataSize() + 4;
  }
  void* getData(){
    return (void*)(header+1); // data starts 4 bytes (1 word) after header
  }
  bool isWritten(){
    /* return getMagick() == 0xcf; // (getMagick() & 0xf0) == 0xc0; */
    return (getMagick() & 0xf0) == 0xc0;
  }
  bool isValidSize(){
    return getDataSize() > 0 && ((uint8_t*)header) + getBlockSize() < (uint8_t*)EEPROM_PAGE_END;
    /* getSize() != 0xffffff; // && getSize() != 0x00; */
  }
  bool isFree(){
    return (*header) == 0xffffffff;
/* getMagick() == 0xff && getDataSize() == 0; */
  }
  bool isDeleted(){
    return getMagick() == 0xc0;
  }
  void setDeleted(){
    // write zeros to bottom 4 bits in magick byte
    *header = (*header) & 0xf0ffffff; // set deleted bits to 0
  }
  void setSize(uint32_t size){
    *header = ((*header) & 0xff000000) | (size & 0x00ffffff);
  }
  bool write(void* data, uint32_t size);
};

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
  bool append(void* data, uint32_t size);
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
