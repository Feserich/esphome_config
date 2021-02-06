#include <Arduino.h>
#if (PLATFORM_IO == ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <BlynkSimpleEsp8266.h>
#elif (PLATFORM_IO == ESP32)
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#endif
#if (HONEYWELL_MANAGER == OPEN_HR20)
#include "HoneywellManager_OpenHR20.h"
#include <arduino-timer.h>
#elif (HONEYWELL_MANAGER == HR20_V1)
#include "HoneywellManager_HR20_V1.h"
#endif

#include <DNSServer.h>
#include <WiFiManager.h>
#include <stdio.h>
#include <stdlib.h>
#include "auth_secret.h"

// auth key is part of not commited "auth_secret.h"
// char auth[] = "abcdefghijklmnopqrstovwxyz"; 

//---------------------------------------------------
// Global defines
//---------------------------------------------------
// temperature unit is fixed point with offset factor 10 (20 => 2.0°C)
#define DISABLE_FAN_TEMPERATURE_DELTA 10 

// digital output pin where the fan is connected to 
#define FAN_ENABLE_GPIO 5 
#define FAN_OBSERVATION_INTERVALL 100000

//---------------------------------------------------
// Global var section
//---------------------------------------------------
#if (PLATFORM_IO == ESP8266)
HardwareSerial HoneywellSerial(0);
HardwareSerial DbgSerial(2);
#elif (PLATFORM_IO == ESP32)
HardwareSerial DbgSerial(0);
HardwareSerial HoneywellSerial(2);
#endif

#if (HONEYWELL_MANAGER == OPEN_HR20)
HoneywellManager_OpenHR20 honeywellManager(&HoneywellSerial);
Timer<1> FanTimer;
#elif (HONEYWELL_MANAGER == HR20_V1)
HoneywellManager_HR20_V1 honeywellManager(&HoneywellSerial);
#endif
WidgetLED errLED(V0);

//---------------------------------------------------
// function declaration
//---------------------------------------------------

bool observeRoomTemperatureDeltaClb(void *);

//---------------------------------------------------
// Blynk functions 
//---------------------------------------------------

// set desired temperature
BLYNK_WRITE(V1)
{
    //TODO: use float instead of int to get values after comma
    float targetTemp = param.asFloat(); // assigning incoming value from pin V1 to a variable
    targetTemp = targetTemp * 10.0; // Add offset

    errLED.off();
    ErrorCode retVal = honeywellManager.SetDesiredTemperature(static_cast<int>(targetTemp));
    if(ErrorCode::E_OK != retVal)
    {
        errLED.on();
    }
#if (HONEYWELL_MANAGER == OPEN_HR20)
    FanTimer.every(FAN_OBSERVATION_INTERVALL, observeRoomTemperatureDeltaClb);
#endif
}

#if (HONEYWELL_MANAGER == OPEN_HR20)
// get the desired temperature 
BLYNK_READ(V2)
{
    int temperature = 0;
    ErrorCode retVal = honeywellManager.GetDesiredTemperature(temperature);

    if(ErrorCode::E_OK != retVal)
    {
        DbgSerial.println("Error: Reading desired temperature!");
        DbgSerial.println(temperature);
    }
    else
    {
        Blynk.virtualWrite(V2, static_cast<float>(temperature)/10.0);
    }
}

// get the current temperature 
BLYNK_READ(V3)
{
    int temperature = 0;
    ErrorCode retVal = honeywellManager.GetCurrentTemperature(temperature);
    if(ErrorCode::E_OK != retVal)
    {
        DbgSerial.println("Error: Reading current temperature!");
        DbgSerial.println(temperature);
    }
    else
    {
        Blynk.virtualWrite(V3, static_cast<float>(temperature)/10);
    }
}

// get the current battery voltage 
BLYNK_READ(V4)
{
    int voltage = 0;
    ErrorCode retVal = honeywellManager.GetCurrentBatteryVoltage(voltage);
    if(ErrorCode::E_OK != retVal)
    {
        DbgSerial.println("Error: Reading battery voltage!");
        DbgSerial.println(voltage);
    }
    else
    {
        Blynk.virtualWrite(V4, static_cast<float>(voltage)/1000);
    }
}

// get the current valve position 
BLYNK_READ(V5)
{
    int position = 0;
    ErrorCode retVal = honeywellManager.GetValvePosition(position);
    if(ErrorCode::E_OK != retVal)
    {
        DbgSerial.println("Error: Reading valve position!");
        DbgSerial.println(position);
    }
    else
    {
        Blynk.virtualWrite(V5, position);
    }
}

// get mode
BLYNK_READ(V6)
{
    Mode mode;
    ErrorCode retVal = honeywellManager.GetMode(mode);
    if(ErrorCode::E_OK != retVal)
    {
        DbgSerial.println("Error: Get Mode!");
    }
    else
    {
        if(mode == Mode::E_MANUAL)
        {
            Blynk.virtualWrite(V6, 0);
        }
        else if(mode == Mode::E_AUTOMATIC)
        {
            Blynk.virtualWrite(V6, 1);
        }
        else
        {
            Blynk.virtualWrite(V6, 2);
        }
    }
}

// set mode
BLYNK_WRITE(V7)
{
    int modeVal = param.asInt();

    if(modeVal == 0)
    {
        honeywellManager.SetMode(Mode::E_MANUAL);
    }
    else if(modeVal == 1)
    {
        honeywellManager.SetMode(Mode::E_AUTOMATIC);
    }
    else
    {
        DbgSerial.println("Error: Set invalid Mode!");
        errLED.on();
    }
}
#endif

//---------------------------------------------------
// user functions 
//---------------------------------------------------

#if (HONEYWELL_MANAGER == OPEN_HR20)

/**
 * @brief This callback function observes the delta between desired and current temperature.
 *        If the current temperature is bellow the desired temperature limit, a fan will be enabled to heat up the room faster. 
 */
bool observeRoomTemperatureDeltaClb(void *)
{
    // if return value is true, the task gets repeated
    bool repeatTask = true;
    int currentTemp = 0;
    int desiredTemp = 0;

    ErrorCode retVal = honeywellManager.GetCurrentTemperature(currentTemp);
    delay(100);
    retVal = honeywellManager.GetDesiredTemperature(desiredTemp);
    if(ErrorCode::E_OK != retVal)
    {
        DbgSerial.println("Error: Reading current temperature (room observation)!");
        DbgSerial.println(desiredTemp);
        DbgSerial.println(currentTemp);
    }
    else
    {
        if((currentTemp + DISABLE_FAN_TEMPERATURE_DELTA) < desiredTemp)
        {
            // enable fan
            digitalWrite(FAN_ENABLE_GPIO, HIGH);
            repeatTask = true;
            DbgSerial.println("Fan enabled");
        }
        else
        {
            // disable fan
            digitalWrite(FAN_ENABLE_GPIO, LOW);
            repeatTask = false;
            DbgSerial.println("Fan disabled");
        }
    }

    return repeatTask;
}
#endif


//---------------------------------------------------
// Arduino setup() & loop()
//---------------------------------------------------
void setup()
{  
#if (PLATFORM_IO == ESP8266)
    HoneywellSerial.begin(2400, SERIAL_8E1);
    DbgSerial.begin(9600);
#elif (PLATFORM_IO == ESP32)
    HoneywellSerial.begin(9600, SERIAL_8N1);
    DbgSerial.begin(9600);
#endif

    WiFiManager wifiManager;
    wifiManager.setDebugOutput(false);
    wifiManager.autoConnect("HoneywellManager_AP");

#if (HONEYWELL_MANAGER == OPEN_HR20)
    pinMode(FAN_ENABLE_GPIO, OUTPUT);
    digitalWrite(FAN_ENABLE_GPIO, LOW);
#endif

    Blynk.begin(auth, WiFi.SSID().c_str(), WiFi.psk().c_str());
}

void loop()
{
    Blynk.run();
#if (HONEYWELL_MANAGER == OPEN_HR20)
    FanTimer.tick();
#endif
}



