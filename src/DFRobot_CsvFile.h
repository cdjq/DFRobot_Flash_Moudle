/*!
 * @file DFRobot_CsvFile.h
 * @brief 此文件中定义了DFRobot_CsvFile类，此文件提供了一系列csv文件格式的接口，可以通过这些接口函数在存储模块内操作csv文件，实现csv格式文件的读写操作
 * @details 为csv格式文件提供的一系列接口，具有以下功能:
 * @n 1. csv文件支持1000x26大小，即最大1000行，最多26列;
 * @n 2. 删除某行,某列（整行，整列删除）;
 * @n 3. 写或覆盖某行某列,或某单元内容（整行，整列写或覆盖）;
 * @n 4. 删除某单元内容（不会删除该单元哥，只是内容为空）
 * @n 
 * @copyright	Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author [Arya](xue.peng@dfrobot.com)
 * @version  V1.0
 * @date  2021-11-19
 * @url https://github.com/DFRobot/DFRobot_Flash_Moudle
 */

#ifndef __DFROBOT_CSVFILE_H
#define __DFROBOT_CSVFILE_H

#include "utility/csv.h"
#include "DFRobot_Flash_Moudle.h"

#if 0
#define CSV_DBG(...) {Serial.print("["); Serial.print(__FUNCTION__); Serial.print("(): "); Serial.print(__LINE__); Serial.print(" ] "); Serial.println(__VA_ARGS__);}
#else
#define CSV_DBG(...)
#endif


class DFRobot_CsvFile: public Print{
private:
  size_t printNumber(unsigned long, uint8_t);
  size_t printFloat(double, uint8_t);
public:
  /**
   * @enum eCSVFormParams_t
   * @brief csv文件的行,列范围，起始行、列从哪开始，最多支持多少行、列。即该行x列范围为1000x26，其实位置从1开始
   * @details eSmallestRow ~ eLargestRow：表示行计算从1开始，最多能写到第1000行
   * @n       eSmallestColumn ~ eLargestColumn: 表示列序号计算从1开始，每行最大能写到第26行
   */
  typedef enum{
    eLargestRow = 1000,                         /**< 该csv的最大支持的行号为1000 */
    eLargestColumn = (uint8_t)('Z' - 'A' + 1),  /**< 该csv的最大支持的列号为26 */
    eSmallestRow = 1,                           /**< 该csv的起始行编号从1开始 */
    eSmallestColumn = (uint8_t)('A' - 'A' + 1), /**< 该csv的起始列编号从1开始 */
  }eCSVFormParams_t;
  /**
   * @fn DFRobot_CsvFile
   * @brief 构造函数(分为无参和有参构造函数)
   * @param file DFRobot_File文件类对象指针，指向一个存在的文件对象
   * @return None
   */
  DFRobot_CsvFile();
  DFRobot_CsvFile(DFRobot_File *file);
  /**
   * @fn ~DFRobot_CsvFile
   * @brief 析构函数
   * @return None
   */
  ~DFRobot_CsvFile();
  /**
   * @fn begin
   * @brief 初始化函数
   * @return 初始化状态
   * @n     0: 初始化成功
   * @n     1: 构造函数中传入的形参file为空，或没传
   * @n     2: 传入的csv文件没有打开，请先打开该文件
   * @n     3: 传入的文件不是文件，是目录
   * @n     4: 传入的文件不是csv文件
   */
  uint8_t begin(DFRobot_File *file);
  /**
   * @fn getRow
   * @brief 获取当前csv文件的有效行数
   * @details 此csv文件最多支持写入(eLargestRow)1000行
   * @return 返回CSV文件最后一行的行号，范围0~1000，其中0代表该csv内容为空，没有数据，行号计数从1开始
   */
  int getRow();
  /**
   * @fn getColumn
   * @brief 获取当前csv文件中最后一列的列数，范围0~26
   * @details 此csv文件最多支持写入eLargestColumn（26）列，注意这里的列数是csv文件中所有行中，单元格最多的那一行，并不意味着每行都有那么多的列数。
   * @return 返回CSV文件最后一列的列号，范围0~26，其中0代表该csv内容为空，没有数据，列号计数从1开始
   */
  int getColumn();
  /**
   * @fn getRowsAndColumns
   * @brief 以指针的方式一次性获取csv文件的最大行和最大列
   * @details 行范围0~1000， 列范围：0~26
   * @param maxRow: 获取csv文件最后一行的行号，范围0~1000
   * @param maxCol: 获取csv文件列项最多的列号，范围0~26
   * @return None
   */
  void getRowsAndColumns(int *maxRow, int *maxCol);
  /**
   * @fn readRow
   * @brief 读取某一行的内容
   * @param row: 行号，从1开始，最大填1000(eLargestRow)
   * @return 返回该行的内容
   */
  String readRow(int row);
  /**
   * @fn writeRow
   * @brief 覆盖或写一行的内容
   * @param row: 行号，从1开始，最大填1000(eLargestRow)
   * @param rowData: 要写的内容
   * @n     如果rowData,用英文","区分每一列，如果存在/r/n，则会变成写多行
   * @return 返回写入的状态
   * @n     true: 写入成功
   * @n     false: 写入失败
   */
  //bool writeRow(int row, const char *rowData);
  //bool writeRow(int row, const String &rowData){ return writeRow(row, rowData.c_str());}
  /**
   * @fn deleteRow
   * @brief 删除此行的内容
   * @param row: 行号，从1开始，最大填1000(eLargestRow)，实际上最大有效值为调用getRow()的函数
   * @return 返回删除状态
   * @n     true: 删除成功
   * @n     false: 删除失败
   */
  //bool deleteRow(int row);
  /**
   * @fn readColumn
   * @brief 读取某一列的内容
   * @details 每一列的内容，被读出来，readColumn会自动在每一项后面加上/r/n
   * @param col: 第几号，从1开始，最大填26(eLargestColumn)，实际上最大有效值为调用getColumn()的函数
   * @return 返回该列的内容
   */
  String readColumn(int col);
  
  /**
   * @fn writeColumn
   * @brief 覆盖或写一列的内容，起始列从1开始
   * @param col: 列号，从1开始，最大填1(eLargestRow)
   * @param colData: 要写的内容
   * @n     colData中，请用/r/n区分每一列的内容，colData字符串或数组中使用了字符','会导致某一行多一列数据 
   * @return 返回写入的状态
   * @n     true: 写入成功
   * @n     false: 写入失败
   */
  //bool writeColumn(int col, const char *colData);
  //bool writeColumn(int col, const String &colData){ return writeColumn(col, colData.c_str());}
  
  /**
   * @fn deleteColumn
   * @brief 删除此列的内容
   * @param col: 列的号，从1开始，最大填1000(eLargestRow)，实际上最大有效值为调用，实际上最大有效值为调用getColumn()的函数
   * @return 返回删除状态
   * @n     true: 删除成功
   * @n     false: 删除失败
   */
  //bool deleteColumn(int col);
  /**
   * @fn readItem
   * @brief 读某行某列的内容
   * @param row: 行号，从1开始，最大填1000(eLargestRow)
   * @param col: 列编号，从1开始，最大填26(eLargestColumn)
   * @return 返回第row行的第col列数据的内容
   */
  String readItem(int row, int col);
  /**
   * @fn writeItem
   * @brief 修改或写某行某列的内容，允许某行某列不存在
   * @param row: 行号，从1开始，最大填1000(eLargestRow)
   * @param col: 列编号，从1开始，最大填26(eLargestColumn)
   * @param item: 要写的内容
   * @n     item里面最后好不要出现"/r/n"，或者","等字符，不然会改变原表的结构
   * @return 返回写入的状态
   * @n     true: 写入成功
   * @n     false: 写入失败
   */
  //bool writeItem(int row, int col, const char *item);
  //bool writeItem(int row, int col, const String &item){ return writeItem(row, col, item.c_str());}
  /**
   * @fn deleteItem
   * @brief 删除某一单元格的内容
   * @param row: 行号，从1开始，最大填1000(eLargestRow)
   * @param col: 列编号，从1开始，最大填26(eLargestColumn)
   * @return 返回删除状态
   * @n     true: 删除成功
   * @n     false: 删除失败
   */
  //bool deleteItem(int row, int col);
  /**
   * @fn write
   * @brief 父类print重载，一个就是一项，顺序存储
   */
  virtual size_t write(uint8_t val);
  size_t write(const char *str) {
      if (str == NULL) return 0;
      return write((const uint8_t *)str, strlen(str));
  }
  virtual size_t write(const uint8_t *buffer, size_t size);
  size_t write(const char *buffer, size_t size) {return write((const uint8_t *)buffer, size);}
  /**
   * @fn print
   * @brief 父类print重载，print写一个单元格的数据,写一项数据
   */
  size_t print(const String &s){return Print::print(s);}
  size_t print(const char str[]){return Print::print(str);}
  size_t print(char c){return Print::print(c);}
  size_t print(unsigned char b, int base = DEC){return Print::print(b, base);}
  size_t print(int n, int base= DEC){return print((long)n, base);}
  size_t print(unsigned int n, int base= DEC){return Print::print(n,base);}
  size_t print(long n, int base = DEC);//{Print::print(n,base);}
  size_t print(unsigned long n, int base= DEC){return Print::print(n, base);}
  size_t print(double, int = 2);
  /**
   * @fn println
   * @brief 父类println重载，println写一项单元格的数据，并结束当前行，开启下一行
   */
  size_t println(const String &s){return Print::println(s);}
  size_t println(const char str[]){return Print::println(str);}
  size_t println(char c){return Print::println(c);}
  size_t println(unsigned char b, int base = DEC){return Print::println(b, base);}
  size_t println(int n, int base= DEC){return println((long)n, base);}
  size_t println(unsigned int n, int base= DEC){return Print::println(n,base);}
  size_t println(long n, int base = DEC);
  size_t println(unsigned long n, int base= DEC){return Print::println(n, base);}
  size_t println(double, int = 2);
  size_t println(void){return Print::println();}
  
  operator bool();

private:
  DFRobot_File *_file;
  bool _initialize;
  int _rowNum;     //< 该文件的行
  int _columnNum;  //< 文件列
};

#endif