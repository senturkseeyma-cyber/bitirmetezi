/**
 * @file    drv_hdc2022.c
 * @brief   HDC2022 Sıcaklık ve Nem Sensörü Sürücüsü - Kaynak
 */

#include "drv_hdc2022.h"
#include "hal_timer.h"

/* ================================================================
 * YARDIMCI FONKSİYONLAR
 * ================================================================ */

/**
 * @brief Ham register değerlerini fiziksel birimlere çevirir
 *        Dönüşüm formülleri: HDC2022 datasheet'e göre
 */
static float HDC2022_ConvertTemp(uint16_t rawTemp)
{
    /* T(°C) = (raw / 65536) * 165 - 40 */
    return ((float)rawTemp / 65536.0f) * 165.0f - 40.0f;
}

static float HDC2022_ConvertHum(uint16_t rawHum)
{
    /* RH(%) = (raw / 65536) * 100 */
    return ((float)rawHum / 65536.0f) * 100.0f;
}

/* ================================================================
 * GENEL FONKSİYONLAR
 * ================================================================ */

int8_t HDC2022_Init(void)
{
    uint8_t midLow  = 0U;
    uint8_t midHigh = 0U;
    uint16_t mid    = 0U;

    /* Üretici ID kontrolü: TI = 0x5449 */
    if (HAL_I2C_ReadReg(HDC2022_I2C_ADDR, HDC2022_REG_MID_LOW,  &midLow)  != ORKO_OK)
        return ORKO_ERROR;
    if (HAL_I2C_ReadReg(HDC2022_I2C_ADDR, HDC2022_REG_MID_HIGH, &midHigh) != ORKO_OK)
        return ORKO_ERROR;

    mid = ((uint16_t)midHigh << 8) | midLow;
    if (mid != 0x5449U)
        return ORKO_ERROR;

    /* Çözünürlük: 14-bit sıcaklık + nem, tek ölçüm modu */
    if (HAL_I2C_WriteReg(HDC2022_I2C_ADDR, HDC2022_REG_MEAS_CFG,
                         HDC2022_RESOLUTION_14BIT) != ORKO_OK)
        return ORKO_ERROR;

    return ORKO_OK;
}

int8_t HDC2022_Read(HDC2022_Data_t *pData)
{
    uint8_t  buf[4] = {0U};
    uint16_t rawTemp = 0U;
    uint16_t rawHum  = 0U;

    /* Tek ölçümü tetikle */
    if (HAL_I2C_WriteReg(HDC2022_I2C_ADDR, HDC2022_REG_MEAS_CFG,
                         HDC2022_MEAS_TRIG) != ORKO_OK)
        return ORKO_ERROR;

    /* Ölçüm tamamlanması için bekleme (~2ms @ 14-bit) */
    HAL_Timer_DelayMs(5U);

    /* 4 byte oku: TEMP_LOW, TEMP_HIGH, HUM_LOW, HUM_HIGH */
    if (HAL_I2C_ReadRegMulti(HDC2022_I2C_ADDR, HDC2022_REG_TEMP_LOW,
                             buf, 4U) != ORKO_OK)
        return ORKO_ERROR;

    rawTemp = ((uint16_t)buf[1] << 8) | buf[0];
    rawHum  = ((uint16_t)buf[3] << 8) | buf[2];

    pData->temperature_C = HDC2022_ConvertTemp(rawTemp);
    pData->humidity_pct  = HDC2022_ConvertHum(rawHum);

    return ORKO_OK;
}

void HDC2022_Sleep(void)
{
    /* HDC2022 varsayılan olarak tek ölçüm modunda güç tasarrufu yapar.
     * Ek bir uyku komutu gerekmez. */
}
