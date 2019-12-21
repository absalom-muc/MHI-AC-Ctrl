# Configurations
With the current hardware circuit different configurations are supported. The PCB assembly for all configurations is identical:

## Configuration 1
For normal use one PCB is connected to the AC:
![standard configuration](/images/MHI-AC-Ctrl-fig.png)

## Configuration 2
You could add a second PCB to monitor the SPI traffic:
![standard configuration](/images/MHI-AC-Ctrl-Spy-fig.png)

## Configuration 3
To investigate the SPI protocol the following configuration is used:
![standard configuration](/images/MHI-AC-RC-Spy-fig.png)

## Configuration 4
Or you replace the AC by an AC-emulator:
![standard configuration](/images/MHI-AC-Emu-RC-Spy-fig.png)
For this configuration you need an external 12V power supply.

Of course different SW is needed for configurations 2 to 4!
