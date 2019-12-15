# SPI Bus
## Interface
The AC is the SPI master and the ESP8266 is the SPI slave. MHI uses the signals SCK, MOSI, MISO.  A slave select signal is not supported.

Name | Function |input/output
------------ | ------------- |--------------
SCK | serial clock | Clock output for AC, input for ESP8266
MOSI | Master Out, Slave In | Data output for MHI, input for ESP8266
MISO | Master In, Slave Out | Input for MHI, output for ESP8266

## Protocol
Clock polarity: CPOL=1 => clock idles at 1, and each cycle consists of a pulse of 0
Clock timing: CPHA=1 => data is captures with the rising clock edge, data changes with the falling edge
## Timing
A byte consists of 8 bits. SCK has a frequency of 32kHz. One bit takes 31.25µs, so one byte takes 8x31.25µs=250µs. There is a pause of 250µs between two bytes.
A frame consists of 20 bytes. A frame consumes 20x2x250µs=10ms. Between 2 frames is a pause of 40ms. So 20 frames per second will be transmitted. The following oscilloscope screenshot shows 3 bytes:
![SPI timing](/images/ScreenImg-11-cut.png)

Yellow: SCK; Purple: MOSI
# SPI Frame
A frame starts with three signature bytes, followed by 15 data bytes and 2 bytes for a checksum. The following table shows the structure of a frame.
In the ESP8266 and in the description mainly the short names are used.

raw byte # | long name | short name
---- | ---- | ----
0|signature byte | SB0
1|signature byte | SB1
2|signature byte | SB2
3|data byte 0| DB0
4|data byte 1| DB1
5|data byte 2| DB2
6|data byte 3| DB3
7|data byte 4| DB4
8|data byte 5| DB5
9|data byte 6| DB6
10|data byte 7| DB7
11|data byte 8| DB8
12|data byte 9| DB9
13|data byte 10| DB10
14|data byte 11| DB11
15|data byte 12| DB12
16|data byte 13| DB13
17|data byte 14| DB14
18|checksum byte high| CBH
19|checksum byte low| CBL

In the description we differ between

MOSI frame -> frame send by MHI-AC, received by ESP8266.

MISO frame -> frame send by ESP8266, received by MHI-AC.

For the testing and evaluation of the protocol the Intesis remote control MH-AC-WIFI-1 was used.

## Signature
The MOSI signature bytes indicate the start of a frame with the 3 bytes 0x6c, 0x80, 0x04.
The usual MISO frame has the signature 0xa9, 0x00, 0x07. (During start of the Intesis remote control once the signature 0x53, 0x80, 0x00 with DB13=0x0f and all other data bytes equal 0x00 is used. The meaning is unclear)

## Data
The following clauses describe the MOSI/MISO decoding for power, mode, fan, vanes, temperature setpoint and room/outdoor temperature
### Power
Power status is coded in MOSI DB0[0].

DB0	| Function
---- | -----
Bit 0| Power
0 | off
1 | on

The same coding is used for setting the Power status. The set bit in the MISO frame is DB0[1].

### Mode
The mode is coded in MOSI DB0[4:2].
<table style="width: 273px; height: 68px;">
<thead>
<tr>
<td style="width: 66.9667px;" colspan="3"><strong>DB0</strong></td>
<td style="width: 66.9667px;"><strong>Function</strong></td>
</tr>
</thead>
<tbody>
<tr>
<td style="width: 66.9667px;">bit 4</td>
<td style="width: 71.4333px;">bit 3</td>
<td style="width: 66.9667px;">bit 2</td>
<td style="width: 66.9667px;">Mode</td>
</tr>
<tr>
<td style="width: 66.9667px;">0</td>
<td style="width: 71.4333px;">0</td>
<td style="width: 66.9667px;">0</td>
<td style="width: 66.9667px;">Auto</td>
</tr>
<tr>
<td style="width: 66.9667px;">0</td>
<td style="width: 71.4333px;">0</td>
<td style="width: 66.9667px;">1</td>
<td style="width: 66.9667px;">Dry</td>
</tr>
<tr>
<td style="width: 66.9667px;">0</td>
<td style="width: 71.4333px;">1</td>
<td style="width: 66.9667px;">0</td>
<td style="width: 66.9667px;">Cool</td>
</tr>
<tr>
<td style="width: 66.9667px;">0</td>
<td style="width: 71.4333px;">1</td>
<td style="width: 66.9667px;">1</td>
<td style="width: 66.9667px;">Fan</td>
</tr>
<td style="width: 66.9667px;">1</td>
<td style="width: 71.4333px;">0</td>
<td style="width: 66.9667px;">0</td>
<td style="width: 66.9667px;">Heat</td>
</tr>
</tbody>
</table>

The same coding is used for setting the Mode. The set bit in the MISO frame is DB0[5].

### Fan
The fan level is coded in MOSI DB1[1:0] and in DB6[6].
<table style="width: 273px; height: 68px;">
<thead>
<tr>
<td style="width: 66.9667px;" colspan="2"><strong>DB1</strong></td>
<td style="width: 66.9667px;"><strong>DB6</strong></td>
<td style="width: 66.9667px;"><strong>Function</strong></td>
</tr>
</thead>
<tbody>
<tr>
<td style="width: 66.9667px;">bit 1</td>
<td style="width: 71.4333px;">bit 0</td>
<td style="width: 66.9667px;">bit 6</td>
<td style="width: 66.9667px;">Fan</td>
</tr>
<tr>
<td style="width: 66.9667px;">0</td>
<td style="width: 71.4333px;">0</td>
<td style="width: 66.9667px;">0</td>
<td style="width: 66.9667px;">1</td>
</tr>
<tr>
<td style="width: 66.9667px;">0</td>
<td style="width: 71.4333px;">1</td>
<td style="width: 66.9667px;">0</td>
<td style="width: 66.9667px;">2</td>
</tr>
<tr>
<td style="width: 66.9667px;">1</td>
<td style="width: 71.4333px;">0</td>
<td style="width: 66.9667px;">0</td>
<td style="width: 66.9667px;">3</td>
</tr>
<tr>
<td style="width: 66.9667px;">x</td>
<td style="width: 71.4333px;">x</td>
<td style="width: 66.9667px;">1</td>
<td style="width: 66.9667px;">4</td>
</tr>
</tbody>
</table>

The same coding is used for setting Fan=1..3. The set bit in the MISO frame is DB1[3].
But for setting Fan=4 DB6[4], not DB6[6] of the MISO frame is used:

<table style="width: 273px; height: 68px;">
<thead>
<tr>
<td style="width: 66.9667px;" colspan="2"><strong>DB1</strong></td>
<td style="width: 66.9667px;"><strong>DB6</strong></td>
<td style="width: 66.9667px;"><strong>Function</strong></td>
</tr>
</thead>
<tbody>
<tr>
<td style="width: 66.9667px;">bit 1</td>
<td style="width: 71.4333px;">bit 0</td>
<td style="width: 66.9667px;">bit 4</td>
<td style="width: 66.9667px;">Fan</td>
</tr>
<tr>
<td style="width: 66.9667px;">0</td>
<td style="width: 71.4333px;">1</td>
<td style="width: 66.9667px;">0</td>
<td style="width: 66.9667px;">4</td>
</tr>
</tbody>
</table>

### Vanes
The vanes up/down level is coded in MOSI DB1[5:4] and in DB0[6].
<table style="width: 273px; height: 68px;">
<thead>
<tr>
<td style="width: 66.9667px;" colspan="2"><strong>DB1</strong></td>
<td style="width: 66.9667px;"><strong>DB0</strong></td>
<td style="width: 66.9667px;"><strong>Function</strong></td>
</tr>
</thead>
<tbody>
<tr>
<td style="width: 66.9667px;">bit 5</td>
<td style="width: 71.4333px;">bit 4</td>
<td style="width: 66.9667px;">bit 6</td>
<td style="width: 66.9667px;">Vanes (up/down)</td>
</tr>
<tr>
<td style="width: 66.9667px;">0</td>
<td style="width: 71.4333px;">0</td>
<td style="width: 66.9667px;">0</td>
<td style="width: 66.9667px;">1</td>
</tr>
<tr>
<td style="width: 66.9667px;">0</td>
<td style="width: 71.4333px;">1</td>
<td style="width: 66.9667px;">0</td>
<td style="width: 66.9667px;">2</td>
</tr>
<tr>
<td style="width: 66.9667px;">1</td>
<td style="width: 71.4333px;">0</td>
<td style="width: 66.9667px;">0</td>
<td style="width: 66.9667px;">3</td>
</tr>
<tr>
<td style="width: 66.9667px;">1</td>
<td style="width: 71.4333px;">1</td>
<td style="width: 66.9667px;">0</td>
<td style="width: 66.9667px;">4</td>
</tr>
<tr>
<td style="width: 66.9667px;">x</td>
<td style="width: 71.4333px;">x</td>
<td style="width: 66.9667px;">1</td>
<td style="width: 66.9667px;">swing</td>
</tr>
</tbody>
</table>
Vanes status is not updated when using IR remote control!

The same coding is used for setting Vanes. The set bit in the MISO frame is DB0[7].


### Room temperature (read only)
The room temperature is coded in MOSI DB3[7:0] according to the formula T[°C]=(DB3[7:0]-61)/4
The resolution is 0.25°C.

### Temperature setpoint
The temperature setpoint is coded in MOSI DB2[6:0] according to the formula T[°C]=DB2[6:0]/2
The resolution of 0.5°C is supported by the wired remote control [RC-E5](https://www.mhi-mth.co.jp/en/products/pdf/pjz012a087b_german.pdf). The IR remote control supports a resolution of 1°C only.
The same coding is used for setting the temperature. The set bit in the MISO frame is DB2[7].

### Error code
The Error code is coded in MOSI DB4[7:0]. It is a number between 0 ... 255. 0 means no error. I've so far not checked if the error code here is consistent with the error numbers in the AC user manual.

## Checksum
The checksum is calculated by the sum of the signature bytes plus the databytes. The low byte of the checksum is stored at byte position 18 and the low byte of the checksum is stored at byte position 19. Maximum value of the checksum is 0x0fe1. Therefore bit [7:4] of the checksum high byte is always 0.
checksum = sum(SB0:SB2) + sum(DB0:DB14)
CBH = checksum[11:8]
CBL = checksum[7:0]


## Settings
For writing MHI-AC - depending on the function - a specific MISO set bit is used:

function | set bit
---- | ----
Power|DB0[1]
Mode|DB0[5]
Fan|DB1[3] and DB6[4] for Fan=4
Vanes|DB0[7]
Tsetpoint|DB2[7]

When writing the corresponding bit in the MOSI frame is set. It seems that it is only cleared when the IR remote control is used.

## Variants
**The following chapter is in draft status!**
Different variants were seen when using the commercial wired RC. The MISO frame (data from RC to MHI-AC) requests data. The variant of the data is identified via MISO-DB9. MHI-AC answers with the same value in MOSI-DB9 (but only when bit2 of MISO-DB14 is set).

The following screenshot shows some SPI traffic:
![MISO MOSI traffic](/images/MISO-MOSI_1.JPG)

In the marked row MISO-DB9=0x80 and MISO-DB14[2] is set, so variant 0 is selected. MOSI-DB9=0x80 indicates that the outdoor temperature is coded in MOSI-DB11.

MISO-DB9	| Variant | MOSI
---- | ----- | -----
0xff| default | default when no SPI RC is connected or no 'special' data are requested
0x80| 0 | DB9=0x80, DB10=0x10, Outdoor temperature: T[°C]=(DB11[7:0]-94)/4 (formula is not finally confirmed). It is rarely possible that DB10=0x20, then DB11 doesn't represent the room temperature. This case is so far not considered in the SW.
0x32| 1 | DB9=0x32, DB10=0x49, DB11=0x02 - meaning unclear
0xf1| 2 | DB9=0xf1, DB10=0x17, DB11=0x06 - meaning unclear
0xd2| 3 | DB9=0xd2, DB10=?, DB11=?, only seen once during heating

note: the numbering of the variants 0..2 is reused from [rjdekker's code here](https://raw.githubusercontent.com/rjdekker/MHI2MQTT/master/src/MHI-SPI2ESP.ino)

## Unknown
In the MOSI frame are more information coded than known for me. E.g. fan active, outdoor fan active, compressor active etc.
I hope that you support to close the gaps.