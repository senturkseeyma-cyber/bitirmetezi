/**
 * @file    hal_timer.h
 * @brief   Timer / SysTick Soyutlama Katmanı - Başlık Dosyası
 * @mcu     Nuvoton M031FB0AE
 *
 * SysTick → 1ms tick sayacı (gecikme ve timeout için)
 * Timer0  → Periyodik uyandırma kesmesi (nöbetçi döngüsü için)
 */

#ifndef HAL_TIMER_H
#define HAL_TIMER_H

#include "orko_config.h"
#include <stdint.h>

/**
 * @brief SysTick ve Timer0'ı başlatır
 *        SysTick: 1ms periyot
 *        Timer0 : 1Hz (1 saniyede bir kesme)
 */
void HAL_Timer_Init(void);

/**
 * @brief Milisaniye cinsinden gecikme
 * @param ms : Bekleme süresi (ms)
 */
void HAL_Timer_DelayMs(uint32_t ms);

/**
 * @brief Mikrosaniye cinsinden meşgul-bekleme (busy-wait)
 * @param us : Bekleme süresi (µs) — kısa süreler için kullanın
 */
void HAL_Timer_DelayUs(uint32_t us);

/**
 * @brief SysTick sayacını döndürür (ms cinsinden, overflow ~49 gün)
 * @return Sistem başlangıcından itibaren geçen ms
 */
uint32_t HAL_Timer_GetTick(void);

/**
 * @brief İki tick arasındaki farkı hesaplar (overflow güvenli)
 * @param startTick : Başlangıç tick değeri
 * @return Geçen süre (ms)
 */
uint32_t HAL_Timer_Elapsed(uint32_t startTick);

/**
 * @brief Saniye sayacını döndürür (Timer0 kesmesi ile artırılır)
 * @return Başlangıçtan itibaren geçen saniye
 */
uint32_t HAL_Timer_GetSeconds(void);

/**
 * @brief Saniye sayacını sıfırlar
 */
void HAL_Timer_ResetSeconds(void);

#endif /* HAL_TIMER_H */
