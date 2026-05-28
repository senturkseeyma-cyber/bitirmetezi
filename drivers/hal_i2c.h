/**
 * @file    hal_i2c.h
 * @brief   I2C Donanım Soyutlama Katmanı - Başlık Dosyası
 * @mcu     Nuvoton M031FB0AE
 *
 * Kullanım: BMP390, SCD40, HDC2022 sensörleri için I2C0 üzerinden
 *           okuma/yazma işlemlerini soyutlar.
 */

#ifndef HAL_I2C_H
#define HAL_I2C_H

#include "orko_config.h"
#include <stdint.h>

/**
 * @brief I2C modülünü başlatır (400kHz Fast Mode)
 */
void HAL_I2C_Init(void);

/**
 * @brief I2C modülünü durdurur
 */
void HAL_I2C_DeInit(void);

/**
 * @brief  Belirtilen slave adresine 1 byte yazar
 * @param  slaveAddr : 7-bit I2C slave adresi
 * @param  data      : Yazılacak byte
 * @return ORKO_OK / ORKO_ERROR
 */
int8_t HAL_I2C_WriteByte(uint8_t slaveAddr, uint8_t data);

/**
 * @brief  Belirtilen register adresine 1 byte yazar
 * @param  slaveAddr : 7-bit I2C slave adresi
 * @param  regAddr   : Register adresi
 * @param  data      : Yazılacak byte
 * @return ORKO_OK / ORKO_ERROR
 */
int8_t HAL_I2C_WriteReg(uint8_t slaveAddr, uint8_t regAddr, uint8_t data);

/**
 * @brief  Belirtilen register adresine birden fazla byte yazar
 * @param  slaveAddr : 7-bit I2C slave adresi
 * @param  regAddr   : Başlangıç register adresi
 * @param  pData     : Yazılacak veri tamponu
 * @param  len       : Byte sayısı
 * @return ORKO_OK / ORKO_ERROR
 */
int8_t HAL_I2C_WriteRegMulti(uint8_t slaveAddr, uint8_t regAddr,
                              const uint8_t *pData, uint16_t len);

/**
 * @brief  Belirtilen register adresinden 1 byte okur
 * @param  slaveAddr : 7-bit I2C slave adresi
 * @param  regAddr   : Register adresi
 * @param  pData     : Okunan byte'ın yazılacağı pointer
 * @return ORKO_OK / ORKO_ERROR
 */
int8_t HAL_I2C_ReadReg(uint8_t slaveAddr, uint8_t regAddr, uint8_t *pData);

/**
 * @brief  Belirtilen register adresinden birden fazla byte okur
 * @param  slaveAddr : 7-bit I2C slave adresi
 * @param  regAddr   : Başlangıç register adresi
 * @param  pData     : Veri tamponu
 * @param  len       : Okunacak byte sayısı
 * @return ORKO_OK / ORKO_ERROR
 */
int8_t HAL_I2C_ReadRegMulti(uint8_t slaveAddr, uint8_t regAddr,
                             uint8_t *pData, uint16_t len);

/**
 * @brief  Komut gönderdikten sonra doğrudan okuma yapar (SCD40 için)
 * @param  slaveAddr : 7-bit I2C slave adresi
 * @param  pCmd      : Gönderilecek komut tamponu
 * @param  cmdLen    : Komut byte sayısı
 * @param  pData     : Okunacak veri tamponu
 * @param  readLen   : Okunacak byte sayısı
 * @return ORKO_OK / ORKO_ERROR
 */
int8_t HAL_I2C_CmdRead(uint8_t slaveAddr,
                        const uint8_t *pCmd, uint16_t cmdLen,
                        uint8_t *pData,      uint16_t readLen);

#endif /* HAL_I2C_H */
