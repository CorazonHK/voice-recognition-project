#include "usart.h"
#include "stm32f10x.h"
#include<string.h>

/*******************************************************************************
* 函数名    : uart3_init
* 描述      : 初始化 USART3，只用于发送（也可接收）
* 输入      : bound - 波特率（如 115200）
* 输出      : 无
* 调用      : 外部调用
*******************************************************************************/
void uart3_init(u32 bound)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    // 1. 使能时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);      // GPIOB 时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);    // USART3 时钟

    // 2. 配置 TX 引脚 (PB10)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;            // 复用推挽输出
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 3. 配置 RX 引脚 (PB11)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;      // 浮空输入
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 4. USART 基本配置
    USART_InitStructure.USART_BaudRate = bound;                // 波特率
    USART_InitStructure.USART_WordLength = USART_WordLength_8b; // 8位数据
    USART_InitStructure.USART_StopBits = USART_StopBits_1;      // 1位停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;        // 无校验
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无流控
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; // 收发均使能
    USART_Init(USART3, &USART_InitStructure);

    // 5. 注意：此处不开启接收中断，避免进入空中断导致死机
    // USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

    // 6. 使能 USART3
    USART_Cmd(USART3, ENABLE);
}

/*******************************************************************************
* 函数名    : USART3_Send_Data
* 描述      : 发送一个字节数据
* 输入      : data - 要发送的数据（8位）
* 输出      : 无
* 调用      : 外部调用
*******************************************************************************/
void USART3_Send_Data(uint8_t data)
{
    // 等待发送数据寄存器空
    while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
    USART_SendData(USART3, data);
    
    // 可选：等待发送完成（如果需要确保数据完全发出）
    // while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
}

/*******************************************************************************
* 函数名    : USART3_Send_String
* 描述      : 发送字符串
* 输入      : str - 以 '\0' 结尾的字符串指针
* 输出      : 无
* 调用      : 外部调用
*******************************************************************************/
void USART3_Send_String(uint8_t *str)
{
    while (*str)
    {
        USART3_Send_Data(*str++);
    }
}

