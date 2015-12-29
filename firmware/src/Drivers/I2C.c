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
	I2C_InitStructure.I2C_ClockSpeed = 50000;//100K�ٶ�
    
	I2C_Cmd(I2C1, ENABLE);
	I2C_Init(I2C1, &I2C_InitStructure);
	/*����1�ֽ�1Ӧ��ģʽ*/
	I2C_AcknowledgeConfig(I2C1, ENABLE);

}

// ------------------------------------------------------------------------------------------
// **������:I2C_ReadBuffer
// **����:��I2C��ȡ����ֽ�
// **����	chipAddr- оƬ��ַ, subaddr-�ӵ�ַ( ��ȡ��ʼ��ַ)
// **		addrmode:�ӵ�ַ����0--���ӵ�ַ�� 1--8λ��ַ��2--16λ��ַ
// ** ����0--�ɹ�, 1--��ʱ, 2--ACK����
// ------------------------------------------------------------------------------------------
u8 I2C1_ReadBuffer(u8 chipAddr, u16 subaddr , u8 addrmode, u8* pBuffer, u16 no)
{
	u32	dwTimeout= 0xfffff;
	if(no==0)
		return 1;
	
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY)&&(--dwTimeout));
	if(dwTimeout==0)
		return 1;
		
	/*����1�ֽ�1Ӧ��ģʽ*/
	I2C_AcknowledgeConfig(I2C1, ENABLE);


	/* ������ʼλ */
	I2C_GenerateSTART(I2C1, ENABLE);
	dwTimeout= 0xfffff;
	while((!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))&&(--dwTimeout));/*EV5,��ģʽ*/
	if(dwTimeout==0)
		return 1;

	/*����������ַ(д)*/
	I2C_Send7bitAddress(I2C1,  chipAddr, I2C_Direction_Transmitter);
	dwTimeout= 0xfffff;
	while ((!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))&&(--dwTimeout));
	if(dwTimeout==0)
		return 1;

	/*���͵�ַ*/
	if(addrmode)
	{
		if(addrmode>1)
		{
			I2C_SendData(I2C1, subaddr>>8);
			dwTimeout= 0xfffff;
			while ((!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))&&(--dwTimeout));/*�����ѷ���*/
			if(dwTimeout==0)
				return 1;
		}
		I2C_SendData(I2C1, subaddr&&0xFF);
		dwTimeout= 0xfffff;
		while ((!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))&&(--dwTimeout));/*�����ѷ���*/
		if(dwTimeout==0)
			return 1;
	}
		
	/*��ʼλ*/
	I2C_GenerateSTART(I2C1, ENABLE);
	dwTimeout= 0xfffff;
	while((!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))&&(--dwTimeout));
	if(dwTimeout==0)
		return 1;
	
	/*������*/
	I2C_Send7bitAddress(I2C1, chipAddr, I2C_Direction_Receiver);
	dwTimeout= 0xfffff;
	while((!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))&&(--dwTimeout));
	if(dwTimeout==0)
		return 1;
	
	while (no)
	{
		if(no==1)
		{
     			I2C_AcknowledgeConfig(I2C1, DISABLE);	//���һλ��Ҫ�ر�Ӧ���
	    		I2C_GenerateSTOP(I2C1, ENABLE);			//����ֹͣλ
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
	//�ٴ�����Ӧ��ģʽ
	I2C_AcknowledgeConfig(I2C1, ENABLE);
	if(dwTimeout==0)
		return 1;
	return 0;
}

/****************************************************
**������:I2C_Standby
**����:I2CоƬ�Ƿ�׼������д����ж�
**ע������:�������������Ϊ:��æ
** ���� 0--���Բ���������--��ʱ
****************************************************/
u8 I2C1_Standby(u8 chipAddr)
{
	u32 dwTimeout= 255;
	vu16 SR1_Tmp;
	do
	{
		/*��ʼλ*/
		I2C_GenerateSTART(I2C1, ENABLE);
		/*��SR1*/
		SR1_Tmp = I2C_ReadRegister(I2C1, I2C_Register_SR1);
		/*������ַ(д)*/
		I2C_Send7bitAddress(I2C1, chipAddr, I2C_Direction_Transmitter);
	}while((!(I2C_ReadRegister(I2C1, I2C_Register_SR1) & 0x0002))&&(--dwTimeout));
  	if(dwTimeout==0)
		return 0xFF;
	/**/
	I2C_ClearFlag(I2C1, I2C_FLAG_AF);
	/*ֹͣλ*/    
	I2C_GenerateSTOP(I2C1, ENABLE);
	return 0;
}

// ------------------------------------------------------------------------------------------
// **������:I2C_WriteBuffer
// **����:��I2Cд�����ֽ�
// **����	chipAddr- оƬ��ַ, subaddr-�ӵ�ַ( ��ȡ��ʼ��ַ)
// **		addrmode:�ӵ�ַ����0--���ӵ�ַ�� 1--8λ��ַ��2--16λ��ַ
// ** ����0--�ɹ�, 1--��ʱ, 2--ACK����
// ------------------------------------------------------------------------------------------
u8 I2C1_WriteBuffer(u8 chipAddr, u8 subaddr, u8 addrmode, u8* pBuffer,  u16 no)
{
	u32	dwTimeout= 0xfffff;
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY)&&(--dwTimeout));
	if(dwTimeout==0)
		return 1;
	/*��ʼλ*/
	I2C_GenerateSTART(I2C1, ENABLE);
	dwTimeout= 0xfffff;
	while((!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))&&(--dwTimeout)); 
	if(dwTimeout==0)
		return 1;

	/*������ַ(д)*/
	I2C_Send7bitAddress(I2C1, chipAddr, I2C_Direction_Transmitter);
	dwTimeout= 0xfffff;
	while((!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))&&(--dwTimeout));
	if(dwTimeout==0)
		return 1;
	
	/*д��ֵַ*/
	if(addrmode)
	{
		if(addrmode>1)
		{
			I2C_SendData(I2C1, subaddr>>8);
			dwTimeout= 0xfffff;
			while ((!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))&&(--dwTimeout));/*�����ѷ���*/
			if(dwTimeout==0)
				return 1;
		}
		I2C_SendData(I2C1, subaddr&&0xFF);
		dwTimeout= 0xfffff;
		while ((!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))&&(--dwTimeout));/*�����ѷ���*/
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
	
	/*ֹͣλ*/
	I2C_GenerateSTOP(I2C1, ENABLE);
	dwTimeout= 0xfffff;
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_STOPF)&&(--dwTimeout));
	if(dwTimeout==0)
		return 1;
	return 0;
}

