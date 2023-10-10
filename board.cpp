#include "esp32-hal-gpio.h"
#include "board.h"
#include "Arduino.h"
#include "epd.h"
#include "sdtxt.h"

#include "log.h"


#define LED1 2
#define ADC_POWER 4 
#define KEY_R 35
#define KEY_M 34
#define KEY_L 39

typedef enum
{
  DEFAULT_MODE,
  DIRECTORY_MODE,
  READ_MODE,
  LOG_MODE,
  SLEEP_MODE
}DEVICE_MODE;

DEVICE_MODE deviceMode =  DIRECTORY_MODE;
DEVICE_MODE deviceLastMode =  DEFAULT_MODE;


bool KEY_SLEEP_FLAG = true;
bool SLEEP_ONCE_FLAG = false;
bool BAN_LOG_FLAG = true;


hw_timer_t * timer = NULL;


KEY KEY_LEFT(KEY_L, KEY_Event);
KEY KEY_RIGHT(KEY_R, KEY_Event);
KEY KEY_MIDDLE(KEY_M, KEY_Event);

// static float BATTERT_CONVERT_NUM = 0.004549; //= vol/4095*3.3/(1/(4.7+1))
static float BATTERT_CONVERT_NUM = 0.005488; //= vol/4095*3.3/(0.91/(4.7+0.91))

String SERIAL2_BUFF = {};

void serialEvent2(void)
{
  // SERIAL2_BUFF.clear();
  while(Serial2.available())
  {
    SERIAL2_BUFF += (char)Serial2.read();
  }
  if(BAN_LOG_FLAG)
  {
    return;

  }
  deviceMode = LOG_MODE;


}

extern void UART2_Init(void)
{
  Serial2.begin(115200);
  Serial2.printf("SERIAL2 INIT SUCCESS!\n");
}

void TIMER_Event(void)
{
  static uint32_t timerSleepCounter = 0;
  static uint32_t timerCounter = 0;
  
  timerSleepCounter ++;
  timerCounter ++;

  LED_Blink();

  if(timerCounter >= 2)
  {
    BAN_LOG_FLAG = false;
  }

  if(KEY_SLEEP_FLAG == false)
  {
    KEY_SLEEP_FLAG = true;
    timerSleepCounter = 0;
  }

  if(timerSleepCounter >= 90 && (!SLEEP_ONCE_FLAG) && (deviceMode != LOG_MODE))
  {
    deviceMode = SLEEP_MODE;
    SLEEP_ONCE_FLAG = true;
  }

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

void KEY::KEY_Clear(void)
{
  this->keyPressTimes = 0;
  this->keyPressed = false;
}

void KEY_Event(void)
{
  if(digitalRead(KEY_LEFT.KEY_PIN) == LOW)
  {
    KEY_LEFT.keyPressTimes ++;
    KEY_LEFT.keyPressed = true;
    KEY_SLEEP_FLAG = false;
  }

  if(digitalRead(KEY_RIGHT.KEY_PIN) == LOW)
  {
    KEY_RIGHT.keyPressTimes ++;
    KEY_RIGHT.keyPressed = true;
    KEY_SLEEP_FLAG = false;
  }

  if(digitalRead(KEY_MIDDLE.KEY_PIN) == LOW)
  {
    KEY_MIDDLE.keyPressTimes ++;
    KEY_MIDDLE.keyPressed = true;
    KEY_SLEEP_FLAG = false;
  }
}


extern void KEY_Init(void)
{

  KEY_LEFT.KEY_EventInit();
  KEY_RIGHT.KEY_EventInit();
  KEY_MIDDLE.KEY_EventInit();
}

extern void KEY_ClearAll(void)
{
  KEY_LEFT.KEY_Clear();
  KEY_RIGHT.KEY_Clear();
  KEY_MIDDLE.KEY_Clear();
}

extern void KEY_ReadMode(void)
{

  if(deviceMode != deviceLastMode)
  {
    deviceLastMode = deviceMode;
    SD_TxtInit();
    SD_GetOnePage();
    KEY_ClearAll();
  }
  if(KEY_LEFT.keyPressed == true)
  {
    SD_GetOnePage();
    KEY_LEFT.keyPressed = false;
  }
  if(KEY_RIGHT.keyPressed == true )
  {
    KEY_RIGHT.keyPressed = false;
    SD_SeekPreviousPage();
    SD_GetOnePage();

  }

  if(KEY_MIDDLE.keyPressed == true )
  {
    deviceMode = DIRECTORY_MODE;
    KEY_MIDDLE.keyPressed == false;
  }


}

extern void KEY_DirMode(void)
{
  static int dirPointer = 0;
  static uint8_t txtNum = 0;
  if(deviceMode != deviceLastMode)
  {
    SD_CloseBook();
    txtNum = SD_FsInit();
    KEY_ClearAll();
    EPD_FingerArrow(1);
    deviceLastMode = deviceMode;
    dirPointer = 0;
  }

  if(txtNum > 8)//最多显示八本
  {
    txtNum = 8;
  }

  if(KEY_LEFT.keyPressed == true)
  {
    dirPointer = ((dirPointer + 1) % txtNum);
    EPD_FingerArrow(dirPointer + 1);
    KEY_LEFT.keyPressed = false;
    Serial.printf("文件列表指向:%d %d", dirPointer, txtNum);

  }
  if(KEY_RIGHT.keyPressed == true )
  {
    dirPointer --;
    if(dirPointer < 0)
    {
      dirPointer = txtNum - 1;
    }
    EPD_FingerArrow(dirPointer + 1);
    KEY_RIGHT.keyPressed = false;
    Serial.printf("文件列表指向:%d %d", dirPointer, txtNum);
  }
  
  if(KEY_MIDDLE.keyPressed == true )
  {
    SD_SelectBook(dirPointer);
    deviceMode = READ_MODE;
    KEY_MIDDLE.keyPressed == false;
  }
  
}

extern void KEY_SleepMode(void)
{
  if(deviceMode != deviceLastMode)
  {
    Serial.printf("进入睡眠模式\n");
    SD_CloseBook();
    EPD_ShowSleepPic();
    KEY_ClearAll();
    deviceLastMode = deviceMode;
    
  }

  if(KEY_SLEEP_FLAG == false)
  {
    deviceMode = DIRECTORY_MODE;
    Serial.printf("退出睡眠模式\n");
  }

}

extern void KEY_LogMode(void)
{
  if(deviceMode != deviceLastMode)
  {
    Serial.printf("进入日志模式\n");
    EPD_ShowLogSign();
    SD_CloseBook();
    KEY_ClearAll();
    deviceLastMode = deviceMode;

  }
  

  if(KEY_MIDDLE.keyPressed == true )
  {
    deviceMode = DIRECTORY_MODE;
    KEY_MIDDLE.keyPressed == false;
  }

  if(SERIAL2_BUFF.length() != 0)
  {
    Serial.printf("日志接收:%s 长度%d\n", SERIAL2_BUFF.c_str(), SERIAL2_BUFF.length());
    LOG_Output("%s", SERIAL2_BUFF.c_str());
    SERIAL2_BUFF.clear();

  }
  
  


}


extern void KEY_ModeDetect(void)
{
  // Serial.printf("mode:%d\n", deviceMode);
  if(deviceMode == DIRECTORY_MODE)
  {
    KEY_DirMode();
  }

  if(deviceMode == READ_MODE)
  {
    KEY_ReadMode();
  }

  if(deviceMode == SLEEP_MODE)
  {
    KEY_SleepMode();
  }

  if(deviceMode == LOG_MODE)
  {
    KEY_LogMode();
  }

}


extern void KEY_Read(void)
{
  if(KEY_LEFT.keyPressed == true && KEY_LEFT.keyPressTimes <= 1)
  {

    KEY_LEFT.keyPressed = false;
  }
  else if(KEY_LEFT.keyPressed == true)
  {

    KEY_LEFT.keyPressed = false;
    
  }

  if(KEY_RIGHT.keyPressed == true )
  {
    KEY_RIGHT.keyPressed = false;


  }

  if(KEY_MIDDLE.keyPressed == true )
  {
    KEY_LEFT.keyPressTimes = 0;
    KEY_RIGHT.keyPressTimes = 0; 
    KEY_MIDDLE.keyPressed = false;
    
  }


}




















