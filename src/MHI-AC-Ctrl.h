// Configuration for MHI-AC-Ctrl v1.4 by absalom-muc

#define WIFI_SSID "your SSID"
#define WIFI_PASSWORD "your WiFi password"
#define HOSTNAME "MHI-AC-Ctrl"

#define MQTT_SERVER "ds218p"                        // broker name or IP address of the broker
#define MQTT_PORT 1883                              // port number used by the broker
#define MQTT_USER ""                                // if authentication is not used, leave it empty
#define MQTT_PASSWORD ""                            // if authentication is not used, leave it empty
#define MQTT_PREFIX HOSTNAME "/"                    // basic prefix used for publishing AC data
#define MQTT_SET_PREFIX MQTT_PREFIX "set/"          // prefix for subscribing set commands
#define MQTT_OP_PREFIX MQTT_PREFIX "OpData/"        // prefix for publishing operating data
#define MQTT_ERR_OP_PREFIX MQTT_PREFIX "ErrOpData/" // prefix for publishing operating data from last error

#define TEMP_MEASURE_PERIOD 30 // period in seconds for temperature measurement, enter 0 if you don't use the DS1820 

//Values which will be published/read on MQTT
#define POWER_ON = "On";
#define POWER_OFF = "Off";
#define MODE_AUTO = "Auto";
#define MODE_DRY = "Dry";
#define MODE_COOL = "Cool";
#define MODE_FAN = "Fan";
#define MODE_HEAT = "Heat";
#define MODE_ERROR = "Stop";

// comment out the data you are not interested, but at least one row must be used.
const byte opdata[][5] {
  { 0xc0, 0x02, 0xff, 0xff, 0xff },  //  1 "MODE"
  { 0xc0, 0x05, 0xff, 0xff, 0xff },  //  2 "SET-TEMP"
  { 0xc0, 0x80, 0xff, 0xff, 0xff },  //  3 "RETURN-AIR" [°C]
  { 0xc0, 0x1f, 0xff, 0xff, 0xff },  //  8 "IU-FANSPEED"
  { 0xc0, 0x1e, 0xff, 0xff, 0xff },  // 12 "TOTAL-IU-RUN" [h]
  { 0x40, 0x80, 0xff, 0xff, 0xff },  // 21 "OUTDOOR" [°C]
  { 0x40, 0x11, 0xff, 0xff, 0xff },  // 24 "COMP" [Hz]
  { 0x40, 0x90, 0xff, 0xff, 0xff },  // 29 "CT" [A]
  { 0x40, 0xb1, 0xff, 0xff, 0xff },  // 32 "TDSH" [°C]
  { 0x40, 0x7c, 0xff, 0xff, 0xff },  // 33 "PROTECTION-No"
  { 0x40, 0x1f, 0xff, 0xff, 0xff },  // 34 "OU-FANSPEED"
  { 0x40, 0x0c, 0xff, 0xff, 0xff },  // 36 "DEFROST"
  { 0x40, 0x1e, 0xff, 0xff, 0xff },  // 37 "TOTAL-COMP-RUN" [h]
  { 0x40, 0x13, 0xff, 0xff, 0xff },  // 38 "OU-EEV" [Puls]
};


// ****************************************************************************************
// The following lines are for program internal purposes and shouldn't be touched
const int opdataCnt = sizeof(opdata) / sizeof(byte) / 5;

#include <ESP8266WiFi.h>        // https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi
#include <PubSubClient.h>       // https://github.com/knolleary/pubsubclient
#include <ArduinoOTA.h>         // https://github.com/esp8266/Arduino/tree/master/libraries/ArduinoOTA

#if TEMP_MEASURE_PERIOD > 0
#include <OneWire.h>            // https://www.pjrc.com/teensy/td_libs_OneWire.html
#include <DallasTemperature.h>  // https://github.com/milesburton/Arduino-Temperature-Control-Library

#define ONE_WIRE_BUS 4 // D2    // PIN for connecting DS18x20 DQ pin
OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass our oneWire reference to Dallas Temperature.
DeviceAddress insideThermometer; // arrays to hold device address

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}
#endif

WiFiClient espClient;
PubSubClient MQTTclient(espClient);


#define SCK 14
#define MOSI 13
#define MISO 12

// constants for the frame
#define SB0 0
#define SB1 SB0 + 1
#define SB2 SB0 + 2
#define DB0 SB2 + 1
#define DB1 SB2 + 2
#define DB2 SB2 + 3
#define DB3 SB2 + 4
#define DB4 SB2 + 5
#define DB6 SB2 + 7
#define DB9 SB2 + 10
#define DB10 SB2 + 11
#define DB11 SB2 + 12
#define DB12 SB2 + 13
#define DB14 SB2 + 15
#define CBH 18
#define CBL 19

void eval_op_mode(char mqtt_msg[], byte db10) {

  switch (db10 & 0x1f) {  // 0x10=Auto, 0x11=Dry, 0x12=Cold, 0x13=Fan, 0x14=heat
    case 0x10:
      if ((db10 & 0x30) == 0x30)
        strcpy(mqtt_msg, MODE_ERROR); // for error operating data
      else
        strcpy(mqtt_msg, MODE_AUTO); // for operating data
      break;
    case 0x11:
      strcpy(mqtt_msg, MODE_DRY);
      break;
    case 0x12:
      strcpy(mqtt_msg, MODE_COOL);
      break;
    case 0x13:
      strcpy(mqtt_msg, MODE_FAN);
      break;
    case 0x14:
      strcpy(mqtt_msg, MODE_HEAT);
      break;
  }
}


void setupOTA() {
  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

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
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    }
    else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    }
    else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    }
    else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    }
    else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.print("OTA Ready, IP address: ");
  Serial.println(WiFi.localIP());
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
  pinMode(SCK, INPUT);
  pinMode(MOSI, INPUT);
  pinMode(MISO, INPUT);
  Serial.println("Measure frequency for SCK, MOSI and MISO pin");
  attachInterrupt(digitalPinToInterrupt(SCK), handleInterrupt_SCK, RISING);
  attachInterrupt(digitalPinToInterrupt(MOSI), handleInterrupt_MOSI, RISING);
  attachInterrupt(digitalPinToInterrupt(MISO), handleInterrupt_SCK, RISING);
  unsigned long starttimeMillis = millis();
  while (millis() - starttimeMillis < 1000);
  detachInterrupt(SCK);
  detachInterrupt(MOSI);
  detachInterrupt(MISO);
  Serial.printf("SCK frequency=%iHz (expected: >3000Hz) ", rising_edge_cnt_SCK);
  if (rising_edge_cnt_SCK > 3000)
    Serial.println("o.k.");
  else
    Serial.println("out of range!");

  Serial.printf("MOSI frequency=%iHz (expected: <SCK frequency) ", rising_edge_cnt_MOSI);
  if (rising_edge_cnt_MOSI > 300 & rising_edge_cnt_MOSI < rising_edge_cnt_SCK)
    Serial.println("o.k.");
  else
    Serial.println("out of range!");

  Serial.printf("MISO frequency=%iHz (expected: ~0Hz) ", rising_edge_cnt_MISO);
  if (rising_edge_cnt_MISO >= 0 & rising_edge_cnt_MISO <= 10) {
    Serial.println("o.k.");
  }
  else {
    Serial.println("out of range!");
    while (1);
  }
}
