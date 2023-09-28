#include "epd.h"
#include "board.h"
#include "log.h"

//墨水屏驱动类 黑白局刷+速刷
#define ENABLE_GxEPD2_GFX 0
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>

#define GxEPD2_DISPLAY_CLASS GxEPD2_BW
#define GxEPD2_DRIVER_CLASS GxEPD2_260

#define MAX_DISPLAY_BUFFER_SIZE 65536ul
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))

GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display(GxEPD2_DRIVER_CLASS(/*CS=*/ 32, /*DC=*/ 26, /*RST=*/ 25, /*BUSY=*/ 27));

SPIClass hspi(HSPI);

//墨水屏驱动类 三色全局更新
#include<EPD_Driver.h>
#include "resources/resources.h"
#define SCREEN 266
EPD_Driver epdGlobal(eScreen_EPD_266, boardESP32DevKitC_EXT3);

//中文字库
#include <U8g2_for_Adafruit_GFX.h>
#include "gb2312.c"

U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;
extern const uint8_t chinese_gb2312[253023] U8G2_FONT_SECTION("chinese_gb2312");


//中文初始化
extern void EPD_ChineseInit(void)
{
  u8g2Fonts.begin(display);                        // 将u8g2过程连接到Adafruit GFX
  u8g2Fonts.setFontMode(1);                        // 使用u8g2透明模式（这是默认设置）
  u8g2Fonts.setFontDirection(0);                   // 从左到右（这是默认设置）
  u8g2Fonts.setForegroundColor(GxEPD_BLACK);             // 设置前景色
  u8g2Fonts.setBackgroundColor(GxEPD_WHITE);             // 设置背景色
  u8g2Fonts.setFont(chinese_gb2312);

}



extern void EPD_ChineseTest(void)
{
  char *chineseStr = "荒风落日，旷野无声";
  int strLength = u8g2Fonts.getUTF8Width(chineseStr);
  uint16_t strX = ((display.width() - strLength) / 2);   //使句子居中

  u8g2Fonts.drawUTF8(strX, 40, chineseStr);
  display.nextPage();

}

//
extern void EPD_GlobalInit(void)
{
  // EPD_Driver epdGlobal(eScreen_EPD_266, boardESP32DevKitC_EXT3);
  epdGlobal.COG_initial();
  // epdGlobal.globalUpdate(IMG_BW_BUFFER, IMG_RED_BUFFER);
  // delay(5);
  epdGlobal.globalUpdate(IMG_BW1_BUFFER, IMG_RED1_BUFFER);
  delay(5);
  // epdGlobal.globalUpdate(IMG_RED2_BUFFER, IMG_BW2_BUFFER);
  // Turn off CoG
  epdGlobal.COG_powerOff();

}

//
extern void EPD_GlobalTest(void)
{
  // epdGlobal.globalUpdate(BWR_blackBuffer, BWR_redBuffer);
  // // Turn off CoG
  // epdGlobal.COG_powerOff();
}

//
extern void EPD_Init(void)
{
  hspi.begin(14, 12, 13, 15);
  display.epd2.selectSPI(hspi, SPISettings(4000000, MSBFIRST, SPI_MODE0));
  display.init(115200);
  Serial.printf("墨水屏长宽:%d %d\n", display.height(), display.width());
  display.setRotation(3);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);

}



const char epdTestStr[] = "Time and tide wait for no man.";
extern void EPD_Test(void)
{
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(epdTestStr, 0, 0, &tbx, &tby, &tbw, &tbh);
  // center bounding box by transposition of origin:
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;
  
  display.setFullWindow();
  // display.setPartialWindow(0,0,100,266);
  display.firstPage();  //显示BUF初始化
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(epdTestStr);
  }
  while (display.nextPage());//发送buffer
  display.drawInvertedBitmap(x, y + 30,  ICON_SUNSHIN, 45, 45, GxEPD_BLACK);
  
}
//电压显示
extern void EPD_ShowVoltage(void)
{
  // display.setPartialWindow(0, 0, display.width(), display.height());
  String voltage = String(POWER_Check()) + " V";

  display.setPartialWindow(256, 0, 38, 10);
  // display.fillScreen(GxEPD_WHITE);
  // display.nextPage();
  u8g2Fonts.setCursor(256, 10);
 
  u8g2Fonts.print(voltage);
  display.nextPage();

  // EPD_PowerRemain();

  // LOG_Output("电压：%s\n", voltage.c_str());
}

extern void EPD_PowerRemain(void)
{
  float batteryV = POWER_Check();
  int powerNum = 0;
  static int powerNumLast = 0;
  if(batteryV > 3.5)
  {
    powerNum = int(101.26 - 0.265/(batteryV - 3.475));
  }
  else if(batteryV > 3.42)
  {
    powerNum = int(709.2*batteryV - 2390.2);
  }
  else
  {
    powerNum = int(239.36*batteryV - 784.0);
  }

  Serial.printf("电量百分比:%d %d\r\n", powerNum, powerNumLast);
  if(powerNum >= 100)
  {
    powerNum = 100;
  }

  if(powerNum <= 0)
  {
    powerNum = 0;
  }

  // if(powerNum <= powerNumLast)
  // {
  //   powerNum = powerNumLast;
  // }


  String powerPercent ="电量:" + String(powerNum) + "%";
  String powerVoltage = String(batteryV) + " V";

  display.setPartialWindow(180, 0, 104, 10);
  display.fillScreen(GxEPD_WHITE);
  display.nextPage();
  u8g2Fonts.setCursor(180, 10);
  u8g2Fonts.print(powerPercent);
  u8g2Fonts.setCursor(244, 10);
  u8g2Fonts.print(powerVoltage);
  display.nextPage();
  


  powerNumLast = powerNum;
}

//更新一行
extern void EPD_LineUpdate(uint8_t lineNum, String lineString) ////发送局部刷新的显示信息到屏幕,带居中
{
  const char *strContent = lineString.c_str();                            //String转换char
  uint16_t strLen = u8g2Fonts.getUTF8Width(strContent);         //获取字符的像素长度
  uint16_t strX = (display.width() / 2) - (strLen / 2);           //计算字符居中的X坐标（屏幕宽度/2-字符宽度/2）
  display.setPartialWindow(0, lineNum * 16, display.width(), 16);   //整行刷新
  display.firstPage();
  do
  {
    u8g2Fonts.setCursor(strX, lineNum * 16 + 13);
    u8g2Fonts.print(strContent);
  }
  while (display.nextPage());
  //display.powerOff(); //关闭屏幕电源
}

//更新一页txt信息
extern void EPD_TxtOnePage(String *zfc) //发送一页TXT信息到屏幕 0-8行
{
  //u8g2Fonts.setFont(chinese_gb2312);
  display.setPartialWindow(4, 20, 286, 180);
  display.firstPage();
  display.nextPage();

  do
  {
    for (uint8_t i = 0; i < 8; i++)
    {
      uint8_t offset = 0; //缩减偏移量
      if (zfc[i][0] == 0x20) //检查首行是否为半角空格 0x20
      {
        //继续检测后3位是否为半角空格，检测到连续的4个半角空格，偏移12个像素
        if (zfc[i][1] == 0x20 && zfc[i][2] == 0x20 && zfc[i][3] == 0x20)
          offset = 12;
      }
      else if (zfc[i][0] == 0xE3 && zfc[i][1] == 0x80 && zfc[i][2] == 0x80) //检查首行是否为全角空格 0x3000 = E3 80 80
      {
        //继续检测后2位是否为全角空格，检测到连续的2个全角空格，偏移2个像素
        if (zfc[i][3] == 0xE3 && zfc[i][4] == 0x80 && zfc[i][5] == 0x80)
          offset = 2;
      }
      u8g2Fonts.setCursor(6 + offset, i * 16 + 32);
      u8g2Fonts.print(zfc[i]);
      Serial.printf(zfc[i].c_str());
     
    }
  }
  while (display.nextPage());
  delay(10);
  display.powerOff(); //仅关闭屏幕电源

}

extern void EPD_Refresh(void)
{

  // display.setPartialWindow(0, 0, display.width(), display.height());
  // display.fillScreen(GxEPD_BLACK);
  // display.nextPage();
  // delay(1000);
  // display.firstPage();
  // display.nextPage();

  display.fillScreen(GxEPD_BLACK);  // 填充屏幕
  display.display(1);         // 显示缓冲内容到屏幕，用于全屏缓冲
  display.fillScreen(GxEPD_WHITE);
  display.display(1);

}
