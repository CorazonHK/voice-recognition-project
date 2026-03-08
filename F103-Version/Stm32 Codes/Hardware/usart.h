#ifndef __USART_H
#define __USART_H

#include "stm32f10x.h"
#include <string.h>

// 函数声明（和 usart.c 完全一致）
void uart3_init(u32 bound);
void USART3_Send_Data(uint8_t data);
void USART3_Send_String(uint8_t *str);

#endif
