# Termostato Smart Touch ESP32 (Multi-Piano & Multi-Abitazione)

Termostato smart avanzato basato su **ESP32**, display TFT a colori **ILI9341 (2.8" / 2.4")** con touchscreen resistivo **XPT2046**, sensore termo-igrometrico/barometrico **BME280** e integrazione nativa **MQTT / Home Assistant Auto-Discovery**.

Supporta nativamente sia la scheda **CYD (ESP32-2432S028R "Cheap Yellow Display" con porte USB-C e Micro-USB)** sia lo shield industriale **AZ-Touch MOD**.

---

## 🌟 Caratteristiche Principali

- **Supporto Hardware Dual-Target (CYD & AZ-Touch)**:
  - **CYD (ESP32-2432S028R)**: Utilizza due bus SPI hardware indipendenti (**HSPI** per il display ILI9341 a 40 MHz e **VSPI** per il controller touch XPT2046), connettore di espansione **P3** per sensore I2C / Relè esterno, retroilluminazione PWM su GPIO 21 e LED RGB di stato (Rosso=Riscaldamento, Blu=Raffrescamento, Spento=Standby).
  - **AZ-Touch MOD**: Configurazione classica con display e touch sullo stesso bus SPI, retroilluminazione attiva LOW su GPIO 15, piezo buzzer su GPIO 21 e relè su GPIO 26.
- **Struttura Flessibile Multi-Immobile & Multi-Zona**:
  - Supporto per **più abitazioni/edifici** (es. *Casa Principale*, *Villa*, *Appartamento 1*, *Appartamento 2*, *Ufficio*, *B&B*).
  - Supporto per **qualsiasi numero di piani** (*Seminterrato*, *Piano Terra*, *Primo Piano*, *Mansarda*, ecc.).
  - Supporto per **più termostati sullo stesso piano** (es. *Soggiorno*, *Cucina*, *Camera Padronale*, *Zona Giorno*, *Zona Notte*).
  - Generazione automatica e gerarchica dei Topic MQTT (es. `casa/clima/piano_terra/soggiorno/`) con possibilità di personalizzazione libera.
- **Integrazione Home Assistant Auto-Discovery Completa**:
  - Entità `climate` principale con modalità *Off*, *Heat*, *Cool* e preset (*Schedule*, *Away/Eco*, *Boost*).
  - **Assegnazione automatica alle Aree di Home Assistant** (basata su Piano e Stanza).
  - Sensore di stato caldaia/relè in tempo reale (`binary_sensor`).
  - Sensori ambientali di precisione: *Temperatura*, *Umidità*, *Pressione Barometrica*, *Punto di Rugiada*, *Previsione Meteo Barometrica & Comfort Termico*.
  - Statistiche consumi: *Ore di accensione caldaia oggi* e *ultimi 7 giorni*.
  - Switch diagnostici e di sicurezza: *Protezione Antigelo*, *Rilevamento Finestra Aperta*, *Buzzer Touch*, *Cronotermostato*.
  - Pulsante di riavvio remoto (`button`) ed entità diagnostiche (`IP`, `Uptime`, `Segnale Wi-Fi`).
- **Interfaccia Web Indipendente**:
  - Portale Web integrato responsive con selettori rapidi a pillole per Casa, Piano e Stanza.
  - Scanner I2C automatico su bus ESP32 con test automatico di 7 coppie di pin GPIO (incluso connettore P3 del CYD) per rilevare e configurare automaticamente il BME280.
- **Display Locale Touch (GUIslice)**:
  - Visualizzazione temperatura ambiente, setpoint, meteo, trend barometrico e grafico storico.
  - Modalità notte con auto-dimming intelligente (risveglio su qualsiasi tocco).
  - Feedback sonoro acustico configurabile (Muto, Basso/Tick, Medio, Alto).

---

## 🛠️ Confronto Hardware & Pinout

### 1. Scheda CYD (ESP32-2432S028R - USB-C & Micro-USB)

Il CYD adotta due bus SPI separati: **HSPI** per il display e **VSPI** per il touch, evitando ogni interferenza sul bus.

| Funzione | Pin ESP32 (CYD) | Note Hardware |
| :--- | :--- | :--- |
| **TFT MOSI** | `GPIO 13` | Bus HSPI Display |
| **TFT MISO** | `GPIO 12` | Bus HSPI Display |
| **TFT SCLK** | `GPIO 14` | Bus HSPI Display |
| **TFT CS** | `GPIO 15` | Chip Select Display ILI9341 |
| **TFT DC** | `GPIO 2` | Data / Command Display |
| **TFT RST** | `-1` | Connesso al Reset generale dell'ESP32 |
| **TFT Backlight**| `GPIO 21` | PWM attivo HIGH (0 = spento, 255 = 100%) |
| **TOUCH CLK** | `GPIO 25` | Bus VSPI Touch XPT2046 dedicato |
| **TOUCH MISO** | `GPIO 39` | Bus VSPI Touch XPT2046 dedicato |
| **TOUCH MOSI** | `GPIO 32` | Bus VSPI Touch XPT2046 dedicato |
| **TOUCH CS** | `GPIO 33` | Chip Select Touch |
| **TOUCH IRQ** | `GPIO 36` | Interrupt tocco Touchscreen |
| **Audio / Buzzer** | `GPIO 26` | DAC / Speaker / Buzzer integrato |
| **LED RGB Rosso** | `GPIO 4` | Onboard RGB LED (attivo LOW, indica Caldo) |
| **LED RGB Verde** | `GPIO 16` | Onboard RGB LED (attivo LOW) |
| **LED RGB Blu** | `GPIO 17` | Onboard RGB LED (attivo LOW, indica Freddo) |
| **Sensore Luce LDR**| `GPIO 34` | Fotoresistenza onboard (ADC1) |
| **Connettore P3** | `GPIO 22` (SDA), `GPIO 27` (SCL) | Porta JST 4-pin per sensore BME280 / Relè |

### 2. Shield AZ-Touch MOD

| Funzione | Pin ESP32 (AZ-Touch) | Note Hardware |
| :--- | :--- | :--- |
| **TFT MOSI / MISO / CLK** | `GPIO 23`, `GPIO 19`, `GPIO 18` | Bus SPI condiviso |
| **TFT CS / DC / RST** | `GPIO 5`, `GPIO 4`, `GPIO 22` | Controllo Display ILI9341 |
| **TOUCH CS** | `GPIO 14` | Chip Select Touch XPT2046 |
| **TFT Backlight** | `GPIO 15` | PWM attivo LOW (0 = 100%, 255 = spento) |
| **Buzzer** | `GPIO 21` | Piezo buzzer integrato |
| **Relè Locale** | `GPIO 26` | Relè a bordo shield |
| **I2C SDA / SCL** | `GPIO 17`, `GPIO 16` | Header I2C per BME280 |

---

## 🚀 Compilazione e Caricamento con PlatformIO

Il file `platformio.ini` include gli ambienti preconfigurati per entrambe le piattaforme:

```bash
# Per compilare e caricare su Scheda CYD (Default):
pio run -e cyd
pio run -e cyd -t upload

# Per compilare e caricare su AZ-Touch MOD:
pio run -e az_touch
pio run -e az_touch -t upload

# Monitor seriale a 115200 baud:
pio device monitor -b 115200
```

---

## 📡 Topic MQTT Supportati

Se il topic base generato è ad esempio `casa/clima/piano_terra/soggiorno/`:

| Topic | Tipo | Valori / Payload |
| :--- | :--- | :--- |
| `.../temperature/get` | Telemetria | Temperatura attuale (°C) |
| `.../humidity/get` | Telemetria | Umidità relativa (%) |
| `.../pressure/get` | Telemetria | Pressione atmosferica (hPa) |
| `.../temperature/target` | Stato | Setpoint impostato (°C) |
| `.../temperature/set` | Comando | Imposta setpoint (es. `20.5`) |
| `.../mode/get` | Stato | `off`, `heat`, `cool` |
| `.../mode/set` | Comando | `off`, `heat`, `cool` |
| `.../preset/get` | Stato | `none`, `schedule`, `away`, `boost` |
| `.../preset/set` | Comando | `none`, `schedule`, `away`, `boost` |
| `.../relay/state` | Stato | `ON`, `OFF` (stato caldaia) |
| `.../reboot/set` | Comando | `PRESS` (riavvia ESP32 da remoto) |
| `.../status` | LWT | `online`, `offline` |

---

## 📄 Licenza

Progetto rilasciato sotto licenza MIT. Sviluppato e manutenuto da AutoDomain.
