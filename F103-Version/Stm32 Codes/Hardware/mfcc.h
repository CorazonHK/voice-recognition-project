#ifndef __MFCC_H
#define __MFCC_H

#include "stm32f10x.h"
#include <arm_math.h>

#define MFCC_SAMPLE_RATE    8000
#define MFCC_FRAME_LEN       64
#define MFCC_FRAME_SHIFT     32
#define MFCC_NUM_FILTERS     20
#define MFCC_COEFFS          13
#define MFCC_FFT_SIZE        128

typedef struct {
    const float32_t *hamming_window;
    const float32_t *mel_filters;
    float32_t *fft_buffer;
    float32_t *mfcc_out;
    uint16_t frame_count;
} MFCC_HandleTypeDef;

void MFCC_Init(MFCC_HandleTypeDef *hmfcc);
void MFCC_ComputeFrame(MFCC_HandleTypeDef *hmfcc, int16_t *pcm_frame, float32_t *mfcc_out);
void MFCC_Quantize(float32_t *mfcc_float, int16_t *mfcc_int, uint8_t shift);

#endif
