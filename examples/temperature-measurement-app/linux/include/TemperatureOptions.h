#pragma once
#include <lib/support/CHIPArgParser.hpp>
#include <string>

enum TemperatureOptionId {
    THERMAL_ZONE = 1000,
    MIN_TEMPERATURE,
    MAX_TEMPERATURE
};

static chip::ArgParser::OptionDef thermal_option_defs[] = {
    {"thermal-zone",    chip::ArgParser::kArgumentRequired,  THERMAL_ZONE},
    {"min-temperature", chip::ArgParser::kArgumentRequired,  MIN_TEMPERATURE},
    {"max-temperature", chip::ArgParser::kArgumentRequired,  MAX_TEMPERATURE},
    {}    
};

class TemperatureOptions: public chip::ArgParser::OptionSetBase {
public:

    TemperatureOptions():
        m_thermal_zone("/sys/class/thermal/thermal_zone0"),
        m_min_temperature(-10.0),
        m_max_temperature(70.0)
    {
        m_help_string += 
            "  --thermal-zone <path>\n"
            "       Path to the thermal zone, default = " + m_thermal_zone + ".\n"
            "  --min-temperature <temperature-as-float>\n"
            "       Minimum supported temperature, default = " + std::to_string(m_min_temperature) + ".\n"
            "  --max-temperature <temperature-as-float>\n"
            "       Maximum supported temperature, default = " + std::to_string(m_max_temperature) + ".\n";
        OptionDefs = thermal_option_defs;
        HelpGroupName = "Sensor Options";
        OptionHelp = m_help_string.c_str();
    }

    bool HandleOption(const char * progName, chip::ArgParser::OptionSet * optSet, int id, const char * name, const char * arg) override {
        switch (id) {
            case THERMAL_ZONE:
                m_thermal_zone = arg;
                return true;
            case MIN_TEMPERATURE:
                m_min_temperature = std::stof(arg);
                return true;
            case MAX_TEMPERATURE:
                m_max_temperature = std::stof(arg);
                return true;
            default:
                return false;
        }
    }

    std::string getThermalZone() const {
        return m_thermal_zone;
    }
    
    float getMinTemperature() const {
        return m_min_temperature;
    }

    float getMaxTemperature() const {
        return m_max_temperature;
    }

private:
    std::string m_thermal_zone;
    float m_min_temperature;
    float m_max_temperature;
    std::string m_help_string;
};
