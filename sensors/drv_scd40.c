/**
 * @file    drv_scd40.c
 * @brief   SCD40 CO2 Sensörü Sürücüsü - Kaynak
 *
 * SCD40 I2C protokolü:
 * - Komutlar 16-bit (2 byte, MSB önce)
 * - Her 2 byte veriden sonra 1 byte CRC-8 eklenir (polynomial: 0x31)
 */

#include "drv_scd40.h"
#include "hal_timer.h"

/* ================================================================
 * CRC-8 DOĞRULAMA
 * CRC polynomial: 0x31 (x^8 + x^5 + x^4 + 1)
 * ================================================================ */

static uint8_t SCD40_CRC8(const uint8_t *data, uint8_t len)
{
    uint8_t crc = 0xFFU;
    uint8_t i, j;

    for (i = 0U; i < len; i++)
    {
        crc ^= data[i];
        for (j = 0U; j < 8U; j++)
        {
            if (crc & 0x80U)
                crc = (uint8_t)((crc << 1) ^ 0x31U);
            else
                crc <<= 1;
        }
    }
    return crc;
}

/* ================================================================
 * KOMUT GÖNDERME YARDIMCISI
 * ================================================================ */

static int8_t SCD40_SendCmd(uint16_t cmd)
{
    uint8_t buf[2];
    buf[0] = (uint8_t)(cmd >> 8);
    buf[1] = (uint8_t)(cmd & 0xFFU);
    return HAL_I2C_CmdRead(SCD40_I2C_ADDR, buf, 2U, NULL, 0U);
}

/* ================================================================
 * GENEL FONKSİYONLAR
 * ================================================================ */

int8_t SCD40_Init(void)
{
    uint8_t  rxBuf[9] = {0U};
    uint8_t  cmd[2];
    uint16_t ser0, ser1, ser2;

    /* Çalışan bir periyodik ölçüm varsa durdur */
    SCD40_SendCmd(SCD40_CMD_STOP_PERIODIC);
    HAL_Timer_DelayMs(500U);

    /* Seri numarasını oku (9 byte: 3 kelime x [2 byte data + 1 byte CRC]) */
    cmd[0] = (uint8_t)(SCD40_CMD_GET_SERIAL_NUMBER >> 8);
    cmd[1] = (uint8_t)(SCD40_CMD_GET_SERIAL_NUMBER & 0xFFU);

    if (HAL_I2C_CmdRead(SCD40_I2C_ADDR, cmd, 2U, rxBuf, 9U) != ORKO_OK)
        return ORKO_ERROR;

    /* CRC doğrulaması */
    if (SCD40_CRC8(&rxBuf[0], 2U) != rxBuf[2])
        return ORKO_ERROR;
    if (SCD40_CRC8(&rxBuf[3], 2U) != rxBuf[5])
        return ORKO_ERROR;
    if (SCD40_CRC8(&rxBuf[6], 2U) != rxBuf[8])
        return ORKO_ERROR;

    ser0 = ((uint16_t)rxBuf[0] << 8) | rxBuf[1];
    ser1 = ((uint16_t)rxBuf[3] << 8) | rxBuf[4];
    ser2 = ((uint16_t)rxBuf[6] << 8) | rxBuf[7];

    /* Seri numarası 0 ise sensör bağlı değil */
    if ((ser0 == 0U) && (ser1 == 0U) && (ser2 == 0U))
        return ORKO_ERROR;

    return ORKO_OK;
}

int8_t SCD40_StartSingleShot(void)
{
    /* Tek atış ölçümü başlat */
    if (SCD40_SendCmd(SCD40_CMD_SINGLE_SHOT) != ORKO_OK)
        return ORKO_ERROR;

    /* Ölçüm tamamlanması için 5 saniye bekle */
    HAL_Timer_DelayMs(5000U);

    return ORKO_OK;
}

int8_t SCD40_Read(SCD40_Data_t *pData)
{
    uint8_t  rxBuf[9] = {0U};
    uint8_t  cmd[2];
    uint16_t co2Raw, tempRaw, humRaw;

    cmd[0] = (uint8_t)(SCD40_CMD_READ_MEASUREMENT >> 8);
    cmd[1] = (uint8_t)(SCD40_CMD_READ_MEASUREMENT & 0xFFU);

    if (HAL_I2C_CmdRead(SCD40_I2C_ADDR, cmd, 2U, rxBuf, 9U) != ORKO_OK)
        return ORKO_ERROR;

    /* CRC doğrula */
    if (SCD40_CRC8(&rxBuf[0], 2U) != rxBuf[2])
        return ORKO_ERROR;
    if (SCD40_CRC8(&rxBuf[3], 2U) != rxBuf[5])
        return ORKO_ERROR;
    if (SCD40_CRC8(&rxBuf[6], 2U) != rxBuf[8])
        return ORKO_ERROR;

    co2Raw  = ((uint16_t)rxBuf[0] << 8) | rxBuf[1];
    tempRaw = ((uint16_t)rxBuf[3] << 8) | rxBuf[4];
    humRaw  = ((uint16_t)rxBuf[6] << 8) | rxBuf[7];

    pData->co2_ppm       = co2Raw;
    pData->temperature_C = -45.0f + (175.0f * (float)tempRaw / 65535.0f);
    pData->humidity_pct  = 100.0f * (float)humRaw / 65535.0f;

    return ORKO_OK;
}

int8_t SCD40_ReadCO2(uint16_t *pCO2_ppm)
{
    SCD40_Data_t data = {0};

    if (SCD40_StartSingleShot() != ORKO_OK)
        return ORKO_ERROR;

    if (SCD40_Read(&data) != ORKO_OK)
        return ORKO_ERROR;

    *pCO2_ppm = data.co2_ppm;
    return ORKO_OK;
}

void SCD40_Sleep(void)
{
    SCD40_SendCmd(SCD40_CMD_STOP_PERIODIC);
    HAL_Timer_DelayMs(500U);
    SCD40_SendCmd(SCD40_CMD_POWER_DOWN);
}

void SCD40_WakeUp(void)
{
    /* Uyandırma: boş bir I2C yazma işlemi */
    uint8_t dummy = 0U;
    HAL_I2C_WriteByte(SCD40_I2C_ADDR, dummy);
    HAL_Timer_DelayMs(20U);
}
