# SPI Bus
## Interface
The AC is the SPI master and the ESP8266 is the SPI slave. MHI uses the signals SCK, MOSI and MISO.  A slave select signal is not supported.

Name | Function |input/output
------------ | ------------- |--------------
SCK | serial clock | Clock output for AC, input for ESP8266
MOSI | Master Out, Slave In | Data output for MHI, input for ESP8266
MISO | Master In, Slave Out | Input for MHI, output for ESP8266

## Protocol
Clock polarity: CPOL=1 => clock idles at 1, and each cycle consists of a pulse of 0
Clock timing: CPHA=1 => data is captured with the rising clock edge, data changes with the falling edge
## Timing
T<sub>Frame</sub> Time for one frame   
T<sub>FramePause</sub> Time between two frames   
T<sub>Byte</sub> Time for one byte   
T<sub>BytePause</sub> Time between two bytes   
T<sub>Bit</sub> Time for one bit   

![timing](/images/timing_frame-byte.png)

The following timing is used by SRK xx ZS-S

T<sub>Frame</sub>|T<sub>FramePause</sub>|T<sub>Byte</sub>|T<sub>BytePause</sub>|T<sub>Bit</sub>
---|---|---|---|---|
10ms|40ms|250µs|250µs|31.25µs

Other models could have different timing

A frame consists of 20 bytes. A frame consumes 20x2x250µs=10ms. Between 2 frames is a pause of 40ms. 20 frames per second will be transmitted. The following oscilloscope screenshot shows 3 bytes:
![SPI timing](/images/ScreenImg-11-cut.png)

Yellow: SCK; Purple: MOSI
# SPI Frame
A frame starts with three signature bytes, followed by 15 data bytes and 2 bytes for a checksum. The following table shows the structure of a frame.
In the ESP8266 program code and in the description mainly the short names are used.

raw byte # | long name | short name
---- | ---- | ----
0|signature byte 0| SB0
1|signature byte 1| SB1
2|signature byte 2| SB2
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

For the testing and evaluation of the protocol the remote controls [MH-AC-WIFI-1](https://www.intesisbox.com/de/mitsubishi-heavy-ascii-wifi-ac-mh-ac-wmp-1/gateway/) and [RC-E5](https://www.mhi-mth.co.jp/en/products/pdf/pjz012a087b_german.pdf) were used.

## Signature
The MOSI signature bytes indicate the start of a frame with the 3 bytes 0x6c, 0x80, 0x04. The first signature byte varies with the AC. For some [Mitsubishi AC models](https://github.com/absalom-muc/MHI-AC-Ctrl/issues/6#issue-558530669) it is 0x6d.
The MISO frame has the signature 0xa9, 0x00, 0x07. 

## Data
The following clauses describe the MOSI/MISO decoding for power, mode, fan, vanes, temperature setpoint and room temperature.
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
note: With the IR remote control you can select "auto" for the fan. But this is not reflected by the SPI payload.

### Vanes
Vanes up/down swing is enabled in DB0[6]. When vanes up/down swing is disabled the vanes up/down position in MOSI DB1[5:4] is used. 

DB0	| Function
---- | -----
Bit 6| vanes up/down swing
0 | off
1 | on

<table style="width: 273px; height: 68px;">
<thead>
<tr>
<td style="width: 66.9667px;" colspan="2"><strong>DB1</strong></td>
<td style="width: 66.9667px;"><strong>Function</strong></td>
</tr>
</thead>
<tbody>
<tr>
<td style="width: 66.9667px;">bit 5</td>
<td style="width: 71.4333px;">bit 4</td>
<td style="width: 66.9667px;">Vanes up/down position</td>
</tr>
<tr>
<td style="width: 66.9667px;">0</td>
<td style="width: 71.4333px;">0</td>
<td style="width: 66.9667px;">1</td>
</tr>
<tr>
<td style="width: 66.9667px;">0</td>
<td style="width: 71.4333px;">1</td>
<td style="width: 66.9667px;">2</td>
</tr>
<tr>
<td style="width: 66.9667px;">1</td>
<td style="width: 71.4333px;">0</td>
<td style="width: 66.9667px;">3</td>
</tr>
<tr>
<td style="width: 66.9667px;">1</td>
<td style="width: 71.4333px;">1</td>
<td style="width: 66.9667px;">4</td>
</tr>
</tbody>
</table>

note: Vanes status is **not** applicable when using IR remote control. The latest vanes status is only visible when it was changed by the SPI-RC.
The latest vanes status is only visible when MOSI DB0[7]=1 or MOSI DB1[7]=1.

The same coding is used for setting vanes.
The set bit for enabling vanes up/down swing in the MISO frame is DB0[7].
The set bit for vanes up/down position in the MISO frame is DB1[7].

### Room temperature
The room temperature is coded in MOSI DB3[7:0] according to the formula T[°C]=(DB3[7:0]-61)/4
The resolution is 0.25°C. When writing a value <255, this temperature value is used instead of the build in room temperature sensor.

### Temperature setpoint
The temperature setpoint is coded in MOSI DB2[6:0] according to the formula T[°C]=DB2[6:0]/2
The resolution of 0.5°C is supported by the wired remote control [RC-E5](https://www.mhi-mth.co.jp/en/products/pdf/pjz012a087b_german.pdf). The IR remote control supports a resolution of 1°C only.
The same coding is used for setting the temperature. The set bit in the MISO frame is DB2[7].

### Error code (read only)
The Error code is a number 0 ... 255 in MOSI DB4[7:0]. 0 means no error. 
According my understanding the error codes listed [here](https://www.hrponline.co.uk/media/pdf/5f/54/33/HRP_NEW_ServiceSupportHandbook.pdf#page=14) are supported, but I haven't really checked it.

## Checksum (read only)
The two byte checksum is calculated by the sum of the signature bytes plus the databytes. The high byte of the checksum CBH is stored at byte position 18 and the low byte of the checksum CBL is stored at byte position 19.

    checksum[15:0] = sum(SB0:SB2) + sum(DB0:DB14)
    CBH = checksum[15:8]
    CBL = checksum[7:0]


## Settings
For writing MHI-AC - depending on the function - a specific MISO set-bit is used:

function | set-bit
---- | ----
Power|DB0[1]
Mode|DB0[5]
Fan|DB1[3] and DB6[4] for Fan=4
Vanes|DB0[7] for swing and DB1[7] for up/down position
Tsetpoint|DB2[7]

Once a set-bit is set to 1, the according bit in the MOSI frame becomes and remains '1' until the IR remote control is used.
All set-bits are cleared when the IR remote control is used. Settings can be done independent from the power state.

## Operation Data

You can read different operating data of the AC related to the indoor and outdoor unit.
The following example shows the reading of the outdoor air temperature:

MISO-DB6 | MISO-DB9 | MISO-DB10 | MISO-DB11 | MISO-DB12 | MOSI-DB9 | MOSI-DB10 | MOSI-DB11 
 --------| ---------| ----------| ----------| ----------| ---------| ----------| ----------
  0x40   | 0x80     | 0xff      | 0xff      | 0xff      | 0x80     | 0x10      |  temperature 

Please check the program code for further details. You find [here](https://github.com/absalom-muc/MHI-AC-Ctrl/blob/master/MQTT.md#mqtt-topics-related-to-operating-data) the list of supported topics related to operating data.

## Last Error Operation Data
The AC stores some operation data of the last error event. This error operation data can be read with the command:

MISO-DB6[7]  | MISO-DB9 
 ------------| --------
     1       | 0x45    
  
AC answers with the following MOSI data sequence:

MOSI-DB6[7] | MOSI-DB9 | MOSI-DB10 | MOSI-DB11 
 ---------- | -------- | --------- | --------  
 1          | 0x45     | 0x11      | error code

If error code > 0 the following MOSI data sequence is send in addition:

MOSI-DB6[7] | MOSI-DB9 | MOSI-DB10 | MOSI-DB11 
 ---------- | -------- | --------- | --------  
 1          | 0x45     | 0x12      | count of the following error operation data

Examples for error operation data

MOSI-DB6[7] | MOSI-DB9 | MOSI-DB10 | MOSI-DB11 
 ---------- | -------- | --------- | --------  
 1          | 0x02     | 0x30=Stop, 0x31=Dry, 0x32=Cold, 0x33=Fan, 0x34=heat
 1          | 0x05     | 0x33      | SET TEMP = MOSI-DB11 / 2
 1          | 0x1e     | 0x30      | TOTAL I/U RUN = MOSI-DB11 * 100


## Unknown
In the SPI frames are more information coded than known for me. In MOSI-DB13 some bits seem to represent the status of the outdoor unit.
I would appreciate your support to close these gaps.
