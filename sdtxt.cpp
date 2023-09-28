#include "SD.h"
#include "SPI.h"
#include "sdtxt.h"
#include <LittleFS.h>
#include "FS.h"

#include "epd.h"


// static bool fsOK;
#define FORMAT_LITTLEFS_IF_FAILED true

#define SD_CS 5

SPIClass SPI_SD(HSPI);
File ROOT_DIR;


extern void SD_FsInit(void)
{
  bool fsState;
  fsState = LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED);
  if(fsState)
  {
    Serial.printf("LittleFS Init success\n");
  }
  else
  {
    Serial.printf("LittleFS Init FAIL\n");
  }

  ROOT_DIR = SD.open("/");  

  int bookNum = 0;
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
      bookNum ++;
      Serial.printf("txtFile: %s\n", fileName.c_str());
      EPD_LineUpdate(bookNum, fileName);
    }
  }


}

extern void SD_Init(void)
{
  // SD_FsInit();

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

  SD_FsInit();

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

File txtFile;                  // 本次打开的txt文件系统对象
String txtPage[8] = {}; 
int8_t txtLine = 0;
int8_t txtLastLine = 0;
char txtByte;
uint16_t ascCount = 0;
uint16_t chineseCount = 0;
boolean hskgState = 1; 

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


extern void SD_TxtInit(void)
{


  txtFile = SD.open("/book1.txt", FILE_READ);
  // txtFile = LittleFS.open("/book1.txt", "r");
  // txtFile = LittleFS.open("/长夜难明.txt", "r");
  if(txtFile)
  {
    Serial.printf("文本读取成功\n");
    txtFile.seek(0, SeekSet); //索引的数据就是TXT文件的偏移量

  }
  else
  {
    Serial.printf("文本读取失败\n");
  }
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
      // Serial.println("");
      // Serial.print("行"); Serial.print(txtLine); Serial.print(" 预计像素长度:"); Serial.println(StringLength);
      // Serial.print("行"); Serial.print(txtLine); Serial.print(" 实际像素长度:"); Serial.println(u8g2Fonts.getUTF8Width(txtPage[txtLine].c_str()));
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
  Serial.printf("txt位置:%d\n", txtFile.position());
  // txtFile.close();
}

extern void SD_NextPage(void)
{

  txtFile.seek(0, SeekSet); //索引的数据就是TXT文件的偏移量


}

//获取ascii字符的长度
int8_t getCharLength(char zf) 
{
  if (zf == 0x20) return 4;      //空格
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



