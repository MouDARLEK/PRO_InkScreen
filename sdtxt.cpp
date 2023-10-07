#include "SD.h"
#include "SPI.h"
#include "sdtxt.h"
#include <LittleFS.h>
#include "FS.h"

#include "epd.h"
#include "string.h"

#define SD_CS 5
SPIClass SPI_SD(HSPI);

File ROOT_DIR;

//计算显示一页txt的全局变量
File txtFile;                  
String txtPage[8] = {}; 
int8_t txtLine = 0;
int8_t txtLastLine = 0;
char txtByte;
uint16_t ascCount = 0;
uint16_t chineseCount = 0;
boolean hskgState = 1; 

BOOK BOOK_OPEN;
String BOOK_INDEX = "/book.idx";
uint8_t BOOK_NUM = 0;
uint32_t BOOK_RECORD_NUM = 0;
File txtIndexFile; 

String bookList[8] = {};
uint8_t READ_BUFF[30] = {};
uint8_t WRITE_BUFF[30] = {};
uint8_t INDEX_BUFF[1000] = {};

void SD_IndexStore(uint32_t startByte, bool beginStore)
{
   static int storeBytes = 0;

  
  if(beginStore)
  {
    memset(INDEX_BUFF, 0, 1000);
    txtIndexFile = SD.open(BOOK_INDEX, FILE_READ);
    storeBytes = txtIndexFile.size() - startByte;

    if(storeBytes <= 0)
    {
      return;
    }

    txtIndexFile.seek(startByte);
    txtIndexFile.read(INDEX_BUFF, storeBytes);
    
    Serial.printf("从%d读取%d字节\n",startByte, storeBytes);

  }
  else
  {
    if(storeBytes <= 0)
    {
      storeBytes = 0;
      return;
    }
    txtIndexFile = SD.open(BOOK_INDEX, FILE_WRITE);

    txtIndexFile.seek(startByte);
    txtIndexFile.write(INDEX_BUFF, storeBytes);
    Serial.printf("从%d写入%d字节\n",startByte, storeBytes);
    storeBytes = 0;


  }
  txtIndexFile.close();



  
}

extern void SD_IndexRecord(void)
{
  if(BOOK_OPEN.readByte >= BOOK_OPEN.bookSize)
  {
    BOOK_OPEN.readByte = BOOK_OPEN.bookSize;
  }
  SD_IndexStore(BOOK_OPEN.indexByte + 40, true);
  SD_IndexWrite(String(BOOK_OPEN.readByte), BOOK_OPEN.indexByte + 30, 10);
  SD_IndexStore(BOOK_OPEN.indexByte + 40, false);
  Serial.printf("阅读位置%s记录在%d\n",String(BOOK_OPEN.readByte), BOOK_OPEN.indexByte + 30);
}

extern int SD_IndexSearch(uint32_t bookNum, String txtName)
{
  if(bookNum == 0)
  {
    return 0;
  }
  int indexNum = 0;
  int indexByte = 0;
  while(indexNum < bookNum)
  {

    indexNum ++;
    indexByte = indexNum * 40;
    SD_IndexRead(READ_BUFF, indexByte, 30);
    Serial.printf("读取到索引书名:%s\n", READ_BUFF);
    if(strcmp(txtName.c_str(), (char*)READ_BUFF) == 0)
    {
      Serial.printf("成功搜索到该书:%s\n", txtName.c_str());
      return indexByte;
    }
  
  }
  Serial.printf("未搜索到该书:%s\n", txtName.c_str());
  return 0;


}

extern void SD_IndexMatch(String txtName)
{
  if (SD.exists(BOOK_INDEX))
  {
    Serial.println("存在书籍索引...");
    SD_IndexRead(READ_BUFF, 0, 30);
    BOOK_RECORD_NUM = String((char*)READ_BUFF).toInt();
    Serial.printf("索引记录%d本\n", BOOK_RECORD_NUM);
    int indexPosition = SD_IndexSearch(BOOK_RECORD_NUM, txtName);
    if(indexPosition == 0)//没搜到，新加一本
    {
      
      BOOK_RECORD_NUM += 1;
      BOOK_OPEN.indexByte = BOOK_RECORD_NUM * 40;
      Serial.printf("没搜到该书,记录在索引%d字节处\n", BOOK_OPEN.indexByte);
      BOOK_OPEN.readByte = 0;
      
      // SD_IndexWrite(String(BOOK_RECORD_NUM), 0, 30);
      SD_IndexWrite(txtName, BOOK_OPEN.indexByte, 30);
      SD_IndexWrite(String(0), BOOK_OPEN.indexByte + 30, 10);

      SD_IndexStore(40, true);
      SD_IndexWrite(String(BOOK_RECORD_NUM), 0, 30);
      SD_IndexStore(40, false);

    }
    else
    {
      BOOK_OPEN.indexByte = indexPosition;

      SD_IndexRead(READ_BUFF, BOOK_OPEN.indexByte + 30, 10);
      BOOK_OPEN.readByte = String((char*)READ_BUFF).toInt();
      Serial.printf("该书已阅读%llu\n", BOOK_OPEN.readByte);
      // SD_IndexWrite(String(BOOK_RECORD_NUM), 0, 30);

    }
  }
  else
  {
    Serial.println("索引不存在，创建...");
    BOOK_RECORD_NUM = 1;

    SD_IndexWrite(String(BOOK_RECORD_NUM), 0, 30);
    Serial.printf("索引记录 %s 本\n", WRITE_BUFF);
  
    BOOK_OPEN.indexByte = BOOK_RECORD_NUM * 40;
    BOOK_OPEN.readByte = 0;
    SD_IndexWrite(txtName, BOOK_OPEN.indexByte, 30);
    SD_IndexWrite(String(0), BOOK_OPEN.indexByte + 30, 10);

  }

}


extern void SD_IndexRead(uint8_t* readBuff, uint64_t seekByte, uint32_t readBytes)
{
  memset(readBuff, 0, 30);
  txtIndexFile = SD.open(BOOK_INDEX, FILE_READ);
  txtIndexFile.seek(seekByte);
  txtIndexFile.read(readBuff, readBytes);
  txtIndexFile.close();
}

extern void SD_IndexWrite(String dataStr, uint64_t seekByte, uint32_t writeBytes)
{
  memset(WRITE_BUFF, 0, 30);
  snprintf((char*)WRITE_BUFF, dataStr.length() + 1, "%s", dataStr.c_str());

  txtIndexFile = SD.open(BOOK_INDEX, FILE_WRITE);
  txtIndexFile.seek(seekByte);
  txtIndexFile.write(WRITE_BUFF, writeBytes);
  // txtIndexFile.print(WRITE_BUFF);
  
  txtIndexFile.close();
}

 
bool BOOK::BOOK_Init(String txtName)
{

  this->bookName = txtName;
  Serial.printf("导入书：");
  Serial.print(this->bookName);
  txtFile = SD.open(this->bookName, FILE_READ);
  if(txtFile)
  {
    Serial.printf("文本读取成功\n");
    txtFile.seek(BOOK_OPEN.readByte, SeekSet); 
    this->bookSize = txtFile.size();
    return true;
  }
  else
  {
    Serial.printf("文本读取失败\n");
    return false;
  }

}

extern uint8_t SD_FsInit(void)
{

  ROOT_DIR = SD.open("/");  

  int bookNum = 1;
  EPD_Refresh();
  EPD_LineAdd(bookNum, "检索到的txt文本:");
  while (true)               
  {
    File rootFile = ROOT_DIR.openNextFile();
    String fileName = rootFile.name(); //文件名
    if(!rootFile)
    {
      break;
    }

    if (fileName.endsWith(".txt")) // 检测TXT文件
    {
      bookList[bookNum - 1] = "/" + fileName;
      bookNum ++;
      Serial.printf("txtFile: %s\n", fileName.c_str());
      EPD_LineAdd(bookNum, fileName);

    }
    else
    {
      Serial.printf("otherFile: %s\n", fileName.c_str());
    }
  }
  EPD_BufferSend();
  BOOK_NUM = bookNum - 1;
  return BOOK_NUM;
  // SD_IndexMatch("/poem.txt");

}

extern void SD_SelectBook(int txtNum)
{
  BOOK_OPEN.bookName = bookList[txtNum];
}

extern void SD_Init(void)
{
  SD.end();
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  delay(1);
  SPI_SD.begin(14, 12, 13, 15);

  if(SD.begin(SD_CS, SPI_SD, 40000000))
  {
    Serial.println("SD卡挂载成功");
    Serial.printf("SD内存/类型:%lld %d\r\n", SD.cardSize(), SD.cardType());
  }
  else
  {
    Serial.printf("SD卡挂载失败%d\r\n", SD.begin(SD_CS, SPI_SD, 40000000));
  }


}


extern void SD_Test(void)
{

  File testFile = SD.open("/poem.txt", FILE_READ);
  if (testFile)
  {
    Serial.println("测试txt文件:");
    while (testFile.available()) Serial.write(testFile.read());
    Serial.println("");
    testFile.close();
  }
  else
  {
    Serial.println("文件打开失败");
  }

}


extern void SD_PageClear(void)
{
  int i = 0;
  while(i <= 7)
  {
    txtPage[i].clear();
    i ++;
  }
}

extern void SD_Clear(void)
{
  txtLine = 0;
  txtByte = 0;
  ascCount = 0;
  chineseCount = 0;
  hskgState = 1;
  SD_PageClear();

}


// extern void SD_TxtInit(void)
// {
//   SD_IndexMatch("/book1.txt");
//   BOOK_OPEN.BOOK_Init("/book1.txt");
// }

extern void SD_TxtInit(void)
{
  SD_IndexMatch(BOOK_OPEN.bookName);
  BOOK_OPEN.BOOK_Init(BOOK_OPEN.bookName);
}

//TXT刷新一页程序 引自甘草
extern void SD_GetOnePage(void)
{
  SD_Clear();
  while (txtLine < 8)
  {
    if (txtLastLine != txtLine) //行首4个空格检测状态重置
    {
      txtLastLine = txtLine;
      hskgState = 1;
      
    }

    txtByte = txtFile.read();
    while (txtByte == '\n' && txtLine <= 7) 
    {
      if (txtLine == 0) //等于首行，并且首行不为空，才插入换行
      {
        if (txtPage[txtLine].length() > 0) txtLine++; //换行
        else txtPage[txtLine].clear();
      }
      else 
      {
        //连续空白的换行合并成一个
        if (txtPage[txtLine].length() > 0) txtLine++;
        else if (txtPage[txtLine].length() == 0 && txtPage[txtLine - 1].length() > 0) txtLine++;
        /*else if (txt[line].length() == 1 && txt[line - 1].length() == 1) hh = 0;*/
      }

      if (txtLine <= 7) txtByte = txtFile.read();
      ascCount = 0;
      chineseCount = 0;
    }

    if (txtByte == '\t') //检查水平制表符 tab
    {
      if (txtPage[txtLine].length() == 0) txtPage[txtLine] += "    "; //行首的一个水平制表符 替换成4个空格
      else txtPage[txtLine] += "       ";//非行首的一个水平制表符 替换成7个空格
    }
    else if ((txtByte >= 0 && txtByte <= 31) || txtByte == 127) //检查没有实际显示功能的字符
    {
      //ESP.wdtFeed();  // 喂狗
    }
    else txtPage[txtLine] += txtByte;
    boolean ascState = 0;
    byte a = B11100000;
    byte b = txtByte & a;
    if (b == B11100000) //中文等 3个字节
    {
      chineseCount ++;
      txtByte = txtFile.read();
      txtPage[txtLine] += txtByte;
      txtByte = txtFile.read();
      txtPage[txtLine] += txtByte;
    }
    else if (b == B11000000) //ascii扩展 2个字节
    {
      ascCount += 14;
      txtByte = txtFile.read();
      txtPage[txtLine] += txtByte;
    }
    else if (txtByte == '\t') //水平制表符，代替两个中文位置，14*2
    {
      if (txtPage[txtLine] == "    ") ascCount += 20; //行首，因为后面会检测4个空格再加8所以这里是20
      else ascCount += 28; //非行首
    }
    else if (txtByte >= 0 && txtByte <= 255)
    {
      ascCount += getCharLength(txtByte) + 1;
      ascState = 1;
    }
    uint16_t StringLength = ascCount + (chineseCount  * 14);
    if (StringLength >= 260 && hskgState) //检测到行首的4个空格预计的长度再加长一点
    {
      if (txtPage[txtLine][0] == ' ' && txtPage[txtLine][1] == ' ' &&
          txtPage[txtLine][2] == ' ' && txtPage[txtLine][3] == ' ') 
      {
        ascCount += 8;
      }
      hskgState = 0;
    }
    if (StringLength >= 272) //检查是否已填满屏幕 283
    {
      if (ascState == 0) //最后一个字符是中文，直接换行
      {
        txtLine++;
        ascCount = 0;
        chineseCount = 0;
      }
      else if (StringLength >= 275) //286 最后一个字符不是中文，在继续检测
      {
        char t = txtFile.read();
        txtFile.seek(-1, SeekCur); //往回移
        int8_t cz =  266 - StringLength;//294
        int8_t t_length = getCharLength(t);
        byte a = B11100000;
        byte b = t & a;
        if (b == B11100000 || b == B11000000) //中文 ascii扩展
        {
          txtLine++;
          ascCount = 0;
          chineseCount = 0;
        }
        else if (t_length > cz)
        {
          txtLine++;
          ascCount = 0;
          chineseCount = 0;
        }
      }
    }
  }
  EPD_TxtOnePage(txtPage); //显示一页txt
  BOOK_OPEN.readByte = txtFile.position();
  SD_IndexRecord();
  Serial.printf("txt位置:%d\n", BOOK_OPEN.readByte);
  // txtFile.close();
   
}

extern void SD_SeekPreviousPage(void)
{
  uint64_t lastPageByte = 0;
  lastPageByte = BOOK_OPEN.readByte - 800;
  if(BOOK_OPEN.readByte <= 800)
  {
    lastPageByte = 0;
  }
  txtFile.seek(lastPageByte); 
  Serial.printf("翻页上移:%d\n", lastPageByte);



}

float SD_GetReadProgerss(void)
{

  uint64_t readByte = BOOK_OPEN.readByte;
  uint64_t allByte = (uint64_t)BOOK_OPEN.bookSize; 

  
  if(allByte <= 0)
  {
    Serial.printf("阅读进度wrong\n");
    return 0;
  }
  uint32_t readCount = ((float)(readByte*1000 / allByte));
  
  if(readCount >= 1000)
  {
    return 100;
  }
  float readProgress = (float)readCount / 10.0; 
  Serial.printf("阅读%llu %llu %f\n",readByte, (uint64_t)allByte, readProgress);
  return readProgress;
}

//获取ascii字符的长度
int8_t getCharLength(char zf) 
{
  if (zf == 0x20) return 4;      
  else if (zf == '!') return 4;
  else if (zf == '"') return 5;
  else if (zf == '#') return 5;
  else if (zf == '$') return 6;
  else if (zf == '%') return 7;
  else if (zf == '&') return 7;
  else if (zf == '\'') return 3;
  else if (zf == '(') return 5;
  else if (zf == ')') return 5;
  else if (zf == '*') return 7;
  else if (zf == '+') return 7;
  else if (zf == ',') return 3;
  else if (zf == '.') return 3;

  else if (zf == '1') return 5;
  else if (zf == ':') return 4;
  else if (zf == ';') return 4;
  else if (zf == '@') return 9;

  else if (zf == 'A') return 8;
  else if (zf == 'D') return 7;
  else if (zf == 'G') return 7;
  else if (zf == 'H') return 7;
  else if (zf == 'I') return 3;
  else if (zf == 'J') return 3;
  else if (zf == 'M') return 8;
  else if (zf == 'N') return 7;
  else if (zf == 'O') return 7;
  else if (zf == 'Q') return 7;
  else if (zf == 'T') return 7;
  else if (zf == 'U') return 7;
  else if (zf == 'V') return 7;
  else if (zf == 'W') return 11;
  else if (zf == 'X') return 7;
  else if (zf == 'Y') return 7;
  else if (zf == 'Z') return 7;

  else if (zf == '[') return 5;
  else if (zf == ']') return 5;
  else if (zf == '`') return 5;

  else if (zf == 'c') return 5;
  else if (zf == 'f') return 5;
  else if (zf == 'i') return 1;
  else if (zf == 'j') return 2;
  else if (zf == 'k') return 5;
  else if (zf == 'l') return 2;
  else if (zf == 'm') return 9;
  else if (zf == 'o') return 7;
  else if (zf == 'r') return 4;
  else if (zf == 's') return 5;
  else if (zf == 't') return 4;
  else if (zf == 'v') return 7;
  else if (zf == 'w') return 9;
  else if (zf == 'x') return 5;
  else if (zf == 'y') return 7;
  else if (zf == 'z') return 5;

  else if (zf == '{') return 5;
  else if (zf == '|') return 4;
  else if (zf == '}') return 5;

  else if ((zf >= 0 && zf <= 31) || zf == 127) return -1; //没有实际显示功能的字符

  else return 6;
}





