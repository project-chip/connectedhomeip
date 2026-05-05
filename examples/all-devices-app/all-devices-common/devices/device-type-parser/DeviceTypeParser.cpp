/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "DeviceTypeParser.h"
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <cstdlib>
#include <cstring>

bool DeviceTypeParser::ParseEndpointId(const char * str, chip::EndpointId & endpoint)
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

bool DeviceTypeParser::ParseDeviceTypeEntry(const char * value, Entry & entry)
{
    VerifyOrReturnValue(value != nullptr, false);

    // Set default values for optional fields
    entry.endpoint = 1;
    entry.parentId = chip::kInvalidEndpointId;

    // Find the comma to separate main config from options
    const char * comma = strchr(value, ',');

    std::string mainConfig;
    if (comma != nullptr)
    {
        mainConfig.assign(value, static_cast<size_t>(comma - value));
    }
    else
    {
        mainConfig = value;
    }

    // Parse main config: "type" or "type:endpoint"
    const char * colon = strchr(mainConfig.c_str(), ':');
    if (colon == nullptr)
    {
        entry.type = mainConfig;
    }
    else
    {
        entry.type.assign(mainConfig.c_str(), static_cast<size_t>(colon - mainConfig.c_str()));
        if (!ParseEndpointId(colon + 1, entry.endpoint))
        {
            ChipLogError(Support, "Invalid endpoint ID in device config: %s\n", value);
            return false;
        }
    }

    if (entry.type.empty())
    {
        ChipLogError(Support, "Invalid empty device type in config: %s\n", value);
        return false;
    }

    // Parse options: "parent=parentId"
    if (comma != nullptr)
    {
        const char * optionsStr   = comma + 1;
        const char * parentPrefix = "parent=";
        const char * parentPtr    = strstr(optionsStr, parentPrefix);
        if (parentPtr != nullptr)
        {
            const char * parentValStr = parentPtr + strlen(parentPrefix);
            if (!ParseEndpointId(parentValStr, entry.parentId))
            {
                ChipLogError(Support, "Invalid parent endpoint ID in device config: %s\n", value);
                return false;
            }
        }
        else
        {
            ChipLogError(Support, "Unknown option in device config: %s\n", value);
            return false;
        }
    }

    return true;
}

CHIP_ERROR DeviceTypeParser::ParseSingleDeviceString(const char * value)
{
    Entry entry;
    if (!ParseDeviceTypeEntry(value, entry))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    mDeviceTypeEntries.push_back(std::move(entry));
    return CHIP_NO_ERROR;
}

const std::vector<DeviceTypeParser::Entry> & DeviceTypeParser::GetDeviceTypeEntries()
{
    return mDeviceTypeEntries;
}
