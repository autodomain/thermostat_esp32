#include <WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

char telemetry_host[64] = "192.168.1.68";
int  telemetry_port     = 8088;
unsigned long last_telemetry_ms = 0;
const unsigned long TELEMETRY_INTERVAL_MS = 15UL * 60UL * 1000UL; // 15 minuti

void send_telemetry_now() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.printf("[TELEMETRY] WiFi non connesso (stato=%d), invio annullato\n", (int)WiFi.status());
    return;
  }

  WiFiClient client;
  client.setTimeout(3); // 3 secondi timeout

  if (!client.connect(telemetry_host, telemetry_port)) {
    Serial.println("[TELEMETRY] Connessione al server fallita");
    return;
  }

  StaticJsonDocument<384> doc;
  doc["mac"] = WiFi.macAddress();
  doc["model"] = HARDWARE_MODEL_CODE;
  doc["version"] = FIRMWARE_VERSION;

  float t = get_effective_temperature();
  if (isnan(t)) {
    t = averageValueT.average();
  }
  doc["temp"] = isnan(t) ? 0.0 : round(t * 10.0f) / 10.0f;

  doc["target_temp"] = round(temp_setpoint * 10.0f) / 10.0f;

  float h = averageValueH.average();
  doc["humidity"] = isnan(h) ? 0.0 : round(h * 10.0f) / 10.0f;

  doc["heating"] = (stato == 1);
  doc["mode"] = crono_enabled ? 2 : (stato ? 1 : 0);
  doc["rssi"] = WiFi.RSSI();
  doc["free_heap"] = ESP.getFreeHeap();
  doc["uptime_s"] = millis() / 1000UL;

  String payload;
  serializeJson(doc, payload);

  client.print("POST /api/telemetry HTTP/1.1\r\n");
  client.printf("Host: %s:%d\r\n", telemetry_host, telemetry_port);
  client.print("Content-Type: application/json\r\n");
  client.print("Connection: close\r\n");
  client.printf("Content-Length: %u\r\n\r\n", payload.length());
  client.print(payload);

  // Lettura breve dell'header di risposta HTTP
  unsigned long timeout = millis();
  while (client.connected() && millis() - timeout < 2000UL) {
    if (client.available()) {
      String line = client.readStringUntil('\n');
      if (line.startsWith("HTTP/1.1 200") || line.startsWith("HTTP/1.0 200")) {
        Serial.printf("[TELEMETRY] Inviata con successo a %s:%d\n", telemetry_host, telemetry_port);
      }
      break;
    }
  }
  client.stop();
}

void handle_telemetry() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }
  unsigned long now = millis();
  // Primo invio dopo 25 secondi dall'avvio
  if (last_telemetry_ms == 0) {
    if (now >= 25000UL) {
      last_telemetry_ms = now;
      send_telemetry_now();
    }
  } else if (now - last_telemetry_ms >= TELEMETRY_INTERVAL_MS) {
    last_telemetry_ms = now;
    send_telemetry_now();
  }
}
