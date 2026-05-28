/**
 * @file    lora_e22.h
 * @brief   E22-900T22D LoRa Modülü Sürücüsü - Başlık
 * @module  EBYTE E22-900T22D (SX1262 tabanlı, 868MHz)
 * @interface UART1 + GPIO (M0, M1, AUX)
 *
 * Mod seçimi:
 *   M0=0, M1=0 → Normal (şeffaf iletim)
 *   M0=1, M1=0 → Konfigürasyon (AT komutları)
 *   M0=0, M1=1 → WOR (Wake On Radio)
 *   M0=1, M1=1 → Deep Sleep
 */

#ifndef LORA_E22_H
#define LORA_E22_H

#include "orko_config.h"
#include "hal_uart.h"
#include "hal_timer.h"
#include <stdint.h>

/* ================================================================
 * E22 MODÜL SABİTLERİ
 * ================================================================ */
#define E22_CONFIG_CMD            0xC0U   /**< Kayıt et ve uygula */
#define E22_CONFIG_CMD_TEMP       0xC2U   /**< Geçici uygula */
#define E22_CONFIG_READ           0xC1U   /**< Konfigürasyonu oku */
#define E22_CONFIG_VERSION        0xC3U   /**< Versiyon oku */
#define E22_CONFIG_RESET          0xC4U   /**< Sıfırla */

#define E22_AUX_READY_TIMEOUT_MS  3000U   /**< AUX bekleme zaman aşımı */
#define E22_CONFIG_WAIT_MS        100U    /**< Mod değişimi bekleme */
#define E22_SEND_TIMEOUT_MS       2000U   /**< Gönderim zaman aşımı */

/* ================================================================
 * PAKET YAPISI
 * Slave → Master gönderim paketi
 * ================================================================ */
#pragma pack(push, 1)
typedef struct {
    uint16_t slaveAddr;        /**< Bu slave node'un adresi */
    uint16_t masterAddr;       /**< Hedef master adresi */
    uint8_t  channel;          /**< LoRa kanalı */
    uint8_t  packetType;       /**< 0x01=Normal rapor, 0x02=Alarm */
    /* Sensör verileri */
    float    temperature_C;    /**< HDC2022 sıcaklık */
    float    humidity_pct;     /**< HDC2022 nem */
    float    pressure_Pa;      /**< BMP390 basınç */
    uint16_t co2_ppm;          /**< SCD40 CO2 */
    float    co_ppm;           /**< MQ9 CO */
    /* GPS */
    float    latitude;
    float    longitude;
    float    altitude_m;
    uint8_t  gps_valid;
    /* Risk değerlendirmesi */
    float    fire_score;       /**< 0.0 - 1.0 */
    uint8_t  alarm_flag;       /**< 1: Yangın alarmı */
    /* Sistem durumu */
    uint32_t uptime_sec;       /**< Çalışma süresi */
} ORKO_Packet_t;
#pragma pack(pop)

/* Paket tipleri */
#define PKT_TYPE_NORMAL           0x01U
#define PKT_TYPE_ALARM            0x02U
#define PKT_TYPE_STATUS           0x03U

/* ================================================================
 * FONKSİYONLAR
 * ================================================================ */

/**
 * @brief E22 modülünü başlatır ve konfigüre eder
 *        (adres, kanal, hava hızı EEPROM'a yazar)
 * @return ORKO_OK / ORKO_ERROR
 */
int8_t LoRa_Init(void);

/**
 * @brief  Master node'a veri paketi gönderir
 * @param  pPacket : Gönderilecek paket pointer'ı
 * @return ORKO_OK / ORKO_ERROR
 */
int8_t LoRa_SendPacket(const ORKO_Packet_t *pPacket);

/**
 * @brief  Gelen veri var mı kontrol eder (non-blocking)
 * @return ORKO_TRUE / ORKO_FALSE
 */
uint8_t LoRa_DataAvailable(void);

/**
 * @brief  Gelen veriyi okur
 * @param  pBuf    : Veri tamponu
 * @param  maxLen  : Maksimum byte
 * @return Okunan byte sayısı
 */
uint16_t LoRa_Receive(uint8_t *pBuf, uint16_t maxLen);

/**
 * @brief Modülü WOR (Wake On Radio) moduna alır → düşük güç
 */
void LoRa_SetWOR(void);

/**
 * @brief Modülü normal iletim moduna alır
 */
void LoRa_SetNormal(void);

/**
 * @brief Modülü deep sleep moduna alır (minimum güç)
 */
void LoRa_SetDeepSleep(void);

/**
 * @brief AUX pini HIGH olana kadar bekler (modül hazır sinyali)
 * @return ORKO_OK / ORKO_TIMEOUT
 */
int8_t LoRa_WaitAUX(void);

#endif /* LORA_E22_H */
