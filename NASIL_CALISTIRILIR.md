# ORKO — Proje Çalıştırma ve Geliştirme Kılavuzu

**Proje:** ORKO Orman Yangını Erken Tespit — Slave Node  
**MCU:** Nuvoton M031FB0AE (ARM Cortex-M0)  
**Tarih:** Mayıs 2026

---

## 1. Genel Bakış

ORKO projesi iki ayrı modda çalışabilir:

| Mod | Açıklama | Donanım Gereksinimi |
|---|---|---|
| **Simülasyon (SIM)** | PC üzerinde sahte sensör verisiyle algoritma testi | YOK — sadece PC |
| **Gerçek Firmware** | Nuvoton M031FB0AE kartına flash'lanan gerçek kod | M031FB0AE kart + Nu-Link |

Simülasyon modu kendi içinde üç yönteme ayrılır:

| Yöntem | Araç | Hız | Donanım |
|---|---|---|---|
| **build_sim.ps1** | ARM GCC + FVP simülatör | Yavaş (UART baud simülasyonu) | YOK |
| **sim_native.ps1** | Saf PowerShell | Anında (~300ms/adım) | YOK |
| **sim_native2.ps1** | Saf PowerShell — Parametreli | Ayarlanabilir | YOK |

---

## 2. Proje Klasör Yapısı

```
bitirmetezi/
│
├── app/
│   ├── main.c              ← Gerçek firmware ana döngüsü (state machine)
│   └── orko_config.h       ← TÜM sabitler: pinler, eşikler, ağırlıklar, zamanlama
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
├── sim/                    ← SIMÜLASYON DOSYALARI
│   ├── fake_sensors.c/h    ← 20 adımlık sahte veri (4 faz: Normal → Alarm)
│   ├── sim_display.c/h     ← Konsol çıktı tablosu
│   ├── sim_main.c          ← FVP simülasyon ana fonksiyonu
│   └── sim_uart.c          ← FVP UART0 register → printf yönlendirmesi
│
├── startup/
│   └── startup_M031Series.s  ← ARM Cortex-M0 vektör tablosu + reset handler
│
├── linker/
│   └── M031_sim.ld         ← FVP bellek haritası (Flash 4MB @ 0x00000000)
│
├── build_sim.ps1           ← ARM GCC ile derle + FVP başlat
├── sim_native.ps1          ← Hızlı PowerShell simülasyonu (sabit, dokunulmaz)
├── sim_native2.ps1         ← Hızlı PowerShell simülasyonu (parametreli)
├── ORKO_Slave.uvprojx      ← Keil µVision proje dosyası (çift tıkla aç)
│
└── build/                  ← Derleme çıktıları (otomatik oluşur)
    ├── orko_sim.elf
    └── orko_sim.map
```

---

## 3. Simülasyon Senaryosu — 20 Adım, 4 Faz

```
Adım  1- 5 │ FAZ-1: NORMAL            │ T≈22°C, CO2≈430ppm  → FireScore < 0.10
Adım  6-10 │ FAZ-2: ERKEN DUMAN       │ T=28→48°C, CO2=700→2900ppm → 0.10→0.40
Adım 11-15 │ FAZ-3: YANGIN GELİŞİYOR  │ T=54→71°C, CO2=3600→4950ppm → 0.40→0.80
Adım 16-20 │ FAZ-4: YANGIN ALARMI     │ T=74→86°C, CO2=5200→7100ppm → ALARM
```

**Alarm koşulu (üçü birden sağlanmalı):**
```
FireScore > 0.60  VE  Sıcaklık ≥ 70°C  VE  (CO2 ≥ 5000 ppm VEYA CO ≥ 200 ppm)
```

**FireScore formülü:**
```
FireScore = 0.25×N_Sicak + 0.25×N_CO2 + 0.25×N_CO + 0.15×N_Basinc + 0.10×N_Nem

N_X (dogru orantili: T, CO2, CO)  = (X_simdiki - X_ort) / (X_kritik - X_ort)
N_X (ters orantili:  Basinc, Nem) = (X_ort - X_simdiki) / (X_ort - X_kritik)
```

Ortalama: dairesel tamponda tutulan son 5 ölçümün ortalaması.

---

## 4. YÖNTEM 1 — build_sim.ps1 (ARM GCC + FVP Simülatör)

### Ne yapar?

1. `arm-none-eabi-gcc` ile tüm `.c` ve `.s` dosyalarını Cortex-M0 binary'sine derler
2. `build/orko_sim.elf` ARM binary'sini oluşturur
3. `FVP_MPS2_Cortex-M0.exe` sanal ARM işlemcisini başlatır
4. ELF'i sanal işlemciye yükler ve çalıştırır
5. UART0 çıktısını terminale yönlendirir, CLCD penceresi açılır

### Çalıştırma

```powershell
cd "C:\Users\victus\Desktop\bitirmetezi"
powershell -ExecutionPolicy Bypass -File .\build_sim.ps1
```

### Adım adım iç işleyiş

```
build_sim.ps1 başlar
│
├─ [1] startup_M031Series.s  → build\startup_M031Series.o   (Assembler)
├─ [2] algo\fire_score.c     → build\fire_score.o            (C Derleyici)
├─ [3] sim\sim_main.c        → build\sim_main.o              (C Derleyici)
├─ [4] sim\fake_sensors.c    → build\fake_sensors.o          (C Derleyici)
├─ [5] sim\sim_display.c     → build\sim_display.o           (C Derleyici)
├─ [6] sim\sim_uart.c        → build\sim_uart.o              (C Derleyici)
│
├─ [7] Tüm .o dosyaları link edilir → build\orko_sim.elf
│       Bellek haritası: linker\M031_sim.ld
│       Flash @ 0x00000000, RAM @ 0x20000000
│
├─ [8] arm-none-eabi-size ile bellek kullanımı gösterilir
│
└─ [9] FVP_MPS2_Cortex-M0.exe başlar:
        ├─ semihosting-enable=0      (UART register modu)
        ├─ UART0.out_file=-          (çıktı terminale yönlendirilir)
        ├─ telnetterminal0.start_telnet=1   (CLCD telnet penceresi)
        └─ disable-visualisation=0  (sanal kart arayüzü görünür)
```

### Derleme bayrakları

```
-mcpu=cortex-m0 -mthumb -mfloat-abi=soft
-DORKO_SIM
-O0 -g3
-fdata-sections -ffunction-sections -Wl,--gc-sections
-specs=nosys.specs -lc -lm -lnosys
```

### Sınırlama

FVP UART baud simülasyonu her karakteri gerçek zamanlı simüle eder. 20 adımlık çıktı birkaç dakika sürebilir. Hızlı test için sim_native.ps1 kullan.

---

## 5. YÖNTEM 2 — sim_native.ps1 (Hızlı, Sabit Referans)

### Ne yapar?

ARM GCC veya FVP gerekmez. PowerShell'in kendisi `fire_score.c` ile birebir aynı algoritmayı hesaplar. Sahte sensör verileri aynıdır, çıktı formatı aynıdır.

**Bu dosyaya dokunulmaz — referans implementasyon olarak korunur.**

### Çalıştırma

```powershell
cd "C:\Users\victus\Desktop\bitirmetezi"
powershell -ExecutionPolicy Bypass -File .\sim_native.ps1
```

### Adım adım iç işleyiş

```
sim_native.ps1 başlar
│
├─ [1] Başlık yazdırılır: algoritma sabitleri, ağırlıklar, GPS, alarm eşiği
│
├─ [2] Dairesel tampon başlatılır: 5 elemanlı, sıfırdan dolu
│       NOT: Tampon sıfırdan başladığı için adım 1'de NormPos payda=0
│            olabilir (ortalama=anlık değer). Bu sim_native2 ile çözüldü.
│
├─ [3] 20 adımlık döngü başlar:
│       ├─ scenario[] tablosundan T, CO2, CO, P, H okunur
│       ├─ Dairesel tampona eklenir
│       ├─ Son 5 ölçümün ortalaması hesaplanır
│       ├─ N_T, N_CO2, N_CO, N_P, N_H normalize faktörleri hesaplanır
│       ├─ FireScore = 0.25×NT + 0.25×NCO2 + 0.25×NCO + 0.15×NP + 0.10×NH
│       ├─ Alarm koşulu: score>0.60 VE tCrit VE (co2Crit VEYA coCrit)
│       ├─ Renkli tablo + bar grafik ekrana yazdırılır
│       └─ Start-Sleep 300ms
│
└─ [4] Adım 16'da YANGIN ALARMI tetiklenir (kırmızı ekran)
        "LoRa ALARM paketi Master'a GÖNDERİLİYOR (sim)"
        "PKT_TYPE_ALARM | Slave=0x0001 -> Master=0x0000"
```

### Çıktı renk kodlaması

| Renk | Durum | FireScore |
|---|---|---|
| Yeşil | Normal | < 0.25 |
| Sarı | Artan risk | 0.25 – 0.50 |
| Sarı (parlak) | Yüksek risk | 0.50 – 0.60 |
| Kırmızı | ALARM | > 0.60 + kritik şartlar |

---

## 6. YÖNTEM 3 — sim_native2.ps1 (Parametreli, Geliştirilebilir)

### Ne yapar?

`sim_native.ps1` ile aynı algoritma. Ek özellikler:
- Adım gecikmesi parametrik (`-StepDelay`)
- Manuel sensör değeri girişi her adımda
- Zamanlama modu (SIM kısa / GERCEK uzun aralıklar)
- Başlangıç tamponu FAZ-1 baseline ile önceden dolu → adım 1'den doğru hesaplama

### Çalıştırma

```powershell
# Varsayılan (MANUAL mod, 300ms gecikme)
powershell -ExecutionPolicy Bypass -File .\sim_native2.ps1

# Otomatik, gecikme yok (en hızlı)
powershell -ExecutionPolicy Bypass -File .\sim_native2.ps1 -Mode AUTO -StepDelay 0

# Her adımda el ile değer gir
powershell -ExecutionPolicy Bypass -File .\sim_native2.ps1 -Mode MANUAL

# SIM zamanlama bilgisi + 100ms gecikme
powershell -ExecutionPolicy Bypass -File .\sim_native2.ps1 -Mode SIM -StepDelay 100

# Gerçek donanım zamanlama modelini göster (5sn/adım)
powershell -ExecutionPolicy Bypass -File .\sim_native2.ps1 -Mode GERCEK

# Yardım
powershell -ExecutionPolicy Bypass -File .\sim_native2.ps1 -Help
```

### Parametreler

| Parametre | Değerler | Varsayılan | Açıklama |
|---|---|---|---|
| `-StepDelay` | 0 – sınırsız (ms) | 300 | Adımlar arası bekleme |
| `-Mode` | AUTO, MANUAL, SIM, GERCEK | MANUAL | Çalışma modu |
| `-Help` | switch | — | Kullanım kılavuzu |

### Mod farkları

| Mod | Sentinel | Okuma | Döngü | Adım Gecikmesi | Girdi Kaynağı |
|---|---|---|---|---|---|
| AUTO | 5 sn | 15 sn | 60 sn | 300ms | Senaryo tablosu |
| MANUAL | 5 sn | 15 sn | 60 sn | 300ms | Her adımda kullanıcı |
| SIM | 5 sn | 15 sn | 60 sn | 300ms | Senaryo tablosu |
| GERCEK | 120 sn | 600 sn | 7200 sn | 5000ms | Senaryo tablosu |

### MANUAL mod komutları

Her adımda prompt açılır:

```
--- Adim  1 / 20  |  Senaryo: FAZ-1: NORMAL
    Senaryo degerleri: T=22,0  CO2=412  CO=5,0  P=101300  H=64,0
    Deger girin (bos=senaryo, SKIP=atla, AUTO=otomatik devam):
    >: _
```

| Girdi | Davranış |
|---|---|
| `<Enter>` (boş) | Senaryo tablosundaki değeri kullan |
| `T=80` | Sadece sıcaklığı geçersiz kıl |
| `T=75 CO2=5500 CO=220 P=94000 H=6` | Tüm değerleri gir → anında alarm tetiklenebilir |
| `SKIP` | Adımı tampon güncellemeden atla |
| `AUTO` | Kalan adımları otomatik senaryo ile çalıştır |

### Adım adım iç işleyiş

```
sim_native2.ps1 başlar
│
├─ [1] -Mode ve -StepDelay parametreleri okunur
├─ [2] Zamanlama sabitleri moda göre ayarlanır
├─ [3] Başlık + zamanlama tablosu yazdırılır
│
├─ [4] Dairesel tampon FAZ-1 baseline ile ÖNCEDEN doldurulur:
│       T=23°C, CO2=420ppm, CO=6ppm, P=101100Pa, H=63%
│       → Adım 1'de ortalama ≠ anlık değer → NormPos doğru çalışır
│
├─ [5] 20 adımlık döngü:
│       ├─ [MANUAL modda] Kullanıcıdan değer al, T/CO2/CO/P/H parse et
│       ├─ Dairesel tampona ekle
│       ├─ Son 5 ölçüm ortalamasını hesapla
│       ├─ NormPos / NormNeg ile normalize et
│       ├─ FireScore hesapla
│       ├─ Alarm koşulu kontrol et
│       ├─ Renkli tablo + bar grafik yazdır
│       └─ -StepDelay kadar bekle
│
└─ [6] Özet: "Mod: X | Adım Gecikmesi: Yms"
```

---

## 7. Gerçek Karta Flash'lama (Keil µVision)

### Gereksinimler

| Araç | Açıklama |
|---|---|
| **Keil µVision 5** | `C:\Users\victus\AppData\Local\Keil_v5\UV4\UV4.exe` |
| **Nuvoton M031 DFP Pack** | Pack Installer'dan kurulur |
| **Nu-Link Debugger** | Karta bağlı USB debugger |
| **M031FB0AE Kart** | Hedef donanım |

### Adım 1 — Keil'de Projeyi Aç

`ORKO_Slave.uvprojx` dosyasına çift tıkla. Tüm dosyalar gruplu açılır:

```
ORKO_Slave
├── Algorithm   → fire_score.c, fire_score.h
├── Application → orko_config.h, main.c
├── Simulation  → sim_main.c, sim_display.c, fake_sensors.c, sim_uart.c
├── Sensors     → drv_bmp390, drv_hdc2022, drv_mq9, drv_neo7m, drv_scd40
├── Drivers     → hal_adc, hal_i2c, hal_timer, hal_uart
├── LoRa        → lora_e22.c
├── Power       → power_mgmt.c
└── Startup     → startup_M031Series.s
```

### Adım 2 — Firmware Build Ayarları

`Project` → `Options for Target`:

**C/C++ sekmesi:**
- **Define kutusunu BOŞ bırak** (`ORKO_SIM` YOK — bu gerçek firmware)
- Include Paths: `.\app;.\algo;.\drivers;.\sensors;.\lora;.\power`

**Simulation grubundaki dosyaları devre dışı bırak:**  
sim/sim_main.c, sim/sim_display.c, sim/fake_sensors.c, sim/sim_uart.c →  
Her birine sağ tık → `Options for File` → `Include in Build` işaretini kaldır.

**Output sekmesi:** Create HEX File ✓

### Adım 3 — Build Al

`Project` → `Build Target` (F7)

### Adım 4 — Flash'la

Nu-Link bağlıyken: `Flash` → `Download` (F8)  
→ `"Erase Done"`, `"Programming Done"` çıktısı görünmeli.

### Adım 5 — Kart Nasıl Çalışır (main.c akışı)

Flash'lama sonrası kart reset'lenince:

```
main() başlar
│
├─ [1] Sistem saati: 12 MHz internal oscillator
├─ [2] GPIO pinleri: LED, LoRa M0/M1 kontrol pinleri
├─ [3] Çevre birimleri:
│       ├─ UART0: debug çıktısı (115200 baud)
│       ├─ I2C0 : BMP390 basınç/sıcaklık + HDC2022 nem
│       ├─ UART1: NEO-7M GPS (9600 baud)
│       ├─ UART2: LoRa E22 (115200 baud)
│       └─ ADC  : MQ-9 CO sensörü (kanal 1)
│
├─ [4] SCD40 CO2 sensörü I2C ile başlatılır
│
└─ [5] Ana durum makinesi (sonsuz döngü):
│
│   ┌─ STATE_SENTINEL  [her 120 saniyede]
│   │   ├─ MQ9 ADC oku → CO ppm hesapla
│   │   ├─ BMP390'dan hızlı T + P oku
│   │   ├─ Ön-alarm FireScore hesapla
│   │   └─ Eşik aşıldıysa → STATE_FULL_READ
│   │
│   ├─ STATE_FULL_READ  [her 600 saniyede]
│   │   ├─ Tüm sensörler oku: T, CO2, CO, P, H
│   │   ├─ GPS koordinat oku: 39.9254N 32.8597E 850m
│   │   ├─ Dairesel tampona ekle (5 örneklik)
│   │   ├─ Tam FireScore hesapla
│   │   ├─ score>0.60 VE T≥70 VE (CO2≥5000 VEYA CO≥200)?
│   │   │   EVET → STATE_ALARM
│   │   └─ HAYIR → STATE_SENTINEL'e dön
│   │
│   └─ STATE_ALARM
│       ├─ LoRa E22 paketi oluştur:
│       │   { PKT_TYPE_ALARM, Slave=0x0001, GPS, T, CO2, CO, P, H, FireScore }
│       ├─ 868 MHz → Master=0x0000'a gönder
│       ├─ LED yakıp söndür (alarm göstergesi)
│       └─ 7200 saniyelik döngü bitince STATE_SENTINEL'e dön
```

### orko_config.h — Gerçek vs Simülasyon Zamanlaması

```c
#ifdef ORKO_SIM
    #define SENTINEL_INTERVAL_SEC    5U   // Simülasyonda 5 saniye
    #define FULL_READ_INTERVAL_SEC  15U   // Simülasyonda 15 saniye
    #define CYCLE_DURATION_SEC      60U   // Simülasyonda 60 saniye
#else
    #define SENTINEL_INTERVAL_SEC  120U   // Gerçek: 2 dakika
    #define FULL_READ_INTERVAL_SEC 600U   // Gerçek: 10 dakika
    #define CYCLE_DURATION_SEC    7200U   // Gerçek: 2 saat
#endif
```

Keil'de `ORKO_SIM` define'ı olmadan derlenince `#else` bloğu aktif — üretim zamanlaması.

---

## 8. Araç Yolları

```
ARM GCC  : C:\arm-gnu-toolchain-15.2.rel1-mingw-w64-i686-arm-none-eabi\bin\arm-none-eabi-gcc.exe
FVP      : C:\Users\victus\AppData\Local\Keil_v5\ARM\avh-fvp\bin\models\FVP_MPS2_Cortex-M0.exe
Keil UV4 : C:\Users\victus\AppData\Local\Keil_v5\UV4\UV4.exe
Ninja    : C:\ninja-win\ninja.exe
```

---

## 9. Sık Karşılaşılan Sorunlar

**FVP çok yavaş çıktı veriyor**  
Normal — FVP her UART karakterini baud hızında simüle eder. Hızlı test için `sim_native2.ps1 -StepDelay 0` kullan.

**Adım 1'de FireScore her zaman 0 çıkıyor**  
`sim_native2.ps1` baseline tampon ön-doldurma ile çözdü. `sim_native.ps1`'de bu durum ilk 1-2 adımda normal.

**Keil'de "Device not found"**  
Pack Installer → Nuvoton → NuMicro M031 → Install. Yalnızca kod görüntüleme içinse Close ile kapat.

**arm-none-eabi-gcc bulunamıyor**  
```powershell
$env:PATH = "C:\arm-gnu-toolchain-15.2.rel1-mingw-w64-i686-arm-none-eabi\bin;" + $env:PATH
```

**Linker uyarıları (_getpid, _kill)**  
`-nosys.specs` kullanımından kaynaklı, hata değildir. ELF derlenir ve çalışır.

---

## 10. Sonraki Adımlar

- [ ] `app/main.c` state machine tamamlanması  
- [ ] `orko_config.h` `#ifdef ORKO_SIM` zamanlama bloğu eklenmesi  
- [ ] Nuvoton M031 BSP entegrasyonu (https://github.com/OpenNuvoton/M031BSP)  
- [ ] MQ9 sensörü R0 kalibrasyonu (sahada, 3 dk ısınma sonrası)  
- [ ] LoRa kanal frekansı doğrulama (Türkiye 868 MHz → Kanal 17/18)  
- [ ] Master kart yazılımı (GSM modülü, alarm SMS)  
- [ ] Saha testi: 5 örnek tampon dolduğunda FireScore doğrulama
