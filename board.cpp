#include "board.h"
#include "Arduino.h"

#define LED1 2
#define ADC_POWER 4 
#define KEY_R 35
#define KEY_M 34
#define KEY_L 39

static float BATTERT_CONVERT_NUM = 0.004549; //= vol/4095*3.3/(1/(4.7+1))

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

extern void POWER_Init(void)
{
  pinMode(ADC_POWER, INPUT);
}

extern float POWER_Check(void)
{
  float powerRead = 0;
  powerRead = analogRead(ADC_POWER) * BATTERT_CONVERT_NUM;
  Serial.printf("BAT:%0.2f\r\n",powerRead);
  return powerRead;
}


extern void KEY_Init(void)
{
  pinMode(KEY_L, INPUT);
  pinMode(KEY_M, INPUT);
  pinMode(KEY_R, INPUT);
}


extern void KEY_Read(void)
{
  Serial.printf("KEY:%d %d %d\r\n",digitalRead(KEY_L), digitalRead(KEY_M), digitalRead(KEY_R));
}




