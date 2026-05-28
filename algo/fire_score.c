/**
 * @file    fire_score.c
 * @brief   Ağırlıklı Çoklu Parametre Yangın Skoru Algoritması - Kaynak
 *
 * Tez Bölüm 3.3 formülleri:
 *
 * Son 5 ortalama: NAVG = (Ns-1 + Ns-2 + Ns-3 + Ns-4 + Ns-5) / 5
 *
 * Yangınla doğru orantılı (T, CO2, CO):
 *   Nx = (Ncurrent - NAVG) / (Ncritical - NAVG)
 *
 * Yangınla ters orantılı (Basınç, Nem):
 *   Nx = (NAVG - Ncurrent) / (NAVG - Ncritical)
 *
 * Nx < 0 ise Nx = 0 (normal altı)
 * Nx > 1 ise Nx = 1 (kritik üstü, doygunluk)
 *
 * FireScore = 0.25*NT + 0.25*NCO2 + 0.25*NCO + 0.15*NP + 0.10*NH
 */

#include "fire_score.h"
#include <string.h>

/* ================================================================
 * YARDIMCI FONKSİYONLAR
 * ================================================================ */

/**
 * @brief Tampondaki son N değerin ortalamasını hesaplar
 */
static float CalcAverage(const float *buf, uint8_t count)
{
    float sum = 0.0f;
    uint8_t i;

    if (count == 0U)
        return 0.0f;

    for (i = 0U; i < count; i++)
        sum += buf[i];

    return sum / (float)count;
}

/**
 * @brief Doğru orantılı parametre normalizasyonu
 *        Nx = (Ncurrent - NAVG) / (Ncritical - NAVG)
 *        [0.0 - 1.0] arasında sıkıştırılmış
 */
static float NormalizePositive(float current, float avg, float critical)
{
    float denom = critical - avg;
    float result;

    if (denom <= 0.0f)
        return 0.0f;

    result = (current - avg) / denom;

    if (result < 0.0f) result = 0.0f;
    if (result > 1.0f) result = 1.0f;

    return result;
}

/**
 * @brief Ters orantılı parametre normalizasyonu
 *        Nx = (NAVG - Ncurrent) / (NAVG - Ncritical)
 *        [0.0 - 1.0] arasında sıkıştırılmış
 */
static float NormalizeNegative(float current, float avg, float critical)
{
    float denom = avg - critical;
    float result;

    if (denom <= 0.0f)
        return 0.0f;

    result = (avg - current) / denom;

    if (result < 0.0f) result = 0.0f;
    if (result > 1.0f) result = 1.0f;

    return result;
}

/* ================================================================
 * GENEL FONKSİYONLAR
 * ================================================================ */

void FireScore_BufferInit(SensorBuffer_t *pBuf)
{
    memset(pBuf, 0, sizeof(SensorBuffer_t));
}

void FireScore_AddSample(SensorBuffer_t *pBuf,
                         float temp, float co2, float co,
                         float press, float hum)
{
    uint8_t idx = pBuf->head;

    pBuf->temperature_C[idx] = temp;
    pBuf->co2_ppm[idx]       = co2;
    pBuf->co_ppm[idx]        = co;
    pBuf->pressure_Pa[idx]   = press;
    pBuf->humidity_pct[idx]  = hum;

    /* Dairesel tampon başlığını ilerlet */
    pBuf->head = (uint8_t)((pBuf->head + 1U) % SAMPLE_BUFFER_SIZE);

    /* Örnek sayısını güncelle */
    if (pBuf->count < SAMPLE_BUFFER_SIZE)
        pBuf->count++;
}

int8_t FireScore_Calculate(const SensorBuffer_t *pBuf,
                           FireScore_Result_t *pResult)
{
    float avgTemp, avgCO2, avgCO, avgPress, avgHum;

    /* En az 1 ölçüm gerekli */
    if (pBuf->count == 0U)
        return ORKO_ERROR;

    /* Ortalama hesapla */
    avgTemp  = CalcAverage(pBuf->temperature_C, pBuf->count);
    avgCO2   = CalcAverage(pBuf->co2_ppm,       pBuf->count);
    avgCO    = CalcAverage(pBuf->co_ppm,        pBuf->count);
    avgPress = CalcAverage(pBuf->pressure_Pa,   pBuf->count);
    avgHum   = CalcAverage(pBuf->humidity_pct,  pBuf->count);

    /* Son (en güncel) değeri al */
    uint8_t lastIdx;
    if (pBuf->head == 0U)
        lastIdx = SAMPLE_BUFFER_SIZE - 1U;
    else
        lastIdx = pBuf->head - 1U;

    float curTemp  = pBuf->temperature_C[lastIdx];
    float curCO2   = pBuf->co2_ppm[lastIdx];
    float curCO    = pBuf->co_ppm[lastIdx];
    float curPress = pBuf->pressure_Pa[lastIdx];
    float curHum   = pBuf->humidity_pct[lastIdx];

    /* --------------------------------------------------------
     * Normalizasyon
     * Doğru orantılı: Sıcaklık, CO2, CO
     * Ters orantılı:  Basınç, Nem
     * -------------------------------------------------------- */
    pResult->n_temp     = NormalizePositive(curTemp,  avgTemp,  TEMP_CRITICAL_C);
    pResult->n_co2      = NormalizePositive(curCO2,   avgCO2,   CO2_CRITICAL_PPM);
    pResult->n_co       = NormalizePositive(curCO,    avgCO,    CO_CRITICAL_PPM);
    pResult->n_pressure = NormalizeNegative(curPress, avgPress, PRESSURE_LOW_PA);
    pResult->n_humidity = NormalizeNegative(curHum,   avgHum,   HUMIDITY_LOW_PCT);

    /* --------------------------------------------------------
     * Fire Score hesabı (Tez Denk. 3.3)
     * FireScore = 0.25*NT + 0.25*NCO2 + 0.25*NCO + 0.15*NP + 0.10*NH
     * -------------------------------------------------------- */
    pResult->fire_score = (WEIGHT_TEMPERATURE * pResult->n_temp)
                        + (WEIGHT_CO2         * pResult->n_co2)
                        + (WEIGHT_CO          * pResult->n_co)
                        + (WEIGHT_PRESSURE    * pResult->n_pressure)
                        + (WEIGHT_HUMIDITY    * pResult->n_humidity);

    /* --------------------------------------------------------
     * Kritik durum bayrakları
     * -------------------------------------------------------- */
    pResult->temp_crit = (curTemp >= TEMP_CRITICAL_C)    ? 1U : 0U;
    pResult->co2_crit  = (curCO2  >= CO2_CRITICAL_PPM)   ? 1U : 0U;
    pResult->co_crit   = (curCO   >= CO_CRITICAL_PPM)    ? 1U : 0U;

    /* --------------------------------------------------------
     * Alarm karar koşulu (Tez Bölüm 3.3.5):
     * [FireScore > 0.6] VE [Sıcaklık kritik]
     * VE [CO2 VEYA CO kritik]
     * -------------------------------------------------------- */
    pResult->alarm = 0U;
    if ((pResult->fire_score > FIRE_SCORE_THRESHOLD) &&
        (pResult->temp_crit == 1U) &&
        ((pResult->co2_crit == 1U) || (pResult->co_crit == 1U)))
    {
        pResult->alarm = 1U;
    }

    return ORKO_OK;
}

uint8_t FireScore_IsCO2Critical(uint16_t co2_ppm)
{
    return ((float)co2_ppm >= CO2_CRITICAL_PPM) ? 1U : 0U;
}
