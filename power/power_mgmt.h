/**
 * @file    power_mgmt.h
 * @brief   Güç Yönetimi Modülü - Başlık
 * @mcu     Nuvoton M031FB0AE
 *
 * M031 Düşük Güç Modları:
 *   Idle Mode     : CPU durur, timer/peripheral çalışır (~3mA)
 *   Power-Down    : CPU + çoğu periferik durur (~10µA)
 *   Deep Power-Down: Tüm sistem neredeyse durur (~2µA)
 *
 * ORKO Güç Stratejisi:
 *   - Normal çalışma: 10 dakika sensör oku → sleep
 *   - Nöbetçi mod:    2 dakika SCD40 kontrol → sleep
 *   - 2 saatlik döngü: Hafıza temizle → durum raporu
 */

#ifndef POWER_MGMT_H
#define POWER_MGMT_H

#include "orko_config.h"
#include "hal_timer.h"
#include <stdint.h>

/* ================================================================
 * UYKU MODLARI
 * ================================================================ */
typedef enum {
    SLEEP_IDLE      = 0U,   /**< CPU durur, Timer çalışır (kısa beklemeler) */
    SLEEP_POWERDOWN = 1U,   /**< Derin uyku, Timer ile uyandırma */
} SleepMode_t;

/* ================================================================
 * FONKSİYONLAR
 * ================================================================ */

/**
 * @brief Güç yönetimi modülünü başlatır
 */
void PowerMgmt_Init(void);

/**
 * @brief  MCU'yu belirli süre için uyku moduna alır
 *         Timer0 kesmesi ile otomatik uyanır
 * @param  mode    : Uyku modu
 * @param  seconds : Uyku süresi (saniye)
 */
void PowerMgmt_Sleep(SleepMode_t mode, uint32_t seconds);

/**
 * @brief Sensörlerin güç hatlarını kapatır (GPIO ile kontrol varsa)
 *        NOT: PCB'de sensör güç anahtarı varsa buraya GPIO ekleyin
 */
void PowerMgmt_SensorsOff(void);

/**
 * @brief Sensörlerin güç hatlarını açar
 */
void PowerMgmt_SensorsOn(void);

/**
 * @brief LoRa modülünü deep sleep moduna alır
 */
void PowerMgmt_LoRaSleep(void);

/**
 * @brief Tüm modülleri uyku moduna alır (maksimum güç tasarrufu)
 */
void PowerMgmt_EnterFullSleep(void);

/**
 * @brief Sistem başlangıcından itibaren geçen süreyi döndürür (saniye)
 */
uint32_t PowerMgmt_GetUptime(void);

#endif /* POWER_MGMT_H */
