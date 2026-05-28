/**
 * @file    drv_mq9.h
 * @brief   MQ9 CO / Yanıcı Gaz Sensörü Sürücüsü - Başlık
 * @interface ADC (Analog)
 *
 * MQ9: CO, CH4, LPG gazlarını algılar.
 * Bu sürücü CO için kalibre edilmiştir (ORKO projesine uygun).
 */

#ifndef DRV_MQ9_H
#define DRV_MQ9_H

#include "orko_config.h"
#include "hal_adc.h"
#include <stdint.h>

/* ================================================================
 * MQ9 HASSAS ISI DÖNGÜSÜ
 * Datasheet: Yüksek ısıtma (60sn @ 5V) + Düşük ısıtma (90sn @ 1.5V)
 * NOT: Isıtma kontrolü harici devre ile yapılıyorsa bu makroları devre dışı bırakın.
 * ================================================================ */
#define MQ9_WARMUP_TIME_MS        180000U  /**< Güç verilince bekleme süresi (3 dk) */
#define MQ9_HIGH_HEAT_TIME_MS      60000U  /**< Yüksek ısıtma süresi (1 dk) */
#define MQ9_LOW_HEAT_TIME_MS       90000U  /**< Düşük ısıtma süresi (1.5 dk) */

/* ================================================================
 * VERİ YAPISI
 * ================================================================ */
typedef struct {
    float    rs_kohm;    /**< Sensör direnci (kΩ) */
    float    rs_r0;      /**< Rs/R0 oranı */
    float    co_ppm;     /**< CO konsantrasyonu (ppm) */
    uint32_t raw_mv;     /**< Ham ADC değeri (mV) */
} MQ9_Data_t;

/* ================================================================
 * FONKSİYONLAR
 * ================================================================ */

/**
 * @brief MQ9 ADC kanalını başlatır
 */
void MQ9_Init(void);

/**
 * @brief  MQ9'dan okuma yapar ve CO ppm hesaplar
 * @param  pData : Ölçüm verisinin yazılacağı yapı
 * @return ORKO_OK / ORKO_ERROR
 */
int8_t MQ9_Read(MQ9_Data_t *pData);

/**
 * @brief  Sadece CO ppm değerini döndürür (hızlı sorgu)
 * @return CO konsantrasyonu (ppm)
 */
float MQ9_ReadCO_ppm(void);

#endif /* DRV_MQ9_H */
