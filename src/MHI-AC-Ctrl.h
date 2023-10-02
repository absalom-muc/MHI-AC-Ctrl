#pragma once

#include <cstdint>

// MQTT topic names
#define TOPIC_CONNECTED "connected"
#define TOPIC_VERSION "Version"
#define TOPIC_RSSI "RSSI"
#define TOPIC_WIFI_LOST "WIFI_LOST"
#define TOPIC_MQTT_LOST "MQTT_LOST"
#define TOPIC_WIFI_BSSID "WIFI_BSSID"
#define TOPIC_CMD_RECEIVED "cmd_received"
#define TOPIC_TDS1820 "Tds1820"
#define TOPIC_FSCK "fSCK"
#define TOPIC_FMOSI "fMOSI"
#define TOPIC_FMISO "fMISO"

#define TOPIC_POWER "Power"
#define TOPIC_MODE "Mode"
#define TOPIC_FAN "Fan"
#define TOPIC_VANES "Vanes"
#define TOPIC_VANESLR "VanesLR"
#define TOPIC_3DAUTO "3Dauto"
#define TOPIC_TROOM "Troom"
#define TOPIC_TSETPOINT "Tsetpoint"
#define TOPIC_ERRORCODE "Errorcode"

#define TOPIC_UNKNOWN "unknown"
#define TOPIC_KWH "KWH"
#define TOPIC_RETURNAIR "RETURN-AIR"
#define TOPIC_THI_R1 "THI-R1"
#define TOPIC_THI_R2 "THI-R2"
#define TOPIC_THI_R3 "THI-R3"
#define TOPIC_IU_FANSPEED "IU-FANSPEED"
#define TOPIC_TOTAL_IU_RUN "TOTAL-IU-RUN"
#define TOPIC_OUTDOOR "OUTDOOR"
#define TOPIC_COMP "COMP"
#define TOPIC_TD "TD"
#define TOPIC_THO_R1 "THO-R1"
#define TOPIC_CT "CT"
#define TOPIC_TDSH "TDSH"
#define TOPIC_PROTECTION_NO "PROTECTION-NO"
#define TOPIC_OU_FANSPEED "OU-FANSPEED"
#define TOPIC_DEFROST "DEFROST"
#define TOPIC_TOTAL_COMP_RUN "TOTAL-COMP-RUN"
#define TOPIC_OU_EEV1 "OU-EEV1"

#define TOPIC_REQUEST_ERROPDATA "ErrOpData"
#define TOPIC_REQUEST_RESET "reset"

// MQTT payload text
#define PAYLOAD_CONNECTED_TRUE "1"
#define PAYLOAD_CONNECTED_FALSE "0"
#define PAYLOAD_CMD_OK "o.k."
#define PAYLOAD_CMD_UNKNOWN "unknown command"
#define PAYLOAD_CMD_INVALID_PARAMETER "invalid parameter"
#define PAYLOAD_POWER_ON "On"
#define PAYLOAD_POWER_OFF "Off"
#define PAYLOAD_MODE_OFF PAYLOAD_POWER_OFF
#define PAYLOAD_MODE_AUTO "Auto"
#define PAYLOAD_MODE_STOP "Stop"
#define PAYLOAD_MODE_DRY "Dry"
#define PAYLOAD_MODE_COOL "Cool"
#define PAYLOAD_MODE_FAN "Fan"
#define PAYLOAD_MODE_HEAT "Heat"
#define PAYLOAD_FAN_AUTO "Auto"
#define PAYLOAD_VANES_UNKNOWN "?"
#define PAYLOAD_VANES_SWING "Swing"
#define PAYLOAD_OP_DEFROST_ON "On"
#define PAYLOAD_OP_DEFROST_OFF "Off"
#define PAYLOAD_REQUEST_RESET "reset"
#define PAYLOAD_VANESLR_SWING "Swing"
#define PAYLOAD_3DAUTO_ON "On"
#define PAYLOAD_3DAUTO_OFF "Off"

enum POWER_STATUS {
    unknown,
    off,
    on
};
