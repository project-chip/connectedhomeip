/*
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
#include <optional>

#include <access/Privilege.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitFlags.h>

namespace chip {
namespace app {
namespace DataModel {

/// Represents various endpoint composition patters as defined in the spec
/// as `9.2.1. Endpoint Composition patterns`
enum class EndpointCompositionPattern : uint8_t
{
    // Tree pattern supports a general tree of endpoints. Commonly used for
    // device types that support physical device composition (e.g. Refrigerator)
    kTree = 0x1,

    // A full-family pattern is a list fo all descendant endpoints, with no
    // imposed hierarchy.
    //
    // For example the Root Node and Aggregator device types use the full-familiy
    // pattern, as defined in their device type specification
    kFullFamily = 0x2,
};

struct EndpointEntry
{
    EndpointId id;

    // kInvalidEndpointId if there is no explicit parent endpoint (which means the parent is endpoint 0,
    // for endpoints other than endpoint 0).
    EndpointId parentId;
    EndpointCompositionPattern compositionPattern;
};

enum class ClusterQualityFlags : uint32_t
{
    kDiagnosticsData = 0x0001, // `K` quality, may be filtered out in subscriptions
};

struct ServerClusterEntry
{
    ClusterId clusterId;
    DataVersion dataVersion; // current cluster data version,
    BitFlags<ClusterQualityFlags> flags;
};

struct ClusterInfo
{
    DataVersion dataVersion; // current cluster data version,
    BitFlags<ClusterQualityFlags> flags;

    /// Constructor that marks data version as mandatory
    /// for this structure.
    ClusterInfo(DataVersion version) : dataVersion(version) {}
};

enum class AttributeQualityFlags : uint32_t
{
    kListAttribute   = 0x0004, // This attribute is a list attribute
    kFabricScoped    = 0x0008, // 'F' quality on attributes
    kFabricSensitive = 0x0010, // 'S' quality on attributes
    kChangesOmitted  = 0x0020, // `C` quality on attributes
    kTimed           = 0x0040, // `T` quality on attributes (writes require timed interactions)
};

struct AttributeEntry
{
    AttributeId attributeId;
    BitFlags<AttributeQualityFlags> flags;

    // read/write access will be missing if read/write is NOT allowed
    //
    // NOTE: this should be compacted for size
    std::optional<Access::Privilege> readPrivilege;  // generally defaults to View if readable
    std::optional<Access::Privilege> writePrivilege; // generally defaults to Operate if writable
};

// Bitmask values for different Command qualities.
enum class CommandQualityFlags : uint32_t
{
    kFabricScoped = 0x0001,
    kTimed        = 0x0002, // `T` quality on commands
    kLargeMessage = 0x0004, // `L` quality on commands
};

struct AcceptedCommandEntry
{
    CommandId commandId;

    // TODO: this can be more compact (use some flags for privilege)
    //       to make this compact, add a compact enum and make flags/invokePrivilege getters (to still be type safe)
    BitFlags<CommandQualityFlags> flags;
    Access::Privilege invokePrivilege = Access::Privilege::kOperate;
};

/// Represents a device type that resides on an endpoint
struct DeviceTypeEntry
{
    DeviceTypeId deviceTypeId;
    uint8_t deviceTypeRevision;

    bool operator==(const DeviceTypeEntry & other) const
    {
        return (deviceTypeId == other.deviceTypeId) && (deviceTypeRevision == other.deviceTypeRevision);
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
