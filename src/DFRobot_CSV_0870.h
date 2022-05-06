/*!
 * @file DFRobot_CSV_0870.h
 * @brief 定义 DFRobot_CSV_0870 类 的基础结构
 * @details DFRobot_CSV_0870 类继承DFRobot_ComCSV抽象类
 * @copyright	Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author [Arya](xue.peng@dfrobot.com)
 * @version  V1.0
 * @date  2021-05-06
 * @url https://github.com/DFRobot/DFRobot_Flash_Moudle
 */

#ifndef __DFRobot_CSV_0870_H
#define __DFRobot_CSV_0870_H

#include "DFRobot_Flash_Moudle.h"
#include "utility/DFRobot_ComCSV.h"

class DFRobot_CSV_0870: public DFRobot_ComCSV{
public:
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

protected:
  uint16_t readData(void *pData, uint16_t size);
  uint16_t writeData(void *pData, uint16_t size);
private:
   
  DFRobot_File *_file;
};
#endif