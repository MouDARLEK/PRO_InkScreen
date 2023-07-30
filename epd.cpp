#include "epd.h"
#include "Arduino.h"

//墨水屏驱动类 黑白速刷
#define ENABLE_GxEPD2_GFX 0
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>

#define GxEPD2_DISPLAY_CLASS GxEPD2_BW
#define GxEPD2_DRIVER_CLASS GxEPD2_260

#define MAX_DISPLAY_BUFFER_SIZE 65536ul
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))

GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display(GxEPD2_DRIVER_CLASS(/*CS=*/ 32, /*DC=*/ 26, /*RST=*/ 25, /*BUSY=*/ 27));

SPIClass hspi(HSPI);

//三色全局刷
#include<EPD_Driver.h>
#include "resources/resources.h"
#define SCREEN 266
EPD_Driver epdGlobal(eScreen_EPD_266, boardESP32DevKitC_EXT3);

//中文字库
// #include <U8g2_for_Adafruit_GFX.h>
// #include "gb2312.c"




extern void EPD_GlobalInit(void)
{
  // EPD_Driver epdGlobal(eScreen_EPD_266, boardESP32DevKitC_EXT3);
  epdGlobal.COG_initial();
  // epdGlobal.globalUpdate(BW_monoBuffer, BW_0x00Buffer);
  epdGlobal.globalUpdate(IMG_BW_BUFFER, IMG_RED_BUFFER);
  // epdGlobal.globalUpdate(BWR_blackBuffer, BWR_redBuffer);
  // // Turn off CoG
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
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(epdTestStr, 0, 0, &tbx, &tby, &tbw, &tbh);
  // center bounding box by transposition of origin:
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;
  display.setFullWindow();
 

  display.firstPage();  //显示BUF初始化
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(epdTestStr);
  }
  while (display.nextPage());//发送buffer

  
  int i = 0;
  for(i = 0; i <= 266; i++)
  {
    display.drawPixel(10, i, GxEPD_BLACK);
    display.drawPixel(11, i, GxEPD_BLACK);
  }
  display.nextPage();
  




}


