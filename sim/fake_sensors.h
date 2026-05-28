/**
 * @file    fake_sensors.h
 * @brief   ORKO Simulasyon - Sahte Sensor Veri Ureteci
 * @note    Gercek donanim gerektirmez. PC'de calisir.
 */

#ifndef FAKE_SENSORS_H
#define FAKE_SENSORS_H

#include <stdint.h>

/** Simulasyon toplam adim sayisi */
#define FAKE_SCENARIO_STEPS   20U

/**
 * @brief Bir adimlik sahte sensor verisi
 */
typedef struct {
    float       temperature_C;   /**< Ortam sicakligi (C)        */
    float       humidity_pct;    /**< Bagil nem (%)              */
    float       pressure_Pa;     /**< Atmosferik basinc (Pa)     */
    float       co2_ppm;         /**< CO2 yogunlugu (ppm)        */
    float       co_ppm;          /**< CO yogunlugu (ppm)         */
    float       latitude;        /**< GPS enlem                  */
    float       longitude;       /**< GPS boylam                 */
    float       altitude_m;      /**< GPS yukseklik (m)          */
    uint8_t     gps_valid;       /**< GPS fix gecerli mi         */
    const char *phase_name;      /**< Senaryo faz adi            */
} FakeSensorData_t;

/**
 * @brief Belirtilen adim numarasina ait sahte veri dondurur.
 * @param step  : 0 tabanli adim (0 ... FAKE_SCENARIO_STEPS-1)
 * @param pData : Doldurulacak veri yapisi
 */
void FakeSensors_GetData(uint8_t step, FakeSensorData_t *pData);

#endif /* FAKE_SENSORS_H */
