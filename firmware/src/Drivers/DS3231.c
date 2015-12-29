#include "hal.h"
#include "task.h"
#include  "ds3231.h"				

#define DS3231_IIC_ADDR 0xd0
/********************************************************************************************************
** 	ȫ�ֳ�������
********************************************************************************************************/
/********************************************************************************************************
** 	DS3231��������
********************************************************************************************************/
#define DS3231_WriteAddress 0xD0    //����д��ַ 
#define DS3231_ReadAddress  0xD1    //��������ַ

#define DS3231_SECOND       0x00    //��
#define DS3231_MINUTE       0x01    //��
#define DS3231_HOUR         0x02    //ʱ
#define DS3231_WEEK         0x03    //����
#define DS3231_DAY          0x04    //��
#define DS3231_MONTH        0x05    //��
#define DS3231_YEAR         0x06    //��
//����1            
#define DS3231_ALARM1SECOND 0x07    //��
#define DS3231_ALARM1MINUTE 0x08    //��
#define DS3231_ALARM1HOUR   0x09    //ʱ
#define DS3231_ALARM1WEEK   0x0A    //����/��
//����2
#define DS3231_ALARM2MINUTE 0x0b    //��
#define DS3231_ALARM2HOUR   0x0c    //ʱ
#define DS3231_ALARM2WEEK   0x0d    //����/��

#define DS3231_CONTROL      0x0e    //���ƼĴ���
#define DS3231_STATUS       0x0f    //״̬�Ĵ���
#define BSY                 2       //æ
#define OSF                 7       //����ֹͣ��־
#define DS3231_XTAL         0x10    //�����ϻ��Ĵ���
#define DS3231_TEMPERATUREH 0x11    //�¶ȼĴ������ֽ�(8λ)
#define DS3231_TEMPERATUREL 0x12    //�¶ȼĴ������ֽ�(��2λ)  	

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

BOOL rtc_GetTime(PMyTime tm)//�ж���仯   ֻ����һ��
{
	tm->bySecond= BCD2HEX(I2CReadByte(DS3231_IIC_ADDR, DS3231_SECOND));
	tm->byMinute= BCD2HEX(I2CReadByte(DS3231_IIC_ADDR, DS3231_MINUTE));
	tm->byHour= BCD2HEX(I2CReadByte(DS3231_IIC_ADDR, DS3231_HOUR)&0x3F);
	return TRUE;
}

