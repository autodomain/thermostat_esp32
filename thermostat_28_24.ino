//<App !Start!>
// FILE: [gui_design.ino]
// Created by GUIslice Builder version: [0.16.b009]
//
// GUIslice Builder Generated File
//
// For the latest guides, updates and support view:
// https://github.com/ImpulseAdventure/GUIslice
//
//<App !End!>



//   Modificare questo file in caso di malfunzionamento touch
//  C:\Users\Carmelo\Documents\Arduino\libraries\GUIslice\configs\esp-tftespi-default-xpt2046.h
//
//
#include <FS.h>
#include <SPIFFS.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ArduinoOTA.h>

#include <EEPROM.h>
#include <GUIslice.h>

// define the number of bytes you want to access
#define EEPROM_SIZE 16
#define EEPROM_MAGIC_ADDR  1
#define EEPROM_MODE_ADDR   2
#define EEPROM_SP_ADDR     4
#define EEPROM_BUZZER_ADDR 8
#define EEPROM_CRONO_ADDR  9
#define EEPROM_MAGIC_VAL   0xA5
#if defined(HARDWARE_CYD)
  #define BUZZER_PIN         26    // Audio DAC/Speaker su CYD (GPIO 26)
  #define I2C_SDA            22    // Pin SDA su connettore P3 del CYD
  #define I2C_SCL            27    // Pin SCL su connettore P3 del CYD
  #define RELE_PIN_DEFAULT   4     // Relè esterno o GPIO disponibile (es. GPIO 4 o P3)
  #define TFT_BL_PIN         21    // Retroilluminazione CYD (GPIO 21, attivo HIGH)
  #define TFT_BL_BRIGHT      255
  #define TFT_BL_DIM         15
  #define TFT_BL_OFF         0
#else
  #define BUZZER_PIN         21    // Piezo buzzer su AZ-Touch (GPIO 21)
  #define I2C_SDA            17    // SDA su AZ-Touch
  #define I2C_SCL            16    // SCL su AZ-Touch
  #define RELE_PIN_DEFAULT   26    // Relè su AZ-Touch
  #define TFT_BL_PIN         15    // Retroilluminazione AZ-Touch (GPIO 15, attivo LOW)
  #define TFT_BL_BRIGHT      0
  #define TFT_BL_DIM         240
  #define TFT_BL_OFF         255
#endif


//#define TOUCH_ROTATION_SWAPXY

//#define TOUCH_ROTATION_FLIPX
//#define TOUCH_ROTATION_FLIPY


#define SEALEVELPRESSURE_HPA (1013.25)
TwoWire I2CBME = TwoWire(0);
Adafruit_BME280 bme;
bool bme_ok = false;
int i2c_sda_pin = I2C_SDA;
int i2c_scl_pin = I2C_SCL;
uint8_t bme_found_addr = 0;
String last_i2c_scan_html = "";
int i2c_scan_count = 0;

#include <DHT.h>
#include <DHT_U.h>

#if defined(ESP8266)
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#else
#include <WiFi.h>          //https://github.com/esp8266/Arduino
#endif
#include "time.h"
//needed for library
#include <DNSServer.h>
#if defined(ESP8266)
#include <ESP8266WebServer.h>
#else
#include <WebServer.h>
#endif
#include <WiFiManager.h>   
//#include "EEPROM.h"
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

#include <MQTT.h>
WiFiClient net;
MQTTClient client(1024);
WebServer webServer(80);
DNSServer dnsServer;
bool ap_mode_active = false;
char wifi_ssid[45] = "";
char wifi_pass[65] = "";
int touch_remap_yx = 0;
int touch_swap_xy  = 1;
int touch_flip_x   = 0;
int touch_flip_y   = 1;

#if defined(HARDWARE_CYD)
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include <GUIslice_th.h>

#define XPT2046_IRQ  36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK  25
#define XPT2046_CS   33

#define CYD_LED_RED   4
#define CYD_LED_GREEN 16
#define CYD_LED_BLUE  17
#define CYD_LDR_PIN   34

class CYDTouchHandler : public TouchHandler {
public:
  CYDTouchHandler() : ts(XPT2046_CS, XPT2046_IRQ) {
    setSize(320, 240);
    setCalibration(250, 3800, 250, 3800);
    setSwapFlip(false, false, false);
  }
  void begin() override {
    SPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    ts.begin();
    ts.setRotation(1);
  }
  THPoint getPoint() override {
    if (!ts.touched()) return THPoint(0, 0, 0);
    TS_Point p = ts.getPoint();
    return scale(THPoint(p.x, p.y, p.z));
  }
private:
  XPT2046_Touchscreen ts;
};

static CYDTouchHandler cydTouchHandler;
#endif


#include <AverageValue.h>
const long MAX_VALUES_NUM = 30;

int set_temp_address = 0;
int slider_pos;
//flag for saving data
bool shouldSaveConfig = false;
bool reset_config_reboot = false;

const char* ntpServer = "192.168.1.250";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

char mqtt_server[45]="192.168.1.250";
char mqtt_user[32]="admin";
char mqtt_pass[45]="bermuda175209";
char house_name[32] = "Casa";
char floor_name[32] = "Piano Terra";
char room_name[32]  = "Zona Giorno";
bool auto_topic_enabled = true;
char thermostat_id[96]= "casa/clima/piano_terra/zona_giorno/";
char rele_id[64]="cmnd/clima/POWER1";
char mqtt_port[6] = "1883";
char temp_offset[6] = "0";
uint32_t chipId = 0;
String stringOne;
float last_pressure_hpa = 0.0f;
float pressure_history[12] = {0};
int pressure_hist_count = 0;
int pressure_hist_idx = 0;
unsigned long last_pressure_hist_ms = 0;
float pressure_delta_3h = 0.0f;
float calc_dew_point = 0.0f;
float calc_heat_index = 0.0f;
String weather_forecast_str = "In attesa letture...";
String weather_short_str = "---";
String comfort_str = "In attesa...";
String comfort_short_str = "---";
String comfort_color_hex = "#4ade80";

// Buzzer & Cronotermostato Settimanale (Feriali Lun-Ven / Weekend Sab-Dom)
bool buzzer_enabled = true;
uint8_t buzzer_vol  = 1; // 0 = Muto, 1 = Basso (Tick leggero - Default), 2 = Medio, 3 = Alto
bool crono_enabled  = false;
String current_preset_mode = "none"; // "none", "schedule", "away", "boost"
int last_crono_key  = -1;
int current_crono_slot = -1;
String current_crono_desc = "Manuale";

// 1. Protezione Antigelo
bool antifrost_enabled = true;
float antifrost_temp_threshold = 5.0f; // Sotto i 5.0 °C parte la caldaia
bool antifrost_active = false;

// 2. Rilevamento Finestra Aperta
bool open_window_enabled = true;
bool open_window_active = false;
unsigned long open_window_until_ms = 0;
float open_window_temp_hist[10] = {0}; // Campioni di temperatura (1 ogni 30s = finestra 5 minuti)
int open_window_hist_count = 0;
int open_window_hist_idx = 0;
unsigned long last_temp_track_ms = 0;

// 3. Contatore Ore di Accensione (Oggi e ultimi 7 giorni)
uint32_t runtime_today_sec = 0;
uint32_t runtime_history_7d[7] = {0}; // 0 = ieri, 1 = 2gg fa, ..., 6 = 7gg fa
int current_day_of_year = -1;
unsigned long last_runtime_check_ms = 0;
unsigned long last_runtime_save_ms = 0;

// 4. Isteresi Regolabile
bool hysteresis_enabled = true;
float hysteresis_val = 0.2f;

// 5. Protezione Antiblocco Pompa / Valvole (Anti-Seize)
bool pump_protection_enabled = true;
unsigned long last_relay_active_time = 0;
bool pump_test_active = false;
unsigned long pump_test_end_ms = 0;
int last_pump_test_yday = -1;

// 6. Modalità Notte & Auto-Dimming Programmato
bool night_mode_enabled = false;
int night_start_hour = 23; // 23:00
int night_end_hour   = 6;  // 06:00
bool night_active = false;
unsigned long night_wake_until_ms = 0;

// 7. Blocco Pulizia / Child Lock
bool child_lock_active = false;
unsigned long child_lock_until_ms = 0;

// 8. Sensore Finestra Aperta Esterno via MQTT
bool ext_window_enabled = false;
char ext_window_topic[100] = "";
bool ext_window_open = false;

void trigger_child_lock(uint32_t seconds = 30);
void trigger_pump_test(uint32_t seconds = 60);

struct CronoSlot {
  uint16_t start_min; // minuti da mezzanotte (es. 06:30 = 390)
  float temp;         // setpoint fascia (°C)
};

CronoSlot crono_sched[2][4] = {
  { {390, 20.5f}, {510, 18.5f}, {1020, 21.0f}, {1350, 18.0f} }, // 0 = Feriali (Lun-Ven): 06:30, 08:30, 17:00, 22:30
  { {480, 21.0f}, {660, 20.0f}, {1020, 21.5f}, {1380, 18.5f} }  // 1 = Weekend (Sab-Dom): 08:00, 11:00, 17:00, 23:00
};
const char* const crono_slot_names[4] = {"Mattina", "Giorno", "Sera", "Notte"};
const char* const crono_slot_short[4] = {"CRONO MATT", "CRONO GIOR", "CRONO SERA", "CRONO NOTT"};


sensors_event_t event;

AverageValue<float> averageValueT(MAX_VALUES_NUM);
AverageValue<float> averageValueH(MAX_VALUES_NUM);

#define DHTPIN 27     // Digital pin connected to the DHT sensor
#define DHTTYPE    DHT22     // DHT 22 (AM2302)
DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS;
float temp_setpoint = 25.0;
unsigned long previousMillis_temperature = 0;
const long interval_temperature = 2000;
unsigned long previousMillis_NTP = 0;
const long interval_NTP = 60000;
unsigned long previousMillis_graph = 0;
const long interval_graph = 30000;

unsigned long previousMillis_rele = 0;
const long interval_rele = 15000;

unsigned long previousMillis_lcd = 0;
const long interval_lcd = 120000;


int stato = 0;
int stato_shadow = 1;
const float threshold_offset = 0.2;
float temperature_offset = 0;

// NOTA: GPIO 18 e' usato da TFT_SCLK! Usiamo GPIO 26 per il rele locale per non bloccare il bus SPI.
const int rele_pin = RELE_PIN_DEFAULT;

// Forward declarations per funzioni in altri file .ino
void connect();
void messageReceived(String &topic, String &payload);
void update_graph();
void save_state_eeprom();
void publish_ha_discovery();
void publish_ha_state();
String get_zone_name();
String get_zone_slug();
String get_house_name();
String get_floor_name();
String get_room_name();
String get_full_location_name();
String generate_auto_topic();
void update_weather_and_comfort(float t, float h, float p);
void play_beep(int freq = 2600, int duration_ms = 25);
void evaluate_crono(bool force_apply = false);
void update_wifi_signal_bars();

// ------------------------------------------------
// Headers to include
// ------------------------------------------------
#include "gui_design_GSLC.h"

inline void lcd_set_bright() {
  ledcWrite(0, TFT_BL_BRIGHT);
}

inline void lcd_set_dim() {
  ledcWrite(0, TFT_BL_DIM);
}

inline void lcd_set_off() {
  ledcWrite(0, TFT_BL_OFF);
}

void trigger_child_lock(uint32_t seconds) {
  child_lock_active = true;
  child_lock_until_ms = millis() + (seconds * 1000UL);
  publish_ha_state();
  play_beep(2400, 50);
}

void trigger_pump_test(uint32_t seconds) {
  pump_test_active = true;
  pump_test_end_ms = millis() + (seconds * 1000UL);
  digitalWrite(rele_pin, HIGH);
  publish_ha_state();
  play_beep(2800, 60);
  Serial.printf("Protezione antiblocco: avviato test pompa per %u secondi\n", seconds);
}

// ------------------------------------------------
// Program Globals
// ------------------------------------------------

// Save some element references for direct access
//<Save_References !Start!>
gslc_tsElemRef* btn_dec_setpoint  = NULL;
gslc_tsElemRef* btn_inc_setpoint  = NULL;
gslc_tsElemRef* btn_main_page     = NULL;
gslc_tsElemRef* btn_reset         = NULL;
gslc_tsElemRef* m_pElemBtn_menu   = NULL;
gslc_tsElemRef* m_pElemBtn_mode   = NULL;
gslc_tsElemRef* m_pElemBtn_save   = NULL;
gslc_tsElemRef* m_pElemBtn_wake_up= NULL;
gslc_tsElemRef* m_pElemBtn_wake_up2= NULL;
gslc_tsElemRef* m_pElemGraph_Temperature= NULL;
gslc_tsElemRef* m_pElemOutTxt5    = NULL;
gslc_tsElemRef* m_pElemOutTxt6    = NULL;
gslc_tsElemRef* m_pElemOutTxt_Date= NULL;
gslc_tsElemRef* m_pElemOutTxt_Ip_Address16= NULL;
gslc_tsElemRef* m_pElemOutTxt_Temp_Outside= NULL;
gslc_tsElemRef* m_pElemOutTxt_Temp_Setpoint= NULL;
gslc_tsElemRef* m_pElemOutTxt_Time= NULL;
gslc_tsElemRef* m_pElemOutTxt_stat= NULL;
gslc_tsElemRef* m_pElemOutTxt_status= NULL;
gslc_tsElemRef* m_pElemOutTxt_temp_offset= NULL;
gslc_tsElemRef* m_pElemRB5        = NULL;
gslc_tsElemRef* m_pElemRB_sample  = NULL;
gslc_tsElemRef* m_pElemRB_sample2 = NULL;
gslc_tsElemRef* m_pElemRB_sample2_3= NULL;
gslc_tsElemRef* m_pElemRB_sample2_4= NULL;
gslc_tsElemRef* m_pElemSlider_temp_offset= NULL;
gslc_tsElemRef* m_pElemOutTxt_Weather = NULL;
gslc_tsElemRef* m_pElemOutTxt_SetpointLbl = NULL;
//<Save_References !End!>

// Define debug message function
static int16_t DebugOut(char ch) { if (ch == (char)'\n') Serial.println(""); else Serial.write(ch); return 0; }

// ------------------------------------------------
// Callback Methods
// ------------------------------------------------
// Common Button callback
bool CbBtnCommon(void* pvGui,void *pvElemRef,gslc_teTouch eTouch,int16_t nX,int16_t nY)
{
  // Typecast the parameters to match the GUI and element types
  gslc_tsGui*     pGui     = (gslc_tsGui*)(pvGui);
  gslc_tsElemRef* pElemRef = (gslc_tsElemRef*)(pvElemRef);
  gslc_tsElem*    pElem    = gslc_GetElemFromRef(pGui,pElemRef);

  char myConc[80];
  char myChar_[19]="mode/get";
  snprintf(myConc, sizeof(myConc), "%s%s", thermostat_id, myChar_);

  if (night_active) {
    night_wake_until_ms = millis() + 15000UL;
  }

  if (child_lock_active) {
    if (millis() < child_lock_until_ms) {
      previousMillis_lcd = millis();
      lcd_set_bright();
      return true;
    } else {
      child_lock_active = false;
      publish_ha_state();
      play_beep(2600, 35);
    }
  }

  if ( eTouch == GSLC_TOUCH_UP_IN ) {
    // From the element's ID we can determine which button was pressed.
    switch (pElem->nId) {
//<Button Enums !Start!>
      case E_ELEM_BTN_INC:
        previousMillis_lcd = millis();
        lcd_set_bright();
        gslc_ElemSetVisible(&m_gui,m_pElemBtn_wake_up,false);
        gslc_ElemSetVisible(&m_gui,m_pElemBtn_wake_up2,false);
        play_beep(2800, 22);
        temp_setpoint = round((temp_setpoint + 0.1f) * 10.0f) / 10.0f;
        save_state_eeprom();
        publish_ha_state();
        delay(15);
        break;
      case E_ELEM_BTN_DEC:
        previousMillis_lcd = millis();
        lcd_set_bright();
        gslc_ElemSetVisible(&m_gui,m_pElemBtn_wake_up,false);
        gslc_ElemSetVisible(&m_gui,m_pElemBtn_wake_up2,false);
        play_beep(2200, 22);
        temp_setpoint = round((temp_setpoint - 0.1f) * 10.0f) / 10.0f;
        save_state_eeprom();
        publish_ha_state();
        delay(15);
        break;
      case E_ELEM_BTN_MODE:
        previousMillis_lcd = millis();
        lcd_set_bright();
        gslc_ElemSetVisible(&m_gui,m_pElemBtn_wake_up,false);
        gslc_ElemSetVisible(&m_gui,m_pElemBtn_wake_up2,false);
        play_beep(2500, 45);
        if (stato == 0) set_thermostat_mode(1);
        else if (stato == 1) set_thermostat_mode(2);
        else set_thermostat_mode(0);
        delay(10);
        break;
      case E_ELEM_BTN_MENU:
        previousMillis_lcd = millis();
        lcd_set_bright();
        gslc_ElemSetVisible(&m_gui,m_pElemBtn_wake_up,false);
        gslc_ElemSetVisible(&m_gui,m_pElemBtn_wake_up2,false);
        play_beep(2600, 25);
        gslc_SetPageCur(&m_gui, E_PG2);
        break;
      case E_ELEM_BTN_WAKE_UP:
        previousMillis_lcd = millis();
        lcd_set_bright();
        gslc_ElemSetVisible(&m_gui,m_pElemBtn_wake_up,false);
        gslc_ElemSetVisible(&m_gui,m_pElemBtn_wake_up2,false);
        break;
      case E_ELEM_BTN_MAIN_PAGE:
        previousMillis_lcd = millis();
        lcd_set_bright();
        gslc_ElemSetVisible(&m_gui,m_pElemBtn_wake_up,false);
        gslc_ElemSetVisible(&m_gui,m_pElemBtn_wake_up2,false);
        play_beep(2600, 25);
        gslc_SetPageCur(&m_gui,E_PG_MAIN);
        break;
      case E_ELEM_BTN10:
        play_beep(2000, 80);
        reset_config_reboot = true;
        break;
      case E_ELEM_BTN_SAVE:
        previousMillis_lcd = millis();
        lcd_set_bright();
        gslc_ElemSetVisible(&m_gui,m_pElemBtn_wake_up,false);
        gslc_ElemSetVisible(&m_gui,m_pElemBtn_wake_up2,false);
        play_beep(2800, 40);
        EEPROM.write(0, slider_pos);
        EEPROM.commit();
        break;
      case E_ELEM_BTN_WAKE_UP2:
        previousMillis_lcd = millis();
        lcd_set_bright();
        gslc_ElemSetVisible(&m_gui,m_pElemBtn_wake_up,false);
        gslc_ElemSetVisible(&m_gui,m_pElemBtn_wake_up2,false);      
        break;
//<Button Enums !End!>
      default:
        break;
    }
  }
  return true;
}
// Checkbox / radio callbacks
// - Creating a callback function is optional, but doing so enables you to
//   detect changes in the state of the elements.
bool CbCheckbox(void* pvGui, void* pvElemRef, int16_t nSelId, bool bState)
{
  gslc_tsGui*     pGui      = (gslc_tsGui*)(pvGui);
  gslc_tsElemRef* pElemRef  = (gslc_tsElemRef*)(pvElemRef);
  gslc_tsElem*    pElem     = gslc_GetElemFromRef(pGui,pElemRef);
  if (pElemRef == NULL) {
    return false;
  }
  
  boolean bChecked = gslc_ElemXCheckboxGetState(pGui,pElemRef);
  (void)bChecked;

  // Determine which element issued the callback
  switch (pElem->nId) {
//<Checkbox Enums !Start!>

//<Checkbox Enums !End!>
    default:
      break;
  } // switch
  return true;
}
//<Keypad Callback !Start!>
//<Keypad Callback !End!>
//<Spinner Callback !Start!>
//<Spinner Callback !End!>
//<Listbox Callback !Start!>
//<Listbox Callback !End!>
//<Draw Callback !Start!>
//<Draw Callback !End!>

// Callback function for when a slider's position has been updated
bool CbSlidePos(void* pvGui,void* pvElemRef,int16_t nPos)
{
  gslc_tsGui*     pGui     = (gslc_tsGui*)(pvGui);
  gslc_tsElemRef* pElemRef = (gslc_tsElemRef*)(pvElemRef);
  gslc_tsElem*    pElem    = gslc_GetElemFromRef(pGui,pElemRef);
  int16_t         nVal;
  //float t_offset;
  char buffer[12];
  // From the element's ID we can determine which slider was updated.
  switch (pElem->nId) {
//<Slider Enums !Start!>

    case E_ELEM_SLIDER_TEMP_OFFSET:
      // Fetch the slider position
      nVal = gslc_ElemXSliderGetPos(pGui,m_pElemSlider_temp_offset);
      slider_pos=nVal;
      temperature_offset=(nVal-50)/10.0;
      dtostrf(temperature_offset, 4, 1, buffer);
      gslc_ElemSetTxtStr(&m_gui,m_pElemOutTxt_temp_offset,buffer);
      previousMillis_lcd = millis();
      lcd_set_bright();
      gslc_ElemSetVisible(&m_gui,m_pElemBtn_wake_up,false);
      gslc_ElemSetVisible(&m_gui,m_pElemBtn_wake_up2,false); 
      break;
//<Slider Enums !End!>
    default:
      break;
  }

  return true;
}
//<Tick Callback !Start!>
//<Tick Callback !End!>

void apply_touch_settings() {
  m_gui.bTouchRemapYX = (touch_remap_yx != 0);
  m_gui.nSwapXY       = (touch_swap_xy != 0);
  m_gui.nFlipX        = (touch_flip_x != 0);
  m_gui.nFlipY        = (touch_flip_y != 0);
  Serial.printf("Touch settings applied: RemapYX=%d SwapXY=%d FlipX=%d FlipY=%d\n",
                touch_remap_yx, touch_swap_xy, touch_flip_x, touch_flip_y);
}

void save_json_config() {
  DynamicJsonDocument json(2048);
  json["wifi_ssid"]     = wifi_ssid;
  json["wifi_pass"]     = wifi_pass;
  json["mqtt_server"]   = mqtt_server;
  json["mqtt_user"]     = mqtt_user;
  json["mqtt_pass"]     = mqtt_pass;
  json["house_name"]    = house_name;
  json["floor_name"]    = floor_name;
  json["room_name"]     = room_name;
  json["auto_topic"]    = auto_topic_enabled ? 1 : 0;
  json["thermostat_id"] = thermostat_id;
  json["rele_id"]       = rele_id;
  json["t_remap"]       = touch_remap_yx;
  json["t_swap"]        = touch_swap_xy;
  json["t_flipx"]       = touch_flip_x;
  json["t_flipy"]       = touch_flip_y;
  json["i2c_sda"]       = i2c_sda_pin;
  json["i2c_scl"]       = i2c_scl_pin;
  json["buzzer"]        = buzzer_enabled ? buzzer_vol : 0;
  json["crono"]         = crono_enabled ? 1 : 0;
  json["antifrost"]     = antifrost_enabled ? 1 : 0;
  json["antifrost_sp"]  = round(antifrost_temp_threshold * 10.0f) / 10.0f;
  json["openwin"]       = open_window_enabled ? 1 : 0;
  json["hyst_en"]       = hysteresis_enabled ? 1 : 0;
  json["hyst_val"]      = round(hysteresis_val * 10.0f) / 10.0f;
  json["pump_prot"]     = pump_protection_enabled ? 1 : 0;
  json["night_en"]      = night_mode_enabled ? 1 : 0;
  json["night_start"]   = night_start_hour;
  json["night_end"]     = night_end_hour;
  json["ext_win_en"]    = ext_window_enabled ? 1 : 0;
  json["ext_win_top"]   = ext_window_topic;

  JsonArray sched = json.createNestedArray("sched");
  for (int d = 0; d < 2; d++) {
    for (int s = 0; s < 4; s++) {
      sched.add(crono_sched[d][s].start_min);
      sched.add(round(crono_sched[d][s].temp * 10.0f) / 10.0f);
    }
  }

  JsonArray runh = json.createNestedArray("runh");
  runh.add(runtime_today_sec);
  for (int i = 0; i < 7; i++) {
    runh.add(runtime_history_7d[i]);
  }
  json["c_day"] = current_day_of_year;

  File configFile = SPIFFS.open("/config.json", "w");
  if (configFile) {
    serializeJson(json, configFile);
    configFile.close();
    Serial.println("Saved /config.json");
  }
}

String run_i2c_scan(int sda, int scl) {
  i2c_scan_count++;
  unsigned long t0 = millis();

  // Controlla livello elettrico dei pin con pull-up interno prima di iniziare
  I2CBME.end();
  pinMode(sda, INPUT_PULLUP);
  pinMode(scl, INPUT_PULLUP);
  delay(3);
  int sda_lvl = digitalRead(sda);
  int scl_lvl = digitalRead(scl);

  // Impulsi di sblocco bus I2C se SDA fosse bloccato basso
  if (sda_lvl == LOW) {
    pinMode(scl, OUTPUT);
    for (int i = 0; i < 9; i++) {
      digitalWrite(scl, LOW);
      delayMicroseconds(10);
      digitalWrite(scl, HIGH);
      delayMicroseconds(10);
    }
    pinMode(scl, INPUT_PULLUP);
    delay(2);
    sda_lvl = digitalRead(sda);
    scl_lvl = digitalRead(scl);
  }

  I2CBME.begin(sda, scl, 100000);
  I2CBME.setTimeOut(10);
  delay(5);

  int count = 0;
  int nack_count = 0;
  int timeout_count = 0;
  uint8_t detected_bme_addr = 0;
  String devs_html = "";

  for (uint8_t addr = 0x08; addr <= 0x77; addr++) {
    I2CBME.beginTransmission(addr);
    uint8_t err = I2CBME.endTransmission();
    if (err == 0) {
      count++;
      // Leggi registro 0xD0 (Chip ID Bosch BME280/BMP280)
      uint8_t chip_id = 0xFF;
      I2CBME.beginTransmission(addr);
      I2CBME.write(0xD0);
      if (I2CBME.endTransmission(false) == 0 && I2CBME.requestFrom(addr, (uint8_t)1) == 1) {
        chip_id = I2CBME.read();
      }
      char hexAddr[8], hexId[8];
      snprintf(hexAddr, sizeof(hexAddr), "0x%02X", addr);
      snprintf(hexId, sizeof(hexId), "0x%02X", chip_id);

      String devName = "Dispositivo I2C generico";
      if (chip_id == 0x60) devName = "Bosch BME280 (Temp + Umidit&agrave; + Pressione)";
      else if (chip_id == 0x58 || chip_id == 0x56 || chip_id == 0x57) devName = "Bosch BMP280 (Temp + Pressione, NO Umidit&agrave;)";
      else if (chip_id == 0x61) devName = "Bosch BME680";
      else if (addr == 0x76 || addr == 0x77) devName = "Sensore su indirizzo standard 0x76/0x77";

      devs_html += "<div style='margin-top:6px;padding:6px;background:#134074;border-left:4px solid #4ade80;border-radius:4px;'>";
      devs_html += "Trovato dispositivo a indirizzo <b>" + String(hexAddr) + "</b> (dec " + String(addr) + ") &mdash; ";
      devs_html += "Chip ID (reg 0xD0): <b>" + String(hexId) + "</b> &rarr; <b>" + devName + "</b></div>";

      Serial.printf("I2C Scanner (SDA=%d, SCL=%d): found 0x%02X (ChipID=0x%02X)\n", sda, scl, addr, chip_id);
      if (addr == 0x76 || addr == 0x77 || chip_id == 0x60) {
        detected_bme_addr = addr;
      }
    } else if (err == 2 || err == 3) {
      nack_count++;
    } else {
      timeout_count++;
    }
  }

  unsigned long elapsed = millis() - t0;
  String rep = "<div style='background:#0b2545;padding:10px;border-radius:8px;margin-top:8px;font-size:14px;text-align:left;border:1px solid #3b82f6;'>";
  rep += "<div style='color:#93c5fd;margin-bottom:4px;'><b>Scansione #" + String(i2c_scan_count) + " completata</b> in <b>" + String(elapsed) + " ms</b> (Uptime: " + String(millis() / 1000) + " s)</div>";
  rep += "<div><b>Livelli elettrici pin:</b> SDA (G<b>" + String(sda) + "</b>) = " + (sda_lvl ? "<span style='color:#4ade80'>HIGH (3.3V OK)</span>" : "<span style='color:#f87171'>LOW (0V - bloccato/corto!)</span>") + " | ";
  rep += "SCL (G<b>" + String(scl) + "</b>) = " + (scl_lvl ? "<span style='color:#4ade80'>HIGH (3.3V OK)</span>" : "<span style='color:#f87171'>LOW (0V - bloccato/corto!)</span>") + "</div>";
  rep += "<div style='font-size:12px;color:#8da9c4;margin-top:2px;'>Esito bus (0x08..0x77): <b>" + String(count) + " ACK</b>, " + String(nack_count) + " NACK, " + String(timeout_count) + " Timeout/Err</div>";
  rep += devs_html;

  if (count == 0) {
    rep += "<div style='margin-top:6px;color:#fca5a5;'><b>Nessun dispositivo I2C trovato su SDA=G" + String(sda) + ", SCL=G" + String(scl) + ".</b><br>";
    rep += "Verifica i 4 fili del sensore: <b>VCC (3.3V)</b>, <b>GND</b>, <b>SDA &rarr; G" + String(sda) + "</b>, <b>SCL &rarr; G" + String(scl) + "</b>.</div>";
    bme_ok = false;
    bme_found_addr = 0;
  } else if (detected_bme_addr != 0) {
    bme_ok = bme.begin(detected_bme_addr, &I2CBME);
    if (bme_ok) {
      bme_found_addr = detected_bme_addr;
      float t = bme.readTemperature();
      float h = bme.readHumidity();
      float p = bme.readPressure() / 100.0F;
      last_pressure_hpa = p;
      for (int i = 0; i < MAX_VALUES_NUM; i++) {
        averageValueT.push(t + temperature_offset);
        averageValueH.push(h);
      }
      update_weather_and_comfort(t + temperature_offset, h, p);
      rep += "<div style='margin-top:6px;color:#4ade80;'><b>BME280 inizializzato con successo su 0x" + String(detected_bme_addr, HEX) + "!</b> ";
      rep += "Lettura diretta: T = <b>" + String(t, 2) + " &deg;C</b>, H = <b>" + String(h, 2) + " %</b>, P = <b>" + String(p, 1) + " hPa</b></div>";
    } else {
      rep += "<div style='margin-top:6px;color:#fde047;'>Dispositivo risponde su 0x" + String(detected_bme_addr, HEX) + ", ma <code>bme.begin()</code> ha restituito false (controlla se il chip ID &egrave; 0x60 = BME280 o 0x58 = BMP280).</div>";
    }
  }
  rep += "</div>";
  last_i2c_scan_html = rep;
  return rep;
}

String run_i2c_autopin_scan() {
  i2c_scan_count++;
  unsigned long t0 = millis();
  const int pairs[][2] = {
    {22, 27}, {27, 22}, // CYD Connettore P3
    {17, 16}, {16, 17}, // AZ-Touch standard
    {25, 33}, {33, 25},
    {25, 26}, {26, 25},
    {27, 26}, {26, 27},
    {32, 33}, {33, 32},
    {21, 16}, {21, 17}
  };
  const int numPairs = sizeof(pairs) / sizeof(pairs[0]);
  String rows = "";

  int found_sda = -1, found_scl = -1;

  for (int i = 0; i < numPairs; i++) {
    int sda = pairs[i][0];
    int scl = pairs[i][1];
    I2CBME.end();
    pinMode(sda, INPUT_PULLUP);
    pinMode(scl, INPUT_PULLUP);
    delay(2);
    int sda_lvl = digitalRead(sda);
    int scl_lvl = digitalRead(scl);

    String line = "SDA=G<b>" + String(sda) + "</b>, SCL=G<b>" + String(scl) + "</b>: ";
    if (!sda_lvl || !scl_lvl) {
      line += "<span style='color:#f87171'>linee basse (SDA=" + String(sda_lvl) + ", SCL=" + String(scl_lvl) + ") - saltato</span>";
      rows += "<div>" + line + "</div>";
      continue;
    }

    I2CBME.begin(sda, scl, 100000);
    I2CBME.setTimeOut(5);
    delay(2);
    bool any = false;
    for (uint8_t addr = 0x08; addr <= 0x77; addr++) {
      I2CBME.beginTransmission(addr);
      uint8_t err = I2CBME.endTransmission();
      if (err == 0) {
        char hexAddr[8];
        snprintf(hexAddr, sizeof(hexAddr), "0x%02X", addr);
        line += "<b style='color:#4ade80'>TROVATO " + String(hexAddr) + "</b> ";
        any = true;
        if (found_sda == -1) {
          found_sda = sda;
          found_scl = scl;
        }
      } else if (err != 2 && err != 3) {
        // Timeout sul bus: inutile testare gli altri 111 indirizzi su questa coppia
        line += "<span style='color:#fde047'>timeout bus</span> ";
        break;
      }
    }
    if (!any) line += "<span style='color:#8da9c4'>nessun dispositivo (pull-up OK)</span>";
    rows += "<div>" + line + "</div>";
  }

  unsigned long elapsed = millis() - t0;
  String rep = "<div style='background:#0b2545;padding:10px;border-radius:8px;margin-top:8px;font-size:13px;text-align:left;border:1px solid #f59e0b;'>";
  rep += "<div style='color:#fde047;margin-bottom:4px;'><b>Auto-Ricerca Pin #" + String(i2c_scan_count) + " completata</b> in <b>" + String(elapsed) + " ms</b> (Uptime: " + String(millis() / 1000) + " s)</div>";
  rep += rows;

  if (found_sda != -1) {
    i2c_sda_pin = found_sda;
    i2c_scl_pin = found_scl;
    save_json_config();
    rep += "<div style='margin-top:8px;color:#4ade80;'><b>Impostati automaticamente SDA=G" + String(found_sda) + ", SCL=G" + String(found_scl) + "!</b></div>";
    rep += "</div>";
    run_i2c_scan(i2c_sda_pin, i2c_scl_pin);
    last_i2c_scan_html = rep + last_i2c_scan_html;
  } else {
    // Ripristina i pin configurati
    I2CBME.end();
    I2CBME.begin(i2c_sda_pin, i2c_scl_pin, 100000);
    I2CBME.setTimeOut(10);
    rep += "<div style='margin-top:8px;color:#fca5a5;'><b>Nessun sensore I2C ha risposto su nessuna delle 12 coppie di pin testate.</b></div></div>";
    last_i2c_scan_html = rep;
  }
  return last_i2c_scan_html;
}

void update_weather_and_comfort(float t, float h, float p) {
  // 1. Punto di Rugiada (Dew Point - Formula di Magnus-Tetens)
  float rh_safe = constrain(h, 1.0f, 100.0f);
  float gamma = (17.27f * t) / (237.7f + t) + log(rh_safe / 100.0f);
  calc_dew_point = (237.7f * gamma) / (17.27f - gamma);

  // 2. Temperatura Percepita (Humidex / Steadman Apparent Temperature)
  float e_vap = (rh_safe / 100.0f) * 6.105f * exp((17.27f * t) / (237.7f + t));
  calc_heat_index = t + 0.5555f * (e_vap - 10.0f);

  // 3. Storico Pressione Barometrica (1 campione ogni 15 min, finestra 3 ore = 12 campioni)
  unsigned long now = millis();
  if (pressure_hist_count == 0 || (now - last_pressure_hist_ms >= 900000UL)) {
    last_pressure_hist_ms = now;
    pressure_history[pressure_hist_idx] = p;
    pressure_hist_idx = (pressure_hist_idx + 1) % 12;
    if (pressure_hist_count < 12) pressure_hist_count++;
  }
  int oldest_idx = (pressure_hist_count < 12) ? 0 : pressure_hist_idx;
  float oldest_p = pressure_history[oldest_idx];
  pressure_delta_3h = (oldest_p > 300.0f) ? (p - oldest_p) : 0.0f;

  // 4. Previsione Barometrica da Delta P (3h) e Pressione Assoluta
  if (pressure_delta_3h >= 1.5f) {
    weather_forecast_str = "In deciso miglioramento &#9728;&#65039; (Soleggiato)";
    weather_short_str = "Sole+";
  } else if (pressure_delta_3h >= 0.5f) {
    weather_forecast_str = "In miglioramento &#127780;&#65039; (Sereno)";
    weather_short_str = "Sereno";
  } else if (pressure_delta_3h <= -1.5f) {
    weather_forecast_str = "In rapido calo &#127783;&#65039; (Pioggia/Temporale)";
    weather_short_str = "Pioggia";
  } else if (pressure_delta_3h <= -0.5f) {
    weather_forecast_str = "In peggioramento &#9729;&#65039; (Nuvoloso)";
    weather_short_str = "Nubi";
  } else {
    if (p >= 1020.0f) {
      weather_forecast_str = "Alta pressione stabile &#9728;&#65039; (Bel tempo)";
      weather_short_str = "Sole";
    } else if (p >= 1009.0f) {
      weather_forecast_str = "Pressione stabile &#9925; (Variabile)";
      weather_short_str = "Variab";
    } else {
      weather_forecast_str = "Bassa pressione &#127783;&#65039; (Instabile/Pioggia)";
      weather_short_str = "Pioggia";
    }
  }

  // 5. Indice di Comfort Termo-Igrometrico
  gslc_tsColor col = {135, 255, 180};
  if (t >= 19.0f && t <= 25.0f && h >= 35.0f && h <= 60.0f) {
    comfort_str = "Ottimale (Benessere)";
    comfort_short_str = "Ottimo";
    comfort_color_hex = "#4ade80";
    col = {135, 255, 180};
  } else if (h > 65.0f && t >= 24.5f) {
    comfort_str = "Afoso / Caldo Umido";
    comfort_short_str = "Afoso";
    comfort_color_hex = "#fb923c";
    col = {255, 170, 80};
  } else if (h > 65.0f) {
    comfort_str = "Umido (Attenzione Condensa/Muffa)";
    comfort_short_str = "Umido";
    comfort_color_hex = "#facc15";
    col = {255, 220, 80};
  } else if (h < 35.0f) {
    comfort_str = "Aria Secca (<35%)";
    comfort_short_str = "Secco";
    comfort_color_hex = "#38bdf8";
    col = {135, 211, 255};
  } else if (t < 19.0f) {
    comfort_str = "Ambiente Fresco";
    comfort_short_str = "Fresco";
    comfort_color_hex = "#60a5fa";
    col = {135, 211, 255};
  } else {
    comfort_str = "Ambiente Caldo";
    comfort_short_str = "Caldo";
    comfort_color_hex = "#f87171";
    col = {255, 150, 120};
  }

  // 6. Aggiorna stringa compatta sul display TFT nel riquadro INDOOR
  if (m_pElemOutTxt_Weather != NULL) {
    char wbuf[28];
    snprintf(wbuf, sizeof(wbuf), "%.0fhPa %s | %s", p, weather_short_str.c_str(), comfort_short_str.c_str());
    gslc_ElemSetTxtCol(&m_gui, m_pElemOutTxt_Weather, col);
    gslc_ElemSetTxtStr(&m_gui, m_pElemOutTxt_Weather, wbuf);
  }
}

void get_temp_print(){
  char buffer[12];

  if (!bme_ok) {
    bme_ok = bme.begin(0x76, &I2CBME);
    if (!bme_ok) bme_ok = bme.begin(0x77, &I2CBME);
    if (bme_ok) {
      Serial.println("BME280 reconnected!");
      for (int i = 0; i < MAX_VALUES_NUM; i++) {
        averageValueT.push(bme.readTemperature());
        averageValueH.push(bme.readHumidity());
      }
    } else {
      gslc_ElemSetTxtStr(&m_gui,m_pElemOutTxt5,"--.-");
      gslc_ElemSetTxtStr(&m_gui,m_pElemOutTxt6,"--.-");
      if (m_pElemOutTxt_Weather != NULL) gslc_ElemSetTxtStr(&m_gui,m_pElemOutTxt_Weather,"Sensore non rilevato");
      return;
    }
  }

  gslc_ElemXCheckboxSetState(&m_gui,m_pElemRB_sample,true);
  gslc_Update(&m_gui);

  float rawT = bme.readTemperature();
  float rawH = bme.readHumidity();
  float rawP = bme.readPressure() / 100.0F;

  if (!isnan(rawT) && rawT > -40.0f && rawT < 85.0f) {
    averageValueT.push(rawT + temperature_offset);
    dtostrf(averageValueT.average(), 4, 1, buffer);
    gslc_ElemSetTxtStr(&m_gui,m_pElemOutTxt5,buffer);

    if (client.connected()) {
      char myConcatenation[80];
      snprintf(myConcatenation, sizeof(myConcatenation), "%stemperature/get", thermostat_id);
      client.publish(myConcatenation, buffer, true, 0);
    }
  }

  if (!isnan(rawH) && rawH >= 0.0f && rawH <= 100.0f) {
    averageValueH.push(rawH);
    dtostrf(averageValueH.average(), 4, 1, buffer);
    gslc_ElemSetTxtStr(&m_gui,m_pElemOutTxt6,buffer);

    if (client.connected()) {
      char myConcatenation[80];
      snprintf(myConcatenation, sizeof(myConcatenation), "%shumidity/get", thermostat_id);
      client.publish(myConcatenation, buffer, true, 0);
    }
  }

  if (!isnan(rawP) && rawP >= 300.0f && rawP <= 1200.0f) {
    last_pressure_hpa = rawP;
    update_weather_and_comfort(averageValueT.average(), averageValueH.average(), last_pressure_hpa);
    if (client.connected()) {
      char myConcatenation[80];
      dtostrf(last_pressure_hpa, 0, 1, buffer);
      snprintf(myConcatenation, sizeof(myConcatenation), "%spressure/get", thermostat_id);
      client.publish(myConcatenation, buffer, true, 0);

      dtostrf(calc_dew_point, 0, 1, buffer);
      snprintf(myConcatenation, sizeof(myConcatenation), "%sdewpoint/get", thermostat_id);
      client.publish(myConcatenation, buffer, true, 0);

      String comfPayload = comfort_short_str + " (" + weather_short_str + ")";
      snprintf(myConcatenation, sizeof(myConcatenation), "%scomfort/get", thermostat_id);
      client.publish(myConcatenation, comfPayload.c_str(), true, 0);
    }
  }

  gslc_ElemXCheckboxSetState(&m_gui,m_pElemRB_sample,false); 
  gslc_Update(&m_gui); 
}


void fill_buffer(){
  Serial.println("Fill buffer");
  if (!bme_ok) {
    get_temp_print();
    return;
  }
  for (int i=0; i < MAX_VALUES_NUM; i++){
    averageValueT.push(bme.readTemperature());
    averageValueH.push(bme.readHumidity());
    delay(50);
  }
  get_temp_print();
}


void update_wifi_signal_bars() {
  // Disegna l'icona con le 4 tacche del Wi-Fi in alto a destra sullo schermo TFT (vicino al tasto MENU)
  // Posizione: X=244, Y=14 (Larghezza: 18px, Altezza: 12px)
  if (gslc_GetPageCur(&m_gui) != E_PG_MAIN) return;

  int rssi = (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : -100;
  int bars = 0;
  gslc_tsColor barCol = {0, 150, 255}; // Blu acceso standard

  if (WiFi.status() == WL_CONNECTED) {
    if (rssi >= -60) {
      bars = 4;
      barCol = {74, 222, 128}; // Verde brillante (Eccellente)
    } else if (rssi >= -70) {
      bars = 3;
      barCol = {135, 255, 180}; // Verde chiaro (Buono)
    } else if (rssi >= -80) {
      bars = 2;
      barCol = {253, 224, 71}; // Giallo (Sufficiente)
    } else {
      bars = 1;
      barCol = {248, 113, 113}; // Rosso (Debole)
    }
  } else if (ap_mode_active) {
    bars = 4;
    barCol = {56, 189, 248}; // Azzurro AP
  }

  // Pulisce l'area di disegno del segnale Wi-Fi (background scuro della barra superiore)
  gslc_tsColor bgCol = {0, 65, 128};
  gslc_tsColor offCol = {0, 45, 90}; // Tacca inattiva / spenta
  gslc_DrawFillRect(&m_gui, (gslc_tsRect){243, 6, 18, 13}, bgCol);

  // Disegna 4 tacche crescenti
  // Tacca 1 (H=3):  X=244
  // Tacca 2 (H=6):  X=248
  // Tacca 3 (H=9):  X=252
  // Tacca 4 (H=12): X=256
  int heights[4] = {3, 6, 9, 12};
  for (int b = 0; b < 4; b++) {
    int bx = 244 + b * 4;
    int by = 18 - heights[b];
    gslc_tsColor c = (b < bars) ? barCol : offCol;
    gslc_DrawFillRect(&m_gui, (gslc_tsRect){(int16_t)bx, (int16_t)by, 3, (uint16_t)heights[b]}, c);
  }
}

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo, 50)){
    if (ap_mode_active && WiFi.status() != WL_CONNECTED) {
      gslc_ElemSetTxtStr(&m_gui,m_pElemOutTxt_Date,"Termostato_AP");
    }
    update_wifi_signal_bars();
    return;
  }

  static const char* const giorni[] = {"Dom", "Lun", "Mar", "Mer", "Gio", "Ven", "Sab"};
  static const char* const mesi[]   = {"Gen", "Feb", "Mar", "Apr", "Mag", "Giu", "Lug", "Ago", "Set", "Ott", "Nov", "Dic"};
  char timeStringBuff[20];
  snprintf(timeStringBuff, sizeof(timeStringBuff), "%s %02d %s",
           giorni[timeinfo.tm_wday % 7], timeinfo.tm_mday, mesi[timeinfo.tm_mon % 12]);
  gslc_ElemSetTxtStr(&m_gui,m_pElemOutTxt_Date,timeStringBuff);
  strftime(timeStringBuff, sizeof(timeStringBuff), "%H:%M", &timeinfo);
  gslc_ElemSetTxtStr(&m_gui,m_pElemOutTxt_Time,timeStringBuff);
  evaluate_crono(false);
  update_wifi_signal_bars();
}

void start_ap_portal() {
  if (!ap_mode_active) {
    WiFi.disconnect(false);
    delay(50);
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP("Termostato_AP");
    dnsServer.start(53, "*", WiFi.softAPIP());
    ap_mode_active = true;
    Serial.print("AP Mode started: SSID=Termostato_AP, IP=");
    Serial.println(WiFi.softAPIP());
  }
  gslc_ElemSetTxtStr(&m_gui, m_pElemOutTxt_Ip_Address16, "AP:192.168.4.1");
  if (WiFi.status() != WL_CONNECTED) {
    gslc_ElemSetTxtStr(&m_gui, m_pElemOutTxt_Date, "Termostato_AP");
  }
  gslc_Update(&m_gui);
}

void play_beep(int freq, int duration_ms) {
  if (!buzzer_enabled || buzzer_vol == 0) return;
  if (i2c_sda_pin == BUZZER_PIN || i2c_scl_pin == BUZZER_PIN) return;

  // Regola frequenza, duty cycle PWM (su 10 bit = 0..1023) e durata in base al volume scelto:
  // Vol 1 (Basso/Default): tick leggerissimo (duty 2/1023 = 0.2%, freq dimezzata lontano dalla risonanza, 3 ms)
  // Vol 2 (Medio):         bip morbido       (duty 10/1023 = 1.0%, 7 ms)
  // Vol 3 (Alto):          bip normale       (duty 80/1023 = 7.8%, 15 ms)
  int eff_freq = freq;
  uint32_t duty = 2;
  int eff_dur = 3;

  if (buzzer_vol == 1) {
    eff_freq = constrain(freq / 2, 900, 1500);
    duty     = 2;
    eff_dur  = 3;
  } else if (buzzer_vol == 2) {
    eff_freq = constrain(freq - 600, 1400, 2200);
    duty     = 10;
    eff_dur  = constrain(duration_ms / 3, 5, 9);
  } else {
    eff_freq = freq;
    duty     = 80;
    eff_dur  = constrain(duration_ms / 2, 10, 18);
  }

  // Canale LEDC 2 usa Timer 1 (separato dal Timer 0 usato dal canale 0 della retroilluminazione TFT)
  ledcSetup(2, eff_freq, 10);
  ledcAttachPin(BUZZER_PIN, 2);
  ledcWrite(2, duty);
  delay(eff_dur);
  ledcWrite(2, 0);
  ledcDetachPin(BUZZER_PIN);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
}

void track_open_window(float current_t) {
  if (!open_window_enabled) {
    open_window_active = false;
    return;
  }
  unsigned long now = millis();
  // Se già in sospensione per finestra aperta
  if (open_window_active) {
    if (now >= open_window_until_ms) {
      open_window_active = false;
      Serial.println("Finestra aperta: timeout terminato, ripristino termostato.");
      if (m_pElemOutTxt_SetpointLbl != NULL) {
        gslc_ElemSetTxtStr(&m_gui, m_pElemOutTxt_SetpointLbl, "SETPOINT");
      }
      publish_ha_state();
    }
    return;
  }

  // Monitora la variazione solo in modalità Caldo
  if (stato != 1) {
    return;
  }

  // 1 campione ogni 30 secondi
  if (last_temp_track_ms == 0 || (now - last_temp_track_ms >= 30000UL)) {
    last_temp_track_ms = now;
    open_window_temp_hist[open_window_hist_idx] = current_t;
    open_window_hist_idx = (open_window_hist_idx + 1) % 10;
    if (open_window_hist_count < 10) open_window_hist_count++;

    if (open_window_hist_count >= 6) { // Almeno 3 minuti di campioni
      int oldest_idx = (open_window_hist_count < 10) ? 0 : open_window_hist_idx;
      float oldest_t = open_window_temp_hist[oldest_idx];
      float temp_drop = oldest_t - current_t;
      if (temp_drop >= 1.5f) {
        // Crollo rapido di >= 1.5 °C rilevato! Attiva sospensione per 20 minuti (1200000 ms)
        open_window_active = true;
        open_window_until_ms = now + 1200000UL;
        Serial.printf("ALLERTA: Finestra aperta rilevata! Crollo di %.1f C in %d min. Sospensione riscaldamento per 20 min.\n",
                      temp_drop, (open_window_hist_count * 30) / 60);
        if (m_pElemOutTxt_SetpointLbl != NULL) {
          gslc_ElemSetTxtStr(&m_gui, m_pElemOutTxt_SetpointLbl, "FINESTRA AP");
        }
        play_beep(1800, 80);
        publish_ha_state();
      }
    }
  }
}

void update_runtime_counter() {
  unsigned long now = millis();
  if (last_runtime_check_ms == 0) {
    last_runtime_check_ms = now;
    return;
  }

  unsigned long elapsed_sec = (now - last_runtime_check_ms) / 1000UL;
  if (elapsed_sec >= 1) {
    last_runtime_check_ms = now;
    // Se il relè fisico è acceso, aggiungi i secondi
    if (digitalRead(rele_pin) == HIGH) {
      runtime_today_sec += elapsed_sec;
    }
  }

  // Controlla cambio giorno tramite struct tm NTP
  struct tm timeinfo;
  if (getLocalTime(&timeinfo, 10)) {
    if (current_day_of_year == -1) {
      current_day_of_year = timeinfo.tm_yday;
    } else if (current_day_of_year != timeinfo.tm_yday) {
      // Cambio di giorno a mezzanotte! Fai scorrere lo storico 7 giorni
      current_day_of_year = timeinfo.tm_yday;
      for (int i = 6; i > 0; i--) {
        runtime_history_7d[i] = runtime_history_7d[i - 1];
      }
      runtime_history_7d[0] = runtime_today_sec;
      runtime_today_sec = 0;
      save_json_config();
      Serial.println("Nuovo giorno: aggiornato storico ore caldaia 7gg");
    }
  }

  // Salva periodicamente lo storico su flash ogni 10 minuti se il relè è stato acceso
  if (now - last_runtime_save_ms >= 600000UL) {
    last_runtime_save_ms = now;
    save_json_config();
  }
}

void evaluate_crono(bool force_apply) {
  if (!crono_enabled) {
    last_crono_key = -1;
    current_crono_slot = -1;
    if (current_preset_mode == "away") {
      current_crono_desc = "Modo Eco (Assente)";
      if (m_pElemOutTxt_SetpointLbl != NULL) gslc_ElemSetTxtStr(&m_gui, m_pElemOutTxt_SetpointLbl, "MODO ECO");
    } else if (current_preset_mode == "boost") {
      current_crono_desc = "Modo Boost (Rapido)";
      if (m_pElemOutTxt_SetpointLbl != NULL) gslc_ElemSetTxtStr(&m_gui, m_pElemOutTxt_SetpointLbl, "MODO BOOST");
    } else {
      current_preset_mode = "none";
      current_crono_desc = "Manuale";
      if (m_pElemOutTxt_SetpointLbl != NULL) gslc_ElemSetTxtStr(&m_gui, m_pElemOutTxt_SetpointLbl, "SETPOINT");
    }
    return;
  }

  current_preset_mode = "schedule";
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 20)) {
    current_crono_desc = "In attesa sincronizzazione NTP";
    if (m_pElemOutTxt_SetpointLbl != NULL) gslc_ElemSetTxtStr(&m_gui, m_pElemOutTxt_SetpointLbl, "CRONO NTP");
    return;
  }

  int wday = timeinfo.tm_wday; // 0 = Dom, 1..5 = Lun..Ven, 6 = Sab
  int day_type = (wday == 0 || wday == 6) ? 1 : 0;
  int now_min = timeinfo.tm_hour * 60 + timeinfo.tm_min;

  int active_slot = -1;
  for (int s = 0; s < 4; s++) {
    if (now_min >= (int)crono_sched[day_type][s].start_min) {
      active_slot = s;
    }
  }
  if (active_slot == -1) {
    active_slot = 3; // Prima della fascia Mattina resta attiva la fascia Notte
  }

  current_crono_slot = active_slot;
  uint16_t sm = crono_sched[day_type][active_slot].start_min;
  float st = crono_sched[day_type][active_slot].temp;
  char descBuf[56];
  snprintf(descBuf, sizeof(descBuf), "%s %s (%02d:%02d -> %.1f C)",
           day_type ? "Weekend" : "Feriali",
           crono_slot_names[active_slot],
           sm / 60, sm % 60, st);
  current_crono_desc = String(descBuf);

  if (m_pElemOutTxt_SetpointLbl != NULL) {
    gslc_ElemSetTxtStr(&m_gui, m_pElemOutTxt_SetpointLbl, crono_slot_short[active_slot]);
  }

  int key = day_type * 10 + active_slot;
  if (force_apply || key != last_crono_key) {
    last_crono_key = key;
    temp_setpoint = round(st * 10.0f) / 10.0f;
    char buf[10];
    dtostrf(temp_setpoint, 0, 1, buf);
    gslc_ElemSetTxtStr(&m_gui, m_pElemOutTxt_Temp_Setpoint, buf);
    save_state_eeprom();
    publish_ha_state();
  }
}

void save_state_eeprom() {
  EEPROM.write(EEPROM_MAGIC_ADDR, EEPROM_MAGIC_VAL);
  EEPROM.write(EEPROM_MODE_ADDR, (uint8_t)stato);
  EEPROM.writeFloat(EEPROM_SP_ADDR, temp_setpoint);
  EEPROM.write(EEPROM_BUZZER_ADDR, buzzer_enabled ? buzzer_vol : 0);
  EEPROM.write(EEPROM_CRONO_ADDR, crono_enabled ? 1 : 0);
  EEPROM.commit();
}

void update_cyd_leds() {
#if defined(HARDWARE_CYD)
  if (stato == 1) {
    // Riscaldamento: LED Rosso acceso (active LOW)
    digitalWrite(CYD_LED_RED, LOW);
    digitalWrite(CYD_LED_GREEN, HIGH);
    digitalWrite(CYD_LED_BLUE, HIGH);
  } else if (stato == 2) {
    // Raffrescamento: LED Blu acceso (active LOW)
    digitalWrite(CYD_LED_RED, HIGH);
    digitalWrite(CYD_LED_GREEN, HIGH);
    digitalWrite(CYD_LED_BLUE, LOW);
  } else {
    // Spento: tutti i LED RGB spenti
    digitalWrite(CYD_LED_RED, HIGH);
    digitalWrite(CYD_LED_GREEN, HIGH);
    digitalWrite(CYD_LED_BLUE, HIGH);
  }
#endif
}

void set_thermostat_mode(int new_mode) {
  char myConc[80];
  snprintf(myConc, sizeof(myConc), "%smode/get", thermostat_id);
  if (new_mode == 1) {
    stato = 1;
    stato_shadow = 1;
    gslc_ElemSetTxtStr(&m_gui,m_pElemOutTxt_status,"Caldo");
    gslc_ElemSetTxtCol(&m_gui,m_pElemOutTxt_status,{255,0,0});
    if (client.connected()) client.publish(myConc,"heat",true,0);
  } else if (new_mode == 2) {
    stato = 2;
    stato_shadow = 2;
    gslc_ElemSetTxtStr(&m_gui,m_pElemOutTxt_status,"Freddo");
    gslc_ElemSetTxtCol(&m_gui,m_pElemOutTxt_status,{0,150,255});
    if (client.connected()) client.publish(myConc,"cool",true,0);
  } else {
    stato_shadow = stato;
    stato = 0;
    gslc_ElemSetTxtStr(&m_gui,m_pElemOutTxt_status,"Spento");
    gslc_ElemSetTxtCol(&m_gui,m_pElemOutTxt_status,{135,135,135});
    if (client.connected()) client.publish(myConc,"off",true,0);
  }
  save_state_eeprom();
  update_cyd_leds();
  publish_ha_state();
  gslc_Update(&m_gui);
}

void handleRoot() {
  if (webServer.hasArg("hadisc")) {
    publish_ha_discovery();
  }
  if (webServer.hasArg("i2cscan")) {
    run_i2c_scan(i2c_sda_pin, i2c_scl_pin);
  } else if (webServer.hasArg("i2cpinscan")) {
    run_i2c_autopin_scan();
  } else if (last_i2c_scan_html.length() == 0) {
    run_i2c_scan(i2c_sda_pin, i2c_scl_pin);
  }

  bool doScan = webServer.hasArg("scan");
  int16_t nNets = -1;
  if (doScan) {
    Serial.println("Avvio scansione reti WiFi...");
    if (WiFi.getMode() != WIFI_AP_STA && WiFi.getMode() != WIFI_STA) {
      WiFi.mode(WIFI_AP_STA);
      delay(50);
    }
    if (WiFi.status() != WL_CONNECTED) {
      WiFi.disconnect(false);
      delay(80);
    }
    WiFi.scanDelete();
    delay(40);
    nNets = WiFi.scanNetworks(false, true);
    if (nNets == WIFI_SCAN_FAILED) {
      Serial.println("Scansione fallita, secondo tentativo...");
      delay(150);
      WiFi.disconnect(false);
      delay(50);
      nNets = WiFi.scanNetworks(false, true);
    }
    Serial.printf("Scansione reti WiFi terminata: trovate %d reti\n", nNets);
  }

  String zoneName = get_zone_name();
  String zoneSlug = get_zone_slug();

  String html = "<!DOCTYPE html><html lang='it'><head><meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
  html += "<title>Termostato " + zoneName + "</title>";
  html += "<style>";
  html += "body{font-family:system-ui,-apple-system,sans-serif;background:#0b2545;color:#eef4ed;margin:0;padding:16px;}";
  html += ".card{background:#134074;border-radius:12px;padding:16px;margin-bottom:16px;box-shadow:0 4px 12px rgba(0,0,0,0.3);}";
  html += "details.card summary{cursor:pointer;list-style:none;user-select:none;display:flex;align-items:center;justify-content:space-between;}";
  html += "details.card summary::-webkit-details-marker{display:none;}";
  html += "details.card summary h3{margin:0;display:inline-flex;align-items:center;gap:6px;color:#8da9c4;font-size:16px;}";
  html += "details.card summary:hover h3{color:#fff;}";
  html += "details.card summary::after{content:'';display:inline-block;width:8px;height:8px;border-right:2px solid #8da9c4;border-bottom:2px solid #8da9c4;transform:rotate(-45deg);transition:transform 0.2s ease;margin-right:6px;}";
  html += "details.card[open] summary::after{transform:rotate(45deg);}";
  html += "details.card[open]>*:not(summary){margin-top:14px;}";
  html += "h2,h3{margin-top:0;color:#8da9c4;}";
  html += ".grid{display:grid;grid-template-columns:repeat(2,1fr);gap:12px;text-align:center;}";
  html += ".val{font-size:28px;font-weight:bold;color:#fff;}";
  html += ".lbl{font-size:12px;color:#8da9c4;text-transform:uppercase;}";
  html += "input,select,button{width:100%;padding:10px;margin:6px 0;border-radius:8px;border:none;box-sizing:border-box;font-size:15px;}";
  html += "button,.btn{background:#0088ff;color:#fff;font-weight:bold;cursor:pointer;display:inline-block;text-align:center;text-decoration:none;}";
  html += ".btn-row{display:flex;gap:8px;margin-top:8px;}";
  html += ".btn-heat{background:#e63946;} .btn-cool{background:#0096c7;} .btn-off{background:#6c757d;}";
  html += ".net-item{background:#0b2545;padding:8px 12px;margin:4px 0;border-radius:6px;cursor:pointer;display:flex;justify-content:space-between;}";
  html += ".pills{display:flex;flex-wrap:wrap;gap:6px;margin:4px 0 10px 0;}";
  html += ".pill{background:#0b2545;font-size:12px;padding:6px 12px;border-radius:15px;cursor:pointer;border:1px solid #334155;color:#93c5fd;font-weight:600;}";
  html += ".pill:hover{background:#1e40af;color:#fff;}";
  html += "</style></head><body>";

  html += "<h2>Termostato Smart &mdash; " + zoneName + "</h2>";

  // Card 1: Stato in tempo reale e controlli
  html += "<div class='card'><h3>Stato Termostato</h3><div class='grid'>";
  html += "<div><div class='lbl'>Temperatura</div><div class='val'>" + (bme_ok ? String(averageValueT.average(), 1) : String("--.-")) + " &deg;C</div></div>";
  html += "<div><div class='lbl'>Umidit&agrave;</div><div class='val'>" + (bme_ok ? String(averageValueH.average(), 1) : String("--.-")) + " %</div></div>";
  html += "<div><div class='lbl'>Setpoint</div><div class='val'>" + String(temp_setpoint, 1) + " &deg;C</div></div>";
  String modeStr = (stato == 1) ? "Caldo" : ((stato == 2) ? "Freddo" : "Spento");
  html += "<div><div class='lbl'>Modalit&agrave; / Rel&egrave;</div><div class='val'>" + modeStr + " (" + (digitalRead(rele_pin) ? "ON" : "OFF") + ")</div></div>";
  html += "</div>";
  html += "<div class='btn-row'>";
  html += "<a class='btn' style='padding:10px;flex:1' href='/ctrl?sp=-0.5'>-0.5 &deg;C</a>";
  html += "<a class='btn' style='padding:10px;flex:1' href='/ctrl?sp=-0.1'>-0.1 &deg;C</a>";
  html += "<a class='btn' style='padding:10px;flex:1' href='/ctrl?sp=0.1'>+0.1 &deg;C</a>";
  html += "<a class='btn' style='padding:10px;flex:1' href='/ctrl?sp=0.5'>+0.5 &deg;C</a>";
  html += "</div>";
  html += "<div class='btn-row'>";
  html += "<a class='btn btn-off' style='padding:10px;flex:1' href='/ctrl?mode=0'>Spento</a>";
  html += "<a class='btn btn-heat' style='padding:10px;flex:1' href='/ctrl?mode=1'>Caldo</a>";
  html += "<a class='btn btn-cool' style='padding:10px;flex:1' href='/ctrl?mode=2'>Freddo</a>";
  html += "</div>";
  String buzStatus = "<span style='color:#f87171'>MUTO &#128263;</span>";
  if (buzzer_enabled && buzzer_vol == 1) buzStatus = "<span style='color:#4ade80'>BASSO (Tick) &#128264;</span>";
  else if (buzzer_enabled && buzzer_vol == 2) buzStatus = "<span style='color:#38bdf8'>MEDIO &#128265;</span>";
  else if (buzzer_enabled && buzzer_vol >= 3) buzStatus = "<span style='color:#fde047'>ALTO &#128266;</span>";

  html += "<div style='margin-top:10px;padding:8px 12px;background:#0b2545;border-radius:8px;font-size:13px;display:flex;justify-content:space-between;align-items:center;flex-wrap:wrap;gap:6px;'>";
  html += "<span><b>Fascia / Preset:</b> <span style='color:#38bdf8;'>" + current_crono_desc + "</span></span>";
  html += "<span><b>Buzzer Touch:</b> " + buzStatus + "</span>";
  html += "</div>";
  html += "<div class='btn-row'>";
  if (crono_enabled) {
    html += "<a class='btn' style='padding:9px;flex:1;background:#16a34a;font-size:13px;' href='/ctrl?crono=0'>Crono: ON (Disattiva)</a>";
  } else {
    html += "<a class='btn' style='padding:9px;flex:1;background:#475569;font-size:13px;' href='/ctrl?crono=1'>Crono: OFF (Attiva)</a>";
  }
  html += "<a class='btn' style='padding:9px;flex:1;background:#0284c7;font-size:13px;' href='/ctrl?preset=away'>Eco (18&deg;C)</a>";
  html += "<a class='btn' style='padding:9px;flex:1;background:#ea580c;font-size:13px;' href='/ctrl?preset=boost'>Boost (23&deg;C)</a>";
  html += "</div>";
  html += "<div class='btn-row'>";
  html += "<a class='btn' style='padding:7px;flex:1;font-size:12px;background:" + String(!buzzer_enabled ? "#dc2626" : "#334155") + ";' href='/ctrl?buzzer=0'>Buzzer Muto</a>";
  html += "<a class='btn' style='padding:7px;flex:1;font-size:12px;background:" + String((buzzer_enabled && buzzer_vol==1) ? "#16a34a" : "#334155") + ";' href='/ctrl?buzzer=1'>Vol. Basso (Tick)</a>";
  html += "<a class='btn' style='padding:7px;flex:1;font-size:12px;background:" + String((buzzer_enabled && buzzer_vol==2) ? "#0284c7" : "#334155") + ";' href='/ctrl?buzzer=2'>Vol. Medio</a>";
  html += "<a class='btn' style='padding:7px;flex:1;font-size:12px;background:" + String((buzzer_enabled && buzzer_vol>=3) ? "#d97706" : "#334155") + ";' href='/ctrl?buzzer=3'>Vol. Alto</a>";
  html += "</div></div>";

  // Card 1-Crono: Programmazione Oraria Giornaliera / Settimanale (Cronotermostato)
  html += "<details class='card' id='crono-box'><summary><h3>&#128197; Programmazione Oraria Settimanale (Cronotermostato)</h3></summary>";
  html += "<p style='margin:4px 0 10px 0;font-size:13px;color:#93c5fd;'>Imposta le 4 fasce orarie giornaliere per <b>Giorni Feriali (Lun&ndash;Ven)</b> e <b>Weekend (Sab&ndash;Dom)</b>. Collegato con Home Assistant (Preset <code>schedule</code>).</p>";
  html += "<form method='POST' action='/crono'>";
  for (int d = 0; d < 2; d++) {
    html += "<div style='background:#0b2545;padding:10px;border-radius:8px;margin-bottom:10px;'>";
    html += "<div style='font-weight:bold;color:#38bdf8;margin-bottom:8px;font-size:14px;'>" + String(d == 0 ? "&#128197; Giorni Feriali (Luned&igrave; &ndash; Venerd&igrave;)" : "&#127969; Weekend (Sabato &ndash; Domenica)") + "</div>";
    html += "<div class='grid'>";
    for (int s = 0; s < 4; s++) {
      char tbuf[8];
      uint16_t sm = crono_sched[d][s].start_min;
      snprintf(tbuf, sizeof(tbuf), "%02u:%02u", (unsigned)(sm / 60), (unsigned)(sm % 60));
      String idx = String(d) + "_" + String(s);
      html += "<div style='background:#134074;padding:8px;border-radius:6px;text-align:left;'>";
      html += "<div class='lbl' style='color:#fde047;margin-bottom:4px;'>" + String(s + 1) + ". " + String(crono_slot_names[s]) + "</div>";
      html += "<div style='display:flex;gap:6px;align-items:center;'>";
      html += "<input type='time' id='t" + idx + "' name='t" + idx + "' value='" + String(tbuf) + "' style='flex:1.2;padding:6px;margin:2px 0;font-size:14px;'>";
      html += "<input type='number' step='0.1' min='10' max='35' id='sp" + idx + "' name='sp" + idx + "' value='" + String(crono_sched[d][s].temp, 1) + "' style='flex:1;padding:6px;margin:2px 0;font-size:14px;'>";
      html += "<span style='font-size:13px;'>&deg;C</span></div></div>";
    }
    html += "</div></div>";
  }
  html += "<div class='btn-row'>";
  html += "<button type='button' style='background:#475569;flex:1;font-size:13px;' onclick=\"for(var s=0;s<4;s++){document.getElementById('t1_'+s).value=document.getElementById('t0_'+s).value;document.getElementById('sp1_'+s).value=document.getElementById('sp0_'+s).value;}\">Copia Feriali su Weekend</button>";
  html += "<button type='submit' style='background:#16a34a;flex:1.5;'>Salva Programmazione &amp; Attiva Crono</button>";
  html += "</div></form></details>";

  // Card 1-Smart: Protezione Antigelo, Finestra Aperta e Ore di Accensione Caldaia
  html += "<details class='card' id='smart-box'><summary><h3>&#128293; Sicurezza &amp; Consumi Caldaia</h3></summary>";
  uint32_t t_m = runtime_today_sec / 60;
  uint32_t t_h = t_m / 60;
  t_m = t_m % 60;
  uint32_t tot_7d_sec = runtime_today_sec;
  for (int i = 0; i < 7; i++) tot_7d_sec += runtime_history_7d[i];
  uint32_t tot_7d_h = (tot_7d_sec / 60) / 60;
  uint32_t tot_7d_m = (tot_7d_sec / 60) % 60;

  html += "<div class='grid'>";
  html += "<div><div class='lbl'>Accensione Oggi</div><div class='val' style='font-size:24px;color:#38bdf8;'>" + String(t_h) + "h " + String(t_m) + "m</div></div>";
  html += "<div><div class='lbl'>Accensione 7 Giorni</div><div class='val' style='font-size:24px;color:#a78bfa;'>" + String(tot_7d_h) + "h " + String(tot_7d_m) + "m</div></div>";
  html += "</div>";

  // Storico dettagliato ultimi 7 giorni
  html += "<div style='background:#0b2545;padding:8px 12px;border-radius:8px;margin-top:10px;font-size:13px;'>";
  html += "<div style='color:#93c5fd;font-weight:bold;margin-bottom:4px;'>Storico Giornaliero Accensione:</div><div style='display:flex;justify-content:space-between;flex-wrap:wrap;gap:4px;'>";
  for (int i = 0; i < 7; i++) {
    uint32_t dm = runtime_history_7d[i] / 60;
    html += "<div style='background:#134074;padding:4px 8px;border-radius:4px;text-align:center;'>";
    html += "<span style='color:#8da9c4;font-size:11px;'>-" + String(i + 1) + "gg</span><br><b>" + String(dm / 60) + "h " + String(dm % 60) + "m</b></div>";
  }
  html += "</div></div>";

  // Indicatori di stato Antigelo e Finestra Aperta
  String antiStr = antifrost_enabled ? (antifrost_active ? "<span style='color:#f87171'><b>IN ALLARME (Acceso!)</b></span>" : "<span style='color:#4ade80'>Attivo (OK)</span>") : "<span style='color:#94a3b8'>Disabilitato</span>";
  String winStr = (open_window_enabled || ext_window_enabled) ? ((open_window_active || ext_window_open) ? "<span style='color:#facc15'><b>APERTA (Pausa!)</b></span>" : "<span style='color:#4ade80'>Attivo (OK)</span>") : "<span style='color:#94a3b8'>Disabilitato</span>";

  html += "<div style='margin-top:10px;display:flex;gap:8px;flex-wrap:wrap;'>";
  html += "<div style='flex:1;background:#0b2545;padding:8px;border-radius:6px;font-size:13px;'>";
  html += "<b>Antigelo (Soglia " + String(antifrost_temp_threshold, 1) + "&deg;C):</b> " + antiStr + "<br>";
  html += "<a class='btn' style='display:inline-block;padding:4px 8px;font-size:11px;margin-top:4px;background:#334155;' href='/ctrl?antifrost=" + String(antifrost_enabled ? 0 : 1) + "'>" + (antifrost_enabled ? "Disabilita" : "Abilita") + "</a>";
  html += "</div>";

  html += "<div style='flex:1;background:#0b2545;padding:8px;border-radius:6px;font-size:13px;'>";
  html += "<b>Rilevamento Finestra:</b> " + winStr + "<br>";
  html += "<a class='btn' style='display:inline-block;padding:4px 8px;font-size:11px;margin-top:4px;background:#334155;' href='/ctrl?openwin=" + String(open_window_enabled ? 0 : 1) + "'>" + (open_window_enabled ? "Disabilita" : "Abilita") + "</a>";
  html += "</div></div></details>";

  // Card 2A: Funzionalità Avanzate & Protezioni Impianto
  html += "<details class='card' id='adv-box'><summary><h3>&#9881; Funzionalit&agrave; Avanzate &amp; Protezioni Impianto</h3></summary>";
  html += "<p style='margin:4px 0 12px 0;font-size:13px;color:#93c5fd;'>Configura le protezioni intelligenti, isteresi dinamica, modalit&agrave; notte, blocco pulizia e sensori esterni.</p>";

  // 1. Isteresi
  String hystStatus = hysteresis_enabled ? ("<span style='color:#4ade80'>ATTIVA (&plusmn;" + String(hysteresis_val, 1) + " &deg;C)</span>") : "<span style='color:#94a3b8'>DISABILITATA (0.05 &deg;C)</span>";
  html += "<div style='background:#0b2545;padding:10px;border-radius:8px;margin-bottom:10px;'>";
  html += "<div style='display:flex;justify-content:space-between;align-items:center;'>";
  html += "<b>1. Isteresi di Commutazione:</b> " + hystStatus;
  html += "<a class='btn' style='padding:5px 10px;font-size:12px;background:" + String(hysteresis_enabled ? "#dc2626" : "#16a34a") + ";' href='/ctrl?hyst_en=" + String(hysteresis_enabled ? 0 : 1) + "#adv-box'>" + (hysteresis_enabled ? "Disabilita" : "Abilita") + "</a>";
  html += "</div>";
  html += "<p style='font-size:12px;color:#8da9c4;margin:4px 0 8px 0;'>Evita accensioni e spegnimenti continui della caldaia attorno alla temperatura di setpoint.</p>";
  html += "<div style='display:flex;gap:6px;align-items:center;flex-wrap:wrap;'>";
  html += "<span style='font-size:12px;color:#cbd5e1;'>Valore Rapido:</span>";
  float hvals[] = {0.1f, 0.2f, 0.3f, 0.5f, 1.0f};
  for (int hi = 0; hi < 5; hi++) {
    bool isSel = (hysteresis_enabled && fabs(hysteresis_val - hvals[hi]) < 0.05f);
    html += "<a class='btn' style='padding:4px 8px;font-size:12px;background:" + String(isSel ? "#0284c7" : "#334155") + ";' href='/ctrl?hyst_val=" + String(hvals[hi], 1) + "#adv-box'>&plusmn;" + String(hvals[hi], 1) + " &deg;C</a>";
  }
  html += "</div></div>";

  // 2. Protezione Antiblocco Pompa
  String pumpStatus = pump_protection_enabled ? "<span style='color:#4ade80'>ATTIVA (Test 60s ogni Mercoled&igrave; ore 12)</span>" : "<span style='color:#94a3b8'>DISABILITATA</span>";
  html += "<div style='background:#0b2545;padding:10px;border-radius:8px;margin-bottom:10px;'>";
  html += "<div style='display:flex;justify-content:space-between;align-items:center;'>";
  html += "<b>2. Protezione Antiblocco Pompa / Valvole:</b> " + pumpStatus;
  html += "<a class='btn' style='padding:5px 10px;font-size:12px;background:" + String(pump_protection_enabled ? "#dc2626" : "#16a34a") + ";' href='/ctrl?pump_prot=" + String(pump_protection_enabled ? 0 : 1) + "#adv-box'>" + (pump_protection_enabled ? "Disabilita" : "Abilita") + "</a>";
  html += "</div>";
  html += "<p style='font-size:12px;color:#8da9c4;margin:4px 0 8px 0;'>Fa circolare l'acqua per 60 secondi in caso di inattivit&agrave; prolungata (es. d'estate), evitando il blocco meccanico della pompa.</p>";
  html += "<div class='btn-row'><a class='btn' style='padding:8px;flex:1;background:#0284c7;font-size:13px;' href='/ctrl?pump_test=1#adv-box'>&#9654; Avvia Test Pompa Subito (60 secondi)</a></div>";
  html += "</div>";

  // 3. Modalità Notte & Auto-Dimming
  String nightStatus = night_mode_enabled ? ("<span style='color:#4ade80'>ATTIVA (" + String(night_start_hour) + ":00 - " + String(night_end_hour) + ":00)</span>") : "<span style='color:#94a3b8'>DISABILITATA</span>";
  html += "<div style='background:#0b2545;padding:10px;border-radius:8px;margin-bottom:10px;'>";
  html += "<div style='display:flex;justify-content:space-between;align-items:center;'>";
  html += "<b>3. Modalit&agrave; Notte (Auto-Dimming Orario):</b> " + nightStatus;
  html += "<a class='btn' style='padding:5px 10px;font-size:12px;background:" + String(night_mode_enabled ? "#dc2626" : "#16a34a") + ";' href='/ctrl?night_en=" + String(night_mode_enabled ? 0 : 1) + "#adv-box'>" + (night_mode_enabled ? "Disabilita" : "Abilita") + "</a>";
  html += "</div>";
  html += "<p style='font-size:12px;color:#8da9c4;margin:4px 0 8px 0;'>Spegne completamente lo schermo durante le ore di sonno; basta toccare lo schermo per riattivarlo per 15s.</p>";
  html += "</div>";

  // 4. Blocco Pulizia / Child Lock
  String lockStatus = child_lock_active ? ("<span style='color:#facc15'>ATTIVO (Termina tra " + String((child_lock_until_ms - millis()) / 1000) + "s)</span>") : "<span style='color:#4ade80'>Touch Libero</span>";
  html += "<div style='background:#0b2545;padding:10px;border-radius:8px;margin-bottom:10px;'>";
  html += "<div style='display:flex;justify-content:space-between;align-items:center;'>";
  html += "<b>4. Blocco Pulizia Schermo (Child Lock):</b> " + lockStatus;
  html += "<a class='btn' style='padding:5px 10px;font-size:12px;background:#7c3aed;' href='/ctrl?clean_screen=1#adv-box'>&#129529; Blocca Touch per 30 Secondi</a>";
  html += "</div>";
  html += "<p style='font-size:12px;color:#8da9c4;margin:4px 0 0 0;'>Permette di passare un panno sullo schermo touch per pulirlo senza modificare accidentalmente setpoint o modalit&agrave;.</p>";
  html += "</div>";

  // 5. Sensore Finestra Esterno via MQTT & 6. Protezione Antigelo
  html += "<form method='POST' action='/adv'>";
  html += "<div style='background:#0b2545;padding:10px;border-radius:8px;margin-bottom:10px;'>";
  String extWinStatus = ext_window_enabled ? (ext_window_open ? "<span style='color:#facc15'>APERTA (Pausa!)</span>" : "<span style='color:#4ade80'>Chiusa (OK)</span>") : "<span style='color:#94a3b8'>Disabilitato</span>";
  html += "<div style='display:flex;justify-content:space-between;align-items:center;'>";
  html += "<b>5. Sensore Finestra Esterno (MQTT):</b> " + extWinStatus;
  html += "<a class='btn' style='padding:5px 10px;font-size:12px;background:" + String(ext_window_enabled ? "#dc2626" : "#16a34a") + ";' href='/ctrl?ext_win_en=" + String(ext_window_enabled ? 0 : 1) + "#adv-box'>" + (ext_window_enabled ? "Disabilita" : "Abilita") + "</a>";
  html += "</div>";
  html += "<label class='lbl' style='margin-top:6px;display:block;'>Topic MQTT Sensore Magnetico (Zigbee2MQTT / Shelly / HA)</label>";
  html += "<input name='ext_win_top' value='" + String(ext_window_topic) + "' placeholder='es. zigbee2mqtt/Finestra_Sala/contact'>";
  html += "</div>";

  html += "<div style='background:#0b2545;padding:10px;border-radius:8px;margin-bottom:10px;'>";
  String afStat = antifrost_enabled ? (antifrost_active ? "<span style='color:#f87171'>ALLARME ATTIVO!</span>" : "<span style='color:#4ade80'>Attivo (OK)</span>") : "<span style='color:#94a3b8'>Disabilitato</span>";
  html += "<div style='display:flex;justify-content:space-between;align-items:center;'>";
  html += "<b>6. Protezione Antigelo:</b> " + afStat;
  html += "<a class='btn' style='padding:5px 10px;font-size:12px;background:" + String(antifrost_enabled ? "#dc2626" : "#16a34a") + ";' href='/ctrl?antifrost=" + String(antifrost_enabled ? 0 : 1) + "#adv-box'>" + (antifrost_enabled ? "Disabilita" : "Abilita") + "</a>";
  html += "</div>";
  html += "<div class='grid' style='margin-top:6px;'>";
  html += "<div><label class='lbl'>Soglia Antigelo (&deg;C)</label><input type='number' step='0.5' min='3.0' max='10.0' name='antifrost_th' value='" + String(antifrost_temp_threshold, 1) + "'></div>";
  html += "<div><label class='lbl'>Delta Isteresi (&deg;C)</label><input type='number' step='0.1' min='0.1' max='2.0' name='hyst_val' value='" + String(hysteresis_val, 1) + "'></div>";
  html += "</div></div>";

  html += "<button type='submit' style='background:#10b981;font-size:16px;font-weight:bold;'>Salva Impostazioni Avanzate</button>";
  html += "</form></details>";

  // Card 1A: Meteo Barometrico & Comfort Ambientale (BME280)
  html += "<details class='card' id='meteo-box'><summary><h3>&#9925; Meteo Barometrico &amp; Comfort Ambientale</h3></summary>";
  if (bme_ok && last_pressure_hpa > 300.0f) {
    String deltaSign = (pressure_delta_3h >= 0.0f) ? "+" : "";
    html += "<div class='grid'>";
    html += "<div><div class='lbl'>Pressione Barometrica</div><div class='val' style='font-size:22px;'>" + String(last_pressure_hpa, 1) + " hPa</div>";
    html += "<div style='font-size:12px;color:#93c5fd;'>Tendenza 3h: <b>" + deltaSign + String(pressure_delta_3h, 1) + " hPa</b></div></div>";
    html += "<div><div class='lbl'>Comfort Termo-Igrometrico</div><div class='val' style='font-size:20px;color:" + comfort_color_hex + ";'>" + comfort_str + "</div></div>";
    html += "<div><div class='lbl'>Temp. Percepita (Humidex)</div><div class='val' style='font-size:22px;'>" + String(calc_heat_index, 1) + " &deg;C</div></div>";
    html += "<div><div class='lbl'>Punto di Rugiada (Condensa)</div><div class='val' style='font-size:22px;'>" + String(calc_dew_point, 1) + " &deg;C</div></div>";
    html += "</div>";
    html += "<div style='margin-top:10px;padding:8px 12px;background:#0b2545;border-radius:8px;font-size:14px;border-left:4px solid #38bdf8;'>";
    html += "<b>Previsione Barometrica:</b> " + weather_forecast_str + "</div>";
  } else {
    html += "<p style='color:#fca5a5;margin:4px 0;'>In attesa di lettura valida dal sensore BME280 (Pressione / Temperatura / Umidit&agrave;).</p>";
  }
  html += "</details>";

  // Card 1B: Scanner I2C & Diagnostica Sensore BME280
  bool hasI2cArg = (webServer.hasArg("i2cscan") || webServer.hasArg("i2cpinscan"));
  html += "<details class='card' id='i2c-box'" + String(hasI2cArg ? " open" : "") + "><summary><h3>&#128269; Scanner I2C &amp; Diagnostica Sensore</h3></summary>";
  char addrStr[12];
  if (bme_ok && bme_found_addr != 0) snprintf(addrStr, sizeof(addrStr), "0x%02X", bme_found_addr);
  else snprintf(addrStr, sizeof(addrStr), "Non rilevato");
  html += "<p style='margin:4px 0;'>Stato BME280: <b>" + (bme_ok ? ("<span style='color:#4ade80'>CONNESSO (" + String(addrStr) + ")</span>") : "<span style='color:#f87171'>NON TROVATO</span>") + "</b>";
  html += " &mdash; Pin attuali: <b>SDA = G" + String(i2c_sda_pin) + "</b>, <b>SCL = G" + String(i2c_scl_pin) + "</b></p>";
  html += "<div class='btn-row'>";
  html += "<a class='btn' style='padding:10px;flex:1;background:#16a34a;' href='/?i2cscan=1&t=" + String(millis()) + "#i2c-box'>Scansiona Bus I2C (G" + String(i2c_sda_pin) + "/G" + String(i2c_scl_pin) + ")</a>";
  html += "<a class='btn' style='padding:10px;flex:1;background:#d97706;' href='/?i2cpinscan=1&t=" + String(millis()) + "#i2c-box'>Auto-Cerca Sensore su Tutti i Pin</a>";
  html += "</div>";
  html += last_i2c_scan_html;
  html += "<form method='POST' action='/i2c' style='margin-top:10px;'>";
  html += "<div class='grid'><div><label class='lbl'>Pin GPIO SDA</label><input type='number' name='sda' value='" + String(i2c_sda_pin) + "'></div>";
  html += "<div><label class='lbl'>Pin GPIO SCL</label><input type='number' name='scl' value='" + String(i2c_scl_pin) + "'></div></div>";
  html += "<button type='submit'>Imposta Pin I2C e Scansiona</button></form></details>";

  // Card 2: Connessione WiFi
  html += "<details class='card' id='wifi-box'" + String(doScan ? " open" : "") + "><summary><h3>&#128246; Connessione WiFi</h3></summary>";
  if (WiFi.status() == WL_CONNECTED) {
    int rssi = WiFi.RSSI();
    int pct = constrain(2 * (rssi + 100), 0, 100);
    String sigColor = (pct >= 70) ? "#4ade80" : ((pct >= 45) ? "#fde047" : "#f87171");
    String sigText = (pct >= 70) ? "Eccellente" : ((pct >= 45) ? "Buono" : "Debole");
    html += "<p style='margin:4px 0;'>Stato: <b>Connesso a " + WiFi.SSID() + "</b> (IP: <b>" + WiFi.localIP().toString() + "</b>)</p>";
    html += "<div style='margin:8px 0;background:#0b2545;padding:10px;border-radius:8px;'>";
    html += "<div style='display:flex;justify-content:space-between;margin-bottom:4px;font-size:13px;'>";
    html += "<span><b>Potenza Segnale Wi-Fi:</b> <span style='color:" + sigColor + ";'>" + sigText + " (" + String(pct) + "%)</span></span>";
    html += "<span style='color:#93c5fd;font-size:12px;'>" + String(rssi) + " dBm</span></div>";
    html += "<div style='background:#1e293b;border-radius:6px;height:14px;overflow:hidden;border:1px solid #334155;'>";
    html += "<div style='background:" + sigColor + ";width:" + String(pct) + "%;height:100%;transition:width 0.5s;'></div>";
    html += "</div></div>";
  } else {
    html += "<p>Stato: <b>Modalit&agrave; Access Point (Termostato_AP - 192.168.4.1)</b></p>";
  }
  html += "<p><a class='btn' style='padding:8px 12px;width:auto;' href='/?scan=1#wifi-box'>Scansiona Reti WiFi</a></p>";
  if (doScan) {
    if (nNets < 0) {
      html += "<p style='color:#f87171;'><b>Scansione non riuscita (codice " + String(nNets) + "). Riprova o digita l'SSID direttamente qui sotto.</b></p>";
    } else if (nNets == 0) {
      html += "<p style='color:#facc15;'><b>Nessuna rete 2.4 GHz rilevata nelle vicinanze.</b> Puoi comunque digitare il nome del tuo Wi-Fi manualmente qui sotto.</p>";
    } else {
      html += "<p style='color:#93c5fd;font-size:13px;'>Trovate <b>" + String(nNets) + "</b> reti. Tocca la tua rete per selezionarla:</p>";
      for (int i = 0; i < nNets; i++) {
        String s = WiFi.SSID(i);
        if (s.length() == 0) s = "(Rete Nascosta)";
        html += "<div class='net-item' onclick=\"document.getElementById('ssid').value='" + s + "'\">";
        html += "<span><b>" + s + "</b></span><span>" + String(WiFi.RSSI(i)) + " dBm</span></div>";
      }
    }
  }
  html += "<form method='POST' action='/wifi'>";
  html += "<label class='lbl'>Nome Rete WiFi (SSID)</label>";
  html += "<input id='ssid' name='ssid' value='" + String(wifi_ssid) + "' placeholder='Inserisci o clicca una rete WiFi'>";
  html += "<label class='lbl'>Password WiFi</label>";
  html += "<input type='password' name='pass' value='" + String(wifi_pass) + "' placeholder='Password WiFi'>";
  html += "<button type='submit'>Salva e Connetti al WiFi</button></form></details>";

  // Card 3: Parametri MQTT, Struttura Immobile & Home Assistant Auto-Discovery
  bool hasHaArg = webServer.hasArg("hadisc");
  html += "<details class='card' id='mqtt-box'" + String(hasHaArg ? " open" : "") + "><summary><h3>&#127968; Struttura Immobile &amp; Home Assistant (MQTT)</h3></summary>";
  html += "<p style='margin:4px 0;'>Stato Broker MQTT: <b>" + (client.connected() ? ("<span style='color:#4ade80'>CONNESSO a " + String(mqtt_server) + "</span>") : ("<span style='color:#f87171'>NON CONNESSO (" + String(mqtt_server) + ")</span>")) + "</b></p>";
  
  html += "<div style='background:#0b2545;padding:12px;border-radius:8px;margin-margin:10px 0;border-left:4px solid #38bdf8;font-size:13px;'>";
  html += "<div style='margin-bottom:4px;'><b>Posizione:</b> <span style='color:#fde047;font-size:14px;'>" + get_full_location_name() + "</span></div>";
  html += "<div style='margin-bottom:4px;'><b>Nome Zona HA:</b> <span style='color:#4ade80;'>" + zoneName + "</span> &mdash; <b>Area Suggerita:</b> <span style='color:#93c5fd;'>" + get_floor_name() + " - " + get_room_name() + "</span></div>";
  html += "<div><b>Topic Base:</b> <code>" + String(thermostat_id) + "</code> &mdash; <b>Entit&agrave; Clima:</b> <code>climate.termostato_" + zoneSlug + "</code></div>";
  html += "</div>";

  if (webServer.hasArg("hadisc")) {
    html += "<div style='background:#0b2545;padding:8px;border-radius:6px;border-left:4px solid #4ade80;margin:6px 0;font-size:13px;'>Pacchetti MQTT Auto-Discovery per <b>" + zoneName + "</b> inviati a Home Assistant!</div>";
  }
  html += "<p style='margin:6px 0;'><a class='btn' style='padding:8px 12px;width:auto;background:#16a34a;' href='/?hadisc=1&t=" + String(millis()) + "#mqtt-box'>Invia Auto-Discovery a Home Assistant</a></p>";
  
  html += "<form method='POST' action='/mqtt' style='margin-top:12px;'>";

  // 1. Abitazione / Edificio
  html += "<label class='lbl'>1. Abitazione / Edificio</label>";
  html += "<input id='h_name' name='house_name' value='" + String(house_name) + "' placeholder='Es. Casa, Villa, Appartamento 1' oninput='updateAutoTopic()'>";
  html += "<div class='pills'>";
  html += "<span class='pill' onclick=\"setPill('h_name','Casa')\">Casa</span>";
  html += "<span class='pill' onclick=\"setPill('h_name','Appartamento 1')\">Appartamento 1</span>";
  html += "<span class='pill' onclick=\"setPill('h_name','Appartamento 2')\">Appartamento 2</span>";
  html += "<span class='pill' onclick=\"setPill('h_name','Villa')\">Villa</span>";
  html += "<span class='pill' onclick=\"setPill('h_name','Ufficio')\">Ufficio</span>";
  html += "<span class='pill' onclick=\"setPill('h_name','Dependance')\">Dependance</span>";
  html += "<span class='pill' onclick=\"setPill('h_name','B&B')\">B&B</span>";
  html += "</div>";

  // 2. Piano / Livello
  html += "<label class='lbl'>2. Piano / Livello</label>";
  html += "<input id='f_name' name='floor_name' value='" + String(floor_name) + "' placeholder='Es. Piano Terra, Primo Piano, Seminterrato' oninput='updateAutoTopic()'>";
  html += "<div class='pills'>";
  html += "<span class='pill' onclick=\"setPill('f_name','Seminterrato')\">Seminterrato</span>";
  html += "<span class='pill' onclick=\"setPill('f_name','Piano Terra')\">Piano Terra</span>";
  html += "<span class='pill' onclick=\"setPill('f_name','Primo Piano')\">Primo Piano</span>";
  html += "<span class='pill' onclick=\"setPill('f_name','Secondo Piano')\">Secondo Piano</span>";
  html += "<span class='pill' onclick=\"setPill('f_name','Mansarda')\">Mansarda</span>";
  html += "<span class='pill' onclick=\"setPill('f_name','Taverna')\">Taverna</span>";
  html += "<span class='pill' onclick=\"setPill('f_name','Interrato')\">Interrato</span>";
  html += "</div>";

  // 3. Stanza / Zona Termica
  html += "<label class='lbl'>3. Stanza / Zona Termica (anche pi&ugrave; termostati per piano)</label>";
  html += "<input id='r_name' name='room_name' value='" + String(room_name) + "' placeholder='Es. Soggiorno, Cucina, Camera, Zona Notte' oninput='updateAutoTopic()'>";
  html += "<div class='pills'>";
  html += "<span class='pill' onclick=\"setPill('r_name','Zona Giorno')\">Zona Giorno</span>";
  html += "<span class='pill' onclick=\"setPill('r_name','Zona Notte')\">Zona Notte</span>";
  html += "<span class='pill' onclick=\"setPill('r_name','Soggiorno')\">Soggiorno</span>";
  html += "<span class='pill' onclick=\"setPill('r_name','Cucina')\">Cucina</span>";
  html += "<span class='pill' onclick=\"setPill('r_name','Camera Matrimoniale')\">Camera Matrimoniale</span>";
  html += "<span class='pill' onclick=\"setPill('r_name','Bagno')\">Bagno</span>";
  html += "<span class='pill' onclick=\"setPill('r_name','Studio')\">Studio</span>";
  html += "<span class='pill' onclick=\"setPill('r_name','Disimpegno')\">Disimpegno</span>";
  html += "</div>";

  // Checkbox Generazione Automatica Topic
  html += "<div style='background:#0b2545;padding:8px 12px;border-radius:8px;margin:8px 0;display:flex;align-items:center;gap:8px;'>";
  html += "<input type='checkbox' id='auto_top' name='auto_topic' value='1' style='width:auto;margin:0;' " + String(auto_topic_enabled ? "checked" : "") + " onchange='toggleAutoTopic()'>";
  html += "<label for='auto_top' style='font-size:13px;color:#93c5fd;cursor:pointer;'>Genera automaticamente il Topic MQTT dalla gerarchia (consigliato)</label>";
  html += "</div>";

  html += "<label class='lbl'>Topic Base Termostato MQTT</label>";
  html += "<input id='tid' name='thermostat_id' value='" + String(thermostat_id) + "' " + String(auto_topic_enabled ? "readonly style='background:#1e293b;color:#94a3b8;'" : "") + ">";

  html += "<label class='lbl'>Topic Rel&egrave; Esterno (Caldaia / Elettrovalvola)</label><input name='rele_id' value='" + String(rele_id) + "'>";
  html += "<label class='lbl'>Server MQTT (IP / Host)</label><input name='mqtt_server' value='" + String(mqtt_server) + "'>";
  html += "<div class='grid'><div><label class='lbl'>Utente MQTT</label><input name='mqtt_user' value='" + String(mqtt_user) + "'></div>";
  html += "<div><label class='lbl'>Password MQTT</label><input type='password' name='mqtt_pass' value='" + String(mqtt_pass) + "'></div></div>";
  html += "<label class='lbl'>Offset Temperatura (&deg;C, da -5.0 a +5.0)</label><input name='offset' value='" + String(temperature_offset, 1) + "'>";
  html += "<button type='submit'>Salva Configurazione Immobile &amp; Riconnetti</button></form></details>";

  html += "<script>";
  html += "if(window.location.hash){";
  html += "  var el=document.querySelector(window.location.hash);";
  html += "  if(el && el.tagName==='DETAILS') el.open=true;";
  html += "}";
  html += "document.querySelectorAll('details.card').forEach(function(d){";
  html += "  var id=d.id;";
  html += "  if(id){";
  html += "    var saved=localStorage.getItem('tab_'+id);";
  html += "    if(saved!==null && !window.location.hash && !window.location.search){";
  html += "      d.open=(saved==='1');";
  html += "    }";
  html += "    d.addEventListener('toggle',function(){";
  html += "      localStorage.setItem('tab_'+id,d.open?'1':'0');";
  html += "    });";
  html += "  }";
  html += "});";
  html += "function slug(s){return s.trim().toLowerCase().replace(/[^a-z0-9]+/g,'_').replace(/^_+|_+$/g,'');}";
  html += "function setPill(id,val){document.getElementById(id).value=val;updateAutoTopic();}";
  html += "function updateAutoTopic(){";
  html += "  var auto=document.getElementById('auto_top').checked;";
  html += "  if(auto){";
  html += "    var h=slug(document.getElementById('h_name').value)||'casa';";
  html += "    var f=slug(document.getElementById('f_name').value)||'piano_terra';";
  html += "    var r=slug(document.getElementById('r_name').value)||'zona_giorno';";
  html += "    document.getElementById('tid').value=h+'/clima/'+f+'/'+r+'/';";
  html += "  }";
  html += "}";
  html += "function toggleAutoTopic(){";
  html += "  var auto=document.getElementById('auto_top').checked;";
  html += "  var tid=document.getElementById('tid');";
  html += "  tid.readOnly=auto;";
  html += "  tid.style.background=auto?'#1e293b':'';";
  html += "  tid.style.color=auto?'#94a3b8':'';";
  html += "  if(auto)updateAutoTopic();";
  html += "}";
  html += "</script>";

  html += "</body></html>";
  webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  webServer.send(200, "text/html", html);
}

void setup_web_server() {
  webServer.on("/", HTTP_GET, handleRoot);

  webServer.on("/ctrl", HTTP_GET, []() {
    if (webServer.hasArg("sp")) {
      temp_setpoint = round((temp_setpoint + webServer.arg("sp").toFloat()) * 10.0f) / 10.0f;
      save_state_eeprom();
      char buf[10];
      dtostrf(temp_setpoint, 0, 1, buf);
      gslc_ElemSetTxtStr(&m_gui, m_pElemOutTxt_Temp_Setpoint, buf);
      publish_ha_state();
      play_beep(2600, 20);
    }
    if (webServer.hasArg("mode")) {
      set_thermostat_mode(webServer.arg("mode").toInt());
      play_beep(2500, 35);
    }
    if (webServer.hasArg("crono")) {
      crono_enabled = (webServer.arg("crono").toInt() != 0);
      current_preset_mode = crono_enabled ? "schedule" : "none";
      evaluate_crono(true);
      save_state_eeprom();
      save_json_config();
      publish_ha_state();
      play_beep(2700, 30);
    }
    if (webServer.hasArg("preset")) {
      String pr = webServer.arg("preset");
      if (pr == "away") {
        crono_enabled = false;
        current_preset_mode = "away";
        temp_setpoint = 18.0f;
      } else if (pr == "boost") {
        crono_enabled = false;
        current_preset_mode = "boost";
        temp_setpoint = 23.0f;
      } else if (pr == "schedule") {
        crono_enabled = true;
        current_preset_mode = "schedule";
      } else {
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
      play_beep(2700, 35);
    }
    if (webServer.hasArg("buzzer")) {
      int bval = constrain(webServer.arg("buzzer").toInt(), 0, 3);
      buzzer_enabled = (bval > 0);
      if (bval > 0) buzzer_vol = (uint8_t)bval;
      save_state_eeprom();
      save_json_config();
      publish_ha_state();
      if (buzzer_enabled) play_beep(2800, 35);
    }
    if (webServer.hasArg("antifrost")) {
      antifrost_enabled = (webServer.arg("antifrost").toInt() != 0);
      save_json_config();
      publish_ha_state();
      play_beep(2600, 25);
    }
    if (webServer.hasArg("openwin")) {
      open_window_enabled = (webServer.arg("openwin").toInt() != 0);
      if (!open_window_enabled) open_window_active = false;
      save_json_config();
      publish_ha_state();
      play_beep(2600, 25);
    }
    if (webServer.hasArg("hyst_en")) {
      hysteresis_enabled = (webServer.arg("hyst_en").toInt() != 0);
      save_json_config();
      publish_ha_state();
      play_beep(2600, 25);
    }
    if (webServer.hasArg("hyst_val")) {
      float hv = webServer.arg("hyst_val").toFloat();
      if (!isnan(hv) && hv >= 0.1f && hv <= 2.0f) {
        hysteresis_val = round(hv * 10.0f) / 10.0f;
        save_json_config();
        publish_ha_state();
        play_beep(2600, 25);
      }
    }
    if (webServer.hasArg("pump_prot")) {
      pump_protection_enabled = (webServer.arg("pump_prot").toInt() != 0);
      save_json_config();
      publish_ha_state();
      play_beep(2600, 25);
    }
    if (webServer.hasArg("pump_test")) {
      trigger_pump_test(60);
    }
    if (webServer.hasArg("night_en")) {
      night_mode_enabled = (webServer.arg("night_en").toInt() != 0);
      save_json_config();
      publish_ha_state();
      play_beep(2600, 25);
    }
    if (webServer.hasArg("clean_screen")) {
      trigger_child_lock(30);
    }
    if (webServer.hasArg("ext_win_en")) {
      ext_window_enabled = (webServer.arg("ext_win_en").toInt() != 0);
      if (!ext_window_enabled) ext_window_open = false;
      if (client.connected() && ext_window_enabled && ext_window_topic[0] != '\0') {
        client.subscribe(ext_window_topic);
      }
      save_json_config();
      publish_ha_state();
      play_beep(2600, 25);
    }
    if (webServer.hasArg("antifrost_th")) {
      float ath = webServer.arg("antifrost_th").toFloat();
      if (!isnan(ath) && ath >= 3.0f && ath <= 10.0f) {
        antifrost_temp_threshold = round(ath * 10.0f) / 10.0f;
        save_json_config();
        publish_ha_state();
        play_beep(2600, 25);
      }
    }
    previousMillis_lcd = millis();
    lcd_set_bright();
    gslc_ElemSetVisible(&m_gui, m_pElemBtn_wake_up, false);
    gslc_ElemSetVisible(&m_gui, m_pElemBtn_wake_up2, false);
    webServer.sendHeader("Location", "/");
    webServer.send(303);
  });

  webServer.on("/adv", HTTP_POST, []() {
    if (webServer.hasArg("hyst_en")) hysteresis_enabled = (webServer.arg("hyst_en").toInt() != 0);
    if (webServer.hasArg("hyst_val")) {
      float hv = webServer.arg("hyst_val").toFloat();
      if (!isnan(hv) && hv >= 0.1f && hv <= 2.0f) hysteresis_val = round(hv * 10.0f) / 10.0f;
    }
    if (webServer.hasArg("pump_prot")) pump_protection_enabled = (webServer.arg("pump_prot").toInt() != 0);
    if (webServer.hasArg("night_en")) night_mode_enabled = (webServer.arg("night_en").toInt() != 0);
    if (webServer.hasArg("night_start")) night_start_hour = constrain(webServer.arg("night_start").toInt(), 0, 23);
    if (webServer.hasArg("night_end")) night_end_hour = constrain(webServer.arg("night_end").toInt(), 0, 23);
    if (webServer.hasArg("ext_win_en")) {
      ext_window_enabled = (webServer.arg("ext_win_en").toInt() != 0);
      if (!ext_window_enabled) ext_window_open = false;
    }
    if (webServer.hasArg("ext_win_top")) {
      strncpy(ext_window_topic, webServer.arg("ext_win_top").c_str(), sizeof(ext_window_topic) - 1);
      ext_window_topic[sizeof(ext_window_topic) - 1] = '\0';
      if (client.connected() && ext_window_enabled && ext_window_topic[0] != '\0') {
        client.subscribe(ext_window_topic);
      }
    }
    if (webServer.hasArg("antifrost_th")) {
      float ath = webServer.arg("antifrost_th").toFloat();
      if (!isnan(ath) && ath >= 3.0f && ath <= 10.0f) antifrost_temp_threshold = round(ath * 10.0f) / 10.0f;
    }
    save_json_config();
    publish_ha_state();
    play_beep(2700, 30);
    webServer.sendHeader("Location", "/#adv-box");
    webServer.send(303);
  });

  webServer.on("/crono", HTTP_POST, []() {
    for (int d = 0; d < 2; d++) {
      for (int s = 0; s < 4; s++) {
        String idx = String(d) + "_" + String(s);
        if (webServer.hasArg("t" + idx)) {
          String tStr = webServer.arg("t" + idx);
          if (tStr.length() >= 5) {
            int hh = tStr.substring(0, 2).toInt();
            int mm = tStr.substring(3, 5).toInt();
            crono_sched[d][s].start_min = constrain(hh * 60 + mm, 0, 1439);
          }
        }
        if (webServer.hasArg("sp" + idx)) {
          float spv = webServer.arg("sp" + idx).toFloat();
          if (!isnan(spv) && spv >= 10.0f && spv <= 35.0f) {
            crono_sched[d][s].temp = round(spv * 10.0f) / 10.0f;
          }
        }
      }
    }
    crono_enabled = true;
    current_preset_mode = "schedule";
    save_json_config();
    save_state_eeprom();
    evaluate_crono(true);
    publish_ha_state();
    play_beep(2800, 45);
    webServer.sendHeader("Location", "/#crono-box");
    webServer.send(303);
  });

  webServer.on("/mqtt", HTTP_POST, []() {
    if (webServer.hasArg("house_name")) strlcpy(house_name, webServer.arg("house_name").c_str(), sizeof(house_name));
    if (webServer.hasArg("floor_name")) strlcpy(floor_name, webServer.arg("floor_name").c_str(), sizeof(floor_name));
    if (webServer.hasArg("room_name")) strlcpy(room_name, webServer.arg("room_name").c_str(), sizeof(room_name));
    auto_topic_enabled = webServer.hasArg("auto_topic");

    if (auto_topic_enabled) {
      strlcpy(thermostat_id, generate_auto_topic().c_str(), sizeof(thermostat_id));
    } else if (webServer.hasArg("thermostat_id")) {
      String tid = webServer.arg("thermostat_id");
      if (tid.length() > 0 && !tid.endsWith("/")) tid += "/";
      strlcpy(thermostat_id, tid.c_str(), sizeof(thermostat_id));
    }
    if (webServer.hasArg("mqtt_server")) strlcpy(mqtt_server, webServer.arg("mqtt_server").c_str(), sizeof(mqtt_server));
    if (webServer.hasArg("mqtt_user")) strlcpy(mqtt_user, webServer.arg("mqtt_user").c_str(), sizeof(mqtt_user));
    if (webServer.hasArg("mqtt_pass")) strlcpy(mqtt_pass, webServer.arg("mqtt_pass").c_str(), sizeof(mqtt_pass));
    if (webServer.hasArg("rele_id")) strlcpy(rele_id, webServer.arg("rele_id").c_str(), sizeof(rele_id));
    if (webServer.hasArg("offset")) {
      float off = constrain(webServer.arg("offset").toFloat(), -5.0f, 5.0f);
      slider_pos = (int)round(off * 10.0f + 50.0f);
      temperature_offset = (slider_pos - 50) / 10.0f;
      EEPROM.write(0, slider_pos);
      EEPROM.commit();
      char buf[12];
      dtostrf(temperature_offset, 4, 1, buf);
      gslc_ElemXSliderSetPos(&m_gui, m_pElemSlider_temp_offset, slider_pos);
      gslc_ElemSetTxtStr(&m_gui, m_pElemOutTxt_temp_offset, buf);
    }
    save_json_config();
    if (client.connected()) client.disconnect();
    client.begin(mqtt_server, net);
    webServer.sendHeader("Location", "/#mqtt-box");
    webServer.send(303);
  });

  webServer.on("/i2c", HTTP_POST, []() {
    if (webServer.hasArg("sda")) i2c_sda_pin = webServer.arg("sda").toInt();
    if (webServer.hasArg("scl")) i2c_scl_pin = webServer.arg("scl").toInt();
    save_json_config();
    run_i2c_scan(i2c_sda_pin, i2c_scl_pin);
    get_temp_print();
    webServer.sendHeader("Location", "/#i2c-box");
    webServer.send(303);
  });

  webServer.on("/wifi", HTTP_POST, []() {
    if (webServer.hasArg("ssid")) strlcpy(wifi_ssid, webServer.arg("ssid").c_str(), sizeof(wifi_ssid));
    if (webServer.hasArg("pass")) strlcpy(wifi_pass, webServer.arg("pass").c_str(), sizeof(wifi_pass));
    save_json_config();

    String resp = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'>";
    resp += "<meta http-equiv='refresh' content='8;url=/'><style>body{background:#0b2545;color:#fff;font-family:sans-serif;padding:24px;text-align:center;}</style></head>";
    resp += "<body><h2>Connessione a " + String(wifi_ssid) + " in corso...</h2><p>Controlla l'indirizzo IP sul display del termostato tra pochi secondi.</p></body></html>";
    webServer.send(200, "text/html", resp);

    delay(300);
    WiFi.begin(wifi_ssid, wifi_pass);
  });

  // Captive portal redirect for smartphones connecting to Termostato_AP
  webServer.onNotFound([]() {
    webServer.sendHeader("Location", "http://192.168.4.1/");
    webServer.send(302, "text/plain", "");
  });

  webServer.begin();
  Serial.println("HTTP WebServer started on port 80");
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\r\n", path);
    if(fs.remove(path)){
        Serial.println("- file deleted");
    } else {
        Serial.println("- delete failed");
    }
}



void setup()
{
  Serial.begin(115200);
  Serial.println("Restarting...");
  char buffer[12];

  chipId = 0;
  for(int i=0; i<17; i=i+8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }

  // Initialize backlight, relay pin, and buzzer
  pinMode(TFT_BL_PIN, OUTPUT);
  pinMode(rele_pin, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
#if defined(HARDWARE_CYD)
  pinMode(CYD_LED_RED, OUTPUT);
  pinMode(CYD_LED_GREEN, OUTPUT);
  pinMode(CYD_LED_BLUE, OUTPUT);
  digitalWrite(CYD_LED_RED, HIGH);
  digitalWrite(CYD_LED_GREEN, HIGH);
  digitalWrite(CYD_LED_BLUE, HIGH);
#endif
  // configure LED PWM functionalities
  ledcSetup(0, 5000, 8);
  ledcAttachPin(TFT_BL_PIN, 0);
  lcd_set_bright();
  digitalWrite(rele_pin, LOW);

  // Initialize I2C and run initial I2C scan + BME280 init
  run_i2c_scan(i2c_sda_pin, i2c_scl_pin);

  EEPROM.begin(EEPROM_SIZE);
  slider_pos = EEPROM.read(0);
  if (slider_pos > 100){
    EEPROM.write(0, 50);
    EEPROM.commit();
    slider_pos=50;
  }
  if (EEPROM.read(EEPROM_MAGIC_ADDR) == EEPROM_MAGIC_VAL) {
    uint8_t saved_mode = EEPROM.read(EEPROM_MODE_ADDR);
    if (saved_mode <= 2) {
      stato = saved_mode;
      if (stato != 0) stato_shadow = stato;
    }
    float saved_sp = EEPROM.readFloat(EEPROM_SP_ADDR);
    if (!isnan(saved_sp) && saved_sp >= 5.0f && saved_sp <= 45.0f) {
      temp_setpoint = round(saved_sp * 10.0f) / 10.0f;
    }
    uint8_t saved_buz = EEPROM.read(EEPROM_BUZZER_ADDR);
    if (saved_buz <= 1) buzzer_enabled = (saved_buz == 1);
    uint8_t saved_cro = EEPROM.read(EEPROM_CRONO_ADDR);
    if (saved_cro <= 1) {
      crono_enabled = (saved_cro == 1);
      current_preset_mode = crono_enabled ? "schedule" : "none";
    }
    Serial.printf("Restored from EEPROM: setpoint=%.1f C, mode=%d, buzzer=%d, crono=%d\n",
                  temp_setpoint, stato, buzzer_enabled, crono_enabled);
  } else {
    save_state_eeprom();
  }

  // Initialize GUIslice display
  gslc_InitDebug(&DebugOut);
#if defined(HARDWARE_CYD)
  gslc_InitTouchHandler(&cydTouchHandler);
#endif
  InitGUIslice_gen();
#if defined(HARDWARE_CYD)
  gslc_SetTouchRemapEn(&m_gui, false);
#endif

  gslc_ElemXSliderSetPos(&m_gui,m_pElemSlider_temp_offset,slider_pos);
  gslc_ElemSetVisible(&m_gui,m_pElemBtn_wake_up,false);
  gslc_ElemSetVisible(&m_gui,m_pElemBtn_wake_up2,false);
  // Disabilita il click sui simboli decorativi dei gradi (°C) e LED stato per evitare quadratini gialli/verdi al tocco
  gslc_ElemSetClickEn(&m_gui,m_pElemRB_sample,false);
  gslc_ElemSetClickEn(&m_gui,m_pElemRB_sample2,false);
  gslc_ElemSetClickEn(&m_gui,m_pElemRB_sample2_3,false);
  gslc_ElemSetClickEn(&m_gui,m_pElemRB_sample2_4,false);
  gslc_ElemSetClickEn(&m_gui,m_pElemRB5,false);
  set_thermostat_mode(stato);
  dtostrf(temp_setpoint, 0, 1, buffer);
  gslc_ElemSetTxtStr(&m_gui,m_pElemOutTxt_Temp_Setpoint,buffer);
  gslc_ElemSetTxtStr(&m_gui,m_pElemOutTxt_stat,"---");
  temperature_offset=(slider_pos-50)/10.0;
  dtostrf(temperature_offset, 4, 1, buffer);
  gslc_ElemSetTxtStr(&m_gui,m_pElemOutTxt_temp_offset,buffer);
  gslc_ElemSetTxtStr(&m_gui,m_pElemOutTxt_Ip_Address16,"WiFi init...");
  gslc_Update(&m_gui);

  // Read configuration from FS json
  Serial.println("mounting FS...");
  if (SPIFFS.begin(true)) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size + 1]);
        configFile.readBytes(buf.get(), size);
        buf[size] = '\0';

        DynamicJsonDocument json(2048);
        auto deserializeError = deserializeJson(json, buf.get());
        if (!deserializeError) {
          if (json["wifi_ssid"].is<const char*>()) strlcpy(wifi_ssid, json["wifi_ssid"], sizeof(wifi_ssid));
          if (json["wifi_pass"].is<const char*>()) strlcpy(wifi_pass, json["wifi_pass"], sizeof(wifi_pass));
          if (json["mqtt_server"].is<const char*>()) strlcpy(mqtt_server, json["mqtt_server"], sizeof(mqtt_server));
          if (json["mqtt_user"].is<const char*>()) strlcpy(mqtt_user, json["mqtt_user"], sizeof(mqtt_user));
          if (json["mqtt_pass"].is<const char*>()) strlcpy(mqtt_pass, json["mqtt_pass"], sizeof(mqtt_pass));
          if (json["house_name"].is<const char*>()) strlcpy(house_name, json["house_name"], sizeof(house_name));
          if (json["floor_name"].is<const char*>()) strlcpy(floor_name, json["floor_name"], sizeof(floor_name));
          if (json["room_name"].is<const char*>()) strlcpy(room_name, json["room_name"], sizeof(room_name));
          if (json["auto_topic"].is<int>()) auto_topic_enabled = (json["auto_topic"].as<int>() != 0);
          if (json["thermostat_id"].is<const char*>()) strlcpy(thermostat_id, json["thermostat_id"], sizeof(thermostat_id));
          if (json["rele_id"].is<const char*>()) strlcpy(rele_id, json["rele_id"], sizeof(rele_id));
          if (json["i2c_sda"].is<int>() && json["i2c_scl"].is<int>()) {
            int sda = json["i2c_sda"].as<int>();
            int scl = json["i2c_scl"].as<int>();
            if (sda != i2c_sda_pin || scl != i2c_scl_pin) {
              i2c_sda_pin = sda;
              i2c_scl_pin = scl;
              run_i2c_scan(i2c_sda_pin, i2c_scl_pin);
            }
          }
          if (json["buzzer"].is<int>()) {
            int bz = json["buzzer"].as<int>();
            buzzer_enabled = (bz > 0);
            if (bz > 0) buzzer_vol = (uint8_t)constrain(bz, 1, 3);
          }
          if (json["crono"].is<int>()) {
            crono_enabled = (json["crono"].as<int>() != 0);
            current_preset_mode = crono_enabled ? "schedule" : "none";
          }
          if (json["antifrost"].is<int>()) {
            antifrost_enabled = (json["antifrost"].as<int>() != 0);
          }
          if (json["openwin"].is<int>()) {
            open_window_enabled = (json["openwin"].as<int>() != 0);
          }
          if (json["antifrost_sp"].is<float>()) {
            antifrost_temp_threshold = constrain(json["antifrost_sp"].as<float>(), 3.0f, 10.0f);
          }
          if (json["hyst_en"].is<int>()) {
            hysteresis_enabled = (json["hyst_en"].as<int>() != 0);
          }
          if (json["hyst_val"].is<float>()) {
            hysteresis_val = constrain(json["hyst_val"].as<float>(), 0.1f, 2.0f);
          }
          if (json["pump_prot"].is<int>()) {
            pump_protection_enabled = (json["pump_prot"].as<int>() != 0);
          }
          if (json["night_en"].is<int>()) {
            night_mode_enabled = (json["night_en"].as<int>() != 0);
          }
          if (json["night_start"].is<int>()) {
            night_start_hour = constrain(json["night_start"].as<int>(), 0, 23);
          }
          if (json["night_end"].is<int>()) {
            night_end_hour = constrain(json["night_end"].as<int>(), 0, 23);
          }
          if (json["ext_win_en"].is<int>()) {
            ext_window_enabled = (json["ext_win_en"].as<int>() != 0);
          }
          if (json["ext_win_top"].is<const char*>()) {
            strncpy(ext_window_topic, json["ext_win_top"].as<const char*>(), sizeof(ext_window_topic) - 1);
          }
          if (json["sched"].is<JsonArray>()) {
            JsonArray sched = json["sched"].as<JsonArray>();
            if (sched.size() >= 16) {
              int k = 0;
              for (int d = 0; d < 2; d++) {
                for (int s = 0; s < 4; s++) {
                  crono_sched[d][s].start_min = constrain(sched[k++].as<int>(), 0, 1439);
                  crono_sched[d][s].temp      = constrain(sched[k++].as<float>(), 10.0f, 35.0f);
                }
              }
            }
          }
          if (json["runh"].is<JsonArray>()) {
            JsonArray runh = json["runh"].as<JsonArray>();
            if (runh.size() >= 1) runtime_today_sec = runh[0].as<uint32_t>();
            for (size_t i = 1; i < runh.size() && i <= 7; i++) {
              runtime_history_7d[i - 1] = runh[i].as<uint32_t>();
            }
          }
          if (json["c_day"].is<int>()) {
            current_day_of_year = json["c_day"].as<int>();
          }
        }
        configFile.close();
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }

  // Apply AZ-Touch MOD touch orientation & calibration settings
  apply_touch_settings();
  evaluate_crono(false);
  play_beep(2600, 30);

  // Fill initial temperature & humidity buffer
  fill_buffer();

  // Connect to WiFi or start AP portal (Termostato_AP -> 192.168.4.1)
  WiFi.mode(WIFI_STA);
  if (strlen(wifi_ssid) > 0) {
    Serial.printf("Connecting to saved WiFi SSID: %s\n", wifi_ssid);
    WiFi.begin(wifi_ssid, wifi_pass);
  } else {
    Serial.println("Attempting WiFi connection with NVS credentials...");
    WiFi.begin();
  }

  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 7000) {
    delay(200);
    gslc_Update(&m_gui);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi connected! IP: ");
    Serial.println(WiFi.localIP());
    gslc_ElemSetTxtStr(&m_gui, m_pElemOutTxt_Ip_Address16, WiFi.localIP().toString().c_str());
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer, "pool.ntp.org", "time.nist.gov");
    printLocalTime();
  } else {
    start_ap_portal();
  }
  setup_web_server();
  gslc_Update(&m_gui);

  client.begin(mqtt_server, net);
  client.onMessage(messageReceived);

  for(int i=0; i<17; i=i+8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }  

  ArduinoOTA.setHostname("ESP32_LCD_Thermostat");
  ArduinoOTA
    .onStart([]() {
      String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]\n", error);
    });

  if (WiFi.status() == WL_CONNECTED) {
    ArduinoOTA.begin();
  }
  previousMillis_lcd = millis();
}

// -----------------------------------
// Main event loop
// -----------------------------------
void loop()
{
  if (reset_config_reboot) {
    reset_config_reboot = false;
    if (!ap_mode_active) {
      // Prima pressione di RESET: attiva subito l'Access Point WiFi 192.168.4.1 senza perdere i dati
      Serial.println("Starting WiFi AP portal from RESET button...");
      start_ap_portal();
      gslc_SetPageCur(&m_gui, E_PG_MAIN);
    } else {
      // Seconda pressione mentre AP e' gia' attivo: formatta credenziali e riavvia
      Serial.println("Erasing WiFi settings and rebooting...");
      WiFi.disconnect(true, true);
      SPIFFS.remove("/config.json");
      delay(500);
      ESP.restart();
    }
  }

  if (ap_mode_active) {
    dnsServer.processNextRequest();
  }
  webServer.handleClient();

  static bool was_connected = false;
  if (WiFi.status() == WL_CONNECTED) {
    if (!was_connected) {
      was_connected = true;
      Serial.print("WiFi connected! IP: ");
      Serial.println(WiFi.localIP());
      gslc_ElemSetTxtStr(&m_gui, m_pElemOutTxt_Ip_Address16, WiFi.localIP().toString().c_str());
      ArduinoOTA.begin();
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer, "pool.ntp.org", "time.nist.gov");
      printLocalTime();
      if (ap_mode_active) {
        // Spegni AP dopo la connessione riuscita
        dnsServer.stop();
        WiFi.mode(WIFI_STA);
        ap_mode_active = false;
      }
    }
    ArduinoOTA.handle();
    client.loop();
    if (!client.connected()) {
      connect();
    }
  } else {
    was_connected = false;
  }

  // ------------------------------------------------
  // Update GUI Elements
  // ------------------------------------------------
  gslc_Update(&m_gui);

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis_temperature >= interval_temperature) {
    previousMillis_temperature = currentMillis;
    get_temp_print();
    if (bme_ok) {
      track_open_window(averageValueT.average());
    }
    if (client.connected()) {
      char myConcatenation[80];
      snprintf(myConcatenation, sizeof(myConcatenation), "%stemperature/target", thermostat_id);
      client.publish(myConcatenation, String(temp_setpoint,1));
    }
  }

  if (currentMillis - previousMillis_NTP >= interval_NTP) {
    previousMillis_NTP = currentMillis;
    if (WiFi.status() == WL_CONNECTED) {
      gslc_ElemSetTxtStr(&m_gui,m_pElemOutTxt_Ip_Address16,WiFi.localIP().toString().c_str());
      printLocalTime();
    } else if (ap_mode_active) {
      gslc_ElemSetTxtStr(&m_gui,m_pElemOutTxt_Ip_Address16,"AP:192.168.4.1");
    }
  }

  // Gestione Blocco Pulizia / Child Lock
  if (child_lock_active) {
    if (millis() < child_lock_until_ms) {
      int rem = (int)((child_lock_until_ms - millis() + 999) / 1000);
      static int last_lock_sec = -1;
      if (rem != last_lock_sec) {
        last_lock_sec = rem;
        char lockBuf[8];
        snprintf(lockBuf, sizeof(lockBuf), "LCK%2d", rem);
        gslc_ElemSetTxtStr(&m_gui, m_pElemOutTxt_stat, lockBuf);
      }
    } else {
      child_lock_active = false;
      publish_ha_state();
      play_beep(2600, 35);
      gslc_ElemSetTxtStr(&m_gui, m_pElemOutTxt_stat, (digitalRead(rele_pin) == HIGH) ? "ON " : "OFF");
    }
  }

  // Gestione Test Pompa Antiblocco
  if (pump_test_active) {
    if (millis() >= pump_test_end_ms) {
      pump_test_active = false;
      digitalWrite(rele_pin, LOW);
      publish_ha_state();
      play_beep(2200, 40);
      Serial.println("Protezione antiblocco: test pompa completato.");
      gslc_ElemSetTxtStr(&m_gui, m_pElemOutTxt_stat, "OFF");
    }
  }

  // Controllo automatico settimanale antiblocco pompa (ogni mercoledi ore 12:00)
  if (pump_protection_enabled && !pump_test_active && WiFi.status() == WL_CONNECTED) {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 20)) {
      if (timeinfo.tm_wday == 3 && timeinfo.tm_hour == 12 && timeinfo.tm_min == 0) {
        if (timeinfo.tm_yday != last_pump_test_yday) {
          last_pump_test_yday = timeinfo.tm_yday;
          trigger_pump_test(60);
        }
      }
    }
  }

  // Gestione Modalità Notte & Auto-Dimming Programmato
  if (night_mode_enabled && WiFi.status() == WL_CONNECTED) {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 20)) {
      int h = timeinfo.tm_hour;
      bool in_night = false;
      if (night_start_hour > night_end_hour) {
        in_night = (h >= night_start_hour || h < night_end_hour);
      } else if (night_start_hour < night_end_hour) {
        in_night = (h >= night_start_hour && h < night_end_hour);
      }
      night_active = in_night;
    }
  } else {
    night_active = false;
  }

  if (night_active && millis() >= night_wake_until_ms) {
    lcd_set_off();
    gslc_ElemSetVisible(&m_gui, m_pElemBtn_wake_up, true);
    gslc_ElemSetVisible(&m_gui, m_pElemBtn_wake_up2, true);
  } else if (currentMillis - previousMillis_lcd >= interval_lcd) {
    previousMillis_lcd = currentMillis;
    lcd_set_dim();
    gslc_ElemSetVisible(&m_gui, m_pElemBtn_wake_up, true);
    gslc_ElemSetVisible(&m_gui, m_pElemBtn_wake_up2, true);    
  }

  if (currentMillis - previousMillis_graph >= interval_graph) {
    previousMillis_graph = currentMillis;
    update_graph();
  }

  char buffer[10];
  dtostrf(temp_setpoint, 0, 1, buffer);
  gslc_ElemSetTxtStr(&m_gui,m_pElemOutTxt_Temp_Setpoint,buffer);

  // Aggiorna contatore tempo di funzionamento relè
  update_runtime_counter();

  if (currentMillis - previousMillis_rele >= interval_rele) {
    previousMillis_rele = currentMillis;       

    // Se il test pompa è attivo, salta il normale controllo del termostato
    if (pump_test_active) {
      gslc_ElemSetTxtStr(&m_gui, m_pElemOutTxt_stat, "PUMP");
      digitalWrite(rele_pin, HIGH);
      publish_ha_state();
      return;
    }

    // 1. Controllo Prioritario Protezione Antigelo: se T < soglia (con isteresi a +1.0 °C)
    if (bme_ok && antifrost_enabled) {
      if (averageValueT.average() <= antifrost_temp_threshold) {
        antifrost_active = true;
      } else if (averageValueT.average() >= (antifrost_temp_threshold + 1.0f)) {
        antifrost_active = false;
      }
    } else {
      antifrost_active = false;
    }

    if (antifrost_active) {
      Serial.println("ALLERTA: Antigelo attivo! Accensione forzata riscaldamento.");
      if (rele_id[0] != 0 && client.connected()) client.publish(rele_id, "ON");
      gslc_ElemSetTxtStr(&m_gui, m_pElemOutTxt_stat, "GELO");
      digitalWrite(rele_pin, HIGH);
      last_relay_active_time = currentMillis;
      if (m_pElemOutTxt_SetpointLbl != NULL) {
        char geloBuf[16];
        snprintf(geloBuf, sizeof(geloBuf), "ANTIGELO %.0fC", antifrost_temp_threshold);
        gslc_ElemSetTxtStr(&m_gui, m_pElemOutTxt_SetpointLbl, geloBuf);
      }
      publish_ha_state();
      return;
    }

    // 2. Controllo Finestra Aperta (sensore locale o esterno via MQTT): sospende riscaldamento/raffrescamento
    bool any_window_open = (open_window_enabled && open_window_active) || (ext_window_enabled && ext_window_open);
    if (any_window_open) {
      Serial.println("Finestra aperta: riscaldamento temporaneamente sospeso.");
      if (rele_id[0] != 0 && client.connected()) client.publish(rele_id, "OFF");
      gslc_ElemSetTxtStr(&m_gui, m_pElemOutTxt_stat, "WNDW");
      digitalWrite(rele_pin, LOW);
      publish_ha_state();
      return;
    }

    // Calcolo isteresi dinamica
    float eff_hyst = hysteresis_enabled ? hysteresis_val : 0.05f;
      
    if (stato == 1 && bme_ok){
      if (averageValueT.average() >= temp_setpoint){
        //stacca rele
        Serial.println("stacco rele");
        if (rele_id[0]!=0 && client.connected()) client.publish(rele_id, "OFF");
        if (!child_lock_active) gslc_ElemSetTxtStr(&m_gui,m_pElemOutTxt_stat,"OFF");
        digitalWrite(rele_pin, LOW);
      }
      else if(averageValueT.average() <= (temp_setpoint - eff_hyst)){   
        //attacca rele  
        Serial.println("attacco rele");
        if (rele_id[0]!=0 && client.connected()) client.publish(rele_id, "ON");
        if (!child_lock_active) gslc_ElemSetTxtStr(&m_gui,m_pElemOutTxt_stat,"ON ");
        digitalWrite(rele_pin, HIGH);
        last_relay_active_time = currentMillis;
      }
    }
        
    if (stato == 2 && bme_ok){
      if (averageValueT.average() <= temp_setpoint){
        //stacca rele
        Serial.println("stacco rele");
        if (rele_id[0]!=0 && client.connected()) client.publish(rele_id, "OFF");
        if (!child_lock_active) gslc_ElemSetTxtStr(&m_gui,m_pElemOutTxt_stat,"OFF");
        digitalWrite(rele_pin, LOW);
      }
      else if(averageValueT.average() >= (temp_setpoint + eff_hyst)){
        //attacca rele  
        Serial.println("attacco rele");
        if (rele_id[0]!=0 && client.connected()) client.publish(rele_id, "ON");
        if (!child_lock_active) gslc_ElemSetTxtStr(&m_gui,m_pElemOutTxt_stat,"ON ");
        digitalWrite(rele_pin, HIGH);
        last_relay_active_time = currentMillis;
      }
    }
    if (stato == 0){
      //stacca rele
      Serial.println("stacco rele");
      if (rele_id[0]!=0 && client.connected()) client.publish(rele_id, "OFF");
      if (!child_lock_active) gslc_ElemSetTxtStr(&m_gui,m_pElemOutTxt_stat,"---");
      digitalWrite(rele_pin, LOW);
    }
    publish_ha_state();
  }
}
