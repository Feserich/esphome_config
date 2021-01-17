# Blynk Honeywell Controller
This is a implementation to remote controll a Honeywell HR-20 V1 radiator regulator with a [Blynk](http://docs.blynk.cc/) app. 
The implementation was tested with the NodeMCU v3 (ESP8266) as IoT getway to remote control the heater. 
It should be also compatible with other arduino based µC (HardwareSerial class from Arduino was used). 
The project is based on a [PlatformIO](https://docs.platformio.org/en/latest/what-is-platformio.html) project, so it should be easy to setup toolchain/ libs and extend it. 
To control the Honeywell HR-20 memory manipulation over a programming/debugging UART interface was used ([protocol](http://symlink.dk/projects/rondo485/).

**Here is a wiring example:**
<img src="https://gitlab.com/Feserich/ParticleRemote-PhotonCode/-/raw/master/Wiring%20Example/Photon%20Wiring_bb.png" width="420"/>


