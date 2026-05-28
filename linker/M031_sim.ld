/* ================================================================
 * Linker Script: orko_sim (FVP MPS2 Cortex-M0 Simulasyon)
 *
 * FVP_MPS2_Cortex-M0 bellek haritasi:
 *   Flash (ITCM) : 0x00000000 - 0x003FFFFF (4MB)
 *   RAM  (DTCM)  : 0x20000000 - 0x203FFFFF (4MB)
 *
 * Gercek M031FB0AE:
 *   Flash : 64KB, RAM: 8KB
 *   (Gercek kart icin M031_fw.ld kullanin)
 * ================================================================ */

ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 4M
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 4M
}

SECTIONS
{
    /* ---- .text: Kod ve salt-okunur veri ---- */
    .text :
    {
        KEEP(*(.isr_vector))    /* Vektor tablosu — en basta olmali */
        *(.text*)
        *(.rodata*)
        *(.glue_7)
        *(.glue_7t)
        KEEP(*(.init))
        KEEP(*(.fini))
        . = ALIGN(4);
    } > FLASH

    /* ARM semihosting / unwind tablolari */
    .ARM.extab : { *(.ARM.extab* .gnu.linkonce.armextab.*) } > FLASH
    .ARM :
    {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } > FLASH

    _sidata = LOADADDR(.data);

    /* ---- .data: Baslatilmis global degiskenler (Flash → RAM kopyalanir) ---- */
    .data :
    {
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT > FLASH

    /* ---- .bss: Baslatilmamis degiskenler (sifirlanir) ---- */
    .bss :
    {
        _sbss = .;
        __bss_start__ = _sbss;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
        __bss_end__ = _ebss;
    } > RAM

    /* ---- Heap: semihosting _sbrk icin 'end' sembolu gerekli ---- */
    ._heap :
    {
        . = ALIGN(4);
        PROVIDE ( end = . );    /* _sbrk heap baslangici */
        PROVIDE ( _end = . );
        . = . + 0x800;          /* 2KB heap */
        . = ALIGN(4);
    } > RAM

    /* ---- Stack (linker scriptte isaretci, gercek stack startup'ta) ---- */
    ._stack :
    {
        . = ALIGN(4);
        . = . + 0x800;  /* 2KB */
        . = ALIGN(4);
    } > RAM

    /DISCARD/ :
    {
        libc.a (*)
        libm.a (*)
        libgcc.a (*)
    }
}
