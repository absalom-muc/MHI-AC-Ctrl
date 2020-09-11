# Openhab Integration

This is an example config for using Openhab 2.5+ to control your AC.

You should enable

    #define POWERON_WHEN_CHANGING_MODE true
    
in support.h when working with home automation software.

## Configs

### aircondition.things

    Bridge mqtt:broker:mosquitto [ host="localhost", secure=false, port=1883, clientID="openHAB25" ] {

        Thing topic Aircondition "Aircondition" @ "Livingroom" {
            Channels:
            Type string : status "Status" [ stateTopic="Aircon/Mode", commandTopic="Aircon/set/Mode" ]
            Type string : power "Power" [ stateTopic="Aircon/Power" ]
            Type string : fanspeed "Fanspeed" [ stateTopic="Aircon/Fan", commandTopic="Aircon/set/Fan" ]
            Type string : vanes "Vanes" [ stateTopic="Aircon/Vanes", commandTopic="Aircon/set/Vanes" ]
            Type number : setpoint "Setpoint" [ stateTopic="Aircon/Tsetpoint", commandTopic="Aircon/set/Tsetpoint" ]
            Type number : temperature "Temperature" [ stateTopic="Aircon/Troom" ]
            Type number : signal "RSSI" [ stateTopic="Aircon/RSSI" ]
            Type number : sck "SCK" [ stateTopic="Aircon/fSCK" ]
            Type number : mosi "MOSI" [ stateTopic="Aircon/fMOSI" ]
            Type number : miso "MISO" [ stateTopic="Aircon/fMISO" ]
            Type number : hours "IU Run hours" [ stateTopic="Aircon/TOTAL-IU-RUN" ]
            Type number : outdoor "Outdoor Temperature" [ stateTopic="Aircon/OpData/OUTDOOR" ]
            Type number : fanspeed_outdoor "Outdoor Fanspeed" [ stateTopic="Aircon/OpData/OU-FANSPEED" ]
            Type number : ampere "Power" [ stateTopic="Aircon/OpData/CT" ]
        }
        
    }

### aircondition.items

    Group  Aircondition "Indoor" { alexa="Endpoint.Thermostat" }
    Switch Aircondition_Power "Power [%s]" (Aircondition) { channel="mqtt:topic:mosquitto:Aircondition:power", alexa="PowerController.powerState" [nonControllable=true] }
    String Aircondition_Status "Status [%s]" (Aircondition) { channel="mqtt:topic:mosquitto:Aircondition:status", alexa="ThermostatController.thermostatMode" [OFF="Off",HEAT="Heat",COOL="Cool"] }
    String Aircondition_Fanspeed "Fanspeed [%s]" { channel="mqtt:topic:mosquitto:Aircondition:fanspeed" }
    String Aircondition_Vanes "Vanes [%s]" { channel="mqtt:topic:mosquitto:Aircondition:vanes" }
    Number Aircondition_Setpoint "Temperature [%s °C]" (Aircondition) { channel="mqtt:topic:mosquitto:Aircondition:setpoint", alexa="ThermostatController.targetSetpoint" }
    Number Aircondition_Temperature "Act. Temperature [%s °C]" (Aircondition) { channel="mqtt:topic:mosquitto:Aircondition:temperature", alexa="TemperatureSensor.temperature" }
    Number Aircondition_Signal "RSSI [%s]" { channel="mqtt:topic:mosquitto:Aircondition:signal" }
    Number Aircondition_Hours "Total run hours [%d]" { channel="mqtt:topic:mosquitto:Aircondition:hours" }
    
    Number Aircondition_Outdoor_Temperature "Temperature [%s °C]" { channel="mqtt:topic:mosquitto:Aircondition:outdoor" }
    Number Aircondition_Outdoor_Fanspeed "Fanspeed [%d]" { channel="mqtt:topic:mosquitto:Aircondition:fanspeed_outdoor" }
    String Aircondition_Outdoor_Ampere "Ampere [%d A]" { channel="mqtt:topic:mosquitto:Aircondition:ampere" }

### aircondition.sitemap

    sitemap aircondition label="Aircondition"
    {
        Frame label="Indoor" icon=climate {
            Selection item=Aircondition_Status      icon=climate    mappings=["Off"="Aus", "Dry"="Trocknen", "Cool"="Kühlen", "Heat"="Heizen", "Fan"="Lüfter", "On"="An"]
            Setpoint item=Aircondition_Fanspeed     icon=fan        minValue=1      maxValue=4  step=1
            Selection item=Aircondition_Vanes       icon=smoke      mappings=[1="Pos 1 (hoch)", 2="Pos 2", 3="Pos 3", 4="Pos 4 (runter)", "Swing"="Swing"]
            Setpoint item=Aircondition_Setpoint     icon=heating    minValue=18     maxValue=30 step=1
            Default item=Aircondition_Temperature   icon=temperature
            Default item=Aircondition_Hours
        }
        Frame label="Outdoor" {
            Default item=Aircondition_Outdoor_Temperature
            Default item=Aircondition_Outdoor_Fanspeed
            Default item=Aircondition_Outdoor_Ampere
        }
    }
    
### aircondition.rules
    
    # This rule will map "Alexa, turn on Aircondition" into a concrete Action based on roomtemperature
    rule "Turn on Aircondition"
    when
        Item Aircondition_Power received command OFF
    then
        Aircondition_Status.sendCommand('Off')
    end
    
    # This rule will power off the AC on "Alexa, turn off Aircondition"
    rule "Turn off Aircondition"
    when
        Item Aircondition_Power received command ON
    then
        if (Aircondition_Temperature.state >= Aircondition_Setpoint.state) {
            Aircondition_Status.sendCommand('Cool')
            return;
        }
    
        Aircondition_Status.sendCommand('Heat')
    end
