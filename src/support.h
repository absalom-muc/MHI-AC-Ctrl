#ifndef SUPPORT_h
#define SUPPORT_h
#include "MHI-AC-Ctrl-core.h"
#include "MHI-AC-Ctrl.h"

#define VERSION "2.5R21"

#define WIFI_SSID "your SSID"
#define WIFI_PASSWORD "your WiFi password"
#define HOSTNAME "MHI-AC-Ctrl"

#define UseStrongestAP true                         // when false then the first WiFi access point with matching SSID found is used.
                                                    // when true then the strongest WiFi access point with matching SSID found is used, and
                                                    // WiFi network re-scan every 12 minutes with alternate to +5dB stronger signal if detected

#define MQTT_SERVER "192.168.178.111"               // broker name or IP address of the broker
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

#define TEMP_MEASURE_PERIOD 0                       // period in seconds for temperature measurement with the external DS18x20 temperature sensor
                                                    // enter 0 if you don't use the DS18x20 
#define ONE_WIRE_BUS 4                              // D2, PIN for connecting temperature sensor DS18x20 DQ pin

// Per default AC internal sensor is used for the room temperature. You can select using the DS18x20 sensor or providing Troom via MQTT.
//#define ROOM_TEMP_DS18X20                         // use room temperature from DS18x20
//#define ROOM_TEMP_MQTT                            // use room temperature from received MQTT topic
#define ROOM_TEMP_MQTT_TIMEOUT  20                  // only considered if ROOM_TEMP_MQTT is defined
                                                    // time in seconds, after this time w/o receiving a valid room temperature
                                                    // via MQTT fallback to IU temperature sensor value

//#define POWERON_WHEN_CHANGING_MODE true           // uncomment it to switch on the AC when the mode (heat, cool, dry etc.) is changed
                                                    // used e.g. for home assistant support

// *** The configuration ends here ***
                                                    
#include <ESP8266WiFi.h>        // https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi
#include <PubSubClient.h>       // https://github.com/knolleary/pubsubclient
#include <ArduinoOTA.h>         // https://github.com/esp8266/Arduino/tree/master/libraries/ArduinoOTA

#if TEMP_MEASURE_PERIOD > 0
#include <OneWire.h>            // https://www.pjrc.com/teensy/td_libs_OneWire.html
#include <DallasTemperature.h>  // https://github.com/milesburton/Arduino-Temperature-Control-Library
#endif

#ifdef ROOM_TEMP_DS18X20
#if (TEMP_MEASURE_PERIOD == 0)
#error "You have to use a value>0 for TEMP_MEASURE_PERIOD when you want to use DS18x20 as an external temperature sensor"
#endif
#endif

#ifndef ROOM_TEMP_DS18X20
#ifndef ROOM_TEMP_MQTT
#define ROOM_TEMP_IU
#endif
#endif

#ifdef ROOM_TEMP_DS18X20
#ifdef ROOM_TEMP_MQTT
#error "It is not valid to select both ROOM_TEMP_DS18X20 and ROOM_TEMP_MQTT. Please select none or one of these options"
#endif
#endif

extern PubSubClient MQTTclient;

void initWiFi();
void setupWiFi();
int MQTTloop();
void MQTTreconnect();
void publish_cmd_ok();
void publish_cmd_unknown();
void publish_cmd_invalidparameter();
void output_P(ACStatus status, PGM_P topic, PGM_P payload);

void setupOTA();
void setup_ds18x20();
byte getDs18x20Temperature(int temp_hysterese);

#endif
