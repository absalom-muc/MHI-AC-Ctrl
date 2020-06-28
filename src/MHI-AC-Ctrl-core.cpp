// MHI-AC-Ctrol-core
// implements the core functions (read & write SPI)

#include "MHI-AC-Ctrl-core.h"

uint16_t calc_checksum(byte* frame) {
  uint16_t checksum = 0;
  for (int i = 0; i < CBH; i++)
    checksum += frame[i];
  return checksum;
}

void MHI_AC_Ctrl_Core::reset_old_values() {  // used e.g. when MQTT connection to broker is lost, to re-output data
  // old status
  status_power_old = 0xff;
  status_mode_old = 0xff;
  status_fan_old = 0xff;
  status_vanes_old = 0xff;
  status_troom_old = 0xff;
  status_tsetpoint_old = 0x00;
  status_errorcode_old = 0xff;

  // old operating data
  op_mode_old = 0xff;
  op_settemp_old = 0xff;
  op_return_air_old = 0xff;
  op_iu_fanspeed_old = 0xff;
  op_thi_r1_old = 0x00;
  op_thi_r2_old = 0x00;
  op_thi_r3_old = 0x00;
  op_total_iu_run_old = 0;
  op_outdoor_old = 0xff;
  op_tho_r1_old = 0x00;
  op_total_comp_run_old = 0;
  op_ct_old = 0xff;
  op_tdsh_old = 0xff;
  op_protection_no_old = 0xff;
  op_ou_fanspeed_old = 0xff;
  op_defrost_old = 0x00;
  op_comp_old = 0xffff;
  op_td_old  = 0x00;
  op_ou_eev1_old = 0xffff;
}

volatile uint rising_edge_cnt_SCK = 0;
ICACHE_RAM_ATTR void handleInterrupt_SCK() {
  rising_edge_cnt_SCK++;
}

volatile uint rising_edge_cnt_MOSI = 0;
ICACHE_RAM_ATTR void handleInterrupt_MOSI() {
  rising_edge_cnt_MOSI++;
}

volatile uint rising_edge_cnt_MISO = 0;
ICACHE_RAM_ATTR void handleInterrupt_MISO() {
  rising_edge_cnt_MISO++;
}

void MeasureFrequency(CallbackInterface_Status *m_cbiStatus) {  // measure the frequency on the pins
  char strtmp[10];
  pinMode(SCK_PIN, INPUT);
  pinMode(MOSI_PIN, INPUT);
  pinMode(MISO_PIN, INPUT);
  Serial.println(F("Measure frequency for SCK, MOSI and MISO pin"));
  attachInterrupt(digitalPinToInterrupt(SCK_PIN), handleInterrupt_SCK, RISING);
  attachInterrupt(digitalPinToInterrupt(MOSI_PIN), handleInterrupt_MOSI, RISING);
  attachInterrupt(digitalPinToInterrupt(MISO_PIN), handleInterrupt_MISO, RISING);
  unsigned long starttimeMicros = micros();
  while (micros() - starttimeMicros < 1000000);
  detachInterrupt(SCK_PIN);
  detachInterrupt(MOSI_PIN);
  detachInterrupt(MISO_PIN);

  m_cbiStatus->cbiStatusFunction(status_fsck, rising_edge_cnt_SCK);
  m_cbiStatus->cbiStatusFunction(status_fmosi, rising_edge_cnt_MOSI);
  m_cbiStatus->cbiStatusFunction(status_fmiso, rising_edge_cnt_MISO);

  Serial.printf_P(PSTR("SCK frequency=%iHz (expected: >3000Hz) "), rising_edge_cnt_SCK);
  if (rising_edge_cnt_SCK > 3000)
    Serial.println(F("o.k."));
  else
    Serial.println(F("out of range!"));

  Serial.printf("MOSI frequency=%iHz (expected: <SCK frequency) ", rising_edge_cnt_MOSI);
  if ((rising_edge_cnt_MOSI > 30) & (rising_edge_cnt_MOSI < rising_edge_cnt_SCK))
    Serial.println(F("o.k."));
  else
    Serial.println(F("out of range!"));

  Serial.printf("MISO frequency=%iHz (expected: ~0Hz) ", rising_edge_cnt_MISO);
  if (rising_edge_cnt_MISO <= 10) {
    Serial.println(F("o.k."));
  }
  else {
    Serial.println(F("out of range!"));
    while (1);
  }
}

void MHI_AC_Ctrl_Core::init() {
  MeasureFrequency(m_cbiStatus);
  pinMode(SCK_PIN, INPUT);
  pinMode(MOSI_PIN, INPUT);
  pinMode(MISO_PIN, OUTPUT);
  MHI_AC_Ctrl_Core::reset_old_values();
}

void MHI_AC_Ctrl_Core::set_power(boolean power) {
  new_Power = 0b10 | power;
}

void MHI_AC_Ctrl_Core::set_mode(ACMode mode) {
  new_Mode = 0b00100000 | mode;
}

void MHI_AC_Ctrl_Core::set_tsetpoint(uint tsetpoint) {
  new_Tsetpoint = 0b10000000 | (2 * tsetpoint);
}

void MHI_AC_Ctrl_Core::set_fan(uint fan) {
  if (fan == 4) {
    new_Fan1 = 0b00001010;
    new_Fan6 = 0b00010000;
  }
  else
    new_Fan1 = 0b00001000 | (fan - 1);
}

void MHI_AC_Ctrl_Core::set_vanes(uint vanes) {
  if (vanes == vanes_swing) {
    new_Vanes0 = 0b11000000; // enable swing
  }
  else {
    new_Vanes0 = 0b10000000; // disable swing
    new_Vanes1 = 0b10000000 | ((vanes - 1) << 4);
  }
}

void MHI_AC_Ctrl_Core::request_ErrOpData() {
  request_erropData = true;
}

int MHI_AC_Ctrl_Core::loop(uint max_time_ms) {
  const byte opdataCnt = sizeof(opdata) / sizeof(byte) / 2;
  static byte opdataNo = 0;               //
  int startMillis = millis();             // start time of this loop run
  byte MOSI_byte;                         // received MOSI byte
  bool new_datapacket_received = false;   // indicated that a new frame was received
  static byte erropdataCnt = 0;           // number of expected error operating data
  static bool doubleframe = false;
  static byte frame = 0;
  static byte MOSI_frame[20];
  //                            sb0   sb1   sb2   db0   db1   db2   db3   db4   db5   db6   db7   db8   db9  db10  db11  db12  db13  db14  chkH  chkL
  static byte MISO_frame[] = { 0xA9, 0x00, 0x07, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x00 };

  static uint call_counter = 0;           // counts how often this loop was called
  call_counter++;
  int SCKMillis = millis();               // time of last SCK low level
  while (millis() - SCKMillis < 5) { // wait for 5ms stable high signal to detect a frame start
    if (!digitalRead(SCK_PIN))
      SCKMillis = millis();
    if (millis() - startMillis > max_time_ms)
      return err_msg_timeout;
  }

  // build the next MISO frame
  if (frame++ >= NoFramesPerPacket) {       // setup MISO frame with each frame packet
    doubleframe = !doubleframe;             // toggle
    MISO_frame[DB14] = doubleframe << 2;    // MISO_frame[DB14] bit2 toggels with every frame packet
    frame = 1;
    if (doubleframe) {                     // and the other MISO data are updated after two frame packets
      MISO_frame[DB0] = 0x00;
      MISO_frame[DB1] = 0x00;
      MISO_frame[DB2] = 0x00;
      if (erropdataCnt == 0) {
        MISO_frame[DB6] = pgm_read_word(opdata + opdataNo);
        MISO_frame[DB9] = pgm_read_word(opdata + opdataNo) >> 8;
        opdataNo = (opdataNo + 1) % opdataCnt;
      }
      else { // error operating data available
        MISO_frame[DB6] = 0x80;
        MISO_frame[DB9] = 0xff;
        erropdataCnt--;
      }

      // set Power, Mode, Tsetpoint, Fan, Vanes
      MISO_frame[DB0] = new_Power;
      new_Power = 0;

      MISO_frame[DB0] |= new_Mode;
      new_Mode = 0;

      MISO_frame[DB2] = new_Tsetpoint;
      new_Tsetpoint = 0;

      MISO_frame[DB1] = new_Fan1;
      MISO_frame[DB6] |= new_Fan6;
      new_Fan1 = 0;
      new_Fan6 = 0;

      MISO_frame[DB0] |= new_Vanes0;
      MISO_frame[DB1] |= new_Vanes1;
      new_Vanes0 = 0;
      new_Vanes1 = 0;

      if (request_erropData) {
        MISO_frame[DB6] = 0x80;
        MISO_frame[DB9] = 0x45;
        request_erropData = false;
      }
    }
  }

  uint16_t checksum = calc_checksum(MISO_frame);
  MISO_frame[CBH] = highByte(checksum);
  MISO_frame[CBL] = lowByte(checksum);

  //Serial.println();
  //Serial.print(F("MISO:"));
  // read/write MOSI/MISO frame
  for (uint8_t byte_cnt = 0; byte_cnt < sizeof(MOSI_frame); byte_cnt++) { // read and write a data packet of 20 bytes
    //Serial.printf("x%02x ", MISO_frame[byte_cnt]);
    MOSI_byte = 0;
    byte bit_mask = 1;
    for (uint8_t bit_cnt = 0; bit_cnt < 8; bit_cnt++) { // read and write 1 byte
      while (digitalRead(SCK_PIN)) {} // wait for falling edge
      if ((MISO_frame[byte_cnt] & bit_mask) > 0)
        digitalWrite(MISO_PIN, 1);
      else
        digitalWrite(MISO_PIN, 0);
      while (!digitalRead(SCK_PIN)) {} // wait for rising edge
      if (digitalRead(MOSI_PIN))
        MOSI_byte += bit_mask;
      bit_mask = bit_mask << 1;
    }
    if (MOSI_frame[byte_cnt] != MOSI_byte) {
      new_datapacket_received = true;
      MOSI_frame[byte_cnt] = MOSI_byte;
    }
  }

  checksum = calc_checksum(MOSI_frame);
  if (((MOSI_frame[SB0] & 0xfe) != 0x6c) | (MOSI_frame[SB1] != 0x80) | (MOSI_frame[SB2] != 0x04))
    return err_msg_invalid_signature;
  if ((MOSI_frame[CBH] << 8 | MOSI_frame[CBL]) != checksum)
    return err_msg_invalid_checksum;

  if (new_datapacket_received) {
    // evaluate status
    if ((MOSI_frame[DB0] & 0x1c) != status_mode_old) { // Mode
      status_mode_old = MOSI_frame[DB0] & 0x1c;
      m_cbiStatus->cbiStatusFunction(status_mode, status_mode_old);
    }

    if ((MOSI_frame[DB0] & 0x01) != status_power_old) { // Power
      status_power_old = MOSI_frame[DB0] & 0x01;
      m_cbiStatus->cbiStatusFunction(status_power, status_power_old);
    }

    uint fantmp;
    if ((MOSI_frame[DB6] & 0x40) != 0) // Fan status
      fantmp = 3;
    else
      fantmp = (MOSI_frame[DB1] & 0x03);
    if (fantmp != status_fan_old) {
      status_fan_old = fantmp;
      m_cbiStatus->cbiStatusFunction(status_fan, status_fan_old);
    }

    // Only updated when Vanes command via wired RC
    uint vanestmp = (MOSI_frame[DB0] & 0xc0) + ((MOSI_frame[DB1] & 0xB0) >> 4);
    if (vanestmp != status_vanes_old) {
      if ((vanestmp & 0x88) == 0) // last vanes update was via IR-RC, so status is not known
        m_cbiStatus->cbiStatusFunction(status_vanes, vanes_unknown);
      else if ((vanestmp & 0x40) != 0) // Vanes status swing
        m_cbiStatus->cbiStatusFunction(status_vanes, vanes_swing);
      else {
        m_cbiStatus->cbiStatusFunction(status_vanes, (vanestmp & 0x03) + 1);
      }
      status_vanes_old = vanestmp;
    }

    int8_t troom_diff = MOSI_frame[DB3] - status_troom_old; // avoid using other functions inside the brackets of abs, see https://www.arduino.cc/reference/en/language/functions/math/abs/
    if (abs(troom_diff) > 1) { // Room temperature delta > 0.25°C
      status_troom_old = MOSI_frame[DB3];
      m_cbiStatus->cbiStatusFunction(status_troom, status_troom_old);
    }

    if (MOSI_frame[DB2] != status_tsetpoint_old) { // Temperature setpoint
      status_tsetpoint_old = MOSI_frame[DB2];
      m_cbiStatus->cbiStatusFunction(status_tsetpoint, (status_tsetpoint_old & 0x7f) >> 1);
    }

    if (MOSI_frame[DB4] != status_errorcode_old) { // error code
      status_errorcode_old = MOSI_frame[DB4];
      m_cbiStatus->cbiStatusFunction(status_errorcode, status_errorcode_old);
    }

    // Evaluate Operating Data and Error Operating Data
    bool MOSI_type_opdata = (MOSI_frame[DB10] & 0x30) == 0x10;

    switch (MOSI_frame[DB9]) {
      case 0x02:
        if ((MOSI_frame[DB6] & 0x80) != 0) {  // 1 MODE
          if (MOSI_type_opdata) {
            if ((MOSI_frame[DB10] != op_mode_old)) {
              op_mode_old = MOSI_frame[DB10];
              m_cbiStatus->cbiStatusFunction(opdata_mode, (op_mode_old & 0x0f) << 2);
            }
          }
          else
            m_cbiStatus->cbiStatusFunction(erropdata_mode, (MOSI_frame[DB10] & 0x0f) << 2);
        }
        break;
      case 0x05:
        if ((MOSI_frame[DB6] & 0x80) != 0) {  // 2 SET-TEMP
          if (MOSI_frame[DB10] == 0x13) {
            if (MOSI_frame[DB11] != op_settemp_old) {
              op_settemp_old = MOSI_frame[DB11];
              m_cbiStatus->cbiStatusFunction(opdata_tsetpoint, op_settemp_old >> 1);
            }
          }
          else if (MOSI_frame[DB10] == 0x33)
            m_cbiStatus->cbiStatusFunction(erropdata_tsetpoint, MOSI_frame[DB11] >> 1);
        }
        break;
      case 0x81:                              // 5 THI-R1 or 6 THI-R2
        if ((MOSI_frame[DB6] & 0x80) != 0) {  // 5 THI-R1
          if ((MOSI_frame[DB10] & 0x30) == 0x20) {
            if (MOSI_frame[DB11] != op_thi_r1_old) {
              op_thi_r1_old = MOSI_frame[DB11];
              m_cbiStatus->cbiStatusFunction(opdata_thi_r1, op_thi_r1_old);
            }
          }
          else
            m_cbiStatus->cbiStatusFunction(erropdata_thi_r1, MOSI_frame[DB11]);
        }
        else {                                // 6 THI-R2
          if (MOSI_type_opdata) {
            if (MOSI_frame[DB11] != op_thi_r2_old) {
              op_thi_r2_old = MOSI_frame[DB11];
              m_cbiStatus->cbiStatusFunction(opdata_thi_r2, op_thi_r2_old);
            }
          }
          else
            m_cbiStatus->cbiStatusFunction(erropdata_thi_r2, MOSI_frame[DB11]);
        }
        break;
      case 0x87:
        if ((MOSI_frame[DB6] & 0x80) != 0) {  // 7 THI-R3
          if (MOSI_type_opdata) {
            if (MOSI_frame[DB11] != op_thi_r3_old) {
              op_thi_r3_old = MOSI_frame[DB11];
              m_cbiStatus->cbiStatusFunction(opdata_thi_r3, op_thi_r3_old);
            }
          }
          else
            m_cbiStatus->cbiStatusFunction(erropdata_thi_r3, MOSI_frame[DB11]);
        }
        break;
      case 0x80:                              // 3 RETURN-AIR or 21 OUTDOOR
        if ((MOSI_frame[DB6] & 0x80) != 0) {  // 3 RETURN-AIR
          if ((MOSI_frame[DB10] & 0x30) == 0x20) {           // operating Data
            if (MOSI_frame[DB11] != op_return_air_old) {
              op_return_air_old = MOSI_frame[DB11];
              m_cbiStatus->cbiStatusFunction(opdata_return_air, op_return_air_old);
            }
          }
          else
            m_cbiStatus->cbiStatusFunction(erropdata_return_air, MOSI_frame[DB11]);
        }
        else {                                // 21 OUTDOOR
          if (MOSI_type_opdata) {
            if (MOSI_frame[DB11] != op_outdoor_old) {
              op_outdoor_old = MOSI_frame[DB11];
              m_cbiStatus->cbiStatusFunction(opdata_outdoor, op_outdoor_old);
            }
          }
          else
            m_cbiStatus->cbiStatusFunction(erropdata_outdoor, MOSI_frame[DB11]);
        }
        break;
      case 0x1f:                              // 8 IU-FANSPEED or 34 OU-FANSPEED
        if ((MOSI_frame[DB6] & 0x80) != 0) {  // 8 IU-FANSPEED
          if (MOSI_type_opdata) {
            if (MOSI_frame[DB10] != op_iu_fanspeed_old) {
              op_iu_fanspeed_old = MOSI_frame[DB10];
              m_cbiStatus->cbiStatusFunction(opdata_iu_fanspeed, op_iu_fanspeed_old & 0x0f);
            }
          }
          else
            m_cbiStatus->cbiStatusFunction(erropdata_iu_fanspeed, MOSI_frame[DB10] & 0x0f);
        }
        else {                                // 34 OU-FANSPEED
          if (MOSI_type_opdata) {
            if (MOSI_frame[DB10] != op_ou_fanspeed_old) {
              op_ou_fanspeed_old = MOSI_frame[DB10];
              m_cbiStatus->cbiStatusFunction(opdata_ou_fanspeed, op_ou_fanspeed_old & 0x0f);
            }
          }
          else
            m_cbiStatus->cbiStatusFunction(erropdata_ou_fanspeed, MOSI_frame[DB10] & 0x0f);
        }
        break;
      case 0x1e:                              // 12 TOTAL-IU-RUN or 37 TOTAL-COMP-RUN
        if ((MOSI_frame[DB6] & 0x80) != 0) {  // 12 TOTAL-IU-RUN
          if (MOSI_type_opdata) {
            if (MOSI_frame[DB11] != op_total_iu_run_old) {
              op_total_iu_run_old = MOSI_frame[DB11];
              m_cbiStatus->cbiStatusFunction(opdata_total_iu_run, op_total_iu_run_old);
            }
          }
          else
            m_cbiStatus->cbiStatusFunction(erropdata_total_iu_run, MOSI_frame[DB11]);
        }
        else {                                // 37 TOTAL-COMP-RUN
          if (MOSI_frame[DB10] == 0x11) {
            if (MOSI_frame[DB11] != op_total_comp_run_old) {
              op_total_comp_run_old = MOSI_frame[DB11];
              m_cbiStatus->cbiStatusFunction(opdata_total_comp_run, op_total_comp_run_old);
            }
          }
          else
            m_cbiStatus->cbiStatusFunction(erropdata_total_comp_run, MOSI_frame[DB11]);
        }
        break;
      case 0x82:
        if ((MOSI_frame[DB6] & 0x80) == 0) {  // 22 ThO-R1
          if (MOSI_type_opdata) {    // operating data
            if (MOSI_frame[DB11] != op_tho_r1_old) {
              op_tho_r1_old = MOSI_frame[DB11];
              m_cbiStatus->cbiStatusFunction(opdata_tho_r1, op_tho_r1_old);
            }
          }
          else
            m_cbiStatus->cbiStatusFunction(erropdata_tho_r1, MOSI_frame[DB11]);
        }
        break;
      case 0x11:
        if ((MOSI_frame[DB6] & 0x80) == 0) {  // 24 COMP
          if (MOSI_type_opdata) {
            if ((MOSI_frame[DB10] << 8 | MOSI_frame[DB11]) != op_comp_old) {
              op_comp_old = MOSI_frame[DB10] << 8 | MOSI_frame[DB11];
              m_cbiStatus->cbiStatusFunction(opdata_comp, op_comp_old & 0x0fff);
            }
          }
          else
            m_cbiStatus->cbiStatusFunction(erropdata_comp, (MOSI_frame[DB10] << 8 | MOSI_frame[DB11]) & 0x0fff);
        }
        break;
      case 0x85:
        if ((MOSI_frame[DB6] & 0x80) == 0) {  // 27 Td
          if (MOSI_type_opdata) {
            if (MOSI_frame[DB11] != op_td_old) {
              op_td_old = MOSI_frame[DB11];
              m_cbiStatus->cbiStatusFunction(opdata_td, op_td_old);
            }
          }
          else
            m_cbiStatus->cbiStatusFunction(erropdata_td, MOSI_frame[DB11]);
        }
        break;
      case 0x90:
        if ((MOSI_frame[DB6] & 0x80) == 0) {  // 29 CT
          if (MOSI_type_opdata) {
            if (MOSI_frame[DB11] != op_ct_old) {
              op_ct_old = MOSI_frame[DB11];
              m_cbiStatus->cbiStatusFunction(opdata_ct, op_ct_old);
            }
          }
          else
            m_cbiStatus->cbiStatusFunction(erropdata_ct, MOSI_frame[DB11]);
        }
        break;
      case 0xb1:
        if ((MOSI_frame[DB6] & 0x80) == 0) {  // 32 TDSH
          if (MOSI_type_opdata) {
            if (MOSI_frame[DB11] != op_tdsh_old) {
              op_tdsh_old = MOSI_frame[DB11];
              m_cbiStatus->cbiStatusFunction(opdata_tdsh, op_tdsh_old / 2);
            }
          }
        }
        break;
      case 0x7c:
        if ((MOSI_frame[DB6] & 0x80) == 0) {  // 33 PROTECTION-No
          if (MOSI_type_opdata) {
            if (MOSI_frame[DB11] != op_protection_no_old) {
              op_protection_no_old = MOSI_frame[DB11];
              m_cbiStatus->cbiStatusFunction(opdata_protection_no, op_protection_no_old);
            }
          }
        }
        break;
      case 0x0c:
        if ((MOSI_frame[DB6] & 0x80) == 0) {  // 36 DEFROST
          if (MOSI_type_opdata) {
            if (MOSI_frame[DB10] != op_defrost_old) {
              op_defrost_old = MOSI_frame[DB10];
              m_cbiStatus->cbiStatusFunction(opdata_defrost, op_defrost_old & 0b1);
            }
          }
        }
        break;
      case 0x13:
        if ((MOSI_frame[DB6] & 0x80) == 0) {  // 38 OU-EEV
          if (MOSI_type_opdata) {
            if ((MOSI_frame[DB12] << 8 | MOSI_frame[DB11]) != op_ou_eev1_old) {
              op_ou_eev1_old = MOSI_frame[DB12] << 8 | MOSI_frame[DB11];
              m_cbiStatus->cbiStatusFunction(opdata_ou_eev1, op_ou_eev1_old);
            }
          }
          else
            m_cbiStatus->cbiStatusFunction(erropdata_ou_eev1, MOSI_frame[DB12] << 8 | MOSI_frame[DB11]);
        }
        break;
      case 0x45: // last error number or count of following error operating data
        if ((MOSI_frame[DB6] & 0x80) != 0) {
          if (MOSI_frame[DB10] == 0x11) {     // last error number
            m_cbiStatus->cbiStatusFunction(erropdata_errorcode, MOSI_frame[DB11]);
          }
          else if (MOSI_frame[DB10] == 0x12) { // count of following error operating data
            erropdataCnt = MOSI_frame[DB11] + 4;
          }
        }
        break;
      case 0x00:  // dummy
        break;
      case 0xff:  // default
        break;
      default:    // unknown operating data
        m_cbiStatus->cbiStatusFunction(opdata_unknwon, MOSI_frame[DB10] << 8 | MOSI_frame[DB9]);
    }
  }
  return call_counter;
}
