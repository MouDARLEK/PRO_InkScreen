#ifndef _BOARD_H_
#define _BOARD_H_

typedef void (* KEY_CB) (void);
class KEY
{
  public:
    uint8_t KEY_PIN;
    KEY_CB KEY_EVENT;

    int keyPressTimes = 0;
    bool keyPressed = false;


    KEY(uint8_t keyPin, KEY_CB keyEvent);
    void KEY_EventInit(void);
    void KEY_Clear(void);
  private:
    

};

extern void LED_Init(void);
extern void LED_Blink(void);

extern void POWER_Init(void);
extern float POWER_Check(void);

void KEY_Event(void);
extern void KEY_Init(void);
extern void KEY_Read(void);
extern void KEY_DirMode(void);
extern void KEY_ReadMode(void);
extern void KEY_LogMode(void);
extern void KEY_ModeDetect(void);

extern void UART2_Init(void);
void serialEvent2(void);

extern void TIMER_Init(void);

#endif