#include "board.h"
#include "Arduino.h"

#define LED1 2

extern void LED_Init(void)
{
  pinMode(LED1, OUTPUT);
}


extern void LED_Blink(void)
{
  static int ledState = 0;
  ledState ++;
  digitalWrite(LED1,  ((ledState%2) == 0) ? HIGH:LOW );
}