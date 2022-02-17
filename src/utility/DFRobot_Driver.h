/*!
 * @file DFRobot_Driver.h
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
#ifndef __DFROBOT_DRIVER_H
#define __DFROBOT_DRIVER_H

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include <Wire.h>

///< Define DBG, change 0 to 1 open the DBG, 1 to 0 to close.  
#if 0
#define DRV_DBG(...) {Serial.print("["); Serial.print(__FUNCTION__); Serial.print("(): "); Serial.print(__LINE__); Serial.print(" ] "); Serial.println(__VA_ARGS__);}
#else
#define DRV_DBG(...)
#endif


class DFRobot_Driver{
public:
  virtual void sendData(void* pData, size_t size) = 0;
  virtual uint8_t recvData(void* pData, size_t size) = 0;
  virtual void writeReg(uint8_t reg, void* pData, size_t size) = 0;
  virtual uint8_t readReg(uint8_t reg, void* pData, size_t size) = 0;
  virtual void flush() = 0;
protected:
  
};


#endif
