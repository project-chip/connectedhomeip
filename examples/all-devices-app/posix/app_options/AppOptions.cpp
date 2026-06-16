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
constexpr uint16_t kOptionDeviceType    = 0xffd0;
constexpr uint16_t kOptionPort          = 0xffd7;
constexpr uint16_t kOptionGroupcast     = 0xffda;
constexpr uint16_t kOptionNamedPipe     = 0xffdb;

DeviceTypeParser AppOptions::sParser;
AppOptions::AppConfig AppOptions::mConfig;
std::string AppOptions::mNamedPipePath;

const AppOptions::AppConfig & AppOptions::GetConfig()
{
    if (mConfig.deviceTypeEntries.empty())
    {
        mConfig.deviceTypeEntries.push_back({
            .type     = chip::app::DeviceFactory::GetInstance().GetDefaultDevice(),
            .endpoint = 1,
            .parentId = chip::kInvalidEndpointId,
        });
    }
    return mConfig;
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
        mConfig.deviceTypeEntries = sParser.GetDeviceTypeEntries();
        return true;
    }
    case kOptionPort: {
        char * endptr;
        unsigned long val = strtoul(value, &endptr, 0);
        if (*endptr != '\0' || val > 0xFFFF)
        {
            ChipLogError(Support, "Invalid port: %s", value);
            return false;
        }
        mConfig.port = static_cast<uint16_t>(val);
        LinuxDeviceOptions::GetInstance().securedDevicePort = static_cast<uint16_t>(val);
        ChipLogProgress(AppServer, "Port option set to %u", static_cast<uint16_t>(val));
        return true;
    }
    case kOptionGroupcast:
        mConfig.enableGroupcast = true;
        ChipLogProgress(AppServer, "Groupcast usage enabled");
        return true;
    case kOptionNamedPipe:
        mNamedPipePath = value;
        ChipLogProgress(AppServer, "Named pipe path set to %s", value);
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
        { "port", kArgumentRequired, kOptionPort },
        { "groupcast", kNoArgument, kOptionGroupcast },
        { "named-pipe", kArgumentRequired, kOptionNamedPipe },
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

        result += "  --port <number>\n";
        result += "       Listen port for secure device messages (default: 5540)\n\n";

        result += "  --named-pipe <path>\n";
        result += "       Path to a named pipe to receive test JSON commands.\n\n";

        return result;
    }();

    static OptionSet sCmdLineOptions = { AllDevicesAppOptionHandler, // handler function
                                         sAllDevicesAppOptionDefs,   // array of option definitions
                                         "PROGRAM OPTIONS",          // help group
                                         gHelpText.c_str() };

    return &sCmdLineOptions;
}
