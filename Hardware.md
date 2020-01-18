# Hardware

## Schematic
![schematic](/images/MHI-AC-Ctrl_Schematic.png)

## PCB
![PCB](/images/PCB.png)

You find the eagle schematic and the PCB in the [eagle folder](/eagle).

## Assembled PCB
![Assembled PCB](/images/Assembled-PCB.jpg)

## Bill of Material
Part |Value           |Package
---- | ----            |----
C1   |22µ/50V          |E5-10,5
C2   |100n             |C025-024X044
C3   |22p              |C025-024X044
JP1  |                 |JP1
LS1  |LEVEL-SHIFTER-4CH|LEVEL-SHIFTER-4CH
U$1  |WEMOS-D1-MINI    |WEMOS-D1-MINI
U$4  |JST-PH_5-PIN_HEADER|JST_B5B-PH_HEADER
U$5  |JST-PH_5-PIN_HEADER|JST_B5B-PH_HEADER
U1   |TSR_1-2450       |TSR-1

## Connector
The AC provides the signals via the CNS connector. It has 5 pins with a pitch of 2 mm (0.079''). It is out of the [PH series from JST](http://www.jst-mfg.com/product/detail_e.php?series=199). You can buy 1:1 cables or cables with interchanged conncections. **I strongly recommend that you use a voltage meter to find out where the +12V is located before you connect the PCB to your AC.** The position of the connector is visible on the following photo of the indoor unit PCB.
![Indoor PCSchematicB](/images/SRK-PCB.jpg)

**Opening of the indoor unit should be done by a qualified professional because faulty handling may cause leakage of water, electric shock or fire!**

## Power Supply
The JST connector provides +12V. The DC-DC converter [TSR 1-2450](https://www.tracopower.com/products/browse-by-category/find/tsr-1/3/) is used to convert it to +5V.

## Signal Connection
The ESP8266 SPI signals SCL (SPI clock), MOSI (Master Out Slave In) and MISO (Master In Slave Out) are connected via a voltage level shifter 5V <-> 3.3V with the AC. Direct connection of the signals without a level shifter could damage your ESP8266!
In a previous version I used a resistor voltage divider, but since the SPI of the AC has high impedance outputs, a voltage level shifter seems to be better. The voltage level shifters are bi-directional. That means the according ESP8266 pins (SCL, MOSI, MISO) could be inputs or outputs. You can use it for different [configurations](/Configurations.md).
