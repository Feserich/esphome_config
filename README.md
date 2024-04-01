# ESPHome config
This repository contains various [ESPHome](https://esphome.io/) configurations, 
to extend my [Home Assistant](https://www.home-assistant.io/) instance with custom sensors and controllers. 
The goal was to group several functionalities on one controller (save energy) and 
enable old/ existing hardware in my houshold with smart IoT funtionalities (avoid electronic waste). 


## GoveeLife Thermo-Hygrometer 2s E-Ink BLE Proxy
This [ESPHome configuration](./config/govee_h5105_proxy/) acts as a bluetooth sensor data proxy,
to make the data of Thermo-Hygrometer *Govee H5105* visible in a HomeAssistant sensor entity. 
The configuration will listen to the Bluetooth Low-Energy advertise packets of the sensor,
parse the sensor data out of that messages and provide the sensor values to HomeAssistant. 

The [pc.yaml](./config/govee_h5105_proxy/pc.yaml) config was made for a ESP32-C3 with a RISC-V CPU, 
but the it could also be adapted for any other ESP32 controller with Bluetooth. 
The config also contains an switch component to remote control an attached relay. 


## Honeywell HR20 Controller
This is a implementation to remote controll a 
[Honeywell HR-20 radiator regulator](./config/honeywell_HR20_controller/) in a Home Assistant instance. 
To control the radiator over UART, an ESP Microcontroller is used. 
The communication between Home Assistant and the Microcontroller is done with the ESPHome framework. 
Therefore an [ESPHome Custom Climate Component](https://esphome.io/components/climate/custom.html) was created, 
which will send/ receive the commands to/ from the Honeywell radiator regulator. 

This repository contains one implementation for the Honeywell HR-20 Hardware version 1 and 
another one for Hardware version 2 with [OpenHR20](https://github.com/OpenHR20/OpenHR20) firmware.
The implementation was tested with the NodeMCU v3 (ESP8266) and ESP32 as IoT getway to remote control the heater. 

The UART protocoll for Hardware version 1 was reengineered, to manipulate the memory 
over a programming/debugging UART interface ([protocol](http://symlink.dk/projects/rondo485/).
The protocoll for the OpenHR20 firmware can be found 
[here](https://www.mikrocontroller.net/articles/Heizungssteuerung_mit_Honeywell_HR20#UART_Protocoll)

**Here is a wiring example:**
<img src="./doc/res/ESP32_Wiring_bb.png" width="420"/>


### How to Use
1. Follow the instructions from [ESPHome](https://esphome.io/guides/getting_started_command_line.html) 
    how to setup the build & deploy toolchain on your host. 
   - Initial flashing must be done over wired USB connection. Afterwards it can be done OTA. 
2. Deploy the C++ header files and the ESPHome YAML from [honeywell_HR20_controller](./config/honeywell_HR20_controller) 
    to the shared config directory of the toolchain. 
    - When you run the ESPHome docker container, you also specify where this shared config directory shall be mounted. 
3. Open the ESPHome dashboard (usually http://localhost:6052/) and install the config from this project. 
4. Install ESPHome in your Home Assistant instance. 
Follow [this guide](https://esphome.io/guides/getting_started_hassio.html). 
5. Add the related ESPHome entities to your Home Assistant Dashboard. 


### Deprecated Version 
The inital version of this repository (git tag V1.0) was implemented to work with
[Legacy blynk.io](https://docs.blynk.io/en/blynk-1.0-and-2.0-comparison/migrate-from-1.0-to-2.0). 
With minor adaptations it should also work with the latest blynk version. 
However the blynk implementation of this project is discontinued and replaced by Home Assistant. 