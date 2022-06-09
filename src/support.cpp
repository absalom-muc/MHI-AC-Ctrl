#include "support.h"
#include <Arduino.h>

WiFiClient espClient;
PubSubClient MQTTclient(espClient);
int WIFI_lost = 0;
int MQTT_lost = 0;


struct rising_edge_cnt_struct{
  volatile uint32_t SCK = 0;
  volatile uint32_t MOSI = 0;
  volatile uint32_t MISO = 0;
} rising_edge_cnt;


IRAM_ATTR void handleInterrupt_SCK() {
  rising_edge_cnt.SCK++;
}

IRAM_ATTR void handleInterrupt_MOSI() {
  rising_edge_cnt.MOSI++;
}

IRAM_ATTR void handleInterrupt_MISO() {
  rising_edge_cnt.MISO++;
}

void MeasureFrequency() {  // measure the frequency on the pins
  pinMode(SCK_PIN, INPUT);
  pinMode(MOSI_PIN, INPUT);
  pinMode(MISO_PIN, INPUT);
  Serial.println(F("Measure frequency for SCK, MOSI and MISO pin"));
  attachInterrupt(digitalPinToInterrupt(SCK_PIN), handleInterrupt_SCK, RISING);
  attachInterrupt(digitalPinToInterrupt(MOSI_PIN), handleInterrupt_MOSI, RISING);
  attachInterrupt(digitalPinToInterrupt(MISO_PIN), handleInterrupt_MISO, RISING);
  unsigned long starttimeMicros = micros();
  while (micros() - starttimeMicros < 1000000);
  detachInterrupt(SCK_PIN);
  detachInterrupt(MOSI_PIN);
  detachInterrupt(MISO_PIN);

  Serial.printf_P(PSTR("SCK frequency=%iHz (expected: >3000Hz) "), rising_edge_cnt.SCK);
  if (rising_edge_cnt.SCK > 3000)
    Serial.println(F("o.k."));
  else
    Serial.println(F("out of range!"));

  Serial.printf("MOSI frequency=%iHz (expected: <SCK frequency) ", rising_edge_cnt.MOSI);
  if ((rising_edge_cnt.MOSI > 30) & (rising_edge_cnt.MOSI < rising_edge_cnt.SCK))
    Serial.println(F("o.k."));
  else
    Serial.println(F("out of range!"));

  Serial.printf("MISO frequency=%iHz (expected: ~0Hz) ", rising_edge_cnt.MISO);
  if (rising_edge_cnt.MISO <= 10) {
    Serial.println(F("o.k."));
  }
  else {
    Serial.println(F("out of range!"));
    while (1);
  }
}


void initWiFi(){
  WiFi.persistent(false);
  WiFi.disconnect(true);    // Delete SDK wifi config
  delay(200);
  WiFi.mode(WIFI_STA);
  WiFi.hostname(HOSTNAME); 
  WiFi.setAutoReconnect(false);
}

int networksFound = 0;
void setupWiFi(int& WiFiStatus) {
  int max_rssi = -999;
  int strongest_AP = -1;
  static unsigned long WiFiTimeoutMillis;

  if(WiFiStatus != WIFI_CONNECT_ONGOING) {
    WiFi.scanDelete();
    int networksFound = WiFi.scanNetworks();
    for (int i = 0; i < networksFound; i++)
    {
      Serial.printf("%2d %25s %2d %ddBm %s %s %02x\n", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.BSSIDstr(i).c_str(), WiFi.encryptionType(i) == ENC_TYPE_NONE ? "open" : "secured"),  (uint)WiFi.encryptionType(i);
      if((strcmp(WiFi.SSID(i).c_str(), WIFI_SSID) == 0) && (WiFi.RSSI(i)>max_rssi)){
          max_rssi = WiFi.RSSI(i);
          strongest_AP = i;
      }
    }
    Serial.printf_P("current BSSID: %s, strongest BSSID: %s\n", WiFi.BSSIDstr().c_str(), WiFi.BSSIDstr(strongest_AP).c_str());
    if((WiFi.status() != WL_CONNECTED) || ((max_rssi > WiFi.RSSI() + 10) && (strcmp(WiFi.BSSIDstr().c_str(), WiFi.BSSIDstr(strongest_AP).c_str()) != 0))) {
      if(strongest_AP != -1) {
        Serial.printf("Connecting from bssid:%s to bssid:%s, channel:%i\n", WiFi.BSSIDstr().c_str(), WiFi.BSSIDstr(strongest_AP).c_str(), WiFi.channel(strongest_AP));
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WiFi.channel(strongest_AP), WiFi.BSSID(strongest_AP), true);
      }
      else {
        Serial.println("No matching AP found (maybe hidden SSID), however try to connect.");
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      }
      WiFiStatus = WIFI_CONNECT_ONGOING;
      Serial.println("WIFI_CONNECT_ONGOING");
      WiFiTimeoutMillis = millis();
    }
  }
  else {
    if(WiFi.status() == WL_CONNECTED){
      Serial.printf_P(PSTR(" connected to %s, IP address: %s (%ddBm)\n"), WIFI_SSID, WiFi.localIP().toString().c_str(), WiFi.RSSI());  // warum wird diese Zeile nicht ausgegeben?
      WiFiStatus = WIFI_CONNECT_OK;
      Serial.println("WIFI_CONNECT_OK");
    }
    else if(millis() - WiFiTimeoutMillis > 10*1000) {  // timeout after 10 seconds
      WiFiStatus = WIFI_CONNECT_TIMEOUT;
      Serial.println(PSTR("WIFI_CONNECT_TIMEOUT"));
    }
  }
}

int MQTTreconnect() {
  char strtmp[50];
  static int reconnect_trials=0;
  
  if(!MQTTclient.connected()) {
    Serial.print(F("Attempting MQTT connection..."));
    if(reconnect_trials++>9){
      WiFi.disconnect(); // work around for https://github.com/esp8266/Arduino/issues/7432
      reconnect_trials=0;
    }

    if (MQTTclient.connect(HOSTNAME, MQTT_USER, MQTT_PASSWORD, MQTT_PREFIX TOPIC_CONNECTED, 0, true, PAYLOAD_CONNECTED_FALSE)) {
      Serial.println(F(" connected"));
      reconnect_trials=0;
      output_P(status_connected, PSTR(TOPIC_CONNECTED), PSTR(PAYLOAD_CONNECTED_TRUE));
      output_P(status_connected, PSTR(TOPIC_VERSION), PSTR(VERSION));
      itoa(WiFi.RSSI(), strtmp, 10);
      output_P(status_rssi, PSTR(TOPIC_RSSI), strtmp);
      itoa(WIFI_lost, strtmp, 10);
      output_P(status_wifi_lost, PSTR(TOPIC_WIFI_LOST), strtmp);
      itoa(MQTT_lost, strtmp, 10);
      output_P(status_mqtt_lost, PSTR(TOPIC_MQTT_LOST), strtmp);
      WiFi.BSSIDstr().toCharArray(strtmp, 20);
      output_P(status_mqtt_lost, PSTR(TOPIC_WIFI_BSSID), strtmp);             // CHECK status_mqtt_lost !!!!!!!!!!!!

      // for testing publish list of access points with the expected SSID 
      for (int i = 0; i < networksFound; i++)
      {
        if(strcmp(WiFi.SSID(i).c_str(), WIFI_SSID) == 0){
          strcpy(strtmp, "BSSID:");
          strcat(strtmp, WiFi.BSSIDstr(i).c_str());
          char strtmp2[20];
          strcat(strtmp, " RSSI:");
          itoa(WiFi.RSSI(i), strtmp2, 10);
          strcat(strtmp, strtmp2);
          MQTTclient.publish(MQTT_PREFIX "APs", strtmp, true);
        }
      }

      itoa(rising_edge_cnt.SCK, strtmp, 10);
      output_P(status_fsck, PSTR(TOPIC_FSCK), strtmp);
      itoa(rising_edge_cnt.MOSI, strtmp, 10);
      output_P(status_fmosi, PSTR(TOPIC_FMOSI), strtmp);
      itoa(rising_edge_cnt.MISO, strtmp, 10);
      output_P(status_fmiso, PSTR(TOPIC_FMISO), strtmp);
      
      MQTTclient.subscribe(MQTT_SET_PREFIX "#");
      return MQTT_RECONNECTED;
    }
    else {
      Serial.print(F(" reconnect failed, reason "));
      itoa(MQTTclient.state(), strtmp, 10);
      Serial.print(strtmp);
      Serial.print(", WiFi status: ");
      Serial.println(WiFi.status());
      return MQTT_NOT_CONNECTED;
    }
  }
  MQTTclient.loop();
  return MQTT_CONNECTED;
}

void publish_cmd_ok() {
  output_P(status_cmd, PSTR(TOPIC_CMD_RECEIVED), PSTR(PAYLOAD_CMD_OK));
}
void publish_cmd_unknown() {
  output_P(status_cmd, PSTR(TOPIC_CMD_RECEIVED), PSTR(PAYLOAD_CMD_UNKNOWN));
}
void publish_cmd_invalidparameter() {
  output_P(status_cmd, PSTR(TOPIC_CMD_RECEIVED), PSTR(PAYLOAD_CMD_INVALID_PARAMETER));
}

void output_P(const ACStatus status, PGM_P topic, PGM_P payload) {
  const int mqtt_topic_size = 100;
  char mqtt_topic[mqtt_topic_size];
  
  Serial.printf_P(PSTR("status=%i topic=%s payload=%s\n"), status, topic, payload);
  
  if ((status & 0xc0) == type_status)
    strncpy_P(mqtt_topic, PSTR(MQTT_PREFIX), mqtt_topic_size);
  else if ((status & 0xc0) == type_opdata)
    strncpy_P(mqtt_topic, PSTR(MQTT_OP_PREFIX), mqtt_topic_size);
  else if ((status & 0xc0) == type_erropdata)
    strncpy_P(mqtt_topic, PSTR(MQTT_ERR_OP_PREFIX), mqtt_topic_size);
  strncat_P(mqtt_topic, topic, mqtt_topic_size - strlen(mqtt_topic));
  MQTTclient.publish_P(mqtt_topic, payload, true);
}

#if TEMP_MEASURE_PERIOD > 0
OneWire oneWire(ONE_WIRE_BUS);       // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass our oneWire reference to Dallas Temperature.
DeviceAddress insideThermometer;     // arrays to hold device address

byte getDs18x20Temperature(int temp_hysterese) {
  static unsigned long DS1820Millis = millis();
  static int16_t tempR_old = 0xffff;

  if (millis() - DS1820Millis > TEMP_MEASURE_PERIOD * 1000) {
    int16_t tempR = sensors.getTemp(insideThermometer);
    int16_t tempR_diff = tempR - tempR_old; // avoid using other functions inside the brackets of abs, see https://www.arduino.cc/reference/en/language/functions/math/abs/
    if (abs(tempR_diff) > temp_hysterese) {
      tempR_old = tempR;
      char strtmp[10];
      dtostrf(sensors.rawToCelsius(tempR), 0, 2, strtmp);
      //Serial.printf_P(PSTR("new DS18x20 temperature=%s°C\n"), strtmp);
      output_P(status_tds1820, PSTR(TOPIC_TDS1820), strtmp);
    }
    DS1820Millis = millis();
    sensors.requestTemperatures();
  }
  //Serial.printf_P(PSTR("temp DS18x20 tempR_old=%i %i\n"), tempR_old, (byte)(tempR_old/32 + 61));
  if(tempR_old < 0)
    return 0;
  return tempR_old/32 + 61;
}

void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16)
      Serial.print(F("0"));
    Serial.print(deviceAddress[i], HEX);
  }
}

void setup_ds18x20() {
  sensors.begin();
  Serial.printf_P(PSTR("Found %i DS18xxx family devices.\n"), sensors.getDS18Count());
  if (!sensors.getAddress(insideThermometer, 0))
    Serial.println(F("Unable to find address for Device 0"));
  else
    Serial.printf_P(PSTR("Device 0 Address: 0x%02x\n"), insideThermometer);
  sensors.setResolution(insideThermometer, 9); // set the resolution to 9 bit
  sensors.setWaitForConversion(false);
  sensors.requestTemperatures(); // Send the command to get temperatures
}
#endif

void setupOTA() {
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  if (strcmp(OTA_PASSWORD, "") != 0)
    ArduinoOTA.setPassword(OTA_PASSWORD);

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";
    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println(F("\nEnd"));
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf_P(PSTR("Progress: %u%%\n"), (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf_P(PSTR("Error[%u]: %i\n"), error);
    if (error == OTA_AUTH_ERROR)
      Serial.println(F("Auth Failed"));
    else if (error == OTA_BEGIN_ERROR)
      Serial.println(F("Begin Failed"));
    else if (error == OTA_CONNECT_ERROR)
      Serial.println(F("Connect Failed"));
    else if (error == OTA_RECEIVE_ERROR)
      Serial.println(F("Receive Failed"));
    else if (error == OTA_END_ERROR)
      Serial.println(F("End Failed"));
  });
  ArduinoOTA.begin();
  Serial.println(F("OTA Ready"));
}
