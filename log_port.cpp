#include "stdio.h"
#include "string.h"
#include "log.h"

#include "board.h"
#include "Arduino.h"
#include "SD.h"

static char PORT_STR_BUF[LOG_BUF_MAX_LENGTH] = {0};


extern void Port_SdcWrite(char* dataStr, int dataLength)
{
	memset(PORT_STR_BUF, 0, LOG_BUF_MAX_LENGTH);
	snprintf(PORT_STR_BUF,  dataLength, "%s", (const char*)dataStr);

  if( LOG_GetPort() != LOG_SDCARD)
  {
      return;
  }

  File logFile = SD.open("/log.txt", FILE_WRITE);
  if(logFile)
  {
    logFile.seek(logFile.size());
    logFile.printf("%s", PORT_STR_BUF);
    logFile.close();
  }
  else
  {
     Serial.printf("打开文件失败%d\n", logFile);
  }


}


extern void PORT_UartSend(char* sendStr, int sendLength)
{
	memset(PORT_STR_BUF, 0, LOG_BUF_MAX_LENGTH);
	snprintf(PORT_STR_BUF, sendLength, "%s", (const char*)sendStr);
  Serial.printf("%s", (uint8_t*)PORT_STR_BUF);
}