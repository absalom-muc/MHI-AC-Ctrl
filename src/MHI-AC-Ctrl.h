#define SCK 14
#define MOSI 13
#define MISO 12

#define MQTT_SERVER "ds218p"      // broker name or IP address of the broker
#define MQTT_PORT 1883            // port number used by the broker
#define MQTT_USER ""              // if authentication is not used, leave it empty
#define MQTT_PASSWORD ""          // if authentication is not used, leave it empty
#define MQTT_PREFIX "MHI-AC-Ctrl/"          // basic prefix used for publishing AC data
#define MQTT_SET_PREFIX MQTT_PREFIX "set/"  // prefix for subscribing set commands
#define MQTT_OP_PREFIX MQTT_PREFIX "op/"    // prefix for publishing operating data

const char* ssid = "**********";
const char* password = "**********";
const char* hostname = "MHI-AC-Ctrl";

// comment out the data you are not interested, but at least one row must be used.
const byte opdata[][5] {
  { 0xc0, 0x80, 0xff, 0xff, 0xff },  //  3 "RETURN-AIR" [°C]
  { 0xc0, 0x1e, 0xff, 0xff, 0xff },  // 12 "TOTAL-IU-RUN" [h]
  { 0x40, 0x80, 0xff, 0xff, 0xff },  // 21 "OUTDOOR" [°C]
  { 0x40, 0x7c, 0xff, 0xff, 0xff },  // 33 "PROTECTION-No"
  { 0x40, 0x1e, 0xff, 0xff, 0xff },  // 37 "TOTAL-COMP-RUN" [h]
  { 0x40, 0x13, 0xff, 0xff, 0xff },  // 38 "OU-EEV" [Puls]
  };
const int opdataCnt = sizeof(opdata) / sizeof(byte) / 5;

// The following lines are for program internal purposes and shouldn't be touched

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


void setupOTA() {
  ArduinoOTA.setHostname(hostname);
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
    if (error == OTA_AUTH_ERROR) { Serial.println("Auth Failed"); }
    else if (error == OTA_BEGIN_ERROR) { Serial.println("Begin Failed"); }
    else if (error == OTA_CONNECT_ERROR) { Serial.println("Connect Failed"); }
    else if (error == OTA_RECEIVE_ERROR) { Serial.println("Receive Failed"); }
    else if (error == OTA_END_ERROR) { Serial.println("End Failed"); }
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
  while(millis() - starttimeMillis < 1000);
  detachInterrupt(SCK);
  detachInterrupt(MOSI);
  detachInterrupt(MISO);
  Serial.printf("SCK frequency=%iHz (expected: 3000 .. 3200Hz) ", rising_edge_cnt_SCK);
  if(rising_edge_cnt_SCK > 3000 & rising_edge_cnt_SCK <= 3200){
      Serial.println("o.k.");
  }
  else{
      Serial.println("out of range!");
      while(1);
  }
  
  Serial.printf("MOSI frequency=%iHz (expected: 300 .. 3000Hz) ", rising_edge_cnt_MOSI);
  if(rising_edge_cnt_MOSI > 300 & rising_edge_cnt_MOSI <= 3000){
      Serial.println("o.k.");
  }
  else{
      Serial.println("out of range!");
      while(1);
  }
  
  Serial.printf("MISO frequency=%iHz (expected: ~0Hz) ", rising_edge_cnt_MISO);
  if(rising_edge_cnt_MISO >= 0 & rising_edge_cnt_MISO <= 100){
      Serial.println("o.k.");
  }
  else{
      Serial.println("out of range!");
      while(1);
  }
}
