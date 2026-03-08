#include "mfcc.h"
#include <string.h>
#include <stdint.h>
#include <arm_math.h>
#include <math.h>  // 用于 log10f, powf, cosf, logf


// ========== 手动实现的sin/cos近似 ==========
static float32_t my_cos(float32_t x)
{
    float32_t x2 = x * x;
    return 1.0f - x2/2.0f + x2*x2/24.0f;
}

static float32_t my_sin(float32_t x)
{
    float32_t x2 = x * x;
    return x - x * x2 / 6.0f;
}

// ========== 手动实现的128点FFT ==========
static void simple_fft_128(float32_t *data)
{
    uint16_t n = 128;
    uint16_t i, j, k, m;
    float32_t t1, t2;
    float32_t u_re, u_im, t_re, t_im, w_re, w_im;
    
    // 位反转排序
    j = 0;
    for(i = 0; i < n-1; i++) {
        if(i < j) {
            t1 = data[2*i]; t2 = data[2*i+1];
            data[2*i] = data[2*j]; data[2*i+1] = data[2*j+1];
            data[2*j] = t1; data[2*j+1] = t2;
        }
        k = n/2;
        while(k <= j) {
            j -= k;
            k /= 2;
        }
        j += k;
    }
    
    // 蝶形运算
    for(m = 1; m < n; m <<= 1) {
        for(i = 0; i < n; i += (m << 1)) {
            for(j = 0; j < m; j++) {
                // 计算旋转因子
                float32_t angle = 2.0f * PI * j / (m << 1);
                w_re = my_cos(angle);
                w_im = -my_sin(angle);
                
                uint16_t idx1 = (i + j) << 1;
                uint16_t idx2 = (i + j + m) << 1;
                
                u_re = data[idx2];
                u_im = data[idx2+1];
                
                t_re = u_re * w_re - u_im * w_im;
                t_im = u_re * w_im + u_im * w_re;
                
                data[idx2] = data[idx1] - t_re;
                data[idx2+1] = data[idx1+1] - t_im;
                data[idx1] = data[idx1] + t_re;
                data[idx1+1] = data[idx1+1] + t_im;
            }
        }
    }
}

// ========== 静态函数声明 ==========
static void create_hamming_window(float32_t *window, uint16_t len);
static void create_mel_filterbank(float32_t *filters, uint16_t fft_size, uint16_t num_filters, uint32_t sample_rate);

// ========== 全局常量 ==========
static float32_t hamming_window[MFCC_FRAME_LEN];
static float32_t mel_filters[MFCC_NUM_FILTERS * (MFCC_FFT_SIZE/2 + 1)];

// ========== 初始化函数 ==========
void MFCC_Init(MFCC_HandleTypeDef *hmfcc)
{
    static float32_t fft_buffer[MFCC_FFT_SIZE];
    static float32_t mfcc_out[MFCC_COEFFS];
    
    create_hamming_window(hamming_window, MFCC_FRAME_LEN);
    create_mel_filterbank(mel_filters, MFCC_FFT_SIZE, MFCC_NUM_FILTERS, MFCC_SAMPLE_RATE);
    
    hmfcc->hamming_window = hamming_window;
    hmfcc->mel_filters = mel_filters;
    hmfcc->fft_buffer = fft_buffer;
    hmfcc->mfcc_out = mfcc_out;
    hmfcc->frame_count = 0;
}

// ========== 汉明窗生成 ==========
static void create_hamming_window(float32_t *window, uint16_t len)
{
    for(uint16_t i = 0; i < len; i++) {
        float32_t x = (2.0f * PI * i) / (len - 1);
        float32_t x2 = x * x;
        float32_t cos_x = 1.0f - x2/2.0f + x2*x2/24.0f;
        window[i] = 0.54f - 0.46f * cos_x;
    }
}

// ========== 计算一帧MFCC ==========
void MFCC_ComputeFrame(MFCC_HandleTypeDef *hmfcc, int16_t *pcm_frame, float32_t *mfcc_out)
{
    uint16_t i, j;
    
    // ----- 1. 加窗 -----
    for(i = 0; i < MFCC_FRAME_LEN; i++) {
        hmfcc->fft_buffer[i] = (float32_t)pcm_frame[i] * hmfcc->hamming_window[i];
    }
    // 补零
    for(; i < MFCC_FFT_SIZE; i++) {
        hmfcc->fft_buffer[i] = 0.0f;
    }
    
    // ----- 2. 手动FFT -----
    simple_fft_128(hmfcc->fft_buffer);
    
    // ----- 3. 计算功率谱 -----
    float32_t power_spec[MFCC_FFT_SIZE/2 + 1];
    for(i = 0; i <= MFCC_FFT_SIZE/2; i++) {
        float32_t real = hmfcc->fft_buffer[2*i];
        float32_t imag = hmfcc->fft_buffer[2*i + 1];
        power_spec[i] = real*real + imag*imag;
    }
    
    // ----- 4. Mel滤波 -----
    float32_t mel_energy[MFCC_NUM_FILTERS] = {0};
    for(i = 0; i < MFCC_NUM_FILTERS; i++) {
        const float32_t *filter = &hmfcc->mel_filters[i * (MFCC_FFT_SIZE/2 + 1)];
        for(j = 0; j <= MFCC_FFT_SIZE/2; j++) {
            mel_energy[i] += power_spec[j] * filter[j];
        }
        // 取对数（避免log0）
        mel_energy[i] = logf(mel_energy[i] + 1e-10f);
    }
    
    // ----- 5. DCT得到MFCC（手动实现）-----
    // 只取前MFCC_COEFFS维（通常是13）
    for(i = 0; i < MFCC_COEFFS; i++) {
        float32_t sum = 0.0f;
        for(j = 0; j < MFCC_NUM_FILTERS; j++) {
            sum += mel_energy[j] * cosf(i * (j + 0.5f) * PI / MFCC_NUM_FILTERS);
        }
        // 归一化系数（和librosa保持一致）
        if(i == 0) {
            mfcc_out[i] = sum * sqrtf(1.0f / MFCC_NUM_FILTERS);
        } else {
            mfcc_out[i] = sum * sqrtf(2.0f / MFCC_NUM_FILTERS);
        }
    }
    
    hmfcc->frame_count++;
}

// ========== 量化 ==========
void MFCC_Quantize(float32_t *mfcc_float, int16_t *mfcc_int, uint8_t shift)
{
    for(uint8_t i = 0; i < MFCC_COEFFS; i++) {
        mfcc_int[i] = (int16_t)(mfcc_float[i] * (1 << shift));
    }
}

// ========== Mel滤波器组生成（完整版）==========
static void create_mel_filterbank(float32_t *filters, uint16_t fft_size, uint16_t num_filters, uint32_t sample_rate)
{
    uint16_t i, j;
    uint16_t fft_bin = fft_size / 2 + 1;  // 正频率点数
    float32_t mel_low = 0.0f;
    float32_t mel_high = 2595.0f * log10f(1.0f + (sample_rate / 2.0f) / 700.0f);
    float32_t mel_step = (mel_high - mel_low) / (num_filters + 1);
    
    // 计算每个滤波器的起始、中心、结束点（Mel刻度）
    float32_t mel_points[num_filters + 2];
    for(i = 0; i < num_filters + 2; i++) {
        mel_points[i] = mel_low + i * mel_step;
    }
    
    // 将Mel刻度转换为频率
    float32_t freq_points[num_filters + 2];
    for(i = 0; i < num_filters + 2; i++) {
        freq_points[i] = 700.0f * (powf(10.0f, mel_points[i] / 2595.0f) - 1.0f);
    }
    
    // 将频率转换为FFT bin索引
    uint16_t bin_points[num_filters + 2];
    for(i = 0; i < num_filters + 2; i++) {
        bin_points[i] = (uint16_t)(freq_points[i] * fft_size / sample_rate);
        if(bin_points[i] >= fft_bin) bin_points[i] = fft_bin - 1;
    }
    
    // 生成每个滤波器的三角系数
    for(i = 0; i < num_filters; i++) {
        float32_t *filter = &filters[i * fft_bin];
        
        // 初始化为0
        for(j = 0; j < fft_bin; j++) {
            filter[j] = 0.0f;
        }
        
        // 上升沿
        for(j = bin_points[i]; j <= bin_points[i+1]; j++) {
            if(bin_points[i+1] > bin_points[i]) {
                filter[j] = (float32_t)(j - bin_points[i]) / (bin_points[i+1] - bin_points[i]);
            }
        }
        
        // 下降沿
        for(j = bin_points[i+1]; j <= bin_points[i+2]; j++) {
            if(bin_points[i+2] > bin_points[i+1]) {
                filter[j] = 1.0f - (float32_t)(j - bin_points[i+1]) / (bin_points[i+2] - bin_points[i+1]);
            }
        }
    }
}
