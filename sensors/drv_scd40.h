/**
 * @file    drv_scd40.h
 * @brief   SCD40 CO2 Sensörü Sürücüsü - Başlık
 * @datasheet Sensirion SCD40/SCD41
 * @interface I2C (komut tabanlı, CRC doğrulamalı)
 *
 * Bu sensör sistemde "nöbetçi sensör" olarak görev yapar.
 * Her 2 dakikada bir ölçüm periyoduna girer.
 */

#ifndef DRV_SCD40_H
#define DRV_SCD40_H

#include "orko_config.h"
#include "hal_i2c.h"
#include <stdint.h>

/* ================================================================
 * SCD40 KOMUTLARI (16-bit, MSB önce)
 * ================================================================ */
#define SCD40_CMD_START_PERIODIC          0x21B1U /**< Normal periyodik ölçüm (5sn) */
#define SCD40_CMD_START_LOWPWR_PERIODIC   0x21ACU /**< Düşük güç periyodik (30sn) */
#define SCD40_CMD_READ_MEASUREMENT        0xEC05U /**< Ölçüm oku */
#define SCD40_CMD_STOP_PERIODIC           0x3F86U /**< Periyodik ölçümü durdur */
#define SCD40_CMD_SET_TEMP_OFFSET         0x241DU /**< Sıcaklık ofseti ayarla */
#define SCD40_CMD_GET_SERIAL_NUMBER       0x3682U /**< Seri numarasını oku */
#define SCD40_CMD_SINGLE_SHOT             0x219DU /**< Tek ölçüm (aktif bekleme) */
#define SCD40_CMD_SINGLE_SHOT_RHT         0x2196U /**< Yalnızca nem+sıcaklık */
#define SCD40_CMD_POWER_DOWN              0x36F6U /**< Uyku modu */
#define SCD40_CMD_WAKE_UP                 0x36F6U /**< Uyandır (I2C yazma) */
#define SCD40_CMD_REINIT                  0x3646U /**< Yeniden başlat */

/* ================================================================
 * VERİ YAPISI
 * ================================================================ */
typedef struct {
    uint16_t co2_ppm;        /**< CO2 yoğunluğu (ppm) */
    float    temperature_C;  /**< SCD40 dahili sıcaklık (°C) */
    float    humidity_pct;   /**< SCD40 dahili nem (%) */
} SCD40_Data_t;

/* ================================================================
 * FONKSİYONLAR
 * ================================================================ */

/**
 * @brief SCD40'ı başlatır, seri numarasını doğrular
 * @return ORKO_OK / ORKO_ERROR
 */
int8_t SCD40_Init(void);

/**
 * @brief Nöbetçi ölçümü başlatır (tek atış, ~5sn bekler)
 * @return ORKO_OK / ORKO_ERROR
 */
int8_t SCD40_StartSingleShot(void);

/**
 * @brief  Son ölçüm sonucunu okur
 * @param  pData : Ölçüm verisinin yazılacağı yapı
 * @return ORKO_OK / ORKO_ERROR
 */
int8_t SCD40_Read(SCD40_Data_t *pData);

/**
 * @brief Sensörü uyku moduna alır (güç tüketimi ~0.4µA)
 */
void SCD40_Sleep(void);

/**
 * @brief Uyku modundan uyandırır (~20ms startup)
 */
void SCD40_WakeUp(void);

/**
 * @brief Yalnızca CO2 değerini okur (hızlı sorgu)
 * @param pCO2_ppm : CO2 ppm değerinin yazılacağı pointer
 * @return ORKO_OK / ORKO_ERROR
 */
int8_t SCD40_ReadCO2(uint16_t *pCO2_ppm);

#endif /* DRV_SCD40_H */
