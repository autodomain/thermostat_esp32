void update_indicator_spie(bool force_redraw);
void update_outdoor_temp_display(float t);
extern bool ext_temp_enabled;
extern char ext_temp_topic[100];
extern float ext_temp_val;

extern bool opt_start_enabled;
extern float opt_start_speed;
extern bool opt_start_active;
extern bool boost_active;
extern uint16_t boost_duration_min;
extern float boost_temp;
extern float eco_temp;
extern float away_temp;
extern uint8_t remote_sensor_mode;
extern char remote_sensor_topic[100];
extern float remote_temp_val;
extern unsigned long remote_temp_last_ms;

extern bool thermal_learning_enabled;
extern float learned_heat_speed;
extern float learned_cool_speed;
extern uint16_t heat_cycles_count;
extern uint16_t cool_cycles_count;
extern bool training_guided_active;
extern uint16_t training_guided_duration_min;
void start_thermal_training(uint16_t duration_minutes);
void cancel_or_finish_thermal_training(bool completed);

float get_effective_temperature();
void trigger_boost(uint16_t duration_minutes, float target_temp);
void cancel_boost();

float parse_temperature_payload(const String& payload) {
  String p = payload;
  p.trim();
  if (p.startsWith("{")) {
    DynamicJsonDocument doc(256);
    DeserializationError err = deserializeJson(doc, p);
    if (!err) {
      if (doc.containsKey("temperature")) return doc["temperature"].as<float>();
      if (doc.containsKey("temp")) return doc["temp"].as<float>();
      if (doc.containsKey("value")) return doc["value"].as<float>();
      if (doc.containsKey("state")) return doc["state"].as<float>();
    }
  }
  return p.toFloat();
}

unsigned long previousMillis_mqtt = 0;
const unsigned long interval_mqtt_retry = 10000;

const char* get_ha_mode_str() {
  if (stato == 1) return "heat";
  if (stato == 2) return "cool";
  return "off";
}

const char* get_ha_action_str() {
  if (stato == 0) return "off";
  if (digitalRead(rele_pin)) {
    return (stato == 1) ? "heating" : "cooling";
  }
  return "idle";
}

void publish_ha_state() {
  update_indicator_spie(false);
  if (!client.connected()) return;
  char topic[96];
  char buf[16];

  snprintf(topic, sizeof(topic), "%sstatus", thermostat_id);
  client.publish(topic, "online", true, 0);

  snprintf(topic, sizeof(topic), "%smode/get", thermostat_id);
  client.publish(topic, get_ha_mode_str(), true, 0);

  snprintf(topic, sizeof(topic), "%saction/get", thermostat_id);
  client.publish(topic, get_ha_action_str(), true, 0);

  dtostrf(temp_setpoint, 0, 1, buf);
  snprintf(topic, sizeof(topic), "%stemperature/target", thermostat_id);
  client.publish(topic, buf, true, 0);

  snprintf(topic, sizeof(topic), "%spreset/get", thermostat_id);
  client.publish(topic, current_preset_mode.c_str(), true, 0);

  snprintf(topic, sizeof(topic), "%scrono/get", thermostat_id);
  client.publish(topic, crono_enabled ? "ON" : "OFF", true, 0);

  snprintf(topic, sizeof(topic), "%sbuzzer/get", thermostat_id);
  client.publish(topic, buzzer_enabled ? "ON" : "OFF", true, 0);

  snprintf(topic, sizeof(topic), "%scrono/slot", thermostat_id);
  client.publish(topic, current_crono_desc.c_str(), true, 0);

  snprintf(topic, sizeof(topic), "%santifrost/get", thermostat_id);
  client.publish(topic, antifrost_enabled ? "ON" : "OFF", true, 0);

  dtostrf(antifrost_temp_threshold, 0, 1, buf);
  snprintf(topic, sizeof(topic), "%santifrost_th/get", thermostat_id);
  client.publish(topic, buf, true, 0);

  snprintf(topic, sizeof(topic), "%sopenwin/get", thermostat_id);
  client.publish(topic, open_window_enabled ? "ON" : "OFF", true, 0);

  snprintf(topic, sizeof(topic), "%shysteresis/get", thermostat_id);
  client.publish(topic, hysteresis_enabled ? "ON" : "OFF", true, 0);

  dtostrf(hysteresis_val, 0, 1, buf);
  snprintf(topic, sizeof(topic), "%shysteresis_val/get", thermostat_id);
  client.publish(topic, buf, true, 0);

  snprintf(topic, sizeof(topic), "%spump_prot/get", thermostat_id);
  client.publish(topic, pump_protection_enabled ? "ON" : "OFF", true, 0);

  snprintf(topic, sizeof(topic), "%snight_mode/get", thermostat_id);
  client.publish(topic, night_mode_enabled ? "ON" : "OFF", true, 0);

  snprintf(topic, sizeof(topic), "%sclean_screen/state", thermostat_id);
  client.publish(topic, child_lock_active ? "ON" : "OFF", true, 0);

  snprintf(topic, sizeof(topic), "%sext_win_en/get", thermostat_id);
  client.publish(topic, ext_window_enabled ? "ON" : "OFF", true, 0);

  snprintf(topic, sizeof(topic), "%sext_win/state", thermostat_id);
  client.publish(topic, ext_window_open ? "ON" : "OFF", true, 0);

  // Ore accensione oggi (in ore decimali con 2 cifre, es. 3.25 h)
  float h_today = runtime_today_sec / 3600.0f;
  dtostrf(h_today, 0, 2, buf);
  snprintf(topic, sizeof(topic), "%sruntime/today", thermostat_id);
  client.publish(topic, buf, true, 0);

  // Ore accensione ultimi 7 giorni totali
  uint32_t tot_7d_sec = runtime_today_sec;
  for (int i = 0; i < 7; i++) tot_7d_sec += runtime_history_7d[i];
  float h_7d = tot_7d_sec / 3600.0f;
  dtostrf(h_7d, 0, 2, buf);
  snprintf(topic, sizeof(topic), "%sruntime/7days", thermostat_id);
  client.publish(topic, buf, true, 0);

  // Duty cycle accensione oggi in %
  struct tm timeinfo_dc;
  float duty_cycle_today = 0.0f;
  if (getLocalTime(&timeinfo_dc, 10)) {
    uint32_t sec_today = timeinfo_dc.tm_hour * 3600 + timeinfo_dc.tm_min * 60 + timeinfo_dc.tm_sec;
    if (sec_today > 30) {
      duty_cycle_today = constrain((runtime_today_sec * 100.0f) / sec_today, 0.0f, 100.0f);
    }
  }
  dtostrf(duty_cycle_today, 0, 1, buf);
  snprintf(topic, sizeof(topic), "%sruntime/duty_cycle", thermostat_id);
  client.publish(topic, buf, true, 0);

  // Anticipo termico stato
  snprintf(topic, sizeof(topic), "%sopt_start/get", thermostat_id);
  client.publish(topic, opt_start_enabled ? "ON" : "OFF", true, 0);

  dtostrf(opt_start_speed, 0, 1, buf);
  snprintf(topic, sizeof(topic), "%sopt_start_speed/get", thermostat_id);
  client.publish(topic, buf, true, 0);

  // Sensore remoto e temperatura effettiva
  snprintf(topic, sizeof(topic), "%sremote_sensor/mode", thermostat_id);
  snprintf(buf, sizeof(buf), "%u", (unsigned)remote_sensor_mode);
  client.publish(topic, buf, true, 0);

  if (!isnan(remote_temp_val)) {
    dtostrf(remote_temp_val, 0, 1, buf);
    snprintf(topic, sizeof(topic), "%sremote_sensor/val", thermostat_id);
    client.publish(topic, buf, true, 0);
  }

  float eff_t = get_effective_temperature();
  if (!isnan(eff_t)) {
    dtostrf(eff_t, 0, 1, buf);
    snprintf(topic, sizeof(topic), "%stemperature/effective", thermostat_id);
    client.publish(topic, buf, true, 0);
  }

  // Auto-Apprendimento Termico & Calibrazione
  snprintf(topic, sizeof(topic), "%sthermal_learning/get", thermostat_id);
  client.publish(topic, thermal_learning_enabled ? "ON" : "OFF", true, 0);

  dtostrf(learned_heat_speed, 0, 2, buf);
  snprintf(topic, sizeof(topic), "%slearned_heat_speed/get", thermostat_id);
  client.publish(topic, buf, true, 0);

  dtostrf(learned_cool_speed, 0, 2, buf);
  snprintf(topic, sizeof(topic), "%slearned_cool_speed/get", thermostat_id);
  client.publish(topic, buf, true, 0);

  snprintf(buf, sizeof(buf), "%u", (unsigned)heat_cycles_count);
  snprintf(topic, sizeof(topic), "%sheat_cycles/get", thermostat_id);
  client.publish(topic, buf, true, 0);

  snprintf(topic, sizeof(topic), "%straining_active/get", thermostat_id);
  client.publish(topic, training_guided_active ? "ON" : "OFF", true, 0);

  if (WiFi.status() == WL_CONNECTED) {
    int rssi = WiFi.RSSI();
    snprintf(buf, sizeof(buf), "%d", rssi);
    snprintf(topic, sizeof(topic), "%swifi/rssi", thermostat_id);
    client.publish(topic, buf, true, 0);

    int pct = constrain(2 * (rssi + 100), 0, 100);
    snprintf(buf, sizeof(buf), "%d", pct);
    snprintf(topic, sizeof(topic), "%swifi/signal", thermostat_id);
    client.publish(topic, buf, true, 0);
  }

  if (bme_ok) {
    dtostrf(averageValueT.average(), 0, 1, buf);
    snprintf(topic, sizeof(topic), "%stemperature/get", thermostat_id);
    client.publish(topic, buf, true, 0);

    dtostrf(averageValueH.average(), 0, 1, buf);
    snprintf(topic, sizeof(topic), "%shumidity/get", thermostat_id);
    client.publish(topic, buf, true, 0);

    if (last_pressure_hpa > 300.0f) {
      dtostrf(last_pressure_hpa, 0, 1, buf);
      snprintf(topic, sizeof(topic), "%spressure/get", thermostat_id);
      client.publish(topic, buf, true, 0);

      dtostrf(calc_dew_point, 0, 1, buf);
      snprintf(topic, sizeof(topic), "%sdewpoint/get", thermostat_id);
      client.publish(topic, buf, true, 0);

      String comfPayload = comfort_short_str + " (" + weather_short_str + ")";
      snprintf(topic, sizeof(topic), "%scomfort/get", thermostat_id);
      client.publish(topic, comfPayload.c_str(), true, 0);
    }
  }

  // 16. Stato Relè Caldaia (ON/OFF)
  snprintf(topic, sizeof(topic), "%srelay/state", thermostat_id);
  client.publish(topic, digitalRead(rele_pin) ? "ON" : "OFF", true, 0);

  // 16b. Temperatura Esterna (da MQTT / HA)
  if (!isnan(ext_temp_val)) {
    dtostrf(ext_temp_val, 0, 1, buf);
    snprintf(topic, sizeof(topic), "%soutdoor_temp/state", thermostat_id);
    client.publish(topic, buf, true, 0);
  }

  // 17-19. Info Gerarchiche Immobile, Piano, Stanza
  snprintf(topic, sizeof(topic), "%sinfo/house", thermostat_id);
  client.publish(topic, get_house_name().c_str(), true, 0);

  snprintf(topic, sizeof(topic), "%sinfo/floor", thermostat_id);
  client.publish(topic, get_floor_name().c_str(), true, 0);

  snprintf(topic, sizeof(topic), "%sinfo/room", thermostat_id);
  client.publish(topic, get_room_name().c_str(), true, 0);

  // 20-21. Diagnostica IP & Uptime
  if (WiFi.status() == WL_CONNECTED) {
    snprintf(topic, sizeof(topic), "%sinfo/ip", thermostat_id);
    client.publish(topic, WiFi.localIP().toString().c_str(), true, 0);
  }

  uint32_t up_s = millis() / 1000;
  uint32_t up_h = up_s / 3600;
  uint32_t up_m = (up_s % 3600) / 60;
  char upBuf[24];
  snprintf(upBuf, sizeof(upBuf), "%uh %um", (unsigned)up_h, (unsigned)up_m);
  snprintf(topic, sizeof(topic), "%sinfo/uptime", thermostat_id);
  client.publish(topic, upBuf, true, 0);
}

String slugify(const String &s) {
  String res = s;
  res.trim();
  res.toLowerCase();
  res.replace(" ", "_");
  res.replace("-", "_");
  res.replace("'", "");
  res.replace("\"", "");
  String clean = "";
  for (unsigned int i = 0; i < res.length(); i++) {
    char c = res[i];
    if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_') {
      clean += c;
    }
  }
  return clean;
}

String get_house_name() {
  String h = String(house_name);
  h.trim();
  if (h.length() == 0) h = "Casa";
  return h;
}

String get_floor_name() {
  String f = String(floor_name);
  f.trim();
  if (f.length() == 0) f = "Piano Terra";
  return f;
}

String get_room_name() {
  String r = String(room_name);
  r.trim();
  if (r.length() == 0) r = "Zona Giorno";
  return r;
}

String get_zone_name() {
  String r = get_room_name();
  String f = get_floor_name();
  if (r.equalsIgnoreCase(f)) return r;
  return r + " (" + f + ")";
}

String get_full_location_name() {
  return get_house_name() + " &rsaquo; " + get_floor_name() + " &rsaquo; " + get_room_name();
}

String get_zone_slug() {
  String h_slug = slugify(get_house_name());
  String f_slug = slugify(get_floor_name());
  String r_slug = slugify(get_room_name());
  String s = "";
  if (h_slug.length() > 0 && h_slug != "casa") s += h_slug + "_";
  if (f_slug.length() > 0) s += f_slug + "_";
  if (r_slug.length() > 0) s += r_slug;
  if (s.length() == 0) s = String(chipId, HEX);
  return s;
}

String generate_auto_topic() {
  String h_slug = slugify(get_house_name());
  String f_slug = slugify(get_floor_name());
  String r_slug = slugify(get_room_name());
  if (h_slug.length() == 0) h_slug = "casa";
  if (f_slug.length() == 0) f_slug = "piano_terra";
  if (r_slug.length() == 0) r_slug = "zona_giorno";
  return h_slug + "/clima/" + f_slug + "/" + r_slug + "/";
}

void publish_ha_discovery() {
  if (!client.connected()) return;

  String zoneName = get_zone_name();
  String zoneSlug = get_zone_slug();
  String devId = "termostato_" + zoneSlug + "_" + String(chipId, HEX);
  String ipUrl = (WiFi.status() == WL_CONNECTED) ? ("http://" + WiFi.localIP().toString() + "/") : "http://192.168.4.1/";
  String baseTopic = String(thermostat_id);

  // 1. Entita' Principale: Climate (Termostato Home Assistant per il singolo piano)
  {
    DynamicJsonDocument doc(896);
    doc["~"]              = baseTopic;
    doc["name"]           = "Clima " + zoneName;
    doc["obj_id"]         = "termostato_" + zoneSlug;
    doc["uniq_id"]        = devId + "_climate";
    doc["avty_t"]         = "~status";
    JsonArray modes       = doc.createNestedArray("modes");
    modes.add("off");
    modes.add("heat");
    modes.add("cool");
    doc["mode_stat_t"]    = "~mode/get";
    doc["mode_cmd_t"]     = "~mode/set";
    JsonArray pr_modes    = doc.createNestedArray("pr_modes");
    pr_modes.add("none");
    pr_modes.add("schedule");
    pr_modes.add("eco");
    pr_modes.add("away");
    pr_modes.add("boost");
    doc["pr_mode_stat_t"] = "~preset/get";
    doc["pr_mode_cmd_t"]  = "~preset/set";
    doc["act_t"]          = "~action/get";
    doc["curr_temp_t"]    = "~temperature/effective";
    doc["curr_hum_t"]     = "~humidity/get";
    doc["temp_stat_t"]    = "~temperature/target";
    doc["temp_cmd_t"]     = "~temperature/set";
    doc["min_temp"]       = 10.0;
    doc["max_temp"]       = 35.0;
    doc["temp_step"]      = 0.1;
    doc["temp_unit"]      = "C";

    JsonObject dev = doc.createNestedObject("dev");
    JsonArray ids = dev.createNestedArray("ids");
    ids.add(devId);
    dev["name"] = "Termostato " + zoneName;
    dev["sa"]   = get_floor_name() + " - " + get_room_name();
    dev["mdl"]  = "Termostato Smart Touch ESP32";
    dev["mf"]   = "AutoDomain";
    dev["sw"]   = "2.5.0";
    dev["cu"]   = ipUrl;

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/climate/" + devId + "/config";
    bool ok = client.publish(discTopic.c_str(), payload.c_str(), true, 0);
    Serial.printf("HA Discovery [climate %s] (%d bytes) -> %s\n", zoneName.c_str(), payload.length(), ok ? "OK" : "ERR");
  }

  // 2. Sensore Temperatura del piano
  {
    DynamicJsonDocument doc(512);
    doc["~"]            = baseTopic;
    doc["name"]         = "Temperatura " + zoneName;
    doc["obj_id"]       = "termostato_" + zoneSlug + "_temperatura";
    doc["uniq_id"]      = devId + "_temp";
    doc["stat_t"]       = "~temperature/get";
    doc["avty_t"]       = "~status";
    doc["dev_cla"]      = "temperature";
    doc["stat_cla"]     = "measurement";
    doc["unit_of_meas"] = "°C";
    JsonObject dev = doc.createNestedObject("dev");
    JsonArray ids = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/sensor/" + devId + "_temp/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 2b. Sensore Temperatura Esterna (da MQTT / Home Assistant)
  {
    DynamicJsonDocument doc(512);
    doc["~"]            = baseTopic;
    doc["name"]         = "Temperatura Esterna " + zoneName;
    doc["obj_id"]       = "termostato_" + zoneSlug + "_temp_esterna";
    doc["uniq_id"]      = devId + "_outdoor_temp";
    doc["stat_t"]       = "~outdoor_temp/state";
    doc["avty_t"]       = "~status";
    doc["dev_cla"]      = "temperature";
    doc["stat_cla"]     = "measurement";
    doc["unit_of_meas"] = "°C";
    doc["icon"]         = "mdi:sun-thermometer-outline";
    JsonObject dev = doc.createNestedObject("dev");
    JsonArray ids = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/sensor/" + devId + "_outdoor_temp/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 3. Sensore Umidita' del piano
  {
    DynamicJsonDocument doc(512);
    doc["~"]            = baseTopic;
    doc["name"]         = "Umidità " + zoneName;
    doc["obj_id"]       = "termostato_" + zoneSlug + "_umidita";
    doc["uniq_id"]      = devId + "_hum";
    doc["stat_t"]       = "~humidity/get";
    doc["avty_t"]       = "~status";
    doc["dev_cla"]      = "humidity";
    doc["stat_cla"]     = "measurement";
    doc["unit_of_meas"] = "%";
    JsonObject dev = doc.createNestedObject("dev");
    JsonArray ids = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/sensor/" + devId + "_hum/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 4. Sensore Pressione Atmosferica del piano
  {
    DynamicJsonDocument doc(512);
    doc["~"]            = baseTopic;
    doc["name"]         = "Pressione " + zoneName;
    doc["obj_id"]       = "termostato_" + zoneSlug + "_pressione";
    doc["uniq_id"]      = devId + "_pres";
    doc["stat_t"]       = "~pressure/get";
    doc["avty_t"]       = "~status";
    doc["dev_cla"]      = "atmospheric_pressure";
    doc["stat_cla"]     = "measurement";
    doc["unit_of_meas"] = "hPa";
    JsonObject dev = doc.createNestedObject("dev");
    JsonArray ids = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/sensor/" + devId + "_pres/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 5. Sensore Punto di Rugiada (Dew Point) del piano
  {
    DynamicJsonDocument doc(512);
    doc["~"]            = baseTopic;
    doc["name"]         = "Punto di Rugiada " + zoneName;
    doc["obj_id"]       = "termostato_" + zoneSlug + "_rugiada";
    doc["uniq_id"]      = devId + "_dew";
    doc["stat_t"]       = "~dewpoint/get";
    doc["avty_t"]       = "~status";
    doc["dev_cla"]      = "temperature";
    doc["stat_cla"]     = "measurement";
    doc["unit_of_meas"] = "°C";
    JsonObject dev = doc.createNestedObject("dev");
    JsonArray ids = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/sensor/" + devId + "_dew/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 6. Sensore Comfort & Previsione Meteo del piano
  {
    DynamicJsonDocument doc(512);
    doc["~"]        = baseTopic;
    doc["name"]     = "Comfort e Meteo " + zoneName;
    doc["obj_id"]   = "termostato_" + zoneSlug + "_comfort";
    doc["uniq_id"]  = devId + "_comfort";
    doc["stat_t"]   = "~comfort/get";
    doc["avty_t"]   = "~status";
    doc["icon"]     = "mdi:weather-partly-cloudy";
    JsonObject dev = doc.createNestedObject("dev");
    JsonArray ids = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/sensor/" + devId + "_comfort/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 7. Interruttore Cronotermostato Settimanale del piano
  {
    DynamicJsonDocument doc(512);
    doc["~"]        = baseTopic;
    doc["name"]     = "Cronotermostato " + zoneName;
    doc["obj_id"]   = "termostato_" + zoneSlug + "_crono";
    doc["uniq_id"]  = devId + "_crono";
    doc["stat_t"]   = "~crono/get";
    doc["cmd_t"]    = "~crono/set";
    doc["avty_t"]   = "~status";
    doc["icon"]     = "mdi:calendar-clock";
    JsonObject dev = doc.createNestedObject("dev");
    JsonArray ids = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/switch/" + devId + "_crono/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 8. Interruttore Buzzer Audio Touch del piano
  {
    DynamicJsonDocument doc(512);
    doc["~"]        = baseTopic;
    doc["name"]     = "Buzzer Touch " + zoneName;
    doc["obj_id"]   = "termostato_" + zoneSlug + "_buzzer";
    doc["uniq_id"]  = devId + "_buzzer";
    doc["stat_t"]   = "~buzzer/get";
    doc["cmd_t"]    = "~buzzer/set";
    doc["avty_t"]   = "~status";
    doc["icon"]     = "mdi:volume-high";
    JsonObject dev = doc.createNestedObject("dev");
    JsonArray ids = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/switch/" + devId + "_buzzer/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 9. Sensore Fascia Oraria / Preset Attivo del piano
  {
    DynamicJsonDocument doc(512);
    doc["~"]        = baseTopic;
    doc["name"]     = "Fascia Oraria " + zoneName;
    doc["obj_id"]   = "termostato_" + zoneSlug + "_fascia_crono";
    doc["uniq_id"]  = devId + "_slot";
    doc["stat_t"]   = "~crono/slot";
    doc["avty_t"]   = "~status";
    doc["icon"]     = "mdi:clock-outline";
    JsonObject dev = doc.createNestedObject("dev");
    JsonArray ids = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/sensor/" + devId + "_slot/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 10. Sensore Ore Accensione Oggi
  {
    DynamicJsonDocument doc(512);
    doc["~"]            = baseTopic;
    doc["name"]         = "Ore Caldaia Oggi " + zoneName;
    doc["obj_id"]       = "termostato_" + zoneSlug + "_ore_oggi";
    doc["uniq_id"]      = devId + "_runtime_today";
    doc["stat_t"]       = "~runtime/today";
    doc["avty_t"]       = "~status";
    doc["unit_of_meas"] = "h";
    doc["icon"]         = "mdi:fire-circle";
    JsonObject dev = doc.createNestedObject("dev");
    JsonArray ids = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/sensor/" + devId + "_runtime_today/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 11. Sensore Ore Accensione Ultimi 7 Giorni
  {
    DynamicJsonDocument doc(512);
    doc["~"]            = baseTopic;
    doc["name"]         = "Ore Caldaia 7 Giorni " + zoneName;
    doc["obj_id"]       = "termostato_" + zoneSlug + "_ore_7gg";
    doc["uniq_id"]      = devId + "_runtime_7d";
    doc["stat_t"]       = "~runtime/7days";
    doc["avty_t"]       = "~status";
    doc["unit_of_meas"] = "h";
    doc["icon"]         = "mdi:chart-timeline-variant";
    JsonObject dev = doc.createNestedObject("dev");
    JsonArray ids = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/sensor/" + devId + "_runtime_7d/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 11b. Sensore Duty Cycle Caldaia Oggi (%)
  {
    DynamicJsonDocument doc(512);
    doc["~"]            = baseTopic;
    doc["name"]         = "Duty Cycle Caldaia " + zoneName;
    doc["obj_id"]       = "termostato_" + zoneSlug + "_duty_cycle";
    doc["uniq_id"]      = devId + "_duty_cycle";
    doc["stat_t"]       = "~runtime/duty_cycle";
    doc["avty_t"]       = "~status";
    doc["unit_of_meas"] = "%";
    doc["icon"]         = "mdi:chart-donut";
    JsonObject dev = doc.createNestedObject("dev");
    JsonArray ids = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/sensor/" + devId + "_duty_cycle/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 12. Switch Protezione Antigelo
  {
    DynamicJsonDocument doc(512);
    doc["~"]        = baseTopic;
    doc["name"]     = "Antigelo " + zoneName;
    doc["obj_id"]   = "termostato_" + zoneSlug + "_antifrost";
    doc["uniq_id"]  = devId + "_antifrost";
    doc["stat_t"]   = "~antifrost/get";
    doc["cmd_t"]    = "~antifrost/set";
    doc["avty_t"]   = "~status";
    doc["icon"]     = "mdi:snowflake-alert";
    JsonObject dev = doc.createNestedObject("dev");
    JsonArray ids = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/switch/" + devId + "_antifrost/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 13. Switch Rilevamento Finestra Aperta
  {
    DynamicJsonDocument doc(512);
    doc["~"]        = baseTopic;
    doc["name"]     = "Finestra Aperta " + zoneName;
    doc["obj_id"]   = "termostato_" + zoneSlug + "_openwin";
    doc["uniq_id"]  = devId + "_openwin";
    doc["stat_t"]   = "~openwin/get";
    doc["cmd_t"]    = "~openwin/set";
    doc["avty_t"]   = "~status";
    doc["icon"]     = "mdi:window-open-variant";
    JsonObject dev = doc.createNestedObject("dev");
    JsonArray ids = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/switch/" + devId + "_openwin/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 14. Sensore Potenza Segnale Wi-Fi (%)
  {
    DynamicJsonDocument doc(512);
    doc["~"]            = baseTopic;
    doc["name"]         = "Segnale Wi-Fi " + zoneName;
    doc["obj_id"]       = "termostato_" + zoneSlug + "_wifi_signal";
    doc["uniq_id"]      = devId + "_wifi_signal";
    doc["stat_t"]       = "~wifi/signal";
    doc["avty_t"]       = "~status";
    doc["unit_of_meas"] = "%";
    doc["icon"]         = "mdi:wifi";
    doc["stat_cla"]     = "measurement";
    JsonObject dev = doc.createNestedObject("dev");
    JsonArray ids = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/sensor/" + devId + "_wifi_signal/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 15. Sensore RSSI Wi-Fi (dBm)
  {
    DynamicJsonDocument doc(512);
    doc["~"]            = baseTopic;
    doc["name"]         = "RSSI Wi-Fi " + zoneName;
    doc["obj_id"]       = "termostato_" + zoneSlug + "_wifi_rssi";
    doc["uniq_id"]      = devId + "_wifi_rssi";
    doc["stat_t"]       = "~wifi/rssi";
    doc["avty_t"]       = "~status";
    doc["dev_cla"]      = "signal_strength";
    doc["stat_cla"]     = "measurement";
    doc["unit_of_meas"] = "dBm";
    doc["icon"]         = "mdi:wifi-strength-3";
    JsonObject dev = doc.createNestedObject("dev");
    JsonArray ids = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/sensor/" + devId + "_wifi_rssi/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 16. Sensore Binario Stato Caldaia / Relè
  {
    DynamicJsonDocument doc(512);
    doc["~"]        = baseTopic;
    doc["name"]     = "Stato Caldaia " + zoneName;
    doc["obj_id"]   = "termostato_" + zoneSlug + "_caldaia";
    doc["uniq_id"]  = devId + "_caldaia";
    doc["stat_t"]   = "~relay/state";
    doc["avty_t"]   = "~status";
    doc["dev_cla"]  = "running";
    doc["icon"]     = "mdi:fire";
    JsonObject dev  = doc.createNestedObject("dev");
    JsonArray ids   = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/binary_sensor/" + devId + "_caldaia/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 17. Info Immobile / Abitazione
  {
    DynamicJsonDocument doc(512);
    doc["~"]        = baseTopic;
    doc["name"]     = "Abitazione " + zoneName;
    doc["obj_id"]   = "termostato_" + zoneSlug + "_house";
    doc["uniq_id"]  = devId + "_house";
    doc["stat_t"]   = "~info/house";
    doc["avty_t"]   = "~status";
    doc["ent_cat"]  = "diagnostic";
    doc["icon"]     = "mdi:home-city-outline";
    JsonObject dev  = doc.createNestedObject("dev");
    JsonArray ids   = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/sensor/" + devId + "_house/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 18. Info Piano / Livello
  {
    DynamicJsonDocument doc(512);
    doc["~"]        = baseTopic;
    doc["name"]     = "Piano " + zoneName;
    doc["obj_id"]   = "termostato_" + zoneSlug + "_floor";
    doc["uniq_id"]  = devId + "_floor";
    doc["stat_t"]   = "~info/floor";
    doc["avty_t"]   = "~status";
    doc["ent_cat"]  = "diagnostic";
    doc["icon"]     = "mdi:stairs";
    JsonObject dev  = doc.createNestedObject("dev");
    JsonArray ids   = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/sensor/" + devId + "_floor/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 19. Info Stanza / Zona
  {
    DynamicJsonDocument doc(512);
    doc["~"]        = baseTopic;
    doc["name"]     = "Stanza " + zoneName;
    doc["obj_id"]   = "termostato_" + zoneSlug + "_room";
    doc["uniq_id"]  = devId + "_room";
    doc["stat_t"]   = "~info/room";
    doc["avty_t"]   = "~status";
    doc["ent_cat"]  = "diagnostic";
    doc["icon"]     = "mdi:floor-plan";
    JsonObject dev  = doc.createNestedObject("dev");
    JsonArray ids   = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/sensor/" + devId + "_room/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 20. Sensore Indirizzo IP
  {
    DynamicJsonDocument doc(512);
    doc["~"]        = baseTopic;
    doc["name"]     = "Indirizzo IP " + zoneName;
    doc["obj_id"]   = "termostato_" + zoneSlug + "_ip";
    doc["uniq_id"]  = devId + "_ip";
    doc["stat_t"]   = "~info/ip";
    doc["avty_t"]   = "~status";
    doc["ent_cat"]  = "diagnostic";
    doc["icon"]     = "mdi:ip-network-outline";
    JsonObject dev  = doc.createNestedObject("dev");
    JsonArray ids   = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/sensor/" + devId + "_ip/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 21. Sensore Uptime
  {
    DynamicJsonDocument doc(512);
    doc["~"]        = baseTopic;
    doc["name"]     = "Tempo Attività " + zoneName;
    doc["obj_id"]   = "termostato_" + zoneSlug + "_uptime";
    doc["uniq_id"]  = devId + "_uptime";
    doc["stat_t"]   = "~info/uptime";
    doc["avty_t"]   = "~status";
    doc["ent_cat"]  = "diagnostic";
    doc["icon"]     = "mdi:timer-outline";
    JsonObject dev  = doc.createNestedObject("dev");
    JsonArray ids   = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/sensor/" + devId + "_uptime/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 22. Tasto Riavvio Remoto
  {
    DynamicJsonDocument doc(512);
    doc["~"]        = baseTopic;
    doc["name"]     = "Riavvia Termostato " + zoneName;
    doc["obj_id"]   = "termostato_" + zoneSlug + "_reboot";
    doc["uniq_id"]  = devId + "_reboot";
    doc["cmd_t"]    = "~reboot/set";
    doc["avty_t"]   = "~status";
    doc["ent_cat"]  = "config";
    doc["dev_cla"]  = "restart";
    doc["icon"]     = "mdi:restart";
    JsonObject dev  = doc.createNestedObject("dev");
    JsonArray ids   = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/button/" + devId + "_reboot/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 23. Selettore Preset Rapido
  {
    DynamicJsonDocument doc(640);
    doc["~"]          = baseTopic;
    doc["name"]       = "Selettore Preset " + zoneName;
    doc["obj_id"]     = "termostato_" + zoneSlug + "_preset_select";
    doc["uniq_id"]    = devId + "_preset_select";
    doc["stat_t"]     = "~preset/get";
    doc["cmd_t"]      = "~preset/set";
    doc["avty_t"]     = "~status";
    doc["icon"]       = "mdi:tune";
    JsonArray opts    = doc.createNestedArray("options");
    opts.add("none");
    opts.add("schedule");
    opts.add("away");
    opts.add("boost");
    JsonObject dev    = doc.createNestedObject("dev");
    JsonArray ids     = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/select/" + devId + "_preset_select/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 24. Switch Isteresi
  {
    DynamicJsonDocument doc(512);
    doc["~"]        = baseTopic;
    doc["name"]     = "Isteresi " + zoneName;
    doc["obj_id"]   = "termostato_" + zoneSlug + "_hysteresis";
    doc["uniq_id"]  = devId + "_hysteresis";
    doc["stat_t"]   = "~hysteresis/get";
    doc["cmd_t"]    = "~hysteresis/set";
    doc["avty_t"]   = "~status";
    doc["icon"]     = "mdi:valve";
    JsonObject dev  = doc.createNestedObject("dev");
    JsonArray ids   = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/switch/" + devId + "_hysteresis/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 25. Number Valore Isteresi (°C)
  {
    DynamicJsonDocument doc(512);
    doc["~"]            = baseTopic;
    doc["name"]         = "Valore Isteresi " + zoneName;
    doc["obj_id"]       = "termostato_" + zoneSlug + "_hysteresis_val";
    doc["uniq_id"]      = devId + "_hysteresis_val";
    doc["stat_t"]       = "~hysteresis_val/get";
    doc["cmd_t"]        = "~hysteresis_val/set";
    doc["avty_t"]       = "~status";
    doc["min"]          = 0.1;
    doc["max"]          = 2.0;
    doc["step"]         = 0.1;
    doc["unit_of_meas"] = "°C";
    doc["mode"]         = "box";
    doc["icon"]         = "mdi:delta";
    JsonObject dev      = doc.createNestedObject("dev");
    JsonArray ids       = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/number/" + devId + "_hysteresis_val/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 26. Switch Protezione Antiblocco Pompa
  {
    DynamicJsonDocument doc(512);
    doc["~"]        = baseTopic;
    doc["name"]     = "Protezione Pompa " + zoneName;
    doc["obj_id"]   = "termostato_" + zoneSlug + "_pump_prot";
    doc["uniq_id"]  = devId + "_pump_prot";
    doc["stat_t"]   = "~pump_prot/get";
    doc["cmd_t"]    = "~pump_prot/set";
    doc["avty_t"]   = "~status";
    doc["icon"]     = "mdi:pump";
    JsonObject dev  = doc.createNestedObject("dev");
    JsonArray ids   = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/switch/" + devId + "_pump_prot/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 27. Button Test Pompa 60s
  {
    DynamicJsonDocument doc(512);
    doc["~"]        = baseTopic;
    doc["name"]     = "Test Pompa 60s " + zoneName;
    doc["obj_id"]   = "termostato_" + zoneSlug + "_pump_test";
    doc["uniq_id"]  = devId + "_pump_test";
    doc["cmd_t"]    = "~pump_test/set";
    doc["avty_t"]   = "~status";
    doc["icon"]     = "mdi:pump";
    JsonObject dev  = doc.createNestedObject("dev");
    JsonArray ids   = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/button/" + devId + "_pump_test/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 28. Switch Modalità Notte (Auto-Dimming)
  {
    DynamicJsonDocument doc(512);
    doc["~"]        = baseTopic;
    doc["name"]     = "Modalita Notte " + zoneName;
    doc["obj_id"]   = "termostato_" + zoneSlug + "_night_mode";
    doc["uniq_id"]  = devId + "_night_mode";
    doc["stat_t"]   = "~night_mode/get";
    doc["cmd_t"]    = "~night_mode/set";
    doc["avty_t"]   = "~status";
    doc["icon"]     = "mdi:weather-night";
    JsonObject dev  = doc.createNestedObject("dev");
    JsonArray ids   = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/switch/" + devId + "_night_mode/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 29. Button Blocco Pulizia Schermo 30s
  {
    DynamicJsonDocument doc(512);
    doc["~"]        = baseTopic;
    doc["name"]     = "Pulizia Schermo " + zoneName;
    doc["obj_id"]   = "termostato_" + zoneSlug + "_clean_screen";
    doc["uniq_id"]  = devId + "_clean_screen";
    doc["cmd_t"]    = "~clean_screen/set";
    doc["avty_t"]   = "~status";
    doc["icon"]     = "mdi:spray-bottle";
    JsonObject dev  = doc.createNestedObject("dev");
    JsonArray ids   = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/button/" + devId + "_clean_screen/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 30. Switch Sensore Finestra Esterno via MQTT
  {
    DynamicJsonDocument doc(512);
    doc["~"]        = baseTopic;
    doc["name"]     = "Finestra Esterna MQTT " + zoneName;
    doc["obj_id"]   = "termostato_" + zoneSlug + "_ext_win_en";
    doc["uniq_id"]  = devId + "_ext_win_en";
    doc["stat_t"]   = "~ext_win_en/get";
    doc["cmd_t"]    = "~ext_win_en/set";
    doc["avty_t"]   = "~status";
    doc["icon"]     = "mdi:window-open";
    JsonObject dev  = doc.createNestedObject("dev");
    JsonArray ids   = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/switch/" + devId + "_ext_win_en/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 31. Binary Sensor Stato Finestra Esterna
  {
    DynamicJsonDocument doc(512);
    doc["~"]        = baseTopic;
    doc["name"]     = "Stato Finestra Esterna " + zoneName;
    doc["obj_id"]   = "termostato_" + zoneSlug + "_ext_win_state";
    doc["uniq_id"]  = devId + "_ext_win_state";
    doc["stat_t"]   = "~ext_win/state";
    doc["avty_t"]   = "~status";
    doc["dev_cla"]  = "window";
    doc["icon"]     = "mdi:window-open-variant";
    JsonObject dev  = doc.createNestedObject("dev");
    JsonArray ids   = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/binary_sensor/" + devId + "_ext_win_state/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 32. Number Soglia Antigelo (°C)
  {
    DynamicJsonDocument doc(512);
    doc["~"]            = baseTopic;
    doc["name"]         = "Soglia Antigelo " + zoneName;
    doc["obj_id"]       = "termostato_" + zoneSlug + "_antifrost_th";
    doc["uniq_id"]      = devId + "_antifrost_th";
    doc["stat_t"]       = "~antifrost_th/get";
    doc["cmd_t"]        = "~antifrost_th/set";
    doc["avty_t"]       = "~status";
    doc["min"]          = 3.0;
    doc["max"]          = 10.0;
    doc["step"]         = 0.5;
    doc["unit_of_meas"] = "°C";
    doc["mode"]         = "box";
    doc["icon"]         = "mdi:snowflake-thermometer";
    JsonObject dev      = doc.createNestedObject("dev");
    JsonArray ids       = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/number/" + devId + "_antifrost_th/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 33. Switch Anticipo Termico (Optimum Start)
  {
    DynamicJsonDocument doc(512);
    doc["~"]        = baseTopic;
    doc["name"]     = "Anticipo Termico " + zoneName;
    doc["obj_id"]   = "termostato_" + zoneSlug + "_opt_start";
    doc["uniq_id"]  = devId + "_opt_start";
    doc["stat_t"]   = "~opt_start/get";
    doc["cmd_t"]    = "~opt_start/set";
    doc["avty_t"]   = "~status";
    doc["icon"]     = "mdi:timer-sand";
    JsonObject dev  = doc.createNestedObject("dev");
    JsonArray ids   = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/switch/" + devId + "_opt_start/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 34. Number Velocita Riscaldamento (°C/h)
  {
    DynamicJsonDocument doc(512);
    doc["~"]            = baseTopic;
    doc["name"]         = "Velocita Riscaldamento " + zoneName;
    doc["obj_id"]       = "termostato_" + zoneSlug + "_opt_start_speed";
    doc["uniq_id"]      = devId + "_opt_start_speed";
    doc["stat_t"]       = "~opt_start_speed/get";
    doc["cmd_t"]        = "~opt_start_speed/set";
    doc["avty_t"]       = "~status";
    doc["min"]          = 0.5;
    doc["max"]          = 5.0;
    doc["step"]         = 0.1;
    doc["unit_of_meas"] = "°C/h";
    doc["mode"]         = "box";
    doc["icon"]         = "mdi:speedometer";
    JsonObject dev      = doc.createNestedObject("dev");
    JsonArray ids       = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/number/" + devId + "_opt_start_speed/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 35. Sensore Temperatura Effettiva
  {
    DynamicJsonDocument doc(512);
    doc["~"]            = baseTopic;
    doc["name"]         = "Temp Effettiva " + zoneName;
    doc["obj_id"]       = "termostato_" + zoneSlug + "_effective_temp";
    doc["uniq_id"]      = devId + "_effective_temp";
    doc["stat_t"]       = "~temperature/effective";
    doc["avty_t"]       = "~status";
    doc["unit_of_meas"] = "°C";
    doc["dev_cla"]      = "temperature";
    doc["icon"]         = "mdi:thermometer-check";
    JsonObject dev      = doc.createNestedObject("dev");
    JsonArray ids       = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/sensor/" + devId + "_effective_temp/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 36. Switch Auto-Apprendimento Termico
  {
    DynamicJsonDocument doc(512);
    doc["~"]       = baseTopic;
    doc["name"]    = "Auto-Apprendimento Termico " + zoneName;
    doc["obj_id"]  = "termostato_" + zoneSlug + "_thermal_learning";
    doc["uniq_id"] = devId + "_thermal_learning";
    doc["stat_t"]  = "~thermal_learning/get";
    doc["cmd_t"]   = "~thermal_learning/set";
    doc["avty_t"]  = "~status";
    doc["icon"]    = "mdi:brain";
    JsonObject dev = doc.createNestedObject("dev");
    JsonArray ids  = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/switch/" + devId + "_thermal_learning/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 37. Sensore Velocita Riscaldamento Appresa (°C/h)
  {
    DynamicJsonDocument doc(512);
    doc["~"]            = baseTopic;
    doc["name"]         = "Velocita Riscaldamento Appresa " + zoneName;
    doc["obj_id"]       = "termostato_" + zoneSlug + "_learned_heat_speed";
    doc["uniq_id"]      = devId + "_learned_heat_speed";
    doc["stat_t"]       = "~learned_heat_speed/get";
    doc["avty_t"]       = "~status";
    doc["unit_of_meas"] = "°C/h";
    doc["icon"]         = "mdi:trending-up";
    JsonObject dev      = doc.createNestedObject("dev");
    JsonArray ids       = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/sensor/" + devId + "_learned_heat_speed/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 38. Sensore Cicli Apprendimento Termico
  {
    DynamicJsonDocument doc(512);
    doc["~"]       = baseTopic;
    doc["name"]    = "Cicli Apprendimento Termico " + zoneName;
    doc["obj_id"]  = "termostato_" + zoneSlug + "_heat_cycles";
    doc["uniq_id"] = devId + "_heat_cycles";
    doc["stat_t"]  = "~heat_cycles/get";
    doc["avty_t"]  = "~status";
    doc["icon"]    = "mdi:school";
    JsonObject dev = doc.createNestedObject("dev");
    JsonArray ids  = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/sensor/" + devId + "_heat_cycles/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 39. Pulsante Avvia Calibrazione Rapida 30m
  {
    DynamicJsonDocument doc(512);
    doc["~"]       = baseTopic;
    doc["name"]    = "Avvia Calibrazione Termica (30m) " + zoneName;
    doc["obj_id"]  = "termostato_" + zoneSlug + "_start_training";
    doc["uniq_id"] = devId + "_start_training";
    doc["cmd_t"]   = "~training/start";
    doc["avty_t"]  = "~status";
    doc["icon"]    = "mdi:play-circle-outline";
    JsonObject dev = doc.createNestedObject("dev");
    JsonArray ids  = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/button/" + devId + "_start_training/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  // 40. Pulsante Ferma Calibrazione
  {
    DynamicJsonDocument doc(512);
    doc["~"]       = baseTopic;
    doc["name"]    = "Interrompi Calibrazione " + zoneName;
    doc["obj_id"]  = "termostato_" + zoneSlug + "_stop_training";
    doc["uniq_id"] = devId + "_stop_training";
    doc["cmd_t"]   = "~training/stop";
    doc["avty_t"]  = "~status";
    doc["icon"]    = "mdi:stop-circle-outline";
    JsonObject dev = doc.createNestedObject("dev");
    JsonArray ids  = dev.createNestedArray("ids");
    ids.add(devId);

    String payload;
    serializeJson(doc, payload);
    String discTopic = "homeassistant/button/" + devId + "_stop_training/config";
    client.publish(discTopic.c_str(), payload.c_str(), true, 0);
  }

  publish_ha_state();
}

void connect() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  unsigned long now = millis();
  if (previousMillis_mqtt != 0 &&
      (now - previousMillis_mqtt < interval_mqtt_retry)) {
    return;
  }
  previousMillis_mqtt = now;

  Serial.print("\nMQTT connecting to ");
  Serial.print(mqtt_server);
  Serial.print("...");

  char charBuf[50];
  stringOne = "termostato_" + String(chipId, HEX);
  stringOne.toCharArray(charBuf, sizeof(charBuf));

  static char willTopic[96];
  snprintf(willTopic, sizeof(willTopic), "%sstatus", thermostat_id);
  client.setWill(willTopic, "offline", true, 0);

  if (client.connect(charBuf, mqtt_user, mqtt_pass)) {
    Serial.println("\nconnected!");
    client.subscribe("home/outside/temperature");
    client.subscribe("home/outside/humidity");
    client.subscribe("scene/#");
    client.subscribe("homeassistant/status");
    client.subscribe(thermostat_id + String("temperature/set"));
    client.subscribe(thermostat_id + String("mode/set"));
    client.subscribe(thermostat_id + String("preset/set"));
    client.subscribe(thermostat_id + String("crono/set"));
    client.subscribe(thermostat_id + String("buzzer/set"));
    client.subscribe(thermostat_id + String("antifrost/set"));
    client.subscribe(thermostat_id + String("openwin/set"));
    client.subscribe(thermostat_id + String("reboot/set"));
    client.subscribe(thermostat_id + String("hysteresis/set"));
    client.subscribe(thermostat_id + String("hysteresis_val/set"));
    client.subscribe(thermostat_id + String("pump_prot/set"));
    client.subscribe(thermostat_id + String("pump_test/set"));
    client.subscribe(thermostat_id + String("night_mode/set"));
    client.subscribe(thermostat_id + String("clean_screen/set"));
    client.subscribe(thermostat_id + String("ext_win_en/set"));
    client.subscribe(thermostat_id + String("antifrost_th/set"));
    client.subscribe(thermostat_id + String("outdoor_temp/set"));
    client.subscribe(thermostat_id + String("ext_temp_en/set"));
    client.subscribe(thermostat_id + String("opt_start/set"));
    client.subscribe(thermostat_id + String("opt_start_speed/set"));
    client.subscribe(thermostat_id + String("remote_sensor/mode/set"));
    client.subscribe(thermostat_id + String("remote_sensor/topic/set"));
    client.subscribe(thermostat_id + String("thermal_learning/set"));
    client.subscribe(thermostat_id + String("training/start"));
    client.subscribe(thermostat_id + String("training/stop"));
    if (ext_temp_enabled && ext_temp_topic[0] != '\0') {
      client.subscribe(ext_temp_topic);
    }
    if (ext_window_enabled && ext_window_topic[0] != '\0') {
      client.subscribe(ext_window_topic);
    }
    if (remote_sensor_mode != 0 && remote_sensor_topic[0] != '\0') {
      client.subscribe(remote_sensor_topic);
    }
    Serial.println("Subscribed to " + String(thermostat_id) + "set");

    publish_ha_discovery();
  } else {
    Serial.println(" failed, will retry later.");
  }
}

void messageReceived(String &topic, String &payload) {
  char myConcatenation[80];

  Serial.println("incoming: " + topic + " - " + payload);
  if (topic == "homeassistant/status" && payload == "online") {
    publish_ha_discovery();
    return;
  }
  // Gestione Temperatura Esterna da Home Assistant via MQTT
  snprintf(myConcatenation, sizeof(myConcatenation), "%soutdoor_temp/set", thermostat_id);
  bool is_outdoor_temp_topic = false;
  if (topic.equals(myConcatenation)) {
    is_outdoor_temp_topic = true;
  } else if (ext_temp_enabled && ext_temp_topic[0] != '\0' && topic.equals(ext_temp_topic)) {
    is_outdoor_temp_topic = true;
  } else if (topic.equals("home/outside/temperature")) {
    is_outdoor_temp_topic = true;
  }

  if (is_outdoor_temp_topic) {
    float val = parse_temperature_payload(payload);
    if (!isnan(val) && val >= -50.0f && val <= 70.0f) {
      update_outdoor_temp_display(val);
      Serial.printf("Temperatura Esterna aggiornata da MQTT: %.1f °C\n", val);
      if (client.connected()) {
        char outTopic[96];
        snprintf(outTopic, sizeof(outTopic), "%soutdoor_temp/state", thermostat_id);
        char outBuf[10];
        dtostrf(val, 0, 1, outBuf);
        client.publish(outTopic, outBuf, true, 0);
      }
    }
    return;
  }

  // Gestione Sensore Remoto Prioritario via MQTT
  if (remote_sensor_mode != 0 && remote_sensor_topic[0] != '\0' && topic.equals(remote_sensor_topic)) {
    float val = parse_temperature_payload(payload);
    if (!isnan(val) && val >= -40.0f && val <= 70.0f) {
      remote_temp_val = val;
      remote_temp_last_ms = millis();
      Serial.printf("Sensore Remoto aggiornato via MQTT: %.1f °C\n", val);
      evaluate_crono(false);
      publish_ha_state();
    }
    return;
  }
  if (topic.substring(0, 5) == "scene") {
    // reserved
  }
  snprintf(myConcatenation, sizeof(myConcatenation), "%stemperature/set", thermostat_id);
  if (topic.equals(myConcatenation)) {
    float val = payload.toFloat();
    if (!isnan(val) && val >= 5.0f && val <= 45.0f) {
      temp_setpoint = round(val * 10.0f) / 10.0f;
      save_state_eeprom();
      char buf[10];
      dtostrf(temp_setpoint, 0, 1, buf);
      gslc_ElemSetTxtStr(&m_gui, m_pElemOutTxt_Temp_Setpoint, buf);
      publish_ha_state();
      play_beep(2600, 20);
    }
  }
  snprintf(myConcatenation, sizeof(myConcatenation), "%smode/set", thermostat_id);
  if (topic.equals(myConcatenation)) {
    if (payload.equals("cool")) {
      set_thermostat_mode(2);
    } else if (payload.equals("heat")) {
      set_thermostat_mode(1);
    } else if (payload.equals("off")) {
      set_thermostat_mode(0);
    }
    play_beep(2500, 35);
  }
  snprintf(myConcatenation, sizeof(myConcatenation), "%spreset/set", thermostat_id);
  if (topic.equals(myConcatenation)) {
    if (payload.equals("schedule")) {
      if (boost_active) cancel_boost();
      crono_enabled = true;
      current_preset_mode = "schedule";
    } else if (payload.equals("away")) {
      if (boost_active) cancel_boost();
      crono_enabled = false;
      current_preset_mode = "away";
      temp_setpoint = away_temp;
    } else if (payload.equals("eco")) {
      if (boost_active) cancel_boost();
      crono_enabled = false;
      current_preset_mode = "eco";
      temp_setpoint = eco_temp;
    } else if (payload.equals("boost")) {
      trigger_boost(boost_duration_min, boost_temp);
    } else {
      if (boost_active) cancel_boost();
      crono_enabled = false;
      current_preset_mode = "none";
    }
    evaluate_crono(true);
    char buf[10];
    dtostrf(temp_setpoint, 0, 1, buf);
    gslc_ElemSetTxtStr(&m_gui, m_pElemOutTxt_Temp_Setpoint, buf);
    save_state_eeprom();
    save_json_config();
    publish_ha_state();
    play_beep(2700, 30);
  }
  snprintf(myConcatenation, sizeof(myConcatenation), "%scrono/set", thermostat_id);
  if (topic.equals(myConcatenation)) {
    crono_enabled = (payload == "ON" || payload == "1" || payload == "true");
    current_preset_mode = crono_enabled ? "schedule" : "none";
    evaluate_crono(true);
    save_state_eeprom();
    save_json_config();
    publish_ha_state();
    play_beep(2700, 30);
  }
  snprintf(myConcatenation, sizeof(myConcatenation), "%sbuzzer/set", thermostat_id);
  if (topic.equals(myConcatenation)) {
    buzzer_enabled = (payload == "ON" || payload == "1" || payload == "true");
    save_state_eeprom();
    save_json_config();
    publish_ha_state();
    if (buzzer_enabled) play_beep(2800, 35);
  }
  snprintf(myConcatenation, sizeof(myConcatenation), "%santifrost/set", thermostat_id);
  if (topic.equals(myConcatenation)) {
    antifrost_enabled = (payload == "ON" || payload == "1" || payload == "true");
    save_json_config();
    publish_ha_state();
    play_beep(2600, 25);
  }
  snprintf(myConcatenation, sizeof(myConcatenation), "%sopenwin/set", thermostat_id);
  if (topic.equals(myConcatenation)) {
    open_window_enabled = (payload == "ON" || payload == "1" || payload == "true");
    if (!open_window_enabled) open_window_active = false;
    save_json_config();
    publish_ha_state();
    play_beep(2600, 25);
  }
  // Gestione Sensore Finestra Esterno via MQTT
  if (ext_window_enabled && ext_window_topic[0] != '\0' && topic.equals(ext_window_topic)) {
    String pl = payload;
    pl.toLowerCase();
    if (pl == "on" || pl == "true" || pl == "open" || pl == "1" || pl.indexOf("\"contact\":false") >= 0) {
      ext_window_open = true;
      Serial.println("Sensore Finestra Esterno: APERTA");
    } else {
      ext_window_open = false;
      Serial.println("Sensore Finestra Esterno: CHIUSA");
    }
    publish_ha_state();
    return;
  }
  snprintf(myConcatenation, sizeof(myConcatenation), "%shysteresis/set", thermostat_id);
  if (topic.equals(myConcatenation)) {
    hysteresis_enabled = (payload == "ON" || payload == "1" || payload == "true");
    save_json_config();
    publish_ha_state();
    play_beep(2600, 25);
  }
  snprintf(myConcatenation, sizeof(myConcatenation), "%shysteresis_val/set", thermostat_id);
  if (topic.equals(myConcatenation)) {
    float hv = payload.toFloat();
    if (!isnan(hv) && hv >= 0.1f && hv <= 2.0f) {
      hysteresis_val = round(hv * 10.0f) / 10.0f;
      save_json_config();
      publish_ha_state();
      play_beep(2600, 25);
    }
  }
  snprintf(myConcatenation, sizeof(myConcatenation), "%spump_prot/set", thermostat_id);
  if (topic.equals(myConcatenation)) {
    pump_protection_enabled = (payload == "ON" || payload == "1" || payload == "true");
    save_json_config();
    publish_ha_state();
    play_beep(2600, 25);
  }
  snprintf(myConcatenation, sizeof(myConcatenation), "%spump_test/set", thermostat_id);
  if (topic.equals(myConcatenation)) {
    trigger_pump_test(60);
  }
  snprintf(myConcatenation, sizeof(myConcatenation), "%snight_mode/set", thermostat_id);
  if (topic.equals(myConcatenation)) {
    night_mode_enabled = (payload == "ON" || payload == "1" || payload == "true");
    save_json_config();
    publish_ha_state();
    play_beep(2600, 25);
  }
  snprintf(myConcatenation, sizeof(myConcatenation), "%sclean_screen/set", thermostat_id);
  if (topic.equals(myConcatenation)) {
    trigger_child_lock(30);
  }
  snprintf(myConcatenation, sizeof(myConcatenation), "%sext_win_en/set", thermostat_id);
  if (topic.equals(myConcatenation)) {
    ext_window_enabled = (payload == "ON" || payload == "1" || payload == "true");
    if (!ext_window_enabled) ext_window_open = false;
    if (client.connected() && ext_window_enabled && ext_window_topic[0] != '\0') {
      client.subscribe(ext_window_topic);
    }
    save_json_config();
    publish_ha_state();
    play_beep(2600, 25);
  }
  snprintf(myConcatenation, sizeof(myConcatenation), "%santifrost_th/set", thermostat_id);
  if (topic.equals(myConcatenation)) {
    float ath = payload.toFloat();
    if (!isnan(ath) && ath >= 3.0f && ath <= 10.0f) {
      antifrost_temp_threshold = round(ath * 10.0f) / 10.0f;
      save_json_config();
      publish_ha_state();
      play_beep(2600, 25);
    }
  }
  snprintf(myConcatenation, sizeof(myConcatenation), "%sext_temp_en/set", thermostat_id);
  if (topic.equals(myConcatenation)) {
    ext_temp_enabled = (payload == "ON" || payload == "1" || payload == "true");
    if (!ext_temp_enabled) update_outdoor_temp_display(NAN);
    if (client.connected() && ext_temp_enabled && ext_temp_topic[0] != '\0') {
      client.subscribe(ext_temp_topic);
    }
    save_json_config();
    publish_ha_state();
    play_beep(2600, 25);
  }
  snprintf(myConcatenation, sizeof(myConcatenation), "%sopt_start/set", thermostat_id);
  if (topic.equals(myConcatenation)) {
    opt_start_enabled = (payload == "ON" || payload == "1" || payload == "true");
    save_json_config();
    evaluate_crono(true);
    publish_ha_state();
    play_beep(2600, 25);
  }
  snprintf(myConcatenation, sizeof(myConcatenation), "%sopt_start_speed/set", thermostat_id);
  if (topic.equals(myConcatenation)) {
    float spd = payload.toFloat();
    if (!isnan(spd) && spd >= 0.5f && spd <= 5.0f) {
      opt_start_speed = round(spd * 10.0f) / 10.0f;
      save_json_config();
      evaluate_crono(true);
      publish_ha_state();
      play_beep(2600, 25);
    }
  }
  snprintf(myConcatenation, sizeof(myConcatenation), "%sremote_sensor/mode/set", thermostat_id);
  if (topic.equals(myConcatenation)) {
    int rmode = payload.toInt();
    if (rmode >= 0 && rmode <= 2) {
      remote_sensor_mode = (uint8_t)rmode;
      save_json_config();
      evaluate_crono(true);
      publish_ha_state();
      play_beep(2600, 25);
    }
  }
  snprintf(myConcatenation, sizeof(myConcatenation), "%sremote_sensor/topic/set", thermostat_id);
  if (topic.equals(myConcatenation)) {
    strncpy(remote_sensor_topic, payload.c_str(), sizeof(remote_sensor_topic) - 1);
    remote_sensor_topic[sizeof(remote_sensor_topic) - 1] = '\0';
    if (client.connected() && remote_sensor_mode != 0 && remote_sensor_topic[0] != '\0') {
      client.subscribe(remote_sensor_topic);
    }
    save_json_config();
    publish_ha_state();
    play_beep(2600, 25);
  }
  snprintf(myConcatenation, sizeof(myConcatenation), "%sthermal_learning/set", thermostat_id);
  if (topic.equals(myConcatenation)) {
    thermal_learning_enabled = (payload == "ON" || payload == "1" || payload == "true");
    save_json_config();
    publish_ha_state();
    play_beep(2600, 25);
  }
  snprintf(myConcatenation, sizeof(myConcatenation), "%straining/start", thermostat_id);
  if (topic.equals(myConcatenation)) {
    uint16_t dur = payload.toInt();
    if (dur < 10 || dur > 120) dur = 30;
    start_thermal_training(dur);
  }
  snprintf(myConcatenation, sizeof(myConcatenation), "%straining/stop", thermostat_id);
  if (topic.equals(myConcatenation)) {
    cancel_or_finish_thermal_training(false);
  }
  snprintf(myConcatenation, sizeof(myConcatenation), "%sreboot/set", thermostat_id);
  if (topic.equals(myConcatenation)) {
    Serial.println("Reboot command received via MQTT / Home Assistant!");
    play_beep(3000, 80);
    delay(500);
    ESP.restart();
  }
}
