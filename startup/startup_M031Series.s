/**
 * @file    startup_M031Series.s
 * @brief   Nuvoton M031 Serisi Cortex-M0 Startup Kodu (GNU Assembly)
 *
 * FVP ve gercek kart icin kullanilir.
 * Stack pointer, interrupt vektoru ve Reset_Handler tanimlidir.
 */

    .syntax unified
    .arch armv6-m

/* ================================================================
 * Stack ve Heap boyutlari
 * ================================================================ */
    .set Stack_Size, 0x800      /* 2KB stack */
    .set Heap_Size,  0x200      /* 512B heap */

/* ================================================================
 * Stack ve Heap alanlari
 * ================================================================ */
    .section .stack, "w"
    .align 3
    .space Stack_Size
_estack:

    .section .heap, "w"
    .align 3
_heap_start:
    .space Heap_Size
_heap_end:

/* ================================================================
 * Vektor tablosu
 * ================================================================ */
    .section .isr_vector, "a", %progbits
    .align 2
    .global g_pfnVectors
g_pfnVectors:
    /* Cortex-M0 cekirdek kesmeler */
    .long   _estack                     /* Baslangic stack pointer  */
    .long   Reset_Handler               /* Reset                    */
    .long   NMI_Handler                 /* NMI                      */
    .long   HardFault_Handler           /* HardFault               */
    .long   0                           /* (rezerve)               */
    .long   0
    .long   0
    .long   0
    .long   0
    .long   0
    .long   0
    .long   SVC_Handler                 /* SVCall                   */
    .long   0
    .long   0
    .long   PendSV_Handler              /* PendSV                   */
    .long   SysTick_Handler             /* SysTick                  */
    /* M031 cevresel kesmeler (32 adet) */
    .long   BOD_IRQHandler
    .long   WDT_IRQHandler
    .long   EINT024_IRQHandler
    .long   EINT135_IRQHandler
    .long   GPABCD_IRQHandler
    .long   GPEF_IRQHandler
    .long   BPWM0_IRQHandler
    .long   0
    .long   TMR0_IRQHandler
    .long   TMR1_IRQHandler
    .long   TMR2_IRQHandler
    .long   TMR3_IRQHandler
    .long   UART0_IRQHandler
    .long   UART1_IRQHandler
    .long   SPI0_IRQHandler
    .long   0
    .long   I2C0_IRQHandler
    .long   I2C1_IRQHandler
    .long   0
    .long   0
    .long   ADC_IRQHandler
    .long   0
    .long   ACMP01_IRQHandler
    .long   0
    .long   0
    .long   0
    .long   PDMA_IRQHandler
    .long   0
    .long   PWRWU_IRQHandler
    .long   0
    .long   CLKFAIL_IRQHandler
    .long   0

/* ================================================================
 * Reset Handler
 * ================================================================ */
    .section .text.Reset_Handler
    .weak   Reset_Handler
    .type   Reset_Handler, %function
Reset_Handler:
    /* .data bolumunu Flash'tan RAM'a kopyala */
    ldr     r0, =_sdata
    ldr     r1, =_edata
    ldr     r2, =_sidata
    movs    r3, #0
    b       loop_copy_data_init

copy_data_init:
    ldr     r4, [r2, r3]
    str     r4, [r0, r3]
    adds    r3, r3, #4

loop_copy_data_init:
    adds    r4, r0, r3
    cmp     r4, r1
    bcc     copy_data_init

    /* .bss bolumunu sifirla */
    ldr     r2, =_sbss
    ldr     r4, =_ebss
    movs    r3, #0
    b       loop_fill_bss

fill_bss:
    str     r3, [r2]
    adds    r2, r2, #4

loop_fill_bss:
    cmp     r2, r4
    bcc     fill_bss

    /* Semihosting baslatma (ORKO_SIM modunda printf icin) */
    .weak   initialise_monitor_handles
    ldr     r0, =initialise_monitor_handles
    cmp     r0, #0
    beq     skip_semihosting
    blx     r0
skip_semihosting:

    /* main() cagir */
    bl      main

    /* main() donerse sonsuz dongu */
infinite_loop:
    b       infinite_loop

    .size Reset_Handler, . - Reset_Handler

/* ================================================================
 * Varsayilan / zayif ISR tanimlamalari
 * ================================================================ */
    .macro DEFINE_DEFAULT_HANDLER name
    .section .text.\name
    .weak   \name
    .type   \name, %function
\name:
    b       .
    .size   \name, . - \name
    .endm

    DEFINE_DEFAULT_HANDLER NMI_Handler
    DEFINE_DEFAULT_HANDLER HardFault_Handler
    DEFINE_DEFAULT_HANDLER SVC_Handler
    DEFINE_DEFAULT_HANDLER PendSV_Handler
    DEFINE_DEFAULT_HANDLER SysTick_Handler
    DEFINE_DEFAULT_HANDLER BOD_IRQHandler
    DEFINE_DEFAULT_HANDLER WDT_IRQHandler
    DEFINE_DEFAULT_HANDLER EINT024_IRQHandler
    DEFINE_DEFAULT_HANDLER EINT135_IRQHandler
    DEFINE_DEFAULT_HANDLER GPABCD_IRQHandler
    DEFINE_DEFAULT_HANDLER GPEF_IRQHandler
    DEFINE_DEFAULT_HANDLER BPWM0_IRQHandler
    DEFINE_DEFAULT_HANDLER TMR0_IRQHandler
    DEFINE_DEFAULT_HANDLER TMR1_IRQHandler
    DEFINE_DEFAULT_HANDLER TMR2_IRQHandler
    DEFINE_DEFAULT_HANDLER TMR3_IRQHandler
    DEFINE_DEFAULT_HANDLER UART0_IRQHandler
    DEFINE_DEFAULT_HANDLER UART1_IRQHandler
    DEFINE_DEFAULT_HANDLER SPI0_IRQHandler
    DEFINE_DEFAULT_HANDLER I2C0_IRQHandler
    DEFINE_DEFAULT_HANDLER I2C1_IRQHandler
    DEFINE_DEFAULT_HANDLER ADC_IRQHandler
    DEFINE_DEFAULT_HANDLER ACMP01_IRQHandler
    DEFINE_DEFAULT_HANDLER PDMA_IRQHandler
    DEFINE_DEFAULT_HANDLER PWRWU_IRQHandler
    DEFINE_DEFAULT_HANDLER CLKFAIL_IRQHandler
