# ================================================================
# CMake Toolchain Dosyasi
# arm-none-eabi-gcc (ARM GNU Toolchain 15.2)
# ================================================================

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)

set(TOOLCHAIN_PREFIX "arm-none-eabi-")
set(TOOLCHAIN_DIR "C:/arm-gnu-toolchain-15.2.rel1-mingw-w64-i686-arm-none-eabi/bin")

find_program(CMAKE_C_COMPILER   ${TOOLCHAIN_PREFIX}gcc   HINTS ${TOOLCHAIN_DIR} REQUIRED)
find_program(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++   HINTS ${TOOLCHAIN_DIR} REQUIRED)
find_program(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}gcc   HINTS ${TOOLCHAIN_DIR} REQUIRED)
find_program(CMAKE_OBJCOPY      ${TOOLCHAIN_PREFIX}objcopy HINTS ${TOOLCHAIN_DIR})
find_program(CMAKE_SIZE         ${TOOLCHAIN_PREFIX}size    HINTS ${TOOLCHAIN_DIR})

# Hedef sistem dosyalarini aramaya calisma (cross-compile icin gerekli)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Baglayici test programini calistirma (cross-compile icin)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
