#include "hal.h"
#include "task.h"
#include  "ds3231.h"				

#define DS3231_IIC_ADDR 0xd0
/********************************************************************************************************
** 	全局常数定义
********************************************************************************************************/
/********************************************************************************************************
** 	DS3231常数定义
********************************************************************************************************/
#define DS3231_WriteAddress 0xD0    //器件写地址 
#define DS3231_ReadAddress  0xD1    //器件读地址

#define DS3231_SECOND       0x00    //秒
#define DS3231_MINUTE       0x01    //分
#define DS3231_HOUR         0x02    //时
#define DS3231_WEEK         0x03    //星期
#define DS3231_DAY          0x04    //日
#define DS3231_MONTH        0x05    //月
#define DS3231_YEAR         0x06    //年
//闹铃1            
#define DS3231_ALARM1SECOND 0x07    //秒
#define DS3231_ALARM1MINUTE 0x08    //分
#define DS3231_ALARM1HOUR   0x09    //时
#define DS3231_ALARM1WEEK   0x0A    //星期/日
//闹铃2
#define DS3231_ALARM2MINUTE 0x0b    //分
#define DS3231_ALARM2HOUR   0x0c    //时
#define DS3231_ALARM2WEEK   0x0d    //星期/日

#define DS3231_CONTROL      0x0e    //控制寄存器
#define DS3231_STATUS       0x0f    //状态寄存器
#define BSY                 2       //忙
#define OSF                 7       //振荡器停止标志
#define DS3231_XTAL         0x10    //晶体老化寄存器
#define DS3231_TEMPERATUREH 0x11    //温度寄存器高字节(8位)
#define DS3231_TEMPERATUREL 0x12    //温度寄存器低字节(高2位)  	

u8	BCD2HEX(u8 val)
{
	return	((val>>4)*10)+(val&0x0f);
}

u8	HEX2BCD(u8 val)
{
	return	(((val%100)/10)<<4)|(val%10);
}


void rtc_Init( void )
{ 
	u8  ControlReg;	
 
       ControlReg= I2CReadByte(DS3231_IIC_ADDR, DS3231_STATUS);

	if((ControlReg&0x80)==0x80)
      	{
		I2CWriteByte(DS3231_IIC_ADDR, DS3231_STATUS, ControlReg&0x7f);	 
		// Rtc_WriteTime(0x12,0x04,0x10,0x12,0x30,0x00);
		rtc_SetDateTime(&g_SysDate, &g_SysTime, g_SysWeek);

		ControlReg= I2CReadByte(DS3231_IIC_ADDR, DS3231_STATUS);

		if((ControlReg&0x80)==0x80)
		{  
			// VFD_Draw_s8(0,0,"Rtc init error!");	
		}else
		{
				//VFD_Draw_s8(0,0,"Rtc init ok!");	

		}
	}else
	{
		// VFD_Draw_s8(0,0,"Rtc init ok!");	
	}
	//Rtc_WriteTime(0x12,0x04,0x10,0x23,0x07,0x00);
	return;
}

BOOL rtc_SetDateTime(PMyDate dt, PMyTime tm, u8 byWeek)
{
	u16	wYear;
	if(dt->wYear>2099)
		wYear= dt->wYear-2100;
	else
		wYear= dt->wYear-2000;

	if(I2CWriteByte(DS3231_IIC_ADDR, DS3231_YEAR, HEX2BCD(wYear)))
	 if(I2CWriteByte(DS3231_IIC_ADDR, DS3231_MONTH, HEX2BCD(dt->byMonth)))
	  if(I2CWriteByte(DS3231_IIC_ADDR, DS3231_DAY, HEX2BCD(dt->byDay)))
	   if(I2CWriteByte(DS3231_IIC_ADDR, DS3231_WEEK, HEX2BCD(byWeek)))
		if(I2CWriteByte(DS3231_IIC_ADDR, DS3231_HOUR, HEX2BCD(tm->byHour)))
	     if(I2CWriteByte(DS3231_IIC_ADDR, DS3231_MINUTE, HEX2BCD(tm->byMinute)))
	      if(I2CWriteByte(DS3231_IIC_ADDR, DS3231_SECOND, HEX2BCD(tm->bySecond)))	   
	       return TRUE;
	 return FALSE;
}	   

BOOL rtc_GetDateTime(PMyDate dt, PMyTime tm, u8 *week)
{
	u16	wYear= BCD2HEX(I2CReadByte(DS3231_IIC_ADDR, DS3231_YEAR));
	if(wYear<13)
		dt->wYear= 2100+wYear;
	else
		dt->wYear= 2000+wYear;
	dt->byMonth= BCD2HEX(I2CReadByte(DS3231_IIC_ADDR, DS3231_MONTH)&0x1F);
	dt->byDay= BCD2HEX(I2CReadByte(DS3231_IIC_ADDR, DS3231_DAY));
	*week= I2CReadByte(DS3231_IIC_ADDR, DS3231_WEEK);
	tm->byHour= BCD2HEX(I2CReadByte(DS3231_IIC_ADDR, DS3231_HOUR)&0x3F);
	tm->byMinute= BCD2HEX(I2CReadByte(DS3231_IIC_ADDR, DS3231_MINUTE));
	tm->bySecond= BCD2HEX(I2CReadByte(DS3231_IIC_ADDR, DS3231_SECOND));
	return TRUE;
}

BOOL rtc_GetDate(PMyDate dt, u8 *week)
{
	u16	wYear= BCD2HEX(I2CReadByte(DS3231_IIC_ADDR, DS3231_YEAR));
	if(wYear<13)
		dt->wYear= 2100+wYear;
	else
		dt->wYear= 2000+wYear;
	dt->byMonth= BCD2HEX(I2CReadByte(DS3231_IIC_ADDR, DS3231_MONTH)&0x1F);
	dt->byDay= BCD2HEX(I2CReadByte(DS3231_IIC_ADDR, DS3231_DAY));
	*week= I2CReadByte(DS3231_IIC_ADDR, DS3231_WEEK);
	return TRUE;
}

BOOL rtc_GetTime(PMyTime tm)//判断秒变化   只调用一次
{
	tm->bySecond= BCD2HEX(I2CReadByte(DS3231_IIC_ADDR, DS3231_SECOND));
	tm->byMinute= BCD2HEX(I2CReadByte(DS3231_IIC_ADDR, DS3231_MINUTE));
	tm->byHour= BCD2HEX(I2CReadByte(DS3231_IIC_ADDR, DS3231_HOUR)&0x3F);
	return TRUE;
}

