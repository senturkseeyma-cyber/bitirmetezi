# ORKO — Orman Yangını Erken Tespit ve Koruma Sistemi
## Proje Geliştirme Notu

**Proje:** ORKO (Orman Yangını Erken Tespit ve Koruma Sistemi)  
**Kart:** Slave Node  
**MCU:** Nuvoton M031FB0AE (ARM Cortex-M0, 48MHz HIRC)  
**IDE:** Keil MDK (µVision)  
**Tarih:** Mayıs 2026  
**Kaynak Klasör:** `C:\Users\victus\Desktop\calisma\`

---

## 1. Proje Genel Açıklaması

ORKO, ormanlık alanlarda ağaçlara yerleştirilen sensör düğümlerinden oluşan IoT tabanlı bir erken yangın tespit sistemidir. Sistem **master/slave** mimarisiyle çalışır:

- **Slave Node:** Sahadaki sensör kartı. Çevresel verileri toplar, Fire Score hesaplar, LoRa ile master'a iletir.
- **Master Node:** Slave'lerden gelen verileri toplar, GSM ile yetkililere bildirim gönderir.

Haberleşme: **LoRa P2P (868 MHz)** — ağ geçidi gerektirmez, ~3 km menzil.  
Enerji: **Güneş paneli + Li-ion pil + CN3791 MPPT şarj devresi**.

---

## 2. Donanım Bileşenleri (Slave Kart)

| Bileşen | Tip | Arayüz | Görev |
|---|---|---|---|
| **M031FB0AE** | MCU | — | Nuvoton ARM Cortex-M0, 48MHz, ana işlemci |
| **HDC2022** | Sıcaklık + Nem | I2C (0x40) | Ortam sıcaklığı (°C) ve bağıl nem (%) |
| **BMP390** | Basınç | I2C (0x76) | Atmosferik basınç (Pa) ve yedek sıcaklık |
| **SCD40** | CO₂ | I2C (0x62) | CO₂ yoğunluğu (ppm) — **nöbetçi sensör** |
| **MQ9** | CO / Yanıcı Gaz | ADC (PB.14/CH14) | CO konsantrasyonu (ppm) |
| **NEO-7M** | GPS | UART0 (PF.2/PF.3) | Enlem, boylam, yükseklik (NMEA) |
| **E22-900T22D** | LoRa 868MHz | UART1 (PA.2/PA.3) | Master ile P2P haberleşme |
| **MT3608** | Boost Regülatör | — | Gerilim yükseltici |
| **LM2596** | Buck Regülatör | — | Gerilim düşürücü |

---

## 3. MCU Pin Atamaları (Şematikten Okundu)

> Kaynak: M031FB0AE şematik görüntüsü (IC4 bloğu)

| MCU Pini | Sinyal Adı | Yön | Bağlı Bileşen |
|---|---|---|---|
| **PB.14** (pin 4) | ADC0_CH14 | Giriş (Analog) | MQ9 analog çıkış |
| **PB.13** (pin 5) | DigitalOut | Çıkış | LED / Durum göstergesi |
| **PB.12** (pin 6) | MCU GPIO output_M0 | Çıkış | LoRa E22 — M0 pini |
| **PB.5** (pin 8) | I2C0_SCL | Çift yönlü | I2C sensörler (SCL) |
| **PB.4** (pin 9) | I2C0_SDA | Çift yönlü | I2C sensörler (SDA) |
| **PB.3** (pin 10) | HDC2022_INT | Giriş | HDC2022 kesme pini |
| **PF.3** (pin 12) | UART0_TXD | Çıkış | NEO-7M GPS — RX |
| **PF.2** (pin 13) | UART0_RXD | Giriş | NEO-7M GPS — TX |
| **PA.3** (pin 14) | UART1_TXD | Çıkış | LoRa E22 — UART TX |
| **PA.2** (pin 15) | UART1_RXD | Giriş | LoRa E22 — UART RX |
| **PA.1** (pin 16) | LoRa AUX | Giriş | LoRa E22 — AUX (hazır sinyali) |
| **PA.0** (pin 17) | — | — | (rezerve) |
| **PF.6 / NRESET** (pin 18) | MCU GPIO output_M1 | Çıkış | LoRa E22 — M1 pini |
| **PF.0** (pin 19) | ICE_DAT | — | Debug (SWD Data) |
| **PF.1** (pin 20) | ICE_CLK | — | Debug (SWD Clock) |

### ⚠️ Önemli Not — M1 = PF.6 (NRESET)
PF.6 normalde NRESET fonksiyonuna atanmıştır. GPIO olarak kullanmak için `SYS_UnlockReg()` ile kilit açılmalı ve MFP register'ına GPIO modu yazılmalıdır. Bu işlem `lora_e22.c` içindeki `LoRa_Init()` fonksiyonunda yapılmaktadır.

---

## 4. Yazılım Dosya Yapısı

```
C:\Users\victus\Desktop\calisma\
│
├── app\
│   ├── orko_config.h       ← Merkezi konfigürasyon (TÜM pinler, eşikler, sabitler)
│   └── main.c              ← Ana uygulama, durum makinesi (state machine)
│
├── drivers\                ← Donanım Soyutlama Katmanı (HAL)
│   ├── hal_i2c.c / .h      ← I2C0 okuma/yazma (BMP390, SCD40, HDC2022)
│   ├── hal_uart.c / .h     ← UART0 (GPS) + UART1 (LoRa)
│   ├── hal_adc.c / .h      ← ADC CH14 (MQ9 analog)
│   └── hal_timer.c / .h    ← SysTick 1ms + Timer0 1sn sayaç
│
├── sensors\                ← Sensör Sürücüleri
│   ├── drv_hdc2022.c / .h  ← HDC2022 sıcaklık + nem
│   ├── drv_bmp390.c / .h   ← BMP390 basınç (OTP kalibrasyon dahil)
│   ├── drv_scd40.c / .h    ← SCD40 CO₂ (CRC-8 doğrulama, tek atış mod)
│   ├── drv_mq9.c / .h      ← MQ9 CO ppm (Rs/R0 logaritmik dönüşüm)
│   └── drv_neo7m.c / .h    ← NEO-7M GPS ($GPGGA NMEA parse)
│
├── lora\
│   ├── lora_e22.c / .h     ← E22-900T22D konfigürasyon + paket gönderme
│
├── algo\
│   ├── fire_score.c / .h   ← Fire Score algoritması (tez Bölüm 3.3)
│
├── power\
│   ├── power_mgmt.c / .h   ← Güç yönetimi, uyku modları
│
├── ORKO_PROJE_NOTU.md      ← Bu dosya
├── BitirmeTez.docx         ← Proje analiz dokümanı
└── tez_content.txt         ← Tezin metin çıktısı
```

---

## 5. Sistem Akış Diyagramı

```
[BAŞLAT]
    │
    ▼
[SYS_Init: 48MHz HIRC, Timer, I2C0, UART0, UART1, ADC]
    │
    ▼
[Sensör Init: BMP390, SCD40, HDC2022, MQ9, GPS]
    │
    ▼
[LoRa Init: E22 → adres 0x0001, kanal 0, 9600 baud, EEPROM'a yaz]
    │
    ▼
┌───────────────────────────────────────────────────────┐
│           NÖBETÇİ DÖNGÜSÜ (her 2 dakika)              │
│  SCD40 tek atış ölçüm                                 │
│  CO₂ ≥ 5000 ppm?                                      │
│  HAYIR → Power-Down Sleep → 2 dk → tekrar             │
│  EVET  ↓                                              │
│  ┌──────────────────────────────────────────────────┐ │
│  │     TAM ÖLÇÜM DÖNGÜSÜ (her 10 dk, 2 saat)       │ │
│  │  HDC2022 + BMP390 + SCD40 + MQ9 + GPS oku       │ │
│  │  FireScore hesapla                               │ │
│  │  Alarm? (FireScore>0.6 + Sıcaklık + CO₂/CO krit)│ │
│  │  EVET → LoRa ALARM paketi gönder                │ │
│  │  HAYIR → 10 dk sleep                            │ │
│  │  2 saat doldu? → DURUM paketi + tampon temizle  │ │
│  └──────────────────────────────────────────────────┘ │
└───────────────────────────────────────────────────────┘
```

---

## 6. Fire Score Algoritması (Tez Bölüm 3.3)

### 6.1 Ortalama Hesabı (Son 5 Ölçüm)
```
NAVG = (Ns-1 + Ns-2 + Ns-3 + Ns-4 + Ns-5) / 5
```

### 6.2 Normalizasyon

**Yangınla doğru orantılı** (Sıcaklık, CO₂, CO):
```
Nx = (Ncurrent - NAVG) / (Ncritical - NAVG)    → [0, 1] sıkıştırılır
```

**Yangınla ters orantılı** (Basınç, Nem):
```
Nx = (NAVG - Ncurrent) / (NAVG - Ncritical)    → [0, 1] sıkıştırılır
```

### 6.3 Fire Score Formülü
```
FireScore = 0.25·NT + 0.25·NCO2 + 0.25·NCO + 0.15·NP + 0.10·NH
```

### 6.4 Alarm Karar Koşulu
```
[FireScore > 0.6] VE [Sıcaklık ≥ 70°C] VE [CO₂ ≥ 5000ppm VEYA CO ≥ 200ppm]
```

### 6.5 Sensör Eşik Değerleri

| Parametre | Normal Aralık | Kritik Eşik | Ortalama |
|---|---|---|---|
| Sıcaklık | -10 ~ 60 °C | **70 °C** | 25 °C |
| CO₂ | 400 ~ 1000 ppm | **5000 ppm** | 450 ppm |
| CO | 0 ~ 50 ppm | **200 ppm** | 10 ppm |
| Basınç | 95000 ~ 102000 Pa | **< 95000 Pa** | 101000 Pa |
| Nem | 20 ~ 80 % | **< 15 %** | 55 % |

---

## 7. LoRa E22-900T22D Konfigürasyonu

| Parametre | Değer |
|---|---|
| Çalışma Frekansı | 868 MHz (Türkiye bandı) |
| Kanal | 0 → 850.125 MHz |
| UART Baud | 9600 bps |
| Hava Veri Hızı | 2.4 kbps |
| TX Gücü | +22 dBm (maksimum) |
| Modülasyon | LoRa (SX1262) |
| Haberleşme | P2P (peer-to-peer, ağ geçidi yok) |
| Slave Adresi | 0x0001 |
| Master Adresi | 0x0000 |
| M0 Pini | PB.12 |
| M1 Pini | PF.6 (NRESET/GPIO) |
| AUX Pini | PA.1 |

### Mod Tablosu
| M0 | M1 | Mod |
|---|---|---|
| 0 | 0 | Normal (şeffaf iletim) |
| 1 | 0 | Konfigürasyon (AT komutları) |
| 0 | 1 | WOR (Wake On Radio) |
| 1 | 1 | Deep Sleep |

### TX Paket Yapısı (`ORKO_Packet_t`)
| Alan | Tip | Açıklama |
|---|---|---|
| slaveAddr | uint16_t | Bu slave'in adresi (0x0001) |
| masterAddr | uint16_t | Hedef master (0x0000) |
| packetType | uint8_t | 0x01=Normal, 0x02=Alarm, 0x03=Durum |
| temperature_C | float | HDC2022 sıcaklık |
| humidity_pct | float | HDC2022 nem |
| pressure_Pa | float | BMP390 basınç |
| co2_ppm | uint16_t | SCD40 CO₂ |
| co_ppm | float | MQ9 CO |
| latitude | float | GPS enlem |
| longitude | float | GPS boylam |
| altitude_m | float | GPS yükseklik |
| gps_valid | uint8_t | GPS fix geçerli mi |
| fire_score | float | 0.0–1.0 risk skoru |
| alarm_flag | uint8_t | 1: Yangın alarmı |
| uptime_sec | uint32_t | Çalışma süresi |

---

## 8. Güç Yönetimi

| Durum | MCU Modu | Tahmini Akım | Tetikleyici |
|---|---|---|---|
| Normal ölçüm | Aktif | ~10 mA | Her 10 dakika |
| Nöbetçi uyku | Power-Down | ~10 µA | CO₂ normal iken |
| LoRa gönderim | Aktif + RF | ~120 mA | Alarm veya 2 saat periyodu |
| Deep Sleep | Deep Power-Down | ~2 µA | İleride eklenecek |

**Enerji Kaynağı:** Güneş paneli → CN3791 (MPPT şarj) → Li-ion pil → BMS → LM2596 + MT3608

---

## 9. Sensör Sürücüsü Notları

### HDC2022
- I2C adres: `0x40` (A0=A1=GND)
- Dönüşüm: `T = (raw/65536) × 165 - 40`, `RH = (raw/65536) × 100`
- Üretici ID kontrolü: `0x5449` (Texas Instruments)
- Tek ölçüm modu kullanılıyor (continuous mod değil — güç tasarrufu)

### BMP390
- I2C adres: `0x76` (SDO=GND)
- Chip ID: `0x60`
- 21 byte OTP kalibrasyon verisi okunur (`0x31` adresinden)
- Forced mod kullanılıyor — her okumada tek ölçüm
- Oversampling: Basınç ×8, Sıcaklık ×1

### SCD40
- I2C adres: `0x62` (sabit)
- Komutlar 16-bit, her 2 byte veriden sonra CRC-8 (`polynomial: 0x31`) var
- **Nöbetçi sensör** olarak kullanılıyor: `CMD_SINGLE_SHOT` → 5 saniye bekle → oku
- Uyku komutu: `0x36F6` (Power Down) — ~0.4 µA

### MQ9
- Analog çıkış → **PB.14 (ADC0_CH14)**
- Rs hesabı: `Rs = ((Vcc - Vadc) / Vadc) × RL`
- CO ppm: `ppm = 26.179 × (Rs/R0)^(-1.179)`
- R0: `10 kΩ` (varsayılan — **sahada temiz havada kalibre edilmeli**)
- Isınma süresi: **3 dakika** (güç verilince beklenecek)

### NEO-7M
- UART0: 9600 baud, PF.2 RX / PF.3 TX
- `$GPGGA` cümlesi parse ediliyor
- Alan 6 (Fix Status) `0` ise veri geçersiz sayılıyor
- `GPS_Read()` çağrısında 30 satır deneme yapılıyor

---

## 10. Keil MDK Proje Yapılandırması (Yapılacak)

> Bu adımlar henüz tamamlanmadı, sonraki oturumda gerçekleştirilecek.

### Adım 1 — Nuvoton BSP Kurulumu
- Nuvoton'un resmi BSP paketini indir: **M031 Series BSP**
- `StdDriver` klasörü include path'e eklenmeli
- `NuMicro.h` başlık dosyası tüm HAL sürücüleri tarafından kullanılıyor

### Adım 2 — Keil Proje Oluşturma
1. `File → New Project` → `C:\Users\victus\Desktop\calisma\` seç
2. Device: **Nuvoton M031FB0AE**
3. CMSIS Core + Startup dosyaları ekle

### Adım 3 — Project Groups (Gruplar)
Aşağıdaki grupları ve `.c` dosyalarını ekle:

| Grup Adı | Eklenecek .c Dosyaları |
|---|---|
| `App` | `app\main.c` |
| `Drivers` | `drivers\hal_i2c.c`, `hal_uart.c`, `hal_adc.c`, `hal_timer.c` |
| `Sensors` | `sensors\drv_hdc2022.c`, `drv_bmp390.c`, `drv_scd40.c`, `drv_mq9.c`, `drv_neo7m.c` |
| `LoRa` | `lora\lora_e22.c` |
| `Algo` | `algo\fire_score.c` |
| `Power` | `power\power_mgmt.c` |

### Adım 4 — Include Paths
`Options for Target → C/C++ → Include Paths` kısmına ekle:
```
C:\Users\victus\Desktop\calisma\app
C:\Users\victus\Desktop\calisma\drivers
C:\Users\victus\Desktop\calisma\sensors
C:\Users\victus\Desktop\calisma\lora
C:\Users\victus\Desktop\calisma\algo
C:\Users\victus\Desktop\calisma\power
[BSP StdDriver include klasörü]
[BSP Device include klasörü]
```

### Adım 5 — Derleyici Ayarları
- Optimization: **-O1** (debug için önce -O0)
- C Standard: **C99**
- Define: `__NUVOTON__` veya BSP'nin gerektirdiği makrolar

### Adım 6 — Linker / Flash Ayarları
- Flash: 64KB (M031FB0AE)
- RAM: 8KB
- Debugger: **Nu-Link** (Nuvoton debug adaptörü)

---

## 11. Bilinen Dikkat Noktaları

1. **M1 = PF.6 (NRESET):** Bu pin LoRa M1 olarak kullanılıyor. GPIO moduna almak için `SYS_UnlockReg()` zorunlu. `LoRa_Init()` içinde yapılıyor.

2. **MQ9 Kalibrasyon:** R0 şu an `10 kΩ` varsayılan. Sahada sensör 3 dakika ısındıktan sonra temiz havada `Rs` ölçülüp `R0` olarak `orko_config.h`'a yazılmalı.

3. **SCD40 Başlangıç:** `SCD40_Init()` çağrısında önce `STOP_PERIODIC` komutu gönderiliyor — önceki çalışmadan kalan durumu temizler.

4. **BMP390 Kalibrasyon:** Chip'in OTP belleğinden 21 byte okunuyor. I2C okuma başarısız olursa sensör bağlantısını kontrol et.

5. **GPS Fix Bekleme:** İlk açılışta NEO-7M fix almak için 30–60 saniye gerekebilir. `GPS_Read()` 30 satır deniyor; fix yoksa `gps_valid = 0` ile paket gönderilir.

6. **Power-Down Mod:** M031'in `CLK_PowerDown()` fonksiyonu tüm çevresel saatleri durdurur. Timer0 kesmesi uyandırmak için açık bırakılmıştır.

7. **LoRa Kanal:** Şu an Kanal 0 → 850.125 MHz. Türkiye'de 868 MHz bandı için Kanal 17 (867.125 MHz) veya Kanal 18 (868.125 MHz) daha uygun olabilir. `orko_config.h`'da `LORA_CHANNEL` değiştirilebilir.

---

## 12. Sonraki Oturumda Yapılacaklar

- [ ] Keil MDK'da proje oluşturma ve BSP entegrasyonu
- [ ] Nuvoton M031 BSP StdDriver kütüphanesi bağlantısı
- [ ] İlk derleme ve hata düzeltmeleri
- [ ] Debug/simülasyon testi
- [ ] Master kart yazılımı (GSM modülü dahil)
- [ ] MQ9 saha kalibrasyonu prosedürü
- [ ] LoRa kanal frekans doğrulaması (868 MHz Türkiye bandı)
