/*!
 * @file 02.dirs.ino
 * @brief 文件夹查询，创建，移除等操作
 * @details 查询某个文件夹是否存在，如果存在，则移除，移除成功后，再创建，如果不存在，则创建
 * @copyright Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license The MIT License (MIT)
 * @author [Arya](xue.peng@dfrobot.com)
 * @version V1.0
 * @date 2021-11-04
 * @url https://github.com/DFRobot/DFRobot_Flash_Moudle
 */
#include "DFRobot_Flash_Moudle.h"

DFRobot_FlashMoudle_IIC iic(/*addr=*/0x55);
DFRobot_FlashMoudle flash;
DFRobot_File floder;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  Serial.print("Initializing Wire bus...");
  uint8_t err = iic.begin();
  if(err != 0){
    Serial.print("failed! error code is 0x");
    Serial.println(err, HEX);
    while(1) yield();
  }
  Serial.println("done.");

  Serial.print("Initializing Flash Memory Module...");
  err = flash.begin(&iic);
  if(err != 0){
    Serial.print("failed! error code is 0x");
    Serial.println(err, HEX);
    while(1) yield();
  }
  Serial.println("done.");

  
  // 查询根目录下是否存在目录flash，如果存在则移除，如果不存在则创建
  if(flash.exists("flash")){
      Serial.println("flash floder is already exists, please delete.");
      // 将根目录下名为flash的空文件夹移除， rmdir和remove只能移除空文件夹
      Serial.print("Removing flash floder...");
      if(flash.rmdir("flash")){ //移除也可以用flash.remove("flash")
        Serial.println("done.");
      }else{
        Serial.println("failed.");
        while(1) yield();
      }
  }else{
      Serial.println("flash floder doesn't exist.");
      // 在根目录下创建文件夹flash
      Serial.print("Creating flash floder...");
      if(flash.mkdir("flash")){
          Serial.println("done.");
      }else{
          Serial.println("failed.");
      }

  }

  // 再次检测根目录下是否存在目录flash
  if(flash.exists("flash")){
      Serial.println("flash floder exists.");
  }else{
      Serial.println("flash floder doesn't exist.");
  }
}

void loop(){
  
}


