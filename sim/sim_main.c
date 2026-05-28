/**
 * @file    sim_main.c
 * @brief   ORKO Slave Node - ARM FVP Simulasyon Ana Programi
 *
 * Derleme: arm-none-eabi-gcc + CMake (build_sim.bat veya VS Code task)
 * Calistirma: FVP_MPS2_Cortex-M0 (donanim gerektirmez)
 * Cikti: FVP "terminal_0" penceresinde veya VS Code Debug Console
 *
 * =============================================================
 * Simulasyon akisi:
 *   1. Fire Score tamponu sifirlanir
 *   2. 20 adim boyunca dongu calisir:
 *      a. Sahte sensor verisi alinir (fake_sensors.c)
 *      b. FireScore_AddSample() ile tampona eklenir
 *      c. FireScore_Calculate() ile score hesaplanir
 *      d. Semihosting printf ile FVP konsoluna yazilir
 *   3. Adim 16'da ALARM tetiklenmesi beklenir
 * =============================================================
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Proje baslik dosyalari */
#include "orko_config.h"    /* Esik degerleri ve agirliklar */
#include "fire_score.h"     /* Gercek Fire Score algoritmasi */
#include "fake_sensors.h"   /* Sahte sensor verileri */
#include "sim_display.h"    /* Konsol cikti modulu */

/* ================================================================
 * SEMIHOSTING: rdimon kutuphanesi bu fonksiyonu saglar.
 * startup kodu cagiriyor; burada bos tanim cakismaya engel.
 * ================================================================ */
#ifndef __SEMIHOSTING_INIT_DONE
extern void initialise_monitor_handles(void) __attribute__((weak));
#endif

/* UART0 baslat (sim_uart.c) */
extern void FVP_UART0_Init(void);

/* ================================================================
 * FVP icin bekleme: FVP simlimit simule saatinde sayar.
 * NOP dongusu simule zamanini tuketir — hic bekleme yapma.
 * ================================================================ */
static void platform_sleep_ms(uint32_t ms)
{
    /* FVP simulasyon modunda gecikme anlamsiz, sadece derleyici uyarisi onle */
    (void)ms;
}

/* ================================================================
 * ANA FONKSIYON
 * ================================================================ */
int main(void)
{
    SensorBuffer_t     buf;
    FireScore_Result_t result;
    FakeSensorData_t   data;
    uint8_t            step;

    /* FVP UART0 baslatma: printf buradan -C UART0.out_file=- ile stdout'a */
    FVP_UART0_Init();

    /* Tampon yok: her printf aninda _write -> semihosting -> terminal */
    setvbuf(stdout, NULL, _IONBF, 0);
    int8_t             calc_ret;
    uint8_t            alarm_count = 0U;

    /* Fire Score tamponunu sifirla */
    FireScore_BufferInit(&buf);

    /* ---- Baslik ---- */
    printf("\n");
    printf("%s\n", "============================================================");
    printf("  ORKO - Orman Yangini Erken Tespit ve Koruma Sistemi\n");
    printf("  PC SIMULASYON MODU  |  Slave Node #1  |  %d Adim\n",
           FAKE_SCENARIO_STEPS);
    printf("%s\n", "============================================================");
    printf("\n");
    printf("  Algoritma: FireScore = 0.25*NT + 0.25*NCO2 +\n");
    printf("             0.25*NCO  + 0.15*NP  + 0.10*NH\n");
    printf("\n");
    printf("  Alarm Esigi  : FireScore > %d.%02d\n",
           (int)(FIRE_SCORE_THRESHOLD * 100) / 100,
           (int)(FIRE_SCORE_THRESHOLD * 100) % 100);
    printf("  Krit.Sicaklik: >= %d C\n",   (int)TEMP_CRITICAL_C);
    printf("  Krit.CO2     : >= %d ppm\n", (int)CO2_CRITICAL_PPM);
    printf("  Krit.CO      : >= %d ppm\n", (int)CO_CRITICAL_PPM);
    printf("\n");
    printf("  GPS Koordinat: 39.9254N  32.8597E  850m (Ankara/Orman)\n");
    printf("\n");
    printf("  Simulasyon basliyor...\n");

    /* ================================================================
     * ANA SIMULASYON DONGUSU
     * ================================================================ */
    for (step = 0U; step < FAKE_SCENARIO_STEPS; step++)
    {
        /* 1. Sahte sensor verisini al */
        FakeSensors_GetData(step, &data);

        /* 2. Fire Score tamponuna ekle */
        FireScore_AddSample(&buf,
                            data.temperature_C,
                            data.co2_ppm,
                            data.co_ppm,
                            data.pressure_Pa,
                            data.humidity_pct);

        /* 3. Fire Score hesapla */
        memset(&result, 0, sizeof(FireScore_Result_t));
        calc_ret = FireScore_Calculate(&buf, &result);

        /* 4. Sonucu ekrana yazdir */
        SimDisplay_PrintStep((uint8_t)(step + 1U),
                             FAKE_SCENARIO_STEPS,
                             &data,
                             &result,
                             calc_ret);

        /* 5. Alarm sayaci */
        if ((calc_ret == 0) && (result.alarm == 1U))
            alarm_count++;

        /* 6. Adimlar arasi bekleme */
        platform_sleep_ms(800U);
    }

    /* ================================================================
     * OZET
     * ================================================================ */
    printf("\n");
    printf("%s\n", "============================================================");
    printf("  SIMULASYON OZETI\n");
    printf("%s\n", "------------------------------------------------------------");
    printf("  Toplam adim    : %d\n", FAKE_SCENARIO_STEPS);
    printf("  Alarm adimi    : %d (Adim 16'dan itibaren beklenir)\n", alarm_count);
    printf("  Son Fire Score : %.3f\n", result.fire_score);
    printf("\n");
    printf("  Buffer Tamponu : %d/5 ornek (circular FIFO)\n", buf.count);
    printf("\n");
    if (alarm_count > 0)
        printf("  [!!] %d adimda YANGIN ALARMI tespit edildi.\n", alarm_count);
    else
        printf("  [OK] Hic alarm tetiklenmedi.\n");
    printf("%s\n", "============================================================");
    printf("\n  [SIM TAMAMLANDI]\n");

    return 0;
}
