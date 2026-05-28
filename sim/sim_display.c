/**
 * @file    sim_display.c
 * @brief   ORKO Simulasyon - Konsol Cikti Modulu
 *
 * Her adimda sensor degerleri, normalize faktorler ve
 * Fire Score sonucu tabloya yazdirilir.
 */

#include "sim_display.h"
#include "orko_config.h"
#include <stdio.h>

/* ----------------------------------------------------------------
 * Cizgi sabitleri (ASCII — her terminalde calisir)
 * ---------------------------------------------------------------- */
#define DLINE "============================================================"
#define SLINE "------------------------------------------------------------"

/* ================================================================
 * YARDIMCI: Float'i %d.%Nd olarak yazdir (printf %f yerine)
 * Cortex-M0 soft-float printf cok yavas — integer printf kullan.
 * ================================================================ */
static void pf1(float v)   /* 1 ondalik: 43.5 */
{
    int x = (int)(v * 10.0f + 0.5f);
    printf("%d.%d", x / 10, x % 10);
}
static void pf3(float v)   /* 3 ondalik: 0.212 */
{
    int x = (int)(v * 1000.0f + 0.5f);
    printf("%d.%03d", x / 1000, x % 1000);
}
static void pf2(float v)   /* 2 ondalik: 0.60 */
{
    int x = (int)(v * 100.0f + 0.5f);
    printf("%d.%02d", x / 100, x % 100);
}
static int pf0(float v)    /* 0 ondalik: integer olarak don */
{
    return (int)(v + 0.5f);
}
static void pf4(float v)   /* 4 ondalik: 39.9254 */
{
    int w = (int)v;
    int f = (int)((v - (float)w) * 10000.0f + 0.5f);
    printf("%d.%04d", w, f);
}

/* ================================================================
 * YARDIMCI: Durum cubugu (0.0-1.0 araligini 20 karaktere esler)
 * ================================================================ */
static void print_bar(float val)
{
    int  filled = (int)(val * 20.0f);
    int  i;

    printf("[");
    for (i = 0; i < 20; i++)
        printf("%c", (i < filled) ? '#' : '-');
    printf("]");
}

/* ================================================================
 * ANA FONKSIYON
 * ================================================================ */
void SimDisplay_PrintStep(uint8_t                   step_num,
                          uint8_t                   total,
                          const FakeSensorData_t   *pData,
                          const FireScore_Result_t *pResult,
                          int8_t                    calc_ok)
{
    printf("\n%s\n", DLINE);
    printf("  ORKO Slave #1  |  Adim: %2d / %d\n", step_num, total);
    printf("  Senaryo : %s\n", pData->phase_name);
    printf("%s\n", SLINE);

    /* ---- Sensor okuma tablosu ---- */
    printf("  Sicaklik : ");      pf1(pData->temperature_C);
    printf(" C      |  CO2     : %7d ppm\n", pf0(pData->co2_ppm));
    printf("  Nem      : ");      pf1(pData->humidity_pct);
    printf(" %%      |  CO      : ");  pf1(pData->co_ppm);
    printf(" ppm\n");
    printf("  Basinc   : %6d Pa     |  GPS     : ", pf0(pData->pressure_Pa));
    pf4(pData->latitude);  printf("N  "); pf4(pData->longitude); printf("E  %dm\n", (int)pData->altitude_m);

    /* ---- Kritik esik gostergesi ---- */
    printf("  Kritik?  : Sicak=%-5s CO2=%-5s CO=%-5s\n",
           (pData->temperature_C >= TEMP_CRITICAL_C)  ? "EVET" : "Hayir",
           (pData->co2_ppm       >= CO2_CRITICAL_PPM) ? "EVET" : "Hayir",
           (pData->co_ppm        >= CO_CRITICAL_PPM)  ? "EVET" : "Hayir");

    printf("%s\n", SLINE);

    if (calc_ok == 0)
    {
        /* ---- Normalize degerler ve cubuklar ---- */
        printf("  Normalize Faktorler  (0.0 = Normal, 1.0 = Kritik):\n");
        printf("    N_Sicak : "); pf3(pResult->n_temp);     printf("  "); print_bar(pResult->n_temp);     printf("\n");
        printf("    N_CO2   : "); pf3(pResult->n_co2);      printf("  "); print_bar(pResult->n_co2);      printf("\n");
        printf("    N_CO    : "); pf3(pResult->n_co);       printf("  "); print_bar(pResult->n_co);       printf("\n");
        printf("    N_Bsnc  : "); pf3(pResult->n_pressure); printf("  "); print_bar(pResult->n_pressure); printf("\n");
        printf("    N_Nem   : "); pf3(pResult->n_humidity); printf("  "); print_bar(pResult->n_humidity); printf("\n");

        printf("%s\n", SLINE);

        /* ---- Fire Score ve karar ---- */
        printf("  FIRE SCORE : "); pf3(pResult->fire_score); printf("  "); print_bar(pResult->fire_score);
        printf("  (Esik: "); pf2(FIRE_SCORE_THRESHOLD); printf(")\n");

        if (pResult->alarm)
        {
            printf("\n");
            printf("  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            printf("  !!!            YANGIN ALARMI TETIKLENDI               !!!\n");
            printf("  !!!   LoRa ALARM paketi Master'a GONDERILIYOR (sim)   !!!\n");
            printf("  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            printf("  Paket : PKT_TYPE_ALARM | Slave=0x0001 -> Master=0x0000\n");
        }
        else
        {
            if (pResult->fire_score > 0.50f)
                printf("  Durum  : [!!] YUKSEK RISK - Esige Yakin! Takip Ediliyor.\n");
            else if (pResult->fire_score > 0.25f)
                printf("  Durum  : [!]  Artan Risk Tespit Edildi. Izleniyor.\n");
            else
                printf("  Durum  : [OK] Normal - Alarm Yok.\n");
        }
    }
    else
    {
        printf("  Tampon: %d/5 ornek - Fire Score hesaplamak icin ornek bekleniyor.\n",
               step_num);
    }

    printf("%s\n", DLINE);
}
