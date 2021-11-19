
#include "DFRobot_Flash_Moudle.h"

DFRobot_FlashMoudle_IIC iic(/*addr=*/0x55);
DFRobot_Flash module;
DFRobot_FlashFile root;

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
  err = module.init(&iic);
  if(err != 0){
    Serial.print("failed! error code is 0x");
    Serial.println(err, HEX);
    while(1) yield();
  }
  Serial.println("done.");

  Serial.print("\nFat type: ");//FAT系统类型 
  switch(module.fatType()){
    case module.eTypeFat12:
      Serial.println("FAT12");
      break;
    case module.eTypeFat16:
      Serial.println("FAT16");
      break;
    case module.eTypeFat32:
      Serial.println("FAT32");
      break;
    default:
      Serial.println("Unknown"); 
  }

  Serial.print("\nFlash capacity: ");
  Serial.print(module.size());
  Serial.println("bytes");
  //剩余可用空间
  //一个文件最小占8K内存
  //此flash最多存储目录和字节加起来共512个
  //打印出flash内所有的文件和目录的实际大小和占用空间，时间属性等

}


void loop(void) {

}
