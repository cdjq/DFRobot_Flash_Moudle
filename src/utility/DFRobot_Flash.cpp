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
#include "DFRobot_Flash.h"

#define STATE_FAILED    0x63
#define STATE_SUCESS    0x53

#define INVAILD_ID      0xFF

#define TYPE_FAT_FILE_CLOSED  0 //< This DFRobot_FlashFile has not been opened.
#define TYPE_FAT_FILE_NORMAL  1 //< DFRobot_FlashFile for a file
#define TYPE_FAT_FILE_ROOT12  2 //< DFRobot_FlashFile for a FAT16 root directory
#define TYPE_FAT_FILE_ROOT16  3 //< DFRobot_FlashFile for a FAT16 root directory
#define TYPE_FAT_FILE_ROOT32  4 //< DFRobot_FlashFile for a FAT32 root directory
#define TYPE_FAT_FILE_SUBDIR  5 //< DFRobot_FlashFile for a subdirectory
#define TYPE_FAT_FILE_MIN_DIR  TYPE_FAT_FILE_ROOT12 //< Test value for directory type

#define TYPE_FAT12      12
#define TYPE_FAT16      16
#define TYPE_FAT32      32

#define AUTH_O_READ     0x01
#define AUTH_O_WRITE    0x02
#define AUTH_O_RDWR     (AUTH_O_READ | AUTH_O_WRITE)

#define ONECE_TRANSFER_MAX_BYTES   256


/////////////////////////////////////////////////////driver start/////////////////////////////////////////////////////
static class DFRobot_Driver *_drvIf = NULL; ///< 指向DFRobot_Driver抽象类的指针, 存放子类的接口实现
#define CHECK_DRV_IF(drv)  (_drvIf != NULL)
/////////////////////////////////////////////////////driver end//////////////////////////////////////////////////////


/////////////////////////////////////////////////////cmd funtion start/////////////////////////////////////////////////////
#define CONFIG_CMD_REG      0x00  ///<配置命令寄存器
#define FLAG_WRITE          0x00  ///<写标志
#define FLAG_READ           0x80  ///<读标志
#define STATUS_REG_FIX      0x02  ///<状态寄存器, 每条命令都有一个状态寄存器，它的实际位置 = STATUS_REG_FIX + 命令数据的长度
#define CMD_LEN_MAX         254

#define CMD_FLASH_INFO      0x01  ///< 读取Flash信息命令
#define CMD_READ_ADDR       0x02  ///< 读取IIC地址命令
#define CMD_MODIFY_ADDR     0x03  ///< 修改IIC地址命令
#define CMD_OPEN_FILE       0x05  ///< 打开文件命令
#define CMD_RESET           0x06  ///< 复位I2C从机命令, 只有应答命令无响应包，成功返回0x53，失败返回0x63
#define CMD_CLOSE_FILE      0x07  ///< 关闭文件命令
#define CMD_WRITE_FILE      0x08  ///< 写文件命令
#define CMD_SYNC_FILE       0x09  ///< 将缓存数据写入实际文件中的命令
#define CMD_SEEK_FILE       0x0A  ///< 设置文件读写指针位置命令
#define CMD_READ_FILE       0x0B  ///< 读文件命令
#define CMD_MKDIR           0x0C  ///< 创建文件夹命令
#define CMD_OPEN_DIR        0x0D  ///< 打开目录命令
#define CMD_REWIND          0x0E  ///< 返回最上级的目录
#define CMD_CLOSE_DIR       0x0F  ///< 关闭目录命令
#define CMD_FILE_INFO       0x10  ///< 获取文件是目录，还是文件，如果是文件，返回TYPE_FAT_FILE_NORMAL，TYPE_FAT_FILE_SUBDIR
#define CMD_RE_ROOT_DIR     0x11  ///< 返回根目录命令
#define CMD_READ_DIR        0x12  ///< 读取目录
#define CMD_REMOVE          0x13  ///< 移除文件或目录
#define CMD_GET_PWD         0x14  ///< 获取当前工作目录命令
#define CMD_CD_PWD          0x15  ///< 切换工作目录命令
#define CMD_DEL             0x16  ///< 删除文件内容命令
#define CMD_INSERT_ONE      0x17  ///< 插入同一个字符
#define CMD_INSERT_MUTI     0x18  ///< 插入同一个字符


#define CMD_LS              0x05  ///< 列出某目录下所有文件或文件夹命令



#define CMD_INVAILD         0x0E  ///< 无效命令

#define LEN_FIX_CMD         0x02  ///< 固定长度
#define LEN_FLASH_INFO      0x02  ///< 读取Flash信息命令
#define LEN_READ_ADDR       0x00  ///< 读取IIC地址命令的固定长度
#define LEN_MODIFY_ADDR     0x01  ///< 修改IIC地址命令的固定长度
#define LEN_OPEN_FILE       0x04  ///< 打开文件命令的固定长度
#define LEN_RESET           0x02  ///< 复位I2C从机命令的固定长度
#define LEN_CLOSE_FILE      0x03  ///< 关闭文件命令固定长度
#define LEN_WRITE_FILE      0x04  ///< 写文件命令的固定长度
#define LEN_SYNC_FILE       0x03  ///< 将缓存数据写入实际文件中命令的固定长度
#define LEN_SEEK_FILE       0x07  ///< 设置文件读写指针位置命令的固定长度 cmd,len,id,pos(4bytes)
#define LEN_READ_FILE       0x04  ///< 读文件命令的固定长度
#define LEN_MKDIR           0x03  ///< 创建文件夹命令的固定长度
#define LEN_OPEN_DIR        0x03  ///< 打开目录命令
#define LEN_REWIND          0x03  ///< 返回最上级的目录
#define LEN_CLOSE_DIR       0x03  ///< 关闭目录命令
#define LEN_FILE_INFO       0x03  ///< 获取文件是目录，还是文件，如果是文件，返回TYPE_FAT_FILE_NORMAL，TYPE_FAT_FILE_SUBDIR
#define LEN_RE_ROOT_DIR     0x02  ///< 返回根目录命令固定长度
#define LEN_READ_DIR        0x03  ///< 读取目录
#define LEN_REMOVE          0x03  ///< 移除文件或目录
#define LEN_GET_PWD         0x02  ///< 获取当前工作目录命令的固定长度
#define LEN_CD_PWD          0x03  ///< 切换工作目录命令固定长度
#define LEN_DEL             0x0C  ///< 删除文件内容命令
#define LEN_INSERT_ONE      0x0C  ///< 插入同一个字符
#define LEN_INSERT_MUTI     0x05  ///< 插入同一个字符

#define LEN_CREATE          0x00  ///< 创建文件命令固定长度
#define LEN_RM_FILE         0x00  ///< 移除文件命令固定长度

#define LEN_RESP_RESET      (1+1) ///<状态(0x53或0x63)，后面包的长度

#define LEN_RESP_FLASH_INFO  (2 + 1 + 4 + 4 + 2 )     ///< 读取Flash信息命令的响应包 分别为FAT类型（1字节）， flash容量（4字节）， 空闲字节数(4字节) 能存储的最大文件数（2字节）crc(1字节)
#define LEN_RESP_OPEN_FILE   (2 + 1 + 4 + 4)          ///< 打开命令的响应包 分别为ID（1字节）， 当前位置（4字节）， 文件总大小（4字节） crc(1字节)
#define LEN_RESP_CLOSE       (2 + 0)
#define LEN_RESP_WRITE_FILE  (2 + 0)
#define LEN_RESP_READ_FILE   (2 + 0)
#define LEN_RESP_SYNC_FILE   (2 + 0)
#define LEN_RESP_SEEK_FILE   (2 + 0)
#define LEN_RESP_MKDIR       (2 + 0)
#define LEN_RESP_OPEN_DIR    (2 + 1)
#define LEN_RESP_REWIND      (2 + 0)  ///< 返回最上级的目录
#define LEN_RESP_CLOSE_DIR   (2 + 0)  ///< 关闭目录命令
#define LEN_RESP_FILE_INFO   (2 + 1)  ///<
#define LEN_RESP_RE_ROOT_DIR (2 + 0)  ///< 返回根目录命令固定长度
#define LEN_RESP_READ_DIR    (2 + 12)  ///< 读取目录
#define LEN_RESP_REMOVE      (2 + 0)  ///< 移除文件或目录
#define LEN_RESP_GET_PWD     (2)  ///< 获取当前工作目录命令的固定长度
#define LEN_RESP_CD_PWD      (2)   ///< 切换工作目录命令固定长度
#define LEN_RESP_DEL         (2)  ///< 删除文件内容命令
#define LEN_RESP_INSERT_ONE  (2)  ///< 插入同一个字符
#define LEN_RESP_INSERT_MUTI (2)  ///< 插入同一个字符

//#define LEN_RESP_OPEN_FILE   (0)         ///< 复位命令的响应包 分别为ID（1字节）， 当前位置（4字节）， 文件总大小（4字节） crc(1字节)

typedef struct{
  uint8_t cmd;    /**< 命令，范围0x00~0x0E,0x0F及之后为无效命令 */
  uint8_t len;    /**< 除去具体命令后的数据长度，这个一般用来计算buf数组的长度 */
  uint8_t buf[0]; /**< 0长度数组，它的大小取决于上一个变量len的值 */
}__attribute__ ((packed)) sCmdPkt_t, *pCmdPkt_t;

typedef struct{
  union{
    struct{
      uint8_t state;    /**< 命令，范围0x00~0x0E,0x0F及之后为无效命令 */
      uint8_t len;    /**< 除去具体命令后的数据长度，这个一般用来计算buf数组的长度 */
    };
    struct{
      uint8_t exeReg;    /**< 命令，范围0x00~0x0E,0x0F及之后为无效命令 */
      uint8_t rwFlag;    /**< 除去具体命令后的数据长度，这个一般用来计算buf数组的长度 */
    };
    
  };
  uint8_t buf[0]; /**< 0长度数组，它的大小取决于上一个变量len的值 */
}__attribute__ ((packed)) sResponsePkt_t, *pResponsePkt_t;


static bool waitForTimeout(uint8_t reg, uint8_t *len, uint32_t timems){
    uint8_t data[2] = {reg, FLAG_READ};
    uint32_t t = 0;
    _drvIf->sendData(data, 2);
    t = millis();
    while(millis() - t < timems){
      _drvIf->recvData(data, 1);
      FLASH_DBG(data[0],HEX);
      if(data[0] == 0x53){
        _drvIf->recvData(len, 1);
        FLASH_DBG(*len,HEX);
        return true;
      }
      if(data[0] == 0x63){
        _drvIf->recvData(len, 1);
         FLASH_DBG(*len,HEX);
        return false;
      }
      delay(50);
    }
    return true;
}

static bool changeDirCommand(bool absolute, const char* fileName){
  if(_drvIf == NULL){
    FLASH_DBG("_drvIf is null.");
    return 0;
  }
  uint8_t *buf = (uint8_t *)malloc((LEN_CD_PWD + strlen(fileName) + 1) > LEN_RESP_CD_PWD ? (LEN_CD_PWD + strlen(fileName) + 1) : LEN_RESP_CD_PWD);//定义了一个buf
  if(buf == NULL){
    FLASH_DBG("pCmd malloc failed. ");
    return false;
  }
  pCmdPkt_t pCmd = (pCmdPkt_t)buf;
  pResponsePkt_t rpkt = (pResponsePkt_t)buf;
  pCmd->cmd = CMD_CD_PWD;
  pCmd->len = (LEN_CD_PWD + strlen(fileName) + 1) - LEN_FIX_CMD;
  pCmd->buf[0] = (uint8_t) absolute;
  memcpy(pCmd->buf+1, fileName, strlen(fileName));
  pCmd->buf[strlen(fileName) + 1] = 0;
  _drvIf->writeReg(CONFIG_CMD_REG, pCmd, (LEN_CD_PWD + strlen(fileName) + 1));//发送命令后，读取命令
  rpkt->exeReg = (LEN_CD_PWD + strlen(fileName) + 1);
  if(waitForTimeout(rpkt->exeReg, &rpkt->len, 1000)){
     FLASH_DBG("sucess.");
     free(buf);
     return true;
  }
  free(buf);
  return false;
}

static String getPWDCommand(){
  if(_drvIf == NULL){
    FLASH_DBG("_drvIf is null.");
    return "";
  }
  uint8_t *buf = (uint8_t *)malloc((LEN_GET_PWD) > LEN_RESP_GET_PWD ? LEN_GET_PWD : LEN_RESP_GET_PWD);//定义了一个buf
  if(buf == NULL){
    FLASH_DBG("pCmd malloc failed. ");
    return "";
  }
  pCmdPkt_t pCmd = (pCmdPkt_t)buf;
  pResponsePkt_t rpkt = (pResponsePkt_t)buf;
  pCmd->cmd = CMD_GET_PWD;
  pCmd->len = LEN_GET_PWD  - LEN_FIX_CMD;
  _drvIf->writeReg(CONFIG_CMD_REG, pCmd, LEN_GET_PWD);//发送命令后，读取命令
  rpkt->exeReg = LEN_GET_PWD;
  if(waitForTimeout(rpkt->exeReg, &rpkt->len, 1000)){
     FLASH_DBG("sucess.");
     char name[rpkt->len];
     _drvIf->recvData(name, rpkt->len);
     free(buf);
     return String(name);
  }
  free(buf);
  return "";

}


static bool rewindRootDir(){
  if(_drvIf == NULL){
    FLASH_DBG("_drvIf is null.");
    return 0;
  }
  uint8_t *buf = (uint8_t *)malloc(LEN_RE_ROOT_DIR > LEN_RESP_RE_ROOT_DIR ? LEN_RE_ROOT_DIR : LEN_RESP_RE_ROOT_DIR);//定义了一个buf
  if(buf == NULL){
    FLASH_DBG("pCmd malloc failed. ");
    return false;
  }
  pCmdPkt_t pCmd = (pCmdPkt_t)buf;
  pResponsePkt_t rpkt = (pResponsePkt_t)buf;
  
  pCmd->cmd = CMD_RE_ROOT_DIR;
  pCmd->len = LEN_RE_ROOT_DIR - LEN_FIX_CMD;
  _drvIf->writeReg(CONFIG_CMD_REG, pCmd, LEN_RE_ROOT_DIR);//发送命令后，读取命令
  rpkt->exeReg = LEN_RE_ROOT_DIR;
  if(waitForTimeout(rpkt->exeReg, &rpkt->len, 1000)){
     FLASH_DBG("sucess.");
     free(buf);
     return true;
  }
  free(buf);
  return false;
}



static bool removeCommand(int8_t pid, char *name){
  if(_drvIf == NULL){
      FLASH_DBG("_drvIf is null.");
      return 0;
    }
    uint8_t *buf = (uint8_t *)malloc((LEN_REMOVE + strlen(name) + 1) > LEN_RESP_REMOVE ? (LEN_REMOVE + strlen(name) + 1)  : LEN_RESP_REMOVE);//定义了一个buf
    if(buf == NULL){
      FLASH_DBG("pCmd malloc failed. ");
      return false;
    }
    pCmdPkt_t pCmd = (pCmdPkt_t)buf;
    pResponsePkt_t rpkt = (pResponsePkt_t)buf;
    
    pCmd->cmd = CMD_REMOVE;
    pCmd->len =(LEN_REMOVE + strlen(name) + 1)  - LEN_FIX_CMD;
    memcpy(pCmd->buf, &pid, 1);
    memcpy(pCmd->buf+1, name, strlen(name)+1);

    _drvIf->writeReg(CONFIG_CMD_REG, pCmd, (LEN_REMOVE + strlen(name) + 1) );//发送命令后，读取命令

    rpkt->exeReg = (LEN_REMOVE + strlen(name) + 1) ;
    rpkt->rwFlag = FLAG_READ;

    if(waitForTimeout(rpkt->exeReg, &rpkt->len, 1000)){
       free(buf);
       return true;
    }

    free(buf);
    return false;
}
static bool syncFileCommand(int8_t id){
  if(_drvIf == NULL){
      FLASH_DBG("_drvIf is null.");
      return 0;
    }
    uint8_t *buf = (uint8_t *)malloc(LEN_SYNC_FILE > LEN_RESP_SYNC_FILE ? LEN_SYNC_FILE : LEN_RESP_SYNC_FILE);//定义了一个buf
    if(buf == NULL){
      FLASH_DBG("pCmd malloc failed. ");
      return false;
    }
    pCmdPkt_t pCmd = (pCmdPkt_t)buf;
    pResponsePkt_t rpkt = (pResponsePkt_t)buf;
    
    pCmd->cmd = CMD_SYNC_FILE;
    pCmd->len =LEN_SYNC_FILE - LEN_FIX_CMD;
    memcpy(pCmd->buf, &id, 1);

    _drvIf->writeReg(CONFIG_CMD_REG, pCmd, LEN_SYNC_FILE);//发送命令后，读取命令

    rpkt->exeReg = LEN_SYNC_FILE;
    rpkt->rwFlag = FLAG_READ;

    if(waitForTimeout(rpkt->exeReg, &rpkt->len, 1000)){
       FLASH_DBG("sucess.");
       free(buf);
       return true;
    }

    free(buf);
    return false;
}



static bool readDirCommand(int8_t id, char *name){
  if(_drvIf == NULL){
      FLASH_DBG("_drvIf is null.");
      return 0;
    }
    uint8_t *buf = (uint8_t *)malloc(LEN_READ_DIR > LEN_RESP_READ_DIR ? LEN_READ_DIR : LEN_RESP_READ_DIR);//定义了一个buf
    if(buf == NULL){
      FLASH_DBG("pCmd malloc failed. ");
      return false;
    }
    pCmdPkt_t pCmd = (pCmdPkt_t)buf;
    pResponsePkt_t rpkt = (pResponsePkt_t)buf;
    
    pCmd->cmd = CMD_READ_DIR;
    pCmd->len =LEN_READ_DIR - LEN_FIX_CMD;
    memcpy(pCmd->buf, &id, 1);

    _drvIf->writeReg(CONFIG_CMD_REG, pCmd, LEN_READ_DIR);//发送命令后，读取命令

    rpkt->exeReg = LEN_READ_DIR;
    rpkt->rwFlag = FLAG_READ;

    if(waitForTimeout(rpkt->exeReg, &rpkt->len, 1000)){
       FLASH_DBG("sucess.");
       FLASH_DBG(rpkt->len);
       
       if(rpkt->len == 0){
         FLASH_DBG("no file or dir.");
         return false;
       }
       
       _drvIf->recvData(name, (rpkt->len > 12 ? 12 : rpkt->len));
       FLASH_DBG("name=");FLASH_DBG(name);
       free(buf);
       return true;
    }

    free(buf);
    return false;
}
static bool resetCommand(){ //发送命令
    //判断_drv是不是null
    if(_drvIf == NULL){
      FLASH_DBG("_drv is null.");
      return 0;
    }
    uint8_t *buf = (uint8_t *)malloc((LEN_RESET > LEN_RESP_RESET ? LEN_RESET : LEN_RESP_RESET));//定义了一个buf
    if(buf == NULL){
      FLASH_DBG("pCmd malloc failed. ");
      return false;
    }
    pCmdPkt_t pCmd = (pCmdPkt_t)buf;
    pResponsePkt_t rpkt = (pResponsePkt_t)buf;
    
    pCmd->cmd = CMD_RESET;
    pCmd->len = LEN_RESET - LEN_FIX_CMD;
    _drvIf->writeReg(CONFIG_CMD_REG, pCmd, LEN_RESET);//发送命令后，读取命令

    rpkt->exeReg = LEN_RESET;
    if(waitForTimeout(rpkt->exeReg, &rpkt->len, 1000)){
       FLASH_DBG("sucess.");
       free(buf);
       return true;
    }

    free(buf);
    return false;
}

static bool flashInfoCommand(uint8_t *fatType, uint32_t *cap, uint32_t *freeS, uint16_t *fns){
  //判断_drv是不是null
    if(_drvIf == NULL){
      FLASH_DBG("_drv is null.");
      return 0;
    }
    uint8_t *buf = (uint8_t *)malloc((LEN_FLASH_INFO > LEN_RESP_FLASH_INFO ? LEN_FLASH_INFO : LEN_RESP_FLASH_INFO));//定义了一个buf
    if(buf == NULL){
      FLASH_DBG("pCmd malloc failed. ");
      return false;
    }
    pCmdPkt_t pCmd = (pCmdPkt_t)buf;
    pResponsePkt_t rpkt = (pResponsePkt_t)buf;
    
    pCmd->cmd = CMD_FLASH_INFO;
    pCmd->len = LEN_FLASH_INFO - LEN_FIX_CMD;
    _drvIf->writeReg(CONFIG_CMD_REG, pCmd, LEN_FLASH_INFO);//发送命令后，读取命令

    rpkt->exeReg = LEN_FLASH_INFO;
    rpkt->rwFlag = FLAG_READ;

    if(waitForTimeout(rpkt->exeReg, &rpkt->len, 1000)){
       FLASH_DBG("sucess.");
       _drvIf->recvData(rpkt->buf, rpkt->len);
       
       *fatType = buf[2]; 
       *cap = ((uint32_t)buf[3] << 24) + ((uint32_t)buf[4] << 16) + ((uint32_t)buf[5] << 8) + (uint32_t)buf[6];
       *freeS = ((uint32_t)buf[7] << 24) | ((uint32_t)buf[8] << 16) | ((uint32_t)buf[9] << 8) | (uint32_t)buf[10];
       *fns = ((uint16_t)buf[11] << 8) | (uint16_t)buf[12];
       FLASH_DBG(*fatType);
       FLASH_DBG(*cap);
       FLASH_DBG(*freeS);
       FLASH_DBG(*fns);
       free(buf);
       return true;
    }

    free(buf);
    return false;
}

static bool openFileCommand(char *name, int8_t pId, uint8_t oflag, int8_t *id, uint32_t *curPos, uint32_t *size){
  if(_drvIf == NULL){
      FLASH_DBG("_drv is null.");
      return 0;
    }
  String str = String(name);
  str.toUpperCase();
  name = (char *)str.c_str();
    uint8_t *buf = (uint8_t *)malloc(((LEN_OPEN_FILE + strlen(name) + 1) > LEN_RESP_OPEN_FILE ? (LEN_OPEN_FILE + strlen(name) + 1) : LEN_RESP_OPEN_FILE));//定义了一个buf
    if(buf == NULL){
      FLASH_DBG("pCmd malloc failed. ");
      return false;
    }
    pCmdPkt_t pCmd = (pCmdPkt_t)buf;
    pResponsePkt_t rpkt = (pResponsePkt_t)buf;
    
    pCmd->cmd = CMD_OPEN_FILE;
    pCmd->len = LEN_OPEN_FILE + strlen(name) + 1 - LEN_FIX_CMD;
    pCmd->buf[0] = (uint8_t)pId;
    pCmd->buf[1] = oflag;
    memcpy(pCmd->buf + 2, name, strlen(name) + 1);
    


    _drvIf->writeReg(CONFIG_CMD_REG, pCmd, LEN_OPEN_FILE + strlen(name) + 1);//发送命令后，读取命令

    rpkt->exeReg = LEN_OPEN_FILE + strlen(name) + 1;
    
    if(waitForTimeout(rpkt->exeReg, &rpkt->len, 1000)){
       FLASH_DBG("sucess.");
       _drvIf->recvData(rpkt->buf, rpkt->len);
       *id = buf[2]; 
       *curPos = ((uint32_t)buf[3] << 24) | ((uint32_t)buf[4] << 16) | ((uint32_t)buf[5] << 8) | (uint32_t)buf[6];
       *size = ((uint32_t)buf[7] << 24) | ((uint32_t)buf[8] << 16) | ((uint32_t)buf[9] << 8) | (uint32_t)buf[10];
       FLASH_DBG(*id);
       FLASH_DBG(*curPos);
       FLASH_DBG(*size);
       free(buf);
       return true;
    }

    free(buf);
    return false;
}

static uint8_t fileInfoCommand(char *name , int8_t pId){//如果父级目录的id为-1，则代表为根目录
  if(_drvIf == NULL){
      FLASH_DBG("_drv is null.");
      return 0;
  }
  String str = String(name);
  str.toUpperCase();
  name = (char *)str.c_str();
  uint8_t *buf = (uint8_t *)malloc(((LEN_FILE_INFO + strlen(name) + 1) > LEN_RESP_FILE_INFO ? (LEN_FILE_INFO + strlen(name) + 1) : LEN_RESP_FILE_INFO));//定义了一个buf
  if(buf == NULL){
    FLASH_DBG("pCmd malloc failed. ");
    return false;
  }
  pCmdPkt_t pCmd = (pCmdPkt_t)buf;
  pResponsePkt_t rpkt = (pResponsePkt_t)buf;
  
  pCmd->cmd = CMD_FILE_INFO;
  pCmd->len = LEN_FILE_INFO + strlen(name) + 1 - LEN_FIX_CMD;
  pCmd->buf[0] = pId;
  memcpy(pCmd->buf+1, name, strlen(name) + 1);
  
  _drvIf->writeReg(CONFIG_CMD_REG, pCmd, LEN_FILE_INFO + strlen(name) + 1);//发送命令后，读取命令

  rpkt->exeReg = LEN_RESP_FILE_INFO + strlen(name) + 1;
  rpkt->rwFlag = FLAG_READ;

  if(waitForTimeout(rpkt->exeReg, &rpkt->len, 1000)){
       uint8_t type = 0;
       free(buf);
       _drvIf->recvData(&type, 1);
        FLASH_DBG("sucess.");
        FLASH_DBG(type);
       
       return type;
  }
  return 0;
}


static bool openDirCommand(char *name, int8_t pId, int8_t *id){
  if(_drvIf == NULL){
      FLASH_DBG("_drv is null.");
      return false;
  }
  String str = String(name);
  str.toUpperCase();
  name = (char *)str.c_str();
  uint8_t *buf = (uint8_t *)malloc(((LEN_OPEN_DIR + strlen(name) + 1) > LEN_RESP_OPEN_DIR ? (LEN_OPEN_FILE + strlen(name) + 1) : LEN_RESP_OPEN_DIR));//定义了一个buf
    if(buf == NULL){
      FLASH_DBG("pCmd malloc failed. ");
      return false;
    }

    pCmdPkt_t pCmd = (pCmdPkt_t)buf;
    pResponsePkt_t rpkt = (pResponsePkt_t)buf;
    
    pCmd->cmd = CMD_OPEN_DIR;
    pCmd->len = LEN_OPEN_DIR + strlen(name) + 1 - LEN_FIX_CMD;
    pCmd->buf[0] = (uint8_t) pId;
    memcpy(pCmd->buf+1, name, strlen(name)+1);


    _drvIf->writeReg(CONFIG_CMD_REG, pCmd, LEN_OPEN_DIR + strlen(name) + 1);//发送命令后，读取命令

    rpkt->exeReg = LEN_OPEN_DIR + strlen(name) + 1;
    rpkt->rwFlag = FLAG_READ;

    if(waitForTimeout(rpkt->exeReg, &rpkt->len, 1000)){
       FLASH_DBG("sucess.");
       free(buf);
       _drvIf->recvData(id, 1);
       FLASH_DBG("id=");FLASH_DBG(*id);
       return true;
    }

    free(buf);
    return false;
}

static bool rewindCommand(int8_t id){
  if(_drvIf == NULL){
      FLASH_DBG("_drvIf is null.");
      return 0;
    }
    uint8_t *buf = (uint8_t *)malloc(LEN_REWIND > LEN_RESP_REWIND ? LEN_REWIND : LEN_RESP_REWIND);//定义了一个buf
    if(buf == NULL){
      FLASH_DBG("pCmd malloc failed. ");
      return false;
    }
    pCmdPkt_t pCmd = (pCmdPkt_t)buf;
    pResponsePkt_t rpkt = (pResponsePkt_t)buf;
    
    pCmd->cmd = CMD_REWIND;
    pCmd->len =LEN_REWIND - LEN_FIX_CMD;
    memcpy(pCmd->buf, &id, 1);

    _drvIf->writeReg(CONFIG_CMD_REG, pCmd, LEN_REWIND);//发送命令后，读取命令

    rpkt->exeReg = LEN_REWIND;
    rpkt->rwFlag = FLAG_READ;

    if(waitForTimeout(rpkt->exeReg, &rpkt->len, 1000)){
       FLASH_DBG("sucess.");
       free(buf);
       return true;
    }

    free(buf);
    return false;

}

static bool closeDirCommand(int8_t id){
  if(_drvIf == NULL){
      FLASH_DBG("_drvIf is null.");
      return 0;
    }
    uint8_t *buf = (uint8_t *)malloc(LEN_CLOSE_DIR > LEN_RESP_CLOSE_DIR ? LEN_CLOSE_DIR : LEN_RESP_CLOSE_DIR);//定义了一个buf
    if(buf == NULL){
      FLASH_DBG("pCmd malloc failed. ");
      return false;
    }
    pCmdPkt_t pCmd = (pCmdPkt_t)buf;
    pResponsePkt_t rpkt = (pResponsePkt_t)buf;
    
    pCmd->cmd = CMD_CLOSE_DIR;
    pCmd->len =LEN_CLOSE_DIR - LEN_FIX_CMD;
    memcpy(pCmd->buf, &id, 1);

    _drvIf->writeReg(CONFIG_CMD_REG, pCmd, LEN_CLOSE_DIR);//发送命令后，读取命令

    rpkt->exeReg = LEN_CLOSE_DIR;
    rpkt->rwFlag = FLAG_READ;

    if(waitForTimeout(rpkt->exeReg, &rpkt->len, 1000)){
       FLASH_DBG("sucess.");
       free(buf);
       return true;
    }

    free(buf);
    return false;

}

static bool closeCommand(int8_t id){
  if(_drvIf == NULL){
      FLASH_DBG("_drvIf is null.");
      return 0;
    }
    uint8_t *buf = (uint8_t *)malloc(LEN_CLOSE_FILE > LEN_RESP_CLOSE ? LEN_CLOSE_FILE : LEN_RESP_CLOSE);//定义了一个buf
    if(buf == NULL){
      FLASH_DBG("pCmd malloc failed. ");
      return false;
    }
    pCmdPkt_t pCmd = (pCmdPkt_t)buf;
    pResponsePkt_t rpkt = (pResponsePkt_t)buf;
    
    pCmd->cmd = CMD_CLOSE_FILE;
    pCmd->len =LEN_CLOSE_FILE - LEN_FIX_CMD;
    memcpy(pCmd->buf, &id, 1);
    FLASH_DBG("close: id=");FLASH_DBG(id);

    _drvIf->writeReg(CONFIG_CMD_REG, pCmd, LEN_CLOSE_FILE);//发送命令后，读取命令

    rpkt->exeReg = LEN_CLOSE_FILE;
    rpkt->rwFlag = FLAG_READ;

    if(waitForTimeout(rpkt->exeReg, &rpkt->len, 1000)){
       FLASH_DBG("sucess.");
       free(buf);
       return true;
    }

    free(buf);
    return false;
}


static uint16_t insertCommand(int8_t id, uint32_t pos, void *data, uint32_t len){
  if(_drvIf == NULL){
      FLASH_DBG("_drvIf is null.");
      return 0;
    }
    uint8_t *buf = (uint8_t *)malloc(LEN_INSERT_MUTI > LEN_RESP_INSERT_MUTI ? LEN_INSERT_MUTI : LEN_RESP_INSERT_MUTI);//定义了一个buf
    if(buf == NULL){
      FLASH_DBG("pCmd malloc failed. ");
      return false;
    }
    pCmdPkt_t pCmd = (pCmdPkt_t)buf;
    pResponsePkt_t rpkt = (pResponsePkt_t)buf;
    uint8_t *pBuf = (uint8_t *)data;
    
    uint16_t remain = len;
    uint16_t t = 0;
    uint8_t flag = 1;
    uint16_t count = 0;
    uint32_t num = 0;
    while(remain){
      len = (remain > (256 - 2 - LEN_INSERT_MUTI)) ? (256 - 2 - LEN_INSERT_MUTI) : remain;
      
      pCmd->cmd = CMD_INSERT_MUTI;
      pCmd->len = LEN_INSERT_MUTI - LEN_FIX_CMD + len;
      
      memcpy(pCmd->buf, &id, 1);
      pCmd->buf[1] = (pos >> 24) & 0xFF;
      pCmd->buf[2] = (pos >> 16) & 0xFF;
      pCmd->buf[3] = (pos >> 8) & 0xFF;
      pCmd->buf[4] = (pos >> 0) & 0xFF;
      
      _drvIf->writeReg(CONFIG_CMD_REG, pCmd, LEN_INSERT_MUTI);//发送命令后，读取命令

      _drvIf->writeReg(LEN_INSERT_MUTI, pBuf, len);
      //while(1) yield();
      //delay(1000);
      rpkt->exeReg = LEN_INSERT_MUTI + len;
      rpkt->rwFlag = FLAG_READ;


      if(!waitForTimeout(rpkt->exeReg, &rpkt->len, 2000)){
        FLASH_DBG(rpkt->len)
        free(buf);
        return t;
      }
      t += len;
      remain -= len;
      pBuf += len;
      count += len;
      pos += len;
      
    }
    free(buf);
    return t;
}

static uint16_t writeFileCommand(int8_t id, void *data, uint16_t len){
  if(_drvIf == NULL){
      FLASH_DBG("_drvIf is null.");
      return 0;
    }
    uint8_t *buf = (uint8_t *)malloc(LEN_WRITE_FILE > LEN_RESP_WRITE_FILE ? LEN_CLOSE_FILE : LEN_RESP_WRITE_FILE);//定义了一个buf
    if(buf == NULL){
      FLASH_DBG("pCmd malloc failed. ");
      return false;
    }
    pCmdPkt_t pCmd = (pCmdPkt_t)buf;
    pResponsePkt_t rpkt = (pResponsePkt_t)buf;
    uint8_t *pBuf = (uint8_t *)data;
    
    uint16_t remain = len;
    uint16_t t = 0;
    uint8_t flag = 1;
    uint16_t count = 0;
    while(remain){
      len = (remain > 250) ? 250 : remain;
      if(count + len > 1024 ){
        count = 0;
        if(!syncFileCommand(id)){
          free(buf);
          return t;
        }

      }
      pCmd->cmd = CMD_WRITE_FILE;
      pCmd->len =LEN_WRITE_FILE - LEN_FIX_CMD + len;
      memcpy(pCmd->buf, &id, 1);
      pCmd->buf[1] = len;
      _drvIf->writeReg(CONFIG_CMD_REG, pCmd, LEN_WRITE_FILE);//发送命令后，读取命令

      _drvIf->writeReg(LEN_WRITE_FILE, pBuf, len);
      //while(1) yield();
      //delay(1000);
      rpkt->exeReg = LEN_WRITE_FILE + len;
      rpkt->rwFlag = FLAG_READ;


      if(!waitForTimeout(rpkt->exeReg, &rpkt->len, 2000)){
        FLASH_DBG(rpkt->len)
        free(buf);
        return t;
      }
      t += len;
      remain -= len;
      pBuf += len;
      count += len;
      
    }
    /*if(!syncFileCommand(id)){
      free(buf);
      return t - count;
    }*/
    free(buf);
    return t;
}

static uint16_t readFileCommand(int8_t id, void *data, uint16_t len){
  if(_drvIf == NULL){
    FLASH_DBG("_drvIf is null.");
    return 0;
  }
  uint8_t *buf = (uint8_t *)malloc(LEN_READ_FILE > LEN_RESP_READ_FILE ? LEN_CLOSE_FILE : LEN_RESP_READ_FILE);//定义了一个buf
  if(buf == NULL){
    FLASH_DBG("pCmd malloc failed. ");
    return 0;
  }
  pCmdPkt_t pCmd = (pCmdPkt_t)buf;
  pResponsePkt_t rpkt = (pResponsePkt_t)buf;
  uint8_t *pBuf = (uint8_t *)data;
  
  
  uint16_t remain = len;
  uint16_t t = 0;
  while(remain){
    len = (remain > 255) ? 255 : remain;
    pCmd->cmd = CMD_READ_FILE;
    pCmd->len = LEN_READ_FILE - LEN_FIX_CMD;
    memcpy(pCmd->buf, &id, 1);
    pCmd->buf[1] = len;
    _drvIf->writeReg(CONFIG_CMD_REG, pCmd, LEN_READ_FILE);//发送命令后，读取命令
  
    rpkt->exeReg = LEN_READ_FILE;

    if(!waitForTimeout(rpkt->exeReg, &rpkt->len, 1000)){
       FLASH_DBG("fail.");
       free(buf);
       return t;
    }
    _drvIf->recvData(pBuf, len);
    //for(int i =0; i < len; i++){
    //  FLASH_DBG(pBuf[i], HEX);
//
    //}
    
    pBuf += len;
    t += len;
    remain -= len;
  }
  free(buf);
  return t;
}


static bool insertOneCommand(int8_t id, uint32_t pos, uint8_t val, uint32_t num){
  if(_drvIf == NULL){
      FLASH_DBG("_drvIf is null.");
      return 0;
    }
    uint8_t *buf = (uint8_t *)malloc(LEN_INSERT_ONE > LEN_RESP_INSERT_ONE ? LEN_INSERT_ONE : LEN_RESP_INSERT_ONE);//定义了一个buf
    if(buf == NULL){
      FLASH_DBG("pCmd malloc failed. ");
      return false;
    }
    pCmdPkt_t pCmd = (pCmdPkt_t)buf;
    pResponsePkt_t rpkt = (pResponsePkt_t)buf;
    
    pCmd->cmd = CMD_INSERT_ONE;
    pCmd->len =LEN_INSERT_ONE - LEN_FIX_CMD;
    memcpy(pCmd->buf, &id, 1);
    pCmd->buf[1] = (pos >> 24) & 0xFF;
    pCmd->buf[2] = (pos >> 16) & 0xFF;
    pCmd->buf[3] = (pos >> 8) & 0xFF;
    pCmd->buf[4] = (pos >> 0) & 0xFF;
    pCmd->buf[5] = (num >> 24) & 0xFF;
    pCmd->buf[6] = (num >> 16) & 0xFF;
    pCmd->buf[7] = (num >> 8) & 0xFF;
    pCmd->buf[8] = (num >> 0) & 0xFF;
    pCmd->buf[9] = val;

    _drvIf->writeReg(CONFIG_CMD_REG, pCmd, LEN_INSERT_ONE);//发送命令后，读取命令

    rpkt->exeReg = LEN_INSERT_ONE;
    rpkt->rwFlag = FLAG_READ;

    if(waitForTimeout(rpkt->exeReg, &rpkt->len, 1000)){
       FLASH_DBG("sucess.");
       free(buf);
       return true;
    }

    free(buf);
    return false;
}
static bool delCommand(int8_t id, uint32_t pos, uint32_t num, bool flag){
  if(_drvIf == NULL){
      FLASH_DBG("_drvIf is null.");
      return 0;
    }
    uint8_t *buf = (uint8_t *)malloc(LEN_DEL > LEN_RESP_DEL ? LEN_DEL : LEN_RESP_DEL);//定义了一个buf
    if(buf == NULL){
      FLASH_DBG("pCmd malloc failed. ");
      return false;
    }
    pCmdPkt_t pCmd = (pCmdPkt_t)buf;
    pResponsePkt_t rpkt = (pResponsePkt_t)buf;
    
    pCmd->cmd = CMD_DEL;
    pCmd->len =LEN_DEL - LEN_FIX_CMD;
    memcpy(pCmd->buf, &id, 1);
    pCmd->buf[1] = (pos >> 24) & 0xFF;
    pCmd->buf[2] = (pos >> 16) & 0xFF;
    pCmd->buf[3] = (pos >> 8) & 0xFF;
    pCmd->buf[4] = (pos >> 0) & 0xFF;
    pCmd->buf[5] = (num >> 24) & 0xFF;
    pCmd->buf[6] = (num >> 16) & 0xFF;
    pCmd->buf[7] = (num >> 8) & 0xFF;
    pCmd->buf[8] = (num >> 0) & 0xFF;
    pCmd->buf[9] = (uint8_t)flag;

    _drvIf->writeReg(CONFIG_CMD_REG, pCmd, LEN_DEL);//发送命令后，读取命令

    rpkt->exeReg = LEN_DEL;
    rpkt->rwFlag = FLAG_READ;

    if(waitForTimeout(rpkt->exeReg, &rpkt->len, 1000)){
       FLASH_DBG("sucess.");
       free(buf);
       return true;
    }

    free(buf);
    return false;

}

static bool seekFileCommand(int8_t id, uint32_t pos){
  if(_drvIf == NULL){
      FLASH_DBG("_drvIf is null.");
      return 0;
    }
    uint8_t *buf = (uint8_t *)malloc(LEN_SEEK_FILE > LEN_RESP_SEEK_FILE ? LEN_SEEK_FILE : LEN_RESP_SEEK_FILE);//定义了一个buf
    if(buf == NULL){
      FLASH_DBG("pCmd malloc failed. ");
      return false;
    }
    pCmdPkt_t pCmd = (pCmdPkt_t)buf;
    pResponsePkt_t rpkt = (pResponsePkt_t)buf;
    
    pCmd->cmd = CMD_SEEK_FILE;
    pCmd->len =LEN_SEEK_FILE - LEN_FIX_CMD;
    memcpy(pCmd->buf, &id, 1);
    pCmd->buf[1] = (pos >> 24) & 0xFF;
    pCmd->buf[2] = (pos >> 16) & 0xFF;
    pCmd->buf[3] = (pos >> 8) & 0xFF;
    pCmd->buf[4] = (pos >> 0) & 0xFF;

    _drvIf->writeReg(CONFIG_CMD_REG, pCmd, LEN_SEEK_FILE);//发送命令后，读取命令

    rpkt->exeReg = LEN_SEEK_FILE;
    rpkt->rwFlag = FLAG_READ;

    if(waitForTimeout(rpkt->exeReg, &rpkt->len, 1000)){
       FLASH_DBG("sucess.");
       free(buf);
       return true;
    }

    free(buf);
    return false;
}


static bool mkdirCommand(int8_t pid, char *name){
  if(_drvIf == NULL){
      FLASH_DBG("_drv is null.");
      return 0;
  }
  String str = String(name);
  str.toUpperCase();
  name = (char *)str.c_str();
    uint8_t *buf = (uint8_t *)malloc(((LEN_MKDIR + strlen(name) + 1) > LEN_RESP_MKDIR ? (LEN_MKDIR + strlen(name) + 1) : LEN_RESP_MKDIR));//定义了一个buf
    if(buf == NULL){
      FLASH_DBG("pCmd malloc failed. ");
      return false;
    }
    pCmdPkt_t pCmd = (pCmdPkt_t)buf;
    pResponsePkt_t rpkt = (pResponsePkt_t)buf;
    
    pCmd->cmd = CMD_MKDIR;
    pCmd->len = LEN_MKDIR + strlen(name) + 1 - LEN_FIX_CMD;
    pCmd->buf[0] = (uint8_t) pid;
    memcpy(pCmd->buf + 1, name, strlen(name) + 1);

    _drvIf->writeReg(CONFIG_CMD_REG, pCmd, LEN_MKDIR + strlen(name) + 1);//发送命令后，读取命令

    rpkt->exeReg = LEN_MKDIR + strlen(name) + 1;
    rpkt->rwFlag = FLAG_READ;

    if(waitForTimeout(rpkt->exeReg, &rpkt->len, 1000)){
       FLASH_DBG("sucess.");
       free(buf);
       return true;
    }

    free(buf);
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////

DFRobot_Flash::DFRobot_Flash()
  : _capacity(0),_freeSpace(0),_fatType(0),_fileNums(0){}

DFRobot_Flash::~DFRobot_Flash(){
    _capacity = 0;
    _fatType = 0;
    _fileNums = 0;
}

uint8_t DFRobot_Flash::init(DFRobot_Driver *drv){
    if(!drv){
        FLASH_DBG("drv is NULL, Error: (1 << 4)");
        return (1 << 4);
    }
    _drvIf = drv;
    FLASH_DBG((uint32_t)((uint32_t *)_drvIf), HEX);
    if(!reset()){
        FLASH_DBG("RESET FAILED. Error: (2 << 4)");
        return (2 << 4);
    }
    delay(2000);//复位后1s内不再接收数据

    if(!flashInfoCommand(&_fatType, &_capacity, &_freeSpace, &_fileNums)){
      FLASH_DBG("get flash info cmd pakage failed! Error: (3 << 4)");
      return (3 << 4);
    }
    return 0;
}

bool DFRobot_Flash::reset(){
    return resetCommand();
}

uint32_t DFRobot_Flash::size(){
  return _capacity;
}

uint8_t DFRobot_Flash::fatType(){
  return _fatType;
}

DFRobot_FlashFile::DFRobot_FlashFile()
  :_flash(NULL), _id(INVAILD_ID), _curPosition(0), _size(0), _authority(0), _type(TYPE_FAT_FILE_CLOSED),_fileSizes(0){}

DFRobot_FlashFile::~DFRobot_FlashFile(){
//    //free(_flash);
//    //_flash = NULL;
//    _id = INVAILD_ID;
//    _curPosition = 0;
//    _size = 0;
//    _authority = 0;
//    _type = TYPE_FAT_FILE_CLOSED;
//    _fileSizes = 0;
}



uint8_t DFRobot_FlashFile::openRoot(DFRobot_Flash *flash){
    if(isOpen()){
        FLASH_DBG("root is already open, error: 1");
        return 1;
    }
    if(!openDirCommand((char *)"/", -1, &_id)){ //打开根目录
      FLASH_DBG("root dir open failed, error: 2");
      return 2;
    }
    
    if(flash->fatType() == TYPE_FAT12){
        _type = TYPE_FAT_FILE_ROOT12;
        //_fileSizes = 32 * 
    }else if(flash->fatType() == TYPE_FAT16){
        _type = TYPE_FAT_FILE_ROOT16;
    }else if(flash->fatType() == TYPE_FAT32){
        _type = TYPE_FAT_FILE_ROOT32;
    }else{
        FLASH_DBG("Invaild fat type, error: 2");
        return 3;
    }
    _flash = flash;
    _authority = AUTH_O_READ;
    return 0;
}


bool DFRobot_FlashFile::open(DFRobot_FlashFile* dirFile, const char* fileName, uint8_t oflag){//有可能打开的是目录，也有可能打开的是文件
    //判断文件是否已经打开
    if(isOpen()) return false;
    _flash = dirFile->_flash;

    if(_flash == NULL){
        FLASH_DBG("_flash is null\n");
        return false;
    }
    _authority = oflag;
    if(oflag == AUTH_O_READ){//查询是文件还是目录
      uint8_t type = fileInfoCommand((char *)fileName, dirFile->_id);
      if((type == 0) || (type > TYPE_FAT_FILE_SUBDIR) ){
        FLASH_DBG("check failed.");
        return false;
      }
      _type = type;
      if(type != TYPE_FAT_FILE_NORMAL){
        if(!openDirCommand((char *)fileName, dirFile->_id, &_id)){
          FLASH_DBG("open dir failed!");
          return false;
        }
        return true;
      }
    }
    if(!openFileCommand((char *)fileName, dirFile->_id, oflag, &_id, &_curPosition, &_size)){
        FLASH_DBG("get open file cmd pakage failed! Error: (3 << 4)");
        return false;
    }

    _type = TYPE_FAT_FILE_NORMAL;
    return true;
}

bool DFRobot_FlashFile::close(){//无法关闭根目录
    if(!isOpen() || isRoot()) {
      FLASH_DBG("is not open or root dir");
      return false;
    }

    if(_type == TYPE_FAT_FILE_NORMAL){
      syncFileCommand(_id);
      if(!closeCommand(_id)) return false;
    }else{
      if(!closeDirCommand(_id)) return false;
    }
    
    _type = TYPE_FAT_FILE_CLOSED;
    return true;
}

bool DFRobot_FlashFile::isOpen(){
    return _type != TYPE_FAT_FILE_CLOSED;
}

size_t DFRobot_FlashFile::write(const void* buf, uint16_t nbyte){
    if(!isFile() || !(_authority & AUTH_O_WRITE)) return 0;
    uint16_t t = writeFileCommand(_id, (void *)buf, nbyte);
    _curPosition += t;
    _size  += t;
    return t;
}

int16_t DFRobot_FlashFile::read(void){
    uint8_t b;
    return read(&b, 1) == 1 ? b : -1;
}

int16_t DFRobot_FlashFile::read(void* buf, uint16_t nbyte){
    if (!isOpen() || !(_authority & AUTH_O_READ)) return -1;
    //计算一个文件能存储的最大字节数，假如为4
    uint16_t  t = readFileCommand(_id, buf, nbyte);
    _curPosition += t;
    if(t == 0) return -1;
    return  (int16_t)t;
}

uint8_t DFRobot_FlashFile::sync(void){
    if(!isOpen()) return false;
    return syncFileCommand(_id);
}

uint32_t DFRobot_FlashFile::fileSize(void) {return _size;}

uint32_t DFRobot_FlashFile::curPosition(void) {return _curPosition;}

uint8_t DFRobot_FlashFile::seekSet(uint32_t pos){
    if(!isFile() || !isOpen() || (pos > _size)) return false;
    if(!seekFileCommand(_id, pos)){
      return false;
    }
    _curPosition = pos;
    return true;
}

int8_t DFRobot_FlashFile::readDir(char *name){
  if(!isDir() || (name == NULL)) return -1;
  if(readDirCommand(_id, name)) return 0;
  return -1;
}

bool DFRobot_FlashFile::isFile(void) {return _type == TYPE_FAT_FILE_NORMAL;}
bool DFRobot_FlashFile::isDir(void) {return _type >= TYPE_FAT_FILE_MIN_DIR;}
bool DFRobot_FlashFile::isRoot(void) {return _type == TYPE_FAT_FILE_ROOT16 || _type == TYPE_FAT_FILE_ROOT32 || _type == TYPE_FAT_FILE_ROOT12;}
uint8_t DFRobot_FlashFile::seekCur(uint32_t pos) {return seekSet(_curPosition + pos);}

bool DFRobot_FlashFile::makeDir(DFRobot_FlashFile& dir, const char* dirName) {  return makeDir(&dir, dirName);}
bool DFRobot_FlashFile::makeDir(DFRobot_FlashFile* dir, const char* dirName){
  if(!dir->isRoot()){
    FLASH_DBG("parent dir is not root dir.");
    return false; 
  } 
  return mkdirCommand(dir->_id, (char *)dirName);
}

void DFRobot_FlashFile::rewind(void){
  if(!isOpen()) {
    Serial.println("is not open");
    return;
  }
  
  if(!rewindCommand(_id)){
    Serial.println("rewind dir is failed.");
    FLASH_DBG("rewind dir is failed.");
  }

}

bool DFRobot_FlashFile::reRootDir(){
  return rewindRootDir();
}

uint8_t DFRobot_FlashFile::remove(DFRobot_FlashFile& dirFile, const char* fileName) {  return remove(&dirFile, fileName);}
uint8_t DFRobot_FlashFile::remove(DFRobot_FlashFile* dirFile, const char* fileName){
  return removeCommand(dirFile->_id, (char *)fileName);
}

bool DFRobot_FlashFile::changeDir(bool absolute, const char* fileName){
   return changeDirCommand(absolute, fileName);
}

String DFRobot_FlashFile::getWorkspacePath(){
    return getPWDCommand();
}

boolean DFRobot_FlashFile::del(uint32_t pos, uint32_t num, bool flag){
  if(!isFile() || !isOpen() || (pos > _size)) return false;
  if(flag){
     if(pos < num) return false;
  }else{
     if(_size - pos < num) return false;
  }
  if(!delCommand(_id, pos, num, flag)){
    return false;
  }
  if(flag) _curPosition = pos - num;
  else _curPosition = pos;
  _size -= num;
  return true;
}
boolean DFRobot_FlashFile::insert(uint32_t pos, uint8_t c, uint32_t num){
  //
  if(!isFile() || !isOpen() || (pos > _size)) return false;
  if(!insertOneCommand(_id, pos, c, num)){
    return false;
  }
  _curPosition = pos + num;
  _size += num;
  return true;
}
boolean DFRobot_FlashFile::insert(uint32_t pos, void *buf, uint32_t len){
  //
  if(!isFile() || !isOpen() || (pos > _size)) return false;
  if(!insertCommand(_id, pos, buf, len)){
    return false;
  }
  _curPosition = pos + len;
  _size += len;
  return true;                                                                                                                     
}
  


