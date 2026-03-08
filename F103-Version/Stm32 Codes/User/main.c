#include "stm32f10x.h"
#include "usart.h"
#include "mfcc.h"
#include "arm_math.h"
#include "OLED.h"
#include "AD.h"
#include "MyDMA.h"

// ========== 全局变量 ==========
MFCC_HandleTypeDef hmfcc;
int16_t mfcc_int[MFCC_COEFFS];
float32_t mfcc_float[MFCC_COEFFS];
uint16_t frame_count = 0;

// 简单延时函数
void delay_ms(u32 ms)
{
    for(u32 i = 0; i < ms * 4000; i++);
}

// 获取音频帧
void get_audio_frame(int16_t *frame)
{
    for(uint16_t i = 0; i < MFCC_FRAME_LEN; i++) {
        frame[i] = (int16_t)AD_Value[i];
    }
}

// ========== OLED显示辅助函数（手动转3位数字）==========
void OLED_Show3Digit(uint8_t line, uint8_t col, uint16_t num)
{
    char buf[4];
    buf[0] = '0' + (num / 100) % 10;
    buf[1] = '0' + (num / 10) % 10;
    buf[2] = '0' + num % 10;
    buf[3] = '\0';
    OLED_ShowString(line, col, buf);
}

void OLED_ShowSigned3Digit(uint8_t line, uint8_t col, int16_t num)
{
    char buf[5];
    uint8_t i = 0;
    
    if(num < 0) {
        buf[i++] = '-';
        num = -num;
    } else {
        buf[i++] = ' ';
    }
    
    buf[i++] = '0' + (num / 100) % 10;
    buf[i++] = '0' + (num / 10) % 10;
    buf[i++] = '0' + num % 10;
    buf[i] = '\0';
    
    OLED_ShowString(line, col, buf);
}

// ========== OLED显示 ==========
void update_oled_display(void)
{
    uint16_t vol = AD_Value[0] >> 4;  // 音量值 0-255
    
    // 第1行：帧计数 F:001
    OLED_ShowString(1, 1, "F:");
    OLED_Show3Digit(1, 4, frame_count);
    
    // 第2行：C0系数 0: 123
    OLED_ShowString(2, 1, "0:");
    OLED_ShowSigned3Digit(2, 4, mfcc_int[0]);
    
    // 第3行：音量 V:123
    OLED_ShowString(3, 1, "V:");
    OLED_Show3Digit(3, 4, vol);
}

int main(void)
{
    uint16_t i;
    int16_t audio_frame[MFCC_FRAME_LEN];
    
    // 初始化
    OLED_Init();                // OLED初始化
    uart3_init(9600);           // 蓝牙串口 9600
    AD_Init();                  // ADC初始化
    MyDMA_Init();               // DMA初始化
    MFCC_Init(&hmfcc);          // MFCC初始化
    
    // 清屏显示启动信息
    OLED_Clear();
    OLED_ShowString(1, 1, "MFCC Ready");
    delay_ms(500);
    OLED_Clear();
    
    while(1)
    {
        // 获取音频帧
        get_audio_frame(audio_frame);
        
        // 计算MFCC
        MFCC_ComputeFrame(&hmfcc, audio_frame, mfcc_float);
        MFCC_Quantize(mfcc_float, mfcc_int, 3);
        
        // 发送数据
        USART3_Send_Data(0xAA);
        USART3_Send_Data(0x55);
        for(i = 0; i < MFCC_COEFFS; i++) {
            USART3_Send_Data((uint8_t)(mfcc_int[i] >> 8));
            USART3_Send_Data((uint8_t)(mfcc_int[i] & 0xFF));
        }
        USART3_Send_Data(0x0D);
        USART3_Send_Data(0x0A);
        
        // 更新显示
        frame_count++;
        update_oled_display();
        
        // 8ms一帧
        delay_ms(8);
    }
}

        
