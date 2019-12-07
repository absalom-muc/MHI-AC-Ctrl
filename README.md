# MHI-AC-Ctrl
Reads and writes data (e.g. power, mode, fan status etc.) from/to a Mitsubishi Heavy Industries (MHI)
air conditioner (AC) via SPI controlled by MQTT. The AC is the SPI master and the ESP8266 is the SPI slave.
The program has a draft status and your input is welcome.

![IoT MQTT Panel](/images/IoT-MQTT-Panel.jpg)

# Attention:
You have to open the indoor unit to have access to the SPI. Opening of the indoor unit should be done by 
a qualified professional because faulty handling may cause leakage of water, electric shock or fire!

# Prerequisites:
For use of the program you have to connect your ESP8266 (I use a LOLIN(WEMOS) D1 R2 & mini with 80 MHz) via a
cable connector to your air conditioner. This has to be a split device (separated indoor and outdoor unit).
I assume that all indoor units of the type SRC xx ZS-S are supported. I use the indoor unit SRK 35 ZS-S and the outdoor unit SRC 35 ZS-S.
  
# Installing:

## Hardware:
The ESP8266 is powered from the AC via DC-DC (12V -> 5V) converter. 
The ESP8266 SPI signals SCL (SPI clock), MOSI (Master Out Slave In) and MISO (Master In Slave Out) are connected via a voltage level shifter 5V <-> 3.3V with the AC. Direct connection of the signals without a level shifter could damage your ESP8266!
More details are described [here](/Hardware.md)

## Software:
The program uses the [MQTT client library](https://github.com/knolleary/pubsubclient) from Nick O'Leary (knolleary). Thank you Nick - great job!
Please check his GitHub page to see how to install it.
Dwonload MHI-AC-Ctrl.ino and adapt ssid, password and the MQTT server data in the row MQTTclient.setServer("ds218p", 1883);
Currently ArduinoOTA is used for remote upload.

In a previous version (see [here](https://github.com/absalom-muc/MHI-AC-SPY)) I used the Hardware-SPI of the ESP8266. But since the SPI documentation of ESP8266 is poor, I decided to switch to a Software based SPI.
This Software based SPI is reliable and the performance of the ESP8266 is sufficient for this use case.

## MQTT
The control via MQTT is described [here](/MQTT.md)

# License
This project is licensed under the MIT License - see the LICENSE.md file for details

# Acknowledgments
The coding of the SPI protocol of the AC is a nightmare. Without [rjdekker's MHI2MQTT](https://github.com/rjdekker/MHI2MQTT) I had no chance to understand the protocol! Unfortunatelly rjdekker is not longer active on GitHub. He used an Arduino plus an ESP8266 for his project.
