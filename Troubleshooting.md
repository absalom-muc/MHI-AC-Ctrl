# Troubleshooting
You find here problems and solutions collected by users. I spend some time to write this documentation. Please take your time to read it before you open a new issue.

If you can't find the solution for your problem here and not in the existing  [Issues](https://github.com/absalom-muc/MHI-AC-Ctrl/issues?q=is%3Aissue) you can open a new issue. But please consider the following topics:
- You should have a basic understanding how to use an ESP8266. That includes the usage of an IDE for programming, compile, flash, OTA and log output via the serial terminal.
- You should have a basic understanding of the MQTT protocol and how to setup a MQTT broker

There are great descriptions in the WWW related to the topics above. Please use your preferred search engine.

If you open a new issue, please consider the following topics:
- Use a use a meaningful title, so that the next user with a similar problem can recognize it
- Which program version do you use?
- Add the exact name of your AC indoor unit, e.g. SRK 35 ZS-S 
- MHI-AC-Ctrl outputs some basic status information via the serial terminal (115000 Baud). Please use it for your first analysis and upload it together with a new Issue
- Sometimes you will be asked to generate a detailed log of the SPI waveforms. This is done via the [SPI-logger](https://github.com/absalom-muc/MHI-AC-Ctrl/blob/master/testprog/SPI_logger.ino). Please don't copy the log output into the Issue description, instead attach it to the Issue.
For finding a solution for your problem, you should use the log output via the serial monitor (115000 Baud).

## Known limitations
MHI-AC-Ctrl doesn't support all functions of the infrared remote control. This is because some functions are not reflected by the SPI payload:
- "auto" for the fan is not supported, only 1, 2, 3,4 for fan are supported
- Vanes left/right is not supported
- Vanes up/down is supported, but with the limitation that if the last command was issued by the IR RC, then the vanes up/down status is not visible in MHI-AC-Ctrl

This should be considered especially when you use the IR RC in parallel to MHI-AC-Ctrl.

## :fire: ESP8266 crashes periodically
For a periodic crash there are different causes possible:

### :fire: Pins not properly connected
During boot the frequency of the SPI pins are checked. This is very helpful to identify SPI signal connection problems. Following values are expected:
- SCK frequency > 3000 Hz
- MOSI frequency < SCK frequency (usually ~ 500 Hz)
- MISO frequency ~ 0 Hz (usually 0Hz)

If you see other values, please re-check
- the connection between the MHI-AC-Ctrl board and your AC
- that there is no short between the three signals

If MISO frequency>10Hz the program will stop and reboot. This prevents damage of your board or AC because there was a signal detected on the MISO pin which should be an output during normal operation.

 Typical faults seen in the past:
- SCK frequency = 0Hz => SCK pin not connected
- SCK frequency < MOSI frequency => SCK and MOSI pins swapped
- MISO frequency = SCK frequency => MISO and SCK pins shorted

### :fire: You use PubSubClient v2.8.0
There was a [bug](https://github.com/knolleary/pubsubclient/issues/747) introduced in PubSubClient version 2.8.0, please use v2.7.0.

## :fire: MQTT connects / disconnects periodically
The HOSTNAME specified in support.h is used as WiFi hostname, MQTT hostname and OTA hostname. In case that you use more than one MHI-AC-Ctrl, e.g. in a multi-split configuration, you have to use unique HOSTNAME to every PCB.

## :fire: AC switches power off sometimes
When there is for >=120 seconds no valid MISO frame, the AC goes into an error state (MQTT topic Errorcode=1) and the AC switches off. You can leave the error state by sending a command via IR-RC or by sending a command via SPI. I assume this is some kind of safety function. This happens when there is >=120 seconds no WiFi or MQTT connection, because the SW loop related to SPI is not served.

## :fire: Receiving the AC status works, but can't change values
Different root causes are possible:
### :fire: MISO pin not connected to AC
Please check this [section](#fire-pins-not-properly-connected)

### :fire: Wrong MQTT set path used
The MQTT path for receiving the status is different from the MQTT path for setting values. Re-check that you use the set-path described in [SW-Configuration.md](https://github.com/absalom-muc/MHI-AC-Ctrl/blob/master/SW-Configuration.md#mqtt-status). Please pay attention to the case sensitivity.
## :fire: Last Vanes up/down status not visible in MHI-AC-Ctrl
That is no bug, but a restriction because the SPI unfortunately doesn't provide the Vanes status when the IR remote control was used for the last command.
## :fire: Fan auto not working
That is no bug, but a restriction because the SPI unfortunately doesn't support 'auto' for the Fan.

## :fire: Log shows errors, but it works nevertheless
With some ACs the SPI connection is fragile because of a different timing. However, this does not appear to be critical and is usually not noticed by the user.

## :fire: Room temperature is toggling
This effect occurs with some AC models. The cause is unclear. 

## :fire: Can't see the device in in Arduino IDE 'tools -> port' for OTA 
I believe this is a general OTA problem, not related to MHI-AC-Ctrl. I made good experience using [espota.py](https://github.com/esp8266/Arduino/blob/master/tools/espota.py) instead of the Arduino IDE for OTA.
