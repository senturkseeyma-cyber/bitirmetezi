/**
 * @file    fake_sensors.c
 * @brief   ORKO Simulasyon - Sahte Sensor Veri Ureteci
 *
 * 20 adimlik 4 fazli senaryo:
 *   Adim  1- 5 : FAZ-1  Normal orman kosullari
 *   Adim  6-10 : FAZ-2  Erken duman belirtisi
 *   Adim 11-15 : FAZ-3  Yangin gelisiyor
 *   Adim 16-20 : FAZ-4  Yangin alarmi (ALARM tetikleniyor, adim 16'dan itibaren)
 *
 * Fire Score Algoritması Doğrulama (adim 16 icin beklenen):
 *   Buffer[11..15]: T={54,60,65,68,71}, ortalama=63.6 C
 *   Buffer[16]    : T=74 C  -> N_Temp = (74-63.6)/(70-63.6) = 1.63 -> 1.0
 *   CO2=5200 ppm  -> N_CO2  = (5200-4330)/(5000-4330) = 1.30 -> 1.0
 *   CO=215 ppm    -> N_CO   = (215-189.8)/(200-189.8) = 2.47 -> 1.0
 *   FireScore = 0.25*1.0 + 0.25*1.0 + 0.25*1.0 + 0.15*1.0 = 0.90 -> ALARM!
 */

#include "fake_sensors.h"

/* GPS: Ankara orman bolgesi (simule edilmis) */
#define SIM_LAT    39.9254f
#define SIM_LON    32.8597f
#define SIM_ALT   850.0f

/* ----------------------------------------------------------------
 * Senaryo tablosu: {sicaklik, co2, co, basinc, nem, faz_adi}
 * ---------------------------------------------------------------- */
typedef struct {
    float       temperature_C;
    float       co2_ppm;
    float       co_ppm;
    float       pressure_Pa;
    float       humidity_pct;
    const char *phase_name;
} ScenarioRow_t;

static const ScenarioRow_t g_scenario[FAKE_SCENARIO_STEPS] = {
    /* ---- FAZ 1: Normal orman (Adim 1-5) ---- */
    { 22.0f,   412.0f,   5.0f,  101300.0f, 64.0f, "FAZ-1: NORMAL  Orman Kosullari" },
    { 22.5f,   418.0f,   5.5f,  101200.0f, 63.5f, "FAZ-1: NORMAL  Orman Kosullari" },
    { 23.0f,   425.0f,   6.0f,  101100.0f, 63.0f, "FAZ-1: NORMAL  Orman Kosullari" },
    { 23.5f,   430.0f,   6.5f,  101000.0f, 62.5f, "FAZ-1: NORMAL  Orman Kosullari" },
    { 24.0f,   435.0f,   7.0f,  100900.0f, 62.0f, "FAZ-1: NORMAL  Orman Kosullari" },
    /* ---- FAZ 2: Erken duman (Adim 6-10) ---- */
    { 28.0f,   700.0f,  20.0f,  100500.0f, 56.0f, "FAZ-2: ERKEN DUMAN  Belirtisi"  },
    { 33.0f,  1100.0f,  38.0f,  100000.0f, 50.0f, "FAZ-2: ERKEN DUMAN  Belirtisi"  },
    { 38.0f,  1600.0f,  60.0f,   99500.0f, 44.0f, "FAZ-2: ERKEN DUMAN  Belirtisi"  },
    { 43.0f,  2200.0f,  85.0f,   99000.0f, 37.0f, "FAZ-2: ERKEN DUMAN  Belirtisi"  },
    { 48.0f,  2900.0f, 115.0f,   98300.0f, 30.0f, "FAZ-2: ERKEN DUMAN  Belirtisi"  },
    /* ---- FAZ 3: Yangin gelisiyor (Adim 11-15) ---- */
    { 54.0f,  3600.0f, 140.0f,   97500.0f, 24.0f, "FAZ-3: YANGIN GELISIYOR !"      },
    { 60.0f,  4000.0f, 160.0f,   96800.0f, 20.0f, "FAZ-3: YANGIN GELISIYOR !"      },
    { 65.0f,  4400.0f, 180.0f,   96000.0f, 17.0f, "FAZ-3: YANGIN GELISIYOR !"      },
    { 68.0f,  4700.0f, 195.0f,   95400.0f, 14.0f, "FAZ-3: YANGIN GELISIYOR !"      },
    { 71.0f,  4950.0f, 199.0f,   95100.0f, 12.0f, "FAZ-3: YANGIN GELISIYOR !"      },
    /* ---- FAZ 4: Yangin alarmi (Adim 16-20) ---- */
    { 74.0f,  5200.0f, 215.0f,   94800.0f, 10.0f, "FAZ-4: *** YANGIN ALARMI ***"   },
    { 77.0f,  5600.0f, 240.0f,   94200.0f,  8.0f, "FAZ-4: *** YANGIN ALARMI ***"   },
    { 80.0f,  6100.0f, 265.0f,   93600.0f,  6.0f, "FAZ-4: *** YANGIN ALARMI ***"   },
    { 83.0f,  6600.0f, 290.0f,   93000.0f,  5.0f, "FAZ-4: *** YANGIN ALARMI ***"   },
    { 86.0f,  7100.0f, 320.0f,   92500.0f,  3.0f, "FAZ-4: *** YANGIN ALARMI ***"   },
};

/* ================================================================ */

void FakeSensors_GetData(uint8_t step, FakeSensorData_t *pData)
{
    const ScenarioRow_t *row;

    if (step >= FAKE_SCENARIO_STEPS)
        step = FAKE_SCENARIO_STEPS - 1U;

    row = &g_scenario[step];

    pData->temperature_C = row->temperature_C;
    pData->co2_ppm       = row->co2_ppm;
    pData->co_ppm        = row->co_ppm;
    pData->pressure_Pa   = row->pressure_Pa;
    pData->humidity_pct  = row->humidity_pct;
    pData->latitude      = SIM_LAT;
    pData->longitude     = SIM_LON;
    pData->altitude_m    = SIM_ALT;
    pData->gps_valid     = 1U;
    pData->phase_name    = row->phase_name;
}
