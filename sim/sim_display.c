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
    printf("  Sicaklik : %6.1f C      |  CO2     : %7.0f ppm\n",
           pData->temperature_C, pData->co2_ppm);
    printf("  Nem      : %6.1f %%      |  CO      : %7.1f ppm\n",
           pData->humidity_pct,  pData->co_ppm);
    printf("  Basinc   : %6.0f Pa     |  GPS     : %.4fN  %.4fE  %dm\n",
           pData->pressure_Pa,
           pData->latitude, pData->longitude, (int)pData->altitude_m);

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
        printf("    N_Sicak : %.3f  ", pResult->n_temp);
        print_bar(pResult->n_temp);
        printf("\n");

        printf("    N_CO2   : %.3f  ", pResult->n_co2);
        print_bar(pResult->n_co2);
        printf("\n");

        printf("    N_CO    : %.3f  ", pResult->n_co);
        print_bar(pResult->n_co);
        printf("\n");

        printf("    N_Bsnc  : %.3f  ", pResult->n_pressure);
        print_bar(pResult->n_pressure);
        printf("\n");

        printf("    N_Nem   : %.3f  ", pResult->n_humidity);
        print_bar(pResult->n_humidity);
        printf("\n");

        printf("%s\n", SLINE);

        /* ---- Fire Score ve karar ---- */
        printf("  FIRE SCORE : %.3f  ", pResult->fire_score);
        print_bar(pResult->fire_score);
        printf("  (Esik: %.2f)\n", FIRE_SCORE_THRESHOLD);

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
