/**
 * @file    lora_e22.c
 * @brief   E22-900T22D LoRa Modülü Sürücüsü - Kaynak
 *
 * E22-900T22D Konfigürasyon Paketi (9 byte):
 * [C0][ADDH][ADDL][NETID][REG0][REG1][REG2][REG3][CRYPT_H][CRYPT_L]
 * REG0: baud + parity + air data rate
 * REG1: sub-packet size + RSSI ambient + power
 * REG2: channel (0-80, freq = 850.125 + CH * 1MHz)
 * REG3: RSSI byte + transmission method + WOR + LBT
 */

#include "lora_e22.h"

/* ================================================================
 * GPIO YARDIMCILARI
 * ================================================================ */

static void LoRa_SetM0(uint8_t val)
{
    PB12 = val ? 1U : 0U;   /* PB.12 - şematik pin 6 */
}

static void LoRa_SetM1(uint8_t val)
{
    /* PF.6 = NRESET / M1 - şematik pin 18
     * Bu pine yazmak için SYS_UnlockReg() önceden çağrılmalıdır */
    PF6 = val ? 1U : 0U;    /* PF.6 (NRESET/GPIO) */
}

static uint8_t LoRa_ReadAUX(void)
{
    return (uint8_t)PA1;    /* PA.1 - şematik pin 16 */
}

/* ================================================================
 * MOD DEĞIŞTIRME
 * ================================================================ */

/**
 * @brief E22'yi Konfigürasyon moduna al (M0=1, M1=0)
 */
static void LoRa_EnterConfigMode(void)
{
    LoRa_SetM0(1U);
    LoRa_SetM1(0U);
    HAL_Timer_DelayMs(E22_CONFIG_WAIT_MS);
    LoRa_WaitAUX();
}

/**
 * @brief E22'yi Normal iletim moduna al (M0=0, M1=0)
 */
static void LoRa_EnterNormalMode(void)
{
    LoRa_SetM0(0U);
    LoRa_SetM1(0U);
    HAL_Timer_DelayMs(E22_CONFIG_WAIT_MS);
    LoRa_WaitAUX();
}

/* ================================================================
 * GENEL FONKSİYONLAR
 * ================================================================ */

int8_t LoRa_WaitAUX(void)
{
    uint32_t start = HAL_Timer_GetTick();

    while (LoRa_ReadAUX() == 0U)
    {
        if (HAL_Timer_Elapsed(start) >= E22_AUX_READY_TIMEOUT_MS)
            return ORKO_TIMEOUT;
    }

    return ORKO_OK;
}

int8_t LoRa_Init(void)
{
    /* M0 = PB.12 çıkış */
    GPIO_SetMode(PB, BIT12, GPIO_MODE_OUTPUT);
    /* M1 = PF.6 (NRESET pin) çıkış - GPIO olarak kullanmak için unlock gerekir */
    SYS_UnlockReg();
    GPIO_SetMode(PF, BIT6, GPIO_MODE_OUTPUT);
    SYS_LockReg();
    /* AUX = PA.1 giriş */
    GPIO_SetMode(PA, BIT1, GPIO_MODE_INPUT);

    /* UART1 başlat */
    HAL_UART_LoRa_Init();

    /* Modülün açılış bekleme süresi */
    HAL_Timer_DelayMs(500U);
    if (LoRa_WaitAUX() != ORKO_OK)
        return ORKO_ERROR;

    /* Konfigürasyon moduna gir */
    LoRa_EnterConfigMode();

    /**
     * E22 Konfigürasyon Paketi:
     * [C0][ADDH][ADDL][NETID][REG0][REG1][REG2][REG3][CRYPT_H][CRYPT_L]
     *
     * ADDH = High byte of slave address
     * ADDL = Low byte of slave address
     * NETID = 0x00 (aynı ağda)
     * REG0  = 0x62: UART 9600 + 8N1 + Hava hızı 2.4kbps
     * REG1  = 0x00: Sub-packet 200B, RSSI kapalı, Max güç (+22dBm)
     * REG2  = 0x06: Kanal 6 → 856.125 MHz (868 bandı)
     *         LORA_CHANNEL = 0 ise 0x00 → 850.125 MHz
     * REG3  = 0x03: RSSI byte etkin, transparent mod, WOR 500ms, LBT kapalı
     * CRYPT = 0x0000 (şifreleme yok)
     */
    uint8_t cfg[10] = {
        E22_CONFIG_CMD,                           /* Kayıt et */
        (uint8_t)(ORKO_SLAVE_ADDRESS >> 8),       /* ADDH */
        (uint8_t)(ORKO_SLAVE_ADDRESS & 0xFFU),    /* ADDL */
        (uint8_t)LORA_NET_ID,                     /* NETID */
        0x62U,  /* REG0: 9600 baud, 8N1, 2.4kbps */
        0x00U,  /* REG1: 200B sub-packet, max güç */
        (uint8_t)LORA_CHANNEL,                    /* REG2: kanal */
        0x03U,  /* REG3: RSSI etkin, transparent */
        0x00U,  /* CRYPT_H */
        0x00U   /* CRYPT_L */
    };

    HAL_UART_LoRa_Write(cfg, sizeof(cfg));
    HAL_Timer_DelayMs(500U);

    if (LoRa_WaitAUX() != ORKO_OK)
        return ORKO_ERROR;

    /* Normal moda geri dön */
    LoRa_EnterNormalMode();

    /* RX tamponunu temizle */
    HAL_UART_LoRa_Flush();

    return ORKO_OK;
}

int8_t LoRa_SendPacket(const ORKO_Packet_t *pPacket)
{
    int8_t  ret;
    uint8_t *pRaw = (uint8_t *)pPacket;

    /* AUX hazır mı? */
    if (LoRa_WaitAUX() != ORKO_OK)
        return ORKO_TIMEOUT;

    /* Paketi gönder */
    ret = HAL_UART_LoRa_Write(pRaw, (uint16_t)sizeof(ORKO_Packet_t));

    /* Gönderim tamamlanana kadar bekle */
    HAL_Timer_DelayMs(500U);
    LoRa_WaitAUX();

    return ret;
}

uint8_t LoRa_DataAvailable(void)
{
    return UART_IS_RX_READY(LORA_UART_PORT) ? ORKO_TRUE : ORKO_FALSE;
}

uint16_t LoRa_Receive(uint8_t *pBuf, uint16_t maxLen)
{
    return HAL_UART_LoRa_Read(pBuf, maxLen, 500U);
}

void LoRa_SetWOR(void)
{
    LoRa_SetM0(0U);
    LoRa_SetM1(1U);
    HAL_Timer_DelayMs(E22_CONFIG_WAIT_MS);
}

void LoRa_SetNormal(void)
{
    LoRa_EnterNormalMode();
}

void LoRa_SetDeepSleep(void)
{
    LoRa_SetM0(1U);
    LoRa_SetM1(1U);
    HAL_Timer_DelayMs(E22_CONFIG_WAIT_MS);
}
