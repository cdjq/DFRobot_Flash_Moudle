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
#include "utility/DFRobot_Flash.h"

DFRobot_File::DFRobot_File(DFRobot_FlashFile f, const char *name){
   _file = (DFRobot_FlashFile *)malloc(sizeof(DFRobot_FlashFile)); //请不要在析构函数里面随便free分配的空间
   DBG((uint32_t)(uint32_t *)&_file, HEX);
    if(_file){
        memcpy(_file, &f, sizeof(DFRobot_FlashFile));
        strncpy(_name, name, 31);
        _name[31] = 0;
    }
    DBG(_name);
}

DFRobot_File::DFRobot_File(void)
  :_file(NULL){
   _name[0] = 0;
}

char *DFRobot_File::name(void) {
  return _name;
}


String DFRobot_File::getAbsolutePath(){
  String str = "";
  if(_file){
    str = _file->absolutePath();
  }
  return str;
}

String DFRobot_File::getParentDirectory(){
  String str = "";
  if(_file){
    str = _file->parentDirectory();
  }
  return str;
}

boolean DFRobot_File::isDirectory(void) {
  return (_file && _file->isDir());
}

size_t DFRobot_File::write(uint8_t val) {
  return write(&val, 1);
}

size_t DFRobot_File::write(const uint8_t *buf, size_t size) {
  size_t t;
  if (!_file) {
    return 0;
  }
  t = _file->write(buf, size);
  return t;
}

int DFRobot_File::read() {
  if (_file) 
    return _file->read();
  return -1;
}

int DFRobot_File::peek() {
  if (! _file) 
    return 0;

  int c = _file->read();
  if (c != -1) _file->seekCur(-1);
  return c;
}

int DFRobot_File::read(void *buf, uint16_t nbyte) {
  if (_file) 
    return _file->read(buf, nbyte);
  return 0;
}

int DFRobot_File::available() {
  if (! _file) return 0;

  uint32_t n = size() - position();
  return n > 0X7FFF ? 0X7FFF : n;
}

void DFRobot_File::flush() {
  if (_file)
    _file->sync();
}

boolean DFRobot_File::seek(uint32_t pos) {
  if (! _file) return false;

  return _file->seekSet(pos);
}

uint32_t DFRobot_File::position() {
  if (! _file) return -1;
  return _file->curPosition();
}

uint32_t DFRobot_File::size() {
  if (! _file) return 0;
  return _file->fileSize();
}

void DFRobot_File::close() {
  if (_file) {
    _file->close();
    free(_file); 
    _file = NULL;
  }
}

DFRobot_File::operator bool() {
  if (_file) 
    return  _file->isOpen();
  return false;
}

/*
boolean DFRobot_File::del(uint32_t pos, uint32_t num, bool flag){
  //
  if (_file) 
    return  _file->del(pos, num, flag);
  return false;
}

boolean DFRobot_File::insert(uint32_t pos, uint8_t c, uint32_t num){
  //
  if (_file) 
    return  _file->insert(pos, c, num);
  return false;
}
boolean DFRobot_File::insert(uint32_t pos, void *buf, uint32_t len){
  //
  if (_file) 
    return  _file->insert(pos, buf, len);
  return false;
}
*/