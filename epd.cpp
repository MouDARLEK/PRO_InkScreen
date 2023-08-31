#include "epd.h"
#include "Arduino.h"
#include "board.h"

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

extern void EPD_GlobalTest(void)
{
  // epdGlobal.globalUpdate(BWR_blackBuffer, BWR_redBuffer);
  // // Turn off CoG
  // epdGlobal.COG_powerOff();
}





extern void EPD_Init(void)
{
  hspi.begin(14, 12, 13, 15);
  display.epd2.selectSPI(hspi, SPISettings(4000000, MSBFIRST, SPI_MODE0));
  display.init(115200);

}



const char epdTestStr[] = "Time and tide wait for no man.";
extern void EPD_Test(void)
{
  display.setRotation(3);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
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
  display.setPartialWindow(0, 0, display.width(), display.height());
  String voltage = String(POWER_Check()) + " V";

  display.setPartialWindow(140, 113, 50, 50);
  display.fillScreen(GxEPD_WHITE);
  display.nextPage();

  u8g2Fonts.setCursor(140, 113 +20);
 
  u8g2Fonts.print(voltage);
  display.nextPage();
}

