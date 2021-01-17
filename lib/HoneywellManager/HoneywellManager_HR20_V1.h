
#ifndef HONEYWELL_MANAGER_HR20_V1_H
#define HONEYWELL_MANAGER_HR20_V1_H

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

        ErrorCode SetTargetTemperature(int temperature);
        ErrorCode EnableAutomaticMode(void);
        ErrorCode EnableManualMode(void);

    private:
        HardwareSerial* serialPtr;
        void waitForAnyResponse(void);
        ErrorCode sendToHoneywell(char* command);
        ErrorCode checkHoneywellReponse(char* expectedResponse);
};

#endif