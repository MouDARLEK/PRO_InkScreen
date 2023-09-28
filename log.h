/********************************************************************************
* @file  :
* @author: XXY
* @date  : 2022.11.22
* @brief :
********************************************************************************/

#ifndef _LOG_H
#define _LOG_H

#include "stdint.h"

typedef enum
{
    LOG_Banned,
    LOG_LEV_A,
    LOG_LEV_B,
    LOG_LEV_C,
    LOG_LEV_D,
    LOG_LEV_E
}LOG_LEVEL_E;

typedef enum
{
    LOG_SERIAL,
    LOG_SDCARD,
    LOG_PC
}LOG_PORT_E;


#define LOG_BUF_MAX_LENGTH 4096
#define LOG_USED_MAX_TIMES 100

extern void LOG_Init(void);

extern void LOG_Msg(LOG_LEVEL_E logLevel, const char* fileName, const char* funcName , int lineNum,const  char *logStr, ...);
extern void LOG_SetLevel(LOG_LEVEL_E logNowLevel);
extern void LOG_SetPort(LOG_PORT_E logNowPort);
extern LOG_PORT_E LOG_GetPort(void);

extern void LOG_Update(void);
extern void LOG_UpdateWithTime(uint64_t logTime);

extern void Port_SdcWrite(char* dataStr, int dataLength);
extern void PORT_UartSend(char* sendStr, int sendLength);

#define LOG_Printf(logStr, ...)             LOG_Msg(LOG_LEV_A, __FILE__, __FUNCTION__, __LINE__, logStr, ##__VA_ARGS__)
#define LOG_Level(logLev, logStr, ...)      LOG_Msg(logLev, __FILE__, __FUNCTION__, __LINE__, logStr, ##__VA_ARGS__)
#define LOG_Output(logStr, ...)             LOG_Msg(LOG_LEV_A, __FILE__, __FUNCTION__, __LINE__, logStr, ##__VA_ARGS__);\ 
                                            LOG_Update()




#endif


