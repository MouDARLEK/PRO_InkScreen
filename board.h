#ifndef _BOARD_H_
#define _BOARD_H_

extern void LED_Init(void);
extern void LED_Blink(void);

extern void POWER_Init(void);
extern float POWER_Check(void);

extern void KEY_Init(void);
extern void KEY_Read(void);

#endif