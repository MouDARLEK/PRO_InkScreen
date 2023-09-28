#ifndef _SDTXT_H_
#define _SDTXT_H_

//SD卡测试
extern void SD_Init(void);
extern void SD_Test(void);
int8_t getCharLength(char zf);

extern void SD_GetOnePage(void);
extern void SD_TxtInit(void);
extern void SD_Clear(void);
extern void SD_FsInit(void);




#endif