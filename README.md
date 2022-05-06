# DFRobot_Flash_Moudle
* [中文版](./README_CN.md)

Flash Memory flash是一个内置文件系统的flash存储模块，采用esp32s2作为主控，板载了16M W25Q128型号存储芯片，该存储芯片单个块的擦写次数达10万次。<br>
用户可以直接通过windows，linux系统电脑通过操作U盘的文件系统的方式操作该存储模块，也可以通过DFRobot提供的DFRobot_Flash_Moudle库通过部分Arduino <br> 
MCU以文件系统的方式操作该存储模块。具体表现方式如下所述：
1. Flash Memory flash模块通过USB接口插入电脑，会弹出一个16M的U盘；
2. 连接Flash Memory flash模块和部分Arduino MCU的I2C接口，可以通过I2C实现文件和目录的相关操作；
3. Arduino MCU操作方式中增加了CSV格式文件的写入，用户可以通过此接口写入CSV格式文件；
* 注意：同一时刻只能用一种操作方式，要么使用U盘，要么使用I2C接口，如果在其中一种方式中操作了文件，更换另一种方式时，必须先让模块断电再操作,否则无法
正确操作文件，常见的情况为，U盘写不进去文件，Arduino MCU初始化模块失败<br>
* 注意： 如果程序操作文件总是不对，此时可以用U盘格式化flash模块，断电后就能正常使用了。<br>


![产品效果图](./resources/images/DFR0870.png)

## Product Link（[https://www.dfrobot.com](https://www.dfrobot.com)）
    SKU：DFR0870

## Table of Contents

* [Summary](#summary)
* [Installation](#installation)
* [Methods](#methods)
* [Compatibility](#compatibility)
* [History](#history)
* [Credits](#credits)

## Summary
1. 这是一个存储模块，能以文件的方式存储数据；<br>
2. 能在操作系统上以U盘的方式实现文件的创建、读写、删除;<br>
3. 内置fat12文件系统，可以通过UNO等主控实现文件的存储、读写、删除<br>
4. 目前主控代码操控方式只支持Arduino，不支持树莓派，树莓派只能操作U盘，不能用代码实现文件的控制。<br>
5. 支持CSV文件操作

U盘操作： 支持Window7/10, linux(树莓派) <br>
文件系统驱动：目前只支持Arduino平台，暂不支持树莓派 <br>
## Installation

There two methods: 
1. To use this library, first download the library file, paste it into the \Arduino\libraries directory, then open the examples folder and run the demo in the folder.
2. Search the DFRobot_Flash_Moudle library from the Arduino Software Library Manager and download it.

## Methods

```C++
/***************************************IIC 接口操作***************************************/
class DFRobot_FlashMoudle_IIC:
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
  uint8_t begin(uint32_t freq = 1000);
/***************************************IIC 接口操作 结束***************************************/

/***************************************磁盘操作***************************************/
class DFRobot_FlashMoudle:
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
/***************************************磁盘操作 结束***************************************/ 

/***************************************文件操作***************************************/
class DFRobot_File：
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
/***************************************文件操作 结束***************************************/

/***************************************CSV文件写入操作***************************************/
class DFRobot_CSV_0870:
  /**
   * @fn DFRobot_CSV_0870
   * @brief DFRobot_CSV_0870类构造
   */
  DFRobot_CSV_0870();

  /**
   * @fn begin
   * @brief Flash Memory Moudle CSV格式文件初始化，此函数主要用来检测传入的文件是否符合CSV文件命名规范
   * @param file DFRobot_File类对象指针，这里应该传入一个后缀名为.CSV or .csv的文件
   * @return 初始化结果
   * @retval 0   初始化成功
   * @retval 1   file为空
   * @retval 2   文件不是csv文件
   * @retval others 保留值，留待以后更新库使用
   */
  int begin(DFRobot_File *file);

  /**
   * @fn DFRobot_ComCSV
   * @brief DFRobot_ComCSV抽象类构造
   */
  DFRobot_ComCSV();

  /**
   * @fn ~DFRobot_ComCSV()
   * @brief 析构函数
   */
  ~DFRobot_ComCSV();

  /**
   * @fn begin
   * @brief 初始化
   * @return 初始化结果
   * @retval 0  初始化成功
   * @retval others 留值，留待以后更新库使用
   */
  int begin();
  
  /**
   * @fn write
   * @brief 向CSV文件写一个数据，用户单独调用这个函数是写同一项数据，如果需要换项，需手动写入字符',',write(0x2c)
   * @n  需要换行，则手动写入'/r/n' write(0x0d) and write(0x0a)
   * @param val 要写入的字节数据
   * @return 返回实际写入字节的大小
   */
  virtual size_t write(uint8_t val);

  /**
   * @fn write
   * @brief 向CSV文件字符串数据，用户单独调用这个函数是写同一项数据，如果需要换项，需手动写入字符',',write(",")
   * @n  需要换行，则手动写入'/r/n' write("/r/n"),
   * @param str 要写入的字符串数据
   * @note str中不要有","或"\r\n",它们代表特殊数据，","会将字符串按列分开，"\r\n"会将字符串按行分开，特殊字符需要加转义字符"\"
   * @n 比如你想在csv表格中显示加双引号的字符串"Jerry",这个时候双引号就必须加"\""转义字符才能显示"\"Jerry\""
   * @return 返回实际写入字节的大小
   */
  size_t write(const char *str) {
      if (str == NULL) return 0;
      return write((const uint8_t *)str, strlen(str));
  }

  /**
   * @fn write
   * @brief 向CSV文件数组数据，用户单独调用这个函数是写同一项数据，如果需要换项，需手动写入字符','，
   * @n  需要换行，则手动写入'/r/n'
   * @param buffer 要写入的数组数据
   * @param size 要写入的数组数据大小
   * @return 返回实际写入字节的大小
   */
  virtual size_t write(const uint8_t *buffer, size_t size);
  size_t write(const char *buffer, size_t size) {
      return write((const uint8_t *)buffer, size);
  }
  
  /**
   * @fn print
   * @brief 向CSV文件写入一项数据，即写入单元格数据，增加一列单元格，print与print之间会用","分开
   * @param const String &s   数据类型重载，const String & 类型数据， 表示能向CSV表格直接写入const String &类型数据
   * @n     const char str[]  数据类型重载，const char []  类型数据， 表示能向CSV表格直接写入const char []类型数据
   * @n     char c            数据类型重载，char           类型数据， 表示能向CSV表格直接写入char类型数据
   * @return 返回实际写入字节的大小
   */
  size_t print(const String &s);
  size_t print(const char str[]);
  size_t print(char c);
  /**
   * @fn print
   * @brief 向CSV文件写入一项数据，即写入单元格数据，增加一列单元格，print与print之间会用","分开
   * @param unsigned char b    数据类型重载， unsigned char 类型数据，表示能向CSV表格直接写入 unsigned char 类型数据
   * @n     int n              数据类型重载， int           类型数据，表示能向CSV表格直接写入 int           类型数据
   * @n     unsigned int n     数据类型重载， unsigned int  类型数据，表示能向CSV表格直接写入 unsigned int  类型数据
   * @n     long n             数据类型重载， long n        类型数据，表示能向CSV表格直接写入 long n        类型数据
   * @n     unsigned long n    数据类型重载， unsigned long 类型数据，表示能向CSV表格直接写入 unsigned long 类型数据
   * @param base  表示以什么进制的数据显示，默认十进制数据
   * @n     DEC   以十进制的数据显示
   * @n     HEX   以十六进制的数据显示
   * @n     OCT   以八进制的数据显示
   * @n     BIN   以二进制的数据显示
   * @return 返回实际写入字节的大小
   */
  size_t print(unsigned char b, int base = DEC);
  size_t print(int n, int base= DEC);
  size_t print(unsigned int n, int base= DEC);
  size_t print(long n, int base = DEC);
  size_t print(unsigned long n, int base= DEC);
  /**
   * @fn print
   * @brief 向CSV文件写入一单元格数据，并','结束该单元格，开始新的一列单元格数据。
   * @param double num    数据类型重载， double 类型数据，表示能向CSV表格直接写入 double 类型数据
   * @param digits  表示显示几位小数点位置，默认显示2位小数点的浮点数据
   * @return 返回实际写入字节的大小
   */
  size_t print(double num, int digits = 2);
  
  /**
   * @fn println
   * @brief 向CSV文件写入一单元格数据，并回车换行，开始新的一行
   * @param const String &s   数据类型重载，const String & 类型数据， 表示能向CSV表格直接写入const String &类型数据
   * @n     const char str[]  数据类型重载，const char []  类型数据， 表示能向CSV表格直接写入const char []类型数据
   * @n     char c            数据类型重载，char           类型数据， 表示能向CSV表格直接写入char类型数据
   * @return 返回实际写入字节的大小
   */
  size_t println(const String &s);
  size_t println(const char str[]);
  size_t println(char c);
  /**
   * @fn println
   * @brief 向CSV文件写入一单元格数据，并回车换行，开始新的一行
   * @param unsigned char b    数据类型重载， unsigned char 类型数据，表示能向CSV表格直接写入 unsigned char 类型数据
   * @n     int n              数据类型重载， int           类型数据，表示能向CSV表格直接写入 int           类型数据
   * @n     unsigned int n     数据类型重载， unsigned int  类型数据，表示能向CSV表格直接写入 unsigned int  类型数据
   * @n     long n             数据类型重载， long n        类型数据，表示能向CSV表格直接写入 long n        类型数据
   * @n     unsigned long n    数据类型重载， unsigned long 类型数据，表示能向CSV表格直接写入 unsigned long 类型数据
   * @param base  表示以什么进制的数据显示，默认十进制数据
   * @n     DEC   以十进制的数据显示
   * @n     HEX   以十六进制的数据显示
   * @n     OCT   以八进制的数据显示
   * @n     BIN   以二进制的数据显示
   * @return 返回实际写入字节的大小
   */
  size_t println(unsigned char b, int base = DEC);
  size_t println(int n, int base= DEC);
  size_t println(unsigned int n, int base= DEC);
  size_t println(long n, int base = DEC);
  size_t println(unsigned long n, int base= DEC);
  /**
   * @fn println
   * @brief 向CSV文件写入一单元格数据，并回车换行，开始新的一行
   * @param double num    数据类型重载， double 类型数据，表示能向CSV表格直接写入 double 类型数据
   * @param digits  表示显示几位小数点位置，默认显示2位小数点的浮点数据
   * @return 返回实际写入字节的大小
   */
  size_t println(double num, int digits = 2);
  /**
   * @fn println
   * @brief 向CSV文件写入\r\n，用以换行
   * @return 返回实际写入字节的大小
   */
  size_t println(void);
/***************************************CSV文件写入操作 结束***************************************/
  
```

## Compatibility

主板               | 通过  | 未通过   | 未测试   | 备注
------------------ | :----------: | :----------: | :---------: | -----
Arduino uno        |      √       |              |             | 
Mega2560        |      √       |              |             | 
Leonardo        |      √       |              |             | 
ESP32           |      √       |              |             | 
ESP8266           |      √       |              |             | 
M0                |      √       |              |             | 
micro:bit        |      √       |              |             | 

## History

- 2021/11/19 - Version 1.0.0 released.

## Credits

Written by Arya(xue.peng@dfrobot.com), 2022. (Welcome to our [website](https://www.dfrobot.com/))