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
    Serial.println("example.txt exists.");
  } else {
    Serial.println("example.txt doesn't exist.");
  }

  // open a new file and immediately close it:
  Serial.println("Creating example.txt...");
  myFile = flash.open("example.txt", FILE_WRITE);
  myFile.close();

  // Check to see if the file exists:
  if (flash.exists("example.txt")) {
    Serial.println("example.txt exists.");
  } else {
    Serial.println("example.txt doesn't exist.");
  }

  // delete the file:
  Serial.println("Removing example.txt...");
  flash.remove("example.txt");

  if (flash.exists("example.txt")) {
    Serial.println("example.txt exists.");
  } else {
    Serial.println("example.txt doesn't exist.");
  }
}

void loop() {
  // nothing happens after setup finishes.
}



