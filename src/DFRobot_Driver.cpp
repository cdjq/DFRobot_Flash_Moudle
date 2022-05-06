/*!
 * @file DFRobot_Driver.cpp
 * @brief 定义 DFRobot_Driver 抽象类子类的实现
 * @details 一系列的通信接口在不同通信协议下的实现，目前支持
 * @n I2C 协议
 * @copyright	Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author [Arya](xue.peng@dfrobot.com)
 * @version  V1.0
 * @date  2021-10-09
 * @url https://github.com/DFRobot/DFRobot_Flash_Moudle
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
#ifdef __AVR__
        if(remain) _pWire->requestFrom((uint8_t)_addr, (uint8_t)size, (uint8_t)false);
        else _pWire->requestFrom((uint8_t)_addr, (uint8_t)size, (uint8_t)endflag);
#else
        if(remain) _pWire->requestFrom(_addr, size, false);
        else _pWire->requestFrom(_addr, size, endflag);
#endif
        for(size_t i = 0; i < size; i++){
            pBuf[i] = _pWire->read();
            DRV_DBG(pBuf[i],HEX);
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