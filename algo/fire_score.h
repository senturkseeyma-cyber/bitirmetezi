/**
 * @file    fire_score.h
 * @brief   Ağırlıklı Çoklu Parametre Yangın Skoru Algoritması - Başlık
 *
 * Kaynak: ORKO Tez Bölüm 3.3 - "Ağırlıklı Çoklu Parametre Karar Algoritması"
 *
 * FireScore = 0.25*NT + 0.25*NCO2 + 0.25*NCO + 0.15*NP + 0.10*NH
 *
 * Alarm koşulu:
 *   [FireScore > 0.6] VE [Sıcaklık kritik] VE [CO2 VEYA CO kritik]
 */

#ifndef FIRE_SCORE_H
#define FIRE_SCORE_H

#include "orko_config.h"
#include <stdint.h>

/* ================================================================
 * SENSÖR ÖRNEK TAMPONU (son 5 ölçüm)
 * ================================================================ */
typedef struct {
    float temperature_C[SAMPLE_BUFFER_SIZE];
    float co2_ppm[SAMPLE_BUFFER_SIZE];
    float co_ppm[SAMPLE_BUFFER_SIZE];
    float pressure_Pa[SAMPLE_BUFFER_SIZE];
    float humidity_pct[SAMPLE_BUFFER_SIZE];
    uint8_t count;      /**< Doldurulan örnek sayısı (maks SAMPLE_BUFFER_SIZE) */
    uint8_t head;       /**< Dairesel tampon başlangıcı */
} SensorBuffer_t;

/* ================================================================
 * ÇIKTI YAPISI
 * ================================================================ */
typedef struct {
    float n_temp;       /**< Normalize sıcaklık [0-1] */
    float n_co2;        /**< Normalize CO2 [0-1] */
    float n_co;         /**< Normalize CO [0-1] */
    float n_pressure;   /**< Normalize basınç [0-1] */
    float n_humidity;   /**< Normalize nem [0-1] */
    float fire_score;   /**< Toplam Fire Score [0-1] */
    uint8_t alarm;      /**< 1: Alarm koşulu sağlandı */
    uint8_t temp_crit;  /**< 1: Sıcaklık kritik eşiği aştı */
    uint8_t co2_crit;   /**< 1: CO2 kritik eşiği aştı */
    uint8_t co_crit;    /**< 1: CO kritik eşiği aştı */
} FireScore_Result_t;

/* ================================================================
 * FONKSİYONLAR
 * ================================================================ */

/**
 * @brief Sensör tamponunu sıfırlar
 * @param pBuf : Tampon pointer'ı
 */
void FireScore_BufferInit(SensorBuffer_t *pBuf);

/**
 * @brief Yeni ölçümü dairesel tampona ekler
 * @param pBuf   : Tampon
 * @param temp   : Sıcaklık (°C)
 * @param co2    : CO2 (ppm)
 * @param co     : CO (ppm)
 * @param press  : Basınç (Pa)
 * @param hum    : Nem (%)
 */
void FireScore_AddSample(SensorBuffer_t *pBuf,
                         float temp, float co2, float co,
                         float press, float hum);

/**
 * @brief  Fire Score hesaplar
 * @param  pBuf    : Sensör tamponu (son 5 ölçüm)
 * @param  pResult : Hesaplama sonucu
 * @return ORKO_OK / ORKO_ERROR (yetersiz örnek)
 */
int8_t FireScore_Calculate(const SensorBuffer_t *pBuf,
                           FireScore_Result_t *pResult);

/**
 * @brief Nöbetçi CO2 değeri kritik eşiği aşıyor mu? (hızlı kontrol)
 * @param co2_ppm : Anlık CO2 değeri
 * @return 1: Kritik, 0: Normal
 */
uint8_t FireScore_IsCO2Critical(uint16_t co2_ppm);

#endif /* FIRE_SCORE_H */
