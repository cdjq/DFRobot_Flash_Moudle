/*!
 * @file writeSensorData.ino
 * @brief 将A0模拟口读到的数据，写入文件中保存。
 * @copyright Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license The MIT License (MIT)
 * @author [Arya](xue.peng@dfrobot.com)
 * @version V1.0
 * @date 2021-11-04
 * @url https://github.com/DFRobot/DFRobot_Flash_Moudle
 */

#include "DFRobot_CSV_0870.h"
#include "DFRobot_Flash_Moudle.h"

#define SENSOR_PIN         A0
#define COLLECTION_TIMES   10  //采集10次数据  
DFRobot_FlashMoudle_IIC iic(/*addr=*/0x55);
DFRobot_FlashMoudle flash;
DFRobot_File myFile;
DFRobot_CSV_0870 csv;

uint32_t number = 0; //记录采集的次数，当等于COLLECTION_TIMES时，停止采集
int      value  = 0; //保存A0口上采集的数据的值

void setup(){
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  Serial.print("Initializing Wire bus...");
  int err = iic.begin();
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

  myFile = flash.open("SENSOR.CSV", FILE_WRITE);

  if(!myFile){
    Serial.println("error opening SENSOR.csv.");
    while(1) yield();
  }
  
  uint8_t ret = csv.begin(&myFile);
  if(ret != 0){
    Serial.print("csv initialize fail, ret=");
    Serial.println(ret);
    while(1) yield();
  }

  //定义第A列为日期：DATE，第B列为序号：NUMBER， 第C列为采集的数据的值：VALUE
  //print表示按A~Z的顺序写一项，即一个单元格数据，println表示写某行的最后一项，并开启新行
  csv.print("DATE"); csv.print("NUMBER"); csv.println("VALUE"); 
}

void loop() {
  if(number == COLLECTION_TIMES){
    myFile.close();
    Serial.println("write sensor data end!");
    while(1) yield();
  }
  number += 1;
  value = analogRead(SENSOR_PIN);
  Serial.print("Write raw number: ");Serial.println(number);
  csv.print(__DATE__); csv.print(number); csv.println(value);
  delay(1000);
}
