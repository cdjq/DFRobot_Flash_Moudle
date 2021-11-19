/*!
 * @file DFRobot_FatCmd.h
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
#ifndef __DFROBOT_FATCMD_H
#define __DFROBOT_FATCMD_H
/*
#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include <Wire.h>

#include "DFRobot_Driver.h"

///< Define DBG, change 0 to 1 open the DBG, 1 to 0 to close.  
#if 1
#define CMD_DBG(...) {Serial.print("["); Serial.print(__FUNCTION__); Serial.print("(): "); Serial.print(__LINE__); Serial.print(" ] "); Serial.println(__VA_ARGS__);}
#else
#define CMD_DBG(...)
#endif


class DFRobot_FatCmd{
public:
  uint16_t getResetCmdConfig(uint8_t *pkt, uint16_t pktSize);
  uint16_t getFlashInfoCmdConfig(uint8_t *pkt, uint16_t pktSize);
  uint16_t getReadAddrCmdConfig(uint8_t *pkt, uint16_t pktSize);
  uint16_t getModifyAddrCmdConfig(uint8_t *pkt, uint16_t pktSize, uint8_t newAddr);
  uint16_t getMkdirCmdConfig(uint8_t *pkt, uint16_t pktSize, void *data, uint8_t len);
  uint16_t getRemoveCmdConfig(uint8_t *pkt, uint16_t pktSize, void *data, uint8_t len);
  uint16_t getCreateCmdConfig(uint8_t *pkt, uint16_t pktSize, void *data, uint8_t len);
  uint16_t getRmFileCmdConfig(uint8_t *pkt, uint16_t pktSize, void *data, uint8_t len);
  uint16_t getReadFileCmdConfig(uint8_t *pkt, uint16_t pktSize, void *data, uint8_t len);
  uint16_t getWriteFileCmdConfig(uint8_t *pkt, uint16_t pktSize, void *data, uint8_t len);
  uint16_t getOpenFileCmdConfig(uint8_t *pkt, uint16_t pktSize, void *data, uint8_t len);
  uint16_t getCloseFileCmdConfig(uint8_t *pkt, uint16_t pktSize, void *data, uint8_t len);
  uint16_t getSyncFileCmdConfig(uint8_t *pkt, uint16_t pktSize, void *data, uint8_t len);
  uint16_t getSeekFileCmdConfig(uint8_t *pkt, uint16_t pktSize, void *data, uint8_t len);

  
  bool parseResPktOfFlashInfoCmd(uint8_t *data, uint16_t len, uint8_t *fatType, uint32_t *cap, uint32_t *freeS, uint16_t *fns);
  bool parseResPktOfOpenFileCmd(uint8_t *data, uint16_t len, int8_t *id, uint32_t *curPos, uint32_t *size);
  


  //uint8_t *getPWDCmdPkt(bool *state, uint16_t *len);
  //uint8_t *getReRootDirCmdPkt(bool *state, uint16_t *len);
  //uint8_t *getCdPWDCmdPkt(bool *state, uint8_t *data, uint8_t size, uint16_t *len);
  //uint8_t *getLsCmdPkt(bool *state, uint8_t *data, uint8_t size, uint16_t *len);
  
  
  //uint8_t *mallocResPktOfOpenFileCmd(bool *state, uint16_t *len);
  //uint8_t *mallocResPktOfFlashInfo(bool *state, uint16_t *len);
  
  //bool parseResPktOfOpenFileCmd(uint8_t *data, uint16_t len, uint8_t *id, uint32_t *curPos, uint32_t *size);
  //
};
*/

#endif
