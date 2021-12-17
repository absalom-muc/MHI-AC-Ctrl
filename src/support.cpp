#include "support.h"
#include <Arduino.h>

WiFiClient espClient;
PubSubClient MQTTclient(espClient);
int WIFI_lost = 0;
int MQTT_lost = 0;

void initWiFi(){
  WiFi.persistent(false);
  WiFi.disconnect(true);    // Delete SDK wifi config
  delay(200);
  WiFi.mode(WIFI_STA);
  WiFi.hostname(HOSTNAME); 
#if UseStrongestAP==true
  WiFi.setAutoReconnect(false);
#else
  WiFi.setAutoReconnect(true);
#endif
}

int networksFound;
void setupWiFi() {
#if UseStrongestAP==true
  int max_rssi = -999;
  int strongest_AP = -1;
  WiFi.scanDelete();
  networksFound = WiFi.scanNetworks();
  for (int i = 0; i < networksFound; i++)
  {
    Serial.printf("%d: %s, Ch:%d (%ddBm) BSSID:%s %s\n", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.BSSIDstr(i).c_str(), WiFi.encryptionType(i) == ENC_TYPE_NONE ? "open" : "");
    if((strcmp(WiFi.SSID(i).c_str(), WIFI_SSID) == 0) && (WiFi.RSSI(i)>max_rssi)){
        max_rssi = WiFi.RSSI(i);
        strongest_AP = i;
    }
  }
  Serial.printf_P("current BSSID: %s, strongest BSSID: %s\n", WiFi.BSSIDstr().c_str(), WiFi.BSSIDstr(strongest_AP).c_str());
  if((WiFi.status() != WL_CONNECTED) || ((max_rssi > WiFi.RSSI() + 6) && (strcmp(WiFi.BSSIDstr().c_str(), WiFi.BSSIDstr(strongest_AP).c_str()) != 0))) {
    Serial.printf("Connecting from bssid:%s to bssid:%s, ", WiFi.BSSIDstr().c_str(), WiFi.BSSIDstr(strongest_AP).c_str());
    Serial.printf("channel:%i\n", WiFi.channel(strongest_AP));
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WiFi.channel(strongest_AP), WiFi.BSSID(strongest_AP), true);
  }
#else
  Serial.print(F("Attempting WiFi connection ..."));
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
#endif

  unsigned long previousMillis = millis();
  while ((WiFi.status() != WL_CONNECTED) && (millis() - previousMillis < 10*1000)) {
    delay(500);
    Serial.print(F("."));
  }
  if(WiFi.status() == WL_CONNECTED)
    Serial.printf_P(PSTR(" connected to %s, IP address: %s, BSSID: %s\n"), WIFI_SSID, WiFi.localIP().toString().c_str(), WiFi.BSSIDstr().c_str());
  else
    Serial.printf_P(PSTR(" not connected, timeout!\n"));
}

void MQTTreconnect() {
  unsigned long runtimeMillisMQTT;
  char strtmp[50];
  while (!MQTTclient.connected()) { // Loop until we're reconnected
    Serial.print(F("Attempting MQTT connection..."));
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println(F("WiFi not connected in MQTTreconnect function"));
        #if UseStrongestAP==true
          setupWiFi();
        #endif
      }
    if (MQTTclient.connect(HOSTNAME, MQTT_USER, MQTT_PASSWORD, MQTT_PREFIX TOPIC_CONNECTED, 0, true, PAYLOAD_CONNECTED_FALSE)) {
      Serial.println(F(" connected"));
      output_P(status_connected, PSTR(TOPIC_CONNECTED), PSTR(PAYLOAD_CONNECTED_TRUE));
      itoa(WiFi.RSSI(), strtmp, 10);
      output_P(status_rssi, PSTR(TOPIC_RSSI), strtmp);
      itoa(WIFI_lost, strtmp, 10);
      output_P(status_wifi_lost, PSTR(TOPIC_WIFI_LOST), strtmp);
      itoa(MQTT_lost, strtmp, 10);
      output_P(status_mqtt_lost, PSTR(TOPIC_MQTT_LOST), strtmp);
      WiFi.BSSIDstr().toCharArray(strtmp, 20);
      output_P(status_mqtt_lost, PSTR(TOPIC_WIFI_BSSID), strtmp);             // CHECK status_mqtt_lost !!!!!!!!!!!!
      // for testing publish list of access points with the expected WIFI_SSID 
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
      
      MQTTclient.subscribe(MQTT_SET_PREFIX "#");
    }
    else {
      Serial.print(F(" reconnect failed, reason "));
      itoa(MQTTclient.state(), strtmp, 10);
      Serial.print(strtmp);
      Serial.println(F(" try again in 5 seconds"));
      Serial.print("WiFi status: ");
      Serial.println(WiFi.status());
      runtimeMillisMQTT = millis();
      MQTTclient.disconnect();
      while (millis() - runtimeMillisMQTT < 5000) { // Wait 5 seconds before retrying
        delay(0);
        ArduinoOTA.handle();
      }
    }
  }
}

int MQTTloop() {
  if(WiFi.status() != WL_CONNECTED)
    WIFI_lost++;

  if (!MQTTclient.connected()) {
    MQTT_lost++;
    MQTTreconnect();
    return 1;         // 1 means that it just reconnected
  }
  MQTTclient.loop();
  return 0;
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
