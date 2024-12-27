# MIDI to CV (ESP8266, MCP4728, AppleMIDI Library)
Converting MIDI messages to control voltage via the AppleMIDI  using an ESP8266 and two MCP4728


## Goals
I use VCV Rack as my DAW and I wanted to be able to send "virtual" control voltage (for example from VCV Rack's LFOs or Envelope Generators) to my synthesizers

* connecting VCV Rack and real synths
* also: connecting miRack (https://mirack.app/) and real synths
* using two MCP4728 to have 8 output jacks

## ESP8266
TODO

## Code from the AppleMIDI-Library example
TODO

https://github.com/lathoub/Arduino-AppleMIDI-Library/blob/master/examples/ESP8266_NoteOnOffEverySec_softAP_mDNS/ESP8266_NoteOnOffEverySec_softAP_mDNS.ino

## Code from the ESP8266 WiFi examples (Range Extender)
TODO

https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/examples/RangeExtender-NAPT/RangeExtender-NAPT.ino


## Changing the device ID of the MCP4728
TODO

https://github.com/TrippyLighting/HPRGB2/blob/master/examples/changeDeviceID/changeDeviceID.ino

https://forum.arduino.cc/t/mcp4728-x2-with-and-without-multiplexer-tc9548a-resolved/983859

## How it works
This section describes how all the components work together and how they are connected.

### Connections
* The ESP8266 is powered via the USB input (plugged into a computer or a power bank)
* D1 of the ESP8266 into the CL input of the MCP4728
* D2 of the ESP8266 into the DA input of the MCP4728
* Ground and 3V from the ESP8266 into the G and V input of the MCP4728
* Also: connect the two MCP4728 boards

### VCV Rack Modules
* CV-MIDI CC module https://vcvrack.com/manual/Core#CV-CC
* Gate-MIDI module https://vcvrack.com/manual/Core#CV-Gate

### Apple core xxx
TODO

### rtpMIDI Software
TODO

https://www.tobias-erichsen.de/software/rtpmidi.html


## Findings / Know issues / Limitations
* output voltage is only 3.3v
* D2 is also internal LED
* ...
