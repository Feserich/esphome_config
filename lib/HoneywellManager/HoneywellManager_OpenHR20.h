
#ifndef HONEYWELL_MANAGER_OPEN_HR20_H
#define HONEYWELL_MANAGER_OPEN_HR20_H

/**
 * @file HoneywellManager_OpenHR20.h
 *
 * @brief This class can be used to controll the radiator thermostat "Honeywell HR20" Hardware Revision 2 with OpenHR20 firmware. 
 *        It can be used in an IOT µC to controll your heater over the Internet. 
 *        All functions are useing the Serial class from Arduino to send the commands to the thermostat over UART. 
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <Arduino.h>


/**
 * @brief Possible error codes 
 */
enum class ErrorCode
{
    E_OK,
    E_NOT_OK,
    E_READ_BUF_OVERFLOW,
    E_RESPONSE_TIMEOUT,
    E_RESPONSE_WRONG
};

/**
 * @brief Possible error codes 
 */
enum class Mode
{
    E_INVALID,
    E_MANUAL,
    E_AUTOMATIC
};

class HoneywellManager_OpenHR20
{
    public:
        HoneywellManager_OpenHR20(HardwareSerial* SerialPtr);

        /**
         * @brief Set the desired temperature for the radiator thermostat.
         *
         * @param temperature Temperature value in celsius and with factor 10 offset (e.g.: 225 => 22.5°C)
         * @return Error code, see enum class definition.
         */
        ErrorCode SetDesiredTemperature(int temperature);

        /**
         * @brief Get the desired temperature for the radiator thermostat.
         *
         * @param temperature Temperature value in celsius and with factor 10 offset (fixed point; e.g.: 225 => 22.5°C)
         * @return Error code, see enum class definition.
         */
        ErrorCode GetDesiredTemperature(int& temperature);

        /**
        * @brief Set manual or automatic mode. In automatic mode the stored heating programm will be used. 
        * 
        * @param mode manual or automatic mode. 
        */
        void SetMode(Mode mode);

        /**
        * @brief Get manual or automatic mode. In automatic mode the stored heating programm will be used. 
        * 
        * @param mode manual or automatic mode.
        * @return Error code, see enum class definition. 
        */
        ErrorCode GetMode(Mode& mode);

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
        HardwareSerial* serialPtr;

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
        ErrorCode extractStatusInformation(const char* startTerminatorStr, const uint8_t startByte, char* dataBufferStr, const uint8_t dataLength);

};

#endif