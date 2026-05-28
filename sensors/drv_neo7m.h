/**
 * @file    drv_neo7m.h
 * @brief   NEO-7M GPS Modülü Sürücüsü - Başlık
 * @interface UART0 (9600 baud, NMEA 0183)
 *
 * $GPGGA cümlesini parse ederek enlem, boylam ve yükseklik alır.
 */

#ifndef DRV_NEO7M_H
#define DRV_NEO7M_H

#include "orko_config.h"
#include "hal_uart.h"
#include <stdint.h>

/* GPS sabitleri */
#define GPS_NMEA_MAX_LEN      82U   /**< NMEA cümle maksimum uzunluğu */
#define GPS_FIX_TIMEOUT_MS    5000U /**< Fix bekleme zaman aşımı */

/* ================================================================
 * VERİ YAPISI
 * ================================================================ */
typedef struct {
    float    latitude;        /**< Enlem (decimal degrees, + = K, - = G) */
    float    longitude;       /**< Boylam (decimal degrees, + = D, - = B) */
    float    altitude_m;      /**< Yükseklik (metre) */
    uint8_t  satellites;      /**< Bağlı uydu sayısı */
    uint8_t  fix_valid;       /**< 1: geçerli fix, 0: geçersiz */
} GPS_Data_t;

/* ================================================================
 * FONKSİYONLAR
 * ================================================================ */

/**
 * @brief GPS UART'ını başlatır
 */
void GPS_Init(void);

/**
 * @brief  GPS verisini okur ($GPGGA parse eder)
 * @param  pData    : GPS verisinin yazılacağı yapı
 * @param  attempts : Deneme sayısı (her deneme 1 NMEA satırı)
 * @return ORKO_OK: geçerli fix alındı / ORKO_ERROR: zaman aşımı
 */
int8_t GPS_Read(GPS_Data_t *pData, uint8_t attempts);

/**
 * @brief GPS modülünü uyku moduna alır (UART komut ile)
 */
void GPS_Sleep(void);

#endif /* DRV_NEO7M_H */
