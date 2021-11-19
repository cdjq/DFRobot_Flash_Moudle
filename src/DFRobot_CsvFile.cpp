#include <math.h>
#include "DFRobot_CsvFile.h"

#define ACHE_BUFFER_LEN    32
static char _ache[32];

DFRobot_CsvFile::DFRobot_CsvFile()
  :_file(NULL), _initialize(false),_rowNum(0), _columnNum(0){
   
}
DFRobot_CsvFile::DFRobot_CsvFile(DFRobot_File *file)
  :_file(file), _initialize(false),_rowNum(0), _columnNum(0){
}

DFRobot_CsvFile::~DFRobot_CsvFile(){
  //_file = NULL;
  _initialize = false;
  //if(_ache) free(_ache);
  //_ache = NULL;
}

uint8_t DFRobot_CsvFile::begin(DFRobot_File *file){
  _initialize = false;
  _file = file;
  //if(!_ache) _ache = (char *)malloc(ACHE_BUFFER_LEN);
  if(!_file ){
    DBG("csv _file is null.");
    return 1;
  }
  if(!(*_file)){
    CSV_DBG("file: ", _file->name(), "is not open.");
    return 2;
  }
  if(_file->isDirectory()){
    CSV_DBG("file: ", _file->name(), "is not csv type, it is a directory.");
    return 3;
  }
  _columnNum = 0;
  _rowNum = 0;
  _initialize = true;
    //计算文件的行和列
  uint32_t pos = _file->position();//保留文件初始位置
  uint32_t total = _file->size(); //获取文件大小
  int colIndex = 0, colDataLen = 0, rowIndex = 0, rowDataLen = 0;
  bool flag = false;
  if(!_file->seek(0)){
    CSV_DBG("rewind file start position failed.");
    return 4;
  }
  Serial.print("pos=");Serial.println(pos);
  Serial.print("total=");Serial.println(total);
  memset(_ache, 0, ACHE_BUFFER_LEN);
  while(total){
    int remain = total > (ACHE_BUFFER_LEN - 1) ? (ACHE_BUFFER_LEN - 1) : total;
    total -= remain;
    Serial.print("remain=");Serial.println(remain);
    if(_file->read(_ache, remain) == remain){
      char *pc = _ache;
      while(remain--){
        rowDataLen += 1;
        if((int)*pc == -1){
          colIndex += 1;
          _columnNum = _columnNum > colIndex ? _columnNum : colIndex;
          _rowNum += 1;
          rowDataLen = 0;
          return 0;
        }
        if(*pc == ','){
          colIndex += 1;
        }
        if(*pc == '\r') flag = true;
        if((*pc == '\n') && flag){
          colIndex += 1;
          //Serial.print("colIndex=");Serial.println(colIndex);
          //Serial.print("_columnNum=");Serial.println(_columnNum);
          _columnNum = _columnNum > colIndex ? _columnNum : colIndex;
          //Serial.print("_columnNum=");Serial.println(_columnNum);
          _rowNum += 1;
          //Serial.print("_rowNum=");Serial.println(_rowNum);
          colIndex = 0;
          rowDataLen = 0;
        }
        if(flag && (*pc != '\r')) {
          flag = false;
        }
        pc ++;
        yield();
      }

    }else{
      CSV_DBG("read data length failed.");

      return 5;
    }
    yield();
  }
  if(rowDataLen > 0){
    colIndex += 1;
    _columnNum = _columnNum > colIndex ? _columnNum : colIndex;
    _rowNum += 1;
  }
  CSV_DBG("file: ", _file->name(), "is not csv format type.");
  return 0;
  
  
}

int DFRobot_CsvFile::getRow(){
  return _rowNum;
}
int DFRobot_CsvFile::getColumn(){
  return _columnNum;
}

void DFRobot_CsvFile::getRowsAndColumns(int *maxRow, int *maxCol){
  *maxRow = _rowNum;
  *maxCol = _columnNum;
}

String DFRobot_CsvFile::readRow(int row){
  String rslt = "";
  if(row < (int)eSmallestRow|| row > _rowNum) return rslt;
  int rowi = 1;
  uint32_t total = 0;
  bool flag = false;
  _file->seek(0);
  total = _file->available();
  while(total){
    int remain = total > ACHE_BUFFER_LEN ? ACHE_BUFFER_LEN : total;
    total -= remain;
    if(_file->read(_ache, remain) == remain){
      char *pc = _ache;
      while(remain--){
        if(rowi > row){
          return rslt; //返回每行数据
        }else if(row == rowi){
          rslt += *pc;
        }
        if(*pc == '\r'){
          flag = true;
        }else if((*pc == '\n') && flag){
          rowi += 1; 
        }else{
          flag = false;
        }
        pc++;
        yield();
      }
    }
    yield();
  }
  return rslt;
}

bool DFRobot_CsvFile::writeRow(int row, const char *rowData){
  if(row < (int)eSmallestRow|| row > (int)eLargestRow) return false;

  uint32_t size = _file->size();
  if(row > _rowNum){
    //判断第_rowNum行，末尾有没有\r\n,如果没有请给最后一行添加/r/n
    if(size >= 2){//可能会有
        _file->seek(size - 2);
        if(_file->read(_ache, 2) == 2){
          if((_ache[0] != '\r') || ((_ache[1] != '\n'))){
            println();
          }
        }
    }else if(size == 1){
      _file->seek(size);
      println();
    }else{
      _file->seek(size);
    }
    _rowNum += 1;
    //新增
    while(1){
      if(_rowNum == row){
        println(rowData);
        return true;
      }else{
        println();
        _rowNum += 1;
      }
    }
  }else{
    //替换
     uint32_t posStart = 0, posEnd = 0, remain = size, total = size;
     int rowi = 1;
     bool flag = false;
     _file->seek(0);
     posStart = _file->position();//记录当前位置
     while(total){
       remain = (uint8_t)(total > ACHE_BUFFER_LEN ? ACHE_BUFFER_LEN : total);
       posStart = _file->position();
       posEnd = posStart;
       total -= remain;
       if(_file->read(_ache, remain) == remain){
         char *pc = _ache;
         while(remain--){
            posEnd += 1;
            if(*pc == '\r'){
              flag = true;
            }else if((*pc == '\n') && (flag == true)){
              if(rowi == row){
                if((strlen(rowData) + 2) == (posEnd - posStart)){
                  _file->seek(posStart);
                  println(rowData);
                  return true;
                }else if((strlen(rowData) + 2) < (posEnd - posStart)){
                  _file->seek(posStart);
                  println(rowData);
                  _file->del(posEnd, (posEnd - posStart) - (strlen(rowData) + 2), true);
                  return true;
                }else{
                  _file->insert(posEnd, (uint8_t)' ', (strlen(rowData) + 2) - (posEnd - posStart));
                  _file->seek(posStart);
                  println(rowData);
                  return true;
                }

              }
              rowi += 1;
  
            }else{
              flag = false;
            }
            pc++;
         }
       }else return false;   
     }
     //跑到这里说明是最后一行，且最后一行没有\r\n
     if(rowi == row){
         if((strlen(rowData) + 2) == (posEnd - posStart)){
           _file->seek(posStart);
           println(rowData);
           return true;
         }else if((strlen(rowData) + 2) < (posEnd - posStart)){
           _file->seek(posStart);
           println(rowData);
           _file->del(posEnd, (posEnd - posStart) - (strlen(rowData) + 2), true);
           return true;
         }else{
           _file->insert(posEnd, (uint8_t)' ', (strlen(rowData) + 2) - (posEnd - posStart));
           _file->seek(posStart);
           println(rowData);
           return true;
         }

     }
  }
  return false;
}

bool DFRobot_CsvFile::deleteRow(int row){
  if((row > _rowNum) || (row < (int)eSmallestRow)) return false;
  uint32_t size = _file->size();
  uint32_t posStart = 0, posEnd = 0, remain = size, total = size;
  int rowi = 1;
  bool flag = false;
  _file->seek(0);
  posStart = _file->position();//记录当前位置
  while(total){
    remain = (uint8_t)(total > ACHE_BUFFER_LEN ? ACHE_BUFFER_LEN : total);
    posStart = _file->position();
    posEnd = posStart;
    total -= remain;
    if(_file->read(_ache, remain) == remain){
      char *pc = _ache;
      while(remain--){
        posEnd += 1;
        if(*pc == '\r'){
          flag = true;
        }else if((*pc == '\n') && (flag == true)){
          if(rowi == row){
            _file->del(posEnd, (posEnd - posStart) , true);
            return true; 
          }else{
              flag = false;
          }
          rowi += 1;
        }else{
          flag = false;
        }
        pc++;
      }
    }else break;
    
  }
  if((rowi == row) && (posEnd - posStart > 0)){
    _file->del(posEnd, (posEnd - posStart) , true);
    return true; 
  }
  return false;
}

String DFRobot_CsvFile::readColumn(int col){
  if(col < (int)eSmallestColumn || col > _columnNum) return "";
  String str = "";
  uint32_t size = _file->size();
  uint32_t posStart = 0, posEnd = 0, remain = size, total = size;
  int rowi = 1;
  int icol = 1;
  int rowindex = 0;
  bool flag = false;
  _file->seek(0);
  posStart = _file->position();//记录当前位置
  while(total){
    remain = (uint8_t)(total > ACHE_BUFFER_LEN ? ACHE_BUFFER_LEN : total);
    total -= remain;
    if(_file->read(_ache, remain) == remain){
      char *pc = _ache;
      while(remain--){
        posEnd += 1;
        if(*pc == ','){
          icol += 1;
        }
        if(*pc == '\r'){
          flag = true;
        }else if((*pc == '\n') && flag){
          flag = false;
          str += "\r\n";
          icol = 1;
          if(total != 0) rowi += 1;
          rowindex += 1;
          continue;
        }else{
          flag = false;
        }
        if((icol == col) && (*pc != ',') && (*pc != '\r') && (*pc != '\n')){
          str += *pc;
        }
      }
    }else return "";
  }
  if(rowindex < rowi){
    str += "\r\n";
  }
  return str;
}

bool DFRobot_CsvFile::writeColumn(int col, const char *colData){
  //按顺序写列
  if(col < (int)eSmallestColumn || col > (int)eLargestColumn || (strlen(colData) == 0)) return false;
  int icol = 1;
  char *ps = (char *)colData;
  char *pe = ps;
  uint32_t posStart = 0, posEnd = 0;
  uint32_t total = strlen(colData);
  bool flag = false;

  _file->seek(0);


  for(uint32_t i =  0; i < total; i++){
    if((*pe == '\r') &&((i + 1 >= total) || (*(pe+1) != '\n'))) return false;
    if(*pe == '\r'){
      //某行某列
LOOP:
      char cv = _file->peek();
      posStart = _file->position();
      posEnd = posStart;
      switch(cv){
        case ',':
              if(icol == col){
                posEnd = _file->position();
                if((uint32_t)(pe - ps) == posEnd - posStart){
                  _file->seek(posStart);
                  _file->write(ps,(uint32_t)(pe - ps));
                  
                }else if((uint32_t)(pe - ps) < posEnd - posStart){
                  _file->seek(posStart);
                  _file->write(ps, (uint32_t)(pe - ps));
                  _file->del(posEnd, (posEnd - posStart) - (uint32_t)(pe - ps), true);
                }else{
                  _file->insert(posEnd, (uint8_t)' ' , (uint32_t)(pe - ps) - (posEnd - posStart));
                  _file->seek(posStart);
                  _file->write(ps, (uint32_t)(pe - ps));
                }
              }
              _file->read();
              icol += 1;
              posStart = _file->position();
              posEnd = posStart;
              flag = false;
              break;
        case '\r':
        {
          posEnd = _file->position();
          flag = true;
          _file->read();
          break;
        }
        case '\n':
        {
          if(flag == true){
            //
            if(icol < col){
              _file->insert(posEnd, (uint8_t)',' , col-icol);
              icol = col;
              posStart = _file->position();
              
            }
            
            if(icol == col){
              posEnd = _file->position();
              if((uint32_t)(pe - ps) == posEnd - posStart){
                  _file->seek(posStart);
                  _file->write(ps, (uint32_t)(pe - ps));
              }else if((uint32_t)(pe - ps) < posEnd - posStart){
                  _file->seek(posStart);
                  _file->write(ps, (uint32_t)(pe - ps));
                  _file->del(posEnd, (posEnd - posStart) - (uint32_t)(pe - ps), true);
              }else{
                  _file->insert(posEnd, (uint8_t)' ' , (uint32_t)(pe - ps) - (posEnd - posStart));
                  _file->seek(posStart);
                  _file->write(ps, (uint32_t)(pe - ps));
              }
              _file->read();
            }
            icol = 1;
            posStart = _file->position() + 1;
            posEnd = posStart;
          }
          flag = false;
          _file->read();
          goto ENDLOOP;
        }
        case 255:
        {
          //
          if(icol < col){
            posEnd = _file->position();
            _file->insert(posEnd, (uint8_t)',' , col-icol);
            icol = col;
            posStart = _file->position();
          }
          if(icol == col){
              posEnd = _file->position();
              if((uint32_t)(pe - ps) == posEnd - posStart){
                  _file->seek(posStart);
                  _file->write(ps, (uint32_t)(pe - ps));
              }else if((uint32_t)(pe - ps) < posEnd - posStart){
                  _file->seek(posStart);
                  _file->write(ps, (uint32_t)(pe - ps));
                  _file->del(posEnd, (posEnd - posStart) - (uint32_t)(pe - ps), true);
              }else{
                  _file->insert(posEnd, (uint8_t)' ' , (uint32_t)(pe - ps) - (posEnd - posStart));
                  _file->seek(posStart);
                  _file->write(ps, (uint32_t)(pe - ps));
              }
              println();
          }
          goto ENDLOOP;

        }
        default:
          _file->read();
          break;

      }
      goto LOOP;
    }
  ENDLOOP:
    ps = pe + 2;
    pe = ps;
  }
  if(ps != pe){
    _file->seek(posStart);
    _file->write(ps, (uint32_t)(pe - ps));
    println();
  }

  return true;
}

bool DFRobot_CsvFile::deleteColumn(int col){
  if(col < (int)eSmallestColumn || col > _columnNum) return false;
  uint32_t size = _file->size();
  uint32_t posStart = 0, posEnd = 0, remain = size, total = size;
  int rowi = 1;
  int icol = 1;
  int rowindex = 0;
  bool flag = false;
  _file->seek(0);
  posStart = _file->position();//记录当前位置
  posEnd = posStart;
  for(int i = 1; i < _rowNum + 1; ){
    char cv = _file->read();
    if((int)cv == -1){
      if(icol == col){
        posEnd = _file->position();
        _file->del(posEnd, posEnd - posStart, true);
        i += 1;
        icol = 1;
        if(i != _rowNum + 1) return false;
      }
    }
    if(cv == ','){
      if(icol == col){
        posEnd = _file->position();
        _file->del(posEnd, posEnd - posStart, true);
      }
      icol += 1;
    }
    if(cv == '\r'){
      flag = true;
    }
    if((cv == '\n') && (flag == true)){
      if(icol == col){
        posEnd = _file->position() - 2;
        if(posEnd - posStart){
          _file->del(posEnd, posEnd - posStart, true);
          _file->read(_ache, 2);
        }
      }
      icol = 1;
      i += 1;
    }
    if(flag && (cv != '\r')){
      flag = false;
    }
  }
  return true;
}

String DFRobot_CsvFile::readItem(int row, int col){
  if((row < (int)eSmallestRow) || (row > _rowNum)){
    CSV_DBG("row out of range.");
    return "";
  }
  if((col < (int)eSmallestColumn) || (col > _columnNum)){
    CSV_DBG("col out of range.");
    return "";
  }
  String str = "";
  uint32_t total = _file->size();
  uint16_t remain;
  int icol = 1;
  bool flag = false;
  for(int irow = 1; irow < row + 1;){
    remain = (uint16_t)(total > ACHE_BUFFER_LEN ? ACHE_BUFFER_LEN : total);
    if(_file->read(_ache, remain) != remain){
      CSV_DBG("read error.");
      return "";
    }
    char *pc = _ache;
    while(remain--){
      if(*pc == ',') icol += 1;
      if(*pc == '\r') flag = true;
      if((flag = true) && (*pc == '\n')) {
        if((irow == row) && (icol == col)){
          return str.substring(0,str.length()-1);
        }
        icol = 1;
        irow += 1;
      }
      if((flag = true) && (*pc != '\r')) flag = false;
      if((irow == row) && (icol == col)){
        str += *pc;
      }
      if((irow == row) && (icol > col)) return str;
      pc ++;
    }
  }
  return str;
  
}

bool DFRobot_CsvFile::writeItem(int row, int col, const char *item){
  if(row < (int)eSmallestRow || row > (int)eLargestRow){
    CSV_DBG("row out of range(1,1000).");
    return false;
  }
  if(col < (int)eSmallestColumn || col > (int)eLargestColumn){
    CSV_DBG("col out of range.(1,26)");
    return false;
  }
  String str = "";
  uint32_t total = _file->size();
  uint16_t remain;
  int icol = 1;
  bool flag = false;
  if(row > _rowNum){
    //判断第_rowNum行，末尾有没有\r\n,如果没有请给最后一行添加/r/n
    if(total >= 2){//可能会有
        _file->seek(total - 2);
        if(_file->read(_ache, 2) == 2){
          if((_ache[0] != '\r') || ((_ache[1] != '\n'))){
            println();
          }
        }
    }else if(total == 1){
      _file->seek(total);
      println();
    }else{
      _file->seek(total);
    }
    _rowNum += 1;
    while(1){
      if(_rowNum == row){
        if(col > 1){
          _file->insert(_file->position(), (uint8_t)',', col - 1);
        }
        println(item);
        return true;
      }else{
        println();
        _rowNum += 1;
      }
    }
  }else{
    uint32_t posStart = 0, posEnd = 0;
    _file->seek(0);
    posStart = _file->position();
    posEnd = posStart;
    int icol = 1;
    for(int irow = 1; irow < row + 1;){
      char pc = _file->read();
      if((int)pc == -1){
        posEnd = _file->position();
        if(irow != row) return false;
        if(icol < col){
          _file->insert(posEnd, (uint8_t)',', col - icol);
          icol = col;
        }
        if(icol == col){
          if((posEnd - posStart) > (strlen(item) + 2)){
            _file->del(posEnd, (posEnd - posStart) - (strlen(item) + 2), true);
            _file->seek(posStart);
          }
          println(item);
          return true;
        } 
        break;
      }
      if(pc == ','){
          if((irow == row) && (icol == col)){
            posEnd = _file->position() - 1;
            if((posEnd - posStart) > strlen(item)){
                _file->del(posEnd, (posEnd - posStart) - strlen(item), true);
                _file->seek(posStart);
            }else if((posEnd - posStart) < strlen(item)){
              _file->insert(posEnd, (uint8_t)' ', strlen(item) - (posEnd - posStart));
            }
            print(item);
            return true;
          }
          icol += 1;
          posStart = _file->position();
      }
        if(pc == '\r') flag = true;
        if((flag = true) && (pc == '\n')) {
          if((irow == row) && (icol == col)){
            posEnd = _file->position() - 2;
            if((posEnd - posStart) > strlen(item)){
                _file->del(posEnd, (posEnd - posStart) - strlen(item), true);
                _file->seek(posStart);
            }else if((posEnd - posStart) < strlen(item)){
              _file->insert(posEnd, (uint8_t)' ', strlen(item) - (posEnd - posStart));
            }
            print(item);
            return true;
          }
          icol = 1;
          irow += 1;
        }
        if((flag = true) && (pc != '\r')) flag = false;
        posStart = _file->position();
      }
    }
  return false;
}

bool DFRobot_CsvFile::deleteItem(int row, int col){
  if((row < (int)eSmallestRow) || (row > _rowNum)){
    CSV_DBG("row out of range.");
    return false;
  }
  if((col < (int)eSmallestColumn) || (col > _columnNum)){
    CSV_DBG("col out of range.");
    return false;
  }
  String str = "";
  uint32_t total = _file->size();
  uint16_t remain;
  int icol = 1;
  bool flag = false;
  uint32_t posStart = 0, posEnd = 0;
  _file->seek(0);
  posStart = _file->position();
  posEnd = posStart;
  for(int irow = 1; irow < row + 1;){
    char pc = _file->read();
    if((int)pc == -1){
      if(irow != row) return false;
      if(icol == col){
        posEnd = _file->position();
        if(posEnd - posStart) _file->del(posEnd, posEnd - posStart, true);
      } 
      return true;
    }
    if(pc == ','){
      if((irow == row) && (icol == col)){
        posEnd = _file->position() - 1;
        if(posEnd - posStart) _file->del(posEnd, posEnd - posStart, true);
        return true;
      }
      icol += 1;
      posStart = _file->position();
    }
    if(pc == '\r') flag = true;
    if((flag = true) && (pc == '\n')) {
      if((irow == row) && (icol == col)){
        posEnd = _file->position() - 2;
        if(posEnd - posStart) _file->del(posEnd, posEnd - posStart, true);
        return true;
      }
      icol = 1;
      irow += 1;
    }
    if((flag = true) && (pc != '\r')) flag = false;
  }
  return false;
}


size_t DFRobot_CsvFile::write(uint8_t val)
{
  return write(&val, 1);
}

size_t DFRobot_CsvFile::write(const uint8_t *buf, size_t size)
{
  if(!_initialize){
    CSV_DBG("_initialize is false.");
    return 0;
  }
  if(!buf || (size == 0)){
    CSV_DBG("buf is null or size is error, (buf, size): ");
    return 0;
  }
  
  char *pc = (char *)buf;
  //DBG(pc);
  //Serial.print("size=%d");
  //Serial.println(size);
  //for(int i = 0; i < size; i++){
  //  Serial.print(buf[i]);
  //  Serial.print(", ");
  //}
  //Serial.println();
  uint32_t pos = _file->position();
  if((size == 2) && (String(pc) == "\r\n")){//查看前一个数据是否为逗号
    //Serial.println("ok");
    if(pos > 0){
      _file->seek(pos - 1);
      if(_file->peek() != ',') _file->seek(pos);
    }
    _file->write(buf, size);
  }else{
    _file->write(buf, size);
    _file->write((uint8_t)',');
  }
  return size;
}

DFRobot_CsvFile::operator bool() {
  if(!(*_file)){
    return false;
  }
  return true;
}


size_t DFRobot_CsvFile::printNumber(unsigned long, uint8_t){
  Serial.println("++++++++++++++++++++++++++1");
  return 0;
  
}
size_t DFRobot_CsvFile::printFloat(double, uint8_t){
  Serial.println("++++++++++++++++++++++++++");
  return 0;
}

size_t DFRobot_CsvFile::print(long n, int base)
{
  int t = 0;
  char buf[8 * sizeof(long) + 1]; // Assumes 8-bit chars plus zero byte.
  char *str = &buf[sizeof(buf) - 1];
  *str = '\0';
  // prevent crash if called with base == 1
  if (base < 2) base = 10;
  if(n < 0){
      //_flag = 0;
      print('-');
      n = -n;
      t += 1;
  }
  do {
    char c = n % base;
    n /= base;

    *--str = c < 10 ? c + '0' : c + 'A' - 10;
  } while(n);
  return t + write(str);
}
size_t DFRobot_CsvFile::println(long n, int base){
  size_t t = print(n, base);
  t += Print::println();
  return t;
}
size_t DFRobot_CsvFile::print(double number, int digits){
  size_t n = 0;
  if (isnan(number)) return Print::print("nan");
  if (isinf(number)) return Print::print("inf");
  if (number > 4294967040.0) return Print::print ("ovf");  // constant determined empirically
  if (number <-4294967040.0) return Print::print ("ovf");  // constant determined empirically
  String str = "";
  if (number < 0.0)
  {
     str += '-';
     n += 1;
     number = -number;
  }
  double rounding = 0.5;
  for (uint8_t i=0; i<digits; ++i)
    rounding /= 10.0;
  number += rounding;
  unsigned long int_part = (unsigned long)number;
  double remainder = number - (double)int_part;
  str += int_part;
  // Print the decimal point, but only if there are digits beyond
  if (digits > 0){
    str += '.';
  }
  while (digits-- > 0)
  {
    remainder *= 10.0;
    unsigned int toPrint = (unsigned int)(remainder);
    str += toPrint;
    remainder -= toPrint; 
  } 
  Print::print(str);
  return n;
}
size_t DFRobot_CsvFile::println(double num, int digits){
  size_t n = print(num, digits);
  n += Print::println();
  return n;
}