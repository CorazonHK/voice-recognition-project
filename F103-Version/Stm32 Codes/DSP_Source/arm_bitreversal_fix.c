#include "arm_math.h"

void arm_bitreversal_32(uint32_t *pSrc, uint32_t bitRevLen, uint32_t bitRevIndexBit)
{
    uint32_t i, j;
    uint32_t tmp;
    uint32_t bitRevIdx;
    
    for(i = 0; i < bitRevLen; i++)
    {
        j = 0;
        for(uint32_t k = 0; k < 32; k++)
        {
            j <<= 1;
            j |= (i >> k) & 1;
        }
        j >>= (32 - bitRevIndexBit);
        
        if(i < j)
        {
            tmp = pSrc[i];
            pSrc[i] = pSrc[j];
            pSrc[j] = tmp;
        }
    }
}

void arm_bitreversal_16(uint16_t *pSrc, uint32_t bitRevLen, uint32_t bitRevIndexBit)
{
    uint32_t i, j;
    uint16_t tmp;
    uint32_t bitRevIdx;
    
    for(i = 0; i < bitRevLen; i++)
    {
        j = 0;
        for(uint32_t k = 0; k < 16; k++)
        {
            j <<= 1;
            j |= (i >> k) & 1;
        }
        j >>= (16 - bitRevIndexBit);
        
        if(i < j)
        {
            tmp = pSrc[i];
            pSrc[i] = pSrc[j];
            pSrc[j] = tmp;
        }
    }
}

