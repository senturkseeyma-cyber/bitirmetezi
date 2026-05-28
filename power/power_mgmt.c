/**
 * @file    power_mgmt.c
 * @brief   Güç Yönetimi Modülü - Kaynak
 * @mcu     Nuvoton M031FB0AE
 */

#include "power_mgmt.h"
#include "lora_e22.h"

/* ================================================================
 * STATİK DEĞİŞKENLER
 * ================================================================ */
static uint32_t s_uptimeSec = 0U;

/* ================================================================
 * GENEL FONKSİYONLAR
 * ================================================================ */

void PowerMgmt_Init(void)
{
    s_uptimeSec = 0U;

    /* M031 güç yönetimi: Brown-out dedektörü seviyesini ayarla */
    SYS_UnlockReg();
    /* BOD: 2.7V kesim seviyesi */
    SYS->BODCTL = SYS_BODCTL_BODEN_Msk | SYS_BODCTL_BODVL_2_7V;
    SYS_LockReg();
}

void PowerMgmt_Sleep(SleepMode_t mode, uint32_t seconds)
{
    uint32_t elapsed = 0U;
    uint32_t startSec = HAL_Timer_GetSeconds();

    if (mode == SLEEP_IDLE)
    {
        /* Idle mod: CPU durur, SysTick + Timer0 çalışır */
        while (elapsed < seconds)
        {
            CLK_Idle();
            elapsed = HAL_Timer_GetSeconds() - startSec;
        }
    }
    else  /* SLEEP_POWERDOWN */
    {
        /* Power-down mod: Timer0 kesmesi ile periyodik uyanma
         * Timer0 1Hz → her saniye bir kesme → sayaç ile kontrol */
        HAL_Timer_ResetSeconds();

        while (HAL_Timer_GetSeconds() < seconds)
        {
            /* CPU'yu durdur; Timer0 kesmesi gelince otomatik devam eder */
            CLK_PowerDown();

            /* Uptime'ı güncelle */
            s_uptimeSec++;
        }
    }
}

void PowerMgmt_SensorsOff(void)
{
    /* PCB'de sensör güç anahtarı (MOSFET) varsa buraya GPIO LOW yaz.
     * Mevcut PCB'de doğrudan besleme bağlıysa bu fonksiyon boş kalır. */
}

void PowerMgmt_SensorsOn(void)
{
    /* Sensörleri aç ve stabilizasyon bekle */
    HAL_Timer_DelayMs(50U);
}

void PowerMgmt_LoRaSleep(void)
{
    LoRa_SetDeepSleep();
}

void PowerMgmt_EnterFullSleep(void)
{
    /* Tüm modüller uyku moduna al */
    PowerMgmt_LoRaSleep();
    PowerMgmt_SensorsOff();
    /* LED kapat */
    LED_OFF();
}

uint32_t PowerMgmt_GetUptime(void)
{
    return HAL_Timer_GetSeconds();
}
