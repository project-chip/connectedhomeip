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

using namespace chip;
using namespace chip::ArgParser;

// App custom argument handling
constexpr uint16_t kOptionDeviceType = 0xffd0;
constexpr uint16_t kOptionEndpoint   = 0xffd1;

const char * AppOptions::mDeviceTypeName     = "contact-sensor"; // defaulting to contact sensor if not specified
chip::EndpointId AppOptions::mDeviceEndpoint = 1;                // defaulting to endpoint 1 if not specified

bool AppOptions::AllDevicesAppOptionHandler(const char * program, OptionSet * options, int identifier, const char * name,
                                            const char * value)
{
    switch (identifier)
    {
    case kOptionDeviceType:
        if (value == nullptr)
        {
            ChipLogError(Support, "INTERNAL ERROR: No device type value passed in.\n");
            return false;
        }
        ChipLogProgress(AppServer, "Using the device type of %s", value);
        mDeviceTypeName = value;
        return true;
    case kOptionEndpoint:
        if (value == nullptr)
        {
            ChipLogError(Support, "INTERNAL ERROR: No endpoint ID value passed in.\n");
            return false;
        }
        mDeviceEndpoint = static_cast<EndpointId>(atoi(value));
        ChipLogProgress(AppServer, "Using endpoint %d for the device.", mDeviceEndpoint);
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
    };

    // TODO: This message on supported device types needs to be updated to scale
    //  better once new devices are added.
    static OptionSet sCmdLineOptions = { AllDevicesAppOptionHandler, // handler function
                                         sAllDevicesAppOptionDefs,   // array of option definitions
                                         "PROGRAM OPTIONS",          // help group
                                         "-d, --device <contact-sensor|water-leak-detector|occupancy-sensor|chime>\n"
                                         "-e, --endpoint <endpoint-number>\n" };

    return &sCmdLineOptions;
}
