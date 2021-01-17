#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <stdio.h>
#include <stdlib.h>
#include "HoneywellManager_HR20_V1.h"

#include <BlynkSimpleEsp8266.h>
#include "auth_secret.h"

// auth key is part of not commited "auth_secret.h"
// char auth[] = "abcdefghijklmnopqrstovwxyz"; 

HoneywellManager_HR20_V1 honeywellManager(&Serial);

WidgetLED errLED(V2);

BLYNK_WRITE(V1)
{
    //TODO: use float instead of int to get values after comma
    float targetTemp = param.asFloat(); // assigning incoming value from pin V1 to a variable
    targetTemp = targetTemp * 10.0; // Add offset

    errLED.off();
    ErrorCode retVal = honeywellManager.SetTargetTemperature(static_cast<int>(targetTemp));
    if(ErrorCode::E_OK != retVal)
    {
        errLED.on();
    }
}

void setup()
{
    //TODO: change everything from Serial to serial!
    Serial.begin(2400, SERIAL_8E1);

    WiFiManager wifiManager;
    wifiManager.setDebugOutput(false);
    wifiManager.autoConnect("NodeMCU-Arduino-PlatformIO");


    Blynk.begin(auth, WiFi.SSID().c_str(), WiFi.psk().c_str());
}

void loop()
{
    Blynk.run();
}


