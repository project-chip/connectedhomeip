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
#include <devices/device-type-parser/DeviceTypeParser.h>
#include <lib/core/DataModelTypes.h>
#include <platform/CHIPDeviceConfig.h>

#include <optional>
#include <string>
#include <vector>

class AppOptions
{
public:
    struct AppConfig
    {
        std::vector<DeviceTypeParser::Entry> deviceTypeEntries;
        bool enableWiFi = false;
        std::string kvsPath;
        std::optional<uint16_t> discriminator;
        std::optional<uint16_t> vendorId;
        std::optional<uint16_t> productId;
        std::optional<uint16_t> port;
        std::optional<uint32_t> interfaceId;
    };

    static chip::ArgParser::OptionSet * GetOptions();
    static const AppConfig & GetConfig()
    {
        if (mConfig.deviceTypeEntries.empty())
        {
            mConfig.deviceTypeEntries.push_back({ "contact-sensor", 1, chip::kInvalidEndpointId });
        }
        return mConfig;
    }

    static const std::vector<DeviceTypeParser::Entry> & GetDeviceTypeEntries() { return GetConfig().deviceTypeEntries; }

private:
    static bool AllDevicesAppOptionHandler(const char * program, chip::ArgParser::OptionSet * options, int identifier,
                                           const char * name, const char * value);

    static DeviceTypeParser sParser;
    static AppConfig mConfig;
};
