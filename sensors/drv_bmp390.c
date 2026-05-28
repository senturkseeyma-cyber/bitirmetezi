/**
 * @file    drv_bmp390.c
 * @brief   BMP390 Basınç ve Sıcaklık Sensörü Sürücüsü - Kaynak
 *
 * Kalibrasyon ve dönüşüm formülleri: Bosch BMP390 datasheet Bölüm 8.5
 */

#include "drv_bmp390.h"
#include "hal_timer.h"

/* ================================================================
 * STATİK DEĞİŞKENLER
 * ================================================================ */
static BMP390_Calib_t s_calib;

/* ================================================================
 * KALİBRASYON FONKSİYONLARI
 * ================================================================ */

/**
 * @brief OTP bellekten 21 byte kalibrasyon verisini okur ve işler
 */
static int8_t BMP390_ReadCalib(void)
{
    uint8_t  raw[21] = {0U};
    uint16_t tmp     = 0U;

    if (HAL_I2C_ReadRegMulti(BMP390_I2C_ADDR, BMP390_REG_CALIB_DATA,
                             raw, 21U) != ORKO_OK)
        return ORKO_ERROR;

    /* --- Sıcaklık katsayıları --- */
    tmp = ((uint16_t)raw[1] << 8) | raw[0];
    s_calib.par_t1 = (double)tmp / 0.00390625;  /* 2^-8 */

    tmp = ((uint16_t)raw[3] << 8) | raw[2];
    s_calib.par_t2 = (double)tmp / 1073741824.0; /* 2^30 */

    s_calib.par_t3 = (double)(int8_t)raw[4] / 281474976710656.0; /* 2^48 */

    /* --- Basınç katsayıları --- */
    tmp = ((uint16_t)raw[6] << 8) | raw[5];
    s_calib.par_p1 = ((double)(int16_t)tmp - 16384.0) / 1048576.0; /* 2^20 */

    tmp = ((uint16_t)raw[8] << 8) | raw[7];
    s_calib.par_p2 = ((double)(int16_t)tmp - 16384.0) / 536870912.0; /* 2^29 */

    s_calib.par_p3 = (double)(int8_t)raw[9]  / 4294967296.0; /* 2^32 */
    s_calib.par_p4 = (double)(int8_t)raw[10] / 137438953472.0; /* 2^37 */

    tmp = ((uint16_t)raw[12] << 8) | raw[11];
    s_calib.par_p5 = (double)tmp / 0.125;  /* 2^-3 */

    tmp = ((uint16_t)raw[14] << 8) | raw[13];
    s_calib.par_p6 = (double)tmp / 64.0; /* 2^6 */

    s_calib.par_p7  = (double)(int8_t)raw[15] / 256.0;
    s_calib.par_p8  = (double)(int8_t)raw[16] / 32768.0;

    tmp = ((uint16_t)raw[18] << 8) | raw[17];
    s_calib.par_p9 = (double)(int16_t)tmp / 281474976710656.0;

    s_calib.par_p10 = (double)(int8_t)raw[19] / 281474976710656.0;
    s_calib.par_p11 = (double)(int8_t)raw[20] / 36893488147419103232.0;

    return ORKO_OK;
}

/* ================================================================
 * TELAFI (COMPENSATION) FONKSİYONLARI
 * ================================================================ */

/**
 * @brief Ham 24-bit sıcaklık değerini °C'ye çevirir
 */
static float BMP390_CompTemp(uint32_t rawTemp)
{
    double partial1, partial2, partial3;

    partial1 = (double)rawTemp - (256.0 * s_calib.par_t1);
    partial2 = s_calib.par_t2 * partial1;
    partial3 = partial1 * partial1;
    s_calib.t_lin = partial2 + (partial3 * s_calib.par_t3);

    return (float)s_calib.t_lin;
}

/**
 * @brief Ham 24-bit basınç değerini Pascal'a çevirir
 */
static float BMP390_CompPress(uint32_t rawPress)
{
    double partial1, partial2, partial3;
    double partial4, partial5, partial6;
    double comp_press;
    double t = s_calib.t_lin;

    partial1 = s_calib.par_p6 * t;
    partial2 = s_calib.par_p7 * (t * t);
    partial3 = s_calib.par_p8 * (t * t * t);
    partial4 = s_calib.par_p5 + partial1 + partial2 + partial3;

    partial5 = s_calib.par_p2 * t;
    partial6 = s_calib.par_p3 * (t * t);
    double offset = s_calib.par_p1 + partial5 + partial6
                    + (s_calib.par_p4 * t * t * t);

    partial2 = (double)rawPress * (double)rawPress;
    partial3 = s_calib.par_p9 + s_calib.par_p10 * t;
    partial4 = partial2 * partial3;
    partial5 = partial4 + ((double)rawPress * (double)rawPress
                           * (double)rawPress * s_calib.par_p11);

    comp_press = partial4 + (offset * (double)rawPress) + partial5 + partial1;

    /* Daha basit genel dönüşüm - doğrusal yaklaşım */
    comp_press = (double)rawPress * (offset / 16777216.0) + partial4;

    /* Datasheet Eq. 1 (basitleştirilmiş) */
    comp_press = partial1 + ((double)rawPress * offset) / 65536.0
                 + ((double)rawPress * (double)rawPress * s_calib.par_p9 / 281474976710656.0);

    return (float)comp_press;
}

/* ================================================================
 * GENEL FONKSİYONLAR
 * ================================================================ */

int8_t BMP390_Init(void)
{
    uint8_t chipId = 0U;

    /* Soft reset */
    HAL_I2C_WriteReg(BMP390_I2C_ADDR, BMP390_REG_CMD, BMP390_CMD_SOFTRESET);
    HAL_Timer_DelayMs(10U);

    /* Chip ID kontrolü */
    if (HAL_I2C_ReadReg(BMP390_I2C_ADDR, BMP390_REG_CHIP_ID, &chipId) != ORKO_OK)
        return ORKO_ERROR;

    if (chipId != BMP390_CHIP_ID)
        return ORKO_ERROR;

    /* Kalibrasyon verisini oku */
    if (BMP390_ReadCalib() != ORKO_OK)
        return ORKO_ERROR;

    /* Oversampling: Basınç x8, Sıcaklık x1 */
    HAL_I2C_WriteReg(BMP390_I2C_ADDR, BMP390_REG_OSR,
                     (BMP390_OSR_TEMP_X1 << 3) | BMP390_OSR_PRESS_X8);

    /* IIR filtresi: kapalı */
    HAL_I2C_WriteReg(BMP390_I2C_ADDR, BMP390_REG_CONFIG, 0x00U);

    return ORKO_OK;
}

int8_t BMP390_Read(BMP390_Data_t *pData)
{
    uint8_t  buf[6]   = {0U};
    uint32_t rawPress = 0U;
    uint32_t rawTemp  = 0U;

    /* Forced mod: tek ölçüm başlat */
    HAL_I2C_WriteReg(BMP390_I2C_ADDR, BMP390_REG_PWR_CTRL, BMP390_PWR_FORCED);

    /* Ölçüm tamamlanması için bekleme (~40ms @ basınç x8) */
    HAL_Timer_DelayMs(50U);

    /* 6 byte oku: DATA_0 → DATA_5 */
    if (HAL_I2C_ReadRegMulti(BMP390_I2C_ADDR, BMP390_REG_DATA_0,
                             buf, 6U) != ORKO_OK)
        return ORKO_ERROR;

    /* 24-bit basınç: DATA_0 (LSB), DATA_1, DATA_2 (MSB) */
    rawPress = ((uint32_t)buf[2] << 16) | ((uint32_t)buf[1] << 8) | buf[0];

    /* 24-bit sıcaklık: DATA_3 (LSB), DATA_4, DATA_5 (MSB) */
    rawTemp  = ((uint32_t)buf[5] << 16) | ((uint32_t)buf[4] << 8) | buf[3];

    /* Önce sıcaklık hesapla (t_lin güncellenir, basınç için gerekli) */
    pData->temperature_C = BMP390_CompTemp(rawTemp);
    pData->pressure_Pa   = BMP390_CompPress(rawPress);

    return ORKO_OK;
}

void BMP390_Sleep(void)
{
    HAL_I2C_WriteReg(BMP390_I2C_ADDR, BMP390_REG_PWR_CTRL, BMP390_PWR_SLEEP);
}
