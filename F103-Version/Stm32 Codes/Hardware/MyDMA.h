#ifndef __MYDMA_H
#define __MYDMA_H

#include "stm32f10x.h"      // 包含标准库头文件，确保uint32_t等类型被定义

/**
  * 功    能：DMA初始化（用于ADC音频数据转运）
  * 说    明：从ADC数据寄存器转运到AD_Value数组，循环模式
  * 参    数：无
  * 返 回 值：无
  */
void MyDMA_Init(void);

#endif
