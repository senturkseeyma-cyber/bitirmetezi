/**
 * @file    hal_i2c.c
 * @brief   I2C Donanım Soyutlama Katmanı - Kaynak Dosyası
 * @mcu     Nuvoton M031FB0AE
 */

#include "hal_i2c.h"

/* ================================================================
 * ÖZEL FONKSİYONLAR
 * ================================================================ */

/**
 * @brief I2C0 pin multiplexer konfigürasyonu
 *        PB.4 = SDA, PB.5 = SCL  (şematik pin 9, pin 8)
 */
static void I2C_PinConfig(void)
{
    SYS->GPB_MFPL = (SYS->GPB_MFPL
                     & ~(SYS_GPB_MFPL_PB4MFP_Msk | SYS_GPB_MFPL_PB5MFP_Msk))
                    | (SYS_GPB_MFPL_PB4MFP_I2C0_SDA | SYS_GPB_MFPL_PB5MFP_I2C0_SCL);
}

/* ================================================================
 * GENEL FONKSİYONLAR
 * ================================================================ */

void HAL_I2C_Init(void)
{
    /* Saat kaynağını etkinleştir */
    CLK_EnableModuleClock(I2C0_MODULE);

    /* Pin multiplexer ayarla */
    SYS_UnlockReg();
    I2C_PinConfig();
    SYS_LockReg();

    /* I2C aç - 400kHz Fast Mode */
    I2C_Open(ORKO_I2C_PORT, I2C_SPEED_HZ);
}

void HAL_I2C_DeInit(void)
{
    I2C_Close(ORKO_I2C_PORT);
    CLK_DisableModuleClock(I2C0_MODULE);
}

int8_t HAL_I2C_WriteByte(uint8_t slaveAddr, uint8_t data)
{
    uint32_t ret;
    ret = I2C_WriteByte(ORKO_I2C_PORT, slaveAddr, data);
    return (ret == 0) ? ORKO_OK : ORKO_ERROR;
}

int8_t HAL_I2C_WriteReg(uint8_t slaveAddr, uint8_t regAddr, uint8_t data)
{
    uint32_t ret;
    ret = I2C_WriteByteOneReg(ORKO_I2C_PORT, slaveAddr, regAddr, data);
    return (ret == 0) ? ORKO_OK : ORKO_ERROR;
}

int8_t HAL_I2C_WriteRegMulti(uint8_t slaveAddr, uint8_t regAddr,
                              const uint8_t *pData, uint16_t len)
{
    uint32_t ret;
    ret = I2C_WriteMultiBytesOneReg(ORKO_I2C_PORT, slaveAddr, regAddr,
                                    (uint8_t *)pData, (uint32_t)len);
    return (ret == len) ? ORKO_OK : ORKO_ERROR;
}

int8_t HAL_I2C_ReadReg(uint8_t slaveAddr, uint8_t regAddr, uint8_t *pData)
{
    *pData = I2C_ReadByteOneReg(ORKO_I2C_PORT, slaveAddr, regAddr);
    return ORKO_OK;
}

int8_t HAL_I2C_ReadRegMulti(uint8_t slaveAddr, uint8_t regAddr,
                             uint8_t *pData, uint16_t len)
{
    uint32_t ret;
    ret = I2C_ReadMultiBytesOneReg(ORKO_I2C_PORT, slaveAddr, regAddr,
                                   pData, (uint32_t)len);
    return (ret == len) ? ORKO_OK : ORKO_ERROR;
}

int8_t HAL_I2C_CmdRead(uint8_t slaveAddr,
                        const uint8_t *pCmd, uint16_t cmdLen,
                        uint8_t *pData,      uint16_t readLen)
{
    uint32_t ret;

    /* Komutu yaz (register adresi olmadan) */
    ret = I2C_WriteMultiBytes(ORKO_I2C_PORT, slaveAddr,
                              (uint8_t *)pCmd, (uint32_t)cmdLen);
    if (ret != cmdLen)
        return ORKO_ERROR;

    /* Veriyi oku */
    ret = I2C_ReadMultiBytes(ORKO_I2C_PORT, slaveAddr,
                             pData, (uint32_t)readLen);
    return (ret == readLen) ? ORKO_OK : ORKO_ERROR;
}
