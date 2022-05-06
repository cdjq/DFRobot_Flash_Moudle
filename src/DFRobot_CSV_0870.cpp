/*!
 * @file DFRobot_CSV_0870.cpp
 * @brief 定义 DFRobot_CSV_0870 类 的实现
 * @details DFRobot_CSV_0870 类继承DFRobot_ComCSV抽象类
 * @copyright	Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author [Arya](xue.peng@dfrobot.com)
 * @version  V1.0
 * @date  2021-05-06
 * @url https://github.com/DFRobot/DFRobot_Flash_Moudle
 */

#include <math.h>
#include "DFRobot_CSV_0870.h"



DFRobot_CSV_0870::DFRobot_CSV_0870()
  :DFRobot_ComCSV(), _file(NULL){}


int DFRobot_CSV_0870::begin(DFRobot_File *file){
  _file = file;
  if(!_file) return 1;
  char *pname = _file->name();
  if(strlen(pname) <= 4){
    _file->close();
    return 2;
  }
  char *pci = pname + strlen(pname) - 1;
  if((*pci != 'V') && (*pci != 'v')){
    _file->close();
    return 2;
  }
  pci -= 1;
  if((*pci != 'S') && (*pci != 's')){
    _file->close();
    return 2;
  }
  pci -= 1;
  if((*pci != 'C') && (*pci != 'c')){
    _file->close();
    return 2;
  }
  pci -= 1;
  if(*pci != '.'){
    _file->close();
    return 2;
  }
  return DFRobot_ComCSV::begin();
}

uint16_t DFRobot_CSV_0870::readData(void *pData, uint16_t size){
  if(!_file) return 0;
  return (uint16_t)(_file->read(pData, size));
}

uint16_t DFRobot_CSV_0870::writeData(void *pData, uint16_t size){
  if(!_file) return 0;
  return (uint16_t)(_file->write((const uint8_t *)pData, size));
}