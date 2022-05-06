/*!
 * @file DFRobot_Driver.h
 * @brief 定义 DFRobot_Driver 抽象类的基础结构
 * @details 组织了一系列的通信接口
 * @copyright	Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author [Arya](xue.peng@dfrobot.com)
 * @version  V1.0
 * @date  2021-10-09
 * @url https://github.com/DFRobot/DFRobot_Flash_Moudle
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
  virtual bool sendData(void* pData, uint16_t size, bool endflag) = 0;
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
  virtual bool recvData(void* pData, uint16_t size, bool endflag) = 0;
  /**
   * @fn flush
   * @brief  清空I2C接收缓冲区的数据
   * @return None
   */
  virtual void flush() = 0;
};


#endif
