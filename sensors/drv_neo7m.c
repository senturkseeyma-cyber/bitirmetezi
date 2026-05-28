/**
 * @file    drv_neo7m.c
 * @brief   NEO-7M GPS Modülü Sürücüsü - Kaynak
 *
 * NMEA $GPGGA cümle formatı:
 * $GPGGA,hhmmss.ss,Lat,N/S,Lon,E/W,FS,NoSV,HDOP,msl,M,Altref,M,DiffAge,DiffStation*cs<CR><LF>
 * Alan 1: Saat  Alan 2: Enlem  Alan 4: Boylam  Alan 9: Yükseklik  Alan 7: Uydu sayısı
 */

#include "drv_neo7m.h"
#include "hal_timer.h"
#include <string.h>
#include <stdlib.h>

/* ================================================================
 * NMEA PARSE YARDIMCILARI
 * ================================================================ */

/**
 * @brief Virgülle ayrılmış NMEA alanını kopyalar
 * @param src    : NMEA cümlesi
 * @param field  : Alan numarası (0-tabanlı)
 * @param dst    : Hedef tampon
 * @param maxLen : Maksimum kopyalanacak karakter
 */
static void NMEA_GetField(const char *src, uint8_t field,
                          char *dst, uint8_t maxLen)
{
    uint8_t  currField = 0U;
    uint8_t  i         = 0U;
    uint8_t  j         = 0U;

    while (src[i] != '\0' && src[i] != '*')
    {
        if (src[i] == ',')
        {
            currField++;
            i++;
            continue;
        }

        if (currField == field)
        {
            if (j < (maxLen - 1U))
                dst[j++] = src[i];
        }
        else if (currField > field)
        {
            break;
        }

        i++;
    }

    dst[j] = '\0';
}

/**
 * @brief NMEA enlem/boylam formatını decimal degrees'e çevirir
 *        Format: DDDMM.MMMM → DDD + MM.MMMM/60
 */
static float NMEA_ToDegrees(const char *str)
{
    float  val     = 0.0f;
    float  deg     = 0.0f;
    float  minutes = 0.0f;
    char  *ptr     = NULL;

    if (str[0] == '\0')
        return 0.0f;

    val = strtof(str, &ptr);

    /* Enlem: DDMM.MMMM → DD + MM/60 */
    deg     = (float)(int)(val / 100.0f);
    minutes = val - (deg * 100.0f);

    return deg + (minutes / 60.0f);
}

/* ================================================================
 * GENEL FONKSİYONLAR
 * ================================================================ */

void GPS_Init(void)
{
    HAL_UART_GPS_Init();
}

int8_t GPS_Read(GPS_Data_t *pData, uint8_t attempts)
{
    char    lineBuf[GPS_NMEA_MAX_LEN + 1U];
    char    field[16];
    uint8_t i;

    pData->fix_valid = 0U;

    for (i = 0U; i < attempts; i++)
    {
        uint16_t len = HAL_UART_GPS_ReadLine(lineBuf, (uint16_t)sizeof(lineBuf));

        if (len < 6U)
            continue;

        /* Sadece $GPGGA veya $GNGGA cümlelerini işle */
        if ((strncmp(lineBuf, "$GPGGA", 6) != 0) &&
            (strncmp(lineBuf, "$GNGGA", 6) != 0))
            continue;

        /* Alan 6: Fix Status (0=geçersiz, 1=GPS, 2=DGPS) */
        NMEA_GetField(lineBuf, 6U, field, sizeof(field));
        if (field[0] == '0' || field[0] == '\0')
            continue;

        /* Enlem (alan 2) ve yönü (alan 3) */
        NMEA_GetField(lineBuf, 2U, field, sizeof(field));
        pData->latitude = NMEA_ToDegrees(field);
        NMEA_GetField(lineBuf, 3U, field, sizeof(field));
        if (field[0] == 'S')
            pData->latitude = -pData->latitude;

        /* Boylam (alan 4) ve yönü (alan 5) */
        NMEA_GetField(lineBuf, 4U, field, sizeof(field));
        pData->longitude = NMEA_ToDegrees(field);
        NMEA_GetField(lineBuf, 5U, field, sizeof(field));
        if (field[0] == 'W')
            pData->longitude = -pData->longitude;

        /* Uydu sayısı (alan 7) */
        NMEA_GetField(lineBuf, 7U, field, sizeof(field));
        pData->satellites = (uint8_t)atoi(field);

        /* Yükseklik (alan 9) */
        NMEA_GetField(lineBuf, 9U, field, sizeof(field));
        pData->altitude_m = strtof(field, NULL);

        pData->fix_valid = 1U;
        return ORKO_OK;
    }

    return ORKO_ERROR;
}

void GPS_Sleep(void)
{
    /* UBX-CFG-RXM Power Save Mode komutu */
    const uint8_t ubxPsm[] = {
        0xB5, 0x62, 0x06, 0x11, 0x02, 0x00, 0x08, 0x01, 0x22, 0x92
    };

    HAL_UART_LoRa_Write(ubxPsm, sizeof(ubxPsm)); /* GPS TX pinine yaz */
    /* NOT: GPS_TX pininden yayınlamak için UART0 TX kullanılır */
}
