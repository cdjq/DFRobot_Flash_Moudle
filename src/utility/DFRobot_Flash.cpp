/*!
 * @file DFRobot_Flash.cpp
 * @brief 定义 DFRobot_Flash 类 和 DFRobot_FlashFile 类 的实现
 * @details DFRobot_Flash类用于获取Flash Memory Moudle模块的fat类型，磁盘大小，以及复位整个模块
 * @n DFRobot_FlashFile 类用于组织文件和目录的相关操作
 * @copyright	Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author [Arya](xue.peng@dfrobot.com)
 * @version  V1.0
 * @date  2021-04-28
 * @url https://github.com/DFRobot/DFRobot_Flash_Moudle
 */
#ifndef FLASH_DBG
#if 0
#define FLASH_DBG(...) {Serial.print("["); Serial.print(__FUNCTION__); Serial.print("(): "); Serial.print(__LINE__); Serial.print(" ] "); Serial.println(__VA_ARGS__);}
#else
#define FLASH_DBG(...)
#endif
#endif

#include <Arduino.h>
#include "DFRobot_Flash.h"

#define STATE_FAILED    0x63
#define STATE_SUCESS    0x53

#define INVAILD_ID      0xFF

#define TYPE_FAT_FILE_CLOSED   0                    ///< This DFRobot_FlashFile has not been opened.
#define TYPE_FAT_FILE_NORMAL   1                    ///< DFRobot_FlashFile for a file
#define TYPE_FAT_FILE_ROOT12   2                    ///< DFRobot_FlashFile for a FAT12 root directory
#define TYPE_FAT_FILE_ROOT16   3                    ///< DFRobot_FlashFile for a FAT16 root directory
#define TYPE_FAT_FILE_ROOT32   4                    ///< DFRobot_FlashFile for a FAT32 root directory
#define TYPE_FAT_FILE_SUBDIR   5                    ///< DFRobot_FlashFile for a subdirectory
#define TYPE_FAT_FILE_MIN_DIR  TYPE_FAT_FILE_ROOT12 ///< Test value for directory type

#define TYPE_FAT12      12
#define TYPE_FAT16      16
#define TYPE_FAT32      32

#define AUTH_O_READ     0x01
#define AUTH_O_WRITE    0x02
#define AUTH_O_RDWR     (AUTH_O_READ | AUTH_O_WRITE)


DFRobot_Flash::DFRobot_Flash()
  : _capacity(0),_freeSpace(0),_fatType(0),_fileNums(0){}

DFRobot_Flash::~DFRobot_Flash(){

}

uint8_t DFRobot_Flash::init(DFRobot_Driver *drv){
    if(!_pro.begin(drv)){
      FLASH_DBG("_pro init failed, Error: (1 << 4)");
      return (1 << 4);
    }
    if(!_pro.reset()){
        FLASH_DBG("RESET FAILED. Error: (2 << 4)");
        return (2 << 4);
    }
    //delay(2000);//复位后1s内不再接收数据

    if(!_pro.getFlashInfo(&_fatType, &_capacity, &_freeSpace, &_fileNums)){
      FLASH_DBG("get flash info cmd pakage failed! Error: (3 << 4)");
      return (3 << 4);
    }
    FLASH_DBG("init sucess.");
    return 0;
}

bool DFRobot_Flash::reset(){
    return _pro.reset();
}

uint32_t DFRobot_Flash::size(){
  return _capacity;
}

uint8_t DFRobot_Flash::fatType(){
  return _fatType;
}

DFRobot_FlashFile::DFRobot_FlashFile()
  :_flash(NULL), _id(INVAILD_ID), _curPosition(0), _size(0), _authority(0), _type(TYPE_FAT_FILE_CLOSED),_fileSizes(0){}

DFRobot_FlashFile::~DFRobot_FlashFile(){

}



uint8_t DFRobot_FlashFile::openRoot(DFRobot_Flash *flash){
    if(isOpen()){
        FLASH_DBG("root is already open, error: 1");
        return 1;
    }
    if(!flash->_pro.openDirectory((char *)"/", -1, &_id)){ //打开根目录
      FLASH_DBG("root dir open failed, error: 2");
      return 2;
    }
    
    if(flash->fatType() == TYPE_FAT12){
        _type = TYPE_FAT_FILE_ROOT12;
        //_fileSizes = 32 * 
    }else if(flash->fatType() == TYPE_FAT16){
        _type = TYPE_FAT_FILE_ROOT16;
    }else if(flash->fatType() == TYPE_FAT32){
        _type = TYPE_FAT_FILE_ROOT32;
    }else{
        FLASH_DBG("Invaild fat type, error: 2");
        return 3;
    }
    _flash = flash;
    _authority = AUTH_O_READ;
    return 0;
}


bool DFRobot_FlashFile::open(DFRobot_FlashFile* dirFile, const char* fileName, uint8_t oflag){//有可能打开的是目录，也有可能打开的是文件
    //判断文件是否已经打开
    if(isOpen()) return false;
    _flash = dirFile->_flash;
    if(_flash == NULL){
        FLASH_DBG("_flash is null\n");
        return false;
    }
    _authority = oflag;
    if(oflag == AUTH_O_READ){//查询是文件还是目录
      uint8_t type = _flash->_pro.getFileAttribute(dirFile->_id, (char *)fileName);
      if((type == 0) || (type > TYPE_FAT_FILE_SUBDIR)){
        FLASH_DBG("check failed.");
        return false;
      }
      _type = type;
      if(type != TYPE_FAT_FILE_NORMAL){
        if(!_flash->_pro.openDirectory((char *)fileName, dirFile->_id, &_id)){
          FLASH_DBG("open dir failed!");
          return false;
        }
        return true;
      }
    }
    if(!_flash->_pro.openFile((char *)fileName, dirFile->_id, oflag, &_id, &_curPosition, &_size)){
        FLASH_DBG("get open file cmd pakage failed! Error: (3 << 4)");
        return false;
    }

    _type = TYPE_FAT_FILE_NORMAL;
    return true;
}

bool DFRobot_FlashFile::close(bool truncate){//无法关闭根目录
    if(!isOpen() || isRoot()) {
      FLASH_DBG("is not open or root dir");
      return false;
    }

    if(_type == TYPE_FAT_FILE_NORMAL){
      _flash->_pro.sync(_id);
      if(!_flash->_pro.closeFile(_id, truncate)){
        return false;
      } 
    }else{
      if(!_flash->_pro.closeDirectory(_id)){
        return false;
      } 
    }
    
    _type = TYPE_FAT_FILE_CLOSED;
    return true;
}

bool DFRobot_FlashFile::isOpen(){
    return _type != TYPE_FAT_FILE_CLOSED;
}

size_t DFRobot_FlashFile::write(const void* buf, uint16_t nbyte){
    if(!isFile() || !(_authority & AUTH_O_WRITE)) return 0;
    uint16_t t = _flash->_pro.writeFile(_id, (void *)buf, nbyte);
     _curPosition += t;
    _size  = _size > _curPosition ? _size : _curPosition;
    return t;
}

int16_t DFRobot_FlashFile::read(void){
    uint8_t b;
    return read(&b, 1) == 1 ? b : -1;
}

int16_t DFRobot_FlashFile::read(void* buf, uint16_t nbyte){
    if (!isOpen() || !(_authority & AUTH_O_READ)) return -1;
    //计算一个文件能存储的最大字节数，假如为4
    uint16_t  t = _flash->_pro.readFile(_id, buf, nbyte);
    _curPosition += t;
    if(t == 0) return -1;
    return  (int16_t)t;
}

uint8_t DFRobot_FlashFile::sync(void){
    if(!isOpen()) return false;
    return _flash->_pro.sync(_id);
}

uint32_t DFRobot_FlashFile::fileSize(void) {return _size;}

uint32_t DFRobot_FlashFile::curPosition(void) {return _curPosition;}

uint8_t DFRobot_FlashFile::seekSet(uint32_t pos){
    if(!isFile() || !isOpen() || (pos > _size)) return false;
    if(!_flash->_pro.seekFile(_id, pos)){
      return false;
    }
    _curPosition = pos;
    return true;
}

int8_t DFRobot_FlashFile::readDir(char *name,uint16_t size){
  if(!isDir() || (name == NULL)) return -1;
  if(_flash->_pro.readDirectory(_id, name, size)) return 0;
  return -1;
}

bool DFRobot_FlashFile::isFile(void) {return _type == TYPE_FAT_FILE_NORMAL;}
bool DFRobot_FlashFile::isDir(void) {return _type >= TYPE_FAT_FILE_MIN_DIR;}
bool DFRobot_FlashFile::isRoot(void) {return _type == TYPE_FAT_FILE_ROOT16 || _type == TYPE_FAT_FILE_ROOT32 || _type == TYPE_FAT_FILE_ROOT12;}
uint8_t DFRobot_FlashFile::seekCur(uint32_t pos) {return seekSet(_curPosition + pos);}


bool DFRobot_FlashFile::makeDir(const char* dirName){
  return _flash->_pro.newDirectory(dirName, _id);
}

void DFRobot_FlashFile::rewind(void){
  if(!isOpen()) {
    FLASH_DBG("is not open");
    return;
  }
  
  if(!_flash->_pro.rewind(_id)){
    FLASH_DBG("rewind dir is failed.");
  }
}

uint8_t DFRobot_FlashFile::remove(const char* fileName){
  return _flash->_pro.remove(_id, (char *)fileName);
}

bool DFRobot_FlashFile::exists(const char* fileName){
  uint8_t ret = _flash->_pro.getFileAttribute(_id, (char *)fileName);
  return (ret != 0);
}

String DFRobot_FlashFile::absolutePath(){
  return _flash->_pro.getAbsolutePath(_id, _type);
}
String DFRobot_FlashFile::parentDirectory(){
  return _flash->_pro.getParentDirectory(_id, _type);
}


/*
boolean DFRobot_FlashFile::del(uint32_t pos, uint32_t num, bool flag){
  if(!isFile() || !isOpen() || (pos > _size)) return false;
  if(flag){
     if(pos < num) return false;
  }else{
     if(_size - pos < num) return false;
  }
  if(!delCommand(_id, pos, num, flag)){
    return false;
  }
  if(flag) _curPosition = pos - num;
  else _curPosition = pos;
  _size -= num;
  return true;
}
boolean DFRobot_FlashFile::insert(uint32_t pos, uint8_t c, uint32_t num){
  //
  if(!isFile() || !isOpen() || (pos > _size)) return false;
  if(!insertOneCommand(_id, pos, c, num)){
    return false;
  }
  _curPosition = pos + num;
  _size += num;
  return true;
}
boolean DFRobot_FlashFile::insert(uint32_t pos, void *buf, uint32_t len){
  //
  if(!isFile() || !isOpen() || (pos > _size)) return false;
  if(!insertCommand(_id, pos, buf, len)){
    return false;
  }
  _curPosition = pos + len;
  _size += len;
  return true;                                                                                                                     
}
*/


