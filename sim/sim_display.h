/**
 * @file    sim_display.h
 * @brief   ORKO Simulasyon - Konsol Cikti Modulu
 */

#ifndef SIM_DISPLAY_H
#define SIM_DISPLAY_H

#include <stdint.h>
#include "fake_sensors.h"
#include "fire_score.h"

/**
 * @brief Bir simulasyon adiminin verilerini konsola yazdirir.
 * @param step_num  : 1 tabanli adim numarasi
 * @param total     : Toplam adim sayisi
 * @param pData     : Sahte sensor verileri
 * @param pResult   : Fire Score sonuclari
 * @param calc_ok   : FireScore_Calculate donus degeri (0=OK, -1=hata)
 */
void SimDisplay_PrintStep(uint8_t                   step_num,
                          uint8_t                   total,
                          const FakeSensorData_t   *pData,
                          const FireScore_Result_t *pResult,
                          int8_t                    calc_ok);

#endif /* SIM_DISPLAY_H */
