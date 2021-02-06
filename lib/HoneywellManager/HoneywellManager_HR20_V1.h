#ifndef HONEYWELL_MANAGER_HR20_V1_H
#define HONEYWELL_MANAGER_HR20_V1_H

/**
 * @file HoneywellManager_HR20_V1.h
 *
 * @brief This class can be used to controll the radiator thermostat "Honeywell HR20" Hardware Revision V1. 
 *        It can be used in an IOT µC to controll your heater over the Internet. 
 *        All functions are useing the Serial class from Arduino to send the commands to the thermostat over UART. 
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <Arduino.h>

enum class ErrorCode
{
    E_OK,
    E_NOT_OK,
    E_READ_BUF_OVERFLOW,
    E_RESPONSE_TIMEOUT,
    E_RESPONSE_WRONG
};

class HoneywellManager_HR20_V1
{
    public:
        HoneywellManager_HR20_V1(HardwareSerial* SerialPtr);


        ErrorCode SetDesiredTemperature(int temperature);
        ErrorCode EnableAutomaticMode(void);
        ErrorCode EnableManualMode(void);

    private:
        HardwareSerial* serialPtr;
        void waitForAnyResponse(void);
        ErrorCode sendToHoneywell(char* command);
        ErrorCode checkHoneywellReponse(char* expectedResponse);
};

#endif