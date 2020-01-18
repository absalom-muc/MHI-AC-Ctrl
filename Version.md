MHI-AC-Ctrl by absalom-muc

**v1.2**

Mode setting corrected
Vanes is now only published if DB0[7]=1 or DB1[7]=1, else last vanes change was via IR-RC (so not visible via SPI)
Options for vanes MQTT values adapted, it is now 1,2,3,4,Swing (before it was 1..5)
Data types of some variables adapted
MQTT status will be published after broker was down
Raw data publishing commented out to reduce the broker load (worst case 20/s)
Remove leading space characters for MQTT room and outdoor temperature
Runtime removed

**v1.1**

Error code (DB4) added
OTA is now also working when waiting for a MQTT connection

**v1.0**

initial