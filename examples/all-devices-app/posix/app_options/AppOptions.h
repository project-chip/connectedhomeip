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

#pragma once

#include <Options.h>
#include <lib/core/DataModelTypes.h>
#include <platform/CHIPDeviceConfig.h>

#include <string>
#include <vector>

class AppOptions
{
public:
    /**
     * @brief Configuration for a single device instance.
     *
     * This structure holds the device type string (e.g. "on-off-light") and the
     * endpoint ID where this device should be instantiated.
     */
    struct DeviceConfig
    {
        std::string type;
        chip::EndpointId endpoint;
    };

    static chip::ArgParser::OptionSet * GetOptions();

    static const std::vector<DeviceConfig> & GetDeviceConfigs();

    static const char * GetDeviceType() { return GetDeviceConfigs().front().type.c_str(); }

    static chip::EndpointId GetDeviceEndpoint() { return GetDeviceConfigs().front().endpoint; }

    static bool EnableWiFi() { return mEnableWiFi; }

private:
    static bool AllDevicesAppOptionHandler(const char * program, chip::ArgParser::OptionSet * options, int identifier,
                                           const char * name, const char * value);

    static bool ParseEndpointId(const char * str, chip::EndpointId & endpoint);
    static bool ParseDeviceConfig(const char * value, DeviceConfig & config);

    static std::vector<DeviceConfig> mDeviceConfigs;
    static bool mEnableWiFi;
};
