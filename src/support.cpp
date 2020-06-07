#include "support.h"
#include <Arduino.h>

WiFiClient espClient;
PubSubClient MQTTclient(espClient);

void setupWiFi() {
  WiFi.hostname(HOSTNAME);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print(F("Attempting WiFi connection ..."));
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.printf(" connected to %s, IP address: %s\n", WIFI_SSID, WiFi.localIP().toString().c_str());
}

void MQTTreconnect() {
  unsigned long runtimeMillisMQTT;
  char strtmp[10];
  while (!MQTTclient.connected()) { // Loop until we're reconnected
    Serial.print("Attempting MQTT connection...");
    if (MQTTclient.connect(HOSTNAME, MQTT_USER, MQTT_PASSWORD, MQTT_PREFIX "connected", 0, true, "0")) {
      Serial.println(F(" connected"));
      output(status_connected, PSTR("connected"), PSTR("1"));
      itoa(WiFi.RSSI(), strtmp, 10);
      output(status_rssi, PSTR("RSSI"), strtmp);
      MQTTclient.subscribe(MQTT_SET_PREFIX "#");
    }
    else {
      Serial.println(F(" reconnect failed"));
      itoa(MQTTclient.state(), strtmp, 10);
      Serial.println(F("try again in 5 seconds"));
      runtimeMillisMQTT = millis();
      while (millis() - runtimeMillisMQTT < 5000) { // Wait 5 seconds before retrying
        delay(0);
        ArduinoOTA.handle();
      }
    }
  }
}

int MQTTloop() {
  if (!MQTTclient.connected()) {
    MQTTreconnect();
    return 1;         // 1 means that it just reconnected
  }
  MQTTclient.loop();
  return 0;
}

void publish_cmd_ok() {
  MQTTclient.publish(MQTT_PREFIX "cmd_received", "o.k.");
}
void publish_cmd_unknown() {
  MQTTclient.publish(MQTT_PREFIX "cmd_received", "unknown command");
}
void publish_cmd_invalidparameter() {
  MQTTclient.publish(MQTT_PREFIX "cmd_received", "invalid parameter");
}

void output(ACStatus status, const char* topic, const char* payload) {
  char mqtt_topic[100];
  Serial.print("status=");
  Serial.print(status);
  Serial.print(" topic=");
  Serial.print(topic);
  Serial.print(" payload=");
  Serial.println(payload);
  
  if ((status & 0xc0) == type_status)
    strcpy(mqtt_topic, MQTT_PREFIX);
  else if ((status & 0xc0) == type_opdata)
    strcpy(mqtt_topic, MQTT_OP_PREFIX);
  else if ((status & 0xc0) == type_erropdata)
    strcpy(mqtt_topic, MQTT_ERR_OP_PREFIX);
  strcat_P(mqtt_topic, topic);
  MQTTclient.publish_P(mqtt_topic, payload, true);
}

#if TEMP_MEASURE_PERIOD > 0
OneWire oneWire(ONE_WIRE_BUS);       // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass our oneWire reference to Dallas Temperature.
DeviceAddress insideThermometer;     // arrays to hold device address

void getDs18x20Temperature(int temp_hysterese) {
  static unsigned long DS1820Millis = millis();
  static int16_t tempR_old = 0xffff;

  if (millis() - DS1820Millis > TEMP_MEASURE_PERIOD * 1000) {
    const int16_t tempR = sensors.getTemp(insideThermometer);
    const int16_t tempR_diff = tempR - tempR_old; // avoid using other functions inside the brackets of abs, see https://www.arduino.cc/reference/en/language/functions/math/abs/
    if (abs(tempR_diff) > temp_hysterese) {
      tempR_old = tempR;
      char strtmp[10];
      dtostrf(sensors.rawToCelsius(tempR), 0, 2, strtmp);
      Serial.printf("new DS18x20 temperature=%s\n", strtmp);
      MQTTclient.publish(MQTT_PREFIX "Tds1820", strtmp, true);
    }
    DS1820Millis = millis();
    sensors.requestTemperatures();
  }
}

void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16)
      Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

void setup_ds18x20() {
  sensors.begin();
  Serial.print("Found ");
  Serial.print(sensors.getDS18Count(), DEC);
  Serial.println(" DS18xxx family devices.");
  if (!sensors.getAddress(insideThermometer, 0))
    Serial.println("Unable to find address for Device 0");
  Serial.print("Device 0 Address: ");
  printAddress(insideThermometer);
  Serial.println();
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
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)
      Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR)
      Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR)
      Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR)
      Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR)
      Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA Ready");
}

uint rising_edge_cnt_SCK = 0;
ICACHE_RAM_ATTR void handleInterrupt_SCK() {
  rising_edge_cnt_SCK++;
}

uint rising_edge_cnt_MOSI = 0;
ICACHE_RAM_ATTR void handleInterrupt_MOSI() {
  rising_edge_cnt_MOSI++;
}

uint rising_edge_cnt_MISO = 0;
ICACHE_RAM_ATTR void handleInterrupt_MISO() {
  rising_edge_cnt_MISO++;
}

void MeasureFrequency() {  // measure the frequency on the pins
  char strtmp[10];
  pinMode(SCK, INPUT);
  pinMode(MOSI, INPUT);
  pinMode(MISO, INPUT);
  Serial.println("Measure frequency for SCK, MOSI and MISO pin");
  attachInterrupt(digitalPinToInterrupt(SCK), handleInterrupt_SCK, RISING);
  attachInterrupt(digitalPinToInterrupt(MOSI), handleInterrupt_MOSI, RISING);
  attachInterrupt(digitalPinToInterrupt(MISO), handleInterrupt_MISO, RISING);
  unsigned long starttimeMillis = millis();
  while (millis() - starttimeMillis < 1000);
  detachInterrupt(SCK);
  detachInterrupt(MOSI);
  detachInterrupt(MISO);

  itoa(rising_edge_cnt_SCK, strtmp, 10);
  MQTTclient.publish(MQTT_PREFIX "fSCK", strtmp, true);
  itoa(rising_edge_cnt_MOSI, strtmp, 10);
  MQTTclient.publish(MQTT_PREFIX "fMOSI", strtmp, true);
  itoa(rising_edge_cnt_MISO, strtmp, 10);
  MQTTclient.publish(MQTT_PREFIX "fMISO", strtmp, true);

  Serial.printf("SCK frequency=%iHz (expected: >3000Hz) ", rising_edge_cnt_SCK);
  if (rising_edge_cnt_SCK > 3000)
    Serial.println("o.k.");
  else
    Serial.println("out of range!");

  Serial.printf("MOSI frequency=%iHz (expected: <SCK frequency) ", rising_edge_cnt_MOSI);
  if ((rising_edge_cnt_MOSI > 30) & (rising_edge_cnt_MOSI < rising_edge_cnt_SCK))
    Serial.println("o.k.");
  else
    Serial.println("out of range!");

  Serial.printf("MISO frequency=%iHz (expected: ~0Hz) ", rising_edge_cnt_MISO);
  if (rising_edge_cnt_MISO <= 10) {
    Serial.println("o.k.");
  }
  else {
    Serial.println("out of range!");
    while (1);
  }
}
