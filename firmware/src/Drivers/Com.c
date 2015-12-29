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
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;             //管脚9    
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;       //复用推挽输出    
  GPIO_Init(GPIOA, &GPIO_InitStructure);                //TX初始化    
      
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;            //管脚10    
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;         //上拉输入    
  GPIO_Init(GPIOA, &GPIO_InitStructure);                //RX初始化    
  GPIO_Init(GPIOA, &GPIO_InitStructure);   
	//usart2
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;             //管脚2    
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;       //复用推挽输出    
  GPIO_Init(GPIOA, &GPIO_InitStructure);                //TX初始化    
      
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;            //管脚3    
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;         //上拉输入    
  GPIO_Init(GPIOA, &GPIO_InitStructure);                //RX初始化    
}  
void USART1_Configuration(void)//串口初始化函数    
{    
    //串口参数初始化      
    USART_InitTypeDef USART_InitStructure;               //串口设置恢复默认参数    
    //初始化参数设置    
    USART_InitStructure.USART_BaudRate = 9600;                  //波特率9600    
    USART_InitStructure.USART_WordLength = USART_WordLength_8b; //字长8位    
    USART_InitStructure.USART_StopBits = USART_StopBits_1;      //1位停止字节    
    USART_InitStructure.USART_Parity = USART_Parity_No;         //无奇偶校验    
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无流控制    
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//打开Rx接收和Tx发送功能    
    USART_Init(USART1,&USART_InitStructure);                   //初始化    
    USART_Cmd(USART1,ENABLE);                                  //启动串口    
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

void USART2_Configuration(void)//串口初始化函数    
{    
    //串口参数初始化      
    USART_InitTypeDef USART_InitStructure;               //串口设置恢复默认参数    
    //初始化参数设置    
    USART_InitStructure.USART_BaudRate = 9600;                  //波特率9600    
    USART_InitStructure.USART_WordLength = USART_WordLength_8b; //字长8位    
    USART_InitStructure.USART_StopBits = USART_StopBits_1;      //1位停止字节    
    USART_InitStructure.USART_Parity = USART_Parity_No;         //无奇偶校验    
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无流控制    
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//打开Rx接收和Tx发送功能    
    USART_Init(USART2,&USART_InitStructure);                   //初始化    
		USART_ITConfig(USART2, USART_IT_RXNE , ENABLE);
    USART_Cmd(USART2,ENABLE);                                  //启动串口     
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

