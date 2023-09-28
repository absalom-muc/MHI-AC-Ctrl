MHI-AC-Ctrl by absalom-muc

**v2.8** (September 2023)
- when ds18x20 used and get disconnected, fallback to  IU temperature sensor by [glsf91](https://github.com/glsf91)
- added status and control 3D auto and vanes L/R (extended frame size (33) used like WF-RAC module). You have to enable this! [3d auto and vanes l/r seems possible over CNS #77](https://github.com/absalom-muc/MHI-AC-Ctrl/issues/77) by [glsf91](https://github.com/glsf91) 
- added SRK35ZC-S to unsupported list [Addition to unsupported list #154](https://github.com/absalom-muc/MHI-AC-Ctrl/issues/154) by [glsf91](https://github.com/glsf91)

**v2.7R4** (April 2023)
- changed setup WiFi connection to async; module starts already communicating with AC during WiFi setup and also during scanning when WiFI_SEARCHStrongestAP is used by [glsf91](https://github.com/glsf91)
- added CONTINUE_WITHOUT_MQTT; module keeps communicating with AC if MQTT is disconnected. See also description in SW-Configuration.md and [Question: why mhi_ac_ctrl_core.loop only when MQTT connected? #144](https://github.com/absalom-muc/MHI-AC-Ctrl/issues/144) by [glsf91](https://github.com/glsf91)
- fix bug for publish list of access points by [glsf91](https://github.com/glsf91)
- added SRK71ZEA-S1 to unsupported list [Addition to unsupported list #143](https://github.com/absalom-muc/MHI-AC-Ctrl/issues/143) by [glsf91](https://github.com/glsf91)
- added Web page [Webpage for accessing MQTT data #141](https://github.com/absalom-muc/MHI-AC-Ctrl/issues/141) to integration list by [glsf91](https://github.com/glsf91)

**v2.7R3** (March 2023)
-  fix some compiler warnings by [glsf91](https://github.com/glsf91)
-  change sending MISO with faster refresh of data every 20s by [glsf91](https://github.com/glsf91)
-  avoid jitter with internal temperature sensor, now updating atmost every 5s by [glsf91](https://github.com/glsf91)
-  fix print wifi encryption type by [glsf91](https://github.com/glsf91)
-  skip not usable values for DB18B20 by [glsf91](https://github.com/glsf91)

**v2.7R2** (March 2023)
-  Added energy kWh from airco by [glsf91](https://github.com/glsf91).

**v2.7R1** (February 2023)
-  Fix for fan not showing 4 and Auto after powerdown AC [pull request #132](https://github.com/absalom-muc/MHI-AC-Ctrl/pull/132#) according to [issue #99](https://github.com/absalom-muc/MHI-AC-Ctrl/issues/99#issuecomment-1407615341) by [glsf91](https://github.com/glsf91).

**v2.6** (January 2023)
- Fix of [Wifi reconnect takes too long #125](https://github.com/absalom-muc/MHI-AC-Ctrl/issues/125)
- Typo related to the unit of TD fixed in MHI-AC-Ctrl-core.h and SW-Configuration.md

**v2.6R5** (December 2022, draft version)
- Added ROOM_TEMP_DS18X20_OFFSET for issue [Adding offset for external temperature sensor #119](https://github.com/absalom-muc/MHI-AC-Ctrl/issues/119)

**v2.6R4** (December 2022, draft version)
- Added Enhance resolution of Tsetpoint according to [pull request #123](https://github.com/absalom-muc/MHI-AC-Ctrl/pull/123) by [glsf91](https://github.com/glsf91).

**v2.6R3** (December 2022, draft version)
- Added switch for Troom filtering to address [issue #82](https://github.com/absalom-muc/MHI-AC-Ctrl/issues/82).

**v2.6R2** (August 2022, draft version)
- final implementation for fan control incl. 'Auto', see [issue](https://github.com/absalom-muc/MHI-AC-Ctrl/issues/99). This is a breaking change, because in the past only the values 1, 2 , 3, 4 were supported for fan status and fan control. Now "Auto" was added.
- added temporary operating data "unknwon" to find out which functions are supported by SPI dependent on the indoor AC unit.

**v2.6R1** (August 2022, draft version)
- new implementation for fan control incl. 'Auto', see [issue](https://github.com/absalom-muc/MHI-AC-Ctrl/issues/99)

**v2.5** (August 2022)
- release w/o changes


**v2.5R3** (June 2022, draft version)
- moved the functions for frequency measurement from MHI-AC-Ctrl.ino to support.cpp
- moved some MQTT init handling from MHI-AC-Ctrl.ino void setup() to MQTTloop()
- moved long previousMillis = millis to inside void loop() as static
- added parameter WiFI_SEARCH_FOR_STRONGER_AP_INTERVALL
- Reworked WiFi network re-scan function
- added work around for "not updating WiFi.status()", see https://github.com/esp8266/Arduino/issues/7432
- ICACHE_RAM_ATTR is deprecated, replaced by IRAM_ATTR
- removed switch ROOM_TEMP_MQTT in support.h, a Troom MQTT set command is always considered  


**v2.5R21** (December 2021, draft version)
- calculation of RETURN-AIR in MHI-AC-Ctrl.ino corrected
- Removed the option #define ROOM_TEMP_IU for adaption by the user in support.h. Now it is generated automatically when ROOM_TEMP_DS18X20 and ROOM_TEMP_MQTT are not defined
- Reworked WiFi network re-scan function
- increased resolution of Tsetpoint to 0.5°C

**v2.5R1** (November 2021, draft version)
- Version number now located in support.h, published once MQTT connection is available

**Versioning changed**

Released versions are availabe on the [release page](https://github.com/absalom-muc/MHI-AC-Ctrl/releases)
All other versions are draft. A new version ends with "R1", "R2"-postfix etc. before it is released w/o "R"-postfix, e.g. v2.5R1 => v2.5R2 => v2.5
For documentation updates usually no new version number is created.

**v2.3** (September 2021, draft version)
- for MHI_AC_Ctrl_Core::loop return value err_msg_timeout replaced by the more detailed return values err_msg_timeout_SCK_low and err_msg_timeout_SCK_high added
- output of the number of WiFi and MQTT lost since last reset
- output of the BSSID used for the WiFi connection
- use of the strongest WiFi access point (testing incomplete)
- option added to use the room temperature from DS18x20 sensor or from MQTT topic instead of AC IU sensor

**v2.2** (February 2021)
- PCB layout update to v2.2 allows to plug-in MHI-AC-Ctrl directly into the AC without using a cable

**v2.03** (July 2020)
- Functionality for POWERON_WHEN_CHANGING_MODE improved in MHI-AC-Ctrl.ino
- WiFi mode restricted to STA (no more AP) in support.cpp

**v2.02** (June 2020)
- Functionality for POWERON_WHEN_CHANGING_MODE enhanced, the according #define shifted to support.h

**v2.01** (June 2020)
- No functional changes
- Further reduction of memory usage
- Compiler warnings removed
- set-mechanism reworked
- SCK by SCK_PIN replaced according to [pull request 19](https://github.com/absalom-muc/MHI-AC-Ctrl/pull/19)
- Shift void MeasureFrequency() from support.cpp to MHI-AC-Ctrl-core.cpp

**v2.0** (June 2020)
- code refactored to allow simplified adaption for other frameworks (e.g. Tasmota, Home Assistant), main changes
	- MHI-AC-Ctrl-core separated according to [Issue #13](https://github.com/absalom-muc/MHI-AC-Ctrl/issues/13)
	- Changeable defs for all topics and payload according to this [Pull request](https://github.com/absalom-muc/MHI-AC-Ctrl/pull/15)
- Option added to switch on the AC when the mode is changed according to [Issue #14](https://github.com/absalom-muc/MHI-AC-Ctrl/issues/14)
- Added operating data THO-R1, THO-R2, THO-R3, THO-R1 and TD added
- Handling for operating data "PROTECTION-No" and "CT" corrected
- Output of the frame raw data is no longer supported
- MQTT.md replaced by [SW-Configuration.md](SW-Configuration.md)

**v1.4** (April 2020)
- Vanes MQTT status corrected
- Frequency measurement limits adapted, ESP is halted only when there is a toggle on MISO detected, results of the measurement will be published via MQTT
- some [operating data](https://github.com/absalom-muc/MHI-AC-Ctrl/blob/master/MQTT.md#mqtt-topics-related-to-operating-data) added (MQTT path changed)
- [error operating data](https://github.com/absalom-muc/MHI-AC-Ctrl/blob/master/MQTT.md#mqtt-topics-related-to-error-data) added
- reset via MQTT for ESP8266 added
- support of an external DS18x20

**v1.3** (March 2020)
- MHI-AC-Ctrl.h added
- Support of Mitsubishi's ZSK series by acceptance of additionally 0x6d as the first MOSI signature byte (SB0) according to [this issue](https://github.com/absalom-muc/MHI-AC-Ctrl/issues/6).
- MQTT authentication support added
- AC operating data are supported
- Outdoor temperature MQTT topic is shifted to operating data
- Frequency measurement of the signals to check correct connection of MHI-AC-Ctrl board to AC
- MQTT topic connection values changed from false/true to 0/1 to be aligned with the documentation

**v1.2** (January 2020)

- Mode setting corrected
- Vanes is now only published if DB0[7]=1 or DB1[7]=1, else last vanes change was via IR-RC (so not visible via SPI)
- Options for vanes MQTT values adapted, it is now 1,2,3,4,Swing (before it was 1..5)
- Data types of some variables adapted
- MQTT status will be published after broker was down
- Raw data publishing commented out to reduce the broker load (worst case 20/s)
- Remove leading space characters for MQTT room and outdoor temperature
- MQTT topic Runtime removed

**v1.1** (December 2019)

- Error code (DB4) added
 OTA is now also working when waiting for a MQTT connection

**v1.0** (December 2019)

- initial
