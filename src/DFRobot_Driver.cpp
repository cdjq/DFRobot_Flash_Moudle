/*!
 * @file DFRobot_Flash_Moudle.cpp
 * @brief 定义DFRobot_Sensor 类的基础结构
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
#include "DFRobot_Flash_Moudle.h"

#if defined(ESP32)
#define IIC_MAX_TRANSFER   128
//#elif defined(M0)
//#define IIC_MAX_TRANSFER   255
#else
#define IIC_MAX_TRANSFER   32 //< AVR/8266
#endif

DFRobot_FlashMoudle_IIC::DFRobot_FlashMoudle_IIC(uint8_t addr,TwoWire *pWire)
  :_pWire(pWire), _addr(addr){}

DFRobot_FlashMoudle_IIC::~DFRobot_FlashMoudle_IIC(){
    DBG((uint32_t)(uint32_t *)_pWire, HEX);
    _pWire = NULL;
}

uint8_t DFRobot_FlashMoudle_IIC::begin(uint32_t freq){
    if(_pWire == NULL){
        DBG("_pWire is null");
        return 1;
    }
    delay(500);
    _pWire->begin();
    _pWire->beginTransmission(_addr);
    _pWire->setClock(freq);
    byte ret = _pWire->endTransmission();
    if(ret != 0){
        DRV_DBG("device not found.");
        return 2;
    }
    return 0;
}

bool DFRobot_FlashMoudle_IIC::sendData(void* pData, uint16_t size, bool endflag){
    if(pData == NULL){
        DRV_DBG("pData is NULL");
        return false;
    }
    uint8_t *pBuf = (uint8_t *)pData;
    uint16_t remain =  size ;
    uint8_t ret = 0;
    DRV_DBG();
    flush();
    DRV_DBG();
    while(remain){
        size = (remain > IIC_MAX_TRANSFER) ? IIC_MAX_TRANSFER : remain;
        remain -= size;
        _pWire->beginTransmission(_addr);
        _pWire->write(pBuf, size);
        if(remain){
            ret = _pWire->endTransmission(false);
            pBuf += size;
        }else{
            ret = _pWire->endTransmission(endflag);
        }
        if(ret != 0) return false;
        yield();
    }
    return true;
}
bool DFRobot_FlashMoudle_IIC::recvData(void* pData, uint16_t size, bool endflag){
    if(pData == NULL){
        DRV_DBG("pData is NULL");
        return false;
    }
    uint8_t *pBuf = (uint8_t *)pData;
    uint16_t remain =  size ;
    while(remain){
        size = (remain > IIC_MAX_TRANSFER) ? IIC_MAX_TRANSFER : remain;
        remain -= size;
        if(remain) _pWire->requestFrom(_addr, size, false);
        else _pWire->requestFrom(_addr, size, endflag);
        for(size_t i = 0; i < size; i++){
            pBuf[i] = _pWire->read();
            yield();
        }
        if(remain) pBuf += size;
    }
    return true;
}

void DFRobot_FlashMoudle_IIC::flush(){
    if(_pWire->available()){
        _pWire->read();
        delay(1);
        yield();
    }
}