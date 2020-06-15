#ifndef SUPPORT_h
#define SUPPORT_h
#include "MHI-AC-Ctrl-core.h"

#define WIFI_SSID "your SSID"
#define WIFI_PASSWORD "your WiFi password"
#define HOSTNAME "MHI-AC-Ctrl"

#define MQTT_SERVER "ds218p"                        // broker name or IP address of the broker
#define MQTT_PORT 1883                              // port number used by the broker
#define MQTT_USER ""                                // if authentication is not used, leave it empty
#define MQTT_PASSWORD ""                            // if authentication is not used, leave it empty
#define MQTT_PREFIX HOSTNAME "/"                    // basic prefix used for publishing AC data (e.g. for status),
                                                    // replace "/" by e.g. "/Living-Room/" when you have multiple ACs
#define MQTT_SET_PREFIX MQTT_PREFIX "set/"          // prefix for subscribing set commands, must end with a "/"
#define MQTT_OP_PREFIX MQTT_PREFIX "OpData/"        // prefix for publishing operating data, must end with a "/"
#define MQTT_ERR_OP_PREFIX MQTT_PREFIX "ErrOpData/" // prefix for publishing operating data from last error, must end with a "/"

#define OTA_HOSTNAME HOSTNAME                       // default for the OTA_HOSTNAME is the HOSTNAME
#define OTA_PASSWORD ""                             // Enter an OTA password if required

#define ONE_WIRE_BUS 4                              // D2, PIN for connecting temperature sensor DS18x20 DQ pin
#define TEMP_MEASURE_PERIOD 30                      // period in seconds for temperature measurement with the external DS18x20 temperature sensor
                                                    // enter 0 if you don't use the DS18x20 

#include <ESP8266WiFi.h>        // https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi
#include <PubSubClient.h>       // https://github.com/knolleary/pubsubclient
#include <ArduinoOTA.h>         // https://github.com/esp8266/Arduino/tree/master/libraries/ArduinoOTA

#if TEMP_MEASURE_PERIOD > 0
#include <OneWire.h>            // https://www.pjrc.com/teensy/td_libs_OneWire.html
#include <DallasTemperature.h>  // https://github.com/milesburton/Arduino-Temperature-Control-Library
#endif

// pin defintions
#define SCK_PIN  14
#define MOSI_PIN 13
#define MISO_PIN 12

extern PubSubClient MQTTclient;

void setupWiFi();
int MQTTloop();
void MQTTreconnect();
void publish_cmd_ok();
void publish_cmd_unknown();
void publish_cmd_invalidparameter();
void output(ACStatus status, const char* topic, const char* payload);

void setupOTA();
void setup_ds18x20();
void getDs18x20Temperature(int temp_hysterese);
void MeasureFrequency();

#endif
