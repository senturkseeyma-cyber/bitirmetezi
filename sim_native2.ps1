# ============================================================
# sim_native2.ps1  -  ORKO Native Simulasyon v2 (Parametreli)
# Kullanim ornekleri:
#   .\sim_native2.ps1                          # Varsayilan (AUTO, 300ms)
#   .\sim_native2.ps1 -StepDelay 0             # Hizli otomatik
#   .\sim_native2.ps1 -Mode MANUAL             # Her adimda el ile deger gir
#   .\sim_native2.ps1 -Mode SIM -StepDelay 100 # SIM zamanlama + hizli
#   .\sim_native2.ps1 -Mode GERCEK             # Gercek donanim zamanlama modeli
#   .\sim_native2.ps1 -Help                    # Yardim
# ============================================================
param(
    [int]$StepDelay  = 300,     # Adim arasi bekleme (ms)
    [ValidateSet("AUTO","MANUAL","SIM","GERCEK")]
    [string]$Mode    = "MANUAL",  # AUTO | MANUAL | SIM | GERCEK
    [switch]$Help
)

if ($Help) {
    Write-Host ""
    Write-Host "ORKO sim_native2.ps1 - Kullanim" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "  -StepDelay <ms>    Adim arasi bekleme suresi (varsayilan: 300)"
    Write-Host "  -Mode AUTO         Otomatik, senaryo tablosundan oku (varsayilan)"
    Write-Host "  -Mode MANUAL       Her adimda el ile sensor degeri gir"
    Write-Host "  -Mode SIM          SIM zamanlama sabitlerini goster (5s/15s/60s)"
    Write-Host "  -Mode GERCEK       Gercek donanim zamanlama sabitlerini goster (120s/600s/7200s)"
    Write-Host "  -Help              Bu yardimi goster"
    Write-Host ""
    Write-Host "  MANUAL mod komutlari:" -ForegroundColor Yellow
    Write-Host "    <Enter>                    -> Senaryo degerini kullan"
    Write-Host "    T=80                       -> Sadece sicakligi gecersiz kil"
    Write-Host "    T=80 CO2=5500 CO=220       -> Birden fazla degeri gecersiz kil"
    Write-Host "    T=80 CO2=5500 CO=220 P=94000 H=8  -> Tum degerleri gir"
    Write-Host "    SKIP                       -> Bu adimi atla"
    Write-Host "    AUTO                       -> Kalan adimlari otomatik devam et"
    Write-Host ""
    exit 0
}

# --- Zamanlama sabitleri ---
if ($Mode -eq "GERCEK") {
    $T_SENTINEL   = 120;  $T_FULLREAD = 600;  $T_CYCLE = 7200
    $ModeLabel    = "GERCEK DONANIM"
    $ModeColor    = "Magenta"
    if ($StepDelay -eq 300) { $StepDelay = 5000 }   # GERCEK modda varsayilan 5sn
} elseif ($Mode -eq "SIM" -or $Mode -eq "AUTO") {
    $T_SENTINEL   = 5;    $T_FULLREAD = 15;   $T_CYCLE = 60
    $ModeLabel    = if ($Mode -eq "SIM") { "SIMULASYON (SIM)" } else { "AUTO (SIM zamanlama)" }
    $ModeColor    = "Cyan"
} else {
    # MANUAL
    $T_SENTINEL   = 5;    $T_FULLREAD = 15;   $T_CYCLE = 60
    $ModeLabel    = "MANUEL GIRIS"
    $ModeColor    = "Yellow"
}

# --- Algoritma sabitleri (orko_config.h ile ayni) ---
$THRESHOLD    = 0.60
$T_CRIT       = 70.0
$CO2_CRIT     = 5000.0
$CO_CRIT      = 200.0
$P_CRIT_LOW   = 95000.0
$H_CRIT_LOW   = 15.0

$W_T  = 0.25; $W_CO2 = 0.25; $W_CO = 0.25; $W_P = 0.15; $W_H = 0.10

$SAMPLE_BUF_SIZE = 5

# --- Sahte sensor verisi (fake_sensors.c ile ayni 20 adim) ---
$scenario = @(
    @{T=22.0;  CO2=412.0;  CO=5.0;   P=101300.0; H=64.0; Phase="FAZ-1: NORMAL  Orman Kosullari"},
    @{T=22.5;  CO2=418.0;  CO=5.5;   P=101200.0; H=63.5; Phase="FAZ-1: NORMAL  Orman Kosullari"},
    @{T=23.0;  CO2=425.0;  CO=6.0;   P=101100.0; H=63.0; Phase="FAZ-1: NORMAL  Orman Kosullari"},
    @{T=23.5;  CO2=430.0;  CO=6.5;   P=101000.0; H=62.5; Phase="FAZ-1: NORMAL  Orman Kosullari"},
    @{T=24.0;  CO2=435.0;  CO=7.0;   P=100900.0; H=62.0; Phase="FAZ-1: NORMAL  Orman Kosullari"},
    @{T=28.0;  CO2=700.0;  CO=20.0;  P=100500.0; H=56.0; Phase="FAZ-2: ERKEN DUMAN  Belirtisi"},
    @{T=33.0;  CO2=1100.0; CO=38.0;  P=100000.0; H=50.0; Phase="FAZ-2: ERKEN DUMAN  Belirtisi"},
    @{T=38.0;  CO2=1600.0; CO=60.0;  P=99500.0;  H=44.0; Phase="FAZ-2: ERKEN DUMAN  Belirtisi"},
    @{T=43.0;  CO2=2200.0; CO=85.0;  P=99000.0;  H=37.0; Phase="FAZ-2: ERKEN DUMAN  Belirtisi"},
    @{T=48.0;  CO2=2900.0; CO=115.0; P=98300.0;  H=30.0; Phase="FAZ-2: ERKEN DUMAN  Belirtisi"},
    @{T=54.0;  CO2=3600.0; CO=140.0; P=97500.0;  H=24.0; Phase="FAZ-3: YANGIN GELISIYOR !"},
    @{T=60.0;  CO2=4000.0; CO=160.0; P=96800.0;  H=20.0; Phase="FAZ-3: YANGIN GELISIYOR !"},
    @{T=65.0;  CO2=4400.0; CO=180.0; P=96000.0;  H=17.0; Phase="FAZ-3: YANGIN GELISIYOR !"},
    @{T=68.0;  CO2=4700.0; CO=195.0; P=95400.0;  H=14.0; Phase="FAZ-3: YANGIN GELISIYOR !"},
    @{T=71.0;  CO2=4950.0; CO=199.0; P=95100.0;  H=12.0; Phase="FAZ-3: YANGIN GELISIYOR !"},
    @{T=74.0;  CO2=5200.0; CO=215.0; P=94800.0;  H=10.0; Phase="FAZ-4: *** YANGIN ALARMI ***"},
    @{T=77.0;  CO2=5600.0; CO=240.0; P=94200.0;  H=8.0;  Phase="FAZ-4: *** YANGIN ALARMI ***"},
    @{T=80.0;  CO2=6100.0; CO=265.0; P=93600.0;  H=6.0;  Phase="FAZ-4: *** YANGIN ALARMI ***"},
    @{T=83.0;  CO2=6600.0; CO=290.0; P=93000.0;  H=5.0;  Phase="FAZ-4: *** YANGIN ALARMI ***"},
    @{T=86.0;  CO2=7100.0; CO=320.0; P=92500.0;  H=3.0;  Phase="FAZ-4: *** YANGIN ALARMI ***"}
)

# --- Yardimci fonksiyonlar ---
function NormPos([double]$cur, [double]$avg, [double]$crit) {
    $denom = $crit - $avg
    if ($denom -le 0) { return 0.0 }
    $r = ($cur - $avg) / $denom
    if ($r -lt 0) { return 0.0 }
    if ($r -gt 1) { return 1.0 }
    return $r
}

function NormNeg([double]$cur, [double]$avg, [double]$crit) {
    $denom = $avg - $crit
    if ($denom -le 0) { return 0.0 }
    $r = ($avg - $cur) / $denom
    if ($r -lt 0) { return 0.0 }
    if ($r -gt 1) { return 1.0 }
    return $r
}

function Bar([double]$val) {
    $filled = [int]($val * 20)
    $s = "["
    for ($i = 0; $i -lt 20; $i++) { $s += if ($i -lt $filled) { "#" } else { "-" } }
    $s += "]"
    return $s
}

function Fmt1([double]$v) { return $v.ToString("0.0") }
function Fmt2([double]$v) { return $v.ToString("0.00") }
function Fmt3([double]$v) { return $v.ToString("0.000") }

# --- Manuel giris parser ---
function Parse-Override([string]$line, [ref]$T, [ref]$co2, [ref]$co, [ref]$P, [ref]$H) {
    $tokens = $line.Trim() -split '\s+'
    foreach ($tok in $tokens) {
        if ($tok -match '^T=(.+)$')    { try { $T.Value   = [double]$Matches[1] } catch {} }
        if ($tok -match '^CO2=(.+)$')  { try { $co2.Value = [double]$Matches[1] } catch {} }
        if ($tok -match '^CO=(.+)$')   { try { $co.Value  = [double]$Matches[1] } catch {} }
        if ($tok -match '^P=(.+)$')    { try { $P.Value   = [double]$Matches[1] } catch {} }
        if ($tok -match '^H=(.+)$')    { try { $H.Value   = [double]$Matches[1] } catch {} }
    }
}

# --- Dairesel tampon ---
$bufT   = @(0.0) * $SAMPLE_BUF_SIZE
$bufCO2 = @(0.0) * $SAMPLE_BUF_SIZE
$bufCO  = @(0.0) * $SAMPLE_BUF_SIZE
$bufP   = @(0.0) * $SAMPLE_BUF_SIZE
$bufH   = @(0.0) * $SAMPLE_BUF_SIZE
$bufCnt = 0; $bufHead = 0

# --- Baslik ---
Write-Host ""
Write-Host "============================================================" -ForegroundColor Green
Write-Host "  ORKO - Orman Yangini Erken Tespit ve Koruma Sistemi" -ForegroundColor Green
Write-Host "  NATIVE SIMULASYON v2  |  Slave Node #1  |  20 Adim" -ForegroundColor Green
Write-Host "============================================================" -ForegroundColor Green
Write-Host ""
Write-Host ("  Mod          : {0}" -f $ModeLabel) -ForegroundColor $ModeColor
Write-Host ("  Adim Gecikmesi: {0} ms" -f $StepDelay)
Write-Host ""
Write-Host "  -- Zamanlama Sabitleri --" -ForegroundColor $ModeColor
Write-Host ("     Sentinel Araligi : {0,5} sn   (Gercek: 120 sn)" -f $T_SENTINEL)
Write-Host ("     Tam Okuma Araligi: {0,5} sn   (Gercek: 600 sn)" -f $T_FULLREAD)
Write-Host ("     Dongu Suresi     : {0,5} sn   (Gercek: 7200 sn)" -f $T_CYCLE)
Write-Host ""
Write-Host "  Algoritma : FireScore = 0.25*NT + 0.25*NCO2 + 0.25*NCO + 0.15*NP + 0.10*NH"
Write-Host ""
Write-Host "  Alarm Esigi  : FireScore > $THRESHOLD"
Write-Host "  Krit.Sicaklik: >= $T_CRIT C"
Write-Host "  Krit.CO2     : >= $CO2_CRIT ppm"
Write-Host "  Krit.CO      : >= $CO_CRIT ppm"
Write-Host ""
Write-Host "  GPS Koordinat: 39.9254N  32.8597E  850m (Ankara/Orman)"
Write-Host ""

if ($Mode -eq "MANUAL") {
    Write-Host "  [MANUEL MOD] Her adimda sensor degeri girebilirsiniz." -ForegroundColor Yellow
    Write-Host "  Komutlar: <Enter>=senaryo | T=XX CO2=XX CO=XX P=XX H=XX | SKIP | AUTO" -ForegroundColor Yellow
    Write-Host ""
}

Write-Host "  Simulasyon basliyor..." -ForegroundColor Cyan
Start-Sleep -Milliseconds 500

# AUTO mod takibi (MANUAL modda AUTO komutu girilirse)
$autoMode = ($Mode -ne "MANUAL")

# --- Ana simulasyon dongusu ---
for ($i = 0; $i -lt 20; $i++) {
    $s = $scenario[$i]
    $T = $s.T; $co2 = $s.CO2; $co = $s.CO; $P = $s.P; $H = $s.H

    # --- MANUEL MOD: kullanicidan giris al ---
    if ($Mode -eq "MANUAL" -and -not $autoMode) {
        Write-Host ""
        Write-Host ("--- Adim {0,2} / 20  |  Senaryo: {1}" -f ($i+1), $s.Phase) -ForegroundColor Yellow
        Write-Host ("    Senaryo degerleri: T={0}  CO2={1}  CO={2}  P={3}  H={4}" -f `
            (Fmt1 $T), ([int]$co2), (Fmt1 $co), ([int]$P), (Fmt1 $H)) -ForegroundColor DarkGray
        Write-Host "    Deger girin (bos=senaryo, SKIP=atla, AUTO=otomatik devam):" -ForegroundColor Yellow
        $input_line = Read-Host "    >"

        $input_line = $input_line.Trim()
        if ($input_line -ieq "SKIP") {
            Write-Host ("    -> Adim {0} atlandi." -f ($i+1)) -ForegroundColor DarkGray
            continue
        } elseif ($input_line -ieq "AUTO") {
            Write-Host "    -> Kalan adimlar otomatik devam edecek." -ForegroundColor Cyan
            $autoMode = $true
        } elseif ($input_line -ne "") {
            Parse-Override $input_line ([ref]$T) ([ref]$co2) ([ref]$co) ([ref]$P) ([ref]$H)
            Write-Host ("    -> Kullanilan degerler: T={0}  CO2={1}  CO={2}  P={3}  H={4}" -f `
                (Fmt1 $T), ([int]$co2), (Fmt1 $co), ([int]$P), (Fmt1 $H)) -ForegroundColor Cyan
        }
    }

    # Tampona ekle
    $idx = $bufHead % $SAMPLE_BUF_SIZE
    $bufT[$idx]   = $T
    $bufCO2[$idx] = $co2
    $bufCO[$idx]  = $co
    $bufP[$idx]   = $P
    $bufH[$idx]   = $H
    $bufHead++
    if ($bufCnt -lt $SAMPLE_BUF_SIZE) { $bufCnt++ }

    # FireScore hesapla (son 5 ornekten ortalama al)
    $cnt = [Math]::Min($bufCnt, $SAMPLE_BUF_SIZE)
    $sumT = 0.0; $sumCO2 = 0.0; $sumCO = 0.0; $sumP = 0.0; $sumH = 0.0
    for ($j = 0; $j -lt $cnt; $j++) {
        $jj = ($bufHead - 1 - $j) % $SAMPLE_BUF_SIZE
        if ($jj -lt 0) { $jj += $SAMPLE_BUF_SIZE }
        $sumT   += $bufT[$jj]
        $sumCO2 += $bufCO2[$jj]
        $sumCO  += $bufCO[$jj]
        $sumP   += $bufP[$jj]
        $sumH   += $bufH[$jj]
    }
    $aT = $sumT / $cnt; $aCO2 = $sumCO2 / $cnt; $aCO = $sumCO / $cnt
    $aP = $sumP / $cnt; $aH = $sumH / $cnt

    $nT   = NormPos $T   $aT   $T_CRIT
    $nCO2 = NormPos $co2 $aCO2 $CO2_CRIT
    $nCO  = NormPos $co  $aCO  $CO_CRIT
    $nP   = NormNeg $P   $aP   $P_CRIT_LOW
    $nH   = NormNeg $H   $aH   $H_CRIT_LOW

    $score = $W_T*$nT + $W_CO2*$nCO2 + $W_CO*$nCO + $W_P*$nP + $W_H*$nH

    $tCrit   = $T -ge $T_CRIT
    $co2Crit = $co2 -ge $CO2_CRIT
    $coCrit  = $co -ge $CO_CRIT
    $alarm   = ($score -gt $THRESHOLD) -and $tCrit -and ($co2Crit -or $coCrit)

    # --- Cikti ---
    Write-Host ""
    Write-Host "============================================================" -ForegroundColor $(if ($alarm) { "Red" } elseif ($score -gt 0.5) { "Yellow" } else { "Green" })
    Write-Host ("  ORKO Slave #1  |  Adim: {0,2} / 20  |  Mod: {1}" -f ($i+1), $ModeLabel)
    Write-Host ("  Senaryo : {0}" -f $s.Phase)
    Write-Host "------------------------------------------------------------"
    Write-Host ("  Sicaklik : {0,6} C      |  CO2     : {1,7} ppm" -f (Fmt1 $T), ([int]$co2))
    Write-Host ("  Nem      : {0,6} %      |  CO      : {1,7} ppm" -f (Fmt1 $H), (Fmt1 $co))
    Write-Host ("  Basinc   : {0,6} Pa     |  GPS     : 39.9254N  32.8597E  850m" -f ([int]$P))
    Write-Host ("  Kritik?  : Sicak={0,-5} CO2={1,-5} CO={2,-5}" -f `
        $(if ($tCrit) {"EVET"} else {"Hayir"}), `
        $(if ($co2Crit) {"EVET"} else {"Hayir"}), `
        $(if ($coCrit) {"EVET"} else {"Hayir"}))
    Write-Host "------------------------------------------------------------"

    Write-Host "  Normalize Faktorler  (0.0 = Normal, 1.0 = Kritik):"
    Write-Host ("    N_Sicak : {0}  {1}" -f (Fmt3 $nT),   (Bar $nT))
    Write-Host ("    N_CO2   : {0}  {1}" -f (Fmt3 $nCO2), (Bar $nCO2))
    Write-Host ("    N_CO    : {0}  {1}" -f (Fmt3 $nCO),  (Bar $nCO))
    Write-Host ("    N_Bsnc  : {0}  {1}" -f (Fmt3 $nP),   (Bar $nP))
    Write-Host ("    N_Nem   : {0}  {1}" -f (Fmt3 $nH),   (Bar $nH))
    Write-Host "------------------------------------------------------------"

    $scoreColor = if ($alarm) { "Red" } elseif ($score -gt 0.5) { "Yellow" } else { "Cyan" }
    Write-Host ("  FIRE SCORE : {0}  {1}  (Esik: {2})" -f (Fmt3 $score), (Bar $score), $THRESHOLD) -ForegroundColor $scoreColor

    if ($alarm) {
        Write-Host ""
        Write-Host "  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" -ForegroundColor Red
        Write-Host "  !!!            YANGIN ALARMI TETIKLENDI               !!!" -ForegroundColor Red -BackgroundColor DarkRed
        Write-Host "  !!!   LoRa ALARM paketi Master'a GONDERILIYOR (sim)   !!!" -ForegroundColor Red
        Write-Host "  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" -ForegroundColor Red
        Write-Host ("  Paket : PKT_TYPE_ALARM | Slave=0x0001 -> Master=0x0000") -ForegroundColor Red
    } elseif ($score -gt 0.50) {
        Write-Host "  Durum  : [!!] YUKSEK RISK - Esige Yakin! Takip Ediliyor." -ForegroundColor Yellow
    } elseif ($score -gt 0.25) {
        Write-Host "  Durum  : [!]  Artan Risk Tespit Edildi. Izleniyor." -ForegroundColor Yellow
    } else {
        Write-Host "  Durum  : [OK] Normal - Alarm Yok." -ForegroundColor Green
    }

    Write-Host "============================================================" -ForegroundColor $(if ($alarm) { "Red" } elseif ($score -gt 0.5) { "Yellow" } else { "Green" })

    if ($StepDelay -gt 0) {
        Start-Sleep -Milliseconds $StepDelay
    }
}

Write-Host ""
Write-Host "============================================================" -ForegroundColor Green
Write-Host "  Simulasyon tamamlandi. 20/20 adim islendi." -ForegroundColor Green
Write-Host ("  Mod: {0}  |  Adim Gecikmesi: {1} ms" -f $ModeLabel, $StepDelay) -ForegroundColor $ModeColor
Write-Host "============================================================" -ForegroundColor Green
