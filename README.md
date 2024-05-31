# MHI-AC-Ctrl
Reads and writes data (e.g. power, mode, fan status etc.) from/to a Mitsubishi Heavy Industries (MHI) air conditioner (AC) via SPI controlled by MQTT. The AC is the SPI master and the ESP8266 is the SPI slave.

<img src="/images/IoT-MQTT-Panel.jpg" width=200 align="right" />

# Attention:
:warning: You have to open the indoor unit to have access to the SPI. Opening of the indoor unit should be done by 
a qualified professional because faulty handling may cause leakage of water, electric shock or fire! :warning: 

# Prerequisites:
For use of the program you have to connect your ESP8266 (I use a LOLIN(WEMOS) D1 R2 & mini with 80 MHz) via a
cable connector to your air conditioner. This has to be a split device (separated indoor and outdoor unit).
I assume that all AC units of the type "SRK xx ZS-S" / "SRC xx ZS-S" are supported. I use the indoor unit SRK 35 ZS-S and the outdoor unit SRC 35 ZS-S. Users reported that additionally the following models are supported:

- [SRF xx ZJX-S1](https://github.com/absalom-muc/MHI-AC-Ctrl/issues/17#issue-632187165)
- [SRF xx ZMX-S](https://github.com/absalom-muc/MHI-AC-Ctrl/issues/17#issuecomment-704789297)
- [SRF xx ZMXA-S](https://github.com/absalom-muc/MHI-AC-Ctrl/pull/91)
- SRF xx ZF-W
- [SRK xx ZJ-S](https://github.com/absalom-muc/MHI-AC-Ctrl/issues/10)
- [SRK xx ZM-S](https://github.com/absalom-muc/MHI-AC-Ctrl/issues/9)
- [SRK xx ZJX-S](https://github.com/absalom-muc/MHI-AC-Ctrl/issues/17#issuecomment-646469621)
- [SRK xx ZJX-S1](https://github.com/absalom-muc/MHI-AC-Ctrl/issues/17#issuecomment-646968940)
- [SRK xx ZRA-W](https://github.com/absalom-muc/MHI-AC-Ctrl/issues/17#issuecomment-730628655) / DXK xx ZRA-W
- [SRK xx ZSA-W](https://github.com/absalom-muc/MHI-AC-Ctrl/issues/17#issuecomment-891649495) / DXK xx ZSA-W
- [SRK xx ZSPR-S](https://github.com/absalom-muc/MHI-AC-Ctrl/issues/149)
- [SRK xx ZSX-S](https://github.com/absalom-muc/MHI-AC-Ctrl/issues/6#issuecomment-582242372)
- [SRK xx ZSX-W](https://github.com/absalom-muc/MHI-AC-Ctrl/issues/17#issuecomment-643748095)
- [SRK xx ZS-W](https://github.com/absalom-muc/MHI-AC-Ctrl/issues/121)
- [SRK xx ZS-WF](https://github.com/absalom-muc/MHI-AC-Ctrl/issues/121#issuecomment-1938350129)
- [SRC 35Z SA-W]

Unsupported models:
- [SRK71ZEA-S1](https://github.com/absalom-muc/MHI-AC-Ctrl/issues/143)
- [SRK35ZC-S](https://github.com/absalom-muc/MHI-AC-Ctrl/issues/154)
- [SRK 25 ZSPR-S](https://github.com/absalom-muc/MHI-AC-Ctrl/issues/185)
 
If you find out that also other models are supported that are not listed here, please give feedback so that I can expand the list. In general, please recheck if your AC has a CNS connector before you spend time and money to build the hardware.

# Installing:

## Hardware:
The ESP8266 is powered from the AC via DC-DC (12V -> 5V) converter. 
The ESP8266 SPI signals SCL (SPI clock), MOSI (Master Out Slave In) and MISO (Master In Slave Out) are connected via a voltage level shifter 5V <-> 3.3V with the AC. Direct connection of the signals without a level shifter could damage your ESP8266!
More details are described in [Hardware.md](Hardware.md).

## Software:
The program uses the following libraries
 - :warning:[MQTT client library](https://github.com/knolleary/pubsubclient) - please don't use v2.8.0! (because of this [issue](https://github.com/knolleary/pubsubclient/issues/747)). Better use v2.7.0:warning:
 - [ArduinoOTA](https://github.com/esp8266/Arduino/tree/master/libraries/ArduinoOTA) (might be removed in future)
 
and optionally you need for the use of an external temperature sensor DS18x20 the libraries
 - [OneWire](https://www.pjrc.com/teensy/td_libs_OneWire.html)
 - [DallasTemperature](https://github.com/milesburton/Arduino-Temperature-Control-Library)

Please check the GitHub pages to see how to install them (usually via tools -> libraries).

Create a sub-directory "MHI-AC-Ctrl" and copy the files from the latest [release](https://github.com/absalom-muc/MHI-AC-Ctrl/releases) src directory in your MHI-AC-Ctrl sub-directory. You could also use the recently updated version in the [src folder](src) but with the risk that it is more unstable. The stability of the program is better when you compile it for a CPU frequency of 160MHz.
The configuration options are described in [SW-Configuration.md](SW-Configuration.md).

In a previous version (see [here](https://github.com/absalom-muc/MHI-AC-SPY)) I used the Hardware-SPI of the ESP8266. But since the SPI documentation of ESP8266 is poor, I decided to switch to a Software based SPI.
This Software based SPI is reliable and the performance of the ESP8266 is sufficient for this use case.
In case of problems please check the [Troubleshooting guide](Troubleshooting.md).

# Enhancement
If you are interested to have a deeper look on the SPI protocol or want to trace the SPI signals, please check [MHI-AC-Trace](https://github.com/absalom-muc/MHI-AC-Trace). But this is not needed for the standard user of MHI-AC-Ctrl.

# License
This project is licensed under the MIT License - see the LICENSE.md file for details

# Acknowledgments
The coding of the [SPI protocol](https://github.com/absalom-muc/MHI-AC-Trace/blob/main/SPI.md) of the AC is a nightmare. Without [rjdekker's MHI2MQTT](https://github.com/rjdekker/MHI2MQTT) I had no chance to understand the protocol! Unfortunately rjdekker is no longer active on GitHub. He used an Arduino plus an ESP8266 for his project.
Also thank you very much on the authors and contributors of [MQTT client](https://github.com/knolleary/pubsubclient), [ArduinoOTA](https://github.com/esp8266/Arduino/tree/master/libraries/ArduinoOTA), [OneWire](https://www.pjrc.com/teensy/td_libs_OneWire.html) and [DallasTemperature](https://github.com/milesburton/Arduino-Temperature-Control-Library) libraries.

Last but not least thank you for the implementation of MHI-AC-Ctrl in [different environments](https://github.com/absalom-muc/MHI-AC-Ctrl/blob/master/SW-Configuration.md#integration-examples) (FHEM, Tasmota, Home Assistant etc.)
