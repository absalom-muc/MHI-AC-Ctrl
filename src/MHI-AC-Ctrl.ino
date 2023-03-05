// MHI-AC-Ctrl by absalom-muc
// read + write data via SPI controlled by MQTT
// for version see support.h

#include "MHI-AC-Ctrl-core.h"
#include "MHI-AC-Ctrl.h"
#include "support.h"

MHI_AC_Ctrl_Core mhi_ac_ctrl_core;

POWER_STATUS power_status = unknown;

unsigned long room_temp_set_timeout_Millis = millis();
bool troom_was_set_by_MQTT = false;

void MQTT_subscribe_callback(const char* topic, byte* payload, unsigned int length) {
  payload[length] = 0;  // we need a string
  Serial.printf_P(PSTR("MQTT_subscribe_callback, topic=%s payload=%s payload_length=%i\n"), topic, (char*)payload, length);
#ifndef POWERON_WHEN_CHANGING_MODE
  if (strcmp_P(topic, PSTR(MQTT_SET_PREFIX TOPIC_POWER)) == 0) {
    if (strcmp_P((char*)payload, PSTR(PAYLOAD_POWER_ON)) == 0) {
      mhi_ac_ctrl_core.set_power(power_on);
      publish_cmd_ok();
    }
    else if (strcmp_P((char*)payload, PSTR(PAYLOAD_POWER_OFF)) == 0) {
      mhi_ac_ctrl_core.set_power(power_off);
      publish_cmd_ok();
    }
    else
      publish_cmd_invalidparameter();
  }
  else 
#endif
  if (strcmp_P(topic, PSTR(MQTT_SET_PREFIX TOPIC_MODE)) == 0) {
#ifdef POWERON_WHEN_CHANGING_MODE
    if (strcmp_P((char*)payload, PSTR(PAYLOAD_POWER_OFF)) == 0) {
      mhi_ac_ctrl_core.set_power(power_off);
      publish_cmd_ok();
    } else
#endif
      if (strcmp_P((char*)payload, PSTR(PAYLOAD_MODE_AUTO)) == 0) {
        mhi_ac_ctrl_core.set_mode(mode_auto);
#ifdef POWERON_WHEN_CHANGING_MODE
        mhi_ac_ctrl_core.set_power(power_on);
#endif
        publish_cmd_ok();
      }
      else if (strcmp_P((char*)payload, PSTR(PAYLOAD_MODE_DRY)) == 0) {
        mhi_ac_ctrl_core.set_mode(mode_dry);
#ifdef POWERON_WHEN_CHANGING_MODE
        mhi_ac_ctrl_core.set_power(power_on);
#endif
        publish_cmd_ok();
      }
      else if (strcmp_P((char*)payload, PSTR(PAYLOAD_MODE_COOL)) == 0) {
        mhi_ac_ctrl_core.set_mode(mode_cool);
#ifdef POWERON_WHEN_CHANGING_MODE
        mhi_ac_ctrl_core.set_power(power_on);
#endif
        publish_cmd_ok();
      }
      else if (strcmp_P((char*)payload, PSTR(PAYLOAD_MODE_FAN)) == 0) {
        mhi_ac_ctrl_core.set_mode(mode_fan);
#ifdef POWERON_WHEN_CHANGING_MODE
        mhi_ac_ctrl_core.set_power(power_on);
#endif
        publish_cmd_ok();
      }
      else if (strcmp_P((char*)payload, PSTR(PAYLOAD_MODE_HEAT)) == 0) {
        mhi_ac_ctrl_core.set_mode(mode_heat);
#ifdef POWERON_WHEN_CHANGING_MODE
        mhi_ac_ctrl_core.set_power(power_on);
#endif
        publish_cmd_ok();
      }
      else
        publish_cmd_invalidparameter();
  }
  else if (strcmp_P(topic, PSTR(MQTT_SET_PREFIX TOPIC_TSETPOINT)) == 0) {
    float f=atof((char*)payload);
    if((f >= 18) & (f <= 30))
      mhi_ac_ctrl_core.set_tsetpoint((byte)(2 * f));
    else
      publish_cmd_invalidparameter();
  }
  else if (strcmp_P(topic, PSTR(MQTT_SET_PREFIX TOPIC_FAN)) == 0) {
    if (strcmp_P((char*)payload, PAYLOAD_FAN_AUTO) == 0){
      mhi_ac_ctrl_core.set_fan(7);
      publish_cmd_ok();
    }
    else if (strcmp_P((char*)payload, "1") == 0){
      mhi_ac_ctrl_core.set_fan(0);
      publish_cmd_ok();
    }
    else if (strcmp_P((char*)payload, "2") == 0){
      mhi_ac_ctrl_core.set_fan(1);
      publish_cmd_ok();
    }
    else if (strcmp_P((char*)payload, "3") == 0){
      mhi_ac_ctrl_core.set_fan(2);
      publish_cmd_ok();
    }
    else if (strcmp_P((char*)payload, "4") == 0){
      mhi_ac_ctrl_core.set_fan(6);
      publish_cmd_ok();
    }
    else
      publish_cmd_invalidparameter();
  }
  else if (strcmp_P(topic, PSTR(MQTT_SET_PREFIX TOPIC_VANES)) == 0) {
    if (strcmp_P((char*)payload, PSTR(PAYLOAD_VANES_SWING)) == 0) {
      mhi_ac_ctrl_core.set_vanes(vanes_swing);
      publish_cmd_ok();
    }
    else {
      if ((atoi((char*)payload) >= 1) & (atoi((char*)payload) <= 5)) {
        mhi_ac_ctrl_core.set_vanes(atoi((char*)payload));
        publish_cmd_ok();
      }
      else
        publish_cmd_invalidparameter();
    }
  }
  else if (strcmp_P(topic, PSTR(MQTT_SET_PREFIX TOPIC_TROOM)) == 0) {
    float f=atof((char*)payload);
#ifdef ENHANCED_RESOLUTION
    f = f + mhi_ac_ctrl_core.get_troom_offset() ;  // increase Troom with current offset to compensate higher setpoint
#endif
    if ((f > -10) & (f < 48)) {
      room_temp_set_timeout_Millis = millis();  // reset timeout
      troom_was_set_by_MQTT=true;
      byte tmp = f*4+61;
      mhi_ac_ctrl_core.set_troom(f*4+61);
      Serial.printf("ROOM_TEMP_MQTT: %f %i %i\n", f, (byte)(f*4+61), (byte)tmp);
      publish_cmd_ok();
    }
    else
      publish_cmd_invalidparameter();
  }
  else if (strcmp_P(topic, PSTR(MQTT_SET_PREFIX TOPIC_REQUEST_ERROPDATA)) == 0) {
    mhi_ac_ctrl_core.request_ErrOpData();
    publish_cmd_ok();
  }
  else if (strcmp_P(topic, PSTR(MQTT_SET_PREFIX TOPIC_REQUEST_RESET)) == 0) {
    if (strcmp_P((char*)payload, PSTR(PAYLOAD_REQUEST_RESET)) == 0) {
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

class StatusHandler : public CallbackInterface_Status {
  public:
    void cbiStatusFunction(ACStatus status, int value) {
      char strtmp[10];
      static int mode_tmp = 0xff;
#ifdef ENHANCED_RESOLUTION      
      float offset = mhi_ac_ctrl_core.get_troom_offset();
      float tmp_value;
#endif
      static byte status_troom_old=0xff;
      //Serial.printf_P(PSTR("status=%i value=%i\n"), status, value);
      switch (status) {
        case status_power:
          // After powerdown AC (230V), fan status is only showing 1, 2 or 3. 4 and Auto is not shown when changing with RC.
          // Only when setting fan to Auto one time after powerdown AC, it will show 4 and Auto.
          // Below will take care of this.
          if (power_status == unknown) {  // First time after startup esp
            Serial.printf("power_status: unknown; received status_power: %i\n", value);
            if (value == power_off) {  // Only when status is power off, set fan to Auto. 
              Serial.println("Set fan to Auto to fix fan status after powerdown (230V) AC");
              mhi_ac_ctrl_core. set_fan(7);
            }
          } else if (power_status == off) 
            Serial.printf("power_status: off; received status_power: %i\n", value);
          else if (power_status == on) 
            Serial.printf("power_status: on; received status_power: %i\n", value);

          if (value == power_on){
            output_P(status, (TOPIC_POWER), PSTR(PAYLOAD_POWER_ON));
            power_status = on;
#ifdef POWERON_WHEN_CHANGING_MODE
            cbiStatusFunction(status_mode, mode_tmp);
#endif
          }
          else {
            output_P(status, (TOPIC_POWER), (PAYLOAD_POWER_OFF));
            power_status = off;
#ifdef POWERON_WHEN_CHANGING_MODE
            output_P(status, PSTR(TOPIC_MODE), PSTR(PAYLOAD_MODE_OFF));
#endif
          }
          break;
        case status_mode:
          mode_tmp = value;
        case opdata_mode:
        case erropdata_mode:
          switch (value) {
            case mode_auto:
              if (status != erropdata_mode)
                output_P(status, PSTR(TOPIC_MODE), PSTR(PAYLOAD_MODE_AUTO));
              else
                output_P(status, PSTR(TOPIC_MODE), PSTR(PAYLOAD_MODE_STOP));
              break;
            case mode_dry:
              output_P(status, PSTR(TOPIC_MODE), PSTR(PAYLOAD_MODE_DRY));
              break;
            case mode_cool:
              output_P(status, PSTR(TOPIC_MODE), PSTR(PAYLOAD_MODE_COOL));
              break;
            case mode_fan:
              output_P(status, PSTR(TOPIC_MODE), PSTR(PAYLOAD_MODE_FAN));
              break;
            case mode_heat:
              output_P(status, PSTR(TOPIC_MODE), PSTR(PAYLOAD_MODE_HEAT));
              break;
          }
          break;
        case opdata_kwh:
          dtostrf(highByte(value)*64.0f + lowByte(value)*0.25f, 0, 2, strtmp);
          output_P(status, PSTR(TOPIC_KWH), strtmp);
          break;
        case opdata_unknown:
          itoa(value, strtmp, 10);
          output_P(status, PSTR(TOPIC_UNKNOWN), strtmp);
          break;
        case status_fan:
          switch (value) {
            case 0:
              output_P(status, TOPIC_FAN, "1");
              break;
            case 1:
              output_P(status, TOPIC_FAN, "2");
              break;
            case 2:
              output_P(status, TOPIC_FAN, "3");
              break;
            case 6:
              output_P(status, TOPIC_FAN, "4");
              break;
            case 7: 
              output_P(status, TOPIC_FAN, PAYLOAD_FAN_AUTO);
              break;
            default: // invalid values
              itoa(value, strtmp, 10);
              strcat(strtmp, "?");
              output_P(status, TOPIC_FAN, strtmp);
              break;              
          }
          break;
        case status_vanes:
          switch (value) {
            case vanes_unknown:
              output_P(status, PSTR(TOPIC_VANES), PSTR(PAYLOAD_VANES_UNKNOWN));
              break;
            case vanes_swing:
              output_P(status, PSTR(TOPIC_VANES), PSTR(PAYLOAD_VANES_SWING));
              break;
            default:
              itoa(value, strtmp, 10);
              output_P(status, PSTR(TOPIC_VANES), strtmp);
          }
          break;
        case status_troom:
          {
            int8_t troom_diff = value - status_troom_old; // avoid using other functions inside the brackets of abs, see https://www.arduino.cc/reference/en/language/functions/math/abs/
            if (abs(troom_diff) > TROOM_FILTER_LIMIT/0.25f) { // Room temperature delta > 0.25°C
              status_troom_old = value;
              dtostrf((value - 61) / 4.0, 0, 2, strtmp);
              output_P(status, PSTR(TOPIC_TROOM), strtmp);
            }
          }
          break;
        case status_tsetpoint:
#ifdef ENHANCED_RESOLUTION
          tmp_value = (value & 0x7f)/ 2.0;
          offset = round(tmp_value) - tmp_value;  // Calculate offset when setpoint is changed
          mhi_ac_ctrl_core.set_troom_offset(offset);
#endif          
        case opdata_tsetpoint:
        case erropdata_tsetpoint:
          dtostrf((value & 0x7f)/ 2.0, 0, 1, strtmp);
          output_P(status, PSTR(TOPIC_TSETPOINT), strtmp);
          break;
        case status_errorcode:
        case erropdata_errorcode:
          itoa(value, strtmp, 10);
          output_P(status, PSTR(TOPIC_ERRORCODE), strtmp);
          break;
        case opdata_return_air:
        case erropdata_return_air:
          dtostrf((value - 61) / 4.0, 0, 2, strtmp);
          output_P(status, PSTR(TOPIC_RETURNAIR), strtmp);
          break;
        case opdata_thi_r1:
        case erropdata_thi_r1:
          itoa(0.327f * value - 11.4f, strtmp, 10); // only rough approximation
          output_P(status, PSTR(TOPIC_THI_R1), strtmp);
          break;
        case opdata_thi_r2:
        case erropdata_thi_r2:
          itoa(0.327f * value - 11.4f, strtmp, 10); // formula for calculation not known
          output_P(status, PSTR(TOPIC_THI_R2), strtmp);
          break;
        case opdata_thi_r3:
        case erropdata_thi_r3:
          itoa(0.327f * value - 11.4f, strtmp, 10); // only rough approximation
          output_P(status, PSTR(TOPIC_THI_R3), strtmp);
          break;
        case opdata_iu_fanspeed:
        case erropdata_iu_fanspeed:
          itoa(value, strtmp, 10);
          output_P(status, PSTR(TOPIC_IU_FANSPEED), strtmp);
          break;
        case opdata_total_iu_run:
        case erropdata_total_iu_run:
          itoa(value * 100, strtmp, 10);
          output_P(status, PSTR(TOPIC_TOTAL_IU_RUN), strtmp);
          break;
        case erropdata_outdoor:
        case opdata_outdoor:
          dtostrf((value - 94) * 0.25f, 0, 2, strtmp);
          output_P(status, PSTR(TOPIC_OUTDOOR), strtmp);
          break;
        case opdata_tho_r1:
        case erropdata_tho_r1:
          itoa(0.327f * value - 11.4f, strtmp, 10); // formula for calculation not known
          output_P(status, PSTR(TOPIC_THO_R1), strtmp);
          break;
        case opdata_comp:
        case erropdata_comp:
          dtostrf(highByte(value) * 25.6f + 0.1f * lowByte(value), 0, 2, strtmp);  // to be confirmed
          output_P(status, PSTR(TOPIC_COMP), strtmp);
          break;
        case erropdata_td:
        case opdata_td:
          if (value < 0x12)
            strcpy(strtmp, "<=30");
          else
            itoa(value / 2 + 32, strtmp, 10);
          output_P(status, PSTR(TOPIC_TD), strtmp);
          break;
        case opdata_ct:
        case erropdata_ct:
          dtostrf(value * 14 / 51.0f, 0, 2, strtmp);
          output_P(status, PSTR(TOPIC_CT), strtmp);
          break;
        case opdata_tdsh:
          itoa(value, strtmp, 10); // formula for calculation not known
          output_P(status, PSTR(TOPIC_TDSH), strtmp);
          break;
        case opdata_protection_no:
          itoa(value, strtmp, 10);
          output_P(status, PSTR(TOPIC_PROTECTION_NO), strtmp);
          break;
        case opdata_ou_fanspeed:
        case erropdata_ou_fanspeed:
          itoa(value, strtmp, 10);
          output_P(status, PSTR(TOPIC_OU_FANSPEED), strtmp);
          break;
        case opdata_defrost:
          if (value)
            output_P(status, PSTR(TOPIC_DEFROST), PSTR(PAYLOAD_OP_DEFROST_ON));
          else
            output_P(status, PSTR(TOPIC_DEFROST), PSTR(PAYLOAD_OP_DEFROST_OFF));
          break;
        case opdata_total_comp_run:
        case erropdata_total_comp_run:
          itoa(value * 100, strtmp, 10);
          output_P(status, PSTR(TOPIC_TOTAL_COMP_RUN), strtmp);
          break;
        case opdata_ou_eev1:
        case erropdata_ou_eev1:
          itoa(value, strtmp, 10);
          output_P(status, PSTR(TOPIC_OU_EEV1), strtmp);
          break;
      }
    }
};
StatusHandler mhiStatusHandler;

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println();
  Serial.println(F("Starting MHI-AC-Ctrl v" VERSION));
  Serial.printf_P(PSTR("CPU frequency[Hz]=%lu\n"), F_CPU);
  Serial.printf("ESP.getCoreVersion()=%s\n", ESP.getCoreVersion().c_str());
  Serial.printf("ESP.getSdkVersion()=%s\n", ESP.getSdkVersion());
  Serial.printf("ESP.checkFlashCRC()=%i\n", ESP.checkFlashCRC());

#if TEMP_MEASURE_PERIOD > 0
  setup_ds18x20();
#endif
  initWiFi();
  MeasureFrequency();
  setupOTA();
  MQTTclient.setServer(MQTT_SERVER, MQTT_PORT);
  MQTTclient.setCallback(MQTT_subscribe_callback);
  mhi_ac_ctrl_core.MHIAcCtrlStatus(&mhiStatusHandler);
  mhi_ac_ctrl_core.init();
  // mhi_ac_ctrl_core.set_fan(7); // set fan AUTO, see https://github.com/absalom-muc/MHI-AC-Ctrl/issues/99
}


void loop() {
  static byte ds18x20_value_old = 0;
  static int WiFiStatus = WIFI_CONNECT_TIMEOUT;
  static int MQTTStatus = MQTT_NOT_CONNECTED;
  static unsigned long previousMillis = millis();
  if (((WiFi.status() != WL_CONNECTED) | (WiFiStatus != WIFI_CONNECT_OK)) || (WiFI_SEARCHStrongestAP & (millis() - previousMillis >= WiFI_SEARCH_FOR_STRONGER_AP_INTERVALL*60*1000))) {
    //Serial.printf("loop: call setupWiFi(WiFiStatus)\n");
    setupWiFi(WiFiStatus);
    previousMillis = millis();
    //Serial.println(WiFiStatus);
  }
  else {
    //Serial.printf("loop: WiFi.status()=%i\n", WiFi.status()); // see https://realglitch.com/2018/07/arduino-wifi-status-codes/
    MQTTStatus=MQTTreconnect();
    if (MQTTStatus == MQTT_RECONNECTED)
      mhi_ac_ctrl_core.reset_old_values();  // after a reconnect
    ArduinoOTA.handle();
  }
  
#if TEMP_MEASURE_PERIOD > 0
  byte ds18x20_value = getDs18x20Temperature(25);
#ifdef ENHANCED_RESOLUTION
  float offset = mhi_ac_ctrl_core.get_troom_offset();
  byte tmp = offset*4;
  ds18x20_value = ds18x20_value + tmp;   // add offset
#endif

#ifdef ROOM_TEMP_DS18X20
  if(ds18x20_value != ds18x20_value_old) {
    if ((ds18x20_value > 21) & (ds18x20_value < 253)) {  // use only values -10°C < T < 48°C
      mhi_ac_ctrl_core.set_troom(ds18x20_value);
      ds18x20_value_old = ds18x20_value;
      Serial.printf("update Troom based on DS18x20 value %i\n", ds18x20_value);
    }
  }
#endif 
#endif
  // fallback to AC internal Troom temperature sensor
  if(troom_was_set_by_MQTT & (millis() - room_temp_set_timeout_Millis >= ROOM_TEMP_MQTT_SET_TIMEOUT*1000)) {
    mhi_ac_ctrl_core.set_troom(0xff);  // use IU temperature sensor
    Serial.println(F("ROOM_TEMP_MQTT_SET_TIMEOUT exceeded, use IU temperature sensor value!"));
    troom_was_set_by_MQTT=false;
  }

  if((MQTTStatus==MQTT_RECONNECTED)|(MQTTStatus==MQTT_CONNECT_OK)){
    //Serial.println("MQTT connected in main loop");
    int ret = mhi_ac_ctrl_core.loop(80);
    if (ret < 0)
      Serial.printf_P(PSTR("mhi_ac_ctrl_core.loop error: %i\n"), ret);
  }
  /*else
    Serial.println("MQTT NOT connected in main loop");*/
}
