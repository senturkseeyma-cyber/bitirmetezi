# ================================================================
# build_sim.ps1  -  ORKO Slave Node - ARM FVP Simulasyon Build
# ================================================================
$REPO  = "C:\Users\victus\Desktop\bitirmetezi"
$BUILD = "$REPO\build"
$GCC   = "C:\arm-gnu-toolchain-15.2.rel1-mingw-w64-i686-arm-none-eabi\bin\arm-none-eabi-gcc.exe"
$SIZE  = "C:\arm-gnu-toolchain-15.2.rel1-mingw-w64-i686-arm-none-eabi\bin\arm-none-eabi-size.exe"
$FVP   = "C:\Users\victus\AppData\Local\Keil_v5\ARM\avh-fvp\bin\models\FVP_MPS2_Cortex-M0.exe"
$ELF   = "$BUILD\orko_sim.elf"
$MAP   = "$BUILD\orko_sim.map"
if (-not (Test-Path $BUILD)) { New-Item -ItemType Directory -Path $BUILD | Out-Null }
if (-not (Test-Path $GCC))   { Write-Host "[HATA] GCC yok" -ForegroundColor Red; exit 1 }
$CPU = @("-mcpu=cortex-m0","-mthumb","-mfloat-abi=soft")
$INC = @("-I$REPO\app","-I$REPO\algo","-I$REPO\sim","-I$REPO\drivers","-I$REPO\sensors","-I$REPO\lora","-I$REPO\power")
$CF  = $CPU + @("-DORKO_SIM","-Wall","-Wextra","-O0","-g3","-fdata-sections","-ffunction-sections") + $INC
$ASF = $CPU + @("-g3")
$LDF = $CPU + @("-T$REPO\linker\M031_sim.ld","-Wl,--gc-sections","-specs=rdimon.specs","-lc","-lm","-lrdimon","-Wl,-Map=$MAP")
function GCC-Run([string[]]$a,[string]$lbl) { Write-Host "  $lbl" -ForegroundColor Cyan; & $GCC @a; if ($LASTEXITCODE -ne 0){Write-Host "[HATA] $lbl" -ForegroundColor Red;exit 1} }
Write-Host "`n============================================================" -ForegroundColor Green
Write-Host "  ORKO - Simulasyon Build" -ForegroundColor Green
Write-Host "============================================================`n" -ForegroundColor Green
$objs = @()
$o = "$BUILD\startup_M031Series.o"; GCC-Run ($ASF+@("-c","$REPO\startup\startup_M031Series.s","-o",$o)) "AS startup_M031Series.s"; $objs += $o
$o = "$BUILD\fire_score.o";         GCC-Run ($CF+@("-c","$REPO\algo\fire_score.c","-o",$o))              "CC fire_score.c";         $objs += $o
$o = "$BUILD\sim_main.o";           GCC-Run ($CF+@("-c","$REPO\sim\sim_main.c","-o",$o))                "CC sim_main.c";           $objs += $o
$o = "$BUILD\fake_sensors.o";       GCC-Run ($CF+@("-c","$REPO\sim\fake_sensors.c","-o",$o))            "CC fake_sensors.c";       $objs += $o
$o = "$BUILD\sim_display.o";        GCC-Run ($CF+@("-c","$REPO\sim\sim_display.c","-o",$o))             "CC sim_display.c";        $objs += $o
$o = "$BUILD\sim_uart.o";           GCC-Run ($CF+@("-c","$REPO\sim\sim_uart.c","-o",$o))                "CC sim_uart.c";           $objs += $o
Write-Host "  LD orko_sim.elf" -ForegroundColor Cyan
& $GCC @($objs+$LDF+@("-o",$ELF)); if ($LASTEXITCODE -ne 0){Write-Host "[HATA] Link basarisiz" -ForegroundColor Red;exit 1}
Write-Host "`n  Bellek Kullanimi:" -ForegroundColor Yellow; & $SIZE $ELF
Write-Host "`n============================================================" -ForegroundColor Green
Write-Host "  [OK] Build basarili  ->  build\orko_sim.elf" -ForegroundColor Green
Write-Host "============================================================`n" -ForegroundColor Green
if (-not (Test-Path $FVP)){ Write-Host "[UYARI] FVP bulunamadi." -ForegroundColor Yellow;exit 0 }
Write-Host "  FVP baslatiliyor..." -ForegroundColor Cyan
Write-Host "  Semihosting aktif: printf direkt terminale gelir" -ForegroundColor Cyan
Write-Host "============================================================`n" -ForegroundColor Green
& $FVP `
    "-a" $ELF `
    "-C" "armcortexm0ct.semihosting-enable=1" `
    "-C" "fvp_mps2.telnetterminal0.start_telnet=0" `
    "-C" "fvp_mps2.telnetterminal1.start_telnet=0" `
    "-C" "fvp_mps2.telnetterminal2.start_telnet=0" `
    "-C" "fvp_mps2.mps2_visualisation.disable-visualisation=1"
Write-Host "`n============================================================" -ForegroundColor Green
Write-Host "  Simulasyon tamamlandi." -ForegroundColor Green
Write-Host "============================================================`n" -ForegroundColor Green
