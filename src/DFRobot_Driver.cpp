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
        DBG("device not found.");
        return 2;
    }
    return 0;
}

void DFRobot_FlashMoudle_IIC::sendData(void* pData, size_t size){
    if(pData == NULL){
        DBG("pData is NULL");
        return;
    }
    uint8_t *pBuf = (uint8_t *)pData;
    size_t remain =  size ;
    flush();
    for(int i = 0; i < size; i++){
        DBG(pBuf[i],HEX);
    }
    DBG(size);
    while(remain){
        size = (remain > IIC_MAX_TRANSFER) ? IIC_MAX_TRANSFER : remain;
        _pWire->beginTransmission(_addr);
        _pWire->write(pBuf, size);
        _pWire->endTransmission();
        remain -= size;
        pBuf += size;

    }
}

uint8_t DFRobot_FlashMoudle_IIC::recvData(void* pData, size_t size){
    if(pData == NULL){
        DBG("pData is NULL");
        return 0;
    }
    uint8_t *pBuf = (uint8_t *)pData;
    size_t remain =  size;
    uint8_t len = 0;
    while(remain){
        size = (remain > IIC_MAX_TRANSFER) ? IIC_MAX_TRANSFER : remain;
        _pWire->requestFrom(_addr, size);
        for(size_t i = 0; i < size; i++){
            //uint32_t t = millis();
            pBuf[i] = _pWire->read();
            DBG(pBuf[i],HEX);
            //Serial.println(pBuf[i],HEX);
            //Serial.println(millis() - t);
        }
        remain -= size;
        pBuf += size;
        len += size;
    }
    return len;
}

void DFRobot_FlashMoudle_IIC::writeReg(uint8_t reg, void* pData, size_t size){
    if(pData == NULL){
        DBG("pData is NULL");
        return ;
    }
    uint8_t *pBuf = (uint8_t *)pData;
    size_t remain =  size;
    flush();
    //for(int i = 0; i < size; i++){
     //   Serial.print(pBuf[i], HEX);
     //   Serial.print(", ");
    //}
    //Serial.println();
    while(remain){
        size = (remain > (IIC_MAX_TRANSFER - 2)) ? (IIC_MAX_TRANSFER - 2) : remain;
        _pWire->beginTransmission(_addr);
        _pWire->write(reg);
        _pWire->write(0x00); //写
        _pWire->write(pBuf, size);
        _pWire->endTransmission();
        //Serial.print("size=");Serial.println(size);
        reg += size;
        remain -= size;
        pBuf += size;
        //delay(50);
    }
}
uint8_t DFRobot_FlashMoudle_IIC::readReg(uint8_t reg, void* pData, size_t size){
    if(pData == NULL){
        DBG("pData is NULL");
        return 0;
    }
    uint8_t *pBuf = (uint8_t *)pData;
    size_t remain =  size;
    flush();
    uint8_t readlen = 0;
    while(remain){
        size = (remain > (IIC_MAX_TRANSFER)) ? (IIC_MAX_TRANSFER) : remain;
        _pWire->beginTransmission(_addr);
        _pWire->write(reg);
        _pWire->write(0x80); //读
        _pWire->endTransmission();
        _pWire->requestFrom(_addr, size);
        for(size_t i = 0; i < size; i++){
            //uint32_t t = millis();
            pBuf[i] = _pWire->read();
            DBG(pBuf[i],HEX);
            //Serial.println(millis() - t);
            yield();
        }
        readlen += size;
        reg += size;
        remain -= size;
        pBuf += size;
        //delay(2);
    }
    return readlen;
}

void DFRobot_FlashMoudle_IIC::flush(){
    if(_pWire->available()){
        _pWire->read();
        delay(1);
    }
}