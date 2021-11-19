/*!
 * @file DFRobot_FatCmd.h
 * @brief 定义DFRobot_FatCmd 类的基础结构
 * @details 这是自定义的主机和从机通信的命令包集，通过这些功能函数能够得到发送的命令包具体内容，包括：
 * @n 获取读地址的命令包内容；
 * @n 获取修改地址的命令包内容；
 * @n 获取当前工作目录的命令包内容；
 * @n 获取返回根目录的命令包内容
 * @n 获取切换工作目录的命令包内容
 * @copyright	Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author [Arya](xue.peng@dfrobot.com)
 * @version  V1.0
 * @date  2021-10-09
 * @url https://github.com/DFRobot/DFRobot_
 */
#ifndef __DFROBOT_FLASH_H
#define __DFROBOT_FLASH_H

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include <Wire.h>

#include "DFRobot_FatCmd.h"
#include "DFRobot_Driver.h"

///< Define DBG, change 0 to 1 open the DBG, 1 to 0 to close.  
#if 0
#define FLASH_DBG(...) {Serial.print("["); Serial.print(__FUNCTION__); Serial.print("(): "); Serial.print(__LINE__); Serial.print(" ] "); Serial.println(__VA_ARGS__);}
#else
#define FLASH_DBG(...)
#endif


class DFRobot_Flash{
public:
  typedef enum{
    eTypeFat12 = 12,
    eTypeFat16 = 16,
    eTypeFat32 = 32,
  }eFatType_t;

  DFRobot_Flash();
  ~DFRobot_Flash();
  uint8_t init(DFRobot_Driver *drv);
  uint32_t size();
  uint8_t fatType();
  bool reset();

private:
  friend class DFRobot_FlashFile; //Allow DFRobot_FlashFile access to DFRobot_Flash private data.
  
  uint32_t _capacity; ///< flash 容量， 单位字节
  uint32_t _freeSpace; ///< 空闲空间， 单位字节
  uint8_t _fatType; ///< fat文件系统类型， 0：FAT2  1：FAT16  2: FAT32
  uint16_t _fileNums; ///< 能够存储的最大的文件数
};


class DFRobot_FlashFile{


public:
  DFRobot_FlashFile();
  ~DFRobot_FlashFile();
  uint8_t openRoot(DFRobot_Flash &flash){ return openRoot(&flash);}
  uint8_t openRoot(DFRobot_Flash *flash);
  bool open(DFRobot_FlashFile& dirFile, const char* fileName, uint8_t oflag) {return open(&dirFile, fileName, oflag);}
  bool open(DFRobot_FlashFile* dirFile, const char* fileName, uint8_t oflag);
  bool isOpen(void);
  bool close();
  size_t write(const void* buf, uint16_t nbyte);
  int16_t read(void);
  int16_t read(void* buf, uint16_t nbyte);
  uint8_t sync(void);
  int8_t readDir(char *name);
  uint32_t curPosition(void);
  uint32_t fileSize(void);
  uint8_t seekSet(uint32_t pos);
  bool isFile(void);
  bool isDir(void);
  bool isRoot(void);
  uint8_t seekCur(uint32_t pos);
  bool makeDir(DFRobot_FlashFile& dir, const char* dirName);
  bool makeDir(DFRobot_FlashFile* dir, const char* dirName);
  void rewind(void);
  bool reRootDir(void);
  uint8_t remove(DFRobot_FlashFile& dirFile, const char* fileName);
  uint8_t remove(DFRobot_FlashFile* dirFile, const char* fileName);
  bool changeDir(bool absolute, const char* fileName);
  boolean del(uint32_t pos, uint32_t num, bool flag);
  boolean insert(uint32_t pos, uint8_t c, uint32_t num);
  boolean insert(uint32_t pos, void *buf, uint32_t len);
  String getWorkspacePath();
protected:
  
  
private:
  DFRobot_Flash *_flash;
  int8_t _id;
  uint32_t _curPosition;
  uint32_t _size;
  uint8_t _authority; ///< 打开权限
  uint8_t _type;
  uint8_t _fileSizes;
};


#endif
