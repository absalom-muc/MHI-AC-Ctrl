#include <FS.h>                   //this needs to be first, or it all crashes and burns...
//#include "SPIFFS.h"
#include "support.h"
#include <Arduino.h>
#include "WiFiManager.h"
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

//flag for saving data
bool shouldSaveConfig = false;

// Custom wifimanager vars
char host_name[40];
char mqtt_server[40];
char mqtt_port[6];
char mqtt_user[40];
char mqtt_pass[40];

char mqtt_prefix[80];
char mqtt_prefix_set[80];
char mqtt_prefix_opdata[80];
char mqtt_prefix_error[80];

WiFiClient espClient;
PubSubClient MQTTclient(espClient);

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void setupWiFi() {
  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_host_name("host", "Host name", host_name, 40);
  WiFiManagerParameter custom_mqtt_server("server", "MQTT server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "MQTT port", mqtt_port, 6);
  WiFiManagerParameter custom_mqtt_user("user", "MQTT user", mqtt_user, 40);
  WiFiManagerParameter custom_mqtt_pass("pass", "MQTT pass", mqtt_pass, 40);

  // WiFiManager
  // Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  
  // Uncomment and run it once, if you want to erase all the stored information
//  wifiManager.resetSettings();
  
  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //set static ip
  //  wifiManager.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  
  //add all your parameters here
  wifiManager.addParameter(&custom_host_name);  
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_user);
  wifiManager.addParameter(&custom_mqtt_pass);

  // set custom ip for portal
  //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  // fetches ssid and pass from eeprom and tries to connect
  // if it does not connect it starts an access point with the specified name
  // here  "AutoConnectAP"
  // and goes into a blocking loop awaiting configuration
  // wifiManager.autoConnect("AutoConnectAP");
  // or use this for auto generated name ESP + ChipID
  wifiManager.autoConnect();

    //Read updated parameters
  strcpy(host_name, custom_host_name.getValue());
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(mqtt_user, custom_mqtt_user.getValue());
  strcpy(mqtt_pass, custom_mqtt_pass.getValue());

  Serial.print("Connected to wifi, setting host name to: ");
  Serial.println(host_name);
  
  WiFi.hostname(host_name);

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    saveConfig();
  }
 
}

void readConfig() {
  //clean FS, for testing
  //SPIFFS.format();
  
  // default values
  strcpy(mqtt_port, MQTT_DEFAULT_PORT);
  
  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonDocument json(1024);
        DeserializationError error = deserializeJson(json, buf.get());
        serializeJson(json, Serial);
        if (!error) {
          Serial.println("\nparsed json");
          strcpy(host_name, json["host_name"]);
          strcpy(mqtt_server, json["mqtt_server"]);
          strcpy(mqtt_port, json["mqtt_port"]);
          strcpy(mqtt_user, json["mqtt_user"]);
          strcpy(mqtt_pass, json["mqtt_pass"]);
        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read
}

void saveConfig() {
    Serial.println("saving config");
    DynamicJsonDocument json(1024);
    json["host_name"] = host_name;
    json["mqtt_server"] = mqtt_server;
    json["mqtt_port"] = mqtt_port;
    json["mqtt_user"] = mqtt_user;
    json["mqtt_pass"] = mqtt_pass;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    serializeJson(json, Serial);
    serializeJson(json, configFile);
    configFile.close();
}

void setupMQTT() {
  Serial.print("Setting up MQTT at: ");
  Serial.println(mqtt_server);
  
  sprintf(mqtt_prefix, "%s/", host_name);
  sprintf(mqtt_prefix_set, "%s%s", mqtt_prefix, MQTT_SET_PREFIX);
  sprintf(mqtt_prefix_opdata, "%s%s", mqtt_prefix, MQTT_OP_PREFIX);
  sprintf(mqtt_prefix_error, "%s%s", mqtt_prefix, MQTT_ERR_OP_PREFIX);

  MQTTclient.setServer(mqtt_server, atoi(mqtt_port));
}

void MQTTreconnect() {
  unsigned long runtimeMillisMQTT;
  char tempString[80];
  sprintf(tempString, "%s%s", mqtt_prefix, TOPIC_CONNECTED);
  while (!MQTTclient.connected()) { // Loop until we're reconnected
    Serial.print(F("Attempting MQTT connection..."));
    if (MQTTclient.connect(mqtt_server, mqtt_user, mqtt_pass, tempString, 0, true, PAYLOAD_CONNECTED_FALSE)) {
      Serial.println(F(" connected"));
      output_P(status_connected, PSTR(TOPIC_CONNECTED), PSTR(PAYLOAD_CONNECTED_TRUE));
      itoa(WiFi.RSSI(), tempString, 10);
      output_P(status_rssi, PSTR(TOPIC_RSSI), tempString);
      sprintf(tempString, "%s#", mqtt_prefix_set);
      MQTTclient.subscribe(tempString);
    }
    else {
      Serial.println(F(" reconnect failed"));
      itoa(MQTTclient.state(), tempString, 10);
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
    strncpy(mqtt_topic, mqtt_prefix, mqtt_topic_size);
  else if ((status & 0xc0) == type_opdata)
    strncpy(mqtt_topic, mqtt_prefix_opdata, mqtt_topic_size);
  else if ((status & 0xc0) == type_erropdata)
    strncpy(mqtt_topic, mqtt_prefix_error, mqtt_topic_size);
    
  strncat_P(mqtt_topic, topic, mqtt_topic_size - strlen(mqtt_topic));
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
    int16_t tempR = sensors.getTemp(insideThermometer);
    int16_t tempR_diff = tempR - tempR_old; // avoid using other functions inside the brackets of abs, see https://www.arduino.cc/reference/en/language/functions/math/abs/
    if (abs(tempR_diff) > temp_hysterese) {
      tempR_old = tempR;
      char strtmp[10];
      dtostrf(sensors.rawToCelsius(tempR), 0, 2, strtmp);
      Serial.printf_P(PSTR("new DS18x20 temperature=%s°C\n"), strtmp);
      output_P(status_tds1820, PSTR(TOPIC_TDS1820), strtmp);
    }
    DS1820Millis = millis();
    sensors.requestTemperatures();
  }
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
  ArduinoOTA.setHostname(host_name);
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
