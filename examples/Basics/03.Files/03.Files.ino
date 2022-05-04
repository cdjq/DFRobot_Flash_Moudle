/*!
 * @file 03.files.ino
 * @brief 文件查询，创建，移除等操作
 * @details 查询某个文件是否存在，如果存在，则移除，移除成功后，再创建，如果不存在，则创建
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
DFRobot_File myFile;

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

  if (flash.exists("example.txt")) {
    Serial.println("example.txt is already exists, please delete.");
    Serial.print("Removing [example.txt] file...");
    if(flash.remove("example.txt")){ //移除也可以用flash.remove("flash")
        Serial.println("done.");
      }else{
        Serial.println("failed.");
        while(1) yield();
    }
  } else {
    Serial.println("example.txt doesn't exist.");
    Serial.print("Creating [example.txt] file...");
    myFile = flash.open("example.txt", FILE_WRITE);
    if(myFile){
      Serial.println("done.");
    }else{
        Serial.println("failed.");
        while(1) yield();
    }
    myFile.close();
  }

  if (flash.exists("example.txt")) {
    Serial.println("example.txt exists.");
  } else {
    Serial.println("example.txt doesn't exist.");
  }
}

void loop() {
  // nothing happens after setup finishes.
}



