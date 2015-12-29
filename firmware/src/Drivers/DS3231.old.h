#ifndef _DS3231_H
#define _DS3231_H

#include "i2c.h"

#define DS3231_CHIPADDR	0xD0

typedef struct _ds3231Time
{
	u8 bcdSecond;
	u8 bcdMinute;
	u8 bcdHour;
}DS3231Time_t;

typedef struct _ds3231Date
{
	u8 byWeek;
	u8 bcdDay;
	u8 bcdMonth;
	u8 bcdYear;
}DS3231Date_t;

typedef struct _ds3231DateTime
{
	u8 bcdSecond;
	u8 bcdMinute;
	u8 bcdHour;
	u8 byWeek;
	u8 bcdDay;
	u8 bcdMonth;
	u8 bcdYear;
}DS3231DateTime_t;

typedef struct _ds3231Alarm
{
	u8 bcdSecond;
	u8 bcdMinute;
	u8 bcdHour;
	u8 bcdDay:7;
	u8 bWeek:1;
}DS3231Alarm_t;

typedef enum {
	dsaTimeAddr,
	dsaDateAddr=0x03,
	dsaAlarm1Addr=0x07,
	dsaAlarm2Addr=0x0B,
	dsaControlAddr=0x0E,
	dsaStatusAddr,
	dsaAgingOffsetAddr,
	dsaTempAddr
}DS3231RegisterAddr_t;

void rtc_Init(void);
BOOL rtc_GetTime(PMyTime tm);
BOOL rtc_GetDate(PMyDate dt, u8 *week);
BOOL rtc_GetDateTime(PMyDate dt, PMyTime tm, u8 *week);
BOOL rtc_SetDateTime(PMyDate dt, PMyTime tm, u8 byWeek);

#endif

