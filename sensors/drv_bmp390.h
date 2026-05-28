/**
 * @file    drv_bmp390.h
 * @brief   BMP390 Basınç ve Sıcaklık Sensörü Sürücüsü - Başlık
 * @datasheet Bosch BMP390
 * @interface I2C
 */

#ifndef DRV_BMP390_H
#define DRV_BMP390_H

#include "orko_config.h"
#include "hal_i2c.h"
#include <stdint.h>

/* ================================================================
 * BMP390 REGISTER ADRESLERI
 * ================================================================ */
#define BMP390_REG_CHIP_ID        0x00U  /**< Chip ID = 0x60 */
#define BMP390_REG_ERR            0x02U
#define BMP390_REG_STATUS         0x03U
#define BMP390_REG_DATA_0         0x04U  /**< Basınç LSB */
#define BMP390_REG_DATA_1         0x05U
#define BMP390_REG_DATA_2         0x06U  /**< Basınç MSB */
#define BMP390_REG_DATA_3         0x07U  /**< Sıcaklık LSB */
#define BMP390_REG_DATA_4         0x08U
#define BMP390_REG_DATA_5         0x09U  /**< Sıcaklık MSB */
#define BMP390_REG_INT_STATUS     0x11U
#define BMP390_REG_FIFO_LENGTH    0x12U
#define BMP390_REG_FIFO_DATA      0x14U
#define BMP390_REG_FIFO_WTM_0     0x15U
#define BMP390_REG_FIFO_CONFIG_1  0x17U
#define BMP390_REG_FIFO_CONFIG_2  0x18U
#define BMP390_REG_INT_CTRL       0x19U
#define BMP390_REG_IF_CONF        0x1AU
#define BMP390_REG_PWR_CTRL       0x1BU  /**< Güç kontrolü */
#define BMP390_REG_OSR            0x1CU  /**< Oversampling */
#define BMP390_REG_ODR            0x1DU  /**< Çıkış veri hızı */
#define BMP390_REG_CONFIG         0x1FU  /**< IIR filtresi */
#define BMP390_REG_CALIB_DATA     0x31U  /**< Kalibrasyon başlangıcı */
#define BMP390_REG_CMD            0x7EU  /**< Komut */

/* Chip ID */
#define BMP390_CHIP_ID            0x60U

/* Güç modu */
#define BMP390_PWR_SLEEP          0x00U
#define BMP390_PWR_FORCED         0x13U  /**< Basınç + Sıcaklık etkin, Forced mod */
#define BMP390_PWR_NORMAL         0x33U  /**< Normal mod */

/* Oversampling */
#define BMP390_OSR_PRESS_X8       0x03U
#define BMP390_OSR_TEMP_X1        0x00U

/* Soft reset */
#define BMP390_CMD_SOFTRESET      0xB6U

/* ================================================================
 * KALİBRASYON VERİSİ YAPISI
 * ================================================================ */
typedef struct {
    /* Sıcaklık kalibrasyon katsayıları */
    double par_t1;
    double par_t2;
    double par_t3;
    /* Basınç kalibrasyon katsayıları */
    double par_p1;
    double par_p2;
    double par_p3;
    double par_p4;
    double par_p5;
    double par_p6;
    double par_p7;
    double par_p8;
    double par_p9;
    double par_p10;
    double par_p11;
    double t_lin;          /**< Ara sıcaklık değeri */
} BMP390_Calib_t;

/* ================================================================
 * VERİ YAPISI
 * ================================================================ */
typedef struct {
    float pressure_Pa;     /**< Basınç (Pascal) */
    float temperature_C;   /**< Sıcaklık (°C) */
} BMP390_Data_t;

/* ================================================================
 * FONKSİYONLAR
 * ================================================================ */

/**
 * @brief BMP390'ı başlatır, kimliği doğrular, kalibrasyon okur
 * @return ORKO_OK / ORKO_ERROR
 */
int8_t BMP390_Init(void);

/**
 * @brief  Basınç ve sıcaklık verisini okur
 * @param  pData : Ölçüm sonucunun yazılacağı yapı
 * @return ORKO_OK / ORKO_ERROR
 */
int8_t BMP390_Read(BMP390_Data_t *pData);

/**
 * @brief Sensörü uyku moduna alır
 */
void BMP390_Sleep(void);

#endif /* DRV_BMP390_H */
