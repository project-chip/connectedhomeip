/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <app_options/AppOptions.h>
#include <devices/device-factory/DeviceFactory.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceConfig.h>

#include <cstdlib>
#include <cstring>

using namespace chip;
using namespace chip::ArgParser;

// App custom argument handling
constexpr uint16_t kOptionDeviceType = 0xffd0;
constexpr uint16_t kOptionEndpoint   = 0xffd1;
constexpr uint16_t kOptionWiFi       = 0xffd2;

std::vector<AppOptions::DeviceConfig> AppOptions::mDeviceConfigs;
bool AppOptions::mEnableWiFi = false;

const std::vector<AppOptions::DeviceConfig> & AppOptions::GetDeviceConfigs()
{
    if (mDeviceConfigs.empty())
    {
        static const std::vector<DeviceConfig> kDefault = { { "contact-sensor", 1 } };
        return kDefault;
    }
    return mDeviceConfigs;
}

bool AppOptions::ParseEndpointId(const char * str, chip::EndpointId & endpoint)
{
    char * endptr;
    long val = strtol(str, &endptr, 10);

    if (endptr == str || *endptr != '\0' || val < 0 || val > UINT16_MAX)
    {
        return false;
    }

    endpoint = static_cast<chip::EndpointId>(val);
    return true;
}

/**
 * Parses a device configuration string in the format "type" or "type:endpoint".
 *
 * Example: "speaker:2" -> type="speaker", endpoint=2
 *
 * @param value The string to parse.
 * @param config The DeviceConfig structure to populate.
 * @return true on success, false on failure (e.g. invalid format or endpoint ID).
 */
bool AppOptions::ParseDeviceConfig(const char * value, DeviceConfig & config)
{
    VerifyOrReturnValue(value != nullptr, false);

    // Set default values for optional fields
    config.endpoint = 1;
    config.parentId = chip::kInvalidEndpointId;

    // Find the first colon to separate type from endpoint
    const char * firstColon = strchr(value, ':');
    
    // Case 1: No colon present. The entire value is treated as the device type.
    // Example: "chime" -> type="chime", endpoint=1, parentId=invalid
    if (firstColon == nullptr)
    {
        config.type = value;
        return true;
    }

    // Extract the type (all characters before the first colon)
    config.type.assign(value, static_cast<size_t>(firstColon - value));

    // Find the second colon to separate endpoint from parent
    const char * secondColon = strchr(firstColon + 1, ':');
    
    // The endpoint ID starts immediately after the first colon
    const char * endpointStart = firstColon + 1;
    
    // If a second colon exists, the endpoint ID is between the colons.
    // Otherwise, it extends to the end of the string.
    size_t endpointLen = secondColon ? static_cast<size_t>(secondColon - endpointStart) : strlen(endpointStart);
    
    // Extract and parse the endpoint ID
    std::string endpointStr(endpointStart, endpointLen);
    if (!ParseEndpointId(endpointStr.c_str(), config.endpoint))
    {
        ChipLogError(Support, "Invalid endpoint ID in device config: %s\n", value);
        return false;
    }

    // Case 2: Two colons present (format: type:endpoint:parent)
    // Extract and parse the parent ID after the second colon.
    if (secondColon != nullptr)
    {
        if (!ParseEndpointId(secondColon + 1, config.parentId))
        {
            ChipLogError(Support, "Invalid parent endpoint ID in device config: %s\n", value);
            return false;
        }
    }

    return true;
}

bool AppOptions::AllDevicesAppOptionHandler(const char * program, OptionSet * options, int identifier, const char * name,
                                            const char * value)
{
    switch (identifier)
    {
    case kOptionDeviceType: {
        DeviceConfig config;
        if (!ParseDeviceConfig(value, config))
        {
            return false;
        }

        ChipLogProgress(AppServer, "Adding device type %s on endpoint %d", config.type.c_str(), config.endpoint);
        mDeviceConfigs.push_back(std::move(config));
        return true;
    }
    case kOptionEndpoint: {
        chip::EndpointId ep;
        if (value == nullptr || !ParseEndpointId(value, ep))
        {
            ChipLogError(Support, "Invalid endpoint ID: %s\n", value ? value : "(null)");
            return false;
        }

        if (mDeviceConfigs.empty())
        {
            ChipLogError(Support, "Warning: --endpoint specified before --device. Creating default 'contact-sensor'.");
            DeviceConfig config;
            config.type     = "contact-sensor";
            config.endpoint = ep;
            mDeviceConfigs.push_back(std::move(config));
        }
        else
        {
            mDeviceConfigs.back().endpoint = ep;
            ChipLogProgress(AppServer, "Updated last device to endpoint %d", ep);
        }
        return true;
    }
    case kOptionWiFi:
        mEnableWiFi = true;
        ChipLogProgress(AppServer, "WiFi usage enabled");
        return true;
    default:
        ChipLogError(Support, "%s: INTERNAL ERROR: Unhandled option: %s\n", program, name);
        return false;
    }

    return true;
}

OptionSet * AppOptions::GetOptions()
{
    static OptionDef sAllDevicesAppOptionDefs[] = {
        { "device", kArgumentRequired, kOptionDeviceType },
        { "endpoint", kArgumentRequired, kOptionEndpoint },
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
        { "wifi", kNoArgument, kOptionWiFi },
#endif
        {}, // need empty terminator
    };

    static const std::string gHelpText = []() {
        // Device option - this is dynamic
        std::string result = "  --device <";
        for (auto & name : app::DeviceFactory::GetInstance().SupportedDeviceTypes())
        {
            result.append(name);
            result.append("|");
        }
        result.replace(result.length() - 1, 1, ">");
        result += "\n";
        result += "       Select the device to start up. Format: 'type' or 'type:endpoint'\n";
        result += "       Can be specified multiple times for multi-endpoint devices.\n";
        result += "       Example: --device chime:1 --device speaker:2\n\n";

        // rest of the help
        result += "  --endpoint <endpoint-number>\n";
        result += "       Define the endpoint for the preceding device (default 1)\n\n";

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
        result += "  --wifi\n";
        result += "       Enable wifi support for commissioning\n\n";
#endif

        return result;
    }();

    static OptionSet sCmdLineOptions = { AllDevicesAppOptionHandler, // handler function
                                         sAllDevicesAppOptionDefs,   // array of option definitions
                                         "PROGRAM OPTIONS",          // help group
                                         gHelpText.c_str() };

    return &sCmdLineOptions;
}
