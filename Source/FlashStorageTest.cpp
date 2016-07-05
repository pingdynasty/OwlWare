/*
  g++ -ISource -ILibraries/CMSIS/ST/STM32F4xx/Include/ -I/opt/local/include -L/opt/local/lib -std=c++11 Source/FlashStorageTest.cpp -lboost_unit_test_framework -o FlashStorageTest && ./FlashStorageTest
*/
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Test
#include <boost/test/unit_test.hpp>
#include <inttypes.h>

enum FLASH_Status {
  FLASH_COMPLETE,
  FLASH_ERROR
};

uint8_t flashdata[1024*1024];
bool flashlock = true;
#define EEPROM_PAGE_BEGIN            ((uint32_t)flashdata)
#define EEPROM_PAGE_SIZE             (128*1024)
#define EEPROM_PAGE_END              ((uint32_t)(flashdata+sizeof(flashdata)))
#define STORAGE_MAX_BLOCKS           16

#include <iostream>
#include "eepromcontrol.h"

void eeprom_lock(){
  flashlock = true;
}

void eeprom_unlock(){
  flashlock = false;
}

int eeprom_write_block(uint32_t address, void* data, uint32_t size){
  uint32_t* p32 = (uint32_t*)data;
  uint32_t i=0; 
  for(;i<size-3; i+=4)
    eeprom_write_word(address+i, *p32++);
  uint8_t* p8 = (uint8_t*)p32;
  for(;i<size; i++)
    eeprom_write_byte(address+i, *p8++);
  return 0;
}

int eeprom_write_byte(uint32_t address, uint8_t data){
  BOOST_CHECK_MESSAGE(!flashlock, "Flash locked");
  if(address >= EEPROM_PAGE_BEGIN && address+1 < EEPROM_PAGE_END){
    *(uint8_t*)address &= data & 0xff;
    // std::cout << "programmed [" << std::hex << address << "][" << (int)data << "][" << (int)*(uint8_t*)address << "][" << *(uint32_t*)flashdata << "]" << std::endl;
    return 0;
  }else{
    return -1;
  }
}

int eeprom_write_word(uint32_t address, uint32_t data){
  BOOST_CHECK_MESSAGE(!flashlock, "Flash locked");
  // BOOST_CHECK_MESSAGE(address & 0x03 == 0, "Invalid block alignment");
  if(address >= EEPROM_PAGE_BEGIN && address+4 < EEPROM_PAGE_END){
    *(uint8_t*)address++ &= data & 0xff;
    *(uint8_t*)address++ &= (data>>8) & 0xff;
    *(uint8_t*)address++ &= (data>>16) & 0xff;
    *(uint8_t*)address++ &= (data>>24) & 0xff;
    address -= 4;
    // std::cout << "programmed [" << std::hex << address << "][" << data << "][" << *(uint32_t*)address << "][" << *(uint32_t*)flashdata << "]" << std::endl;
    return 0;
  }else{
    return -1;
  }
}

int eeprom_erase(uint32_t address){
  BOOST_CHECK_MESSAGE(!flashlock, "Flash locked");
  uint32_t size = EEPROM_PAGE_SIZE;
  BOOST_REQUIRE_MESSAGE(address >= EEPROM_PAGE_BEGIN, "Underflow");
  BOOST_CHECK_MESSAGE(address+size <= EEPROM_PAGE_END, "Overflow");
  memset((void*)address, 0xff, size);
  return 0;
}

#define __device_h__
#include "StorageBlock.cpp"
#include "FlashStorage.cpp"

static int8_t errorstatus = NO_ERROR;
int8_t getErrorStatus(){
  return errorstatus;
}

void error(int8_t err, const char* msg){
  errorstatus = err;
  BOOST_CHECK_MESSAGE(err == NO_ERROR, msg);
}

#define FLOAT_TEST_TOLERANCE 0.00001 // percent tolerance when comparing floats

BOOST_AUTO_TEST_CASE(universeInOrder){
  BOOST_CHECK(2+2 == 4);
  BOOST_CHECK_EQUAL((uint32_t)flashdata&0x03, 0);
}

BOOST_AUTO_TEST_CASE(testInit){
  memset(flashdata, 0xff, sizeof(flashdata));
  FlashStorage storage;
  storage.init();
  BOOST_CHECK_EQUAL(storage.getBlocksTotal(), 0);
  BOOST_CHECK_EQUAL(storage.getTotalUsedSize(), 0);
  BOOST_CHECK_EQUAL(storage.getDeletedSize(), 0);
  BOOST_CHECK_EQUAL(storage.getTotalAllocatedSize(), 1024*1024);
  BOOST_CHECK_EQUAL(storage.getFreeSize(), 1024*1024);
}

BOOST_AUTO_TEST_CASE(testWrite){
  memset(flashdata, 0xff, sizeof(flashdata));
  FlashStorage storage;
  storage.init();
  uint8_t data[555];
  for(int i=0; i<sizeof(data); ++i)
    data[i] = i;
  StorageBlock block = storage.append(data, sizeof(data));
  BOOST_CHECK_EQUAL(block.getDataSize(), sizeof(data));
  uint8_t* store = (uint8_t*)block.getData();
  for(int i=0; i<sizeof(data); ++i)
    BOOST_CHECK_EQUAL(data[i], store[i]);
  int expectedBlockSize = sizeof(data)+4+(4-sizeof(data)%4);
  BOOST_CHECK_EQUAL(storage.getBlocksTotal(), 1);
  BOOST_CHECK_EQUAL(storage.getTotalUsedSize(), expectedBlockSize);
  BOOST_CHECK_EQUAL(storage.getDeletedSize(), 0);
  BOOST_CHECK_EQUAL(storage.getTotalAllocatedSize(), 1024*1024);
  BOOST_CHECK_EQUAL(storage.getFreeSize(), 1024*1024-expectedBlockSize);
}

BOOST_AUTO_TEST_CASE(testErase){
  FlashStorage storage;
  storage.erase();
  for(int i=0; i<sizeof(flashdata); ++i)
    BOOST_CHECK_EQUAL(flashdata[i], 0xff);
  BOOST_CHECK_EQUAL(storage.getBlocksTotal(), 0);
  BOOST_CHECK_EQUAL(storage.getTotalUsedSize(), 0);
  BOOST_CHECK_EQUAL(storage.getDeletedSize(), 0);
  BOOST_CHECK_EQUAL(storage.getTotalAllocatedSize(), 1024*1024);
  BOOST_CHECK_EQUAL(storage.getFreeSize(), 1024*1024);
}

BOOST_AUTO_TEST_CASE(testDelete){
  memset(flashdata, 0xff, sizeof(flashdata));
  FlashStorage storage;
  storage.init();
  uint8_t data[333];
  int expectedBlockSize = sizeof(data)+4+(4-sizeof(data)%4);
  for(int i=0; i<sizeof(data); ++i)
    data[i] = i;
  StorageBlock b1 = storage.append(data, sizeof(data));
  StorageBlock b2 = storage.append(data, sizeof(data));
  StorageBlock b3 = storage.append(data, sizeof(data));
  BOOST_CHECK_EQUAL(storage.getBlocksTotal(), 3);
  BOOST_CHECK_EQUAL(storage.getTotalUsedSize(), expectedBlockSize*3);
  BOOST_CHECK_EQUAL(storage.getDeletedSize(), 0);
  BOOST_CHECK_EQUAL(storage.getWrittenSize(), expectedBlockSize*3);
  BOOST_CHECK_EQUAL(b1.getBlockSize(), expectedBlockSize);
  BOOST_CHECK_EQUAL(b1.getDataSize(), sizeof(data));
  BOOST_CHECK(!b1.isDeleted());
  BOOST_CHECK(b1.verify());
  b1.setDeleted();
  BOOST_CHECK_EQUAL(storage.getBlocksTotal(), 3);
  BOOST_CHECK_EQUAL(storage.getTotalUsedSize(), expectedBlockSize*3);
  BOOST_CHECK_EQUAL(storage.getDeletedSize(), expectedBlockSize);
  BOOST_CHECK_EQUAL(storage.getWrittenSize(), expectedBlockSize*2);
  BOOST_CHECK_EQUAL(storage.getFreeSize(), 1024*1024-expectedBlockSize*3);
  BOOST_CHECK_EQUAL(b1.getBlockSize(), expectedBlockSize);
  BOOST_CHECK_EQUAL(b1.getDataSize(), sizeof(data));
  BOOST_CHECK(b1.isDeleted());
  BOOST_CHECK(!b1.verify());
}

BOOST_AUTO_TEST_CASE(testDefrag){
  memset(flashdata, 0xff, sizeof(flashdata));
  FlashStorage storage;
  storage.init();
  uint8_t data[29];
  int expectedBlockSize = sizeof(data)+4+(4-sizeof(data)%4);
  for(int i=0; i<sizeof(data); ++i)
    data[i] = i;
  StorageBlock b1 = storage.append(data, sizeof(data));
  StorageBlock b2 = storage.append(data, sizeof(data));
  StorageBlock b3 = storage.append(data, sizeof(data));
  StorageBlock b4 = storage.append(data, sizeof(data));
  StorageBlock b5 = storage.append(data, sizeof(data));
  StorageBlock b6 = storage.append(data, sizeof(data));
  StorageBlock b7 = storage.append(data, sizeof(data));
  b2.setDeleted();
  b5.setDeleted();
  b4.setDeleted();
  BOOST_CHECK_EQUAL(storage.getTotalUsedSize(), expectedBlockSize*7);
  BOOST_CHECK_EQUAL(storage.getFreeSize(), 1024*1024-expectedBlockSize*7);
  BOOST_CHECK_EQUAL(storage.getDeletedSize(), expectedBlockSize*3);
  BOOST_CHECK_EQUAL(storage.getBlocksVerified(), 4);
  BOOST_CHECK_EQUAL(storage.getBlocksTotal(), 7);
  uint8_t* defragbuffer = new uint8_t[storage.getTotalUsedSize()];
  storage.defrag(defragbuffer, storage.getTotalUsedSize());
  BOOST_CHECK_EQUAL(storage.getTotalUsedSize(), expectedBlockSize*4);
  BOOST_CHECK_EQUAL(storage.getFreeSize(), 1024*1024-expectedBlockSize*4);
  BOOST_CHECK_EQUAL(storage.getDeletedSize(), 0);
  BOOST_CHECK_EQUAL(storage.getBlocksVerified(), 4);
  BOOST_CHECK_EQUAL(storage.getBlocksTotal(), 4);  
}
