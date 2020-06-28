// MHI-AC-Ctrl v2.02 by absalom-muc
// read + write data via SPI controlled by MQTT

#include "MHI-AC-Ctrl-core.h"
#include "MHI-AC-Ctrl.h"
#include "support.h"

MHI_AC_Ctrl_Core mhi_ac_ctrl_core;

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
    if ((atoi((char*)payload) >= 18) & (atoi((char*)payload) <= 30)) {
      mhi_ac_ctrl_core.set_tsetpoint(atoi((char*)payload));
      publish_cmd_ok();
    }
    else
      publish_cmd_invalidparameter();
  }
  else if (strcmp_P(topic, PSTR(MQTT_SET_PREFIX TOPIC_FAN)) == 0) {
    if ((atoi((char*)payload) >= 1) & (atoi((char*)payload) <= 4)) {
      mhi_ac_ctrl_core.set_fan(atoi((char*)payload));
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
      Serial.printf_P(PSTR("status=%i value=%i\n"), status, value);
      switch (status) {
        case status_fsck:
          itoa(value, strtmp, 10);
          output_P(status, PSTR(TOPIC_FSCK), strtmp);
          break;
        case status_fmosi:
          itoa(value, strtmp, 10);
          output_P(status, PSTR(TOPIC_FMOSI), strtmp);
          break;
        case status_fmiso:
          itoa(value, strtmp, 10);
          output_P(status, PSTR(TOPIC_FMISO), strtmp);
          break;
        case status_power:
          if (value == power_on)
            output_P(status, (TOPIC_POWER), PSTR(PAYLOAD_POWER_ON));
          else {
            output_P(status, (TOPIC_POWER), (PAYLOAD_POWER_OFF));
#ifdef POWERON_WHEN_CHANGING_MODE
            output_P(status, PSTR(TOPIC_MODE), PSTR(PAYLOAD_MODE_OFF));
#endif
          }
          break;
        case status_mode:
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
        case status_fan:
          itoa(value + 1, strtmp, 10);
          output_P(status, TOPIC_FAN, strtmp);
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
          dtostrf((value - 61) / 4.0, 0, 2, strtmp);
          output_P(status, PSTR(TOPIC_TROOM), strtmp);
          break;
        case status_tsetpoint:
        case opdata_tsetpoint:
        case erropdata_tsetpoint:
          itoa(value, strtmp, 10);
          output_P(status, PSTR(TOPIC_TSETPOINT), strtmp);
          break;
        case status_errorcode:
        case erropdata_errorcode:
          itoa(value, strtmp, 10);
          output_P(status, PSTR(TOPIC_ERRORCODE), strtmp);
          break;
        case opdata_return_air:
        case erropdata_return_air:
          dtostrf(value * 0.25f - 15, 0, 2, strtmp);
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
  Serial.println();
  Serial.println(F("MHI-AC-Ctrl starting"));
  Serial.printf_P(PSTR("CPU frequency[Hz]=%lu\n"), F_CPU);

#if TEMP_MEASURE_PERIOD > 0
  setup_ds18x20();
#endif
  setupWiFi();
  setupOTA();
  MQTTclient.setServer(MQTT_SERVER, MQTT_PORT);
  MQTTclient.setCallback(MQTT_subscribe_callback);
  MQTTreconnect();
  mhi_ac_ctrl_core.MHIAcCtrlStatus(&mhiStatusHandler);
  mhi_ac_ctrl_core.init();
}

void loop() {
  if (MQTTloop())
    mhi_ac_ctrl_core.reset_old_values();  // after a reconnect
  ArduinoOTA.handle();

#if TEMP_MEASURE_PERIOD > 0
  getDs18x20Temperature(25);
#endif

  int ret = mhi_ac_ctrl_core.loop(100);
  if (ret < 0)
    Serial.printf_P(PSTR("mhi_ac_ctrl_core.loop error: %i\n"), ret);
}
