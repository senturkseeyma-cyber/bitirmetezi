/**
 * @file    main.c
 * @brief   ORKO Slave Node - Ana Uygulama
 * @project ORKO - Orman Yangını Erken Tespit ve Koruma Sistemi
 * @board   Slave Kart (Slave Node #1)
 * @mcu     Nuvoton M031FB0AE (ARM Cortex-M0 @ 48MHz)
 *
 * ================================================================
 * SİSTEM AKIŞ DİYAGRAMI (Tez Bölüm 3.1)
 * ================================================================
 *
 *  [BAŞLAT]
 *     │
 *     ▼
 *  [Sistem İnit: MCU, Timer, I2C, UART, ADC]
 *     │
 *     ▼
 *  [Sensörler İnit: BMP390, SCD40, HDC2022, MQ9, GPS]
 *     │
 *     ▼
 *  [LoRa İnit: E22 konfigüre et]
 *     │
 *     ▼
 *  ┌──[NÖBETÇİ DÖNGÜSÜ: her 2 dakika]───────────────────────┐
 *  │  SCD40 tek atış ölçüm                                   │
 *  │  CO2 kritik eşik > 5000 ppm?                            │
 *  │  HAYIR → sleep 2 dakika → tekrar                        │
 *  │  EVET  ↓                                                │
 *  │  [TAM ÖLÇÜM DÖNGÜSÜ: her 10 dakika, 2 saat boyunca]    │
 *  │    Tüm sensörleri oku (HDC2022, BMP390, SCD40, MQ9, GPS)│
 *  │    FireScore hesapla                                    │
 *  │    Alarm koşulu sağlandı mı?                            │
 *  │    EVET → LoRa ile Master'a ALARM paketi gönder         │
 *  │    HAYIR → 10 dakika bekle                              │
 *  │    2 saat doldu? → DURUM paketi gönder + hafızayı temizle│
 *  └─────────────────────────────────────────────────────────┘
 *
 * ================================================================
 */

#include "NuMicro.h"
#include "orko_config.h"
#include "hal_i2c.h"
#include "hal_uart.h"
#include "hal_adc.h"
#include "hal_timer.h"
#include "drv_bmp390.h"
#include "drv_scd40.h"
#include "drv_hdc2022.h"
#include "drv_mq9.h"
#include "drv_neo7m.h"
#include "lora_e22.h"
#include "fire_score.h"
#include "power_mgmt.h"

/* ================================================================
 * SİSTEM DURUM MAKİNESİ
 * ================================================================ */
typedef enum {
    STATE_INIT        = 0U,  /**< Başlangıç konfigürasyonu */
    STATE_SENTINEL    = 1U,  /**< Nöbetçi modu (SCD40 periyodik kontrol) */
    STATE_FULL_READ   = 2U,  /**< Tam sensör okuması */
    STATE_ALARM       = 3U,  /**< Alarm gönderme */
    STATE_STATUS_SEND = 4U,  /**< Periyodik durum raporu (2 saatte bir) */
    STATE_SLEEP       = 5U,  /**< Uyku modu */
} SystemState_t;

/* ================================================================
 * GLOBAL DEĞİŞKENLER
 * ================================================================ */
static SystemState_t    s_state        = STATE_INIT;
static SensorBuffer_t   s_sensorBuf;
static ORKO_Packet_t    s_txPacket;

/* Zamanlayıcılar */
static uint32_t         s_sentinelTimer   = 0U;  /**< Nöbetçi zaman sayacı (sn) */
static uint32_t         s_fullReadTimer   = 0U;  /**< Tam okuma sayacı (sn) */
static uint32_t         s_cycleTimer      = 0U;  /**< 2 saatlik döngü sayacı (sn) */

/* ================================================================
 * SİSTEM BAŞLATMA FONKSİYONLARI
 * ================================================================ */

/**
 * @brief MCU saat ve güç konfigürasyonu
 *        HIRC (48MHz) dahili RC osilatör
 */
static void SYS_Init(void)
{
    SYS_UnlockReg();

    /* HIRC (48MHz) dahili RC etkinleştir ve hazır olana kadar bekle */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);

    /* CPU çekirdeğini 48MHz'e ayarla */
    CLK_SetCoreClock(SYSTEM_CORE_CLOCK_HZ);

    /* HCLK kaynağı: HIRC */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));

    SYS_LockReg();

    /* SystemCoreClock global değişkenini güncelle */
    SystemCoreClockUpdate();
}

/**
 * @brief LED GPIO konfigürasyonu
 *        PB.13 = DigitalOut (şematik pin 5)
 */
static void LED_Init(void)
{
    GPIO_SetMode(PB, BIT13, GPIO_MODE_OUTPUT);
    LED_OFF();
}

/**
 * @brief Başlangıç LED sinyali (3 kez yanıp söner)
 */
static void LED_StartupBlink(void)
{
    uint8_t i;
    for (i = 0U; i < 3U; i++)
    {
        LED_ON();
        HAL_Timer_DelayMs(200U);
        LED_OFF();
        HAL_Timer_DelayMs(200U);
    }
}

/* ================================================================
 * VERİ OKUMA FONKSİYONLARI
 * ================================================================ */

/**
 * @brief Tüm sensörleri okur ve tampona ekler
 * @return ORKO_OK / ORKO_ERROR
 */
static int8_t ReadAllSensors(void)
{
    HDC2022_Data_t hdc  = {0};
    BMP390_Data_t  bmp  = {0};
    SCD40_Data_t   scd  = {0};
    MQ9_Data_t     mq9  = {0};
    GPS_Data_t     gps  = {0};
    int8_t         ret  = ORKO_OK;

    /* HDC2022: Sıcaklık + Nem */
    if (HDC2022_Read(&hdc) != ORKO_OK)
    {
        hdc.temperature_C = TEMP_AVERAGE_C;
        hdc.humidity_pct  = HUMIDITY_AVERAGE_PCT;
        ret = ORKO_ERROR;
    }

    /* BMP390: Basınç */
    if (BMP390_Read(&bmp) != ORKO_OK)
    {
        bmp.pressure_Pa   = PRESSURE_AVERAGE_PA;
        bmp.temperature_C = TEMP_AVERAGE_C;
        ret = ORKO_ERROR;
    }

    /* SCD40: CO2 (tek atış) */
    if (SCD40_StartSingleShot() == ORKO_OK)
    {
        SCD40_Read(&scd);
    }
    else
    {
        scd.co2_ppm = (uint16_t)CO2_AVERAGE_PPM;
        ret = ORKO_ERROR;
    }

    /* MQ9: CO */
    MQ9_Read(&mq9);

    /* GPS: Konum */
    GPS_Read(&gps, 30U);  /* 30 deneme */

    /* Fire Score tamponuna ekle */
    FireScore_AddSample(&s_sensorBuf,
                        hdc.temperature_C,
                        (float)scd.co2_ppm,
                        mq9.co_ppm,
                        bmp.pressure_Pa,
                        hdc.humidity_pct);

    /* TX paketini doldur */
    s_txPacket.slaveAddr     = ORKO_SLAVE_ADDRESS;
    s_txPacket.masterAddr    = ORKO_MASTER_ADDRESS;
    s_txPacket.channel       = LORA_CHANNEL;
    s_txPacket.temperature_C = hdc.temperature_C;
    s_txPacket.humidity_pct  = hdc.humidity_pct;
    s_txPacket.pressure_Pa   = bmp.pressure_Pa;
    s_txPacket.co2_ppm       = scd.co2_ppm;
    s_txPacket.co_ppm        = mq9.co_ppm;
    s_txPacket.latitude      = gps.latitude;
    s_txPacket.longitude     = gps.longitude;
    s_txPacket.altitude_m    = gps.altitude_m;
    s_txPacket.gps_valid     = gps.fix_valid;
    s_txPacket.uptime_sec    = PowerMgmt_GetUptime();

    return ret;
}

/* ================================================================
 * DURUM MAKİNESİ İŞLEMCİLERİ
 * ================================================================ */

/**
 * @brief STATE_INIT: Tüm donanımı başlat
 */
static SystemState_t Handle_Init(void)
{
    int8_t err = ORKO_OK;

    /* Donanım başlat */
    SYS_Init();
    LED_Init();
    HAL_Timer_Init();
    PowerMgmt_Init();

    /* I2C ve UART başlat */
    HAL_I2C_Init();
    HAL_UART_GPS_Init();

    /* Sensörleri başlat */
    if (BMP390_Init()  != ORKO_OK) err = ORKO_ERROR;
    if (SCD40_Init()   != ORKO_OK) err = ORKO_ERROR;
    if (HDC2022_Init() != ORKO_OK) err = ORKO_ERROR;
    MQ9_Init();
    GPS_Init();

    /* LoRa başlat */
    if (LoRa_Init() != ORKO_OK) err = ORKO_ERROR;

    /* Fire Score tamponunu sıfırla */
    FireScore_BufferInit(&s_sensorBuf);

    /* Hata durumunda hızlı yanıp söner */
    if (err != ORKO_OK)
    {
        uint8_t i;
        for (i = 0U; i < 6U; i++)
        {
            LED_TOGGLE();
            HAL_Timer_DelayMs(100U);
        }
    }
    else
    {
        LED_StartupBlink();
    }

    /* Zamanlayıcıları başlat */
    s_sentinelTimer = HAL_Timer_GetSeconds();
    s_fullReadTimer = HAL_Timer_GetSeconds();
    s_cycleTimer    = HAL_Timer_GetSeconds();

    return STATE_SENTINEL;
}

/**
 * @brief STATE_SENTINEL: Her 2 dakikada SCD40 nöbetçi ölçümü
 */
static SystemState_t Handle_Sentinel(void)
{
    uint16_t co2 = 0U;
    uint32_t elapsed;

    /* 2 dakika dolmadıysa uyku */
    elapsed = HAL_Timer_GetSeconds() - s_sentinelTimer;
    if (elapsed < SENTINEL_INTERVAL_SEC)
    {
        /* Kalan süre kadar uyku */
        uint32_t remaining = SENTINEL_INTERVAL_SEC - elapsed;
        PowerMgmt_Sleep(SLEEP_POWERDOWN, remaining);
    }

    /* Nöbetçi zamanı sıfırla */
    s_sentinelTimer = HAL_Timer_GetSeconds();

    /* SCD40 hızlı CO2 ölçümü */
    SCD40_ReadCO2(&co2);

    LED_ON();
    HAL_Timer_DelayMs(50U);
    LED_OFF();

    /* CO2 kritik eşiği aştı mı? */
    if (FireScore_IsCO2Critical(co2))
    {
        /* Acil tam ölçüme geç */
        return STATE_FULL_READ;
    }

    /* Normal tam okuma zamanı geldi mi? (10 dakika) */
    elapsed = HAL_Timer_GetSeconds() - s_fullReadTimer;
    if (elapsed >= FULL_READ_INTERVAL_SEC)
    {
        return STATE_FULL_READ;
    }

    /* Henüz değil → nöbetçiye devam */
    return STATE_SENTINEL;
}

/**
 * @brief STATE_FULL_READ: Tüm sensörleri oku, Fire Score hesapla
 */
static SystemState_t Handle_FullRead(void)
{
    FireScore_Result_t result = {0};

    /* Tam sensör okuması */
    ReadAllSensors();

    /* Full read zamanlayıcısını sıfırla */
    s_fullReadTimer = HAL_Timer_GetSeconds();

    /* Fire Score hesapla */
    if (FireScore_Calculate(&s_sensorBuf, &result) != ORKO_OK)
    {
        /* Yetersiz örnek — nöbetçiye dön */
        return STATE_SENTINEL;
    }

    /* TX paketine Fire Score yaz */
    s_txPacket.fire_score = result.fire_score;
    s_txPacket.alarm_flag = result.alarm;

    /* Alarm koşulu sağlandı mı? */
    if (result.alarm == 1U)
    {
        s_txPacket.packetType = PKT_TYPE_ALARM;
        LED_ON(); /* Alarm süresince LED açık */
        return STATE_ALARM;
    }

    /* 2 saatlik döngü doldu mu? → Durum raporu */
    uint32_t cycleElapsed = HAL_Timer_GetSeconds() - s_cycleTimer;
    if (cycleElapsed >= CYCLE_DURATION_SEC)
    {
        s_txPacket.packetType = PKT_TYPE_STATUS;
        return STATE_STATUS_SEND;
    }

    /* Normal → nöbetçiye dön */
    return STATE_SENTINEL;
}

/**
 * @brief STATE_ALARM: Master'a alarm paketi gönder
 */
static SystemState_t Handle_Alarm(void)
{
    /* Alarm paketini gönder */
    LoRa_SetNormal();
    LoRa_SendPacket(&s_txPacket);

    /* Alarm LED: 5 kez yanıp söner */
    uint8_t i;
    for (i = 0U; i < 5U; i++)
    {
        LED_ON();
        HAL_Timer_DelayMs(200U);
        LED_OFF();
        HAL_Timer_DelayMs(200U);
    }

    /* Kısa bekleme sonrası ölçüme devam et */
    HAL_Timer_DelayMs(5000U);

    return STATE_FULL_READ;
}

/**
 * @brief STATE_STATUS_SEND: 2 saatlik periyodik durum raporu
 */
static SystemState_t Handle_StatusSend(void)
{
    /* Durum paketini gönder */
    LoRa_SetNormal();
    LoRa_SendPacket(&s_txPacket);

    /* 2 saatlik döngü zamanlayıcısını sıfırla */
    s_cycleTimer = HAL_Timer_GetSeconds();

    /* Fire Score tamponunu temizle (tez: ilk 7 kayıt silinir) */
    FireScore_BufferInit(&s_sensorBuf);

    LED_ON();
    HAL_Timer_DelayMs(500U);
    LED_OFF();

    return STATE_SENTINEL;
}

/* ================================================================
 * ANA FONKSİYON
 * ================================================================ */

int main(void)
{
    /* Durum makinesini çalıştır */
    while (1)
    {
        switch (s_state)
        {
            case STATE_INIT:
                s_state = Handle_Init();
                break;

            case STATE_SENTINEL:
                s_state = Handle_Sentinel();
                break;

            case STATE_FULL_READ:
                s_state = Handle_FullRead();
                break;

            case STATE_ALARM:
                s_state = Handle_Alarm();
                break;

            case STATE_STATUS_SEND:
                s_state = Handle_StatusSend();
                break;

            case STATE_SLEEP:
            default:
                /* Beklenmedik durum — yeniden başlat */
                s_state = STATE_INIT;
                break;
        }
    }

    /* Buraya asla ulaşılmamalı */
    return 0;
}
