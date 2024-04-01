#ifndef I_HONEYWELL_MANAGER_H
#define I_HONEYWELL_MANAGER_H

/**
 * @file IHoneywellManager.h
 *
 * @brief Interface class for generic API for OpenHR20 and HR20_V1 implementation.
 *        It can be used in an IOT µC to controll your heater over the Internet.
 *
 */

#include <cstdint>

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

class IHoneywellManager
{
public:
    /**
     * @brief Default d'tor
     */
    ~IHoneywellManager() = default;

    /**
     * @brief Set the desired temperature for the radiator thermostat.
     *
     * @param temperature Temperature value in celsius and with factor 10 offset (e.g.: 225 => 22.5°C)
     * @return Error code, see enum class definition.
     */
    virtual ErrorCode SetDesiredTemperature(int temperature) = 0;

    /**
     * @brief Get the desired temperature for the radiator thermostat.
     *
     * @param temperature Temperature value in celsius and with factor 10 offset (fixed point; e.g.: 225 => 22.5°C)
     * @return Error code, see enum class definition.
     */
    virtual ErrorCode GetDesiredTemperature(int& temperature) = 0;

    /**
     * @brief Set manual or automatic mode. In automatic mode the stored heating programm will be used.
     *
     * @param mode manual or automatic mode.
     */
    virtual ErrorCode SetMode(Mode mode) = 0;

    /**
     * @brief Get manual or automatic mode. In automatic mode the stored heating programm will be used.
     *
     * @param mode manual or automatic mode.
     * @return Error code, see enum class definition.
     */
    virtual ErrorCode GetMode(Mode& mode) = 0;

private:
};

#endif
