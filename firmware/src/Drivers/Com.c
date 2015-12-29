#include "hal.h"


void RCC1_Configuration(void)      
{       
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO,ENABLE);       
}     
    
void GPIO1_Configuration(void)      
{      
  GPIO_InitTypeDef GPIO_InitStructure;      
   //usart1   
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;             //�ܽ�9    
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;       //�����������    
  GPIO_Init(GPIOA, &GPIO_InitStructure);                //TX��ʼ��    
      
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;            //�ܽ�10    
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;         //��������    
  GPIO_Init(GPIOA, &GPIO_InitStructure);                //RX��ʼ��    
  GPIO_Init(GPIOA, &GPIO_InitStructure);   
	//usart2
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;             //�ܽ�2    
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;       //�����������    
  GPIO_Init(GPIOA, &GPIO_InitStructure);                //TX��ʼ��    
      
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;            //�ܽ�3    
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;         //��������    
  GPIO_Init(GPIOA, &GPIO_InitStructure);                //RX��ʼ��    
}  
void USART1_Configuration(void)//���ڳ�ʼ������    
{    
    //���ڲ�����ʼ��      
    USART_InitTypeDef USART_InitStructure;               //�������ûָ�Ĭ�ϲ���    
    //��ʼ����������    
    USART_InitStructure.USART_BaudRate = 9600;                  //������9600    
    USART_InitStructure.USART_WordLength = USART_WordLength_8b; //�ֳ�8λ    
    USART_InitStructure.USART_StopBits = USART_StopBits_1;      //1λֹͣ�ֽ�    
    USART_InitStructure.USART_Parity = USART_Parity_No;         //����żУ��    
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��������    
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//��Rx���պ�Tx���͹���    
    USART_Init(USART1,&USART_InitStructure);                   //��ʼ��    
    USART_Cmd(USART1,ENABLE);                                  //��������    
}  

void USART2_NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Configure the NVIC Preemption Priority Bits */  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

	/* Enable the USARTy Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void USART2_Configuration(void)//���ڳ�ʼ������    
{    
    //���ڲ�����ʼ��      
    USART_InitTypeDef USART_InitStructure;               //�������ûָ�Ĭ�ϲ���    
    //��ʼ����������    
    USART_InitStructure.USART_BaudRate = 9600;                  //������9600    
    USART_InitStructure.USART_WordLength = USART_WordLength_8b; //�ֳ�8λ    
    USART_InitStructure.USART_StopBits = USART_StopBits_1;      //1λֹͣ�ֽ�    
    USART_InitStructure.USART_Parity = USART_Parity_No;         //����żУ��    
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��������    
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//��Rx���պ�Tx���͹���    
    USART_Init(USART2,&USART_InitStructure);                   //��ʼ��    
		USART_ITConfig(USART2, USART_IT_RXNE , ENABLE);
    USART_Cmd(USART2,ENABLE);                                  //��������     
}  

void Com_Init(void)
{
	  RCC1_Configuration();    
    GPIO1_Configuration();     
    USART1_Configuration(); 
		USART2_Configuration();
		USART2_NVIC_Configuration();
}

void UART_PutChar(USART_TypeDef* USARTx,uint8_t m_Data)
{  
	USARTx->DR = (((m_Data)&0xFF) & (uint16_t)0x01FF);
	/* Loop until the end of transmission */
	while (USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET)
	{} 
}
 
void UART_PutStr(USART_TypeDef* USARTx,uint8_t *pData,uint32_t Len)
{
	while(Len--)
	{
		UART_PutChar(USARTx,*pData++);
	}
}

