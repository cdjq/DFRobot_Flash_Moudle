/*!
 * @file DFRobot_FatCmd.cpp
 * @brief 定义 DFRobot_DFR0870_Protocol 类的实现
 * @details 此协议用于Arduino主控同Flash Memory Moudle模块交互，目前能实现的交互内容为：
 * @n CMD_RESET            复位Flash Memory Moudle模块
 * @n CMD_FLASH_INFO       读取Flash Memory Moudle容量存储信息
 * @n CMD_READ_ADDR        读取IIC地址命令
 * @n CMD_SET_ADDR         设置I2C地址命令（此命令模块掉电后生效）
 * @n CMD_OPEN_FILE        打开文件
 * @n CMD_CLOSE_FILE       关闭文件或截断并关闭文件
 * @n CMD_WRITE_FILE       写文件命令
 * @n CMD_READ_FILE        读文件命令
 * @n CMD_SYNC_FILE        同步文件，将缓存数据写入实际文件中的命令
 * @n CMD_SEEK_FILE        设置文件读写指针位置命令
 * @n CMD_MKDIR            创建文件夹命令
 * @n CMD_OPEN_DIR         打开目录命令
 * @n CMD_CLOSE_DIR        关闭目录命令
 * @n CMD_REMOVE           移除文件或目录
 * @n CMD_FILE_ATTR        获取文件属性：0-文件或目录不存在 1-这是一个文件 2-FAT12根目录 3-FAT16根目录 4-FAT32根目录 5-子目录
 * @n CMD_READ_DIR         读取目录下的所有文件项和目录项
 * @n CMD_REWIND           回到读目录起始位置
 * @n CMD_ABSPATH          获取当前目录或文件的绝对路径
 * @n CMD_PARENTDIR        获取当前目录或文件的父级目录路径  
 * @copyright	Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author [Arya](xue.peng@dfrobot.com)
 * @version  V1.0
 * @date  2022-04-28
 * @url https://github.com/DFRobot/DFRobot_Flash_Moudle
 */
#include <Arduino.h>
#include "DFRobot_FatCmd.h"
#include "Wire.h"

///< Define DBG, change 0 to 1 open the DBG, 1 to 0 to close.  
#ifndef CMD_DBG
#if 0
#define CMD_DBG(...) {Serial.print("["); Serial.print(__FUNCTION__); Serial.print("(): "); Serial.print(__LINE__); Serial.print(" ] "); Serial.println(__VA_ARGS__);}
#else
#define CMD_DBG(...)
#endif
#endif

#define REG_I2C_ADDR        0x0001
#define REG_FAT_TYPE        0x0001
#define REG_DISK_CAPACITY_L 0x0002

#define SEND_PKT_PRE_FIX_LEN   3
#define CMD_START           0x01
#define CMD_RESET           0x01  ///< 复位I2C从机命令, 只有应答命令无响应包，成功返回0x53，失败返回0x63
#define CMD_FLASH_INFO      0x02  ///< 读取Flash信息命令
#define CMD_READ_ADDR       0x03  ///< 读取IIC地址命令
#define CMD_SET_ADDR        0x04  ///< 设置I2C地址命令（此命令模块掉电后生效）
#define CMD_OPEN_FILE       0x05  ///< 打开文件命令
#define CMD_CLOSE_FILE      0x06  ///< 关闭文件命令
#define CMD_WRITE_FILE      0x07  ///< 写文件命令
#define CMD_READ_FILE       0x08  ///< 读文件命令
#define CMD_SYNC_FILE       0x09  ///< 同步文件，将缓存数据写入实际文件中的命令
#define CMD_SEEK_FILE       0x0A  ///< 设置文件读写指针位置命令
#define CMD_MKDIR           0x0B  ///< 创建文件夹命令
#define CMD_OPEN_DIR        0x0C  ///< 打开目录命令
#define CMD_CLOSE_DIR       0x0D  ///< 关闭目录命令
#define CMD_REMOVE          0x0E  ///< 移除文件或目录
#define CMD_FILE_ATTR       0x0F  ///< 获取文件属性，是目录，还是文件，如果是文件，返回TYPE_FAT_FILE_NORMAL，TYPE_FAT_FILE_SUBDIR，不存在
#define CMD_READ_DIR        0x10  ///< 读取目录下的所有文件和目录
#define CMD_REWIND          0x11  ///< 回到读目录起始位置
#define CMD_ABSPATH         0x12  ///< 获取当前目录或文件的绝对路径
#define CMD_PARENTDIR       0x13  ///< 获取当前目录或文件的父级目录路径  
#define CMD_END             CMD_PARENTDIR

#define STATUS_SUCCESS      0x53  ///< 响应成功状态   
#define STATUS_FAILED       0x63  ///< 响应成功状态  

#define IIC_MAX_TRANSFER    32     ///< I2C最大传输数据

#define DEBUG_TIMEOUT_MS    20000

static DFRobot_Driver *_drv = NULL;

typedef struct{
  uint8_t cmd;           /**< 命令，范围0x00~0x0E,0x0F及之后为无效命令 */
  union{
    struct{
      uint8_t bit0:1;
      uint8_t bit1:1;
      uint8_t rsv: 6;
    };
    uint8_t fix;           /**< 固定判定，bit0为1表示sendLen为固定数据， bit1为1表示responseLen为固定数据*/
  };
  uint16_t sendLen;      /**< 此命令发送包有效数据的长度，如果为0xFFFF则代表不是固定反应  */
  uint16_t responseLen;  /**< 此命令响应包有效数据的长度，如果为0xFFFF则代表不是固定反应  */
}__attribute__ ((packed)) sCmdStruct_t, *pCmdStruct_t;

typedef struct{
  uint8_t cmd;    /**< 命令，范围0x00~0x0E,0x0F及之后为无效命令 */
  uint8_t lenL;    /**< 除去具体命令后的数据长度，这个一般用来计算buf数组的长度 */
  uint8_t lenH;
  uint8_t buf[0]; /**< 0长度数组，它的大小取决于上一个变量len的值 */
}__attribute__ ((packed)) sSendCmdPkt_t, *pSendCmdPkt_t;

typedef struct{
  uint8_t state;    /**< 响应包状态，0x53，响应成功，0x63，响应失败 */
  uint8_t cmd;      /**< 命令的响应包 */
  uint8_t lenL;     /**< 除去具体命令后的数据长度低位，这个一般用来计算buf数组的长度 */
  uint8_t lenH;     /**< 除去具体命令后的数据长度高位，这个一般用来计算buf数组的长度 */
  uint8_t buf[0];   /**< 0长度数组，它的大小取决于上一个变量len的值 */
}__attribute__ ((packed)) sResponseCmdPkt_t, *pResponseCmdPkt_t;
//const uint8_t SIMKAIFont12ptBitmaps[] PROGMEM
static const uint8_t  DFR0870_CMD_STRUCT[] PROGMEM = {
  CMD_RESET,      0x03, 0, 0 ,
  CMD_FLASH_INFO, 0x03, 0, 11,
  CMD_READ_ADDR,  0x03, 0, 1 ,
  CMD_SET_ADDR,   0x03, 1, 0 ,
  CMD_OPEN_FILE,  0x02, 2, 9 ,
  CMD_CLOSE_FILE, 0x03, 2, 0 ,
  CMD_WRITE_FILE, 0x02, 1, 2 ,
  CMD_READ_FILE,  0x01, 3, 0 ,
  CMD_SYNC_FILE,  0x03, 1, 0 ,
  CMD_SEEK_FILE,  0x03, 5, 4 ,
  CMD_MKDIR,      0x00, 1, 0 ,
  CMD_OPEN_DIR,   0x00, 1, 1 ,
  CMD_CLOSE_DIR,  0x01, 1, 0 ,
  CMD_REMOVE,     0x02, 1, 0 ,
  CMD_FILE_ATTR,  0x02, 1, 1 ,
  CMD_READ_DIR,   0x01, 1, 0 ,
  CMD_REWIND,     0x03, 1, 0 ,
  CMD_ABSPATH,    0x01, 2, 0 ,
  CMD_PARENTDIR,  0x01, 2, 0 ,
};

static sCmdStruct_t getCmdStructConfig(uint8_t cmd){
  sCmdStruct_t cmdStu;
  if(cmd < CMD_START || cmd > CMD_END){
    memset(&cmdStu, 0, sizeof(sCmdStruct_t));
    return cmdStu;
  }
  uint8_t *addr = (uint8_t *)(DFR0870_CMD_STRUCT + 4 * (cmd - 1));
  cmdStu.cmd         = pgm_read_byte(addr++);
  cmdStu.fix         = pgm_read_byte(addr++);
  cmdStu.sendLen     = (uint16_t)pgm_read_byte(addr++);
  cmdStu.responseLen = (uint16_t)pgm_read_byte(addr);

  return cmdStu;
}

void * DFRobot_DFR0870_Protocol::recvCmdResponsePkt(uint8_t cmd){
  if(cmd < CMD_START || cmd > CMD_END){
    CMD_DBG("cmd is error!");
    return NULL;
  }
  sResponseCmdPkt_t responsePkt;
  pResponseCmdPkt_t responsePktPtr = NULL;
  uint16_t length = 0;
  uint32_t t = millis();
  while(millis() - t < DEBUG_TIMEOUT_MS/*time_ms*/){
    readResponseData(&responsePkt.state, 1);
RECVYIMEOUTFLAG:
    delay(50);
    yield();
    if((responsePkt.state == STATUS_SUCCESS) || (responsePkt.state == STATUS_FAILED)) {
      readResponseData(&responsePkt.cmd, 1);
      if(responsePkt.cmd == cmd){
        readResponseData(&responsePkt.lenL, 2);
        length = (responsePkt.lenH << 8) | responsePkt.lenL;
        CMD_DBG(responsePkt.lenH,HEX);
        CMD_DBG(responsePkt.lenL,HEX);
        CMD_DBG(length,HEX);
        responsePktPtr = (pResponseCmdPkt_t)malloc(sizeof(sResponseCmdPkt_t) + length);
        if(responsePktPtr == NULL){
          CMD_DBG("responsePktPtr malloc failed!");
          CMD_DBG(cmd, HEX);
          CMD_DBG(length);
          return NULL;
        }
        memcpy(responsePktPtr, &responsePkt, sizeof(sResponseCmdPkt_t));
        if(length) readResponseData(responsePktPtr->buf, length);
        CMD_DBG(millis() - t);
        return responsePktPtr;
      }else{
        responsePkt.state = responsePkt.cmd;
        goto RECVYIMEOUTFLAG;
      }
    }
    
  }
  CMD_DBG("Time out!");
  return NULL;
}
void * DFRobot_DFR0870_Protocol::packedCmdPacket(uint8_t cmd, uint16_t len){
  if(cmd < CMD_START || cmd > CMD_END){
    CMD_DBG("cmd is error!");
    return NULL;
  }
  sCmdStruct_t cmdStu = getCmdStructConfig(cmd);
  pSendCmdPkt_t sendPkt = (pSendCmdPkt_t)malloc(sizeof(sSendCmdPkt_t) + cmdStu.sendLen + len);
  if(sendPkt){
    len += cmdStu.sendLen;
    sendPkt->cmd = cmd;
    sendPkt->lenL = len & 0xFF;
    sendPkt->lenH = (len >> 8) & 0xFF;
  }
  return sendPkt;
}

bool DFRobot_DFR0870_Protocol::writeCmdPacket(void *pData, uint16_t size, bool endflag){
  CMD_DBG((uint32_t)((uint32_t *)_drv), HEX);
  if(_drv) {
    CMD_DBG();
    bool ret = _drv->sendData(pData, size, endflag);
    CMD_DBG((uint32_t)((uint32_t *)_drv), HEX);
    return ret;

  }
  CMD_DBG();
  return false;
}

bool DFRobot_DFR0870_Protocol::readResponseData(void *pData, uint16_t size, bool endflag){
  if(_drv) return _drv->recvData(pData, size, endflag);
  return false;
}

bool DFRobot_DFR0870_Protocol::begin(DFRobot_Driver *drv){
  _drv = drv;
  if(_drv == NULL) return false;
  return true;
}

bool DFRobot_DFR0870_Protocol::reset(){
  sCmdStruct_t cmdStu = getCmdStructConfig(CMD_RESET);
  pSendCmdPkt_t sendPkt = (pSendCmdPkt_t)packedCmdPacket(cmdStu.cmd, 0);
  
  if(sendPkt == NULL){
    CMD_DBG("reset: packed malloc failed.");
    return false;
  }
  if(writeCmdPacket(sendPkt, SEND_PKT_PRE_FIX_LEN + ((sendPkt->lenH << 8) | sendPkt->lenL)) == false){
    CMD_DBG("reset: send packet fail.");
    free(sendPkt);
    return false;
  }
  free(sendPkt);

  pResponseCmdPkt_t responsePkt = (pResponseCmdPkt_t)recvCmdResponsePkt(CMD_RESET);
  if(responsePkt == NULL){
    CMD_DBG("reset: response packet fail.");
    return false;
  }
  if((responsePkt->state != STATUS_SUCCESS) || (responsePkt->cmd != CMD_RESET) || (((responsePkt->lenH << 8) | responsePkt->lenL) != cmdStu.responseLen)){
    CMD_DBG("reset: response recv packet failrd.");
    free(responsePkt);
    return false;
  }
  //Serial.print("state=");Serial.println(responsePkt->state,HEX);
  //Serial.print("cmd=");Serial.println(responsePkt->cmd,HEX);
  //Serial.print("length=");Serial.println((responsePkt->lenH << 8) | responsePkt->lenL, HEX);
  free(responsePkt);
  return true;
}

bool DFRobot_DFR0870_Protocol::getFlashInfo(uint8_t *fatType, uint32_t *capacity, uint32_t *freeSec, uint16_t *maxFileNums){
  sCmdStruct_t cmdStu = getCmdStructConfig(CMD_FLASH_INFO);
  pSendCmdPkt_t sendPkt = (pSendCmdPkt_t)packedCmdPacket(cmdStu.cmd, 0);
  
  if(sendPkt == NULL){
    CMD_DBG("FlashInfo: packed malloc failed.");
    return false;
  }
  if(writeCmdPacket(sendPkt, SEND_PKT_PRE_FIX_LEN + ((sendPkt->lenH << 8) | sendPkt->lenL)) == false){
    CMD_DBG("FlashInfo: send packet fail.");
    free(sendPkt);
    return false;
  }
  free(sendPkt);

  pResponseCmdPkt_t responsePkt = (pResponseCmdPkt_t)recvCmdResponsePkt(CMD_FLASH_INFO);
  if(responsePkt == NULL){
    CMD_DBG("FlashInfo: response packet fail.");
    return false;
  }
  if((responsePkt->state != STATUS_SUCCESS) || (responsePkt->cmd != CMD_FLASH_INFO) || (((responsePkt->lenH << 8) | responsePkt->lenL) != cmdStu.responseLen)){
    CMD_DBG("FlashInfo: response recv packet failrd.");
    free(responsePkt);
    return false;
  }
  if(fatType) *fatType = responsePkt->buf[0];
  if(capacity) *capacity = responsePkt->buf[1] | (responsePkt->buf[2] << 8) | (responsePkt->buf[3] << 16) | (responsePkt->buf[4] << 24);
  if(freeSec) *freeSec = responsePkt->buf[5] | (responsePkt->buf[6] << 8) | (responsePkt->buf[7] << 16) | (responsePkt->buf[8] << 24);
  if(maxFileNums) *maxFileNums = responsePkt->buf[9] | (responsePkt->buf[10] << 8);
  if(fatType) CMD_DBG(*fatType, HEX);
  if(capacity) CMD_DBG(*capacity, HEX);
  if(freeSec) CMD_DBG(*freeSec, HEX);
  if(maxFileNums) CMD_DBG(*maxFileNums, HEX);

  free(responsePkt);
  return true;
}

uint8_t DFRobot_DFR0870_Protocol::getI2CAddress(){
  uint8_t addr = 0;
   sCmdStruct_t cmdStu = getCmdStructConfig(CMD_READ_ADDR);
  pSendCmdPkt_t sendPkt = (pSendCmdPkt_t)packedCmdPacket(cmdStu.cmd, 0);
  if(sendPkt == NULL){
    CMD_DBG("FlashInfo: packed malloc failed.");
    return false;
  }
  if(writeCmdPacket(sendPkt, SEND_PKT_PRE_FIX_LEN + ((sendPkt->lenH << 8) | sendPkt->lenL)) == false){
    CMD_DBG("FlashInfo: send packet fail.");
    free(sendPkt);
    return false;
  }
  free(sendPkt);

  pResponseCmdPkt_t responsePkt = (pResponseCmdPkt_t)recvCmdResponsePkt(CMD_READ_ADDR);
  if(responsePkt == NULL){
    CMD_DBG("FlashInfo: response packet fail.");
    return false;
  }
  if((responsePkt->state != STATUS_SUCCESS) || (responsePkt->cmd != CMD_READ_ADDR) || (((responsePkt->lenH << 8) | responsePkt->lenL) != cmdStu.responseLen)){
    CMD_DBG("FlashInfo: response recv packet failrd.");
    free(responsePkt);
    return false;
  }
  addr = responsePkt->buf[0];
  CMD_DBG(addr, HEX);

  free(responsePkt);
  return addr;
}

bool DFRobot_DFR0870_Protocol::setI2CAddress(uint8_t addr){
  if(addr < 1 || addr > 0x7F){
    CMD_DBG("addr range is error.");
    return false;
  }
  sCmdStruct_t cmdStu = getCmdStructConfig(CMD_SET_ADDR);
  pSendCmdPkt_t sendPkt = (pSendCmdPkt_t)packedCmdPacket(cmdStu.cmd, 0);

  if(sendPkt == NULL){
    CMD_DBG("packed malloc failed.");
    return false;
  }
  sendPkt->buf[0] = addr;
  if(writeCmdPacket(sendPkt, SEND_PKT_PRE_FIX_LEN + ((sendPkt->lenH << 8) | sendPkt->lenL)) == false){
    CMD_DBG("send packet fail.");
    free(sendPkt);
    return false;
  }
  free(sendPkt);

  pResponseCmdPkt_t responsePkt = (pResponseCmdPkt_t)recvCmdResponsePkt(CMD_SET_ADDR);
  if(responsePkt == NULL){
    CMD_DBG("response packet fail.");
    return false;
  }
  if((responsePkt->state != STATUS_SUCCESS) || (responsePkt->cmd != CMD_SET_ADDR) || (((responsePkt->lenH << 8) | responsePkt->lenL) != cmdStu.responseLen)){
    CMD_DBG("response recv packet failrd.");
    free(responsePkt);
    return false;
  }
  
  free(responsePkt);
  return true;
}

bool DFRobot_DFR0870_Protocol::openFile(const char *name, int8_t pid, uint8_t oflag, int8_t *id, uint32_t *curPos, uint32_t *size){
  sCmdStruct_t cmdStu = getCmdStructConfig(CMD_OPEN_FILE);
  pSendCmdPkt_t sendPkt = (pSendCmdPkt_t)packedCmdPacket(cmdStu.cmd, strlen(name) + 1);

  if(sendPkt == NULL){
    CMD_DBG("CMD_OPEN_FILE packed malloc failed.");
    return false;
  }
  sendPkt->buf[0] = (uint8_t)pid;
  sendPkt->buf[1] = oflag;
  memcpy(&sendPkt->buf[2], name, strlen(name));
  sendPkt->buf[strlen(name)+2] = '\0';
  if(writeCmdPacket(sendPkt, (SEND_PKT_PRE_FIX_LEN + ((sendPkt->lenH << 8) | sendPkt->lenL))) == false){
    CMD_DBG("CMD_OPEN_FILE send packet fail.");
    free(sendPkt);
    return false;
  }
  free(sendPkt);

  pResponseCmdPkt_t responsePkt = (pResponseCmdPkt_t)recvCmdResponsePkt(CMD_OPEN_FILE);
  if(responsePkt == NULL){
    CMD_DBG("CMD_OPEN_FILE response packet fail.");
    return false;
  }
  if((responsePkt->state != STATUS_SUCCESS) || (responsePkt->cmd != CMD_OPEN_FILE) || (((responsePkt->lenH << 8) | responsePkt->lenL) != cmdStu.responseLen)){
    CMD_DBG("response recv packet failrd.");
    free(responsePkt);
    return false;
  }
  if(id)     *id     = responsePkt->buf[0];
  if(curPos) *curPos = (responsePkt->buf[4] << 24) | (responsePkt->buf[3] << 16) | (responsePkt->buf[2] << 8) | responsePkt->buf[1];
  if(size)   *size   = (responsePkt->buf[8] << 24) | (responsePkt->buf[7] << 16) | (responsePkt->buf[6] << 8) | responsePkt->buf[5];
  if(id)     CMD_DBG(*id, HEX);
  if(curPos) CMD_DBG(*curPos, HEX);    
  if(size)   CMD_DBG(*size, HEX);       
  return true;
}

bool DFRobot_DFR0870_Protocol::closeFile(int8_t id, bool truncate){
  sCmdStruct_t cmdStu = getCmdStructConfig(CMD_CLOSE_FILE);
  pSendCmdPkt_t sendPkt = (pSendCmdPkt_t)packedCmdPacket(cmdStu.cmd, 0);

  if(sendPkt == NULL){
    CMD_DBG("CMD_CLOSE_FILE packed malloc failed.");
    return false;
  }
  sendPkt->buf[0] = (uint8_t)id;
  sendPkt->buf[1] = (uint8_t)truncate;
  if(writeCmdPacket(sendPkt, (SEND_PKT_PRE_FIX_LEN + ((sendPkt->lenH << 8) | sendPkt->lenL))) == false){
    CMD_DBG("CMD_CLOSE_FILE send packet fail.");
    free(sendPkt);
    return false;
  }
  free(sendPkt);

  pResponseCmdPkt_t responsePkt = (pResponseCmdPkt_t)recvCmdResponsePkt(CMD_CLOSE_FILE);
  if(responsePkt == NULL){
    CMD_DBG("CMD_CLOSE_FILE response packet fail.");
    return false;
  }
  if((responsePkt->state != STATUS_SUCCESS) || (responsePkt->cmd != CMD_CLOSE_FILE) || (((responsePkt->lenH << 8) | responsePkt->lenL) != cmdStu.responseLen)){
    CMD_DBG("response recv packet failrd.");
    free(responsePkt);
    return false;
  }    
  return true;
}

uint16_t DFRobot_DFR0870_Protocol::writeFile(int8_t id, void *data, uint16_t len){
  if(_drv == NULL) return 0;
  uint8_t *pBuf = (uint8_t *)data;
  uint16_t remain = len;
  sCmdStruct_t cmdStu = getCmdStructConfig(CMD_WRITE_FILE);
  pSendCmdPkt_t sendPkt = (pSendCmdPkt_t)packedCmdPacket(cmdStu.cmd, 0);

  if(sendPkt == NULL){
    CMD_DBG("CMD_WRITE_FILE packed malloc failed.");
    return 0;
  }
  sendPkt->lenL = (len + 1) & 0xFF;
  sendPkt->lenH = ((len + 1) >> 8) & 0xFF;
  sendPkt->buf[0] = (uint8_t)id;
  
  bool flag = _drv->sendData(sendPkt, SEND_PKT_PRE_FIX_LEN + 1, false);

  if(!flag) return 0;
  free(sendPkt);
  flag = _drv->sendData(data, len, true);
  if(!flag) return 0;
  
  pResponseCmdPkt_t responsePkt = (pResponseCmdPkt_t)recvCmdResponsePkt(CMD_WRITE_FILE);
  if(responsePkt == NULL){
    CMD_DBG("CMD_WRITE_FILE response packet fail.");
    return 0;
  }
  if((responsePkt->state != STATUS_SUCCESS) || (responsePkt->cmd != CMD_WRITE_FILE) || (((responsePkt->lenH << 8) | responsePkt->lenL) != cmdStu.responseLen)){
    CMD_DBG("response recv packet failrd.");
    free(responsePkt);
    return 0;
  }  
  uint16_t total = (responsePkt->buf[1] << 8) | responsePkt->buf[0];
  CMD_DBG(total);
  free(responsePkt);
  return total;
}

uint16_t DFRobot_DFR0870_Protocol::readFile(int8_t id, void *data, uint16_t len){
  uint16_t total = 0;
  sCmdStruct_t cmdStu = getCmdStructConfig(CMD_READ_FILE);
  pSendCmdPkt_t sendPkt = (pSendCmdPkt_t)packedCmdPacket(cmdStu.cmd, 0);

  if(sendPkt == NULL){
    CMD_DBG("CMD_READ_ADDR packed malloc failed.");
    return 0;
  }
  sendPkt->buf[0] = (uint8_t)id;
  sendPkt->buf[1] = len & 0xFF;
  sendPkt->buf[2] = (len >> 8) & 0xFF;
  if(writeCmdPacket(sendPkt, (SEND_PKT_PRE_FIX_LEN + ((sendPkt->lenH << 8) | sendPkt->lenL))) == false){
    CMD_DBG("CMD_READ_ADDR send packet fail.");
    free(sendPkt);
    return 0;
  }
  free(sendPkt);
  sResponseCmdPkt_t responsePkt;
  uint16_t length = 0;
  uint16_t recvsize = 0;
  uint32_t t = millis();
  while(millis() - t < DEBUG_TIMEOUT_MS/*time_ms*/){
    delay(50);
    yield();
    readResponseData(&responsePkt.state, 1);
MILLISLOOP:
    if((responsePkt.state == STATUS_SUCCESS) || (responsePkt.state == STATUS_FAILED)){
      readResponseData(&responsePkt.cmd, 1);
      if(responsePkt.cmd == CMD_READ_FILE){
        readResponseData(&responsePkt.lenL, 2);
        length = (responsePkt.lenH << 8) | responsePkt.lenL;
        if(responsePkt.state == STATUS_SUCCESS){
          total = (len > length) ? length : len;
          readResponseData(data, total);
          return total;
        }else{
          return 0;
        }
      }else{
        responsePkt.state = responsePkt.cmd;
        goto MILLISLOOP;
      }
    }
  }
  return 0;
}

bool DFRobot_DFR0870_Protocol::sync(int8_t id){

  sCmdStruct_t cmdStu = getCmdStructConfig(CMD_SYNC_FILE);
  pSendCmdPkt_t sendPkt = (pSendCmdPkt_t)packedCmdPacket(cmdStu.cmd, 0);

  if(sendPkt == NULL){
    CMD_DBG("FlashInfo: packed malloc failed.");
    return false;
  }
  sendPkt->buf[0] = (uint8_t)id;
  if(writeCmdPacket(sendPkt, (SEND_PKT_PRE_FIX_LEN + ((sendPkt->lenH << 8) | sendPkt->lenL))) == false){
    CMD_DBG("send packet fail.");
    free(sendPkt);
    return false;
  }
  free(sendPkt);

  pResponseCmdPkt_t responsePkt = (pResponseCmdPkt_t)recvCmdResponsePkt(CMD_SYNC_FILE);
  if(responsePkt == NULL){
    CMD_DBG("response packet fail.");
    return false;
  }
  if((responsePkt->state != STATUS_SUCCESS) || (responsePkt->cmd != CMD_SYNC_FILE) || (((responsePkt->lenH << 8) | responsePkt->lenL) != cmdStu.responseLen)){
    CMD_DBG("response recv packet failrd.");
    free(responsePkt);
    return false;
  }
  free(responsePkt);
  return true;
}

bool DFRobot_DFR0870_Protocol::seekFile(int8_t id, uint32_t pos){
  sCmdStruct_t cmdStu = getCmdStructConfig(CMD_SEEK_FILE);
  pSendCmdPkt_t sendPkt = (pSendCmdPkt_t)packedCmdPacket(cmdStu.cmd, 0);
  
  if(sendPkt == NULL){
    CMD_DBG("CMD_SEEK_FILE packed malloc failed.");
    return false;
  }
  sendPkt->buf[0] = (uint8_t)id;
  sendPkt->buf[1] = pos & 0xFF;
  sendPkt->buf[2] = (pos >> 8) & 0xFF;
  sendPkt->buf[3] = (pos >> 16) & 0xFF;
  sendPkt->buf[4] = (pos >> 24) & 0xFF;
  if(writeCmdPacket(sendPkt, (SEND_PKT_PRE_FIX_LEN + ((sendPkt->lenH << 8) | sendPkt->lenL))) == false){
    CMD_DBG("CMD_SEEK_FILE send packet fail.");
    free(sendPkt);
    return 0;
  }
  free(sendPkt);

  pResponseCmdPkt_t responsePkt = (pResponseCmdPkt_t)recvCmdResponsePkt(CMD_SEEK_FILE);
  if(responsePkt == NULL){
    CMD_DBG("CMD_SEEK_FILE response packet fail.");
    return false;
  }
  if((responsePkt->state != STATUS_SUCCESS) || (responsePkt->cmd != CMD_SEEK_FILE) || (cmdStu.responseLen != ((responsePkt->lenH << 8) | responsePkt->lenL))){
    CMD_DBG("CMD_SEEK_FILE response recv packet failrd.");
    free(responsePkt);
    return false;
  }
  uint32_t curpos = (responsePkt->buf[3] << 24) | (responsePkt->buf[2] << 16) | (responsePkt->buf[1] << 8) | responsePkt->buf[0];
  CMD_DBG(curpos);
  CMD_DBG(pos);
  return true;
}



bool DFRobot_DFR0870_Protocol::newDirectory(const char *name, int8_t pid){
  sCmdStruct_t cmdStu = getCmdStructConfig(CMD_MKDIR);
  pSendCmdPkt_t sendPkt = (pSendCmdPkt_t)packedCmdPacket(cmdStu.cmd, strlen(name) + 1);
  
  if(sendPkt == NULL){
    CMD_DBG("FlashInfo: packed malloc failed.");
    return false;
  }
  sendPkt->buf[0] = (uint8_t)pid;
  memcpy(&sendPkt->buf[1], name, strlen(name));
  sendPkt->buf[strlen(name)+1] = '\0';
  CMD_DBG(SEND_PKT_PRE_FIX_LEN + ((sendPkt->lenH << 8) | sendPkt->lenL));
  if(writeCmdPacket(sendPkt, (SEND_PKT_PRE_FIX_LEN + ((sendPkt->lenH << 8) | sendPkt->lenL))) == false){
    CMD_DBG("send packet fail.");
    free(sendPkt);
    return false;
  }
  free(sendPkt);
  CMD_DBG();
  pResponseCmdPkt_t responsePkt = (pResponseCmdPkt_t)recvCmdResponsePkt(CMD_MKDIR);
  if(responsePkt == NULL){
    CMD_DBG("response packet fail.");
    return false;
  }
  if((responsePkt->state != STATUS_SUCCESS) || (responsePkt->cmd != CMD_MKDIR) || (cmdStu.responseLen != ((responsePkt->lenH << 8) | responsePkt->lenL))){
    CMD_DBG("response recv packet failrd.");
    free(responsePkt);
    return false;
  }
  free(responsePkt);
  return true;
}

bool DFRobot_DFR0870_Protocol::openDirectory(const char *name, int8_t pid, int8_t *id){
  sCmdStruct_t cmdStu = getCmdStructConfig(CMD_OPEN_DIR);
  pSendCmdPkt_t sendPkt = (pSendCmdPkt_t)packedCmdPacket(cmdStu.cmd, strlen(name) + 1);
  
  if(sendPkt == NULL){
    CMD_DBG("FlashInfo: packed malloc failed.");
    return false;
  }
  sendPkt->buf[0] = (uint8_t)pid;
  memcpy(&sendPkt->buf[1], name, strlen(name));
  sendPkt->buf[strlen(name)+1] = '\0';
  CMD_DBG(SEND_PKT_PRE_FIX_LEN + ((sendPkt->lenH << 8) | sendPkt->lenL));
  if(writeCmdPacket(sendPkt, (SEND_PKT_PRE_FIX_LEN + ((sendPkt->lenH << 8) | sendPkt->lenL))) == false){
    CMD_DBG("send packet fail.");
    free(sendPkt);
    return false;
  }
  free(sendPkt);

  pResponseCmdPkt_t responsePkt = (pResponseCmdPkt_t)recvCmdResponsePkt(CMD_OPEN_DIR);
  if(responsePkt == NULL){
    CMD_DBG("response packet fail.");
    return false;
  }
  if((responsePkt->state != STATUS_SUCCESS) || (responsePkt->cmd != CMD_OPEN_DIR) ){
    CMD_DBG("response recv packet failrd.");
    free(responsePkt);
    return false;
  }
  if(id) *id = responsePkt->buf[0];
  uint16_t length = (responsePkt->lenH << 8) | responsePkt->lenL;
  char parent[length];
  memcpy(parent, &responsePkt->buf[1], length-1);
  parent[length - 1] = '\0';
  CMD_DBG(length);
  CMD_DBG(parent);
  if(id) CMD_DBG(*id,HEX);
  free(responsePkt);
  return true;
}

bool DFRobot_DFR0870_Protocol::closeDirectory(int8_t id){
  sCmdStruct_t cmdStu = getCmdStructConfig(CMD_CLOSE_DIR);
  pSendCmdPkt_t sendPkt = (pSendCmdPkt_t)packedCmdPacket(cmdStu.cmd, 0);
  
  if(sendPkt == NULL){
    CMD_DBG("FlashInfo: packed malloc failed.");
    return false;
  }
  sendPkt->buf[0] = (uint8_t)id;
  if(writeCmdPacket(sendPkt, (SEND_PKT_PRE_FIX_LEN + ((sendPkt->lenH << 8) | sendPkt->lenL))) == false){
    CMD_DBG("send packet fail.");
    free(sendPkt);
    return false;
  }
  free(sendPkt);

  pResponseCmdPkt_t responsePkt = (pResponseCmdPkt_t)recvCmdResponsePkt(CMD_CLOSE_DIR);
  if(responsePkt == NULL){
    CMD_DBG("response packet fail.");
    return false;
  }
  if((responsePkt->state != STATUS_SUCCESS) || (responsePkt->cmd != CMD_CLOSE_DIR) ){
    CMD_DBG("response recv packet failrd.");
    free(responsePkt);
    return false;
  }
  free(responsePkt);
  return true;
}

 bool DFRobot_DFR0870_Protocol::remove(int8_t pid, char *name){
  uint8_t attr = 0;
  sCmdStruct_t cmdStu = getCmdStructConfig(CMD_REMOVE);
  pSendCmdPkt_t sendPkt = (pSendCmdPkt_t)packedCmdPacket(cmdStu.cmd, strlen(name) + 1);
  
  if(sendPkt == NULL){
    CMD_DBG("FlashInfo: packed malloc failed.");
    return false;
  }
  sendPkt->buf[0] = (uint8_t)pid;
  memcpy(&sendPkt->buf[1], name, strlen(name));
  sendPkt->buf[strlen(name)+1] = '\0';
  
  if(writeCmdPacket(sendPkt, (SEND_PKT_PRE_FIX_LEN + ((sendPkt->lenH << 8) | sendPkt->lenL))) == false){
    CMD_DBG("send packet fail.");
    free(sendPkt);
    return false;
  }
  free(sendPkt);

  pResponseCmdPkt_t responsePkt = (pResponseCmdPkt_t)recvCmdResponsePkt(CMD_REMOVE);
  if(responsePkt == NULL){
    CMD_DBG("response packet fail.");
    return false;
  }
  if((responsePkt->state != STATUS_SUCCESS) || (responsePkt->cmd != CMD_REMOVE) || (((responsePkt->lenH << 8) | responsePkt->lenL) != cmdStu.responseLen) ){
    CMD_DBG("response recv packet failrd.");
    free(responsePkt);
    return false;
  }
  free(responsePkt);
  return true;
}

uint8_t DFRobot_DFR0870_Protocol::getFileAttribute(int8_t pid, char *name){
  uint8_t attr = 0;
  sCmdStruct_t cmdStu = getCmdStructConfig(CMD_FILE_ATTR);
  pSendCmdPkt_t sendPkt = (pSendCmdPkt_t)packedCmdPacket(cmdStu.cmd, strlen(name) + 1);
  
  if(sendPkt == NULL){
    CMD_DBG("FlashInfo: packed malloc failed.");
    return 0;
  }
  sendPkt->buf[0] = (uint8_t)pid;
  memcpy(&sendPkt->buf[1], name, strlen(name));
  sendPkt->buf[strlen(name)+1] = '\0';
  
  if(writeCmdPacket(sendPkt, (SEND_PKT_PRE_FIX_LEN + ((sendPkt->lenH << 8) | sendPkt->lenL))) == false){
    CMD_DBG("send packet fail.");
    free(sendPkt);
    return 0;
  }
  free(sendPkt);

  pResponseCmdPkt_t responsePkt = (pResponseCmdPkt_t)recvCmdResponsePkt(CMD_FILE_ATTR);
  if(responsePkt == NULL){
    CMD_DBG("response packet fail.");
    return 0;
  }
  if((responsePkt->state != STATUS_SUCCESS) || (responsePkt->cmd != CMD_FILE_ATTR) || (((responsePkt->lenH << 8) | responsePkt->lenL) != cmdStu.responseLen) ){
    CMD_DBG("response recv packet failrd.");
    free(responsePkt);
    return 0;
  }
  attr = responsePkt->buf[0];
  free(responsePkt);
  CMD_DBG(attr);
  return attr;
}

bool DFRobot_DFR0870_Protocol::readDirectory(int8_t id,  char *name, uint16_t namebufsize){
  sCmdStruct_t cmdStu = getCmdStructConfig(CMD_READ_DIR);
  pSendCmdPkt_t sendPkt = (pSendCmdPkt_t)packedCmdPacket(cmdStu.cmd, 0);
  
  if(sendPkt == NULL){
    CMD_DBG("FlashInfo: packed malloc failed.");
    return false;
  }
  sendPkt->buf[0] = (uint8_t)id;
  if(writeCmdPacket(sendPkt, (SEND_PKT_PRE_FIX_LEN + ((sendPkt->lenH << 8) | sendPkt->lenL))) == false){
    CMD_DBG("send packet fail.");
    free(sendPkt);
    return false;
  }
  free(sendPkt);

  pResponseCmdPkt_t responsePkt = (pResponseCmdPkt_t)recvCmdResponsePkt(CMD_READ_DIR);
  if(responsePkt == NULL){
    CMD_DBG("response packet fail.");
    return false;
  }
  if((responsePkt->state != STATUS_SUCCESS) || (responsePkt->cmd != CMD_READ_DIR) || (((responsePkt->lenH << 8) | responsePkt->lenL) > namebufsize)){
    CMD_DBG(responsePkt->state,HEX);
    CMD_DBG(responsePkt->cmd,HEX);
    CMD_DBG((responsePkt->lenH << 8) | responsePkt->lenL);
    CMD_DBG(namebufsize);
    CMD_DBG("response recv packet failrd.");
    free(responsePkt);
    return false;
  }

  namebufsize = (responsePkt->lenH << 8) | responsePkt->lenL;
  if(namebufsize == 0) return false;
  memcpy(name, responsePkt->buf, namebufsize);
  CMD_DBG(name);
  free(responsePkt);
  return true;
}

bool DFRobot_DFR0870_Protocol::rewind(int8_t id){
  sCmdStruct_t cmdStu = getCmdStructConfig(CMD_REWIND);
  pSendCmdPkt_t sendPkt = (pSendCmdPkt_t)packedCmdPacket(cmdStu.cmd, 0);
  
  if(sendPkt == NULL){
    CMD_DBG("FlashInfo: packed malloc failed.");
    return false;
  }
  sendPkt->buf[0] = (uint8_t)id;
  if(writeCmdPacket(sendPkt, (SEND_PKT_PRE_FIX_LEN + ((sendPkt->lenH << 8) | sendPkt->lenL))) == false){
    CMD_DBG("send packet fail.");
    free(sendPkt);
    return false;
  }
  free(sendPkt);

  pResponseCmdPkt_t responsePkt = (pResponseCmdPkt_t)recvCmdResponsePkt(CMD_REWIND);
  if(responsePkt == NULL){
    CMD_DBG("response packet fail.");
    return false;
  }
  if((responsePkt->state != STATUS_SUCCESS) || (responsePkt->cmd != CMD_REWIND) || (((responsePkt->lenH << 8) | responsePkt->lenL) != cmdStu.responseLen)){
    CMD_DBG("response recv packet failrd.");
    free(responsePkt);
    return false;
  }
  free(responsePkt);
  return true;
}

String DFRobot_DFR0870_Protocol::getAbsolutePath(int8_t id, uint8_t type){
  String str = "";
  sCmdStruct_t cmdStu = getCmdStructConfig(CMD_ABSPATH);
  pSendCmdPkt_t sendPkt = (pSendCmdPkt_t)packedCmdPacket(cmdStu.cmd, 0);
  
  if(sendPkt == NULL){
    CMD_DBG("FlashInfo: packed malloc failed.");
    return str;
  }
  sendPkt->buf[0] = (uint8_t)id;
  sendPkt->buf[1] = type;
  if(writeCmdPacket(sendPkt, (SEND_PKT_PRE_FIX_LEN + ((sendPkt->lenH << 8) | sendPkt->lenL))) == false){
    CMD_DBG("send packet fail.");
    free(sendPkt);
    return str;
  }
  free(sendPkt);

  pResponseCmdPkt_t responsePkt = (pResponseCmdPkt_t)recvCmdResponsePkt(CMD_ABSPATH);
  if(responsePkt == NULL){
    CMD_DBG("response packet fail.");
    return str;
  }
  if((responsePkt->state != STATUS_SUCCESS) || (responsePkt->cmd != CMD_ABSPATH) || (((responsePkt->lenH << 8) | responsePkt->lenL) == 0)){
    CMD_DBG("response recv packet failrd.");
    free(responsePkt);
    return str;
  }
  uint16_t length = (responsePkt->lenH << 8) | responsePkt->lenL;

  char pname[length + 1];
  memcpy(pname, responsePkt->buf, length);
  pname[length] = '\0';
  free(responsePkt);
  return String(pname);
}
String DFRobot_DFR0870_Protocol::getParentDirectory(int8_t id, uint8_t type){
  String str = "";
  sCmdStruct_t cmdStu = getCmdStructConfig(CMD_PARENTDIR);
  pSendCmdPkt_t sendPkt = (pSendCmdPkt_t)packedCmdPacket(cmdStu.cmd, 0);

  if(sendPkt == NULL){
    CMD_DBG("FlashInfo: packed malloc failed.");
    return str;
  }
  sendPkt->buf[0] = (uint8_t)id;
  sendPkt->buf[1] = type;
  if(writeCmdPacket(sendPkt, (SEND_PKT_PRE_FIX_LEN + ((sendPkt->lenH << 8) | sendPkt->lenL))) == false){
    CMD_DBG("send packet fail.");
    free(sendPkt);
    return str;
  }
  free(sendPkt);

  pResponseCmdPkt_t responsePkt = (pResponseCmdPkt_t)recvCmdResponsePkt(CMD_PARENTDIR);
  if(responsePkt == NULL){
    CMD_DBG("response packet fail.");
    return str;
  }
  if((responsePkt->state != STATUS_SUCCESS) || (responsePkt->cmd != CMD_PARENTDIR) || (((responsePkt->lenH << 8) | responsePkt->lenL) == 0)){
    CMD_DBG("response recv packet failrd.");
    free(responsePkt);
    return str;
  }
  uint16_t length = (responsePkt->lenH << 8) | responsePkt->lenL;

  char pname[length + 1];
  memcpy(pname, responsePkt->buf, length);
  pname[length] = '\0';
  free(responsePkt);
  return String(pname);
}

