MHI-AC-Ctrl by absalom-muc

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