/*!
 * @file DFRobot_Flash_Moudle.h
 * @brief 此文件中定义了DFRobot_File基本文件操作类，DFRobot_FlashMoudle flash模块操作类， DFRobot_FlashMoudle_IIC驱动类。
 * @details 这是一个文件系统库，专门用来操作DFRobot自制的一款通信接口为I2C的flash Memory Moudle。使用该库，可以用文件的方式对
 * @n 该flash存储模块进行操作，你可以直接在该模块上面创建，读，写文件，或者创建目录。此驱动它有以下特点：
 * @n 1. 能实现用文件的方式操作特定的存储模块;
 * @n 2. 目前只支持短文件名
 * @n 3. 目前支持I2C驱动
 * @n 相关类的简单介绍：
 * @n DFRobot_File： 此类定义了文件或目录的简单操作，包括读、写、增加、删除文件内容，打开目录的下级文件，以及获取文件名或目录名
 * @n DFRobot_FlashMoudle：此类定义了flash模块的相关操作，可以用来在该存储模块内创建，打开，增加，删除多个文件或目录，并查询相关
 * @n 文件或目录是否存在。
 * @n DFRobot_FlashMoudle_IIC 此类定义了模块的接口驱动，用户只需操作begin函数
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


///< Define DBG, change 0 to 1 open the DBG, 1 to 0 to close.  
#if 0
#define DBG(...) {Serial.print("["); Serial.print(__FUNCTION__); Serial.print("(): "); Serial.print(__LINE__); Serial.print(" ] "); Serial.println(__VA_ARGS__);}
#else
#define DBG(...)
#endif

#define FILE_READ  0x01
#define FILE_WRITE (0x01 | 0x02 | 0x10) //read write ALWAYS  apend:0x30

class DFRobot_File : public Stream{
private:
 DFRobot_FlashFile *_file;
 char _name[32];
public:
  /**
   * @fn DFRobot_File
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
   * @brief The current position for a file or directory.
   * @return The current position.
   */
  uint32_t position();
  /**
   * @fn size
   * @brief The total number of bytes in a file or directory.
   * @return file size.
   */
  uint32_t size();
  /**
   * @fn close
   * @brief close the file.
   */
  void close();
  /**
   * @fn isDirectory
   * @brief Determine if the current file is a directory or a file.
   * @return True if this is a UdFile for a directory else false.
   */
  boolean isDirectory(void);
  /**
   * @fn del
   * @brief 删除文件里的内容(能不用，请尽量不要使用，flash有擦写次数限制，尽量不要使用)
   * @param pos 光标在文件中距离起始位置的位置
   * @param num 删除的字符数，如果是中文，请自动乘以2
   * @param flag 删除方向， 
   * @n     true: 删除光标前面的字符
   * @n     false: 删除光标后面的字符
   * @return 删除状态
   * @n     true: 成功
   * @n     false: 失败
   */
  //boolean del(uint32_t pos, uint32_t num, bool flag = true);
  //boolean insert(uint32_t pos, uint8_t c, uint32_t num);
  //boolean insert(uint32_t pos, void *buf, uint32_t len);

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
  DFRobot_FlashFile getParentDir(const char *filepath, int *indx);
public:
 /**
  * @fn DFRobot_FlashMoudle
  * @brief 空构造函数.
  */
  DFRobot_FlashMoudle();
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
   * @n     根目录的目录或文件包含最多递归10级，/dir1/dir2/dir3/dir4/dir5/dir6/dir6/dir7/dir8/dir9, dir9下不能再创建任何目录和文件
   * @n     单个目录项名字不能超过11个字符
   * @param mode 打开权限
   * @n     FILE_READ  只读权限，此种方式可以打开文件或目录
   * @n     FILE_WRITE  读写创建权限，此种方式只可以打开文件，如果文件存在则以读写方式打开，如果不存在则创建一个新的文件再以读写的方式打开
   * @return 返回DFRobot_File类对象
   * @attention 可同时打开10个文件和10个目录（包含根目录），注意根目录由系统打开，用户无法关闭它
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
   * @n     根目录的目录或文件包含最多递归10级，/dir1/dir2/dir3/dir4/dir5/dir6/dir6/dir7/dir8/dir9, dir9下不能再创建任何目录和文件
   * @n     单个目录项名字不能超过11个字符
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
   * @param filepath 目录的绝对路径，根目录为"/"
   * @n     根目录的目录或文件包含最多递归10级，/dir1/dir2/dir3/dir4/dir5/dir6/dir6/dir7/dir8/dir9, dir9下不能再创建任何目录和文件
   * @n     单个目录项名字不能超过11个字符
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
   * @param filepath 目录的绝对路径，根目录为"/"
   * @n     根目录的目录或文件包含最多递归10级，/dir1/dir2/dir3/dir4/dir5/dir6/dir6/dir7/dir8/dir9, dir9下不能再创建任何目录和文件
   * @n     单个目录项名字不能超过11个字符
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
   */
  DFRobot_FlashMoudle_IIC(uint8_t addr = 0x55,TwoWire *pWire=&Wire);
  ~DFRobot_FlashMoudle_IIC();
  /**
   * @fn begin
   * @brief IIC接口初始化
   * @return 返回初始化状态
   * @retval 0 初始化成功
   * @retval 1 DFRobot_FlashMoudle_IIC构造中传入的pWire为NULL
   * @retval 2 设备未找到
   */
  uint8_t begin(uint32_t freq = 1000);
  /**
   * @fn sendData
   * @brief  发送数据到I2C总线
   * @param pData 指向要发送的数据的指针
   * @param size 要发送的数据
   * @return None
   */
  virtual void sendData(void* pData, size_t size);
  /**
   * @fn recvData
   * @brief  从I2C总线上接收数据
   * @param pData 存储从IIC总线上接收的数据
   * @param size 需要接收的数据
   * @return 返回实际接收的数据的长度
   */
  virtual uint8_t recvData(void* pData, size_t size);
  /**
   * @fn writeReg
   * @brief  写寄存器
   * @param reg 寄存器地址
   * @param pData 指向要发送的数据的指针
   * @param size 要发送的数据
   * @return None
   */
  virtual void writeReg(uint8_t reg, void* pData, size_t size);
  /**
   * @fn readReg
   * @brief  从I2C总线上接收数据
   * @param reg 寄存器地址
   * @param pData 存储从IIC总线上接收的数据
   * @param size 需要接收的数据
   * @return 返回实际接收的数据的长度
   */
  virtual uint8_t readReg(uint8_t reg, void* pData, size_t size);
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
