
#ifndef ESPHOME_CLIMATE_HONEYWELL_ADAPTER_H
#define ESPHOME_CLIMATE_HONEYWELL_ADAPTER_H

// #include "HoneywellManager_HR20_V1.h"
#include "HoneywellManager_OpenHR20.h"
#include "IHoneywellManager.h"
#include "esphome.h"

/// ESPHome Custom Climate Adapter to control a Honeywell HR20 rondostat
class EsphomeClimateHoneywellAdapter : public PollingComponent, public Climate
{
public:
    EsphomeClimateHoneywellAdapter() = delete;

    /// custom c'tor
    explicit EsphomeClimateHoneywellAdapter(UARTComponent* parent_component)
        : PollingComponent(10 * 60 * 1000)
        , honeywell_manager_(parent_component)
    //, temp_sensor_ptr_(temp_sensor_ptr)
    {
    }

    void setup() override
    {
        // This will be called by App.setup()
    }

    void control(const ClimateCall& call) override
    {
        esphome::optional<float> target_temperature_opt = call.get_target_temperature();

        if (call.get_mode().has_value())
        {
            // User requested mode change
            esphome::optional<float> target_temperature_from_mode = set_mode(*call.get_mode());

            // dependent of the current active mode, the selected target temperature can be overwritten
            if (target_temperature_from_mode.has_value())
            {
                target_temperature_opt = target_temperature_from_mode;
            }
        }

        if (target_temperature_opt.has_value())
        {
            set_target_temperature(*target_temperature_opt);
        }

        set_current_temperature_from_external_sensor();

        // update all states for the Home Assistant GUI
        this->publish_state();
    }

    ClimateTraits traits() override
    {
        // The capabilities of the climate device
        auto traits = climate::ClimateTraits();
        traits.set_supports_current_temperature(true);
        traits.set_supported_modes({ climate::CLIMATE_MODE_AUTO, climate::CLIMATE_MODE_HEAT, climate::CLIMATE_MODE_OFF });
        traits.set_visual_min_temperature(8.0);
        traits.set_visual_max_temperature(28.0);
        traits.set_visual_temperature_step(0.5);

        return traits;
    }

    void update() override
    {
        set_current_temperature_from_external_sensor();

        int desiredTemperature = 0;
        if (ErrorCode::E_OK == honeywell_manager_.GetDesiredTemperature(desiredTemperature))
        {
            this->target_temperature = static_cast<float>(desiredTemperature) / 10.0;

            // if the temperature is below 20° Celcius, get the current mode
            if (this->target_temperature > 20.0)
            {
                this->mode = ClimateMode::CLIMATE_MODE_HEAT;
            }
            else
            {
                Mode mode{ Mode::E_INVALID };
                if (ErrorCode::E_OK == honeywell_manager_.GetMode(mode))
                {
                    if (mode == Mode::E_AUTOMATIC)
                    {
                        this->mode = ClimateMode::CLIMATE_MODE_AUTO;
                    }
                    else if (mode == Mode::E_MANUAL)
                    {
                        this->mode = ClimateMode::CLIMATE_MODE_OFF;
                    }
                }
            }
        }

        this->publish_state();
    }

    void set_current_temperature_from_external_sensor()
    {
        // if an external temperature sensor is given, receive it's value and set if for this climate instance
        // Current temperature format from HR20_V1 is unknown from an A/D converter --> receive current temperature from external sensor.
        /*
        if (temp_sensor_ptr_ != nullptr)
        {
            if (temp_sensor_ptr_->has_state())
            {
                this->current_temperature = temp_sensor_ptr_->get_state();
            }
        }
        */
    }

    esphome::optional<float> set_mode(ClimateMode mode)
    {
        ErrorCode error_code = ErrorCode::E_NOT_OK;
        esphome::optional<float> target_temperature_opt;

        // Send mode to hardware
        if (mode == ClimateMode::CLIMATE_MODE_OFF)
        {
            error_code = honeywell_manager_.SetMode(Mode::E_MANUAL);
        }
        else if (mode == ClimateMode::CLIMATE_MODE_HEAT)
        {
            error_code             = ErrorCode::E_OK;
            target_temperature_opt = esphome::optional<float>(22.0);
        }
        else if (mode == ClimateMode::CLIMATE_MODE_AUTO)
        {
            error_code = honeywell_manager_.SetMode(Mode::E_AUTOMATIC);

            int desiredTemperature = 0;
            if (ErrorCode::E_OK == honeywell_manager_.GetDesiredTemperature(desiredTemperature))
            {
                this->target_temperature = static_cast<float>(desiredTemperature) / 10.0;
            }
        }
        else
        {
            // not supported modes
            error_code = ErrorCode::E_NOT_OK;
        }

        // Publish updated state only if no error occured and mode is supported
        if (ErrorCode::E_OK == error_code)
        {
            this->mode = mode;
        }

        return target_temperature_opt;
    }

    void set_target_temperature(float target_temperature)
    {
        const int expected_temperature = static_cast<int>(target_temperature * 10.0);
        ErrorCode error_code           = honeywell_manager_.SetDesiredTemperature(expected_temperature);

        // only if setting the temeperature was successful, update the target temperature for the GUI
        if (ErrorCode::E_OK == error_code)
        {
            this->target_temperature = target_temperature;
        }
    }

    /// @brief Honeywell Manager instance
    HoneywellManager_OpenHR20 honeywell_manager_;

    /// @brief Pointer to an external temperature sensor to set the current temperature
    // sensor::Sensor* temp_sensor_ptr_;
};

#endif
