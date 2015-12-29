#ifndef __COM_H
#define __COM_H

#ifdef __cplusplus
 extern "C" {
#endif 	 

#include <stdio.h>

void Com_Init(void); 
void UART_PutChar(USART_TypeDef* USARTx, uint8_t Data);  
void UART_PutStr(USART_TypeDef* USARTx,uint8_t *pData,uint32_t Len);

#ifdef __cplusplus
}
#endif

#endif


