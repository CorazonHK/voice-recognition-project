#include "stm32f10x.h"                  // Device header
#include "AD.h"                          // 包含AD.h，以便使用AD_Value数组

/**
  * 功    能：DMA初始化（用于ADC音频数据转运）
  * 说    明：从ADC数据寄存器转运到AD_Value数组，循环模式
  * 参    数：无
  * 返 回 值：无
  */
void MyDMA_Init(void)
{
	/*开启时钟*/
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//开启DMA1的时钟
	
	/*DMA初始化*/
	DMA_InitTypeDef DMA_InitStructure;
	
	// 外设地址：ADC1的数据寄存器（固定不变）
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	// 半字，对应16位ADC数据
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;				// 外设地址不自增
	
	// 存储器地址：AD_Value数组（需要自增）
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)AD_Value;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;			// 半字，与ADC匹配
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;						// 存储器地址自增
	
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;			// 外设到存储器
	DMA_InitStructure.DMA_BufferSize = 512;						// 转运512个点（与AD_Value数组大小一致）
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;			// 循环模式，用于连续音频采集
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;				// 由ADC触发，不是存储器到存储器
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;		// 优先级中等
	
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);				// 配置DMA1通道1
	
	/*DMA使能*/
	DMA_Cmd(DMA1_Channel1, ENABLE);		// 使能DMA，开始转运
}
