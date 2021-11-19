/*!
 * @file DFRobot_FatCmd.cpp
 * @brief 定义DFRobot_FatCmd 类的基础结构，基础方法的实现
 * @details 这是一个虚拟的传感器，IIC地址不可改变,不对应任何实物，可以通过IIC和SPI口来控制它，假设它有下面这些功能
 * @n 向寄存器0里写入数据，点亮不同颜色的LED灯
 * @n 从寄存器2里读出数据，高四位表示光线强度，低四位表示声音强度
 * @n 从寄存器3 bit0里写入数据，写1表示正常模式，写0表示低功耗模式
 * @n 从寄存器4 读取数据，读到的是芯片版本0xDF
 * @copyright	Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author [Ouki](ouki.wang@dfrobot.com)
 * @version  V1.0
 * @date  2019-07-13
 * @url https://github.com/ouki-wang/DFRobot_Sensor
 */
#include <Arduino.h>
#include "DFRobot_FatCmd.h"

//#define CONFIG_CMD_REG      0x00  ///<配置命令寄存器
//#define STATUS_REG_FIX      0x02  ///<状态寄存器, 每条命令都有一个状态寄存器，它的实际位置 = STATUS_REG_FIX + 命令数据的长度
//#define CMD_LEN_MAX         254
//
//#define CMD_FLASH_INFO      0x01  ///< 读取Flash信息命令
//#define CMD_READ_ADDR       0x02  ///< 读取IIC地址命令
//#define CMD_MODIFY_ADDR     0x03  ///< 修改IIC地址命令
//#define CMD_OPEN_FILE       0x05  ///< 打开文件命令
//#define CMD_RESET           0x06  ///< 复位I2C从机命令, 只有应答命令无响应包，成功返回0x53，失败返回0x63
//#define CMD_CLOSE_FILE      0x07  ///< 关闭文件命令
//#define CMD_WRITE_FILE      0x08  ///< 写文件命令
//#define CMD_SYNC_FILE       0x09  ///< 将缓存数据写入实际文件中的命令
//#define CMD_SEEK_FILE       0x0A  ///< 设置文件读写指针位置命令
//#define CMD_READ_FILE       0x0B  ///< 读文件命令
//
//
//#define CMD_GET_PWD         0x02  ///< 获取当前工作目录命令
//#define CMD_RE_ROOT_DIR     0x03  ///< 返回根目录命令
//#define CMD_CD_PWD          0x04  ///< 切换工作目录命令
//#define CMD_LS              0x05  ///< 列出某目录下所有文件或文件夹命令
//#define CMD_MKDIR           0x06  ///< 创建文件夹命令
//#define CMD_REMOVE          0x07  ///< 移除文件夹命令
//#define CMD_CREATE          0x08  ///< 创建文件命令
//#define CMD_RM_FILE         0x09  ///< 移除文件命令
//
//
//
//#define CMD_INVAILD         0x0E  ///< 无效命令
//
//
//#define LEN_FLASH_INFO      0x00  ///< 读取Flash信息命令
//#define LEN_READ_ADDR       0x00  ///< 读取IIC地址命令的固定长度
//#define LEN_MODIFY_ADDR     0x01  ///< 修改IIC地址命令的固定长度
//#define LEN_OPEN_FILE       0x00  ///< 打开文件命令的固定长度
//#define LEN_RESET           0x00  ///< 复位I2C从机命令的固定长度
//#define LEN_CLOSE_FILE      0x00  ///< 关闭文件命令固定长度
//#define LEN_WRITE_FILE      0x00  ///< 写文件命令的固定长度
//#define LEN_SYNC_FILE       0x00  ///< 将缓存数据写入实际文件中命令的固定长度
//#define LEN_SEEK_FILE       0x00  ///< 设置文件读写指针位置命令的固定长度
//#define LEN_READ_FILE       0x00  ///< 读文件命令的固定长度
//
//#define LEN_GET_PWD         0x00  ///< 获取当前工作目录命令的固定长度
//#define LEN_RE_ROOT_DIR     0x00  ///< 返回根目录命令固定长度
//#define LEN_CD_PWD          0x00  ///< 切换工作目录命令固定长度
//#define LEN_LS              0x00  ///< 列出某目录下所有文件或文件夹命令的固定长度
//#define LEN_MKDIR           0x00  ///< 创建文件夹命令的固定长度
//#define LEN_REMOVE          0x00  ///< 移除文件夹命令的固定长度
//#define LEN_CREATE          0x00  ///< 创建文件命令固定长度
//#define LEN_RM_FILE         0x00  ///< 移除文件命令固定长度
//
//
//
//
//
//#define LEN_RESP_FLASH_INFO  (1 + 4 + 4 + 2 )     ///< 读取Flash信息命令的响应包 分别为FAT类型（1字节）， flash容量（4字节）， 空闲字节数(4字节) 能存储的最大文件数（2字节）crc(1字节)
//#define LEN_RESP_OPEN_FILE   (1 + 4 + 4)         ///< 打开命令的响应包 分别为ID（1字节）， 当前位置（4字节）， 文件总大小（4字节） crc(1字节)
////#define LEN_RESP_OPEN_FILE   (0)         ///< 复位命令的响应包 分别为ID（1字节）， 当前位置（4字节）， 文件总大小（4字节） crc(1字节)
//
//
//typedef struct{
//  uint8_t reg; /**< 固定包头0xAA */
//  uint8_t cmd;    /**< 命令，范围0x00~0x0E,0x0F及之后为无效命令 */
//  uint8_t len;    /**< 除去具体命令后的数据长度，这个一般用来计算buf数组的长度 */
//  uint8_t buf[CMD_LEN_MAX]; /**< 0长度数组，它的大小取决于上一个变量len的值 */
//}__attribute__ ((packed)) sCmdPkt_t, *pCmdPkt_t;
//
///**
// * @fn CmdPacket
// * @brief 模块命令打包
// * @param cmd   指向uint8_t类型数组的指针，存储了命令包的包头(0x55 0xAA)、具体命令、命令长度
// * @param data  指向uint8_t类型数组的指针，这个内容主要用来为sCmdPkt_t结构体类型中的buf成员变量赋值
// * @param state 指向布尔类型变量的指针， 获取打包状态，true表示打包成功， false表示打包失败
// * @param len   指向uint16_t类型变量的指针， 获取命令包的总长度，单位字节
// * @return uint8_t *类型数组，表示命令包的起始地址， NULL表示打包失败
// */
//static uint16_t cmdPacket(uint8_t *pkt, uint8_t cmd, uint8_t cmdLen, void *data, uint16_t len){
//  pCmdPkt_t pCmd = (pCmdPkt_t)pkt;
//  uint16_t reLen = 3;
//  pCmd->reg = CONFIG_CMD_REG;
//  pCmd->cmd = cmd;
//  pCmd->len = cmdLen;
//  if((data != NULL) && (len > 0)){
//    len = (len > CMD_LEN_MAX) ? CMD_LEN_MAX : len;
//    memcpy(pCmd->buf, data, len);
//    reLen += len;
//    pCmd->len += len;
//  }
//  return reLen;
//}
//
//uint16_t DFRobot_FatCmd::getResetCmdConfig(uint8_t *pkt, uint16_t pktSize){
//  if(pkt == NULL){
//    CMD_DBG("pkt is NULL");
//    return 0;
//  }
//  if(pktSize < (LEN_RESET + STATUS_REG_FIX + 1)){
//    CMD_DBG("pktSize is not enough.");
//    return 0;
//  }
//  return cmdPacket(pkt, CMD_RESET, LEN_RESET, NULL, 0);
//}
//
//uint16_t DFRobot_FatCmd::getFlashInfoCmdConfig(uint8_t *pkt, uint16_t pktSize){
//  if(pkt == NULL){
//    CMD_DBG("pkt is NULL");
//    return 0;
//  }
//  if(pktSize < (LEN_RESET + STATUS_REG_FIX + 1)){
//    CMD_DBG("pktSize is not enough.");
//    return 0;
//  }
//  return cmdPacket(pkt, CMD_FLASH_INFO, LEN_FLASH_INFO, NULL, 0);
//}
//
///**
// * @fn getReadAddrCmdPkt
// * @brief 获取读地址命令包结构
// * @param state 指向布尔类型变量的指针， 获取打包状态，true表示打包成功， false表示打包失败
// * @param len   指向uint16_t类型变量的指针， 获取命令包的总长度，单位字节
// * @return uint8_t *类型数组首地址指针，表示命令包的起始地址， NULL表示打包失败
// * @attention 这里返回的数组是用malloc动态分配的，用完之后一定要用free函数释放改内存
// */
//
//uint16_t DFRobot_FatCmd::getReadAddrCmdConfig(uint8_t *pkt, uint16_t pktSize){
//  if(pkt == NULL){
//    CMD_DBG("pkt is NULL");
//    return 0;
//  }
//  if(pktSize < (LEN_RESET + STATUS_REG_FIX + 1)){
//    CMD_DBG("pktSize is not enough.");
//    return 0;
//  }
//  return cmdPacket(pkt, CMD_READ_ADDR, LEN_READ_ADDR, NULL, 0);
//}
//
///**
// * @fn getModifyAddrCmdPkt
// * @brief 获取修改地址命令包结构
// * @param state 指向布尔类型变量的指针， 获取打包状态，true表示打包成功， false表示打包失败
// * @param newAddr 新的7位IIC地址，范围1~127
// * @param len   指向uint16_t类型变量的指针， 获取命令包的总长度，单位字节
// * @return uint8_t *类型数组首地址指针，表示命令包的起始地址， NULL表示打包失败
// * @attention 这里返回的数组是用malloc动态分配的，用完之后一定要用free函数释放改内存
// */
//
//uint16_t DFRobot_FatCmd::getModifyAddrCmdConfig(uint8_t *pkt, uint16_t pktSize, uint8_t newAddr){
//  if(pkt == NULL){
//    CMD_DBG("pkt is NULL");
//    return 0;
//  }
//  if(pktSize < (LEN_RESET + STATUS_REG_FIX + 1)){
//    CMD_DBG("pktSize is not enough.");
//    return 0;
//  }
//  return cmdPacket(pkt, CMD_MODIFY_ADDR, LEN_MODIFY_ADDR, &newAddr, 1);
//}
//
///**
// * @fn getPWDCmdPkt
// * @brief 获取当前工作目录命令包结构
// * @param state 指向布尔类型变量的指针， 获取打包状态，true表示打包成功， false表示打包失败
// * @param len   指向uint16_t类型变量的指针， 获取命令包的总长度，单位字节
// * @return uint8_t *类型数组首地址指针，表示命令包的起始地址， NULL表示打包失败
// * @attention 这里返回的数组是用malloc动态分配的，用完之后一定要用free函数释放改内存
// */
///*uint8_t *DFRobot_FatCmd::getPWDCmdPkt(bool *state, uint16_t *len){
//  if((state == NULL) && (len == NULL)) {
//    CMD_DBG("state or len is NULL");
//    return NULL;
//  }
//  uint8_t cmd[] = {0x55, 0xAA, CMD_GET_PWD, LEN_GET_PWD};
//  uint8_t *pkt = CmdPacket(cmd, NULL, state, len);
//  return pkt;
//}
//*/
///**
// * @fn getReRootDirCmdPkt
// * @brief 获取返回根目录命令包结构
// * @param state 指向布尔类型变量的指针， 获取打包状态，true表示打包成功， false表示打包失败
// * @param len   指向uint16_t类型变量的指针， 获取命令包的总长度，单位字节
// * @return uint8_t *类型数组首地址指针，表示命令包的起始地址， NULL表示打包失败
// * @attention 这里返回的数组是用malloc动态分配的，用完之后一定要用free函数释放改内存
// */
///*uint8_t *DFRobot_FatCmd::getReRootDirCmdPkt(bool *state, uint16_t *len){
//  if((state == NULL) && (len == NULL)) {
//    CMD_DBG("state or len is NULL");
//    return NULL;
//  }
//  uint8_t cmd[] = {0x55, 0xAA, CMD_RE_ROOT_DIR, LEN_RE_ROOT_DIR};
//  uint8_t *pkt = CmdPacket(cmd, NULL, state, len);
//  return pkt;
//}
//*/
///*
//uint8_t *DFRobot_FatCmd::getCdPWDCmdPkt(bool *state, uint8_t *data, uint8_t size, uint16_t *len){
//  if((state == NULL) && (len == NULL)) {
//    CMD_DBG("state or len is NULL");
//    return NULL;
//  }
//  size += LEN_CD_PWD;
//  uint8_t cmd[] = {0x55, 0xAA, CMD_CD_PWD, size};
//  uint8_t *pkt = CmdPacket(cmd, data, state, len);
//  return pkt;
//}
//*/
///*
//uint8_t *DFRobot_FatCmd::getLsCmdPkt(bool *state, uint8_t *data, uint8_t size, uint16_t *len){
//  if((state == NULL) && (len == NULL)) {
//    CMD_DBG("state or len is NULL");
//    return NULL;
//  }
//  size += LEN_LS;
//  uint8_t cmd[] = {0x55, 0xAA, CMD_LS, size};
//  uint8_t *pkt = CmdPacket(cmd, data, state, len);
//  return pkt;
//
//}
//*/
//
//uint16_t DFRobot_FatCmd::getMkdirCmdConfig(uint8_t *pkt, uint16_t pktSize, void *data, uint8_t len){
//  if(pkt == NULL){
//    CMD_DBG("pkt is NULL");
//    return 0;
//  }
//  if(pktSize < (LEN_RESET + STATUS_REG_FIX + 1)){
//    CMD_DBG("pktSize is not enough.");
//    return 0;
//  }
//  return cmdPacket(pkt, CMD_MKDIR, LEN_MKDIR, data, len);
//}
//
//
//uint16_t DFRobot_FatCmd::getRemoveCmdConfig(uint8_t *pkt, uint16_t pktSize, void *data, uint8_t len){
//  if(pkt == NULL){
//    CMD_DBG("pkt is NULL");
//    return 0;
//  }
//  if(pktSize < (LEN_RESET + STATUS_REG_FIX + 1)){
//    CMD_DBG("pktSize is not enough.");
//    return 0;
//  }
//  return cmdPacket(pkt, CMD_REMOVE, LEN_REMOVE, data, len);
//
//}
//
//uint16_t DFRobot_FatCmd::getCreateCmdConfig(uint8_t *pkt, uint16_t pktSize, void *data, uint8_t len){
//  if(pkt == NULL){
//    CMD_DBG("pkt is NULL");
//    return 0;
//  }
//  if(pktSize < (LEN_RESET + STATUS_REG_FIX + 1)){
//    CMD_DBG("pktSize is not enough.");
//    return 0;
//  }
//  return cmdPacket(pkt, CMD_CREATE, LEN_CREATE, data, len);
//}
//
//uint16_t DFRobot_FatCmd::getRmFileCmdConfig(uint8_t *pkt, uint16_t pktSize, void *data, uint8_t len){
//  if(pkt == NULL){
//    CMD_DBG("pkt is NULL");
//    return 0;
//  }
//  if(pktSize < (LEN_RESET + STATUS_REG_FIX + 1)){
//    CMD_DBG("pktSize is not enough.");
//    return 0;
//  }
//  return cmdPacket(pkt, CMD_RM_FILE, LEN_RM_FILE, data, len);
//}
//
//uint16_t DFRobot_FatCmd::getReadFileCmdConfig(uint8_t *pkt, uint16_t pktSize, void *data, uint8_t len){
//  if(pkt == NULL){
//    CMD_DBG("pkt is NULL");
//    return 0;
//  }
//  if(pktSize < (LEN_RESET + STATUS_REG_FIX + 1)){
//    CMD_DBG("pktSize is not enough.");
//    return 0;
//  }
//  return cmdPacket(pkt, CMD_READ_FILE, LEN_READ_FILE, data, len);
//}
//
//
//uint16_t DFRobot_FatCmd::getWriteFileCmdConfig(uint8_t *pkt, uint16_t pktSize, void *data, uint8_t len){
//  if(pkt == NULL){
//    CMD_DBG("pkt is NULL");
//    return 0;
//  }
//  if(pktSize < (LEN_RESET + STATUS_REG_FIX + 1)){
//    CMD_DBG("pktSize is not enough.");
//    return 0;
//  }
//  return cmdPacket(pkt, CMD_WRITE_FILE, LEN_WRITE_FILE, data, len);
//}
//
//uint16_t DFRobot_FatCmd::getOpenFileCmdConfig(uint8_t *pkt, uint16_t pktSize, void *data, uint8_t len){
//  if(pkt == NULL){
//    CMD_DBG("pkt is NULL");
//    return 0;
//  }
//  if(pktSize < (LEN_RESET + STATUS_REG_FIX + 1)){
//    CMD_DBG("pktSize is not enough.");
//    return 0;
//  }
//  return cmdPacket(pkt, CMD_OPEN_FILE, LEN_OPEN_FILE, data, len);
//}
//
//uint16_t DFRobot_FatCmd::getCloseFileCmdConfig(uint8_t *pkt, uint16_t pktSize, void *data, uint8_t len){
//  if(pkt == NULL){
//    CMD_DBG("pkt is NULL");
//    return 0;
//  }
//  if(pktSize < (LEN_RESET + STATUS_REG_FIX + 1)){
//    CMD_DBG("pktSize is not enough.");
//    return 0;
//  }
//  return cmdPacket(pkt, CMD_CLOSE_FILE, LEN_CLOSE_FILE, data, len);
//}
//
//uint16_t DFRobot_FatCmd::getSyncFileCmdConfig(uint8_t *pkt, uint16_t pktSize, void *data, uint8_t len){
//  if(pkt == NULL){
//    CMD_DBG("pkt is NULL");
//    return 0;
//  }
//  if(pktSize < (LEN_RESET + STATUS_REG_FIX + 1)){
//    CMD_DBG("pktSize is not enough.");
//    return 0;
//  }
//  return cmdPacket(pkt, CMD_SYNC_FILE, LEN_SYNC_FILE, data, len);
//}
//
//uint16_t DFRobot_FatCmd::getSeekFileCmdConfig(uint8_t *pkt, uint16_t pktSize, void *data, uint8_t len){
//  if(pkt == NULL){
//    CMD_DBG("pkt is NULL");
//    return 0;
//  }
//  if(pktSize < (LEN_RESET + STATUS_REG_FIX + 1)){
//    CMD_DBG("pktSize is not enough.");
//    return 0;
//  }
//  return cmdPacket(pkt, CMD_SEEK_FILE, LEN_SEEK_FILE, data, len);
//}
//
//bool DFRobot_FatCmd::parseResPktOfFlashInfoCmd(uint8_t *data, uint16_t len, uint8_t *fatType, uint32_t *cap, uint32_t *freeS, uint16_t *fns){
//  if((!data) || (len != LEN_RESP_FLASH_INFO)) return false;
//  if(fatType) *fatType = data[0];
//  if(cap) *cap = (uint32_t)((data[1] << 24) | (data[2] << 16) | (data[3] << 8) | data[4]);
//  if(freeS) *freeS = (uint32_t)((data[5] << 24) | (data[6]) << 16 | (data[7] << 8) | data[8]);
//  if(fns) *fns = (uint32_t)((data[9] << 8) | data[10]);
//  return true;
//}
//
//bool DFRobot_FatCmd::parseResPktOfOpenFileCmd(uint8_t *data, uint16_t len, int8_t *id, uint32_t *curPos, uint32_t *size){
//  if((!data) || (len != LEN_RESP_OPEN_FILE)) return false;
//  if(id) *id = (int8_t)data[0];
//  if(curPos) *curPos = (uint32_t)((data[1] << 24) | (data[2]) << 16 | (data[3] << 8) | data[4]);
//  if(size) *size = (uint32_t)((data[5] << 24) | (data[6]) << 16 | (data[7] << 8) | data[8]);
//  return true;
//}

/*
uint8_t *DFRobot_FatCmd::mallocResPktOfOpenFileCmd(bool *state, uint16_t *len){
  return mallocResponsePkt(LEN_RESP_OPEN_FILE, state, len);
}

uint8_t *DFRobot_FatCmd::mallocResPktOfFlashInfo(bool *state, uint16_t *len){
  return mallocResponsePkt(LEN_RESP_FLASH_INFO, state, len);
}

bool DFRobot_FatCmd::parseResPktOfOpenFileCmd(uint8_t *data, uint16_t len, uint8_t *id, uint32_t *curPos, uint32_t *size){
  if((!data) || (len != LEN_RESP_OPEN_FILE)) return false;
  bool ret = checkCSSum(data, len - 1, data[len-1]);
  if(!ret) return false;
  if(id) *id = data[0];
  if(curPos) *curPos = (data[1] << 24) | (data[2]) << 16 | (data[3] << 8) | data[4];
  if(size) *size = (data[5] << 24) | (data[6]) << 16 | (data[7] << 8) | data[8];
  return true;
}


*/