# Standard Aziendale Telemetria IoT — Autodomain

[![Standard: Autodomain IoT v1.0](https://img.shields.io/badge/Standard-Autodomain%20IoT%20v1.0-blue.svg)]()
[![Platform: Raspberry Pi Hub](https://img.shields.io/badge/Hub-Raspberry%20Pi%203B-red.svg)]()
[![Endpoint: HTTP POST :8088](https://img.shields.io/badge/Endpoint-HTTP%20%3A8088-green.svg)]()

Questo documento definisce lo **standard unico di telemetria e diagnostica sul campo** per tutti i dispositivi dell'ecosistema **Autodomain**:
1. **Termostati Smart ESP32** (CYD 2.8″ TFT & AZ-Touch 2.4″ MOD)
2. **Gateway Ethernet & MQTT ATmega1284P** (Bridge W5500 tra Bus RS485 e Rete IP/MQTT)
3. **Interfacce Tastiera & Relè IO16** (Nodi periferici slave su bus RS485 PJON ThroughSerial)

---

## 🏛️ Architettura di Rete & Comunicazione

> [!IMPORTANT]
> **Topologia di Rete IO16**: Le interfacce IO16 **non hanno connettività IP/WiFi diretta**. Sono slave periferici che comunicano esclusivamente su bus bilanciato RS485 a 38400 baud tramite protocollo **PJON ThroughSerial**.  
> È il **Gateway ATmega1284P** (dotato di controller Ethernet Wiznet W5500) che interroga i nodi, ne aggrega i parametri diagnostici (`remote_nodes[]`) e li trasmette al Server Raspberry Pi (`192.168.1.68:8088`) per loro conto.

```
                    ┌─────────────────────────┐
                    │     Server Telemetria   │
                    │   Raspberry Pi (Debian) │
                    │       192.168.1.68      │
                    │   Porta HTTP :8088      │
                    └────────────▲────────────┘
                                 │
           ┌─────────────────────┴─────────────────────┐
           │ HTTP POST (WiFi)                          │ HTTP POST Unificato (Ethernet W5500)
┌──────────┴──────────┐                      ┌─────────┴───────────────┐
│  Termostati Smart   │                      │  Gateway Ethernet MQTT  │
│     ESP32 (WiFi)    │                      │      (ATmega1284P)      │
│   CYD & AZ-Touch    │                      └────────────▲────────────┘
└─────────────────────┘                                   │ Bus RS485 PJON
                                              ┌───────────┴───────────┐
                                              │       │       │       │
                                             IO16    IO16    IO16    IO16
                                             (#1)    (#2)    (#3)    (#4)
```

- **Server di Raccolta**: Raspberry Pi a `http://192.168.1.68:8088/api/telemetry`
- **Dashboard Web Unificata**: [http://192.168.1.68:8088](http://192.168.1.68:8088)
- **Database**: SQLite WAL mode permanente (`telemetry.db`).
- **Politica di Conservazione**: **Nessun dispositivo viene mai cancellato automaticamente**. Anche se un dispositivo è spento o offline per mesi, rimane censito con lo storico e lo stato `Offline`. Ogni riga dispone di un pulsante dedicato **Elimina** per la rimozione manuale singola.

---

## 📦 1. Specifiche Payload JSON

Tutti i dispositivi inviano pacchetti JSON con metodo **`POST`** all'endpoint:
`http://192.168.1.68:8088/api/telemetry`

Header HTTP richiesti:
```http
POST /api/telemetry HTTP/1.1
Host: 192.168.1.68:8088
Content-Type: application/json
Connection: close
Content-Length: <lunghezza_payload>
```

---

### A. Termostati Smart ESP32 (`thermostat`)

Inviato direttamente dal termostato via WiFi ogni **15 minuti** (e dopo 5 secondi dall'aggancio WiFi):

```json
{
  "device_type": "thermostat",
  "mac": "24:6F:28:B5:0C:98",
  "model": "az_touch",
  "version": "2.5.0",
  "temp": 21.4,
  "target_temp": 22.0,
  "humidity": 55.0,
  "heating": 1,
  "mode": 1,
  "rssi": -62,
  "free_heap": 182400,
  "uptime_s": 84200
}
```

---

### B. Gateway MQTT + Nodi IO16 in Batch (Report Unificato)

Dato che l'ATmega1284P ha risorse SRAM limitate (16 KB) e 8 socket Ethernet sul W5500, **il Gateway trasmette il proprio stato e la lista di tutti i nodi IO16 attivi in un unico HTTP POST compatto**.

```json
{
  "device_type": "gateway",
  "mac": "00:08:DC:12:34:56",
  "model": "ATmega1284P MQTT Gateway",
  "version": "2.1-beta",
  "ip": "192.168.1.150",
  "mqtt_connected": true,
  "mqtt_broker": "192.168.1.68:1883",
  "active_nodes": 3,
  "throughput": "4.2 pkt/s",
  "free_ram": 11420,
  "uptime_s": 128400,
  "nodes": [
    {
      "node_id": 1,
      "model": "IO16 Keyboard vA",
      "version": "2.1-beta",
      "bus": 0,
      "inputs": 0,
      "outputs": 3,
      "temp_lm75": 21.8,
      "backlight": 80
    },
    {
      "node_id": 2,
      "model": "IO16 Keyboard vA",
      "version": "2.1-beta",
      "bus": 0,
      "inputs": 1,
      "outputs": 0,
      "temp_lm75": 22.4,
      "backlight": 100
    },
    {
      "node_id": 5,
      "model": "IO16 Keyboard vB",
      "version": "2.1-beta",
      "bus": 1,
      "inputs": 0,
      "outputs": 0,
      "temp_lm75": 20.9,
      "backlight": 70
    }
  ]
}
```

#### Effetto sul Server:
1. **Aggiornamento Gateway**: aggiorna la riga del Gateway nel riquadro *"Gateway Ethernet & MQTT"*.
2. **Aggiornamento Nodi IO16**: il server scansiona l'array `nodes` e registra o aggiorna ciascun nodo nel riquadro *"Interfacce Tastiera & Relè IO16"*, associandovi automaticamente il `gateway_id` del Gateway master!

---

### C. Singolo Report Nodo IO16 (Proxy Alternativo)
Se il Gateway preferisce inviare nodi singolarmente su evento:
```json
{
  "device_type": "io16",
  "node_id": 4,
  "gateway_id": "00:08:DC:12:34:56",
  "model": "IO16 Keyboard vA",
  "version": "2.1-beta",
  "bus": 0,
  "inputs": 5,
  "outputs": 1,
  "temp_lm75": 23.2,
  "backlight": 80,
  "uptime_s": 95000
}
```

---

## 💻 2. Implementazione C++ nel Gateway (`io16_mqtt_gateway_dhcp_v2.ino`)

Nel Gateway ATmega1284P, basta aggiungere la routine che cicla l'array diagnostico già esistente `remote_nodes[]`:

```cpp
#include <Ethernet.h>
#include <ArduinoJson.h>
#include "node_diagnostics.h"

extern RemoteNodeInfo remote_nodes[MAX_DIAG_NODES + 1];

void send_gateway_unified_telemetry() {
  EthernetClient client;
  if (!client.connect("192.168.1.68", 8088)) {
    return;
  }

  // Dimensionamento buffer per Gateway + fino a 16 nodi
  StaticJsonDocument<768> doc;
  doc["device_type"] = "gateway";
  doc["mac"] = "00:08:DC:12:34:56"; // MAC W5500
  doc["model"] = "ATmega1284P MQTT Gateway";
  doc["version"] = "2.1-beta";
  doc["mqtt_connected"] = mqttClient.connected();
  doc["mqtt_broker"] = "192.168.1.68:1883";
  doc["free_ram"] = freeRam();
  doc["uptime_s"] = millis() / 1000UL;

  JsonArray nodes = doc.createNestedArray("nodes");
  uint8_t count = 0;
  for (uint8_t i = 1; i <= MAX_DIAG_NODES; i++) {
    if (remote_nodes[i].active) {
      JsonObject n = nodes.createNestedObject();
      n["node_id"] = i;
      n["bus"] = remote_nodes[i].bus_id ? remote_nodes[i].bus_id : 1;
      n["temp_lm75"] = remote_nodes[i].temperature;
      n["backlight"] = remote_nodes[i].backlight_pct;
      count++;
    }
  }
  doc["active_nodes"] = count;

  String payload;
  serializeJson(doc, payload);

  client.print("POST /api/telemetry HTTP/1.1\r\n");
  client.print("Host: 192.168.1.68:8088\r\n");
  client.print("Content-Type: application/json\r\n");
  client.print("Connection: close\r\n");
  client.printf("Content-Length: %u\r\n\r\n", payload.length());
  client.print(payload);
  client.stop();
}
```

---

## 📊 3. Struttura della Dashboard Web (`:8088`)

La dashboard sul Raspberry Pi ([http://192.168.1.68:8088](http://192.168.1.68:8088)) organizza automaticamente i dispositivi in 3 riquadri dedicati:
1. **KPI Header**: Totale Flotta, Termostati Online, Nodi IO16 Attivi, Gateway Connessi.
2. **Riquadro Termostati Smart ESP32**: Temperature, umidità, stato caldaia, RSSI WiFi, storico e tasto Elimina.
3. **Riquadro Interfacce IO16**: ID Nodo, Gateway di riferimento, Bus RS485, visualizzatore grafico a 8 bit di pulsanti e LED, temperatura LM75, backlight PWM e tasto Elimina.
4. **Riquadro Gateway MQTT ATmega1284P**: MAC/IP W5500, broker MQTT, nodi periferici gestiti, throughput e RAM libera.
