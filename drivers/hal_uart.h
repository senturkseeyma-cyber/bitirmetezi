/**
 * @file    hal_uart.h
 * @brief   UART Donanım Soyutlama Katmanı - Başlık Dosyası
 * @mcu     Nuvoton M031FB0AE
 *
 * UART0 → NEO-7M GPS (9600 baud)
 * UART1 → E22-900T22D LoRa (9600 baud)
 */

#ifndef HAL_UART_H
#define HAL_UART_H

#include "orko_config.h"
#include <stdint.h>

/* ================================================================
 * GPS UART (UART0) FONKSİYONLARI
 * ================================================================ */

/** @brief GPS UART0'ı başlatır */
void HAL_UART_GPS_Init(void);

/** @brief GPS UART0'ı durdurur */
void HAL_UART_GPS_DeInit(void);

/**
 * @brief  GPS UART0'dan 1 byte okur (blokaj)
 * @param  pByte : Okunan byte'ın yazılacağı pointer
 * @param  timeoutMs : Zaman aşımı (ms)
 * @return ORKO_OK / ORKO_TIMEOUT
 */
int8_t HAL_UART_GPS_ReadByte(uint8_t *pByte, uint32_t timeoutMs);

/**
 * @brief  GPS UART0'dan bir satır okur (\n'e kadar)
 * @param  pBuf   : Tampon
 * @param  maxLen : Maksimum karakter sayısı
 * @return Okunan karakter sayısı
 */
uint16_t HAL_UART_GPS_ReadLine(char *pBuf, uint16_t maxLen);

/* ================================================================
 * LORA UART (UART1) FONKSİYONLARI
 * ================================================================ */

/** @brief LoRa UART1'i başlatır */
void HAL_UART_LoRa_Init(void);

/** @brief LoRa UART1'i durdurur */
void HAL_UART_LoRa_DeInit(void);

/**
 * @brief  LoRa UART1'e veri yazar
 * @param  pData : Yazılacak veri tamponu
 * @param  len   : Byte sayısı
 * @return ORKO_OK / ORKO_ERROR
 */
int8_t HAL_UART_LoRa_Write(const uint8_t *pData, uint16_t len);

/**
 * @brief  LoRa UART1'den veri okur
 * @param  pData     : Veri tamponu
 * @param  maxLen    : Maksimum okunacak byte
 * @param  timeoutMs : Zaman aşımı (ms)
 * @return Okunan byte sayısı
 */
uint16_t HAL_UART_LoRa_Read(uint8_t *pData, uint16_t maxLen, uint32_t timeoutMs);

/**
 * @brief  LoRa RX tamponunu temizler
 */
void HAL_UART_LoRa_Flush(void);

#endif /* HAL_UART_H */
