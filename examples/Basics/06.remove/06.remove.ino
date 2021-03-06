/*!
 * @file 06.remove.ino
 * @brief 移除flash内的所有文件后，再创建一个名为SD的文件夹。
 * 
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
DFRobot_File root;

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

  //01.打印目录
  Serial.println("============print dir list=============");
  root = flash.open("/");
  printDirectory(root, 0);
  
  //02.返回目录的第1个文件，并依次删除
  Serial.println("============remove============");
  root.rewindDirectory();
  removeDirectory(root);
  
  //02.打印目录
  Serial.println("============print=============");
  root = flash.open("/");
  root.rewindDirectory();
  flash.mkdir("/sd");
  printDirectory(root, 0);
  Serial.println("=============end==============");
  root.close();
}

void loop(){
  
}
//06移除文件夹和文件夹里面的文件
void removeDirectory(DFRobot_File dir) {
  while (true) {
    DFRobot_File entry =  dir.openNextFile();
    if (!entry) {
      Serial.print("remove [" + String(dir.name()) +"] floder...");
      String abpath = dir.getAbsolutePath();
      dir.close();
      
      if(flash.rmdir(abpath)){
          Serial.println("done.");
      }else{
          Serial.println("failed.");
      }
      break;
    }

    if (entry.isDirectory()) {
      removeDirectory(entry);
    } else {
      // files have sizes, directories do not
      Serial.print("remove [" + String(entry.name()) +"] file...");
      String abpath = entry.getAbsolutePath();
      entry.close();
      if(flash.remove(abpath)){
          Serial.println("done.");
      }else{
          Serial.println("failed.");
      }
    }
  }
}

void printDirectory(DFRobot_File dir, int numTabs) {
  while (true) {
    DFRobot_File entry = dir.openNextFile();
    if (!entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}

