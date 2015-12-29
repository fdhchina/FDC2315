#include "log.h"
#include "hal.h"
#include "DS3231.h"
static FIL		logFile;
static char logBuffer[512]={0};
static char logInitBuffer[512]={0};
static TMyTime tm;
static TMyDate dt;
static u8 week;
void Log_addDate(char *pdt, u32 length);

void Log_Init()
{	
	uint32_t		dwWriteCnt;
	
	if(f_open(&logFile, LOG_PATH,  FA_OPEN_ALWAYS | FA_WRITE)==FR_OK)
	{
		sprintf(logInitBuffer,"welcome to use FDC2315, the version is %s\r\n",PRODUCT_VER);
		Log_addDate(logInitBuffer, 0);
		if(logFile.fsize<2*1024*1024)			
			f_lseek(&logFile, logFile.fsize);
		f_write(&logFile, logBuffer, strlen(logBuffer), &dwWriteCnt);
		f_sync(&logFile);
	}
}

void Log_write(char *pdt, u32 length)
{
		uint32_t		dwWriteCnt;
		Log_addDate(pdt, length);
		f_write(&logFile, logBuffer, strlen(logBuffer), &dwWriteCnt);
		f_sync(&logFile);
}

void Log_addDate(char *pdt, u32 length)
{
		length = length>0?length:strlen(pdt);
		rtc_GetDate(&dt, &week);
		rtc_GetTime(&tm);
		memset(logBuffer, 0, 512 );
		sprintf(logBuffer,"[%4d-%2d-%2d %2d:%2d:%2d ÐÇÆÚ%d]",dt.wYear, dt.byMonth, dt.byDay, tm.byHour,tm.byMinute,tm.bySecond, week);
		memcpy(logBuffer+strlen(logBuffer), pdt, length);
		memcpy(logBuffer+strlen(logBuffer), "\r\n", 2);
}
	
void Log_test()
{

}









