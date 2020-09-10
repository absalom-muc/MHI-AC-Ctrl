# Hardware

## Schematic
![schematic](/images/MHI-AC-Ctrl_Schematic.png)

## PCB
![PCB](/images/PCB.png)

You find the eagle schematic, the gerber files and the PCB in the [eagle folder](/eagle). Alternatively to the PCB you could use a breadboard.

## Assembled PCB

<img src="/images/Assembled-PCB.jpg" width=400 align="center"/>


## Bill of Material
Part |Value           |Package
---- | ----            |----
C1   |22µ/50V          |E5-10,5
C2   |100n             |C025-024X044
C3   |22p              |C025-024X044
LS1  |LEVEL-SHIFTER-4CH|LEVEL-SHIFTER-4CH
U$1  |WEMOS-D1-MINI    |WEMOS-D1-MINI
U$4  |JST-PH_5-PIN_HEADER|JST_B5B-PH_HEADER
U1   |TSR_1-2450       |TSR-1

#### Optional parts for temp sensor
Part |Value           |Package
---- | ----            |----
R1   | 4.7k Ohm        | 
DS   | DS18B20         | 

## Connector
The AC provides the signals via the CNS connector. It has 5 pins with a pitch of 2.5 mm. It is out of the [XH series from JST](http://www.jst-mfg.com/product/detail_e.php?series=277). The position of the connector is visible on the following photo of the indoor unit PCB.
![Indoor PCSchematicB](/images/SRK-PCB.jpg)

The PCB of the remote control uses also a 5 pin CNS connector but with a pitch of 2 mm. It is out of the [PH series from JST](http://www.jst-mfg.com/product/detail_e.php?series=199).
Please consider that there is no 1:1 connection between AC and the remote control. The Pins SCL and MOSI are (unfortunately) swapped. Your cable connection of the AC with the remote control PCB should consider that.
![Indoor PCSchematicB](/images/SRK-PCB-RC.jpg)
<img src="/images/AC_to_MHI-AC-Ctrl.png" width="600" align="center">

Please consider using an oscilloscope to confirm the layout of the pins before you connect the remote control PCB to your AC.


:warning: **Opening of the indoor unit should be done by a qualified professional because faulty handling may cause leakage of water, electric shock or fire!** :warning:

## Power Supply
The JST connector provides +12V. The DC-DC converter [TSR 1-2450](https://www.tracopower.com/products/browse-by-category/find/tsr-1/3/) is used to convert the +12V to +5V.

## Signal Connection
The ESP8266 SPI signals SCL (SPI clock), MOSI (Master Out Slave In) and MISO (Master In Slave Out) are connected via a voltage level shifter 5V <-> 3.3V with the AC. Direct connection of the signals without a level shifter could damage your ESP8266!
In a previous version I used a resistor voltage divider, but since the SPI of the AC has high impedance outputs, a voltage level shifter seems to be better. The voltage level shifters are bi-directional. That means the according ESP8266 pins (SCL, MOSI, MISO) could be inputs or outputs. You can use it for different [configurations](/Configurations.md).

## External Temperature Sensor
With version v1.4 of the software an external temperature sensor DS18x20 is (optional) supported. The layout of the MHI-AC-Ctrl PCB does support that connection. The DS18x20 is connected to GND, +3V3 and GPIO 4 (D2). Addtionally you need a 4k7 resistor between DQ and +3V3.

## Hints for Assembly
The photo of the assembled PCB shows a 2-pin-connector int the right top corner not mentioned in the bill of material. 
<img src="/images/Assembled-PCB-mark.jpg" width=400 align="center"/>

I use this connector for test purposes. You should **not** solder this connector because if you short-circuit these pins e.g. by a jumper, then the AC power supply is short-circuited and could damage the AC. :warning:
