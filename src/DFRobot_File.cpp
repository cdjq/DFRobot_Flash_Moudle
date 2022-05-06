/*!
 * @file DFRobot_File.cpp
 * @brief DFRobot_File 类的实现
 * @details 文件的相关操作的实现：
 * @n 获取文件名
 * @n 获取父级目录
 * @n 获取绝对路径
 * @n 判断是目录还是文件
 * @n 读或写文件
 * @n 打开并读取下级目录
 * @n 返回读取目录的首位置
 * @n 判断文件是否打开
 * @n 获取或设置文件读写指针的位置
 * @n 获取文件的大小
 * @n 关闭或截断或关闭文件
 * @n 同步文件内容
 * @n 判断文件还有多少字节未读取
 * @copyright	Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author [Arya](xue.peng@dfrobot.com)
 * @version  V1.0
 * @date  2021-11-19
 * @url https://github.com/DFRobot/DFRobot_Flash_Moudle
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

bool DFRobot_File::close(bool truncate) {
  bool status = false;
  if (_file) {
    status = _file->close(truncate);
    free(_file); 
    _file = NULL;
  }
  return status;
}

DFRobot_File::operator bool() {
  if (_file) 
    return  _file->isOpen();
  return false;
}

