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
        chip::EndpointId endpoint = chip::kInvalidEndpointId;
        chip::EndpointId parentId = chip::kInvalidEndpointId;
        bool bridged              = false;
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
     * Expands any wildcard "*" entries in the parsed list into the given list of device types,
     * maintaining parentage and allocating sequential endpoints.
     *
     * @param wildcardExpandedTypes The list of leaf device types that wildcard should expand to.
     */
    void ExpandWildcards(const std::vector<std::string> & wildcardExpandedTypes);

    /**
     * Gets the list of parsed device configurations.
     */
    const std::vector<Entry> & GetDeviceTypeEntries();

    /**
     * Validates that the list of device entries contains no duplicate endpoints,
     * and that parentage rules are followed for bridged nodes.
     */
    static CHIP_ERROR ValidateConfig(const std::vector<Entry> & entries);

    /**
     * Clears the list of parsed device configurations.
     */
    void Clear() { mDeviceTypeEntries.clear(); }

private:
    bool ParseDeviceTypeEntry(const char * value, Entry & entry);
    static bool ParseEndpointId(const char * str, chip::EndpointId & endpoint);
    static const Entry * FindEntryByEndpoint(const std::vector<Entry> & entries, chip::EndpointId endpointId);

    std::vector<Entry> mDeviceTypeEntries;
};
