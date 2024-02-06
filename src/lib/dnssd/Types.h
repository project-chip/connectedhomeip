/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <cstdint>
#include <cstring>

namespace chip {
namespace Dnssd {

enum class DiscoveryFilterType : uint8_t
{
    kNone,
    kShortDiscriminator,
    kLongDiscriminator,
    kVendorId,
    kDeviceType,
    kCommissioningMode,
    kInstanceName,
    kCommissioner,
    kCompressedFabricId,
};

struct DiscoveryFilter
{
    DiscoveryFilterType type;
    uint64_t code             = 0;
    const char * instanceName = nullptr;
    DiscoveryFilter() : type(DiscoveryFilterType::kNone), code(0) {}
    DiscoveryFilter(const DiscoveryFilterType newType) : type(newType) {}
    DiscoveryFilter(const DiscoveryFilterType newType, uint64_t newCode) : type(newType), code(newCode) {}
    DiscoveryFilter(const DiscoveryFilterType newType, const char * newInstanceName) : type(newType), instanceName(newInstanceName)
    {}
    bool operator==(const DiscoveryFilter & other) const
    {
        if (type != other.type)
        {
            return false;
        }
        if (type == DiscoveryFilterType::kInstanceName)
        {
            return (instanceName != nullptr) && (other.instanceName != nullptr) && (strcmp(instanceName, other.instanceName) == 0);
        }

        return code == other.code;
    }
};

enum class DiscoveryType
{
    kUnknown,
    kOperational,
    kCommissionableNode,
    kCommissionerNode
};

} // namespace Dnssd
} // namespace chip
