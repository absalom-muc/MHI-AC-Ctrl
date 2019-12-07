# Hardware

## Schematic
![Indoor PCB](/images/MHI-AC-Ctrl_Schematic.png)


## Connector
The AC provides the signals via the CNS connector. It has 5 pins with a pitch of 2 mm (0.079''). It is out of the [PH series from JST](http://www.jst-mfg.com/product/detail_e.php?series=199). The position of the connector is visible on the following photo of the indoor unit PCB.
![Indoor PCSchematicB](/images/SRK-PCB.jpg)

**Opening of the indoor unit should be done by a qualified professional because faulty handling may cause leakage of water, electric shock or fire!**

## Power Supply
The JST connector provides +12V. The DC-DC converter [TSR 1-2450](https://www.tracopower.com/products/browse-by-category/find/tsr-1/3/) is used to convert it to +5V.

## Signal Connection
The ESP8266 SPI signals SCL (SPI clock), MOSI (Master Out Slave In) and MISO (Master In Slave Out) are connected via a voltage level shifter 5V <-> 3.3V with the AC. Direct connection of the signals without a level shifter could damage your ESP8266!
In a previous version I used a resistor voltage divider, but since the SPI of the AC has high impedance outputs, a voltage level shifter seems to be better.
