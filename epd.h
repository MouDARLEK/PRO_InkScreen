#ifndef _EPD_H_
#define _EPD_H_

#include "Arduino.h"

extern void EPD_Init(void);
extern void EPD_Test(void);

extern void EPD_GlobalInit(void);
extern void EPD_GlobalTest(void);

extern void EPD_ChineseInit(void);
extern void EPD_ChineseTest(void);

extern void EPD_ShowVoltage(void);
extern void EPD_PowerRemain(void);
extern void EPD_LineUpdate(uint8_t lineNum, String lineString);
extern void EPD_TxtOnePage(String *zfc);
extern void EPD_Refresh(void);
#endif