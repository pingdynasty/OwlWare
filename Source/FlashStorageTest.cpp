/*
  g++ -ISource -ILibraries/CMSIS/ST/STM32F4xx/Include/ -I/opt/local/include -L/opt/local/lib Source/FlashStorageTest.cpp -lboost_unit_test_framework -o FlashStorageTest && ./FlashStorageTest
*/
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Test
#include <boost/test/unit_test.hpp>
#include <inttypes.h>

enum FLASH_Status {
  FLASH_COMPLETE,
  FLASH_ERROR
};

#define VoltageRange_3 3
#define FLASH_Sector_0 0

uint8_t flashdata[1024*1024];
#define EEPROM_PAGE_BEGIN            ((uint32_t)flashdata)
#define EEPROM_PAGE_SIZE             (2*1024)
#define EEPROM_PAGE_END              ((uint32_t)(flashdata+sizeof(flashdata)))
#define STORAGE_MAX_BLOCKS           16

#include <iostream>

FLASH_Status FLASH_ProgramByte(uint32_t address, uint8_t data){
  if(address >= EEPROM_PAGE_BEGIN && address+1 < EEPROM_PAGE_END){
    *(uint8_t*)address &= data & 0xff;
    // std::cout << "programmed [" << std::hex << address << "][" << (int)data << "][" << (int)*(uint8_t*)address << "][" << *(uint32_t*)flashdata << "]" << std::endl;
    return FLASH_COMPLETE;
  }else{
    return FLASH_ERROR;
  }
}

FLASH_Status FLASH_ProgramWord(uint32_t address, uint32_t data){
  if(address >= EEPROM_PAGE_BEGIN && address+4 < EEPROM_PAGE_END){
    *(uint8_t*)address++ &= data & 0xff;
    *(uint8_t*)address++ &= (data>>8) & 0xff;
    *(uint8_t*)address++ &= (data>>16) & 0xff;
    *(uint8_t*)address++ &= (data>>24) & 0xff;
    address -= 4;
    // std::cout << "programmed [" << std::hex << address << "][" << data << "][" << *(uint32_t*)address << "][" << *(uint32_t*)flashdata << "]" << std::endl;
    return FLASH_COMPLETE;
  }else{
    return FLASH_ERROR;
  }
}
FLASH_Status FLASH_EraseSector(uint32_t sector, uint8_t VoltageRange){
  if(EEPROM_PAGE_BEGIN+(sector+1)*EEPROM_PAGE_SIZE > EEPROM_PAGE_END)
    return FLASH_ERROR;
  memset((void*)(EEPROM_PAGE_BEGIN+sector*EEPROM_PAGE_SIZE), 0xff, EEPROM_PAGE_SIZE);
  return FLASH_COMPLETE;
}

#define __STM32F4xx_H
#define __device_h__
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
}

BOOST_AUTO_TEST_CASE(testInit){
  memset(flashdata, 0xff, sizeof(flashdata));
  FlashStorage storage;
  storage.init();
  BOOST_CHECK_EQUAL(storage.getBlocksTotal(), 1);
  BOOST_CHECK_EQUAL(storage.getTotalUsedSize(), 4);
  BOOST_CHECK_EQUAL(storage.getDeletedSize(), 0);
  BOOST_CHECK_EQUAL(storage.getTotalAllocatedSize(), 1024*1024);
  BOOST_CHECK_EQUAL(storage.getFreeSize(), 1024*1024-4);
}

BOOST_AUTO_TEST_CASE(testWrite){
  memset(flashdata, 0xff, sizeof(flashdata));
  FlashStorage storage;
  storage.init();
  uint8_t data[555];
  for(int i=0; i<sizeof(data); ++i)
    data[i] = i;
  storage.append(data, sizeof(data));
  uint8_t* store = (uint8_t*)storage.getLastBlock().getData();
  for(int i=0; i<sizeof(data); ++i)
    BOOST_CHECK_EQUAL(data[i], store[i]);
  BOOST_CHECK_EQUAL(storage.getBlocksTotal(), 1);
  BOOST_CHECK_EQUAL(storage.getTotalUsedSize(), sizeof(data)+4);
  BOOST_CHECK_EQUAL(storage.getDeletedSize(), 0);
  BOOST_CHECK_EQUAL(storage.getTotalAllocatedSize(), 1024*1024);
  BOOST_CHECK_EQUAL(storage.getFreeSize(), 1024*1024-sizeof(data)-4);
}
