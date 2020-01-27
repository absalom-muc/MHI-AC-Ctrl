// MHI-AC-Ctrl v1.2 by absalom-muc
// read + write data via SPI controlled by MQTT

#include <ESP8266WiFi.h>
#include <PubSubClient.h> // see https://github.com/knolleary/pubsubclient
#include <ArduinoOTA.h>

#define SCK 14
#define MOSI 13
#define MISO 12

#define MQTT_SERVER "ds218p"
#define MQTT_PORT 1883
#define MQTT_PREFIX "MHI-AC-Ctrl/"
#define MQTT_SET_PREFIX MQTT_PREFIX "set/"

const char* ssid = "**********";
const char* password = "**********";
const char* hostname = "MHI-AC-Ctrl";

WiFiClient espClient;
PubSubClient MQTTclient(espClient);

bool sync = 0;
byte rx_SPIframe[20];
bool updateMQTTStatus=true;

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
  Serial.println("OTA Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void MQTTreconnect() {
  unsigned long runtimeMillisMQTT;
  Serial.println("MQTTreconnect");
  while (!MQTTclient.connected()) { // Loop until we're reconnected
    update_sync(false);
    Serial.print("Attempting MQTT connection...");
    if (MQTTclient.connect (hostname, MQTT_PREFIX "connected", 0, true, "false")) {
      Serial.println("connected");
      MQTTclient.publish(MQTT_PREFIX "connected", "true", true);
      MQTTclient.subscribe(MQTT_SET_PREFIX "#");
      updateMQTTStatus=true;
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(MQTTclient.state());
      Serial.println(" try again in 5 seconds");
      runtimeMillisMQTT = millis();
      while (millis() - runtimeMillisMQTT < 5000) { // Wait 5 seconds before retrying
        delay(0);
        ArduinoOTA.handle();
      }
    }
  }
}

void update_sync(bool sync_new) {
  if (sync_new != sync) {
    sync = sync_new;
    Serial.printf("sync=%i\n", sync);
    if (sync)
      MQTTclient.publish (MQTT_PREFIX "synced", "1", true);
    else 
      MQTTclient.publish (MQTT_PREFIX "synced", "0", true);
  }
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

bool set_Power = false;
byte new_Power;
bool set_Mode = false;
byte new_Mode;
bool set_Tsetpoint = false;
byte new_Tsetpoint;
bool set_Fan = false;
byte new_Fan;
bool set_Vanes = false;
byte new_Vanes;

void MQTT_subscribe_callback(char* topic, byte* payload, unsigned int length) {
  char payload_str[20];
  memcpy(payload_str, payload, length);
  payload_str[length] = '\0';

  if (strcmp(topic, MQTT_SET_PREFIX "Power") == 0) {
    new_Power = rx_SPIframe[DB0] | 0b11;
    if (strcmp(payload_str, "On") == 0) {
      set_Power = true;
      new_Power = 0b11;
      publish_cmd_ok();
    }
    else if (strcmp(payload_str, "Off") == 0) {
      set_Power = true;
      new_Power = 0b10;
      publish_cmd_ok();
    }
    else
      publish_cmd_invalidparameter();
  }
  else if (strcmp(topic, MQTT_SET_PREFIX "Mode") == 0) {
    if (strcmp(payload_str, "Auto") == 0) {
      set_Mode = true;
      new_Mode = 0b00100000;
      publish_cmd_ok();
    }
    else if (strcmp(payload_str, "Dry") == 0) {
      set_Mode = true;
      new_Mode = 0b00100100;
      publish_cmd_ok();
    }
    else if (strcmp(payload_str, "Cool") == 0) {
      set_Mode = true;
      new_Mode = 0b00101000;
      publish_cmd_ok();
    }
    else if (strcmp(payload_str, "Fan") == 0) {
      set_Mode = true;
      new_Mode = 0b00101100;
      publish_cmd_ok();
    }
    else if (strcmp(payload_str, "Heat") == 0) {
      set_Mode = true;
      new_Mode = 0b00110000;
      publish_cmd_ok();
    }
    else
      publish_cmd_invalidparameter();
  }
  else if (strcmp(topic, MQTT_SET_PREFIX "Tsetpoint") == 0) {
    new_Tsetpoint = atoi(payload_str);
    if ((new_Tsetpoint >= 18) & (new_Tsetpoint <= 30)) {
      set_Tsetpoint = true;
      publish_cmd_ok();
    }
    else
      publish_cmd_invalidparameter();
  }
  else if (strcmp(topic, MQTT_SET_PREFIX "Fan") == 0) {
    new_Fan = atoi(payload_str);
    if ((new_Fan >= 1) & (new_Fan <= 4)) {
      set_Fan = true;
      publish_cmd_ok();
    }
    else
      publish_cmd_invalidparameter();
  }
  else if (strcmp(topic, MQTT_SET_PREFIX "Vanes") == 0) {
    if(strcmp(payload_str, "Swing") == 0) {
      new_Vanes = 5;
      set_Vanes = true;
      publish_cmd_ok();
    }
    else {
      new_Vanes = atoi(payload_str);
      if ((new_Vanes >= 1) & (new_Vanes <= 5)) {
        set_Vanes = true;
        publish_cmd_ok();
      }
      else 
        publish_cmd_invalidparameter();  
    }
  }
  else
    publish_cmd_unknown();
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.printf("%lu:MHI-AC-Ctrl starting\n", millis());
  pinMode(SCK, INPUT_PULLUP);
  pinMode(MOSI, INPUT);
  pinMode(MISO, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);  // indicates that a frame was received, active low
  digitalWrite(LED_BUILTIN, HIGH);

  WiFi.hostname(hostname);
  WiFi.begin(ssid, password);
  Serial.println("");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  setupOTA();
  Serial.printf(" connected to %s, IP address: %s\n", ssid, WiFi.localIP().toString().c_str());
  MQTTclient.setServer(MQTT_SERVER, MQTT_PORT);
  MQTTclient.setCallback(MQTT_subscribe_callback);
  MQTTreconnect();
}

//                        sb0   sb1   sb2   db0   db1   db2   db3   db4   db5   db6   db7   db8   db9  db10  db11  db12  db13  db14  chkH  chkL
byte tx_SPIframe[20] = { 0xA9, 0x00, 0x07, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xFF, 0xFF, 0xFF, 0x0F, 0x04, 0x05, 0xF5 };
const byte frameVariant[3][9] {                                              { 0x40, 0x00, 0x00, 0x80, 0xFF, 0xFF, 0xFF, 0x0F, 0x04 },  //variant number 0
																				                                     { 0x80, 0x00, 0x00, 0x32, 0xD6, 0x01, 0x00, 0x0F, 0x04 },  //variant number 1
																				                                     { 0x80, 0x00, 0x00, 0xF1, 0xF7, 0xFF, 0xFF, 0x0F, 0x04 }}; //variant number 2

uint16_t calc_tx_checksum() {
  uint16_t checksum = 0;
  for (int i = 0; i < CBH; i++)
    checksum += tx_SPIframe[i];
  return checksum;
}

void loop() {
  uint8_t fan_old = 0xff;
  uint8_t vanes_old = 0xff;
  uint8_t power_old = 0xff;
  uint8_t mode_old = 0xff;
  uint8_t errorcode_old = 0xff;
  byte troom_old = 0xff;
  byte toutdoor_old = 0xff;
  byte tsetpoint_old = 0xff;
  char strtmp[10]; // for the MQTT strings to send
  byte payload_byte;
  unsigned long lastDatapacketMillis = 0;
  unsigned long SCKMillis;
  bool valid_datapacket_received = false;
  bool new_datapacket_received = false;
  uint16_t packet_cnt = 0;
  uint8_t repetitionNo = 0;
  uint8_t variantnumber = 0;
  uint8_t doubleframe = 1;
  uint8_t frame = 1;


  while (1) {

    SCKMillis = millis();
    while (millis() - SCKMillis < 5) { // wait for 5ms stable high signal
      delay(0);
      if (!digitalRead(SCK)) {
        SCKMillis = millis();
        if (millis() - lastDatapacketMillis > 35)
          update_sync(false);
      }
    }

    if (frame++ > 19) { // setup tx frame
      doubleframe++;  // toggle between 0 and 1
      tx_SPIframe[DB14] = doubleframe % 2;
      frame = 1;
      if (doubleframe % 2) {
        variantnumber = (variantnumber + 1) % 3;
        memcpy(&tx_SPIframe[DB6], &frameVariant[variantnumber][0], 9);

        tx_SPIframe[DB0] = 0;
        tx_SPIframe[DB1] = 0;
        tx_SPIframe[DB2] = 0;
       
        if (set_Power) {
          tx_SPIframe[DB0] = new_Power;
          set_Power = false;
        }

        if (set_Mode) {
          tx_SPIframe[DB0] = new_Mode;
          set_Mode = false;
        }

        if (set_Tsetpoint) {
          tx_SPIframe[DB2] = 2 * new_Tsetpoint | 0b10000000;
          set_Tsetpoint = false;
        }

        if (set_Fan) { // Kollidiert mit Variante 0 da dort db6=0x40
          if (new_Fan == 4) {
            tx_SPIframe[DB1] = 0b1010;
            tx_SPIframe[DB6] |= 0b00010000;
          }
          else
            tx_SPIframe[DB1] = (1 << 3) | (new_Fan - 1);
          set_Fan = false;
        }

        if (set_Vanes) {
          if (new_Vanes == 5) //5: swing
            tx_SPIframe[DB0] = 0b11000000;
          else { // when setting a new vanes position, swing is automatically disabled
            tx_SPIframe[DB1] = (1 << 7) | ((new_Vanes - 1) << 4);
          }
          set_Vanes = false;
        }
      }
    }

    uint16_t tx_checksum = calc_tx_checksum();
    tx_SPIframe[CBH] = highByte(tx_checksum);
    tx_SPIframe[CBL] = lowByte(tx_checksum);

    new_datapacket_received = false;
    uint16_t rx_checksum = 0;
    for (uint8_t byte_cnt = 0; byte_cnt < 20; byte_cnt++) { // read and write a data packet of 20 bytes
      payload_byte = 0;
      byte bit_mask = 1;
      for (uint8_t bit_cnt = 0; bit_cnt < 8; bit_cnt++) { // read and write 1 byte
        while (digitalRead(SCK)) {} // wait for falling edge
        if ((tx_SPIframe[byte_cnt] & bit_mask) > 0)
          digitalWrite(MISO, 1);
        else
          digitalWrite(MISO, 0);
        while (!digitalRead(SCK)) {} // wait for rising edge
        if (digitalRead(MOSI))
          payload_byte += bit_mask;
        bit_mask = bit_mask << 1;
      }

      if (rx_SPIframe[byte_cnt] != payload_byte) {
        new_datapacket_received = true;
        rx_SPIframe[byte_cnt] = payload_byte;
      }

      if (byte_cnt < 18)
        rx_checksum += payload_byte;
    }

    if ((rx_SPIframe[SB0] == 0x6c) & (rx_SPIframe[SB1] == 0x80) & (rx_SPIframe[SB2] == 0x04) & (rx_SPIframe[CBH] == highByte(rx_checksum)) & (rx_SPIframe[CBL] == lowByte(rx_checksum)))
      valid_datapacket_received = true;
    else
      update_sync(false);

    if (valid_datapacket_received) { // valid frame received
      packet_cnt++;
      repetitionNo++;
      valid_datapacket_received = false;
      if (millis() - lastDatapacketMillis < 60)
        update_sync(true);
      lastDatapacketMillis = millis();

      digitalWrite(LED_BUILTIN, HIGH);
      if (new_datapacket_received) {
        new_datapacket_received = false;
        /*MQTTclient.beginPublish (MQTT_PREFIX "raw", 43, true);  // raw data are usually not needed
        MQTTclient.write(highByte(packet_cnt));
        MQTTclient.write(lowByte(packet_cnt));
        MQTTclient.write(rx_SPIframe, 20);
        MQTTclient.write(tx_SPIframe, 20);
        MQTTclient.write(repetitionNo);
        MQTTclient.endPublish();*/
        repetitionNo = 0;
        if (updateMQTTStatus | ((rx_SPIframe[DB0] & 0x01) != power_old)) { // Power
          power_old = rx_SPIframe[DB0] & 0x01;
          if (power_old == 0)
            MQTTclient.publish(MQTT_PREFIX "Power", "Off", true);
          else
            MQTTclient.publish(MQTT_PREFIX "Power", "On", true);
        }

        if (updateMQTTStatus | ((rx_SPIframe[DB0] & 0x1c) != mode_old)) { // Mode
          mode_old = rx_SPIframe[DB0] & 0x1c;
          switch (mode_old) {
            case 0x00:
              MQTTclient.publish(MQTT_PREFIX "Mode", "Auto", true);
              break;
            case 0x04:
              MQTTclient.publish(MQTT_PREFIX "Mode", "Dry", true);
              break;
            case 0x08:
              MQTTclient.publish(MQTT_PREFIX "Mode", "Cool", true);
              break;
            case 0x0c:
              MQTTclient.publish(MQTT_PREFIX "Mode", "Fan", true);
              break;
            case 0x10:
              MQTTclient.publish(MQTT_PREFIX "Mode", "Heat", true);
              break;
            default:
              MQTTclient.publish(MQTT_PREFIX "Mode", "invalid", true);
              break;
          }
        }

        uint fantmp;
        if ((rx_SPIframe[DB6] & 0x40) != 0) // Fan status
          fantmp = 4;
        else
          fantmp = (rx_SPIframe[DB1] & 0x03) + 1;
        if (updateMQTTStatus | (fantmp != fan_old)) {
          fan_old = fantmp;
          itoa(fan_old, strtmp, 10);
          MQTTclient.publish(MQTT_PREFIX "Fan", strtmp, true);
        }

        // Only updated when Vanes command via wired RC
        uint vanestmp = (rx_SPIframe[DB0] & 0xc0) + ((rx_SPIframe[DB1] & 0xB0) >> 4);
        if (updateMQTTStatus | (vanestmp != vanes_old)) {
          if ((vanestmp & 0x88) == 0) // last vanes update was via IR-RC, so status is not known
            MQTTclient.publish(MQTT_PREFIX "Vanes", "?", true);
          else if ((vanestmp & 0x40) != 0) // Vanes status swing
            MQTTclient.publish(MQTT_PREFIX "Vanes", "Swing", true);
          else {
            switch (vanestmp & 0x03) {
              case 0x00:
                MQTTclient.publish(MQTT_PREFIX "Vanes", "1", true);  // up
                break;
              case 0x01:
                MQTTclient.publish(MQTT_PREFIX "Vanes", "2", true);
                break;
              case 0x02:
                MQTTclient.publish(MQTT_PREFIX "Vanes", "3", true);
                break;
              case 0x03:
                MQTTclient.publish(MQTT_PREFIX "Vanes", "4", true);  // down
                break;
            }
          }
          vanes_old = vanestmp;
        }

        if (updateMQTTStatus | (abs(rx_SPIframe[DB3] - troom_old) > 1)) { // Room temperature delta > 0.25°C
          troom_old = rx_SPIframe[DB3];
          float troom = (float)(troom_old - 61) / 4.0;
          dtostrf(troom, 0, 2, strtmp);
          MQTTclient.publish(MQTT_PREFIX "Troom", strtmp, true);
        }

        if (updateMQTTStatus | ((rx_SPIframe[DB2] & 0x7f) >> 1 != tsetpoint_old)) { // Temperature setpoint
          tsetpoint_old = (rx_SPIframe[DB2] & 0x7f) >> 1;
          itoa(tsetpoint_old, strtmp, 10);
          MQTTclient.publish(MQTT_PREFIX "Tsetpoint", strtmp, true);
        }

        if (updateMQTTStatus | (rx_SPIframe[DB4] != errorcode_old)) { // error code
          errorcode_old = rx_SPIframe[DB4];
          itoa(errorcode_old, strtmp, 10);
          MQTTclient.publish(MQTT_PREFIX "Errorcode", strtmp, true);
        }

        if (updateMQTTStatus | ((rx_SPIframe[DB9] == 0x80) & (rx_SPIframe[DB10] == 0x10))) { // indicates variant 0 => outdoor temperature
          if (abs(rx_SPIframe[DB11] - toutdoor_old) > 1) { // Outdoor temperature delta > 0.25°C to prevent toggling
            toutdoor_old = rx_SPIframe[DB11];
            float toutdoor = (float)(toutdoor_old - 94) / 4.0;
            dtostrf(toutdoor, 0, 2, strtmp);
            MQTTclient.publish(MQTT_PREFIX "Toutdoor", strtmp, true);
          }
        }
        updateMQTTStatus = false;
      } // if(new_datapacket_received)
    } // if(valid_datapacket_received)

    if (!MQTTclient.connected())
      MQTTreconnect();
    MQTTclient.loop();
    ArduinoOTA.handle();
  }
}
