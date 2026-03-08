#include "stm32f10x.h"
#include "OLED_Font.h"

// 声明延时函数
void Delay_us(uint32_t xus);
void Delay_ms(uint32_t xms);

/*引脚配置*/
#define OLED_W_SCL(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_6, (BitAction)(x))
#define OLED_W_SDA(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_7, (BitAction)(x))

/*引脚初始化*/
void OLED_I2C_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 推挽输出
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // 初始化为高电平
    GPIO_SetBits(GPIOB, GPIO_Pin_6 | GPIO_Pin_7);
}

/**
  * @brief  I2C开始
  */
void OLED_I2C_Start(void)
{
    OLED_W_SDA(1);
    OLED_W_SCL(1);
    Delay_us(5);        // 建立时间
    OLED_W_SDA(0);
    Delay_us(5);        // 保持时间
    OLED_W_SCL(0);
    Delay_us(5);        // 准备发送数据
}

/**
  * @brief  I2C停止
  */
void OLED_I2C_Stop(void)
{
    OLED_W_SDA(0);
    OLED_W_SCL(1);
    Delay_us(5);        // 建立时间
    OLED_W_SDA(1);
    Delay_us(5);        // 停止条件保持
}

/**
  * @brief  I2C发送一个字节
  */
void OLED_I2C_SendByte(uint8_t Byte)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        // 设置数据
        if (Byte & (0x80 >> i))
            OLED_W_SDA(1);
        else
            OLED_W_SDA(0);
        Delay_us(2);     // 数据建立时间
        
        // SCL高电平（数据被读取）
        OLED_W_SCL(1);
        Delay_us(5);     // 时钟高电平宽度
        
        // SCL低电平
        OLED_W_SCL(0);
        Delay_us(5);     // 时钟低电平宽度
    }
    
    // 额外的一个时钟，不处理应答信号
    OLED_W_SCL(1);
    Delay_us(5);
    OLED_W_SCL(0);
    Delay_us(2);
}

/**
  * @brief  OLED写命令
  */
void OLED_WriteCommand(uint8_t Command)
{
    OLED_I2C_Start();
    Delay_us(2);
    
    OLED_I2C_SendByte(0x78);		//从机地址 (改回0x78)
    Delay_us(2);
    
    OLED_I2C_SendByte(0x00);		//写命令
    Delay_us(2);
    
    OLED_I2C_SendByte(Command); 
    Delay_us(2);
    
    OLED_I2C_Stop();
    Delay_us(10);                   // 命令处理时间
}

/**
  * @brief  OLED写数据
  */
void OLED_WriteData(uint8_t Data)
{
    OLED_I2C_Start();
    Delay_us(2);
    
    OLED_I2C_SendByte(0x78);		//从机地址 (改回0x78)
    Delay_us(2);
    
    OLED_I2C_SendByte(0x40);		//写数据
    Delay_us(2);
    
    OLED_I2C_SendByte(Data);
    Delay_us(2);
    
    OLED_I2C_Stop();
    Delay_us(5);
}

/**
  * @brief  OLED设置光标位置
  */
void OLED_SetCursor(uint8_t Y, uint8_t X)
{
	OLED_WriteCommand(0xB0 | Y);
	OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));
	OLED_WriteCommand(0x00 | (X & 0x0F));
}

/**
  * @brief  OLED清屏
  */
void OLED_Clear(void)
{  
	uint8_t i, j;
	for (j = 0; j < 8; j++)
	{
		OLED_SetCursor(j, 0);
		for(i = 0; i < 128; i++)
		{
			OLED_WriteData(0x00);
		}
	}
}

/**
  * @brief  OLED显示一个字符
  */
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{      	
	uint8_t i;
	OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);
	for (i = 0; i < 8; i++)
	{
		OLED_WriteData(OLED_F8x16[Char - ' '][i]);
	}
	OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);
	for (i = 0; i < 8; i++)
	{
		OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]);
	}
}

/**
  * @brief  OLED显示字符串
  */
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i++)
	{
		OLED_ShowChar(Line, Column + i, String[i]);
	}
}

/**
  * @brief  OLED次方函数
  */
uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while (Y--)
	{
		Result *= X;
	}
	return Result;
}

/**
  * @brief  OLED显示数字（十进制，正数）
  */
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i++)							
	{
		OLED_ShowChar(Line, Column + i, Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
	}
}

/**
  * @brief  OLED显示数字（十进制，带符号数）
  */
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length)
{
	uint8_t i;
	uint32_t Number1;
	if (Number >= 0)
	{
		OLED_ShowChar(Line, Column, '+');
		Number1 = Number;
	}
	else
	{
		OLED_ShowChar(Line, Column, '-');
		Number1 = -Number;
	}
	for (i = 0; i < Length; i++)							
	{
		OLED_ShowChar(Line, Column + i + 1, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0');
	}
}

/**
  * @brief  OLED显示数字（十六进制，正数）
  */
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i, SingleNumber;
	for (i = 0; i < Length; i++)							
	{
		SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
		if (SingleNumber < 10)
		{
			OLED_ShowChar(Line, Column + i, SingleNumber + '0');
		}
		else
		{
			OLED_ShowChar(Line, Column + i, SingleNumber - 10 + 'A');
		}
	}
}

/**
  * @brief  OLED显示数字（二进制，正数）
  */
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i++)							
	{
		OLED_ShowChar(Line, Column + i, Number / OLED_Pow(2, Length - i - 1) % 2 + '0');
	}
}

/**
  * @brief  OLED初始化
  */
void OLED_Init(void)
{
	uint32_t i, j;
	
	// 上电延时
	for (i = 0; i < 1000; i++)			
	{
		for (j = 0; j < 1000; j++);
	}
	Delay_ms(100);
	
	OLED_I2C_Init();
	
	// 完整的初始化序列
	OLED_WriteCommand(0xAE);	//关闭显示
	Delay_ms(10);
	
	OLED_WriteCommand(0xD5);	//设置显示时钟分频比
	OLED_WriteCommand(0x80);
	Delay_ms(10);
	
	OLED_WriteCommand(0xA8);	//设置多路复用率
	OLED_WriteCommand(0x3F);
	Delay_ms(10);
	
	OLED_WriteCommand(0xD3);	//设置显示偏移
	OLED_WriteCommand(0x00);
	Delay_ms(10);
	
	OLED_WriteCommand(0x40);	//设置显示开始行
	Delay_ms(10);
	
	OLED_WriteCommand(0x8D);	//设置充电泵
	OLED_WriteCommand(0x14);	//开启充电泵
	Delay_ms(10);
	
	OLED_WriteCommand(0x20);	//设置内存寻址模式
	OLED_WriteCommand(0x00);	//水平寻址模式
	Delay_ms(10);
	
	OLED_WriteCommand(0xA1);	//设置左右方向（正常）
	Delay_ms(10);
	
	OLED_WriteCommand(0xC8);	//设置上下方向（正常）
	Delay_ms(10);
	
	OLED_WriteCommand(0xDA);	//设置COM引脚硬件配置
	OLED_WriteCommand(0x12);
	Delay_ms(10);
	
	OLED_WriteCommand(0x81);	//设置对比度
	OLED_WriteCommand(0xCF);
	Delay_ms(10);
	
	OLED_WriteCommand(0xD9);	//设置预充电周期
	OLED_WriteCommand(0xF1);
	Delay_ms(10);
	
	OLED_WriteCommand(0xDB);	//设置VCOMH
	OLED_WriteCommand(0x30);
	Delay_ms(10);
	
	OLED_WriteCommand(0xA4);	//设置整个显示打开/关闭
	Delay_ms(10);
	
	OLED_WriteCommand(0xA6);	//设置正常/倒转显示
	Delay_ms(10);
	
	OLED_WriteCommand(0xAF);	//开启显示
	Delay_ms(50);
	
	OLED_Clear();
	
	// 显示测试字符
	OLED_ShowString(1, 1, "Hello");
	OLED_ShowString(2, 1, "STM32");
	OLED_ShowString(3, 1, "OLED OK");
}
