/*!
 * @file DFRobot_ComCSV.cpp
 * @brief 定义 DFRobot_ComCSV 类的实现
 * @details 组织了一系列的按csv格式写入文件的接口函数
 * @copyright	Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author [Arya](xue.peng@dfrobot.com)
 * @version  V1.0
 * @date  2022-05-6
 * @url https://github.com/DFRobot/DFRobot_Flash_Moudle
 */

#include "DFRobot_ComCSV.h"

#ifndef COMCSV_DBG
#if 0
#define COMCSV_DBG(...) {Serial.print("["); Serial.print(__FUNCTION__); Serial.print("(): "); Serial.print(__LINE__); Serial.print(" ] "); Serial.println(__VA_ARGS__);}
#else
#define COMCSV_DBG(...)
#endif
#endif


DFRobot_ComCSV::DFRobot_ComCSV()
{
  _commaFlag = 0;
  _commaFlag1 = 0;
}

DFRobot_ComCSV::~DFRobot_ComCSV(){

}

int DFRobot_ComCSV::begin(){
  return 0;
}
size_t DFRobot_ComCSV::write(uint8_t val)
{
  return write((const uint8_t *)&val, 1);
}

size_t DFRobot_ComCSV::write(const uint8_t *buf, size_t size)
{
  size_t n = writeData((void *)buf, size);

  if((_commaFlag > 0) && (_commaFlag1 == 0)){
    char c = ',';
    n += writeData((void *)&c, 1);
  }
  return n;
}

size_t DFRobot_ComCSV::print(const String &s){
  COMCSV_DBG(_commaFlag);
  if(_commaFlag < 255) _commaFlag += 1;
  size_t nb = Print::print(s);
  if((_commaFlag < 255) && _commaFlag) _commaFlag -= 1;
  COMCSV_DBG(_commaFlag);
  return nb;
}

size_t DFRobot_ComCSV::print(const char str[]){
  COMCSV_DBG(_commaFlag);
  if(_commaFlag < 255) _commaFlag += 1;
  size_t nb = Print::print(str);
  if((_commaFlag < 255) && _commaFlag) _commaFlag -= 1;
  COMCSV_DBG(_commaFlag);
  return nb;
}

size_t DFRobot_ComCSV::print(char c){
  COMCSV_DBG(_commaFlag);
  if(_commaFlag < 255) _commaFlag += 1;
  size_t nb = Print::print(c);
  if((_commaFlag < 255) && _commaFlag) _commaFlag -= 1;
  COMCSV_DBG(_commaFlag);
  return nb;  
}

size_t DFRobot_ComCSV::print(unsigned char b, int base){
  COMCSV_DBG(_commaFlag);
  if(_commaFlag < 255) _commaFlag += 1;
  size_t nb = Print::print(b, base);
  if((_commaFlag < 255) && _commaFlag) _commaFlag -= 1;
  COMCSV_DBG(_commaFlag);
  return nb;
}

size_t DFRobot_ComCSV::print(int n, int base){
  COMCSV_DBG(_commaFlag);
  if(_commaFlag < 255) _commaFlag += 1;
  size_t nb = print((long)n, base);
  if((_commaFlag < 255) && _commaFlag) _commaFlag -= 1;
  COMCSV_DBG(_commaFlag);
  return nb;
}

size_t DFRobot_ComCSV::print(unsigned int n, int base){
  COMCSV_DBG(_commaFlag);
  if(_commaFlag < 255) _commaFlag += 1;
  size_t nb = Print::print(n,base);
  if((_commaFlag < 255) && _commaFlag) _commaFlag -= 1;
  COMCSV_DBG(_commaFlag);
  return nb;
}

size_t DFRobot_ComCSV::print(long n, int base)
{
  COMCSV_DBG(_commaFlag);
  if(_commaFlag < 255) _commaFlag += 1;
  int t = 0;
  char buf[8 * sizeof(long) + 1]; // Assumes 8-bit chars plus zero byte.
  char *str = &buf[sizeof(buf) - 1];
  *str = '\0';
  // prevent crash if called with base == 1
  if (base < 2) base = 10;
  if(n < 0){
      print('-');
      n = -n;
      t += 1;
  }
  do {
    char c = n % base;
    n /= base;

    *--str = c < 10 ? c + '0' : c + 'A' - 10;
  } while(n);
  t += write(str);
  if((_commaFlag < 255) && _commaFlag) _commaFlag -= 1;
  COMCSV_DBG(_commaFlag);
  return t;
}

size_t DFRobot_ComCSV::print(unsigned long n, int base){
  COMCSV_DBG(_commaFlag);
  if(_commaFlag < 255) _commaFlag += 1;
  size_t nb = Print::print(n, base);
  if((_commaFlag < 255) && _commaFlag) _commaFlag -= 1;
  COMCSV_DBG(_commaFlag);
  return nb;
}

size_t DFRobot_ComCSV::print(double number, int digits){
  size_t nb = 0;
  COMCSV_DBG(_commaFlag);
  if(_commaFlag < 255) _commaFlag += 1;
  if (isnan(number)){
    nb = Print::print("nan");
    if((_commaFlag < 255) && _commaFlag) _commaFlag -= 1;
    COMCSV_DBG(_commaFlag);
    return nb; 
  } 
  if (isinf(number)){
    nb = Print::print("inf");
    if((_commaFlag < 255) && _commaFlag) _commaFlag -= 1;
    COMCSV_DBG(_commaFlag);
    return nb;
  } 
  if (number > 4294967040.0){
    nb = Print::print ("ovf");  // constant determined empirically
    if((_commaFlag < 255) && _commaFlag) _commaFlag -= 1;
    COMCSV_DBG(_commaFlag);
    return nb;
  }
  if (number <-4294967040.0){
    nb = Print::print ("ovf");
    if((_commaFlag < 255) && _commaFlag) _commaFlag -= 1;
    COMCSV_DBG(_commaFlag);
    return nb;
  }

  String str = "";
  if (number < 0.0)
  {
     str += '-';
     nb += 1;
     number = -number;
  }
  double rounding = 0.5;
  for (uint8_t i=0; i < digits; ++i)
    rounding /= 10.0;
  number += rounding;
  unsigned long int_part = (unsigned long)number;
  double remainder = number - (double)int_part;
  str += int_part;
  // Print the decimal point, but only if there are digits beyond
  if (digits > 0){
    str += '.';
  }
  while (digits-- > 0)
  {
    remainder *= 10.0;
    unsigned int toPrint = (unsigned int)(remainder);
    str += toPrint;
    remainder -= toPrint; 
  } 
  Print::print(str);
  if((_commaFlag < 255) && _commaFlag) _commaFlag -= 1;
  COMCSV_DBG(_commaFlag);
  return nb;
}

size_t DFRobot_ComCSV::println(const String &s){
  COMCSV_DBG(_commaFlag1);
  if(_commaFlag1 < 255) _commaFlag1 += 1;
  size_t nb = Print::println(s);
  if((_commaFlag1 < 255) && _commaFlag1) _commaFlag1 -= 1;
  COMCSV_DBG(_commaFlag1);
  return nb;
}

size_t DFRobot_ComCSV::println(const char str[]){
  COMCSV_DBG(_commaFlag1);
  if(_commaFlag1 < 255) _commaFlag1 += 1;
  size_t nb = Print::println(str);
  if((_commaFlag1 < 255) && _commaFlag1) _commaFlag1 -= 1;
  COMCSV_DBG(_commaFlag1);
  return nb;
}
size_t DFRobot_ComCSV::println(char c){
  COMCSV_DBG(_commaFlag1);
  if(_commaFlag1 < 255) _commaFlag1 += 1;
  size_t nb = Print::println(c);
  if((_commaFlag1 < 255) && _commaFlag1) _commaFlag1 -= 1;
  COMCSV_DBG(_commaFlag1);
  return nb;
}

size_t DFRobot_ComCSV::println(unsigned char b, int base){
  COMCSV_DBG(_commaFlag1);
  if(_commaFlag1 < 255) _commaFlag1 += 1;
  size_t nb = Print::println(b, base);
  if((_commaFlag1 < 255) && _commaFlag1) _commaFlag1 -= 1;
  COMCSV_DBG(_commaFlag1);
  return nb;
}

size_t DFRobot_ComCSV::println(int n, int base){
  COMCSV_DBG(_commaFlag1);
  if(_commaFlag1 < 255) _commaFlag1 += 1;
  size_t nb = println((long)n, base);
  if((_commaFlag1 < 255) && _commaFlag1) _commaFlag1 -= 1;
  COMCSV_DBG(_commaFlag1);
  return nb;
}

size_t DFRobot_ComCSV::println(unsigned int n, int base){
  COMCSV_DBG(_commaFlag1);
  if(_commaFlag1 < 255) _commaFlag1 += 1;
  size_t nb = Print::println(n,base);
  if((_commaFlag1 < 255) && _commaFlag1) _commaFlag1 -= 1;
  COMCSV_DBG(_commaFlag1);
  return nb;
}

size_t DFRobot_ComCSV::println(long n, int base){
  COMCSV_DBG(_commaFlag1);
  if(_commaFlag1 < 255) _commaFlag1 += 1;
  size_t nb = print(n, base);
  nb += Print::println();
  if((_commaFlag1 < 255) && _commaFlag1) _commaFlag1 -= 1;
  COMCSV_DBG(_commaFlag1);
  return nb;
}
size_t DFRobot_ComCSV::println(unsigned long n, int base){
  COMCSV_DBG(_commaFlag1);
  if(_commaFlag1 < 255) _commaFlag1 += 1;
  size_t nb = Print::println(n, base);
  if((_commaFlag1 < 255) && _commaFlag1) _commaFlag1 -= 1;
  COMCSV_DBG(_commaFlag1);
  return nb;
}

size_t DFRobot_ComCSV::println(double num, int digits){
  COMCSV_DBG(_commaFlag1);
  if(_commaFlag1 < 255) _commaFlag1 += 1;
  size_t nb = print(num, digits);
  nb += Print::println();
  if((_commaFlag1 < 255) && _commaFlag1) _commaFlag1 -= 1;
  COMCSV_DBG(_commaFlag1);
  return nb;
}
size_t DFRobot_ComCSV::println(void){
  COMCSV_DBG(_commaFlag1);
  if(_commaFlag1 < 255) _commaFlag1 += 1;
  size_t nb = Print::println();
  if((_commaFlag1 < 255) && _commaFlag1) _commaFlag1 -= 1;
  COMCSV_DBG(_commaFlag1);
  return nb;
}
