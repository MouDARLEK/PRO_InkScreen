
#include "epd.h"
#include "board.h"


void setup()
{
  LED_Init();
  Serial.begin(115200);

  EPD_GlobalInit();
  delay(1000);
  EPD_Init();
  EPD_Test();

}

void loop() 
{
  LED_Blink();
  Serial.printf("H(⊙w⊙)H\r\n");
  delay(500);

}
