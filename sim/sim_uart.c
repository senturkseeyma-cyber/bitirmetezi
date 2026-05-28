/**
 * @file    sim_uart.c
 * @brief   FVP MPS2 Cortex-M0 - printf UART0 yonlendirmesi
 *
 * FVP_MPS2_Cortex-M0 UART0 adresi: 0x40004000
 * -C fvp_mps2.UART0.out_file=- parametresiyle FVP stdout'a yonlendirilir.
 *
 * Bu dosya: printf -> _write -> UART0 DR zincirini saglar.
 * Semihosting yerine UART kullanildiginda daha guvenilir.
 */

#include <stdint.h>
#include <sys/stat.h>

/* MPS2 UART0 base address (FVP ve CMSDK) */
#define UART0_BASE      (0x40004000UL)
#define UART0_DATA      (*(volatile uint32_t*)(UART0_BASE + 0x00)) /* Data register */
#define UART0_STATE     (*(volatile uint32_t*)(UART0_BASE + 0x04)) /* State: bit0=TX full */
#define UART0_CTRL      (*(volatile uint32_t*)(UART0_BASE + 0x08)) /* Control */
#define UART0_BAUDDIV   (*(volatile uint32_t*)(UART0_BASE + 0x10)) /* Baud divisor */

/* UART baslat: BAUDDIV=1 → maksimum hiz (FVP simule clock'ta 115200 cok yavas) */
void FVP_UART0_Init(void)
{
    UART0_BAUDDIV = 1U;             /* Maksimum hiz — FVP simulasyonunda hizlandirir */
    UART0_CTRL    = 0x01U;          /* TX enable */
}

/* Tek karakter gonder — BAUDDIV=1 ile maksimum hiz, ama TX dolu kontrolu ile guvenli */
static void uart_putchar(char c)
{
    while (UART0_STATE & 0x01U)     /* TX full ise bekle */
        ;
    UART0_DATA = (uint32_t)(uint8_t)c;
}

/* ----------------------------------------------------------------
 * Newlib _write: printf buraya gelir
 * ---------------------------------------------------------------- */
int _write(int fd, char *buf, int len)
{
    int i;
    (void)fd;
    for (i = 0; i < len; i++)
    {
        if (buf[i] == '\n')
            uart_putchar('\r');      /* CRLF donusumu */
        uart_putchar(buf[i]);
    }
    return len;
}

/* Newlib stubs - link hatasi onleme */
int _close(int fd)          { (void)fd; return -1; }
int _fstat(int fd, struct stat *st) { (void)fd; st->st_mode = S_IFCHR; return 0; }
int _isatty(int fd)         { (void)fd; return 1; }
int _lseek(int fd, int off, int whence) { (void)fd; (void)off; (void)whence; return 0; }
int _read(int fd, char *buf, int len) { (void)fd; (void)buf; (void)len; return 0; }
