/*!
 * @file DFRobot_ComCSV.h
 * @brief 定义 DFRobot_ComCSV 类的基础结构
 * @details 组织了一系列的按csv格式写入文件的接口函数
 * @copyright	Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author [Arya](xue.peng@dfrobot.com)
 * @version  V1.0
 * @date  2022-05-6
 * @url https://github.com/DFRobot/DFRobot_Flash_Moudle
 */
#ifndef DFRobot_COMCSV_H
#define DFRobot_COMCSV_H

#include <Arduino.h>
#include <Stream.h>
#ifdef ARDUINO_SAM_ZERO
#include <variant.h>
#endif


#ifndef DEC
#define DEC 10
#endif

#ifndef HEX
#define HEX 16
#endif

#ifndef OCT
#define OCT 8
#endif

#ifdef BIN // Prevent warnings if BIN is previously defined in "iotnx4.h" or similar
#undef BIN
#endif
#define BIN 2

class DFRobot_ComCSV: public Print{
public:
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
protected:
  virtual uint16_t readData(void *pData, uint16_t size) = 0;
  virtual uint16_t writeData(void *pData, uint16_t size) = 0;
private:
  uint8_t _commaFlag; 
  uint8_t _commaFlag1; 
};
#endif