#include "hal.h"
#include "task.h"
#include "ds3231.h"

#define ds3231_i2c_Get(subaddr, buf, size) I2C1_ReadBuffer(DS3231_CHIPADDR, subaddr, 1, buf, size)
#define ds3231_i2c_Put(subaddr, buf, size) I2C1_WriteBuffer(DS3231_CHIPADDR, subaddr, 1, buf, size)

u8 rtc_getStatus()
{
	u8 res;
	if(ds3231_i2c_Get(dsaStatusAddr, &res, 1)==0)
		return res&0x07F;
	return 0;
}

u8 ds_getDat(u8 addr, u8* buf, u8 size)
{
	u8 i;
	for(i=0;i<size; i++)
	{
		while(rtc_getStatus()&0x04);	// ÅÐ¶ÏÐ¾Æ¬¿ÕÏÐ·ñ
		if(ds3231_i2c_Get(addr++, buf++, 1)!=0)
			break;
	}
	return i;
}

u8 ds_putDat(u8 addr, u8* buf, u8 size)
{
	u8 i;
	for(i=0;i<size; i++)
	{
		while(rtc_getStatus()&0x04);	// ÅÐ¶ÏÐ¾Æ¬¿ÕÏÐ·ñ
		if(ds3231_i2c_Put(addr++, buf++, 1)!=0)
			break;
	}
	return i;
}

void rtc_Init()
{
	u8 byDat[2]= {0x00, 0x00};
	I2C1_Configuration();

	// ds3231_i2c_Put(dsaControlAddr,  byDat,  2);
	ds_putDat(dsaControlAddr, byDat, 2);
}

u8 bcd2dec(u8 byBcd)
{
	u8 res= ((byBcd &0xF0)>>4)*10;
	res += byBcd&0x0F;
	return res;
}

u8 dec2bcd(u8 byDec)
{
	u8 res= (byDec / 10)<<4;
	res |= byDec % 10;
	return res;
}

BOOL rtc_GetDateTime(PMyDate dt, PMyTime tm, u8 *week)
{
	DS3231DateTime_t dtm;
	//while(rtc_getStatus()&0x04);	// ÅÐ¶ÏÐ¾Æ¬¿ÕÏÐ·ñ
	//if(ds3231_i2c_Get(dsaTimeAddr,  (u8 *)&dtm, sizeof(DS3231DateTime_t))==0)
	if(ds_getDat(dsaTimeAddr, (u8 *)&dtm, sizeof(DS3231DateTime_t))==sizeof(DS3231DateTime_t))
	{
		dt->wYear= 2000+bcd2dec(dtm.bcdYear);
		dt->byMonth= bcd2dec(dtm.bcdMonth);
		dt->byDay= bcd2dec(dtm.bcdDay);
		*week= dtm.byWeek;
		tm->byHour= bcd2dec(dtm.bcdHour);
		tm->byMinute= bcd2dec(dtm.bcdMinute);
		tm->bySecond= bcd2dec(dtm.bcdSecond);
		return TRUE;
	}
	return FALSE;
}

BOOL rtc_GetDate(PMyDate dt, u8 *week)
{
	DS3231Date_t dtm;
	while(rtc_getStatus()&0x04);	// ÅÐ¶ÏÐ¾Æ¬¿ÕÏÐ·ñ
	if(ds3231_i2c_Get(dsaDateAddr,  (u8 *)&dtm, sizeof(DS3231Date_t))==0)
	{
		dt->wYear= 2000+bcd2dec(dtm.bcdYear);
		dt->byMonth= bcd2dec(dtm.bcdMonth);
		dt->byDay= bcd2dec(dtm.bcdDay);
		*week= dtm.byWeek;
		return TRUE;
	}
	return FALSE;
}

BOOL rtc_GetTime(PMyTime tm)
{
	DS3231Time_t dtm;
	//while(rtc_getStatus()&0x04);	// ÅÐ¶ÏÐ¾Æ¬¿ÕÏÐ·ñ
	//if(ds3231_i2c_Get(dsaTimeAddr,  (u8 *)&dtm, sizeof(DS3231Time_t))==0)
	if(ds_getDat(dsaTimeAddr, (u8 *)&dtm, sizeof(DS3231Time_t))==sizeof(DS3231Time_t))
	{
		tm->byHour= bcd2dec(dtm.bcdHour);
		tm->byMinute= bcd2dec(dtm.bcdMinute);
		tm->bySecond= bcd2dec(dtm.bcdSecond);
		return TRUE;
	}
	return FALSE;
}

BOOL rtc_SetDateTime(PMyDate dt, PMyTime tm, u8 byWeek)
{
	DS3231DateTime_t dtm;
	while(rtc_getStatus()&0x04);	// ÅÐ¶ÏÐ¾Æ¬¿ÕÏÐ·ñ
	dtm.bcdYear= dec2bcd(dt->wYear-2000);
	dtm.bcdMonth= dec2bcd(dt->byMonth);
	dtm.bcdDay= dec2bcd(dt->byDay);
	dtm.byWeek= byWeek;
	dtm.bcdHour= dec2bcd(tm->byHour);
	dtm.bcdMinute= dec2bcd(tm->byMinute);
	dtm.bcdSecond= dec2bcd(tm->bySecond);

	return (BOOL)(ds3231_i2c_Put(dsaTimeAddr, (u8 *)&dtm, sizeof(DS3231DateTime_t))==0);
}

