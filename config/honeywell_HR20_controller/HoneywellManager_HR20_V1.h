#ifndef HONEYWELL_MANAGER_HR20_V1_H
#define HONEYWELL_MANAGER_HR20_V1_H

/**
 * @file HoneywellManager_HR20_V1.h
 *
 * @brief This class can be used to controll the radiator thermostat "Honeywell HR20" Hardware
 * Revision V1. It can be used in an IOT µC to controll your heater over the Internet. All functions
 * are useing the UART class from ESPHome to send the commands to the thermostat.
 *
 */

#include "IHoneywellManager.h"
#include "esphome.h"
#include <cstddef>
#include <cstdint>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * constants
 */

/// @brief 8 char + null terminator
constexpr size_t WRITE_COMMAND_CHAR_COUNT{ 9 };

/// @brief 4 char + null terminator
constexpr size_t READ_COMMAND_CHAR_COUNT{ 5 };

/*
 * class definition
 */
class HoneywellManager_HR20_V1 : public IHoneywellManager
{
public:
    /**
     * @brief C'tor to create an instance with the pointer to the ESPHome UartComponent
     * @param parent_component Pointer to the parent UART component
     */
    HoneywellManager_HR20_V1(UARTComponent* parent_component);

    /**
     * @brief Set the desired temperature for the radiator thermostat.
     *
     * @param temperature Temperature value in celsius and with factor 10 offset (e.g.: 225 => 22.5°C)
     * @return Error code, see enum class definition.
     */
    ErrorCode SetDesiredTemperature(int temperature) override;

    /**
     * @brief Get the desired temperature for the radiator thermostat.
     *
     * @param temperature Temperature value in celsius and with factor 10 offset (fixed point; e.g.: 225 => 22.5°C)
     * @return Error code, see enum class definition.
     */
    ErrorCode GetDesiredTemperature(int& temperature) override;

    /**
     * @brief Get manual or automatic mode. In automatic mode the stored heating programm will be used.
     *
     * @param mode manual or automatic mode.
     */
    ErrorCode GetMode(Mode& mode) override;

    /**
     * @brief Set manual or automatic mode. In automatic mode the stored heating programm will be used.
     *
     * @param mode manual or automatic mode.
     */
    ErrorCode SetMode(Mode mode) override;

private:
    /**
     * @brief Send some dummy command and wait till Honeywell replied with a response command.
     */
    void waitForAnyResponse(void);

    /**
     * @brief Send an commmand string to the honeywell controler over UART.
     * @param command C-String of the command to send.
     */
    ErrorCode sendToHoneywell(char* command);

    ErrorCode readFromHoneywell(char* command, char* readValue);

    /**
     * @brief Check if the Honeywell controller replied with the expected response message within a specified timeout
     * @param expectedResponse C-String of the expected response command
     */
    ErrorCode checkHoneywellReponse(char* expectedResponse);

    /**
     * @brief Find a given substring in the received UART messages.
     * @param expectedResponse C-String of the expected substring
     */
    ErrorCode findResponseString(char* expectedResponse);

    /**
     * @brief EspHome UART Device which is used for serial communication with the Honeywell controller.
     */
    UARTDevice serial_device_;
};

/*
 * public functions
 */

HoneywellManager_HR20_V1::HoneywellManager_HR20_V1(UARTComponent* parent_component)
    : serial_device_(parent_component)
{
}

ErrorCode HoneywellManager_HR20_V1::SetDesiredTemperature(int temperature)
{
    // TODO: range check of targetTemp
    ErrorCode retVal              = ErrorCode::E_NOT_OK;
    constexpr int TEMPERATURE_MIN = 75;
    constexpr int TEMPERATURE_MAX = 280;

    if (TEMPERATURE_MIN <= temperature && TEMPERATURE_MAX >= temperature)
    {
        int targetTempOffset                           = temperature - 60; // Offset is 60 (=6° C), Unit is 1/10° C
        char writeDisplayRAM[WRITE_COMMAND_CHAR_COUNT] = "W13600"; // 0x136 is the memory location of the target Temperature on the Display
        char writeMotorRam[WRITE_COMMAND_CHAR_COUNT]   = "W20C10"; // 0x20c is the memory location of the target temperatur for the DC Motor

        char tmp[3]; /* one bytes of hex = 2 characters, plus NULL terminator */

        sprintf(&tmp[0], "%02X", targetTempOffset);
        strcat(writeDisplayRAM, tmp);
        strcat(writeMotorRam, tmp);

        ErrorCode displayResponse = sendToHoneywell(writeDisplayRAM);
        ErrorCode motorResponse   = sendToHoneywell(writeMotorRam);

        if (ErrorCode::E_OK == displayResponse)
        {
            if (ErrorCode::E_OK == motorResponse)
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

ErrorCode HoneywellManager_HR20_V1::GetDesiredTemperature(int& temperature)
{
    ErrorCode retVal = ErrorCode::E_NOT_OK;

    char readDesiredTemperatureCmd[READ_COMMAND_CHAR_COUNT] = "R136";
    char readValue[WRITE_COMMAND_CHAR_COUNT];
    char* pEnd;

    retVal = readFromHoneywell(readDesiredTemperatureCmd, readValue);

    if (ErrorCode::E_OK == retVal)
    {
        char* pEnd;
        temperature = static_cast<int>(strtol(readValue, &pEnd, 16)) + 60.0;
    }

    return retVal;
}

ErrorCode HoneywellManager_HR20_V1::SetMode(Mode mode)
{
    ErrorCode ret_val{ ErrorCode::E_OK };

    if (mode == Mode::E_MANUAL)
    {
        char command[WRITE_COMMAND_CHAR_COUNT] = "W12B0000";
        ret_val                                = sendToHoneywell(command);
    }
    else if (mode == Mode::E_AUTOMATIC)
    {
        char command[WRITE_COMMAND_CHAR_COUNT] = "W12B0010";
        ret_val                                = sendToHoneywell(command);
    }
    else
    {
        /* invalid */
        ret_val = ErrorCode::E_NOT_OK;
    }

    return ret_val;
}

ErrorCode HoneywellManager_HR20_V1::GetMode(Mode& mode)
{
    ErrorCode retVal = ErrorCode::E_NOT_OK;

    constexpr uint8_t AUTOMATIC_MANUAL_MODE_BIT{ 2 };
    char readDesiredTemperatureCmd[READ_COMMAND_CHAR_COUNT] = "R12B";
    char readValue[WRITE_COMMAND_CHAR_COUNT]                = { 0 };
    char* pEnd;

    retVal = readFromHoneywell(readDesiredTemperatureCmd, readValue);

    if (ErrorCode::E_OK == retVal)
    {
        if (readValue[AUTOMATIC_MANUAL_MODE_BIT] == '1')
        {
            mode = Mode::E_AUTOMATIC;
        }
        else if (readValue[AUTOMATIC_MANUAL_MODE_BIT] == '0')
        {
            mode = Mode::E_MANUAL;
        }
        else
        {
            mode = Mode::E_INVALID;
        }
    }

    return retVal;
}

/*
 * private functions
 */

void HoneywellManager_HR20_V1::waitForAnyResponse(void)
{
    // send empty commands till Honeywell is responding.
    for (int i = 0; i < 20; i++)
    {
        serial_device_.write_str("K\r\n");
        serial_device_.flush();
        delay(50);

        if (serial_device_.available())
        {
            break;
        }
    }

    while (serial_device_.available())
    {
        // consume response from empty commands
        serial_device_.read();
    }
}

ErrorCode HoneywellManager_HR20_V1::sendToHoneywell(char* command)
{
    ErrorCode retVal = ErrorCode::E_NOT_OK;
    char expectedResponse[WRITE_COMMAND_CHAR_COUNT];
    constexpr int SEND_RETRIES = 3;

    strcpy(expectedResponse, command);
    expectedResponse[0] = 'M';

    if (0 == strcmp(expectedResponse, "M20C100F"))
    {
        // change character 7 from 'F' to '0' only for motor command if honeywell is set to OFF
        expectedResponse[7] = '0';
    }

    waitForAnyResponse();

    for (int i = 0; i < SEND_RETRIES; i++)
    {
        // send command to Honeywell
        serial_device_.write_str(command);
        serial_device_.write('\r');
        serial_device_.write('\n');
        serial_device_.flush();

        retVal = checkHoneywellReponse(expectedResponse);

        if (ErrorCode::E_OK == retVal)
        {
            break;
        }
    }

    return retVal;
}

ErrorCode HoneywellManager_HR20_V1::readFromHoneywell(char* command, char* readValue)
{
    ErrorCode retVal = ErrorCode::E_NOT_OK;
    char expectedResponse[READ_COMMAND_CHAR_COUNT];
    constexpr int SEND_RETRIES = 3;
    uint8_t readByte           = 0;

    strcpy(expectedResponse, command);
    expectedResponse[0] = 'M';

    waitForAnyResponse();

    for (int i = 0; i < SEND_RETRIES; i++)
    {
        // send command to Honeywell
        serial_device_.write_str(command);
        serial_device_.write('\r');
        serial_device_.write('\n');
        serial_device_.flush();

        retVal = checkHoneywellReponse(expectedResponse);

        if (ErrorCode::E_OK == retVal)
        {
            for (size_t i{ 0 }; i < READ_COMMAND_CHAR_COUNT; i++)
            {
                if (serial_device_.read_byte(&readByte))
                {
                    readValue[i] = static_cast<char>(readByte);
                }
                else
                {
                    readValue[i] = '\0';
                    break;
                }
            }

            break;
        }
    }

    return retVal;
}

ErrorCode HoneywellManager_HR20_V1::checkHoneywellReponse(char* expectedResponse)
{
    ErrorCode retVal = ErrorCode::E_RESPONSE_TIMEOUT;
    constexpr uint32_t MAX_RESPONSE_LOOP_CYCLES{ 100 };

    for (uint32_t i{ 0 }; i < MAX_RESPONSE_LOOP_CYCLES; ++i)
    {
        retVal = findResponseString(expectedResponse);

        // if the string was found, break the loop. Otherwise sleep and retry it.
        if (retVal == ErrorCode::E_OK)
        {
            break;
        }
        else
        {
            delay(10);
        }
    }

    return retVal;
}

ErrorCode HoneywellManager_HR20_V1::findResponseString(char* expectedResponse)
{
    ErrorCode retVal             = ErrorCode::E_RESPONSE_TIMEOUT;
    uint8_t receivedChar         = 0;
    size_t expectedResponseIndex = 0;

    if (serial_device_.available())
    {
        while (serial_device_.read_byte(&receivedChar))
        {
            if (static_cast<char>(receivedChar) == expectedResponse[expectedResponseIndex])
            {
                ++expectedResponseIndex;

                // if we reach the end of the expected response string, break the loops
                if (expectedResponseIndex >= strlen(expectedResponse))
                {
                    retVal = ErrorCode::E_OK;
                    break;
                }
            }
            else
            {
                // string are not equal. Start again from beginning.
                expectedResponseIndex = 0;
                retVal                = ErrorCode::E_RESPONSE_WRONG;
            }
        }
    }

    return retVal;
}

#endif
