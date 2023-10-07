#ifndef _SDTXT_H_
#define _SDTXT_H_

#include "Arduino.h"

extern void SD_Init(void);
extern void SD_Test(void);
int8_t getCharLength(char zf);

extern void SD_GetOnePage(void);
extern void SD_TxtInit(void);
extern void SD_Clear(void);
extern void SD_FsInit(void);
extern void SD_SeekPreviousPage(void);
float SD_GetReadProgerss(void);


extern void SD_IndexMatch(String txtName);
extern void SD_IndexRead(uint8_t* readBuff, uint64_t seekByte, uint32_t readBytes);
extern void SD_IndexWrite(String dataStr, uint64_t seekByte, uint32_t writeBytes);
extern int SD_IndexSearch(uint32_t bookNum, String txtName);



class BOOK
{
  public:
    String bookName;
    uint64_t bookSize;
    uint64_t readByte;

    uint32_t indexByte;


    bool BOOK_Init(String txtName);


  private:
    

};



#endif