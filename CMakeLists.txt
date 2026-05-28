cmake_minimum_required(VERSION 3.20)

# ================================================================
# ORKO - Slave Node CMake Build Sistemi
# Hedef: ARM Cortex-M0 (Nuvoton M031FB0AE)
# Toolchain: arm-none-eabi-gcc
# Simulasyon: ARM FVP_MPS2_Cortex-M0 (donanim gerektirmez)
# ================================================================

project(ORKO_Slave C ASM)

# ---- Toolchain kontrolu ----
if(NOT CMAKE_C_COMPILER MATCHES "arm-none-eabi")
    message(FATAL_ERROR "Bu proje arm-none-eabi-gcc gerektirir. "
        "cmake -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake .. ile calistirin.")
endif()

# ================================================================
# DERLEME BAYRAKLARI (Cortex-M0 = ARMv6-M, thumb, no FPU)
# ================================================================
set(CPU_FLAGS "-mcpu=cortex-m0 -mthumb -mfloat-abi=soft")

set(COMMON_FLAGS "${CPU_FLAGS} \
    -Wall -Wextra \
    -fdata-sections -ffunction-sections \
    -fno-exceptions -fno-unwind-tables \
    -O0 -g3")

set(CMAKE_C_FLAGS   "${COMMON_FLAGS}" CACHE STRING "" FORCE)
set(CMAKE_ASM_FLAGS "${CPU_FLAGS} -g3" CACHE STRING "" FORCE)

# ================================================================
# KAYNAK DOSYALAR
# ================================================================

# --- Algoritma (derleme bagimli degil, her zaman derlenir) ---
set(ALGO_SOURCES
    algo/fire_score.c
)

# --- Simulasyon kaynaklar (ORKO_SIM modunda kullanilan) ---
set(SIM_SOURCES
    sim/sim_main.c
    sim/fake_sensors.c
    sim/sim_display.c
)

# --- Firmware kaynaklar (gercek kart icin) ---
set(FW_SOURCES
    startup/startup_M031Series.s
    app/main.c
    drivers/hal_i2c.c
    drivers/hal_uart.c
    drivers/hal_adc.c
    drivers/hal_timer.c
    sensors/drv_bmp390.c
    sensors/drv_hdc2022.c
    sensors/drv_scd40.c
    sensors/drv_mq9.c
    sensors/drv_neo7m.c
    lora/lora_e22.c
    power/power_mgmt.c
)

# ================================================================
# INCLUDE YOLLARI (her ikisi icin ortak)
# ================================================================
set(INCLUDE_DIRS
    app
    algo
    drivers
    sensors
    lora
    power
    sim
)

# ================================================================
# HEDEF 1: orko_sim.elf (Fake data, FVP ile calis)
# ================================================================
add_executable(orko_sim.elf
    ${ALGO_SOURCES}
    ${SIM_SOURCES}
    startup/startup_M031Series.s
)

target_include_directories(orko_sim.elf PRIVATE ${INCLUDE_DIRS})

target_compile_definitions(orko_sim.elf PRIVATE
    ORKO_SIM
)

target_compile_options(orko_sim.elf PRIVATE
    -specs=rdimon.specs          # semihosting (printf → FVP konsoluna)
)

target_link_options(orko_sim.elf PRIVATE
    ${CPU_FLAGS}
    -T ${CMAKE_SOURCE_DIR}/linker/M031_sim.ld
    -Wl,--gc-sections
    -specs=rdimon.specs          # semihosting
    -lc -lrdimon                 # semihosting kutuphanesi
    -Wl,-Map=${CMAKE_BINARY_DIR}/orko_sim.map
)

# ================================================================
# HEDEF 2: orko_fw.elf (Gercek kart firmware — BSP gerektirir)
# ================================================================
# NOT: Nuvoton BSP (NuMicro.h, StdDriver) olmadan DERLENMEZ.
# BSP'yi indirip startup/BSP klasorune koyunca aktive edin.
# Simdilik sadece ORKO_SIM hedefi aktif.
# ================================================================
option(BUILD_FIRMWARE "Nuvoton BSP ile gercek firmware derle" OFF)

if(BUILD_FIRMWARE)
    # BSP klasorunu burada belirtin:
    set(BSP_DIR "BSP/M031Series" CACHE PATH "Nuvoton M031 BSP klasoru")

    add_executable(orko_fw.elf
        ${ALGO_SOURCES}
        ${FW_SOURCES}
    )

    target_include_directories(orko_fw.elf PRIVATE
        ${INCLUDE_DIRS}
        ${BSP_DIR}/Library/StdDriver/inc
        ${BSP_DIR}/Library/Device/Nuvoton/M031Series/Include
        ${BSP_DIR}/Library/CMSIS/Include
    )

    target_link_options(orko_fw.elf PRIVATE
        ${CPU_FLAGS}
        -T ${CMAKE_SOURCE_DIR}/linker/M031_fw.ld
        -Wl,--gc-sections
        -specs=nosys.specs
        -lc -lm -lnosys
        -Wl,-Map=${CMAKE_BINARY_DIR}/orko_fw.map
    )
endif()

# ================================================================
# BINARY / HEX URETIMI (flash icin)
# ================================================================
find_program(OBJCOPY arm-none-eabi-objcopy HINTS
    "C:/arm-gnu-toolchain-15.2.rel1-mingw-w64-i686-arm-none-eabi/bin")

if(OBJCOPY AND BUILD_FIRMWARE)
    add_custom_command(TARGET orko_fw.elf POST_BUILD
        COMMAND ${OBJCOPY} -O binary orko_fw.elf orko_fw.bin
        COMMAND ${OBJCOPY} -O ihex   orko_fw.elf orko_fw.hex
        COMMENT "Binary ve HEX dosyalari olusturuluyor..."
    )
endif()

# ================================================================
# BOYUT RAPORU (her build sonrasi)
# ================================================================
find_program(SIZE_TOOL arm-none-eabi-size HINTS
    "C:/arm-gnu-toolchain-15.2.rel1-mingw-w64-i686-arm-none-eabi/bin")

if(SIZE_TOOL)
    add_custom_command(TARGET orko_sim.elf POST_BUILD
        COMMAND ${SIZE_TOOL} orko_sim.elf
        COMMENT "Bellek kullanimi:"
    )
endif()
