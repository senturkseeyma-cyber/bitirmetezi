@echo off
chcp 65001 >nul 2>&1
cd /d "%~dp0.."

echo.
echo  =====================================================
echo   ORKO - PC Simulasyon Build ve Calistirma Scripti
echo  =====================================================
echo.

:: GCC kontrolu
where gcc >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo  [HATA] GCC bulunamadi!
    echo.
    echo  MinGW-w64 indirmek icin:
    echo    https://github.com/niXman/mingw-builds-binaries/releases
    echo    (ornegin: x86_64-13.2.0-release-win32-seh-ucrt-rt_v11-rev0.7z)
    echo.
    echo  Indirdikten sonra "bin" klasorunu Windows PATH'e ekleyin:
    echo    Sistem Ozellikleri -> Ortam Degiskenleri -> Path -> Yeni
    echo.
    pause
    exit /b 1
)

echo  [OK] GCC bulundu:
gcc --version | findstr /i "gcc"
echo.

:: Derleme
echo  Derleniyor...
echo  -------------------------------------------------------

gcc ^
    -DORKO_SIM ^
    -I./app ^
    -I./algo ^
    -I./sim ^
    -Wall ^
    -Wextra ^
    -o ./sim/orko_sim.exe ^
    ./sim/sim_main.c ^
    ./algo/fire_score.c ^
    ./sim/fake_sensors.c ^
    ./sim/sim_display.c ^
    -lm

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo  [HATA] Derleme basarisiz! Yukaridaki hatalari inceleyin.
    pause
    exit /b 1
)

echo.
echo  [OK] Derleme basarili  ->  sim\orko_sim.exe
echo.
echo  =====================================================
echo   Simulasyon baslatiliyor...
echo  =====================================================
echo.

.\sim\orko_sim.exe

echo.
pause
