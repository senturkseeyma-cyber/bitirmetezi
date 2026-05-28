/**
 * @file    sim_uart.c
 * @brief   Direkt ARM semihosting ile printf -> terminal
 *
 * rdimon kullanmadan kendi _write ile semihosting SYS_WRITE (0x05) cagrisi.
 * Cortex-M0 Thumb modunda: r0=islem, r1=params, BKPT 0xAB
 */

#include <stdint.h>
#include <sys/stat.h>

/* ----------------------------------------------------------------
 * Direkt semihosting SYS_WRITE (0x05)
 * args[0]=handle, args[1]=data ptr, args[2]=len
 * ---------------------------------------------------------------- */
static void semi_write(int handle, const char *buf, int len)
{
    volatile uint32_t args[3];
    args[0] = (uint32_t)handle;
    args[1] = (uint32_t)(uintptr_t)buf;
    args[2] = (uint32_t)len;

    register uint32_t r0 __asm__("r0") = 0x05U; /* SYS_WRITE */
    register volatile uint32_t *r1 __asm__("r1") = args;
    __asm__ volatile ("bkpt 0xAB" : "+r"(r0) : "r"(r1) : "memory");
}

/* FVP baslat: bu modda UART register yok, semihosting yeterli */
void FVP_UART0_Init(void)
{
    /* Direkt semihosting: UART register kurulumuna gerek yok */
}

/* ----------------------------------------------------------------
 * Newlib _write: printf -> buraya gelir -> semihosting -> terminal
 * ---------------------------------------------------------------- */
int _write(int fd, char *buf, int len)
{
    (void)fd;
    semi_write(1, buf, len); /* 1 = stdout */
    return len;
}

/* Newlib stubs */
int _close(int fd)          { (void)fd; return -1; }
int _fstat(int fd, struct stat *st) { (void)fd; st->st_mode = S_IFCHR; return 0; }
int _isatty(int fd)         { (void)fd; return 1; }
int _lseek(int fd, int off, int whence) { (void)fd; (void)off; (void)whence; return 0; }
int _read(int fd, char *buf, int len) { (void)fd; (void)buf; (void)len; return 0; }

