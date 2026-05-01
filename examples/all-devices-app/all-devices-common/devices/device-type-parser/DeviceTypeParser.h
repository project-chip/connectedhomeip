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

#pragma once

#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <string>
#include <vector>

class DeviceTypeParser
{
public:
    struct Entry
    {
        std::string type;
        chip::EndpointId endpoint;
        chip::EndpointId parentId = chip::kInvalidEndpointId;
    };

    DeviceTypeParser()  = default;
    ~DeviceTypeParser() = default;

    // Disable copy and assignment to be safe, though not strictly necessary if only one instance is used.
    DeviceTypeParser(const DeviceTypeParser &)             = delete;
    DeviceTypeParser & operator=(const DeviceTypeParser &) = delete;

    /**
     * Parses a device configuration string in the format "type" or "type:endpoint" or "type:endpoint,parent=parentId"
     * and adds it to the list of device configs.
     *
     * @param value The string to parse.
     * @return CHIP_NO_ERROR on success, or a CHIP_ERROR on failure.
     */
    CHIP_ERROR ParseSingleDeviceString(const char * value);

    /**
     * Gets the list of parsed device configurations.
     * If no devices were parsed, returns a default list (e.g. contact-sensor on endpoint 1).
     */
    const std::vector<Entry> & GetDeviceTypeEntries();

    /**
     * Clears the list of parsed device configurations.
     */
    void Clear() { mDeviceConfigs.clear(); }

private:
    bool ParseDeviceTypeEntry(const char * value, Entry & entry);
    static bool ParseEndpointId(const char * str, chip::EndpointId & endpoint);

    std::vector<Entry> mDeviceConfigs;
};
