/*!
 * @file read.ino
 * @brief 打开TEST.CSV文件，获得总的行数，和指定行的列数，读取指定的行的值，指定位置的值，并打印出来
 * @n 实验现象：串口监视器打印出TEST.CSV文件指定的内容
 *
 * @copyright   Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author [Arya](xue.peng@dfrobot.com)
 * @version  V1.0
 * @date  2021-11-17
 * @get from https://www.dfrobot.com
 * @url https://github.com/DFRobot/DFRobot_Flash_Moudle
 */

#include "DFRobot_CsvFile.h"
#include "DFRobot_Flash_Moudle.h"

DFRobot_FlashMoudle_IIC iic(/*addr=*/0x55);
DFRobot_FlashMoudle flash;
DFRobot_File myFile;
DFRobot_File root;
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
  int row = csv.getRow();
  int column = csv.getColumn();
  Serial.print("Number of rows: ");
  Serial.println(row);
  Serial.print("Columns in the first row: ");
  Serial.println(column);

  String rowdata = csv.readRow(/*row = */1);//读取第一行数据
  Serial.print("First row data: ");
  Serial.println(rowdata);
  String coldata = csv.readColumn(/*col = */1);//读取第一列数据
  Serial.print("First column data: ");
  Serial.println(coldata);
  String itemData = csv.readItem(/*row = */1, /*col = */1);//读取第一列数据
  Serial.print("1 row and 1 column data: ");
  Serial.println(itemData);
  myFile.close();
}

void loop() {
}
