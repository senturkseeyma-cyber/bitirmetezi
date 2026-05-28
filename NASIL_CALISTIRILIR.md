# ORKO — Proje Geliştirme Kılavuzu
## Ne Yaptık & Nasıl Test Ederiz

**Proje:** ORKO Orman Yangını Erken Tespit — Slave Node  
**MCU:** Nuvoton M031FB0AE (ARM Cortex-M0)  
**Tarih:** Mayıs 2026

---

## 1. Genel Bakış — Ne Yaptık?

Gerçek kart elimizde olmadığı için **iki katmanlı bir simülasyon ortamı** kurduk:

```
[Kaynak Kodlar (.c/.h)]
        |
        | arm-none-eabi-gcc ile derlenir
        v
[orko_sim.elf  — ARM binary]
        |
        | FVP_MPS2_Cortex-M0.exe içinde çalışır
        v
[Sanal Cortex-M0 işlemcisi koşuyor]
        |
        | UART0.out_file=- ile
        v
[VS Code terminali → Sensor verileri + Fire Score ekrana]
```

### Kullanılan Araçlar

| Araç | Konum | Ne İşe Yarıyor |
|---|---|---|
| **arm-none-eabi-gcc** | `C:\arm-gnu-toolchain-15.2...` | ARM Cortex-M0 için çapraz derleyici |
| **FVP_MPS2_Cortex-M0.exe** | `C:\Users\victus\AppData\Local\Keil_v5\ARM\avh-fvp\bin\models\` | Sanal ARM kartı — donanım gerektirmez |
| **cmake** | `C:\Program Files\CMake\bin\` | Build sistemi (ileride kullanılacak) |
| **VS Code** | — | Editör, task runner, debug arayüzü |

---

## 2. Proje Klasör Yapısı

```
bitirmetezi/
│
├── app/
│   ├── main.c              ← Gerçek firmware ana döngüsü (state machine)
│   └── orko_config.h       ← TÜM sabitler: pinler, eşikler, ağırlıklar
│
├── algo/
│   ├── fire_score.c        ← FireScore algoritması (tez Bölüm 3.3)
│   └── fire_score.h
│
├── drivers/                ← HAL katmanı (I2C, UART, ADC, Timer)
├── sensors/                ← Sensör sürücüleri (BMP390, SCD40, HDC2022, MQ9, GPS)
├── lora/                   ← LoRa E22 sürücüsü
├── power/                  ← Güç yönetimi
│
├── sim/                    ← SIMÜLASYON DOSYALARI (kart gerektirmez)
│   ├── fake_sensors.c/h    ← 20 adımlık sahte veri (4 faz: Normal→Alarm)
│   ├── sim_display.c/h     ← Konsol çıktı tablosu
│   ├── sim_main.c          ← Simülasyon ana fonksiyonu
│   └── sim_uart.c          ← FVP UART0 → printf yönlendirmesi
│
├── startup/
│   └── startup_M031Series.s  ← ARM Cortex-M0 vektör tablosu ve reset handler
│
├── linker/
│   └── M031_sim.ld         ← FVP bellek haritası linker scripti
│
├── cmake/
│   └── arm-none-eabi.cmake ← CMake toolchain tanımı
│
├── .vscode/
│   ├── tasks.json          ← Ctrl+Shift+B ile build görevi
│   ├── launch.json         ← F5 ile FVP debug
│   ├── settings.json       ← Araç yolları, CMake ayarları
│   └── c_cpp_properties.json ← IntelliSense (kod renklendirme için)
│
├── build_sim.ps1           ← TEK TIKLA BUILD + SİMÜLASYON scripti
├── build/                  ← Derleme çıktıları (otomatik oluşur)
│   ├── orko_sim.elf        ← ARM binary
│   └── orko_sim.map        ← Bellek haritası raporu
│
└── CMakeLists.txt          ← İlerideki BSP entegrasyonu için
```

---

## 3. Simülasyon Senaryosu — 20 Adım

Sahte sensör verileri 4 faza ayrılmıştır:

| Adımlar | Faz | Açıklama |
|---|---|---|
| 1–5 | FAZ-1: NORMAL | Sıcaklık ~22°C, CO₂ ~430 ppm, CO ~6 ppm → FireScore < 0.1 |
| 6–10 | FAZ-2: ERKEN DUMAN | Sıcaklık 28→48°C, CO₂ 700→2900 ppm → FireScore 0.1→0.4 |
| 11–15 | FAZ-3: YANGIN GELİŞİYOR | Sıcaklık 54→71°C, CO₂ 3600→4950 ppm → FireScore 0.5→0.8 |
| 16–20 | FAZ-4: YANGIN ALARMI | Sıcaklık 74→86°C, CO₂ 5200→7100 ppm → **ALARM tetikleniyor** |

### Alarm Koşulu (Tez Bölüm 3.3.5)
```
FireScore > 0.60 VE Sıcaklık ≥ 70°C VE (CO₂ ≥ 5000 VEYA CO ≥ 200 ppm)
```

---

## 4. Nasıl Çalıştırılır?

### Yöntem 1 — PowerShell Script (En Kolay)

**VS Code terminalinde:**
```powershell
cd "C:\Users\victus\Desktop\bitirmetezi"
powershell -ExecutionPolicy Bypass -File .\build_sim.ps1
```

**Veya Windows Gezgini'nden:**  
`build_sim.ps1` dosyasına sağ tık → **"PowerShell ile Çalıştır"**

Script otomatik olarak:
1. Tüm `.c` ve `.s` dosyalarını `arm-none-eabi-gcc` ile derler
2. `build\orko_sim.elf` oluşturur
3. FVP simülatörünü başlatır
4. Simülasyon çıktısını terminale yazar

---

### Yöntem 2 — VS Code Task (Ctrl+Shift+B)

1. VS Code'da projeyi açın
2. `Ctrl + Shift + B` tuşlayın  
3. **"ORKO: Build (Simulasyon ELF)"** görevi seçin
4. Terminal panelinde çıktıyı izleyin

---

### Yöntem 3 — Sadece ELF Derle, FVP Manuel Çalıştır

**Önce derle:**
```powershell
$GCC = "C:\arm-gnu-toolchain-15.2.rel1-mingw-w64-i686-arm-none-eabi\bin\arm-none-eabi-gcc.exe"
& $GCC -mcpu=cortex-m0 -mthumb -mfloat-abi=soft -DORKO_SIM `
    -I.\app -I.\algo -I.\sim `
    -specs=nosys.specs -O0 -g3 `
    -T.\linker\M031_sim.ld `
    .\startup\startup_M031Series.s `
    .\algo\fire_score.c .\sim\sim_main.c .\sim\fake_sensors.c `
    .\sim\sim_display.c .\sim\sim_uart.c `
    -lc -lm -lnosys -o build\orko_sim.elf
```

**Sonra FVP ile çalıştır:**
```powershell
$FVP = "C:\Users\victus\AppData\Local\Keil_v5\ARM\avh-fvp\bin\models\FVP_MPS2_Cortex-M0.exe"
& $FVP -a build\orko_sim.elf --simlimit 600000 `
    -C armcortexm0ct.semihosting-enable=0 `
    -C fvp_mps2.UART0.out_file=- `
    -C fvp_mps2.UART0.shutdown_on_eot=1 `
    -C fvp_mps2.telnetterminal0.start_telnet=0 `
    -C fvp_mps2.telnetterminal1.start_telnet=0 `
    -C fvp_mps2.telnetterminal2.start_telnet=0 `
    -C fvp_mps2.mps2_visualisation.disable-visualisation=1
```

---

## 5. Çıktı Nasıl Görünür?

Terminalde şuna benzer bir tablo görünmeli:

```
============================================================
  ORKO Slave #1  |  Adim:  8 / 20
  Senaryo : FAZ-2: ERKEN DUMAN  Belirtisi
------------------------------------------------------------
  Sicaklik :   43.0 C      |  CO2     :    2200 ppm
  Nem      :   37.0 %      |  CO      :    85.0 ppm
  Basinc   :  99000 Pa     |  GPS     : 39.9254N  32.8597E  850m
  Kritik?  : Sicak=Hayir CO2=Hayir  CO=Hayir
------------------------------------------------------------
  Normalize Faktorler  (0.0 = Normal, 1.0 = Kritik):
    N_Sicak : 0.212  [####----------------]
    N_CO2   : 0.358  [#######-------------]
    N_CO    : 0.371  [#######-------------]
    N_Bsnc  : 0.041  [#-------------------]
    N_Nem   : 0.201  [####----------------]
------------------------------------------------------------
  FIRE SCORE : 0.278  [#####---------------]  (Esik: 0.60)
  Durum  : [!]  Artan Risk Tespit Edildi. Izleniyor.
============================================================
```

**Adım 16'da ALARM tetiklenince:**
```
  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  !!!            YANGIN ALARMI TETIKLENDI               !!!
  !!!   LoRa ALARM paketi Master'a GONDERILIYOR (sim)   !!!
  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  Paket : PKT_TYPE_ALARM | Slave=0x0001 -> Master=0x0000
```

---

## 6. Sık Karşılaşılan Sorunlar

### "telnet.exe bulunamıyor" hatası

**Neden:** FVP terminal pencerelerini telnet ile açmaya çalışıyor, Windows'ta telnet varsayılan kapalı.  
**Çözüm:** `build_sim.ps1` içinde aşağıdaki parametreler zaten eklenmiş:
```
-C fvp_mps2.telnetterminal0.start_telnet=0
-C fvp_mps2.telnetterminal1.start_telnet=0
-C fvp_mps2.telnetterminal2.start_telnet=0
```
Bu parametreler telnet penceresini engeller, çıktı terminale gelir.

---

### "CLCD Cortex-M0 MPS2" penceresi açılıyor

**Neden:** FVP varsayılan olarak sanal LED/LCD ekranı gösterir.  
**Çözüm:** Zaten eklendi:
```
-C fvp_mps2.mps2_visualisation.disable-visualisation=1
```

---

### "arm-none-eabi-gcc is not recognized" hatası

PATH'e ekli değil. PowerShell'de manuel ekle:
```powershell
$env:PATH = "C:\arm-gnu-toolchain-15.2.rel1-mingw-w64-i686-arm-none-eabi\bin;" + $env:PATH
```
Kalıcı yapmak için: **Sistem Özellikleri → Ortam Değişkenleri → Path → Yeni** satırı ekle:  
`C:\arm-gnu-toolchain-15.2.rel1-mingw-w64-i686-arm-none-eabi\bin`

---

### Linker uyarıları (_getpid, _kill)

```
warning: _getpid is not implemented and will always fail
warning: _kill is not implemented and will always fail
```
**Bu uyarılar normaldir**, hata değildir. `-nosys.specs` ile eksik sistem çağrıları stub olarak bırakılır. ELF derlenir ve çalışır.

---

## 7. Simülasyon → Gerçek Kart Geçişi (İleride)

Gerçek Nuvoton M031FB0AE kartı geldiğinde:

1. **Nuvoton M031 BSP indir:**  
   https://github.com/OpenNuvoton/M031BSP  
   `BSP/M031Series/` klasörüne koy

2. **CMakeLists.txt'de firmware modu aç:**
   ```
   cmake -DBUILD_FIRMWARE=ON -DBSP_DIR=BSP/M031Series ...
   ```

3. **`orko_fw.elf` oluşur** — Nu-Link debugger ile karta yükle

4. **`orko_config.h`'daki `#ifndef ORKO_SIM` blokları** otomatik aktif olur,  
   gerçek `NuMicro.h` hardware register'larını kullanır

---

## 8. Sonraki Adımlar

- [ ] MQ9 sensörü R0 kalibrasyonu (sahada, 3 dk ısınma sonrası)
- [ ] LoRa kanal frekansı doğrulama (Türkiye 868 MHz bandı → Kanal 17/18)
- [ ] Master kart yazılımı (GSM modülü, alarm SMS)
- [ ] Nuvoton BSP entegrasyonu ve gerçek firmware derlemesi
- [ ] Saha testi: 5 örnek tampon dolduğunda FireScore doğrulama

---

## 9. Özet Komutlar

```powershell
# --- Build ve Simülasyon ---
cd "C:\Users\victus\Desktop\bitirmetezi"
powershell -ExecutionPolicy Bypass -File .\build_sim.ps1

# --- Sadece ELF boyutunu gör ---
C:\arm-gnu-toolchain-15.2.rel1-mingw-w64-i686-arm-none-eabi\bin\arm-none-eabi-size.exe build\orko_sim.elf

# --- ELF içindeki sembolleri listele ---
C:\arm-gnu-toolchain-15.2.rel1-mingw-w64-i686-arm-none-eabi\bin\arm-none-eabi-nm.exe build\orko_sim.elf | Select-String "FireScore|main|UART"

# --- Map dosyasına bak (bellek dağılımı) ---
notepad build\orko_sim.map
```
