/**
 * @file    hal_uart.c
 * @brief   UART Donanım Soyutlama Katmanı - Kaynak Dosyası
 * @mcu     Nuvoton M031FB0AE
 */

#include "hal_uart.h"
#include "hal_timer.h"

/* ================================================================
 * PIN KONFIGÜRASYONU
 * NOT: PCB şemanıza göre güncelleyiniz.
 * ================================================================ */

/**
 * UART0 Pin Konfigürasyonu: PF.2=RX, PF.3=TX (GPS)
 * şematik: pin 13 = UART0_RXD (PF.2), pin 12 = UART0_TXD (PF.3)
 */
static void UART0_PinConfig(void)
{
    SYS->GPF_MFPL = (SYS->GPF_MFPL
                     & ~(SYS_GPF_MFPL_PF2MFP_Msk | SYS_GPF_MFPL_PF3MFP_Msk))
                    | (SYS_GPF_MFPL_PF2MFP_UART0_RXD | SYS_GPF_MFPL_PF3MFP_UART0_TXD);
}

/**
 * UART1 Pin Konfigürasyonu: PA2=RX, PA3=TX (LoRa)
 */
static void UART1_PinConfig(void)
{
    SYS->GPA_MFPL = (SYS->GPA_MFPL
                     & ~(SYS_GPA_MFPL_PA2MFP_Msk | SYS_GPA_MFPL_PA3MFP_Msk))
                    | (SYS_GPA_MFPL_PA2MFP_UART1_RXD | SYS_GPA_MFPL_PA3MFP_UART1_TXD);
}

/* ================================================================
 * GPS UART (UART0)
 * ================================================================ */

void HAL_UART_GPS_Init(void)
{
    CLK_EnableModuleClock(UART0_MODULE);
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));

    SYS_UnlockReg();
    UART0_PinConfig();
    SYS_LockReg();

    UART_Open(GPS_UART_PORT, GPS_UART_BAUD);
}

void HAL_UART_GPS_DeInit(void)
{
    UART_Close(GPS_UART_PORT);
    CLK_DisableModuleClock(UART0_MODULE);
}

int8_t HAL_UART_GPS_ReadByte(uint8_t *pByte, uint32_t timeoutMs)
{
    uint32_t start = HAL_Timer_GetTick();

    while (!UART_IS_RX_READY(GPS_UART_PORT))
    {
        if ((HAL_Timer_GetTick() - start) >= timeoutMs)
            return ORKO_TIMEOUT;
    }

    *pByte = (uint8_t)UART_READ(GPS_UART_PORT);
    return ORKO_OK;
}

uint16_t HAL_UART_GPS_ReadLine(char *pBuf, uint16_t maxLen)
{
    uint16_t idx = 0;
    uint8_t  ch  = 0;

    while (idx < (maxLen - 1U))
    {
        if (HAL_UART_GPS_ReadByte(&ch, 1000U) != ORKO_OK)
            break;

        pBuf[idx++] = (char)ch;

        if (ch == '\n')
            break;
    }

    pBuf[idx] = '\0';
    return idx;
}

/* ================================================================
 * LORA UART (UART1)
 * ================================================================ */

void HAL_UART_LoRa_Init(void)
{
    CLK_EnableModuleClock(UART1_MODULE);
    CLK_SetModuleClock(UART1_MODULE, CLK_CLKSEL1_UART1SEL_HIRC, CLK_CLKDIV0_UART1(1));

    SYS_UnlockReg();
    UART1_PinConfig();
    SYS_LockReg();

    UART_Open(LORA_UART_PORT, LORA_UART_BAUD);
}

void HAL_UART_LoRa_DeInit(void)
{
    UART_Close(LORA_UART_PORT);
    CLK_DisableModuleClock(UART1_MODULE);
}

int8_t HAL_UART_LoRa_Write(const uint8_t *pData, uint16_t len)
{
    UART_Write(LORA_UART_PORT, (uint8_t *)pData, (uint32_t)len);
    return ORKO_OK;
}

uint16_t HAL_UART_LoRa_Read(uint8_t *pData, uint16_t maxLen, uint32_t timeoutMs)
{
    uint16_t idx   = 0;
    uint32_t start = HAL_Timer_GetTick();

    while (idx < maxLen)
    {
        if (UART_IS_RX_READY(LORA_UART_PORT))
        {
            pData[idx++] = (uint8_t)UART_READ(LORA_UART_PORT);
            start = HAL_Timer_GetTick(); /* Her byte sonrası timeout sıfırla */
        }
        else if ((HAL_Timer_GetTick() - start) >= timeoutMs)
        {
            break;
        }
    }

    return idx;
}

void HAL_UART_LoRa_Flush(void)
{
    while (UART_IS_RX_READY(LORA_UART_PORT))
        (void)UART_READ(LORA_UART_PORT);
}
