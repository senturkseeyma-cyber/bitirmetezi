/**
 * @file    hal_timer.c
 * @brief   Timer / SysTick Soyutlama Katmanı - Kaynak Dosyası
 * @mcu     Nuvoton M031FB0AE
 */

#include "hal_timer.h"

/* ================================================================
 * GLOBAL DEĞİŞKENLER
 * ================================================================ */
volatile static uint32_t s_u32MsTick  = 0U;   /**< SysTick ms sayacı */
volatile static uint32_t s_u32SecTick = 0U;    /**< Saniye sayacı */

/* ================================================================
 * KESİM SERVISLERI
 * ================================================================ */

/** SysTick kesmesi — her 1ms'de bir çalışır */
void SysTick_Handler(void)
{
    s_u32MsTick++;
}

/** Timer0 kesmesi — her 1 saniyede bir çalışır */
void TMR0_IRQHandler(void)
{
    if (TIMER_GetIntFlag(TIMER0))
    {
        TIMER_ClearIntFlag(TIMER0);
        s_u32SecTick++;
    }
}

/* ================================================================
 * GENEL FONKSİYONLAR
 * ================================================================ */

void HAL_Timer_Init(void)
{
    /* SysTick: 1ms periyot */
    SysTick_Config(SYSTEM_CORE_CLOCK_HZ / 1000UL);

    /* Timer0: 1Hz (1 saniyede bir kesme) */
    CLK_EnableModuleClock(TMR0_MODULE);
    CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HIRC, 0U);

    TIMER_Open(TIMER0, TIMER_PERIODIC_MODE, 1U); /* 1Hz */
    TIMER_EnableInt(TIMER0);
    NVIC_EnableIRQ(TMR0_IRQn);
    TIMER_Start(TIMER0);
}

void HAL_Timer_DelayMs(uint32_t ms)
{
    uint32_t tickstart = s_u32MsTick;

    while ((s_u32MsTick - tickstart) < ms)
    {
        /* Bekle */
    }
}

void HAL_Timer_DelayUs(uint32_t us)
{
    /* 48MHz → her döngü ~3 komut → ~62.5ns */
    /* Yaklaşık: 16 çevrim/µs @ 48MHz */
    volatile uint32_t count = us * 16U;

    while (count--)
        __NOP();
}

uint32_t HAL_Timer_GetTick(void)
{
    return s_u32MsTick;
}

uint32_t HAL_Timer_Elapsed(uint32_t startTick)
{
    return (s_u32MsTick - startTick);
}

uint32_t HAL_Timer_GetSeconds(void)
{
    return s_u32SecTick;
}

void HAL_Timer_ResetSeconds(void)
{
    s_u32SecTick = 0U;
}
