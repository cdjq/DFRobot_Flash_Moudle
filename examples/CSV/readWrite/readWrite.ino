/*!
 * @file writeRead.ino
 * @brief 顺序写csv文件，并读取内容。
 * @copyright Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license The MIT License (MIT)
 * @author [Arya](xue.peng@dfrobot.com)
 * @version V1.0
 * @date 2021-11-04
 * @url https://github.com/DFRobot/DFRobot_Flash_Moudle
 */

#include "DFRobot_CsvFile.h"
#include "DFRobot_Flash_Moudle.h"

DFRobot_FlashMoudle_IIC iic(/*addr=*/0x55);
DFRobot_FlashMoudle flash;
DFRobot_File myFile;
DFRobot_CsvFile csv;

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

  myFile = flash.open("TEST.CSV", FILE_WRITE);
  
  uint8_t ret = csv.begin(&myFile);
  if(ret != 0){
    Serial.print("csv initialize fail, ret=");
    Serial.println(ret);
    while(1) yield();
  }
  
  // if the file opened okay, write to it:
  if(myFile) {
    Serial.println("Writing to TEST.CSV...");
    csv.print("group");csv.print("number");csv.println("mark");        //以csv格式往文件写入数据，用print输入每列的值，用println输入值并换行
    csv.print("\"Jerry\"");csv.print('1');csv.println(3.65);
    csv.print("3,Herny");csv.print("10");csv.println(99.5);
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening TEST.csv");
  }

  myFile = flash.open("TEST.CSV");
  if (myFile) {
    Serial.println("TEST.CSV:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening TEST.csv");
  }
}

void loop() {
}
