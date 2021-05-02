# Blynk Honeywell Controller
This is a implementation to remote controll a Honeywell HR-20 radiator regulator with a [Blynk](http://docs.blynk.cc/) app. 
This repository contains one library for the Honeywell HR-20 Hardware version 1 and another one for Hardware version 2 with [OpenHR20](https://github.com/OpenHR20/OpenHR20) firmware.
The implementation was tested with the NodeMCU v3 (ESP8266) and ESP32 as IoT getway to remote control the heater. 
It should be also compatible with other arduino based µC (HardwareSerial class from Arduino was used). 
The project is based on a [PlatformIO](https://docs.platformio.org/en/latest/what-is-platformio.html) project, so it should be easy to setup toolchain/ libs and extend it. 

The UART protocoll for Hardware version 1 was reengineered, to manipulate the memory over a programming/debugging UART interface ([protocol](http://symlink.dk/projects/rondo485/).
The protocoll for the OpenHR20 firmware can be found [here](https://www.mikrocontroller.net/articles/Heizungssteuerung_mit_Honeywell_HR20#UART_Protocoll)

**Here is a wiring example:**
<img src="https://raw.githubusercontent.com/Feserich/ParticleRemote-PhotonCode/master/Wiring%20Example/Photon%20Wiring_bb.png" width="420"/>


