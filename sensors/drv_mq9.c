/**
 * @file    drv_mq9.c
 * @brief   MQ9 CO / Yanıcı Gaz Sensörü Sürücüsü - Kaynak
 *
 * CO ppm dönüşümü:
 *   Rs = ((Vcc - Vadc) / Vadc) * RL    [kΩ]
 *   ppm = A * (Rs/R0)^B
 *   A = 26.179, B = -1.179  (MQ9 CO eğrisi, datasheet'ten)
 *   R0: temiz havada kalibre edilen sensör direnci
 */

#include "drv_mq9.h"
#include <math.h>

/* ================================================================
 * GENEL FONKSİYONLAR
 * ================================================================ */

void MQ9_Init(void)
{
    HAL_ADC_Init();
}

int8_t MQ9_Read(MQ9_Data_t *pData)
{
    uint32_t vMV;
    float    rs;
    float    ratio;

    /* ADC okuma (mV cinsinden) */
    vMV = HAL_ADC_ReadMV(MQ9_ADC_CH_NUM);
    pData->raw_mv = vMV;

    /* Sensör gerilimi 0V ise okuma geçersiz */
    if (vMV == 0U)
        return ORKO_ERROR;

    /* Rs = ((Vcc - Vadc) / Vadc) * RL
     * Vcc = 5000mV, RL = 10kΩ */
    rs = ((float)(MQ9_VCC_MV - vMV) / (float)vMV) * MQ9_RL_KOHM;

    if (rs < 0.001f)
        rs = 0.001f;  /* Sıfır bölme koruması */

    ratio = rs / MQ9_RO_KOHM;

    /* CO ppm = A * (Rs/R0)^B */
    pData->rs_kohm = rs;
    pData->rs_r0   = ratio;
    pData->co_ppm  = MQ9_A_COEFF * powf(ratio, MQ9_B_COEFF);

    /* Negatif değerleri sıfırla */
    if (pData->co_ppm < 0.0f)
        pData->co_ppm = 0.0f;

    return ORKO_OK;
}

float MQ9_ReadCO_ppm(void)
{
    MQ9_Data_t data = {0};

    if (MQ9_Read(&data) != ORKO_OK)
        return 0.0f;

    return data.co_ppm;
}
