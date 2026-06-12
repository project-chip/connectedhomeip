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

#include <algorithm>
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
        if (entry.type == "*")
        {
            entry.endpoint = chip::kInvalidEndpointId;
        }
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

    // Parse options: comma-separated list of options
    if (comma != nullptr)
    {
        std::string optionsStr = comma + 1;
        size_t pos = 0;
        while (pos < optionsStr.size())
        {
            size_t nextComma = optionsStr.find(',', pos);
            std::string opt = (nextComma == std::string::npos) ? optionsStr.substr(pos) : optionsStr.substr(pos, nextComma - pos);
            pos = (nextComma == std::string::npos) ? optionsStr.size() : nextComma + 1;

            if (opt.empty())
            {
                continue;
            }

            if (opt.rfind("parent=", 0) == 0) // starts with "parent="
            {
                std::string parentValStr = opt.substr(7);
                if (!ParseEndpointId(parentValStr.c_str(), entry.parentId))
                {
                    ChipLogError(Support, "Invalid parent endpoint ID in device config: %s\n", value);
                    return false;
                }
            }
            else if (opt == "bridged")
            {
                entry.bridged = true;
            }
            else
            {
                ChipLogError(Support, "Unknown option '%s' in device config: %s\n", opt.c_str(), value);
                return false;
            }
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

void DeviceTypeParser::ExpandWildcards(const std::vector<std::string> & wildcardExpandedTypes)
{
    // Find the highest explicit endpoint ID in the list
    chip::EndpointId maxEp = 0;
    for (const auto & entry : mDeviceTypeEntries)
    {
        if (entry.endpoint != chip::kInvalidEndpointId)
        {
            chip::EndpointId effectiveEp = entry.bridged ? static_cast<chip::EndpointId>(entry.endpoint + 1) : entry.endpoint;
            if (effectiveEp > maxEp)
            {
                maxEp = effectiveEp;
            }
        }
    }

    std::vector<Entry> expandedEntries;
    chip::EndpointId nextAvailableEp = static_cast<chip::EndpointId>(maxEp + 1);

    for (const auto & entry : mDeviceTypeEntries)
    {
        if (entry.type == "*")
        {
            chip::EndpointId nextEp = (entry.endpoint == chip::kInvalidEndpointId) ? nextAvailableEp : entry.endpoint;
            for (const auto & deviceType : wildcardExpandedTypes)
            {
                if (entry.bridged)
                {
                    // For bridged wildcard, add a bridged-node parent, then the device itself
                    expandedEntries.push_back({
                        .type     = "bridged-node",
                        .endpoint = nextEp,
                        .parentId = entry.parentId,
                        .bridged  = true,
                    });
                    expandedEntries.push_back({
                        .type     = deviceType,
                        .endpoint = static_cast<chip::EndpointId>(nextEp + 1),
                        .parentId = nextEp,
                        .bridged  = true,
                    });
                    nextEp = static_cast<chip::EndpointId>(nextEp + 2);
                }
                else
                {
                    // Non-bridged wildcard
                    expandedEntries.push_back({
                        .type     = deviceType,
                        .endpoint = nextEp++,
                        .parentId = entry.parentId,
                        .bridged  = false,
                    });
                }
            }
            if (nextEp > nextAvailableEp)
            {
                nextAvailableEp = nextEp;
            }
        }
        else if (entry.bridged && entry.type != "bridged-node")
        {
            // Explicit bridged entry: expand to bridged-node + target device
            expandedEntries.push_back({
                .type     = "bridged-node",
                .endpoint = entry.endpoint,
                .parentId = entry.parentId,
                .bridged  = true,
            });
            expandedEntries.push_back({
                .type     = entry.type,
                .endpoint = static_cast<chip::EndpointId>(entry.endpoint + 1),
                .parentId = entry.endpoint,
                .bridged  = true,
            });
        }
        else
        {
            expandedEntries.push_back(entry);
        }
    }

    mDeviceTypeEntries = std::move(expandedEntries);
}
