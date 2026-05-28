/**
 * @file    drv_hdc2022.h
 * @brief   HDC2022 Sıcaklık ve Nem Sensörü Sürücüsü - Başlık
 * @datasheet Texas Instruments HDC2022
 * @interface I2C
 */

#ifndef DRV_HDC2022_H
#define DRV_HDC2022_H

#include "orko_config.h"
#include "hal_i2c.h"
#include <stdint.h>

/* ================================================================
 * HDC2022 REGISTER ADRESLERI
 * ================================================================ */
#define HDC2022_REG_TEMP_LOW      0x00U  /**< Sıcaklık düşük byte */
#define HDC2022_REG_TEMP_HIGH     0x01U  /**< Sıcaklık yüksek byte */
#define HDC2022_REG_HUM_LOW       0x02U  /**< Nem düşük byte */
#define HDC2022_REG_HUM_HIGH      0x03U  /**< Nem yüksek byte */
#define HDC2022_REG_STATUS        0x04U  /**< Durum register */
#define HDC2022_REG_MEAS_CFG      0x0FU  /**< Ölçüm konfigürasyon */
#define HDC2022_REG_MID_LOW       0x14U  /**< Üretici ID düşük */
#define HDC2022_REG_MID_HIGH      0x15U  /**< Üretici ID yüksek */

/* Ölçüm konfigürasyon bitleri */
#define HDC2022_MEAS_TRIG         0x01U  /**< Tek ölçüm başlat */
#define HDC2022_MEAS_BOTH         0x00U  /**< Sıcaklık + Nem */
#define HDC2022_RESOLUTION_14BIT  0x00U  /**< 14-bit çözünürlük */

/* ================================================================
 * VERİ YAPISI
 * ================================================================ */
typedef struct {
    float temperature_C;   /**< Sıcaklık (°C) */
    float humidity_pct;    /**< Bağıl nem (%) */
} HDC2022_Data_t;

/* ================================================================
 * FONKSİYONLAR
 * ================================================================ */

/**
 * @brief HDC2022'yi başlatır ve kimliğini doğrular
 * @return ORKO_OK / ORKO_ERROR
 */
int8_t HDC2022_Init(void);

/**
 * @brief  Sıcaklık ve nem verisini okur
 * @param  pData : Ölçüm sonucunun yazılacağı yapı
 * @return ORKO_OK / ORKO_ERROR
 */
int8_t HDC2022_Read(HDC2022_Data_t *pData);

/**
 * @brief Sensörü düşük güç moduna alır
 */
void HDC2022_Sleep(void);

#endif /* DRV_HDC2022_H */
