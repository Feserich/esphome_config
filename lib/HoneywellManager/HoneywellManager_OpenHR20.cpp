#include "HoneywellManager_OpenHR20.h"
#include <string.h>
#include <cstdlib>

//Constants
constexpr size_t COMMAND_CHAR_COUNT = 9;                // 8 char + null terminator 
constexpr unsigned long RESPONSE_TIMEOUT = 1000;
constexpr int SEND_RETRIES = 3;
constexpr int TEMPERATURE_MIN = 75;
constexpr int TEMPERATURE_MAX = 280;

HoneywellManager_OpenHR20::HoneywellManager_OpenHR20(HardwareSerial* SerialPtr)
{
    serialPtr = SerialPtr;
}

ErrorCode HoneywellManager_OpenHR20::SetDesiredTemperature(int temperature)
{
    ErrorCode retVal = ErrorCode::E_NOT_OK;
    char buffer[10];

    if((TEMPERATURE_MIN <= temperature) && (TEMPERATURE_MAX >= temperature))
    {
        // only 0.5°C steps are allowed --> always round down 
        temperature = temperature - (temperature % 5);

        int n = sprintf(buffer,"\nA%x\n", temperature/5);
        if(n > 0)
        {
            serialPtr->print(buffer);
            retVal = ErrorCode::E_OK;
        }
    }

    return retVal;
}

void HoneywellManager_OpenHR20::SetMode(Mode mode)
{
    if(mode == Mode::E_MANUAL)
    {
        serialPtr->print("\nM00\n");
    }
    else if(mode == Mode::E_AUTOMATIC)
    {
        serialPtr->print("\nM01\n");
    }
    else
    {
        /* invalid */
    }
}

ErrorCode HoneywellManager_OpenHR20::GetMode(Mode& mode)
{
    char buffer[10];
    ErrorCode retVal = extractStatusInformation("D: ", 21u, &buffer[0], 1u);

    if(retVal == ErrorCode::E_OK)
    {
        if(buffer[0] == 'A')
        {
            mode = Mode::E_AUTOMATIC;
        }
        else if(buffer[0] == 'M')
        {
            mode = Mode::E_MANUAL;
        }
        else
        {
            mode = Mode::E_INVALID;
        }
    }
    else
    {
        mode = Mode::E_INVALID;
    }
    
    return retVal;
}

ErrorCode HoneywellManager_OpenHR20::GetDesiredTemperature(int& temperature)
{
    char buffer[16];
    ErrorCode retVal = extractStatusInformation("S: ", 0u, &buffer[0], 3u);

    if(retVal == ErrorCode::E_OK)
    {
        temperature = std::atoi(&buffer[0]);
    }
    
    return retVal;
}

ErrorCode HoneywellManager_OpenHR20::GetCurrentTemperature(int& temperature)
{
    char buffer[16];
    ErrorCode retVal = extractStatusInformation("I: ", 0u, &buffer[0], 3u);

    if(retVal == ErrorCode::E_OK)
    {
        temperature = std::atoi(&buffer[0]);
    }
    
    return retVal;
}

ErrorCode HoneywellManager_OpenHR20::GetCurrentBatteryVoltage(int& voltage)
{
    char buffer[16];
    ErrorCode retVal = extractStatusInformation("B: ", 0u, &buffer[0], 4u);

    if(retVal == ErrorCode::E_OK)
    {
        voltage = std::atoi(&buffer[0]);
    }
    
    return retVal;
}

ErrorCode HoneywellManager_OpenHR20::GetValvePosition(int& valvePosition)
{
    char buffer[16];
    ErrorCode retVal = extractStatusInformation("V: ", 0u, &buffer[0], 2u);

    if(retVal == ErrorCode::E_OK)
    {
        valvePosition = std::atoi(&buffer[0]);
    }
    
    return retVal;
}

/*
// private functions 
*/

ErrorCode HoneywellManager_OpenHR20::extractStatusInformation(const char* startTerminatorStr, const uint8_t startByte, char* dataBufferStr, const uint8_t dataLength)
{
    ErrorCode retVal = ErrorCode::E_NOT_OK;

    flushInputBuffer();

    // Request status message
    serialPtr->print("D\n");

    serialPtr->setTimeout(RESPONSE_TIMEOUT);

    if(serialPtr->find(startTerminatorStr))
    {
        retVal = ErrorCode::E_OK;

        for(uint8_t i = 0; i < startByte; i++)
        {
            // discard all bytes till start byte of the data of interest
            serialPtr->read();
        }

        serialPtr->readBytes(dataBufferStr, dataLength);
        dataBufferStr[dataLength] = '\0';
    }

    // clean up remaining bytes 
    flushInputBuffer();

    return retVal;   
}



void HoneywellManager_OpenHR20::flushInputBuffer()
{
    serialPtr->print("\n");

    delay(100);
    while(serialPtr->available())
    {
        serialPtr->read();
    }
}

