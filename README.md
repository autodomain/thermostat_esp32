# Termostato Smart Touch ESP32 (Multi-Piano & Multi-Abitazione)

Termostato smart avanzato basato su **ESP32**, display TFT a colori **ILI9341 (2.8" / 2.4")** con touchscreen resistivo **XPT2046**, sensore termo-igrometrico/barometrico **BME280** e integrazione nativa **MQTT / Home Assistant Auto-Discovery**.

---

## 🌟 Caratteristiche Principali

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
  - Portale Web integrato responsive con grafici, selettori rapidi a pillole e anteprima in tempo reale.
  - Scanner I2C automatico su bus ESP32 per rilevamento e risoluzione conflitti sensore BME280.
- **Display Locale Touch (GUIslice)**:
  - Visualizzazione temperatura ambiente, setpoint, meteo, trend barometrico e grafico storico.
  - Feedback sonoro buzzer configurabile (Muto, Basso/Tick, Medio, Alto).

---

## 🛠️ Hardware & Pinout

| Funzione | Pin ESP32 | Descrizione |
| :--- | :--- | :--- |
| **TFT MOSI** | `GPIO 23` | Linea dati SPI Display |
| **TFT MISO** | `GPIO 19` | Linea dati SPI Touch / Display |
| **TFT SCLK** | `GPIO 18` | Clock SPI bus |
| **TFT CS** | `GPIO 5` | Chip Select Display ILI9341 |
| **TFT DC** | `GPIO 4` | Data / Command Display |
| **TFT RST** | `GPIO 22` | Reset Display |
| **TOUCH CS** | `GPIO 14` | Chip Select Touchscreen XPT2046 |
| **I2C SDA** | `GPIO 21` (default) | Linea dati sensore BME280 (configurabile) |
| **I2C SCL** | `GPIO 22` (default) | Linea clock sensore BME280 (configurabile) |
| **Relè Locale** | `GPIO 26` | Comando Caldaia / Elettrovalvola |
| **Buzzer** | `GPIO 25` | Segnalatore acustico feedback touch |

---

## 🚀 Compilazione e Caricamento

Il progetto è strutturato per **PlatformIO**:

```bash
# Compilazione firmware
pio run

# Caricamento su ESP32 via porta USB
pio run -t upload

# Monitor seriale
pio device monitor -b 115200
```

---

## 📡 Topic MQTT Supportati

Se il topic base configurato è ad esempio `casa/clima/piano_terra/soggiorno/`:

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
| `.../reboot/set` | Comando | `PRESS` (riavvia ESP32) |
| `.../status` | LWT | `online`, `offline` |

---

## 📄 Licenza

Progetto rilasciato sotto licenza MIT. Sviluppato e manutenuto da AutoDomain.
