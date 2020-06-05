# MHI-AC-Ctrl
Reads and writes data (e.g. power, mode, fan status etc.) from/to a Mitsubishi Heavy Industries (MHI) air conditioner (AC) via SPI controlled by MQTT. The AC is the SPI master and the ESP8266 is the SPI slave.
The program has a draft status and your input is welcome.

<img src="/images/IoT-MQTT-Panel.jpg" width=200 align="right" />

# Attention:
:warning: You have to open the indoor unit to have access to the SPI. Opening of the indoor unit should be done by 
a qualified professional because faulty handling may cause leakage of water, electric shock or fire! :warning: 

# Prerequisites:
For use of the program you have to connect your ESP8266 (I use a LOLIN(WEMOS) D1 R2 & mini with 80 MHz) via a
cable connector to your air conditioner. This has to be a split device (separated indoor and outdoor unit).
I assume that all AC units of the type "SRK xx ZS-S" / "SRC xx ZS-S" are supported. I use the indoor unit SRK 35 ZS-S and the outdoor unit SRC 35 ZS-S.
User reported that also [SRK20ZSA-W](https://github.com/absalom-muc/MHI-AC-Ctrl/issues/7#issuecomment-583957375) and [SRK xx ZSX-S](https://github.com/absalom-muc/MHI-AC-Ctrl/issues/6#issue-558530669) are supported.
# Installing:

## Hardware:
The ESP8266 is powered from the AC via DC-DC (12V -> 5V) converter. 
The ESP8266 SPI signals SCL (SPI clock), MOSI (Master Out Slave In) and MISO (Master In Slave Out) are connected via a voltage level shifter 5V <-> 3.3V with the AC. Direct connection of the signals without a level shifter could damage your ESP8266!
More details are described in [Hardware.md](Hardware.md).

## Software:
The program uses the following libraries
 - [MQTT client library](https://github.com/knolleary/pubsubclient)
 - [ArduinoOTA](https://github.com/esp8266/Arduino/tree/master/libraries/ArduinoOTA) (might be removed in future)
 
and optionally you need for the use of an external temperature sensor DS18x20 the libraries
 - [OneWire](https://www.pjrc.com/teensy/td_libs_OneWire.html)
 - [DallasTemperature](https://github.com/milesburton/Arduino-Temperature-Control-Library)

Please check the GitHub pages to see how to install them (usually via tools -> libraries).

Create a sub-directory "MHI-AC-Ctrl" and copy the files from the [src directory](src) in your MHI-AC-Ctrl sub-directory.
The configuration options are described in [SW-Configuration.md](SW-Configuration.md).

In a previous version (see [here](https://github.com/absalom-muc/MHI-AC-SPY)) I used the Hardware-SPI of the ESP8266. But since the SPI documentation of ESP8266 is poor, I decided to switch to a Software based SPI.
This Software based SPI is reliable and the performance of the ESP8266 is sufficient for this use case.
In case of problems please check the serial debug output first.

# License
This project is licensed under the MIT License - see the LICENSE.md file for details

# Acknowledgments
The coding of the [SPI protocol](SPI.md) of the AC is a nightmare. Without [rjdekker's MHI2MQTT](https://github.com/rjdekker/MHI2MQTT) I had no chance to understand the protocol! Unfortunately rjdekker is no longer active on GitHub. He used an Arduino plus an ESP8266 for his project.
Also thank you very much on the authors and contributors of [MQTT client](https://github.com/knolleary/pubsubclient), [ArduinoOTA](https://github.com/esp8266/Arduino/tree/master/libraries/ArduinoOTA), [OneWire](https://www.pjrc.com/teensy/td_libs_OneWire.html) and [DallasTemperature](https://github.com/milesburton/Arduino-Temperature-Control-Library) libraries.