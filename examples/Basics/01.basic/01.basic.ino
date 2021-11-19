/*!
 * @file 01.basic.ino
 * @brief 文件和目录的基础操作。
 * @details 包括创建文件、创建目录、读写文件和关闭文件，此demo旨在向用户介绍如何分别在根目录和子目录下创建目录和文件， 
 * @n 注意，子目录级数（包括根目录在内）不能超过10级，文件名或目录名不能超过11字节，且文件名不区分大小写，无论小写
 * @n 还是大写，写入flash内的文件名统一为大写。（通过操作系统创建的文件或目录除外）
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

  /* 目录创建 */
  // 在根目录下(/)创建一个目录名为DIR的目录
  /**
   * @fn mkdir
   * @param filepath 目录的绝对路径，根目录为/
   * @param mode  打开权限 FILE_READ：以只读的方式打开文件， FILE_WRITE：以读写创建的方式打开文件
   * @return 目录已存在或创建成功返回true，不存在并创建失败返回false
   */
  bool ret = flash.mkdir("DIR");
  if(ret){
    Serial.println("Create directory [DIR] successfully.");
  }else{
    Serial.println("Create directory [DIR] failed");
    while(1) yield();  // 创建失败，则死等，yield函数让出线程，防止程序循环死等崩溃
  }

  // 在路径/DIR下创建一个目录名为FILE的文件,注意：在子目录下创建新的子目录，成功的前提是父目录已经存在，否则会创建失败
  ret = flash.mkdir("DIR/FILE");
  if(ret){
    Serial.println("Create directory [DIR/FILE] successfully.");
  }else{
    Serial.println("Create directory [DIR/FILE] failed");
    while(1) yield();  // 创建失败，则死等，yield函数让出线程，防止程序循环死等崩溃
  }

  /* 文件创建或读写打开、 写、 关闭*/
  // 在根目录下(/)以读写的方式打开或创建一个名为TEST.TXT的文件,并向其写入一句话"I'm TEST.TXT"
  myFile = flash.open("TEST.TXT", FILE_WRITE);
  if(myFile){
    Serial.print("Writing to TEST.TXT...");

    myFile.println("I'm TEST.TXT");
    myFile.close();// close the file:

    Serial.println("done.");
  }else{
    Serial.println("error opening TEST.TXT");
  }

  // 在路径/DIR下创建或打开一个名为SAMPLE.TXT的文件,并向其写入一句话"I'm SAMPLE.TXT"
  /**
   * @fn open
   * @param filepath 文件或目录的绝对路径，根目录为/
   * @param mode  打开权限 FILE_READ：以只读的方式打开文件， FILE_WRITE：以读写创建的方式打开文件
   */
  myFile = flash.open("DIR/SAMPLE.TXT", FILE_WRITE);
  if(myFile){
    Serial.print("Writing to DIR/SAMPLE.TXT...");

    myFile.println("I'm SAMPLE.TXT");
    myFile.close();// close the file:

    Serial.println("done.");
  }else{
    Serial.println("error opening DIR/SAMPLE.TXT");
  }
  
  /* 文件读打开、文件读、文件关闭 */
  // 读取根目录下名为TEST.TXT文件的内容，并用串口打印出来
  myFile = flash.open(/*filepath =*/"TEST.TXT", /*mode =*/FILE_READ);
  if (myFile) {
    Serial.println("TEST.TXT:  ");
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    myFile.close(); // close the file:
  } else {
    Serial.println("error opening TEST.TXT"); // if the file didn't open, print an error:
  }

  // 读取/DIR路径下名为SAMPLE.TXT的文件的内容，并用串口打印出来
  myFile = flash.open("DIR/SAMPLE.TXT", FILE_READ);
  if (myFile) {
    Serial.println("DIR/SAMPLE.TXT:  ");
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    myFile.close(); // close the file:
  } else {
    Serial.println("error opening DIR/SAMPLE.TXT"); // if the file didn't open, print an error:
  }


}

void loop() {
  // nothing happens after setup finishes.
}


