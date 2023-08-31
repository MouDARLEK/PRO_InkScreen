
#include "epd.h"
#include "board.h"
#include "sdtxt.h"


void setup()
{
  LED_Init();
  Serial.begin(115200);

  // EPD_GlobalInit();
  // delay(1000);

  EPD_Init();
  EPD_Test();
  
  // delay(3000);
  EPD_ChineseInit();
  EPD_ChineseTest();

  KEY_Init();
  // SD_Test();


}

void loop() 
{
  static int loopTimes = 0;
  loopTimes ++;

  if(loopTimes % 300 == 0)
  {
    LED_Blink();
    Serial.printf("H(⊙w⊙)H\r\n");
    
  }
  
  if(loopTimes % 100 == 0)
  {

    KEY_Read();
  }

  if(loopTimes % 1500 == 0)
  {

   EPD_ShowVoltage();
  }

  delay(1);
}
