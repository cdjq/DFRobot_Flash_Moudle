/*!
 * @file DFRobot_FatCmd.h
 * @brief 定义 DFRobot_DFR0870_Protocol 类的基础结构
 * @details 此协议用于Arduino主控同Flash Memory Moudle模块交互，目前能实现的交互内容为：
 * @n CMD_RESET            复位Flash Memory Moudle模块
 * @n CMD_FLASH_INFO       读取Flash Memory Moudle容量存储信息
 * @n CMD_READ_ADDR        读取IIC地址命令
 * @n CMD_SET_ADDR         设置I2C地址命令（此命令模块掉电后生效）
 * @n CMD_OPEN_FILE        打开文件
 * @n CMD_CLOSE_FILE       关闭文件或截断并关闭文件
 * @n CMD_WRITE_FILE       写文件命令
 * @n CMD_READ_FILE        读文件命令
 * @n CMD_SYNC_FILE        同步文件，将缓存数据写入实际文件中的命令
 * @n CMD_SEEK_FILE        设置文件读写指针位置命令
 * @n CMD_MKDIR            创建文件夹命令
 * @n CMD_OPEN_DIR         打开目录命令
 * @n CMD_CLOSE_DIR        关闭目录命令
 * @n CMD_REMOVE           移除文件或目录
 * @n CMD_FILE_ATTR        获取文件属性：0-文件或目录不存在 1-这是一个文件 2-FAT12根目录 3-FAT16根目录 4-FAT32根目录 5-子目录
 * @n CMD_READ_DIR         读取目录下的所有文件项和目录项
 * @n CMD_REWIND           回到读目录起始位置
 * @n CMD_ABSPATH          获取当前目录或文件的绝对路径
 * @n CMD_PARENTDIR        获取当前目录或文件的父级目录路径  
 * @copyright	Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author [Arya](xue.peng@dfrobot.com)
 * @version  V1.0
 * @date  2022-04-28
 * @url https://github.com/DFRobot/DFRobot_Flash_Moudle
 */
#ifndef __DFROBOT_FATCMD_H
#define __DFROBOT_FATCMD_H

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include <Wire.h>
#include "DFRobot_Driver.h"

class DFRobot_DFR0870_Protocol{
public:
 /**
  * @fn DFRobot_DFR0870_Protocol
  * @brief 空构造函数.
  */
  DFRobot_DFR0870_Protocol() :_timeoutms(0){}
 /**
  * @fn begin
  * @brief 协议接口初始化.
  * @param drv DFRobot_Driver抽象类指针
  * @return 接口初始化状态
  * @retval true  接口初始化成功
  * @retval false 接口初始化失败
  */
  bool begin(DFRobot_Driver *drv);
  /**
   * @fn reset
   * @brief 模块复位.
   * @param drv DFRobot_Driver抽象类指针
   * @return 复位结果
   * @retval true  复位成功
   * @retval false 复位失败
   */
  bool reset(); 
  /**
   * @fn getFlashInfo
   * @brief 获取模块参数.
   * @param fatType     保存模块FAT文件系统类型数据
   * @param capacity    保存模块容量数据
   * @param freeSec     保存模块空闲扇区数量数据
   * @param maxFileNums 保存模块最多能存储的文件的数量数据
   * @return 获取状态
   * @retval true  获取模块参数成功
   * @retval false 获取模块参数失败
   */
  bool getFlashInfo(uint8_t *fatType, uint32_t *capacity, uint32_t *freeSec, uint16_t *maxFileNums);
  /**
   * @fn getI2CAddress
   * @brief 获取模块的I2C地址.
   * @return 模块的I2C地址.
   */
  uint8_t getI2CAddress();
  /**
   * @fn getI2CAddress
   * @brief 设置模块的I2C地址.
   * @param addr  1~127 7位I2C地址
   * @return 设置状态.
   * @retval true  设置成功
   * @retval false 设置失败
   */
  bool setI2CAddress(uint8_t addr);
  /**
   * @fn openFile
   * @brief 打开文件.
   * @param name    文件名
   * @param pid     文件父级目录id
   * @param oflag   打开权限
   * @param id      存放文件id数据
   * @param curPos  存放文件当前读写指针位置数据
   * @param size    存放文件总大小数据
   * @return 打开状态.
   * @retval true  打开成功
   * @retval false 打开失败
   */
  bool openFile(const char *name, int8_t pid, uint8_t oflag, int8_t *id, uint32_t *curPos, uint32_t *size);
  /**
   * @fn closeFile
   * @brief 关闭文件.
   * @param id      文件id
   * @return 关闭状态.
   * @retval true  关闭成功
   * @retval false 关闭失败
   */
  bool closeFile(int8_t id, bool truncate);
  /**
   * @fn writeFile
   * @brief 向文件写数据.
   * @param id      文件id
   * @param data    指向要写入数据的缓存
   * @param len     要写入数据的大小，单位字节
   * @return 实际写入的数据字节.
   */
  uint16_t writeFile(int8_t id, void *data, uint16_t len);
  /**
   * @fn writeFile
   * @brief 向文件读数据.
   * @param id      文件id
   * @param data    保存读取的数据
   * @param len     读取数据的字节数
   * @return 实际读取数据大小.
   */
  uint16_t readFile(int8_t id, void *data, uint16_t len);
  /**
   * @fn sync
   * @brief 同步文件内容
   * @param id 文件id
   * @return 同步结果
   * @retval true  同步成功
   * @retval false 同步失败
   */
  bool sync(int8_t id);
  /**
   * @fn seekFile
   * @brief 设置文件读写指针位置
   * @param id  文件id
   * @param pos 文件读写指针位置
   * @return 设置结果
   * @retval true  设置成功
   * @retval false 设置失败
   */
  bool seekFile(int8_t id, uint32_t pos);
  /**
   * @fn newDirectory
   * @brief 创建目录
   * @param name  目录名
   * @param pid   父级目录id
   * @return 创建结果
   * @retval true  创建成功
   * @retval false 创建失败
   */
  bool newDirectory(const char *name, int8_t pid);
  /**
   * @fn openDirectory
   * @brief 打开目录
   * @param name  目录名
   * @param pid   父级目录id
   * @param id    存放目录id数据
   * @return 打开结果
   * @retval true  打开成功
   * @retval false 打开失败
   */
  bool openDirectory(const char *name, int8_t pid, int8_t *id);
  /**
   * @fn closeDirectory
   * @brief 关闭目录
   * @param id    目录id
   * @return 关闭结果
   * @retval true  关闭成功
   * @retval false 关闭失败
   */
  bool closeDirectory(int8_t id);
  /**
   * @fn remove
   * @brief 移除目录或文件
   * @param pid    父级目录id
   * @param name   目录或文件名
   * @return 移除结果
   * @retval true  移除成功
   * @retval false 移除失败
   */
  bool remove(int8_t pid, char *name);
  /**
   * @fn getFileAttribute
   * @brief 获取文件属性
   * @param pid    父级目录id
   * @param name   目录或文件名
   * @return 属性
   * @retval 0  文件或目录不存在
   * @retval 1  这是一个文件
   * @retval 2  FAT12根目录
   * @retval 3  FAT16根目录
   * @retval 4  FAT32根目录
   * @retval 5  子目录
   */
  uint8_t getFileAttribute(int8_t pid, char *name);
  /**
   * @fn readDirectory
   * @brief 读取目录内容
   * @param id            目录id
   * @param name          保存读取的文件项名
   * @param namebufsize   name缓存区大小
   * @return 读取结果
   * @retval true  读取成功
   * @retval false 读取失败
   */
  bool readDirectory(int8_t id,  char *name, uint16_t namebufsize);
  /**
   * @fn rewind
   * @brief 返回读取目录首项
   * @return 返回结果
   * @retval true  返回成功
   * @retval false 返回失败
   */
  bool rewind(int8_t id);
  /**
   * @fn getAbsolutePath
   * @brief 获取此文件或目录的绝对路径
   * @param id   文件或目录id
   * @param type 文件或目录属性
   * @n     0  文件或目录不存在
   * @n     1  这是一个文件
   * @n     2  FAT12根目录
   * @n     3  FAT16根目录
   * @n     4  FAT32根目录
   * @n     5  子目录
   * @return 此文件或目录的绝对路径
   */
  String getAbsolutePath(int8_t id, uint8_t type);
  /**
   * @fn getParentDirectory
   * @brief 获取此文件或目录的父级目录的绝对路径
   * @param id   文件或目录id
   * @param type 文件或目录属性
   * @n     0  文件或目录不存在
   * @n     1  这是一个文件
   * @n     2  FAT12根目录
   * @n     3  FAT16根目录
   * @n     4  FAT32根目录
   * @n     5  子目录
   * @return 此文件或目录的父级目录的绝对路径
   */
  String getParentDirectory(int8_t id, uint8_t type);


protected:
  bool writeCmdPacket(void *pData, uint16_t size,bool endflag = true);
  bool readResponseData(void *pData, uint16_t size, bool endflag = true);
  void *recvCmdResponsePkt(uint8_t cmd);
  void *packedCmdPacket(uint8_t cmd, uint16_t len);

private:
  uint32_t _timeoutms;

};



#endif
