#include "HoneywellManager_HR20_V1.h"
#include <string.h>

//Constants
constexpr size_t COMMAND_CHAR_COUNT = 9;                // 8 char + null terminator 
constexpr unsigned long RESPONSE_TIMEOUT = 1000;
constexpr int SEND_RETRIES = 3;
constexpr int TEMPERATURE_MIN = 75;
constexpr int TEMPERATURE_MAX = 280;

HoneywellManager_HR20_V1::HoneywellManager_HR20_V1(HardwareSerial* SerialPtr)
{
    serialPtr = SerialPtr;
}

ErrorCode HoneywellManager_HR20_V1::SetDesiredTemperature(int temperature)
{
    //TODO: range check of targetTemp
    ErrorCode retVal = ErrorCode::E_NOT_OK;

    if(TEMPERATURE_MIN <= temperature && TEMPERATURE_MAX >= temperature)
    {
        int targetTempOffset = temperature - 60;                        //Offset is 60 (=6° C), Unit is 1/10° C
        char writeDisplayRAM[COMMAND_CHAR_COUNT] = "W13600";                              //0x136 is the memory location of the target Temperature on the Display
        char writeMotorRam[COMMAND_CHAR_COUNT] = "W20C10";                                //0x20c is the memory location of the target temperatur for the DC Motor

        char tmp[3]; /* one bytes of hex = 2 characters, plus NULL terminator */

        sprintf(&tmp[0], "%02X", targetTempOffset);
        strcat(writeDisplayRAM, tmp);
        strcat(writeMotorRam, tmp);

        ErrorCode displayResponse = sendToHoneywell(writeDisplayRAM);
        ErrorCode motorResponse = sendToHoneywell(writeMotorRam); 

        if(ErrorCode::E_OK == displayResponse)
        {
            if(ErrorCode::E_OK == motorResponse)
            {
                retVal = ErrorCode::E_OK;
            }
            else
            {
                retVal = motorResponse;
            }
        }
        else
        {
            retVal = displayResponse;
        }
    }

    return retVal;
}

ErrorCode HoneywellManager_HR20_V1::EnableAutomaticMode(void)
{
    char command[COMMAND_CHAR_COUNT] = "W12B0010";
    return sendToHoneywell(command);
}

ErrorCode HoneywellManager_HR20_V1::EnableManualMode(void)
{
    char command[COMMAND_CHAR_COUNT] = "W12B0000";
    return sendToHoneywell(command);
}


/*
// private functions 
*/

void HoneywellManager_HR20_V1::waitForAnyResponse(void)
{
    //send empty commands till Honeywell is responding.
    for(int i = 0; i < 20; i++)
    {
		serialPtr->println("K");
	    delay(50);

        if(serialPtr->available())
        {
            break;
        }
    }

    while (serialPtr->available()) 
    {
        //consume response from empty commands
        serialPtr->read();																		           	
    }
}

ErrorCode HoneywellManager_HR20_V1::sendToHoneywell(char* command)
{
    ErrorCode retVal = ErrorCode::E_NOT_OK;
    char expectedResponse[COMMAND_CHAR_COUNT];
    strcpy (expectedResponse, command);
    expectedResponse[0] = 'M';                        

    if(0 == strcmp(expectedResponse, "M20C100F"))
    {
        //change character 7 from 'F' to '0' only for motor command if honeywell is set to OFF
        expectedResponse[7] = '0';           
    }

    waitForAnyResponse();

    for(int i = 0; i < SEND_RETRIES; i++)
    {
        //send command to Honeywell
        serialPtr->println(command);
        retVal = checkHoneywellReponse(expectedResponse);

        if(ErrorCode::E_OK == retVal)
        {
            break;
        }
    }

    return retVal;
}

ErrorCode HoneywellManager_HR20_V1::checkHoneywellReponse(char* expectedResponse)
{
    ErrorCode retVal = ErrorCode::E_NOT_OK;

    serialPtr->setTimeout(RESPONSE_TIMEOUT);

    if(serialPtr->find(expectedResponse, strlen(expectedResponse)))
    {
        retVal = ErrorCode::E_OK;
    }
    else
    {
        retVal = ErrorCode::E_RESPONSE_WRONG;
    }

    return retVal;
}