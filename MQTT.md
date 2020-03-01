# Introduction
The program uses the [MQTT client library](https://github.com/knolleary/pubsubclient) from Nick O'Leary (knolleary).
If you are not familiar with MQTT you find on the Internet endless numbers of descriptions and tutorials. My favorites are [here](https://www.hivemq.com/blog/how-to-get-started-with-mqtt/) and [here](https://www.heise.de/developer/artikel/Kommunikation-ueber-MQTT-3238975.html).
I recommend [MQTT Explorer](http://mqtt-explorer.com/) an all-round MQTT client that provides a structured topic overview for the first steps.

Please adapt [MHI-AC-Ctrl.h](https://github.com/absalom-muc/MHI-AC-Ctrl/blob/master/src/MHI-AC-Ctrl.h) according to your needs.
You have to adapt the server (broker) name and the port if needed:

```
#define MQTT_SERVER "ds218p"  // broker name or IP address of the broker
#define MQTT_PORT 1883        // port number used by the broker
```

If you want to use MQTT authentication enter user name and password:
```
#define MQTT_USER ""          // if authentication is not used, leave it empty
#define MQTT_PASSWORD ""      // if authentication is not used, leave it empty
```

# MQTT topic
The read topic level consists of the MQTT_PREFIX and the function name separated by a slash, e.g. *MHI-AC-Ctrl/Power*.
For writes the topic level consists of the MQTT_PREFIX, the prefix for set commands (MQTT_SET_PREFIX) and the function name, each separated by a slash, e.g. *MHI-AC-Ctrl/set/Power*.  
You can change MQTT_PREFIX and MQTT_SET_PREFIX in the program, per default it is

```
#define MQTT_PREFIX "MHI-AC-Ctrl/"          // basic prefix used for publishing AC data
#define MQTT_SET_PREFIX MQTT_PREFIX "set/"  // prefix for subscribing set commands
```
Please pay attention to the case sensitivity.

The following data topics/payload are available (prefix is not listed)
They are only published when there is a change of the message. The retained flag is 'true'.
When writing data, the retain flag shall be 'false'.

topic|r/w|value|comment
---|---|---|---
Power|r/w|"On", "Off"|
Mode|r/w|"Auto", "Dry", "Cool", "Fan" or "Heat"
Tsetpoint|r/w|18 ... 30|Target room temperature (integer) in °C
Fan|r/w|1 ... 4|Fan level
Vanes|r/w|1,2,3,4,"Swing"|Vanes up/down position
Troom|r|0 ... 35|Room temperature (float) in °C.
Errorcode|r|0 .. 255|error code (unsigned int)

Additionally, the following program status topics are available:

topic | r/w| value |comment
---|---|---|---
connected|r|0, 1|ESP8266 connection status to broker
synced|r|0, 1|synchronization to SPI rx frame
raw|r||raw data *1)
cmd_received|r|"o.k.", "unknown command" or "invalid parameter"|feedback for last set command

*1) The raw data is a byte array of 43 bytes. The following table shows the structure. The raw data are only published when there is change of the data:

byte |comment
---|---
[0]|frame count high byte
[1]|frame count low byte
[2:21]|rx SPI frame (MOSI) - 20 bytes
[22:41]|tx SPI frame (MISO) - 20 bytes
[42]|number of identical MOSI frames

Publishing of the raw data is per default commented out to reduce broker's work load.

# MQTT topics related to operating data

MHI-AC-Ctrl can also provide operating data of the indoor and outdoor unit. This data is not needed for daily use, but might be interesting in specific use cases. Operating data is only published when there is a change of the content. The retained flag is 'true'.
The path to the operating data topic can be adapted.

```
#define MQTT_OP_PREFIX "op/"    // prefix for publishing operating data
```

Without changes of the path, subscribe to *MHI-AC-Ctrl/op/#* for receiving all operating data.   
Currently the following operating data topics in double quotes are supported

```
  { 0xc0, 0x80, 0xff, 0xff, 0xff },  //  3 "RETURN-AIR" [°C]
  { 0xc0, 0x1e, 0xff, 0xff, 0xff },  // 12 "TOTAL-IU-RUN" [h]
  { 0x40, 0x80, 0xff, 0xff, 0xff },  // 21 "OUTDOOR" [°C]
  { 0x40, 0x7c, 0xff, 0xff, 0xff },  // 33 "PROTECTION-No"
  { 0x40, 0x1e, 0xff, 0xff, 0xff },  // 37 "TOTAL COMP RUN" [h]
  { 0x40, 0x13, 0xff, 0xff, 0xff },  // 38 "OU-EEV" [Puls]
```
The list of supported operating data will be enhanced in future versions. 
To reduce the MQTT load, you can comment out the lines you are not interested. But at least one line must be available.
You can find some hints related to the meaning of the operating data [here](https://www.hrponline.co.uk/files/images/HRP/Catalogues/HRP_NEW_ServiceSupportHandbook.pdf#page=7).

# MQTT topics related to error data
Some operating data during the last error are stored in the AC. Implementation in MHI-AC-Ctrl is planned for future version.
