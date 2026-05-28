/**
 * @file    sim_uart.c
 * @brief   Semihosting modunda UART stub
 *
 * printf -> rdimon semihosting -> FVP ARM trap -> host terminal
 * UART register kullanimina gerek yok, rdimon her seyi halleder.
 */

#include <stdint.h>

/* Semihosting modunda UART init gerekli degil — stub olarak bırakıldı */
void FVP_UART0_Init(void)
{
    /* rdimon semihosting aktif: printf direkt FVP host terminaline gider */
}
