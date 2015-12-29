#include "hal.h"

void I2C1_Configuration(void)
{
	I2C_InitTypeDef  I2C_InitStructure;
	// GPIO_InitTypeDef  GPIO_InitStructure; 

	I2C_Cmd(I2C1, DISABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1,ENABLE);

	/* PB6,7 SCL and SDA */
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	I2C_DeInit(I2C1);
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = 0x30; // I2C1_SLAVE_ADDRESS7;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = 50000;//100K速度
    
	I2C_Cmd(I2C1, ENABLE);
	I2C_Init(I2C1, &I2C_InitStructure);
	/*允许1字节1应答模式*/
	I2C_AcknowledgeConfig(I2C1, ENABLE);

}

// ------------------------------------------------------------------------------------------
// **函数名:I2C_ReadBuffer
// **功能:从I2C读取多个字节
// **参数	chipAddr- 芯片地址, subaddr-子地址( 读取开始地址)
// **		addrmode:子地址类型0--无子地址， 1--8位地址，2--16位地址
// ** 返回0--成功, 1--超时, 2--ACK错误
// ------------------------------------------------------------------------------------------
u8 I2C1_ReadBuffer(u8 chipAddr, u16 subaddr , u8 addrmode, u8* pBuffer, u16 no)
{
	u32	dwTimeout= 0xfffff;
	if(no==0)
		return 1;
	
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY)&&(--dwTimeout));
	if(dwTimeout==0)
		return 1;
		
	/*允许1字节1应答模式*/
	I2C_AcknowledgeConfig(I2C1, ENABLE);


	/* 发送起始位 */
	I2C_GenerateSTART(I2C1, ENABLE);
	dwTimeout= 0xfffff;
	while((!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))&&(--dwTimeout));/*EV5,主模式*/
	if(dwTimeout==0)
		return 1;

	/*发送器件地址(写)*/
	I2C_Send7bitAddress(I2C1,  chipAddr, I2C_Direction_Transmitter);
	dwTimeout= 0xfffff;
	while ((!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))&&(--dwTimeout));
	if(dwTimeout==0)
		return 1;

	/*发送地址*/
	if(addrmode)
	{
		if(addrmode>1)
		{
			I2C_SendData(I2C1, subaddr>>8);
			dwTimeout= 0xfffff;
			while ((!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))&&(--dwTimeout));/*数据已发送*/
			if(dwTimeout==0)
				return 1;
		}
		I2C_SendData(I2C1, subaddr&&0xFF);
		dwTimeout= 0xfffff;
		while ((!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))&&(--dwTimeout));/*数据已发送*/
		if(dwTimeout==0)
			return 1;
	}
		
	/*起始位*/
	I2C_GenerateSTART(I2C1, ENABLE);
	dwTimeout= 0xfffff;
	while((!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))&&(--dwTimeout));
	if(dwTimeout==0)
		return 1;
	
	/*器件读*/
	I2C_Send7bitAddress(I2C1, chipAddr, I2C_Direction_Receiver);
	dwTimeout= 0xfffff;
	while((!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))&&(--dwTimeout));
	if(dwTimeout==0)
		return 1;
	
	while (no)
	{
		if(no==1)
		{
     			I2C_AcknowledgeConfig(I2C1, DISABLE);	//最后一位后要关闭应答的
	    		I2C_GenerateSTOP(I2C1, ENABLE);			//发送停止位
		}
	    
		dwTimeout= 0xfffff;
		while((!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED))&&(--dwTimeout)); /* EV7 */
		if(dwTimeout==0)
			return 1;
		*pBuffer = I2C_ReceiveData(I2C1);
		pBuffer++;
		/* Decrement the read bytes counter */
		no--;
	}
	dwTimeout= 0xfffff;
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_STOPF)&&(--dwTimeout));
	//再次允许应答模式
	I2C_AcknowledgeConfig(I2C1, ENABLE);
	if(dwTimeout==0)
		return 1;
	return 0;
}

/****************************************************
**函数名:I2C_Standby
**功能:I2C芯片是否准备好再写入的判断
**注意事项:本函数可以理解为:判忙
** 返回 0--可以操作，其它--超时
****************************************************/
u8 I2C1_Standby(u8 chipAddr)
{
	u32 dwTimeout= 255;
	vu16 SR1_Tmp;
	do
	{
		/*起始位*/
		I2C_GenerateSTART(I2C1, ENABLE);
		/*读SR1*/
		SR1_Tmp = I2C_ReadRegister(I2C1, I2C_Register_SR1);
		/*器件地址(写)*/
		I2C_Send7bitAddress(I2C1, chipAddr, I2C_Direction_Transmitter);
	}while((!(I2C_ReadRegister(I2C1, I2C_Register_SR1) & 0x0002))&&(--dwTimeout));
  	if(dwTimeout==0)
		return 0xFF;
	/**/
	I2C_ClearFlag(I2C1, I2C_FLAG_AF);
	/*停止位*/    
	I2C_GenerateSTOP(I2C1, ENABLE);
	return 0;
}

// ------------------------------------------------------------------------------------------
// **函数名:I2C_WriteBuffer
// **功能:向I2C写入多个字节
// **参数	chipAddr- 芯片地址, subaddr-子地址( 读取开始地址)
// **		addrmode:子地址类型0--无子地址， 1--8位地址，2--16位地址
// ** 返回0--成功, 1--超时, 2--ACK错误
// ------------------------------------------------------------------------------------------
u8 I2C1_WriteBuffer(u8 chipAddr, u8 subaddr, u8 addrmode, u8* pBuffer,  u16 no)
{
	u32	dwTimeout= 0xfffff;
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY)&&(--dwTimeout));
	if(dwTimeout==0)
		return 1;
	/*起始位*/
	I2C_GenerateSTART(I2C1, ENABLE);
	dwTimeout= 0xfffff;
	while((!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))&&(--dwTimeout)); 
	if(dwTimeout==0)
		return 1;

	/*器件地址(写)*/
	I2C_Send7bitAddress(I2C1, chipAddr, I2C_Direction_Transmitter);
	dwTimeout= 0xfffff;
	while((!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))&&(--dwTimeout));
	if(dwTimeout==0)
		return 1;
	
	/*写地址值*/
	if(addrmode)
	{
		if(addrmode>1)
		{
			I2C_SendData(I2C1, subaddr>>8);
			dwTimeout= 0xfffff;
			while ((!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))&&(--dwTimeout));/*数据已发送*/
			if(dwTimeout==0)
				return 1;
		}
		I2C_SendData(I2C1, subaddr&&0xFF);
		dwTimeout= 0xfffff;
		while ((!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))&&(--dwTimeout));/*数据已发送*/
		if(dwTimeout==0)
			return 1;
	}

	while(no--)  
	{
		I2C_SendData(I2C1, *pBuffer); 
		pBuffer++; 
		dwTimeout= 0xfffff;
		while ((!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))&&(--dwTimeout));
		if(dwTimeout==0)
			return 1;
	}
	
	/*停止位*/
	I2C_GenerateSTOP(I2C1, ENABLE);
	dwTimeout= 0xfffff;
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_STOPF)&&(--dwTimeout));
	if(dwTimeout==0)
		return 1;
	return 0;
}

