#include "epd.h"
#include "board.h"
#include "sdtxt.h"



void setup()
{
  LED_Init();
  TIMER_Init();
  Serial.begin(115200);
  UART2_Init();
  
  // EPD_GlobalInit();


  EPD_Init();
  // EPD_Test();
  
  EPD_ChineseInit();
  // EPD_ChineseTest();

  KEY_Init();
  SD_Init();

}

void loop() 
{
  static uint32_t loopTimes = 0;
  loopTimes ++;

  if(loopTimes % 40 == 0)
  {
    KEY_ModeDetect();
  }
  
  if(loopTimes % 200 == 0)
  {
  }

  if(loopTimes % (200*600) == 0)
  {
   loopTimes = 0;

  }

  delay(5);
}
