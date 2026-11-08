# 🌡️ Termostato Smart Touch ESP32 (Multi-Piano & Multi-Abitazione)

Termostato smart avanzato basato su microcontrollore **ESP32**, display TFT a colori **ILI9341 (2.8" / 2.4")** con touchscreen resistivo **XPT2046**, sensore ambientale termo-igrometrico/barometrico **BME280** e integrazione nativa **MQTT / Home Assistant Auto-Discovery**.

Supporta in modo trasparente e bivalente sia la scheda **CYD (ESP32-2432S028R "Cheap Yellow Display" con doppie porte USB-C e Micro-USB)** sia lo shield industriale **AZ-Touch MOD 2.8"**.

---

## 🌟 Nuova Dashboard Display LCD: 8 Spie di Stato a Colori

Nella barra superiore del display LCD (da `X=5` a `X=202`, `Y=27..39`), il termostato mostra una **dashboard grafica a 8 indicatori pillole (23x13 px)** con tecnologia **anti-flicker** (aggiornamento mirato e istantaneo in <50ms solo sul badge che varia stato):

| Spia | Funzione Protetta | Stato Inattivo / Spento | Stato Armato / Abilitato | Stato Allarme / In Corso |
| :---: | :--- | :--- | :--- | :--- |
| **`CRN`** | **Cronotermostato Settimanale** | `Navy scuro` (Modo manuale libero) | 🟩 **Verde Brillante** (`#16a34a`) &mdash; Fasce orarie attive | &mdash; |
| **`GEL`** | **Protezione Antigelo Caldaia** | `Navy scuro` (Disabilitata) | 🟦 **Azzurro Cielo** (`#0284c7`) &mdash; Guardia attiva (&le;5.0 °C) | 🟥 **Rosso Allarme** (`#dc2626`) &mdash; Caldaia forzata ON |
| **`WIN`** | **Rilevamento Finestra Aperta** | `Navy scuro` (Disabilitato) | 🟩 **Verde Acqua** (`#10b981`) &mdash; Monitoraggio locale / MQTT | 🟧 **Ambra/Arancio** (`#d97706`) &mdash; Finestra aperta (Pausa 20m) |
| **`PMP`** | **Antiblocco Pompa & Valvole** | `Navy scuro` (Disabilitato) | 🟦 **Ciano Vivo** (`#0891b2`) &mdash; Ciclo settimanale armato | 🟧 **Arancio Fuoco** (`#ea580c`) &mdash; Test sblocco 60s in corso |
| **`NIT`** | **Modalità Notte & Dimming** | `Navy scuro` (Disabilitata) | 🟪 **Viola** (`#7c3aed`) &mdash; Auto-dimming notturno armato | 🟦 **Blu Notte** (`#4338ca`) &mdash; Fascia notturna (Display spento) |
| **`IST`** | **Isteresi Dinamica (0.1..2.0°C)** | `Navy scuro` (Isteresi fissa) | 🟩 **Teal / Petrolio** (`#0d9488`) &mdash; Isteresi regolabile attiva | &mdash; |
| **`BUZ`** | **Feedback Acustico Buzzer** | `Navy scuro` (Muto / Vol. 0) | 🟨 **Giallo Oro** (`#ca8a04`) &mdash; Suono touch attivo (Vol. 1..3) | &mdash; |
| **`MQT`** | **Broker MQTT / Home Assistant**| `Navy scuro` (Non configurato) | 🟦 **Sky Blue HA** (`#0284c7`) &mdash; Connesso al Broker | 🟥 **Rosso Allarme** (`#dc2626`) &mdash; Configurato ma Disconnesso |

---

## 📱 Interfaccia Web Moderna con Card Richiudibili (Accordion)

La pagina web interna (raggiungibile all'IP locale es. `http://192.168.1.150` da qualsiasi smartphone, tablet o PC senza installare app) è organizzata in sezioni ergonomiche a scomparsa:

1. **Card 1 &mdash; Stato Termostato (Sempre Aperta in Alto)**:
   - Temperatura ambiente, umidità relativa, setpoint desiderato, stato relè caldaia (ON/OFF) e tasti rapidi (+0.1, -0.1, +0.5, -0.5, Eco 18°C, Boost 23°C).
   - Barra intensità segnale Wi-Fi con indicazione qualitativa (Eccellente, Buono, Debole) e misurazione RSSI in dBm.
2. **Card 2 &mdash; Programmazione Oraria (Cronotermostato)**:
   - 8 fasce orarie indipendenti (4 Feriali Lun-Ven, 4 Weekend Sab-Dom) con tasto rapido *Copia Feriali su Weekend*.
   - 🟢 **Tasto "Salva e Attiva Crono"**: memorizza gli orari e mette subito in funzione la programmazione (spia `CRN` verde).
   - ⚪ **Tasto "Salva (Solo Manuale)"**: salva gli orari senza attivare il cronotermostato (lascia il controllo manuale libero).
   - 🟠 **Tasto "Ferma Crono"**: interrompe all'istante la programmazione oraria tornando in manuale con setpoint libero.
3. **Card 3 &mdash; Meteo Barometrico & Benessere Ambientale**:
   - Pressione atmosferica con trend a 3 ore, **Punto di Rugiada** (prevenzione condensa e muffe) e indice termico **Humidex**.
4. **Card 4 &mdash; Funzioni Avanzate & Sicurezza**:
   - Cursore Isteresi Dinamica (0.1..2.0 °C con switch ON/OFF).
   - Switch Antiblocco Pompa con pulsante per avviare il test di sblocco 60s.
   - Switch Modalità Notte con orari di inizio e fine auto-dimming display.
   - Pulsante *Pulisci Schermo* (Child Lock 30s) per pulire il vetro senza premere tasti accidentali.
   - Switch e casella Topic per Sensore Finestra Esterno via MQTT (es. sensore magnetico Zigbee / Shelly).
   - Switch e Topic per **Temperatura Esterna da Home Assistant (MQTT)**: visualizza `T --.- °C` a destra della data sul display LCD.
   - Soglia antigelo personalizzabile da 3.0 a 10.0 °C.
5. **Card 5 &mdash; Consumi & Storico Caldaia**:
   - Ore effettive di accensione della caldaia oggi e totale ultimi 7 giorni con tabella storica giorno per giorno.
6. **Card 6 &mdash; Connessione Wi-Fi**:
   - Scansione reti Wi-Fi in tempo reale e configurazione guidata con protezione e conservazione dei dati in memoria Flash.
7. **Card 7 &mdash; Configurazione MQTT & Multi-Immobile / Multi-Piano**:
   - Selettore rapido a pillole per Abitazione, Piano e Stanza con generazione automatica gerarchica dei Topic.

---

## 🛠️ Confronto Hardware & Pinout Dettagliato

### 1. Scheda CYD (ESP32-2432S028R "Yellow Display")

Il CYD utilizza due bus SPI hardware indipendenti (**HSPI** per il display a 40 MHz e **VSPI** per il touch), garantendo zero conflitti di bus:

| Funzione | Pin ESP32 (CYD) | Note di Collegamento Hardware |
| :--- | :--- | :--- |
| **Display TFT (HSPI)** | `GPIO 13` (MOSI), `GPIO 12` (MISO), `GPIO 14` (SCK), `GPIO 15` (CS), `GPIO 2` (DC) | Display LCD ILI9341 a colori 320x240 |
| **Touchscreen (VSPI)** | `GPIO 32` (MOSI), `GPIO 39` (MISO), `GPIO 25` (CLK), `GPIO 33` (CS), `GPIO 36` (IRQ) | Controller resistivo XPT2046 dedicato |
| **Retroilluminazione LCD**| `GPIO 21` | Pilotaggio PWM attivo HIGH (0 = spento, 255 = 100%) |
| **Buzzer / Audio** | `GPIO 26` | Uscita speaker/DAC audio integrata |
| **LED RGB Onboard** | `GPIO 4` (Rosso=Caldo), `GPIO 16` (Verde), `GPIO 17` (Blu=Freddo) | LED di stato integrato (attivo LOW) |
| **Connettore I2C Sensore**| **Connettore JST CN1 / P3 (1.25mm 4-pin):**<br/>&bull; Pin 1: `GND`<br/>&bull; Pin 2: `3.3V`<br/>&bull; Pin 3: `GPIO 22` (**SDA**)<br/>&bull; Pin 4: `GPIO 27` (**SCL**) | Collegamento per sensore termo-igrometrico/barometrico BME280 |

### 2. Shield Industriale AZ-Touch MOD 2.8"

| Funzione | Pin ESP32 (AZ-Touch) | Note di Collegamento Hardware |
| :--- | :--- | :--- |
| **Display TFT & Touch** | `GPIO 23` (MOSI), `GPIO 19` (MISO), `GPIO 18` (SCK), `GPIO 5` (TFT CS), `GPIO 4` (DC), `GPIO 14` (Touch CS) | Bus SPI condiviso |
| **Retroilluminazione LCD**| `GPIO 15` | Pilotaggio PWM attivo LOW (0 = 100%, 255 = spento) |
| **Relè Locale Caldaia** | `GPIO 26` | Relè a bordo scheda a contatto pulito |
| **Buzzer** | `GPIO 21` | Piezo buzzer integrato su scheda |
| **Morsetti I2C Sensore**| `GPIO 17` (**SDA**), `GPIO 16` (**SCL**), `3.3V`, `GND` | Morsettiera a vite a bordo shield |

---

## 📡 Integrazione Home Assistant (MQTT Auto-Discovery)

All'avvio e ad ogni riconnessione, il termostato pubblica i descrittori di Auto-Discovery: in Home Assistant compare automaticamente il dispositivo completo associato all'Area (Piano e Stanza) di competenza:

| Entità Home Assistant | Tipo | Descrizione |
| :--- | :--- | :--- |
| `climate.termostato_<stanza>` | Termostato | Modalità (*Off*, *Heat*, *Cool*) e Preset (*Schedule*, *Away/Eco*, *Boost*) |
| `switch.termostato_<stanza>_crono` | Switch | Attivazione/disattivazione programmazione settimanale |
| `switch.termostato_<stanza>_antifrost` | Switch | Attivazione sorveglianza antigelo |
| `number.termostato_<stanza>_antifrost_th` | Cursore | Regolazione soglia emergenza antigelo (3.0..10.0 °C) |
| `switch.termostato_<stanza>_openwin` | Switch | Rilevamento caduta termica rapida finestra aperta |
| `switch.termostato_<stanza>_ext_window` | Switch | Abilitazione sensore magnetico finestra wireless MQTT |
| `sensor.termostato_<stanza>_temp_esterna` | Sensore | Temperatura esterna mostrata a schermo (`T --.- °C`) |
| `switch.termostato_<stanza>_hysteresis` | Switch | Abilitazione isteresi dinamica |
| `number.termostato_<stanza>_hysteresis_val` | Cursore | Valore isteresi (0.1..2.0 °C) |
| `switch.termostato_<stanza>_pump_prot` | Switch | Protezione antiblocco settimanale pompa |
| `button.termostato_<stanza>_pump_test` | Pulsante | Avvio immediato test pompa 60 secondi |
| `switch.termostato_<stanza>_night_mode` | Switch | Modalità notte con auto-dimming programmato |
| `button.termostato_<stanza>_clean_screen` | Pulsante | Blocco schermo 30 secondi per pulizia |
| `sensor.termostato_<stanza>_ore_oggi` | Sensore | Ore di lavoro caldaia oggi |
| `sensor.termostato_<stanza>_ore_7gg` | Sensore | Ore di lavoro caldaia ultimi 7 giorni |
| `sensor.termostato_<stanza>_duty_cycle` | Sensore | Percentuale di funzionamento caldaia oggi (%) |
| `switch.termostato_<stanza>_opt_start` | Switch | Anticipo termico intelligente (Optimum Start) |
| `number.termostato_<stanza>_opt_start_speed` | Cursore | Velocità riscaldamento impianto (0.5..5.0 °C/h) |
| `sensor.termostato_<stanza>_effective_temp` | Sensore | Temperatura di regolazione effettiva (°C) |
| `switch.termostato_<stanza>_thermal_learning` | Switch | Auto-apprendimento continuo inerzia termica edificio |
| `sensor.termostato_<stanza>_learned_heat_speed` | Sensore | Velocità di riscaldamento appresa (°C/h) |
| `sensor.termostato_<stanza>_heat_cycles` | Sensore | Numero di cicli di apprendimento completati |
| `button.termostato_<stanza>_start_training` | Pulsante | Avvio sessione di calibrazione rapida (30 min) |
| `button.termostato_<stanza>_stop_training` | Pulsante | Interruzione sessione di calibrazione rapida |
| `sensor.termostato_<stanza>_temperatura` | Sensore | Temperatura ambiente (°C) |
| `sensor.termostato_<stanza>_umidita` | Sensore | Umidità relativa (%) |
| `sensor.termostato_<stanza>_pressione` | Sensore | Pressione barometrica (hPa) |
| `sensor.termostato_<stanza>_rugiada` | Sensore | Punto di rugiada (°C) |
| `sensor.termostato_<stanza>_wifi_signal` | Sensore | Qualità segnale Wi-Fi (%) con RSSI (dBm) |
| `button.termostato_<stanza>_reboot` | Pulsante | Riavvio da remoto del termostato |

---

## 🚀 Compilazione e Flash con PlatformIO

```bash
# Compilazione ed upload su scheda CYD (default):
pio run -e cyd
pio run -e cyd -t upload --upload-port COM5

# Compilazione ed upload su shield AZ-Touch MOD:
pio run -e az_touch
pio run -e az_touch -t upload

# Monitor seriale a 115200 baud:
pio device monitor -b 115200
```

---

## 📖 Manuale Utente Completo in PDF

È disponibile il manuale d'uso ufficiale a colori pronto per la stampa in formato A4:
- File generato: [`Manuale_Istruzioni_Termostato_Smart.pdf`](./Manuale_Istruzioni_Termostato_Smart.pdf)
- Per rigenerare il PDF con Python e ReportLab:
  ```bash
  python generate_manual_pdf.py
  ```

---

## 📄 Licenza

Distribuito sotto licenza MIT. Progetto sviluppato e manutenuto da AutoDomain.
