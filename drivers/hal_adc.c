/**
 * @file    hal_adc.c
 * @brief   ADC Donanım Soyutlama Katmanı - Kaynak Dosyası
 * @mcu     Nuvoton M031FB0AE
 */

#include "hal_adc.h"

/* ================================================================
 * PIN KONFIGÜRASYONU
 * NOT: PCB şemanıza göre güncelleyiniz.
 *      ADC CH0 → PB0 (varsayılan)
 * ================================================================ */

static void ADC_PinConfig(void)
{
    /* PB.14 → ADC kanal 14  (şematik pin 4: ADC0_CH14) */
    SYS->GPB_MFPH = (SYS->GPB_MFPH & ~SYS_GPB_MFPH_PB14MFP_Msk)
                    | SYS_GPB_MFPH_PB14MFP_ADC0_CH14;

    /* ADC pini için digital I/O devre dışı bırak */
    GPIO_DISABLE_DIGITAL_PATH(PB, BIT14);
}

/* ================================================================
 * GENEL FONKSİYONLAR
 * ================================================================ */

void HAL_ADC_Init(void)
{
    /* ADC saat kaynağı: HIRC, bölücü = 1 */
    CLK_EnableModuleClock(ADC_MODULE);
    CLK_SetModuleClock(ADC_MODULE, CLK_CLKSEL2_ADCSEL_HIRC,
                       CLK_CLKDIV0_ADC(6)); /* 48MHz / 6 = 8MHz ADC saati */

    SYS_UnlockReg();
    ADC_PinConfig();
    SYS_LockReg();

    /* Tek uçlu giriş, tek dönüşüm modu - kanal 14 */
    ADC_Open(ADC, ADC_INPUT_MODE_SINGLE_END,
             ADC_OPERATION_MODE_SINGLE, ADC_CH_14_MASK);

    ADC_POWER_ON(ADC);
}

void HAL_ADC_DeInit(void)
{
    ADC_POWER_DOWN(ADC);
    ADC_Close(ADC);
    CLK_DisableModuleClock(ADC_MODULE);
}

uint16_t HAL_ADC_Read(uint8_t channel)
{
    uint32_t chMask = (1UL << channel);

    /* Kanalı seç */
    ADC_SET_INPUT_CHANNEL(ADC, chMask);

    /* Dönüşümü başlat */
    ADC_CLR_INT_FLAG(ADC, ADC_ADF_INT);
    ADC_START_CONV(ADC);

    /* Dönüşüm tamamlanana kadar bekle */
    while (ADC_IS_CONVERSION_DONE(ADC) == 0)
        __NOP();

    return (uint16_t)ADC_GET_CONVERSION_VALUE(ADC, channel);
}

uint32_t HAL_ADC_ReadMV(uint8_t channel)
{
    uint32_t raw = (uint32_t)HAL_ADC_Read(channel);

    /* mV = (raw * Vref) / ADC_MAX */
    return (raw * ADC_VREF_MV) / ADC_MAX_VALUE;
}
