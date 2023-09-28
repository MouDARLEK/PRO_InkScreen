
#include "epd.h"
#include "board.h"
#include "sdtxt.h"



void setup()
{
  LED_Init();
  TIMER_Init();
  Serial.begin(115200);

 
  
  // EPD_GlobalInit();
  // delay(1000);

  EPD_Init();
  // EPD_Test();
  
  // delay(3000);
  EPD_ChineseInit();
  // EPD_ChineseTest();

  KEY_Init();
  SD_Init();

  


}

void loop() 
{
  static uint32_t loopTimes = 0;
  loopTimes ++;

  if(loopTimes % 300 == 0)
  {
    KEY_Read();
  }
  
  if(loopTimes % 1000 == 0)
  {
    // KEY_Read();
  }

  if(loopTimes % (1000*40) == 0 || loopTimes == 1)
  {
  //  EPD_ShowVoltage();
   EPD_PowerRemain();
  
  }

  if(loopTimes % (1000*90) == 0)
  {
   loopTimes = 0;

  }

  delay(1);
}
