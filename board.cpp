#include "esp32-hal-gpio.h"
#include "board.h"
#include "Arduino.h"
#include "epd.h"
#include "sdtxt.h"


#define LED1 2
#define ADC_POWER 4 
#define KEY_R 35
#define KEY_M 34
#define KEY_L 39

hw_timer_t * timer = NULL;

KEY KEY_LEFT(KEY_L, KEY_Event);
KEY KEY_RIGHT(KEY_R, KEY_Event);
KEY KEY_MIDDLE(KEY_M, KEY_Event);

static float BATTERT_CONVERT_NUM = 0.004549; //= vol/4095*3.3/(1/(4.7+1))

void TIMER_Event(void)
{
  static int timerCounter = 0;
  LED_Blink();
}

extern void TIMER_Init(void)
{
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &TIMER_Event, true);
  timerAlarmWrite(timer, 1000000, true);
  timerAlarmEnable(timer);
}

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





KEY::KEY(uint8_t keyPin, KEY_CB keyEvent)
{

  this->KEY_PIN = keyPin;
  this->KEY_EVENT = keyEvent;
}

void KEY::KEY_EventInit(void)
{
  pinMode(this->KEY_PIN, INPUT_PULLUP);
  attachInterrupt(this->KEY_PIN, this->KEY_EVENT, FALLING);
}

void KEY_Event(void)
{
  if(digitalRead(KEY_LEFT.KEY_PIN) == LOW)
  {
    KEY_LEFT.keyPressTimes ++;
    KEY_LEFT.keyPressed = true;
  }

  if(digitalRead(KEY_RIGHT.KEY_PIN) == LOW)
  {
    KEY_RIGHT.keyPressTimes ++;
    KEY_RIGHT.keyPressed = true;
  }

  if(digitalRead(KEY_MIDDLE.KEY_PIN) == LOW)
  {
    KEY_MIDDLE.keyPressTimes ++;
    KEY_MIDDLE.keyPressed = true;
  }
}


extern void KEY_Init(void)
{

  KEY_LEFT.KEY_EventInit();
  KEY_RIGHT.KEY_EventInit();
  KEY_MIDDLE.KEY_EventInit();
}


extern void KEY_Read(void)
{
  // Serial.printf("KEY:%d %d %d\r\n",KEY_LEFT.keyPressTimes, KEY_MIDDLE.keyPressTimes, KEY_RIGHT.keyPressTimes);
  if(KEY_LEFT.keyPressed == true && KEY_LEFT.keyPressTimes <= 1)
  {
 
    SD_TxtInit();
    SD_GetOnePage();
    KEY_LEFT.keyPressed = false;
    
  }
  else if(KEY_LEFT.keyPressed == true)
  {
    // SD_Clear();
    SD_GetOnePage();
    KEY_LEFT.keyPressed = false;
    
  }

  if(KEY_RIGHT.keyPressed == true )
  {
    KEY_LEFT.keyPressTimes = 0;
    KEY_RIGHT.keyPressed = false;
    SD_Test();
    // EPD_GlobalInit();

  }

   if(KEY_MIDDLE.keyPressed == true )
  {
    EPD_Refresh();

    KEY_RIGHT.keyPressTimes = 0;
    KEY_MIDDLE.keyPressTimes = 0; 
    KEY_MIDDLE.keyPressed = false;
    
  }


}





















