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
constexpr uint16_t kOptionWiFi       = 0xffd2;

DeviceTypeParser AppOptions::sParser;
bool AppOptions::mEnableWiFi = false;

const std::vector<DeviceTypeParser::Entry> & AppOptions::GetDeviceTypeEntries()
{
    const auto & entries = sParser.GetDeviceTypeEntries();
    if (entries.empty())
    {
        static const std::vector<DeviceTypeParser::Entry> kDefault = { { "contact-sensor", 1, kInvalidEndpointId } };
        return kDefault;
    }
    return entries;
}

bool AppOptions::AllDevicesAppOptionHandler(const char * program, OptionSet * options, int identifier, const char * name,
                                            const char * value)
{
    switch (identifier)
    {
    case kOptionDeviceType: {
        if (sParser.ParseSingleDeviceString(value) != CHIP_NO_ERROR)
        {
            return false;
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
        result += "       Select the device to start up. Format: 'type' or 'type:endpoint' or 'type:endpoint,parent=parentId'\n";
        result += "       Can be specified multiple times for multi-endpoint devices.\n";
        result += "       Example: --device chime:1 --device speaker:2,parent=1\n\n";

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
