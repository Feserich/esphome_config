#ifndef HONEYWELL_MANAGER_OPEN_HR20_H
#define HONEYWELL_MANAGER_OPEN_HR20_H

/**
 * @file HoneywellManager_OpenHR20.h
 *
 * @brief This class can be used to controll the radiator thermostat "Honeywell HR20" Hardware Revision 2 with OpenHR20 firmware.
 *        It can be used in an IOT µC to controll your heater over the Internet.
 *        All functions are useing the UART class from ESPHome to send the commands to the thermostat.
 *
 */

#include "IHoneywellManager.h"
#include "esphome.h"
#include <cstdint>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

class HoneywellManager_OpenHR20 : public IHoneywellManager
{
public:
    HoneywellManager_OpenHR20(UARTComponent* uart_component_ptr);

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
     * @brief Set manual or automatic mode. In automatic mode the stored heating programm will be used.
     *
     * @param mode manual or automatic mode.
     */
    ErrorCode SetMode(Mode mode) override;

    /**
     * @brief Get manual or automatic mode. In automatic mode the stored heating programm will be used.
     *
     * @param mode manual or automatic mode.
     * @return Error code, see enum class definition.
     */
    ErrorCode GetMode(Mode& mode) override;

    /**
     * @brief Get the current temperature for the radiator thermostat.
     *
     * @param temperature Temperature value in celsius and with factor 10 offset (fixed point; e.g.: 225 => 22.5°C)
     * @return Error code, see enum class definition.
     */
    ErrorCode GetCurrentTemperature(int& temperature);

    /**
     * @brief Get the current battery voltage
     *
     * @param voltage The voltage value of the batteries in unit mV.
     * @return Error code, see enum class definition.
     */
    ErrorCode GetCurrentBatteryVoltage(int& voltage);

    /**
     * @brief Current position of the valve
     *
     * @param valvePosition The position of the valve in percent %.
     * @return Error code, see enum class definition.
     */
    ErrorCode GetValvePosition(int& valvePosition);

private:
    /**
     * @brief Flush everything from the input uart buffer.
     */
    void flushInputBuffer(void);

    /**
     * @brief Extract one part of the status message from the thermostat
     *
     * @param startTerminatorStr The start terminator string of the status message, after which the data of intrest follwes.
     * @param startByte The number of byte after startTerminatorStr, from which the data shall be extracted.
     * @param dataBufferStr Buffer for the data bytes to extract (Null terminated).
     * @param dataLength Length of the data to extract.
     * @return Error code, see enum class definition.
     */
    ErrorCode extractStatusInformation(const char* startTerminatorStr, const uint8_t startByte, char* dataBufferStr,
                                       const uint8_t dataLength);

    /**
     * @brief Find a given string in the received uart response
     * @param expectedResponse String of the expected response
     * @return Error code, see enum class definition.
     */
    ErrorCode findResponseString(const char* expectedResponse);

    /**
     * @brief Uart device definded by the ESPHome implementation. API is similar to Arduino Serial.
     */
    UARTDevice serial_device_;
};

// Constants

HoneywellManager_OpenHR20::HoneywellManager_OpenHR20(UARTComponent* uart_component_ptr)
    : serial_device_(uart_component_ptr)
{
}

ErrorCode HoneywellManager_OpenHR20::SetDesiredTemperature(int temperature)
{
    ErrorCode retVal              = ErrorCode::E_NOT_OK;
    constexpr int TEMPERATURE_MIN = 75;
    constexpr int TEMPERATURE_MAX = 280;
    char buffer[10];

    if ((TEMPERATURE_MIN <= temperature) && (TEMPERATURE_MAX >= temperature))
    {
        // only 0.5°C steps are allowed --> always round down
        temperature = temperature - (temperature % 5);

        int n = sprintf(buffer, "\nA%x\n", temperature / 5);
        if (n > 0)
        {
            serial_device_.write_str(buffer);
            serial_device_.flush();
            retVal = ErrorCode::E_OK;
        }
    }

    return retVal;
}

ErrorCode HoneywellManager_OpenHR20::SetMode(Mode mode)
{
    ErrorCode ret_val{ ErrorCode::E_OK };

    if (mode == Mode::E_MANUAL)
    {
        serial_device_.write_str("\nM00\n");
        serial_device_.flush();
    }
    else if (mode == Mode::E_AUTOMATIC)
    {
        serial_device_.write_str("\nM01\n");
        serial_device_.flush();
    }
    else
    {
        /* invalid */
        ret_val = ErrorCode::E_NOT_OK;
    }

    return ret_val;
}

ErrorCode HoneywellManager_OpenHR20::GetMode(Mode& mode)
{
    char buffer[10];
    ErrorCode retVal = extractStatusInformation("D: ", 21u, &buffer[0], 1u);

    if (retVal == ErrorCode::E_OK)
    {
        if (buffer[0] == 'A')
        {
            mode = Mode::E_AUTOMATIC;
        }
        else if (buffer[0] == 'M')
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

    if (retVal == ErrorCode::E_OK)
    {
        temperature = std::atoi(&buffer[0]);
    }

    return retVal;
}

ErrorCode HoneywellManager_OpenHR20::GetCurrentTemperature(int& temperature)
{
    char buffer[16];
    ErrorCode retVal = extractStatusInformation("I: ", 0u, &buffer[0], 3u);

    if (retVal == ErrorCode::E_OK)
    {
        temperature = std::atoi(&buffer[0]);
    }

    return retVal;
}

ErrorCode HoneywellManager_OpenHR20::GetCurrentBatteryVoltage(int& voltage)
{
    char buffer[16];
    ErrorCode retVal = extractStatusInformation("B: ", 0u, &buffer[0], 4u);

    if (retVal == ErrorCode::E_OK)
    {
        voltage = std::atoi(&buffer[0]);
    }

    return retVal;
}

ErrorCode HoneywellManager_OpenHR20::GetValvePosition(int& valvePosition)
{
    char buffer[16];
    ErrorCode retVal = extractStatusInformation("V: ", 0u, &buffer[0], 2u);

    if (retVal == ErrorCode::E_OK)
    {
        valvePosition = std::atoi(&buffer[0]);
    }

    return retVal;
}

/*
// private functions
*/

ErrorCode HoneywellManager_OpenHR20::extractStatusInformation(const char* startTerminatorStr, const uint8_t startByte, char* dataBufferStr,
                                                              const uint8_t dataLength)
{
    ErrorCode retVal = ErrorCode::E_NOT_OK;
    constexpr uint32_t MAX_RESPONSE_LOOP_CYCLES{ 10 };

    flushInputBuffer();

    // Request status message
    serial_device_.write_str("D\n");

    for (uint32_t i{ 0 }; i < MAX_RESPONSE_LOOP_CYCLES; ++i)
    {
        retVal = findResponseString(startTerminatorStr);

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

    // if response string was found, then
    if (retVal == ErrorCode::E_OK)
    {
        for (uint8_t i = 0; i < startByte; i++)
        {
            // discard all bytes till start byte of the data of interest
            serial_device_.read();
        }

        serial_device_.read_array(reinterpret_cast<uint8_t*>(dataBufferStr), static_cast<size_t>(dataLength));
        dataBufferStr[dataLength] = '\0';
    }

    // clean up remaining bytes
    flushInputBuffer();

    return retVal;
}

ErrorCode HoneywellManager_OpenHR20::findResponseString(const char* expectedResponse)
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

void HoneywellManager_OpenHR20::flushInputBuffer()
{
    serial_device_.write_str("\n");

    delay(100);
    while (serial_device_.available())
    {
        serial_device_.read();
    }
}
#endif
