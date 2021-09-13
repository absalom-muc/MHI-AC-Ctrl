#ifndef SUPPORT_h
#define SUPPORT_h
#include "MHI-AC-Ctrl-core.h"
#include "MHI-AC-Ctrl.h"

#define MQTT_SET_PREFIX "set/"          // prefix for subscribing set commands, must end with a "/"
#define MQTT_OP_PREFIX "OpData/"        // prefix for publishing operating data, must end with a "/"
#define MQTT_ERR_OP_PREFIX "ErrOpData/" // prefix for publishing operating data from last error, must end with a "/"
#define MQTT_DEFAULT_PORT "1883"

#define OTA_PASSWORD ""                             // Enter an OTA password if required

#define ONE_WIRE_BUS 4                              // D2, PIN for connecting temperature sensor DS18x20 DQ pin
#define TEMP_MEASURE_PERIOD 0                       // period in seconds for temperature measurement with the external DS18x20 temperature sensor
                          													// set to e.g. 30 to read the sensor every 30 seconds.

//#define POWERON_WHEN_CHANGING_MODE              // uncomment it to switch on the AC when the mode (heat, cool, dry etc.) is changed
                                                    // used e.g. for home assistant support

#include <ESP8266WiFi.h>        // https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi
#include <PubSubClient.h>       // https://github.com/knolleary/pubsubclient
#include <ArduinoOTA.h>         // https://github.com/esp8266/Arduino/tree/master/libraries/ArduinoOTA

#if TEMP_MEASURE_PERIOD > 0
#include <OneWire.h>            // https://www.pjrc.com/teensy/td_libs_OneWire.html
#include <DallasTemperature.h>  // https://github.com/milesburton/Arduino-Temperature-Control-Library
#endif

extern PubSubClient MQTTclient;

void saveConfigCallback();
void setupWiFi();
void setupMQTT();
void readConfig();
void saveConfig();
int MQTTloop();
void MQTTreconnect();
void publish_cmd_ok();
void publish_cmd_unknown();
void publish_cmd_invalidparameter();
void output_P(ACStatus status, PGM_P topic, PGM_P payload);

void setupOTA();
void setup_ds18x20();
void getDs18x20Temperature(int temp_hysterese);

#endif
