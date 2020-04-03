// MHI-AC-Ctrl v1.4 by absalom-muc
// read + write data via SPI controlled by MQTT
#include "MHI-AC-Ctrl.h"

bool sync = 0;
byte rx_SPIframe[20];
bool updateMQTTStatus=true;
uint updateMQTTStatus_opdata=opdataCnt;

void MQTTreconnect() {
  unsigned long runtimeMillisMQTT;
  char strtmp[10];
  while (!MQTTclient.connected()) { // Loop until we're reconnected
    update_sync(false);
    Serial.print("Attempting MQTT connection...");
    if (MQTTclient.connect (HOSTNAME, MQTT_USER, MQTT_PASSWORD, MQTT_PREFIX "connected", 0, true, "0")) {
      Serial.println("connected");
      MQTTclient.publish(MQTT_PREFIX "connected", "1", true);
      itoa(rising_edge_cnt_SCK, strtmp, 10);
      MQTTclient.publish(MQTT_PREFIX "fSCK", strtmp, true);
      itoa(rising_edge_cnt_MOSI, strtmp, 10);
      MQTTclient.publish(MQTT_PREFIX "fMOSI", strtmp, true);
      itoa(rising_edge_cnt_MISO, strtmp, 10);
      MQTTclient.publish(MQTT_PREFIX "fMISO", strtmp, true);
      itoa(WiFi.RSSI(), strtmp, 10);
      MQTTclient.publish(MQTT_PREFIX "RSSI", strtmp, true);
      MQTTclient.subscribe(MQTT_SET_PREFIX "#");
      updateMQTTStatus=true;
      updateMQTTStatus_opdata=opdataCnt;
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
bool request_erropData = false;

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
  else if (strcmp(topic, MQTT_SET_PREFIX "ErrOpData") == 0) {
    request_erropData = true;
    publish_cmd_ok();
  }
  else if (strcmp(topic, MQTT_SET_PREFIX "reset") == 0) {
    if(strcmp(payload_str, "reset") == 0) {
      publish_cmd_ok();
      delay(500);
      ESP.restart();
    }
    else 
      publish_cmd_invalidparameter();  
  }
  else
    publish_cmd_unknown();
}


void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("MHI-AC-Ctrl starting");

  WiFi.hostname(HOSTNAME);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  MeasureFrequency();
  pinMode(SCK, INPUT);
  pinMode(MOSI, INPUT);
  pinMode(MISO, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);  // indicates that a frame was received, active low
  digitalWrite(LED_BUILTIN, HIGH);
#if TEMP_MEASURE_PERIOD > 0
  sensors.begin();
  Serial.print("Found ");
  // Serial.print(sensors.getDeviceCount(), DEC);
  Serial.print(sensors.getDS18Count(), DEC);
  Serial.println(" DS18xxx family devices.");
  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0"); 
  Serial.print("Device 0 Address: ");
  printAddress(insideThermometer);
  Serial.println();
  sensors.setResolution(insideThermometer, 9); // set the resolution to 9 bit
  sensors.setWaitForConversion(false);
  sensors.requestTemperatures(); // Send the command to get temperatures
#endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.printf(" connected to %s, IP address: %s\n", WIFI_SSID, WiFi.localIP().toString().c_str());
  setupOTA();
  MQTTclient.setServer(MQTT_SERVER, MQTT_PORT);
  MQTTclient.setCallback(MQTT_subscribe_callback);
  MQTTreconnect();
}

//                        sb0   sb1   sb2   db0   db1   db2   db3   db4   db5   db6   db7   db8   db9  db10  db11  db12  db13  db14  chkH  chkL
byte tx_SPIframe[20] = { 0xA9, 0x00, 0x07, 0x50, 0x10, 0x2e, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x05, 0xf5 };

uint16_t calc_tx_checksum() {
  uint16_t checksum = 0;
  for (int i = 0; i < CBH; i++)
    checksum += tx_SPIframe[i];
  return checksum;
}

void loop() {
  // old status
  uint8_t fan_old = 0xff;
  uint8_t vanes_old = 0xff;
  uint8_t power_old = 0xff;
  uint8_t mode_old = 0xff;
  uint8_t errorcode_old = 0xff;
  byte troom_old = 0xff;
  byte tsetpoint_old = 0xff;
  // old operating data
  byte op_settemp_old = 0xff;
  byte op_mode_old = 0xff;
  uint16_t op_ou_eev_old = 0xffff;
  byte op_total_iu_run_old = 0;
  byte op_iu_fanspeed_old = 0xff;
  byte op_ou_fanspeed_old = 0xff;
  byte op_outdoor_old = 0xff;
  byte op_return_air_old = 0xff;
  byte op_total_comp_run_old = 0; 
  byte op_protection_no_old = 0xff; 
  byte op_defrost_old = 0x00; 
  uint16_t op_comp_old = 0xffff;
  byte op_tdsh_old = 0xff;
  byte op_ct_old = 0xff;
  
  char strtmp[10];    // for the MQTT strings to send
  byte payload_byte;  // received MOSI byte
  unsigned long lastDatapacketMillis = 0;
  unsigned long SCKMillis;
  bool valid_datapacket_received = false;
  bool new_datapacket_received = false;
  uint16_t packet_cnt = 0;
  uint8_t repetitionNo = 0;
  uint8_t opdataNo = 0;
  uint8_t erropdataCnt = 0;
  uint8_t doubleframe = 1;
  uint8_t frame = 1;
  unsigned long DS1820Millis = millis();
  int16_t tempR_old = 0xffff;

  while (1) {

    SCKMillis = millis();
    while (millis() - SCKMillis < 5) { // wait for 5ms stable high signal
      delay(0);
      if (!digitalRead(SCK)) {
        SCKMillis = millis();
        if (millis() - lastDatapacketMillis > 35) {
          update_sync(false);
          Serial.println("async: millis() - lastDatapacketMillis > 35");
        }
      }
    }

    if (frame++ > 19) { // setup tx frame
      doubleframe++;  // toggle between 0 and 1
      tx_SPIframe[DB14] = (doubleframe % 2) << 2;
      frame = 1;
      if (doubleframe % 2) {
        if (erropdataCnt == 0) {
          tx_SPIframe[DB0] = 0x00;
          tx_SPIframe[DB1] = 0x00;
          tx_SPIframe[DB2] = 0x00;
          tx_SPIframe[DB6]  = opdata[opdataNo][0];
          tx_SPIframe[DB9]  = opdata[opdataNo][1];
          tx_SPIframe[DB10] = opdata[opdataNo][2];
          tx_SPIframe[DB11] = opdata[opdataNo][3];
          tx_SPIframe[DB12] = opdata[opdataNo][4];
          opdataNo = (opdataNo + 1) % opdataCnt;
        }
        else { // error operating data available
          tx_SPIframe[DB6]  = 0x80;
          tx_SPIframe[DB9]  = 0xff;
          tx_SPIframe[DB10] = 0xff;
          tx_SPIframe[DB11] = 0xff;
          tx_SPIframe[DB12] = 0xff;
          erropdataCnt--;
        }
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

        if (set_Fan) {
          if (new_Fan == 4) {
            tx_SPIframe[DB1] = 0b1010;
            tx_SPIframe[DB6] |= 0b00010000;
          }
          else {
            tx_SPIframe[DB1] = (1 << 3) | (new_Fan - 1);
          }
          set_Fan = false;
        }

        if (set_Vanes) {
          if (new_Vanes == 5) { //5: swing
            tx_SPIframe[DB0] = 0b11000000;
          }
          else {
            tx_SPIframe[DB0] = 0b10000000; // disable swing
            tx_SPIframe[DB1] = (1 << 7) | ((new_Vanes - 1) << 4);
          }
          set_Vanes = false;
        }

        if (request_erropData) {
          tx_SPIframe[DB6] = 0x80;
          tx_SPIframe[DB9] = 0x45;
          request_erropData = false;
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
    if (((rx_SPIframe[SB0] & 0xfe) == 0x6c) & (rx_SPIframe[SB1] == 0x80) & (rx_SPIframe[SB2] == 0x04) & ((rx_SPIframe[CBH]<<8 | rx_SPIframe[CBL]) == rx_checksum)) {
      valid_datapacket_received = true;
    }
    else {
      update_sync(false);
      Serial.printf("Wrong MOSI signature: 0x%.2X 0x%.2X 0x%.2X or checksum received!\n", rx_SPIframe[SB0], rx_SPIframe[SB1], rx_SPIframe[SB2]);
    }
    
    if (valid_datapacket_received) { // valid MOSI frame received
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

        int8_t troom_diff = rx_SPIframe[DB3] - troom_old; // avoid using other functions inside the brackets of abs, see https://www.arduino.cc/reference/en/language/functions/math/abs/  
        if (updateMQTTStatus | (abs(troom_diff) > 1)) { // Room temperature delta > 0.25°C
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

        // Operating Data
        switch (rx_SPIframe[DB9]) {
          case 0x02: // 1 MODE
            if ((rx_SPIframe[DB6] & 0x80) != 0){
              if ((rx_SPIframe[DB10] | 0x30) == 0x30){
                eval_op_mode(strtmp, rx_SPIframe[DB10]);
                MQTTclient.publish(MQTT_ERR_OP_PREFIX "Mode", strtmp, true);
              }
              else if ((rx_SPIframe[DB10] != op_mode_old) | (updateMQTTStatus_opdata > 0)){
                if (updateMQTTStatus_opdata > 0)
                  updateMQTTStatus_opdata--;
                op_mode_old = rx_SPIframe[DB10];
                eval_op_mode(strtmp, rx_SPIframe[DB10]);
                if(!MQTTclient.publish(MQTT_OP_PREFIX "Mode", strtmp, true))
                  op_mode_old = 0xff;
              }
            }
            break;
          case 0x05: // 1 SET-TEMP
            if ((rx_SPIframe[DB10] == 0x13) & ((rx_SPIframe[DB6] & 0x80) != 0)) {
              if ((rx_SPIframe[DB11] != op_settemp_old) | (updateMQTTStatus_opdata > 0)){
                if (updateMQTTStatus_opdata > 0)
                  updateMQTTStatus_opdata--;
                op_settemp_old = rx_SPIframe[DB11];
                itoa(rx_SPIframe[DB11] >> 1, strtmp, 10);
                if(!MQTTclient.publish(MQTT_OP_PREFIX "SET-TEMP", strtmp, true))
                  op_settemp_old = 0xff;
              }
            }
            else if ((rx_SPIframe[DB10] == 0x33) & ((rx_SPIframe[DB6] & 0x80) != 0)) { // last error data
                itoa(rx_SPIframe[DB11] >> 1, strtmp, 10);
                MQTTclient.publish(MQTT_ERR_OP_PREFIX "SET-TEMP", strtmp, true);
            }
            break;
          case 0x80: // 3 RETURN-AIR or 21 OUTDOOR
            if ((rx_SPIframe[DB10] == 0x20) & ((rx_SPIframe[DB6] & 0x80) != 0)) { // 3 RETURN-AIR
              if ((rx_SPIframe[DB11] != op_return_air_old) | (updateMQTTStatus_opdata > 0)){
                if (updateMQTTStatus_opdata > 0)
                  updateMQTTStatus_opdata--;
                op_return_air_old = rx_SPIframe[DB11];
                dtostrf(rx_SPIframe[DB11] * 0.25f - 15, 0, 2, strtmp);
                if(!MQTTclient.publish(MQTT_OP_PREFIX "RETURN-AIR", strtmp, true))
                  op_return_air_old = 0xff;
              }
            }
            else if ((rx_SPIframe[DB10] == 0x30) & ((rx_SPIframe[DB6] & 0x80) != 0)) { // 3 RETURN-AIR last error data
                dtostrf(rx_SPIframe[DB11] * 0.25f - 15, 0, 2, strtmp);
                MQTTclient.publish(MQTT_ERR_OP_PREFIX "RETURN-AIR", strtmp, true);
            }
            else if ((rx_SPIframe[DB10] == 0x10) & ((rx_SPIframe[DB6] & 0x80) == 0)) { // 21 OUTDOOR
              if ((rx_SPIframe[DB11] != op_outdoor_old) | (updateMQTTStatus_opdata > 0)){
                if (updateMQTTStatus_opdata > 0)
                  updateMQTTStatus_opdata--;
                op_outdoor_old = rx_SPIframe[DB11];
                dtostrf((rx_SPIframe[DB11] - 94) * 0.25f, 0, 2, strtmp);
                if(!MQTTclient.publish(MQTT_OP_PREFIX "OUTDOOR", strtmp, true))
                  op_outdoor_old = 0xff;
              }
            }
            break;
          case 0x1f: // 8 IU-FANSPEED or 34 OU-FANSPEED
            if ((rx_SPIframe[DB6] & 0x80) != 0) {
              if ((rx_SPIframe[DB10] & 0x30) == 0x10) { // 8 IU-FANSPEED
                if ((rx_SPIframe[DB10] != op_iu_fanspeed_old) | (updateMQTTStatus_opdata > 0)) {
                  if (updateMQTTStatus_opdata > 0)
                    updateMQTTStatus_opdata--;
                  op_iu_fanspeed_old = rx_SPIframe[DB10];
                  itoa(rx_SPIframe[DB10] & 0x0f, strtmp, 10);
                  if(!MQTTclient.publish(MQTT_OP_PREFIX "IU-FANSPEED", strtmp, true))
                    op_iu_fanspeed_old = 0xff;
                }
              }
              else if ((rx_SPIframe[DB10] & 0x30) == 0x30) { // 8 IU-FANSPEED last error data
                itoa(rx_SPIframe[DB10] & 0x0f, strtmp, 10);
                MQTTclient.publish(MQTT_ERR_OP_PREFIX "IU-FANSPEED", strtmp, true);
              }
            }
            else { // 34 OU-FANSPEED
              if ((rx_SPIframe[DB10] != op_ou_fanspeed_old) | (updateMQTTStatus_opdata > 0)){
                if (updateMQTTStatus_opdata > 0)
                  updateMQTTStatus_opdata--;
                op_ou_fanspeed_old = rx_SPIframe[DB10];
                itoa(op_ou_fanspeed_old &0x0f, strtmp, 10);
                if(!MQTTclient.publish(MQTT_OP_PREFIX "OU-FANSPEED", strtmp, true))
                  op_ou_fanspeed_old = 0xff;
              }
            }
            break;
          case 0x1e: // 12 TOTAL-IU-RUN or 37 TOTAL-COMP-RUN
            if ((rx_SPIframe[DB10] == 0x10) & ((rx_SPIframe[DB6] & 0x80) != 0)){ // 12 TOTAL-IU-RUN
              if ((rx_SPIframe[DB11] != op_total_iu_run_old) | (updateMQTTStatus_opdata > 0)){
                if (updateMQTTStatus_opdata > 0)
                  updateMQTTStatus_opdata--;
                op_total_iu_run_old = rx_SPIframe[DB11];
                itoa(rx_SPIframe[DB11]*100, strtmp, 10);
                if(!MQTTclient.publish(MQTT_OP_PREFIX "TOTAL-IU-RUN", strtmp, true))
                  op_total_iu_run_old = 0xff;
              }
            }
            else if ((rx_SPIframe[DB10] == 0x30) & ((rx_SPIframe[DB6] & 0x80) != 0)){ // 12 TOTAL-IU-RUN last error data
                itoa(rx_SPIframe[DB11]*100, strtmp, 10);
                MQTTclient.publish(MQTT_ERR_OP_PREFIX "TOTAL-IU-RUN", strtmp, true);
            }
            else if ((rx_SPIframe[DB10] == 0x11) & ((rx_SPIframe[DB6] & 0x80) == 0)){ // 37 TOTAL-COMP-RUN
              if ((rx_SPIframe[DB11] != op_total_comp_run_old) | (updateMQTTStatus_opdata > 0)){
                if (updateMQTTStatus_opdata > 0)
                  updateMQTTStatus_opdata--;
                op_total_comp_run_old = rx_SPIframe[DB11];
                itoa(rx_SPIframe[DB11]*100, strtmp, 10);
                if(!MQTTclient.publish(MQTT_OP_PREFIX "TOTAL-COMP-RUN", strtmp, true))
                  op_total_comp_run_old = 0xff;
              }
            }
            break;
          case 0x11: // 24 COMP
            if ((rx_SPIframe[DB6] & 0x80) == 0){
              if (((rx_SPIframe[DB11] << 8 | rx_SPIframe[DB10]) != op_comp_old) | (updateMQTTStatus_opdata > 0)){
                if (updateMQTTStatus_opdata > 0)
                  updateMQTTStatus_opdata--;
                op_comp_old = rx_SPIframe[DB11] << 8 | rx_SPIframe[DB10]; 
                dtostrf((rx_SPIframe[DB10] - 0x10) * 25.6f + 0.1f * rx_SPIframe[DB11], 0, 2, strtmp);
                if(!MQTTclient.publish(MQTT_OP_PREFIX "COMP", strtmp, true))
                  op_comp_old = 0xffff;
              }
            }
            break;
          case 0x90: // 29 CT
            if ((rx_SPIframe[DB10] == 0x10) & ((rx_SPIframe[DB6] & 0x80) == 0)){
              if ((rx_SPIframe[DB11] != op_protection_no_old) | (updateMQTTStatus_opdata > 0)){
                if (updateMQTTStatus_opdata > 0)
                  updateMQTTStatus_opdata--;
                op_protection_no_old = rx_SPIframe[DB11];
                itoa(rx_SPIframe[DB11], strtmp, 10);
                if(!MQTTclient.publish(MQTT_OP_PREFIX "CT", strtmp, true))
                  op_protection_no_old = 0xff;
              }
            }
            break;
          case 0xb1: // 32 TDSH
            if ((rx_SPIframe[DB10] == 0x10) & ((rx_SPIframe[DB6] & 0x80) == 0)){
              if ((rx_SPIframe[DB11] != op_tdsh_old) | (updateMQTTStatus_opdata > 0)){
                if (updateMQTTStatus_opdata > 0)
                  updateMQTTStatus_opdata--;
                op_tdsh_old = rx_SPIframe[DB11];
                itoa(rx_SPIframe[DB11]/2, strtmp, 10);
                if(!MQTTclient.publish(MQTT_OP_PREFIX "TDSH", strtmp, true))
                  op_tdsh_old = 0xff;
              }
            }
            break;
          case 0x7c: // 33 PROTECTION-No
            if ((rx_SPIframe[DB10] == 0x10) & ((rx_SPIframe[DB6] & 0x80) == 0)){
              if ((rx_SPIframe[DB11] != op_ct_old) | (updateMQTTStatus_opdata > 0)){
                if (updateMQTTStatus_opdata > 0)
                  updateMQTTStatus_opdata--;
                op_ct_old = rx_SPIframe[DB11];
                itoa(rx_SPIframe[DB11], strtmp, 10);
                if(!MQTTclient.publish(MQTT_OP_PREFIX "PROTECTION-No", strtmp, true))
                  op_ct_old = 0xff;
              }
            }
            break;
          case 0x0c: // 36 DEFROST
            if ((rx_SPIframe[DB6] & 0x80) == 0){
              if ((rx_SPIframe[DB10] != op_defrost_old) | (updateMQTTStatus_opdata > 0)){
                if (updateMQTTStatus_opdata > 0)
                  updateMQTTStatus_opdata--;
                op_defrost_old = rx_SPIframe[DB10];
                if(rx_SPIframe[DB10] = 0x10)
                  strcpy(strtmp, "Off");
                else  //0x11
                  strcpy(strtmp, "On");
                if(!MQTTclient.publish(MQTT_OP_PREFIX "DEFROST", strtmp, true))
                  op_defrost_old = 0x00;
              }
            }
            break;
           case 0x13: // 38 OU-EEV
            if ((rx_SPIframe[DB10] == 0x10) & ((rx_SPIframe[DB6] & 0x80) == 0)){ // 38 OU-EEV
              if (((rx_SPIframe[DB12] << 8 | rx_SPIframe[DB11]) != op_ou_eev_old) | (updateMQTTStatus_opdata > 0)){
                if (updateMQTTStatus_opdata > 0)
                  updateMQTTStatus_opdata--;
                op_ou_eev_old = rx_SPIframe[DB12] << 8 | rx_SPIframe[DB11];
                itoa(op_ou_eev_old, strtmp, 10);
                if(!MQTTclient.publish(MQTT_OP_PREFIX "OU-EEV", strtmp, true))
                  op_ou_eev_old = 0xffff;
              }
            }
            break;
           // related to last error operating data 
           case 0x45: // last error number or count of following error operating data
            if ((rx_SPIframe[DB10] == 0x11) & ((rx_SPIframe[DB6] & 0x80) != 0)){ // last error number
              itoa(rx_SPIframe[DB11], strtmp, 10);
              MQTTclient.publish(MQTT_ERR_OP_PREFIX "Errorcode", strtmp, true);
            }
            else if ((rx_SPIframe[DB10] == 0x12) & ((rx_SPIframe[DB6] & 0x80) != 0)){ // count of following error operating data
              erropdataCnt = rx_SPIframe[DB11] + 4;
            }
            break;
       }

        updateMQTTStatus = false;
      } // if(new_datapacket_received)
    } // if(valid_datapacket_received)

#if TEMP_MEASURE_PERIOD > 0
    if(millis() - DS1820Millis > TEMP_MEASURE_PERIOD*1000) {
      int16_t tempR = sensors.getTemp(insideThermometer);
      int16_t tempR_diff = tempR - tempR_old; // avoid using other functions inside the brackets of abs, see https://www.arduino.cc/reference/en/language/functions/math/abs/  
      if(abs(tempR_diff) > 2) {
        tempR_old = tempR;
        dtostrf(sensors.rawToCelsius(tempR), 0, 2, strtmp);
        MQTTclient.publish(MQTT_PREFIX "Tds1820", strtmp, true); 
      }   
      DS1820Millis = millis();
      sensors.requestTemperatures(); // Send the command to get temperatures
    }
#endif
   
    if (!MQTTclient.connected())
      MQTTreconnect();
    MQTTclient.loop();
    ArduinoOTA.handle();
  }
}
