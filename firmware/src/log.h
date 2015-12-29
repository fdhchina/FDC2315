#ifndef _LOG_H
#define _LOG_H
#include "stm32f10x.h"
#include "ff.h"
#define LOG_PATH "0:\\fdc2315.log"
void Log_Init(void);
void Log_write(char *pdt, u32 length);
void Log_test(void);
#endif
