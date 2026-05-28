/**
 * @file    sim_uart.c
 * @brief   FVP MPS2 UART0 register retargeting -> telnet terminal
 *
 * FVP_MPS2 UART0 adresi: 0x40004000
 * BAUDDIV=16 ile hizli simülasyon, CRLF donusumu yapilir.
 */

#include <stdint.h>
#include <sys/stat.h>

/* FVP MPS2 UART0 register adresleri */
#define UART0_BASE    0x40004000U
#define UART0_DATA    (*(volatile uint32_t *)(UART0_BASE + 0x00U))
#define UART0_STATE   (*(volatile uint32_t *)(UART0_BASE + 0x04U))
#define UART0_CTRL    (*(volatile uint32_t *)(UART0_BASE + 0x08U))
#define UART0_BAUDDIV (*(volatile uint32_t *)(UART0_BASE + 0x10U))

/* TX tampon dolu biti: bit0 */
#define UART_TX_FULL  0x01U

/* ----------------------------------------------------------------
 * UART0 baslat: baud=16 (simülasyonda fark etmez), TX etkin
 * ---------------------------------------------------------------- */
void FVP_UART0_Init(void)
{
    UART0_BAUDDIV = 16U;
    UART0_CTRL    = 0x01U; /* TX enable */
}

static void uart_putchar(char c)
{
    while (UART0_STATE & UART_TX_FULL) {} /* TX tampon bos olana dek bekle */
    UART0_DATA = (uint32_t)(uint8_t)c;
}

/* ----------------------------------------------------------------
 * Newlib _write: printf -> buraya gelir -> UART0 -> FVP telnet
 * LF -> CRLF donusumu yapilir
 * ---------------------------------------------------------------- */
int _write(int fd, char *buf, int len)
{
    (void)fd;
    for (int i = 0; i < len; i++) {
        if (buf[i] == '\n') {
            uart_putchar('\r');
        }
        uart_putchar(buf[i]);
    }
    return len;
}

/* Newlib stubs */
int _close(int fd)          { (void)fd; return -1; }
int _fstat(int fd, struct stat *st) { (void)fd; st->st_mode = S_IFCHR; return 0; }
int _isatty(int fd)         { (void)fd; return 1; }
int _lseek(int fd, int off, int whence) { (void)fd; (void)off; (void)whence; return 0; }
int _read(int fd, char *buf, int len) { (void)fd; (void)buf; (void)len; return 0; }

