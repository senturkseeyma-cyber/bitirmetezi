/**
 * @file    orko_config.h
 * @brief   ORKO Slave Node - Merkezi Konfigürasyon Dosyası
 * @project ORKO - Orman Yangını Erken Tespit ve Koruma Sistemi
 * @board   Slave Kart
 * @mcu     Nuvoton M031FB0AE (ARM Cortex-M0 @ 48MHz)
 *
 * @note    PCB pinlerini kendi şemanıza göre güncelleyiniz.
 *          Tüm eşik değerleri ve zamanlama bu dosyadan yönetilir.
 */

#ifndef ORKO_CONFIG_H
#define ORKO_CONFIG_H

#ifndef ORKO_SIM
#include "NuMicro.h"
#endif
#include <stdint.h>

/* ================================================================
 * NODE KİMLİK BİLGİLERİ
 * ================================================================ */
#define ORKO_SLAVE_ADDRESS        0x0001U   /**< Bu slave node'un adresi */
#define ORKO_MASTER_ADDRESS       0x0000U   /**< Master node adresi */

/* ================================================================
 * SİSTEM ZAMANLAMA PARAMETRELERİ (saniye)
 * ================================================================ */
#define SENTINEL_INTERVAL_SEC     120U      /**< Nöbetçi (SCD40) ölçüm: 2 dakika */
#define FULL_READ_INTERVAL_SEC    600U      /**< Tam ölçüm döngüsü: 10 dakika */
#define CYCLE_DURATION_SEC        7200U     /**< 2 saatlik ana döngü */
#define SAMPLE_BUFFER_SIZE        5U        /**< Fire Score için son N örnek */

/* ================================================================
 * FIRE SCORE AĞIRLIK KATSAYILARI (toplam = 1.0)
 * ================================================================ */
#define WEIGHT_TEMPERATURE        0.25f     /**< Sıcaklık ağırlığı */
#define WEIGHT_CO2                0.25f     /**< CO2 ağırlığı */
#define WEIGHT_CO                 0.25f     /**< CO ağırlığı */
#define WEIGHT_PRESSURE           0.15f     /**< Basınç ağırlığı */
#define WEIGHT_HUMIDITY           0.10f     /**< Nem ağırlığı */

/* ================================================================
 * KRİTİK EŞİK DEĞERLERİ
 * ================================================================
 * Kaynak: Tez Bölüm 3.3.6 - Sensör Eşik Değerleri Tablosu
 */

/** Sıcaklık (°C) */
#define TEMP_NORMAL_MAX_C         60.0f
#define TEMP_NORMAL_MIN_C        -10.0f
#define TEMP_CRITICAL_C           70.0f     /**< Yangın kritik sıcaklık */
#define TEMP_AVERAGE_C            25.0f     /**< Bölge normal ortalama */

/** CO2 (ppm) */
#define CO2_NORMAL_MAX_PPM        1000.0f
#define CO2_NORMAL_MIN_PPM         400.0f   /**< Atmosferik normal */
#define CO2_CRITICAL_PPM          5000.0f   /**< Yangın kritik eşik */
#define CO2_AVERAGE_PPM            450.0f   /**< Orman ortamı ortalama */

/** CO - Karbon Monoksit (ppm) */
#define CO_NORMAL_MAX_PPM          50.0f
#define CO_CRITICAL_PPM           200.0f    /**< Yangın kritik eşik */
#define CO_AVERAGE_PPM             10.0f    /**< Normal ortam ortalama */

/** Basınç (Pa) */
#define PRESSURE_NORMAL_PA        101325.0f /**< Deniz seviyesi */
#define PRESSURE_LOW_PA            95000.0f /**< Kritik düşük basınç */
#define PRESSURE_AVERAGE_PA       101000.0f /**< Ortalama bölge basıncı */

/** Nem (%) */
#define HUMIDITY_NORMAL_MIN_PCT    20.0f
#define HUMIDITY_NORMAL_MAX_PCT    80.0f
#define HUMIDITY_LOW_PCT           15.0f    /**< Kritik kuru ortam */
#define HUMIDITY_AVERAGE_PCT       55.0f    /**< Orman ortamı ortalama */

/* ================================================================
 * ALARM KARAR KOŞULU EŞİĞİ
 * ================================================================ */
#define FIRE_SCORE_THRESHOLD      0.60f     /**< FireScore > 0.6 → Alarm */

/* ================================================================
 * I2C KONFIGÜRASYONU
 * ================================================================
 * Tüm I2C sensörler: BMP390, SCD40, HDC2022
 * NOT: PCB şemanıza göre pin numaralarını doğrulayınız.
 */
#ifndef ORKO_SIM
#define ORKO_I2C_PORT             I2C0
#endif
#define I2C_SPEED_HZ              400000U   /**< 400kHz Fast Mode */

/** I2C Cihaz Adresleri */
#define BMP390_I2C_ADDR           0x76U     /**< SDO=GND → 0x76 */
#define SCD40_I2C_ADDR            0x62U     /**< Sabit adres */
#define HDC2022_I2C_ADDR          0x40U     /**< A0=A1=GND → 0x40 */

/* ================================================================
 * UART KONFIGÜRASYONU
 * ================================================================ */

/** UART0 → NEO-7M GPS */
#ifndef ORKO_SIM
#define GPS_UART_PORT             UART0
#endif
#define GPS_UART_BAUD             9600U
#define GPS_RX_BUF_SIZE           128U

/** UART1 → E22-900T22D LoRa */
#ifndef ORKO_SIM
#define LORA_UART_PORT            UART1
#endif
#define LORA_UART_BAUD            9600U
#define LORA_RX_BUF_SIZE          64U

/* ================================================================
 * ADC KONFIGÜRASYONU - MQ9 GAZ SENSÖRÜ
 * ================================================================ */
#ifndef ORKO_SIM
#define MQ9_ADC_CH                ADC_CH_14_MASK  /**< ADC Channel 14 = PB.14 (şematik pin 4) */
#endif
#define MQ9_ADC_CH_NUM            14U
#define MQ9_VCC_MV                5000U           /**< MQ9 besleme (mV) */
#define MQ9_RL_KOHM               10.0f           /**< Yük direnci (kΩ) */
#define MQ9_RO_KOHM               10.0f           /**< Kalibrasyon R0 (kΩ) - sahada kalibre et */
#define MQ9_A_COEFF               26.179f         /**< CO dönüşüm katsayısı a */
#define MQ9_B_COEFF              -1.179f          /**< CO dönüşüm katsayısı b */
#define ADC_MAX_VALUE             4095U           /**< 12-bit ADC maksimum */
#define ADC_VREF_MV               3300U           /**< ADC referans gerilimi (mV) */

/* ================================================================
 * E22-900T22D LORA GPIO PİNLERİ
 * ================================================================
 * NOT: PCB şemanıza göre güncelleyiniz.
 */
#ifndef ORKO_SIM
#define LORA_M0_PORT              PB             /**< PB.12 - şematik pin 6 */
#define LORA_M0_BIT               BIT12
#define LORA_M1_PORT              PF             /**< PF.6 (NRESET) - şematik pin 18 */
#define LORA_M1_BIT               BIT6           /**< NOT: NRESET/GPIO ortak pin, SYS_UnlockReg gerektirir */
#define LORA_AUX_PORT             PA             /**< PA.1 - şematik pin 16 */
#define LORA_AUX_BIT              BIT1
#endif

/** LoRa Parametre Konfigürasyonu */
#define LORA_CHANNEL              0U            /**< Kanal 0 */
#define LORA_NET_ID               0U            /**< Ağ ID */
#define LORA_AIR_RATE             0x03U         /**< 2.4kbps hava hızı */
#define LORA_TX_POWER             0x00U         /**< Maksimum güç */

/* ================================================================
 * LED / DURUM GÖSTERGESİ
 * ================================================================ */
#ifndef ORKO_SIM
#define LED_PORT                  PB             /**< PB.13 DigitalOut - şematik pin 5 */
#define LED_BIT                   BIT13
#define LED_ON()                  (PB13 = 1)  /**< Aktif yüksek */
#define LED_OFF()                 (PB13 = 0)
#define LED_TOGGLE()              (PB13 ^= 1)
#else
/* Simülasyon: LED işlemleri devre dışı (no-op) */
#define LED_ON()                  ((void)0)
#define LED_OFF()                 ((void)0)
#define LED_TOGGLE()              ((void)0)
#endif

/* ================================================================
 * GENEL TANIMLAR
 * ================================================================ */
#define ORKO_TRUE                 1U
#define ORKO_FALSE                0U
#define ORKO_OK                   0
#define ORKO_ERROR               -1
#define ORKO_TIMEOUT             -2

/** Sistem çekirdek frekansı */
#define SYSTEM_CORE_CLOCK_HZ      48000000UL   /**< M031 HIRC: 48MHz */

#endif /* ORKO_CONFIG_H */
