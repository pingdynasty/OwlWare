#ifndef __StorageBlock_h__
#define __StorageBlock_h__

#include <inttypes.h>

class StorageBlock {
private:
  uint32_t* header; 
  // one byte of the header is used for block magic and deleted status, 
  // 3 bytes for size (24 bits: max 16MB)
public:
  StorageBlock();
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
  bool isValidSize();
  bool verify(); // true if block is not null, is written, has valid size, and is not deleted
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

#endif // __StorageBlock_h__
