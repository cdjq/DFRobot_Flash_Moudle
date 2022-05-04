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
  DFRobot_DFR0870_Protocol _pro;

private:
  friend class DFRobot_FlashFile; //Allow DFRobot_FlashFile access to DFRobot_Flash private data.
  
  uint32_t _capacity; ///< flash 容量， 单位字节
  uint32_t _freeSpace; ///< 空闲空间， 单位字节
  uint8_t _fatType; ///< fat文件系统类型， 0：FAT2  1：FAT16  2: FAT32
  uint16_t _fileNums; ///< 能够存储的最大的文件数
};


class DFRobot_FlashFile{
public:
 /**
  * @fn DFRobot_FlashFile
  * @brief 空构造函数.
  */
  DFRobot_FlashFile();
 /**
  * @fn DFRobot_FlashFile
  * @brief 析构函数.
  */
  ~DFRobot_FlashFile();
  /**
   * @fn openRoot
   * @brief 打开根目录，根目录只能被打开一次
   * @param flash DFRobot_Flash类对象引用
   * @return 返回打开结果
   * @retval 0 打开成功
   * @retval others 打开失败
   */
  uint8_t openRoot(DFRobot_Flash &flash){ return openRoot(&flash);}
  uint8_t openRoot(DFRobot_Flash *flash);
  /**
   * @fn open
   * @brief 打开文件或目录
   * @param dirFile DFRobot_Flash类对象，父级目录
   * @param fileName 文件名
   * @param oflag 打开权限
   * @return 返回打开结果
   * @retval true  打开成功
   * @retval false 打开失败
   */
  bool open(DFRobot_FlashFile& dirFile, const char* fileName, uint8_t oflag) {return open(&dirFile, fileName, oflag);}
  bool open(DFRobot_FlashFile* dirFile, const char* fileName, uint8_t oflag);
  /**
   * @fn isOpen
   * @brief 判断文件或目录是否打开
   * @return 返回判断结果
   * @retval true  文件或目录打开
   * @retval false 文件或目录未打开
   */
  bool isOpen(void);
  /**
   * @fn close
   * @brief 关闭文件或目录，注意，根目录一经打开，则不会再关闭
   * @return 返回关闭结果
   * @retval true  文件或目录关闭成功
   * @retval false 文件或目录关闭失败
   */
  bool close();
  /**
   * @fn write
   * @brief 向文件中写数据
   * @param buf   指向要写入数据的缓存指针
   * @param nbyte 写入数据的大小，单位字节
   * @return 返回实际写入数据的大小
   */
  size_t write(const void* buf, uint16_t nbyte);
  /**
   * @fn read
   * @brief 向文件中读取一个数据
   * @return 读取的数据
   */
  int16_t read(void);
  /**
   * @fn read
   * @brief 从文件中读取数据
   * @param buf   指向数据的缓存指针
   * @param nbyte 读取数据的大小，单位字节
   * @return 返回实际读取数据的大小
   */
  int16_t read(void* buf, uint16_t nbyte);
  /**
   * @fn sync
   * @brief 同步文件内容
   * @return 同步结果
   * @retval 1 同步成功
   * @retval 0 同步失败
   */
  uint8_t sync(void);
  /**
   * @fn readDir
   * @brief 读取当前目录的内容
   * @param name  保存读取的目录内的子目录或子文件
   * @param size  name缓存区的大小
   * @return 读取结果
   * @retval -1 读取失败
   * @retval  0 读取成功
   */
  int8_t readDir(char *name,uint16_t size);
  /**
   * @fn curPosition
   * @brief 获取指针当前位置
   * @return 指针当前位置
   */
  uint32_t curPosition(void);
  /**
   * @fn fileSize
   * @brief 获取文件总大小，单位字节
   * @return 文件总大小
   */
  uint32_t fileSize(void);
  /**
   * @fn seekSet
   * @brief 设置文件读写指针位置
   * @param pos 文件读写指针位置
   * @return 设置结果
   * @retval 0  设置失败
   * @retval 1  设置成功
   */
  uint8_t seekSet(uint32_t pos);
  /**
   * @fn isFile
   * @brief 判断是否为文件
   * @return 判断结果
   * @retval true  文件
   * @retval false 不是文件
   */
  bool isFile(void);
  /**
   * @fn isDir
   * @brief 判断是否为目录
   * @return 判断结果
   * @retval true  目录
   * @retval false 不是目录
   */
  bool isDir(void);
  /**
   * @fn isRoot
   * @brief 判断是否为根目录
   * @return 判断结果
   * @retval true  根目录
   * @retval false 不是根目录
   */
  bool isRoot(void);
  /**
   * @fn seekCur
   * @brief 设置文件读写指针位置
   * @param pos 文件读写指针位置
   * @return 设置结果
   * @retval 0  设置失败
   * @retval 1  设置成功
   */
  uint8_t seekCur(uint32_t pos);
  /**
   * @fn makeDir
   * @brief 创建目录
   * @param dirName 目录名相对路径
   * @return 创建结果
   * @retval false  创建失败
   * @retval true   创建成功
   */
  bool makeDir(const char* dirName);
  /**
   * @fn rewind
   * @brief 返回读目录首项
   */
  void rewind(void);
  /**
   * @fn remove
   * @brief 移除目录
   * @param fileName 目录或文件名
   * @return 移除结果
   * @retval 0  创建失败
   * @retval 1   创建成功
   */
  uint8_t remove(const char* fileName);
  /**
   * @fn exists
   * @brief 判断文件或目录是否存在
   * @param fileName 目录或文件名
   * @return 存在结果
   * @retval false  不存在
   * @retval true   存在
   */
  bool exists(const char* fileName);
  /**
   * @fn absolutePath
   * @brief 获取此文件或目录的绝对路径
   * @return 此文件或目录的绝对路径
   */
  String absolutePath();
  /**
   * @fn parentDirectory
   * @brief 获取此文件或目录的父级目录路径
   * @return 此文件或目录的父级目录路径
   */
  String parentDirectory();

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
