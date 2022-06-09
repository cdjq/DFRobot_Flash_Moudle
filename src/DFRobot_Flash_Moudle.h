/*!
 * @file DFRobot_Flash_Moudle.h
 * @brief 定义了 DFRobot_File 类，DFRobot_FlashMoudle 类 和 DFRobot_FlashMoudle_IIC 类的基础结构。 
 * @details DFRobot_File 类定义了文件和目录的相关操作；
 * @n DFRobot_FlashMoudle 类定义了磁盘的相关操作，如打开文件，移除文件，创建目录，判断文件或目录是否存在
 * @n DFRobot_FlashMoudle_IIC 类继承并实现了DFRobot_Driver抽象类的相关实现
 * @n 
 * @copyright	Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author [Arya](xue.peng@dfrobot.com)
 * @version  V1.0
 * @date  2021-11-19
 * @url https://github.com/DFRobot/DFRobot_Flash_Moudle
 */

#ifndef __DFROBOT_FLASH_MOUDLE_H
#define __DFROBOT_FLASH_MOUDLE_H

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include <Wire.h>

#include "utility/DFRobot_Flash.h"
#include "utility/DFRobot_Driver.h"
#include "utility/DFRobot_FatCmd.h"


///< Define DBG, change 0 to 1 open the DBG, 1 to 0 to close.  
#if 0
#define DBG(...) {Serial.print("["); Serial.print(__FUNCTION__); Serial.print("(): "); Serial.print(__LINE__); Serial.print(" ] "); Serial.println(__VA_ARGS__);}
#else
#define DBG(...)
#endif

#define FILE_READ  0x01
#define FILE_WRITE (0x01 | 0x02 | 0x10) //read write ALWAYS  apend:0x30
#define FILE_APPEND		(0x01 | 0x02 | 0x10 | 0x30)

class DFRobot_File : public Stream{
private:
 DFRobot_FlashFile *_file;
 char _name[32];
public:
  /**
   * @fn DFRobot_File
   * @brief DFRobot_File类构造
   * @param f DFRobot_FlashFile类对象
   * @param name 文件名
   */
  DFRobot_File(DFRobot_FlashFile f, const char *name);

  /**
   * @fn DFRobot_File
   * @brief 'empty' constructor.
   */
  DFRobot_File(void);

  /**
   * @fn name
   * @brief Strorage type and thehe file or directory name.
   * @return The Strorage type and file or directory name.
   */
  char * name();

  /**
   * @fn getAbsolutePath
   * @brief Get absolute path of file or directory.
   * @return The absolute path of file or directory.
   */
  String getAbsolutePath();

  /**
   * @fn getParentDirectory
   * @brief Get parent directory of file or directory.
   * @return The parent directory path of file or directory.
   */
  String getParentDirectory();
  
  /**
   * @fn write
   * @brief Write one byte into file. 
   * @param val: 即将被写入的值
   * @return Returns the size in bytes written
   */
  virtual size_t write(uint8_t val);
  
  /**
   * @fn write
   * @brief Write multiple bytes into file. 
   * @param buf 指向要写入的数组的指针
   * @param size 要写入的字节的数量，需小于或等于buf指针所指向的数组的大小
   * @return Returns the size in bytes written
   */
  virtual size_t write(const uint8_t *buf, size_t size);
  
  /**
   * @fn read
   * @brief Read 1 byte in file, 文件读指针自动加1
   * @return Return the readings
   */
  virtual int read();
  
  /**
   * @fn read
   * @brief Read multiple bytes in file, 文件读指针自动增加
   * @param buf 存储从文件中读取的数据
   * @param nbyte 要写入的字节的数量，需小于或等于buf指针所指向的数组的大小
   * @return Returns the size in bytes written
   */
  int read(void *buf, uint16_t nbyte);
  
  /**
   * @fn peek
   * @brief Read 1 byte in file. Reads the value at the same position in the file.文件读指针不变
   * @return Return the readings
   */
  virtual int peek();
  
  /**
   * @fn flush
   * @brief Wait for the data to be writen into file,等待数据被写入文件
   */
  virtual void flush();
  
  /**
   * @fn available
   * @brief Get the number of bytes in file.
   * @return 返回文件当前指针到文件结束位置字节的数量
   */
  virtual int available();
  
  /**
   * @fn seek
   * @brief Sets a file's position.
   * @param pos  pos The new position in bytes from the beginning of the file.
   * @return true, is returned for success and the value zero, false, is returned for failure.
   */
  boolean seek(uint32_t pos);
  
  /**
   * @fn position
   * @brief The current position for a file.
   * @return The current position.
   */
  uint32_t position();
  
  /**
   * @fn size
   * @brief The total number of bytes in a file.
   * @return file size.
   */
  uint32_t size();
  
  /**
   * @fn close
   * @brief close the file or close and truncate the file.
   * @param truncate 在关闭时是否截断读写指针之后的内容
   * @n     false 不截断
   * @n     true  截断
   * @return close or close and truncate result.
   * @retval false failed.
   * @retval true  sucess.
   */
  bool close(bool truncate = false);
  
  /**
   * @fn isDirectory
   * @brief Determine if the current file is a directory or a file.
   * @return True if this is a UdFile for a directory else false.
   */
  boolean isDirectory(void);

  /**
   * @fn bool
   * @brief 判断文件或目录是否被打开
   * @return 打开状态
   * @retval true  打开成功
   * @retval false 打开失败.
   */
  operator bool();

  /**
   * @fn openNextFile
   * @brief Open all files or directories in the current directory
   * @param mode Open permissions
   * @return Return file object.
   */
  DFRobot_File openNextFile(uint8_t mode = FILE_READ);

  /**
   * @fn rewindDirectory
   * @brief Set the file's current position to zero.
   */
  void rewindDirectory(void);
  using Print::write;
};


class DFRobot_FlashMoudle{
private:
  DFRobot_Flash _card;
  DFRobot_FlashFile _root;
  friend class DFRobot_File;
  void getParentDir(const char *filepath, int *index);
public:
  /**
   * @fn DFRobot_FlashMoudle
   * @brief 空构造函数.
   */
  DFRobot_FlashMoudle();

  /**
   * @fn ~DFRobot_FlashMoudle
   * @brief 析构函数.
   */
  ~DFRobot_FlashMoudle();

  /**
   * @fn begin
   * @brief flash模块初始化
   * @param drv DFRobot_Driver抽象类指针，可以传继承DFRobot_Driver类的所有子类对象的指针
   * @return 返回初始化结果
   * @retval 0 初始化成功
   * @retval others 初始化失败
   */
  uint8_t begin(DFRobot_Driver *drv);

  /**
   * @fn open
   * @brief 打开文件或目录
   * @details 以读写创建或只读的方式打开文件或目录，其中目录打开只能以只读的方式打开。
   * @param filepath 文件或目录的绝对路径，根目录为"/"
   * @param mode 打开权限
   * @n     FILE_READ   以只读的方式打开文件或目录
   * @n     FILE_WRITE  以读写的方式打开文件，打开后读写指针位置在文件首位
   * @n     FILE_APPEND 以追加的方式打开文件，打开后读写指针位置在文件末尾
   * @return 返回DFRobot_File类对象
   * @attention 注意根目录由系统打开，用户无法关闭它
   */
  DFRobot_File open(const char *filepath, uint8_t mode = FILE_READ);
  DFRobot_File open(const String &filepath, uint8_t mode = FILE_READ) { return open( filepath.c_str(), mode ); }
  
  /**
   * @fn exists
   * @brief 查看某个文件或目录是否是否存在
   * @details 这里需传绝对路径
   * @param filepath 文件或目录的绝对路径
   * @return 返回创建状态
   * @retval true 创建成功或该目录已经存在
   * @retval false 创建失败
   */
  boolean exists(const char *filepath);
  boolean exists(const String &filepath) { return exists(filepath.c_str()); }
  
  /**
   * @fn mkdir
   * @brief 创建目录
   * @param filepath 目录的绝对路径
   * @return 返回创建状态
   * @retval true 创建成功或该目录已经存在
   * @retval false 创建失败
   */
  boolean mkdir(const char *filepath);
  boolean mkdir(const String &filepath) { return mkdir(filepath.c_str()); }
  
  /**
   * @fn remove
   * @brief 移除空文件夹或文件
   * @details 此函数用来删除文件，可以移除单个文件或空文件夹
   * @param filepath 目录或文件的绝对路径。根目录为"/"，不可移除
   * @return 返回移除状态
   * @retval true 移除成功
   * @retval false 移除失败
   */
  boolean remove(const char *filepath);
  boolean remove(const String &filepath) { return remove(filepath.c_str()); }
  
  /**
   * @fn rmdir
   * @brief 移除空文件夹或文件
   * @details 此函数用来删除文件，可以移除单个文件或空文件夹
   * @param filepath 目录或文件的绝对路径。根目录为"/"，不可移除
   * @return 返回移除状态
   * @retval true 移除成功
   * @retval false 移除失败
   */
  boolean rmdir(const char *filepath);
  boolean rmdir(const String &filepath) { return rmdir(filepath.c_str()); }
private:
  friend class File;
};

class DFRobot_FlashMoudle_IIC: public DFRobot_Driver{
public:
  /**
   * @fn DFRobot_FlashMoudle_IIC
   * @brief constructor function.
   * @param addr I2C地址，固定为0x55
   * @param pWire TwoWire类指针
   */
  DFRobot_FlashMoudle_IIC(uint8_t addr = 0x55, TwoWire *pWire=&Wire);
  /**
   * @fn ~DFRobot_FlashMoudle_IIC
   * @brief 析构函数
   */
  ~DFRobot_FlashMoudle_IIC();
  /**
   * @fn begin
   * @brief IIC接口初始化
   * @param freq I2C通信频率
   * @return 返回初始化状态
   * @retval 0 初始化成功
   * @retval 1 DFRobot_FlashMoudle_IIC构造中传入的pWire为NULL
   * @retval 2 设备未找到
   */
  uint8_t begin(uint32_t freq = 100000);

  /**
   * @fn sendData
   * @brief  发送数据到I2C总线
   * @param pData 指向要发送的数据的指针
   * @param size 要发送的数据
   * @param endflag 是否发送停止位
   * @n     true 发送停止位
   * @n     false 不发送停止位
   * @return 发送状态
   * @retval true  发送成功
   * @retval true  发送失败
   */
  bool sendData(void* pData, uint16_t size, bool endflag = true);
  /**
   * @fn recvData
   * @brief  从I2C总线上接收数据
   * @param pData 存储从IIC总线上接收的数据
   * @param size 需要接收的数据
   * @param endflag 是否发送停止位
   * @n     true 发送停止位
   * @n     false 不发送停止位
   * @return 接收状态
   * @retval true  接收成功
   * @retval true  接收失败
   */
  bool recvData(void* pData, uint16_t size, bool endflag = true);
  /**
   * @fn flush
   * @brief  清空I2C接收缓冲区的数据
   * @return None
   */
  virtual void flush();

private:
  TwoWire *_pWire;
  uint8_t _addr;
};


#endif
