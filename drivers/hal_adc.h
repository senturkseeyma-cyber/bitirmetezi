/**
 * @file    hal_adc.h
 * @brief   ADC Donanım Soyutlama Katmanı - Başlık Dosyası
 * @mcu     Nuvoton M031FB0AE
 *
 * Kullanım: MQ9 gaz sensörünün analog çıkışını okumak için
 */

#ifndef HAL_ADC_H
#define HAL_ADC_H

#include "orko_config.h"
#include <stdint.h>

/**
 * @brief ADC modülünü başlatır (tek uçlu, tek dönüşüm modu)
 */
void HAL_ADC_Init(void);

/**
 * @brief ADC modülünü durdurur
 */
void HAL_ADC_DeInit(void);

/**
 * @brief  Belirtilen kanaldan ham 12-bit ADC değerini okur
 * @param  channel : ADC kanal numarası (0-7)
 * @return 0-4095 arası ham ADC değeri
 */
uint16_t HAL_ADC_Read(uint8_t channel);

/**
 * @brief  Belirtilen kanaldan mV cinsinden gerilim okur
 * @param  channel : ADC kanal numarası
 * @return Ölçülen gerilim (mV)
 */
uint32_t HAL_ADC_ReadMV(uint8_t channel);

#endif /* HAL_ADC_H */
