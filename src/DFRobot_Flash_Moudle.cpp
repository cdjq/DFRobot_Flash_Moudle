/*!
 * @file DFRobot_Flash_Moudle.cpp
 * @brief 此文件中定义了DFRobot_File基本文件操作类，DFRobot_FlashMoudle flash模块操作类， DFRobot_FlashMoudle_IIC驱动类。
 * @details 这是一个文件系统库，专门用来操作DFRobot自制的一款通信接口为I2C的flash Memory Moudle。使用该库，可以用文件的方式对
 * @n 该flash存储模块进行操作，你可以直接在该模块上面创建，读，写文件，或者创建目录。此驱动它有以下特点：
 * @n 1. 能实现用文件的方式操作特定的存储模块;
 * @n 2. 目前只支持短文件名
 * @n 3. 目前支持I2C驱动
 * @n 相关类的简单介绍：
 * @n DFRobot_File： 此类定义了文件或目录的简单操作，包括读、写、增加、删除文件内容，打开目录的下级文件，以及获取文件名或目录名
 * @n DFRobot_FlashMoudle：此类定义了flash模块的相关操作，可以用来在该存储模块内创建，打开，增加，删除多个文件或目录，并查询相关
 * @n 文件或目录是否存在。
 * @n DFRobot_FlashMoudle_IIC 此类定义了模块的接口驱动，用户只需操作begin函数
 * @n 
 * @copyright	Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author [Arya](xue.peng@dfrobot.com)
 * @version  V1.0
 * @date  2021-11-19
 * @url https://github.com/DFRobot/DFRobot_Flash_Moudle
 */

/*!
 * @file DFRobot_Flash_Moudle.cpp
 * @brief 定义了 DFRobot_File 类的部分，以及DFRobot_FlashMoudle 类的实现。 
 * @details DFRobot_File 类实现了目录的相关操作；
 * @n DFRobot_FlashMoudle 类实现磁盘的相关操作，如打开文件，移除文件，创建目录，判断文件或目录是否存在
 * @n 
 * @copyright	Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author [Arya](xue.peng@dfrobot.com)
 * @version  V1.0
 * @date  2021-11-19
 * @url https://github.com/DFRobot/DFRobot_Flash_Moudle
 */
#include <Arduino.h>
#include "DFRobot_Flash_Moudle.h"
                       
DFRobot_File DFRobot_File::openNextFile(uint8_t mode) {
  char name[13];
  DFRobot_FlashFile f;
  if(_file->readDir(name, sizeof(name)) == 0){
    if(f.open(_file, name, mode)){
      return DFRobot_File(f, name);
    }else{
      return DFRobot_File();
    }
  }
  return DFRobot_File();
}

void DFRobot_File::rewindDirectory(void) {  
  if (isDirectory()){
     _file->rewind();
  }else{
     DBG("is not dir");
  }
}

DFRobot_FlashMoudle::DFRobot_FlashMoudle(){
  //_card = (DFRobot_Flash *)malloc(sizeof(DFRobot_Flash));
}

DFRobot_FlashMoudle::~DFRobot_FlashMoudle(){
  
}

void DFRobot_FlashMoudle::getParentDir(const char *filepath, int *index){
  const char *origpath = filepath;
  while (strchr(filepath, '/')) {
    // get rid of leading /'s
    if (filepath[0] == '/') {
      filepath++;
      continue;
    }
    
    if (!strchr(filepath, '/')) {
      // it was in the root directory, so leave now
      break;
    }

    // extract just the name of the next subdirectory
    uint8_t idx = strchr(filepath, '/') - filepath;
    if (idx > 12)
      idx = 12;    // dont let them specify long names
    filepath += idx;
  }
  *index = (int)(filepath - origpath);
  // parent is now the parent diretory of the file!
}


/*获取卡的基本信息*/
uint8_t DFRobot_FlashMoudle::begin(DFRobot_Driver *drv) {
  return (_card.init(drv) | _root.openRoot(_card));
}
 
//打开文件
DFRobot_File DFRobot_FlashMoudle::open(const char *filepath, uint8_t mode){
  DFRobot_FlashFile file;
  int pathidx;
  char *pathsave = (char *)filepath;
  getParentDir(filepath, &pathidx);
  // no more subdirs!
  
  filepath += pathidx;
  DBG(filepath);
  if (!_root.isOpen()){
    return DFRobot_File();
  }

  if (!filepath[0]) {
    // it was the directory itself!
    return DFRobot_File(_root, "/");
  }

  if (!file.open(_root, pathsave, mode)) {
      // failed to open the file :(
      return DFRobot_File();
  }

 /* if (parentdir.isRoot()) {
    if (!file.open(_root, filepath, mode)) {
      // failed to open the file :(
      return DFRobot_File();
    }
    // dont close the root!
  } else {
    if (!file.open(parentdir, filepath, mode)) {
      return DFRobot_File();
    }
    // close the parent
    parentdir.close();
  }*/
  return DFRobot_File(file, filepath);
}

boolean DFRobot_FlashMoudle::exists(const char *filepath) {
  return _root.exists(filepath);
}

boolean DFRobot_FlashMoudle::mkdir(const char *filepath) {
  /*
    Makes a single directory or a heirarchy of directories.
    A rough equivalent to `mkdir -p`.
   */
  if(exists(filepath)) return true;
  return _root.makeDir(filepath);
}

boolean DFRobot_FlashMoudle::remove(const char *filepath) {
  DFRobot_FlashFile child;
  return _root.remove(filepath);
}

boolean DFRobot_FlashMoudle::rmdir(const char *filepath) {
  return remove(filepath);
}


