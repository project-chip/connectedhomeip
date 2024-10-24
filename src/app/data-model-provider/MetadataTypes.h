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
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteClusterPath.h>
#include <app/ConcreteCommandPath.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitFlags.h>

namespace chip {
namespace app {
namespace DataModel {

enum class ClusterQualityFlags : uint32_t
{
    kDiagnosticsData = 0x0001, // `K` quality, may be filtered out in subscriptions
};

struct ClusterInfo
{
    DataVersion dataVersion; // current cluster data version,
    BitFlags<ClusterQualityFlags> flags;

    /// Constructor that marks data version as mandatory
    /// for this structure.
    ClusterInfo(DataVersion version) : dataVersion(version) {}
};

struct ClusterEntry
{
    ConcreteClusterPath path;
    ClusterInfo info;

    bool IsValid() const { return path.HasValidIds(); }

    static const ClusterEntry kInvalid;
};

enum class AttributeQualityFlags : uint32_t
{
    kListAttribute   = 0x0004, // This attribute is a list attribute
    kFabricScoped    = 0x0008, // 'F' quality on attributes
    kFabricSensitive = 0x0010, // 'S' quality on attributes
    kChangesOmitted  = 0x0020, // `C` quality on attributes
    kTimed           = 0x0040, // `T` quality on attributes (writes require timed interactions)
};

struct AttributeInfo
{
    BitFlags<AttributeQualityFlags> flags;

    // read/write access will be missing if read/write is NOT allowed
    std::optional<Access::Privilege> readPrivilege;  // generally defaults to View if readable
    std::optional<Access::Privilege> writePrivilege; // generally defaults to Operate if writable
};

struct AttributeEntry
{
    ConcreteAttributePath path;
    AttributeInfo info;

    bool IsValid() const { return path.HasValidIds(); }

    static const AttributeEntry kInvalid;
};

enum class CommandQualityFlags : uint32_t
{
    kFabricScoped = 0x0001,
    kTimed        = 0x0002, // `T` quality on commands
};

struct CommandInfo
{
    BitFlags<CommandQualityFlags> flags;
    Access::Privilege invokePrivilege = Access::Privilege::kOperate;
};

struct CommandEntry
{
    ConcreteCommandPath path;
    CommandInfo info;

    bool IsValid() const { return path.HasValidIds(); }

    static const CommandEntry kInvalid;
};

/// Represents a device type that resides on an endpoint
struct DeviceTypeEntry
{
    DeviceTypeId deviceTypeId;
    uint8_t deviceTypeVersion;

    bool operator==(const DeviceTypeEntry & other) const
    {
        return (deviceTypeId == other.deviceTypeId) && (deviceTypeVersion == other.deviceTypeVersion);
    }
};

/// Provides metadata information for a data model
///
/// The data model can be viewed as a tree of endpoint/cluster/(attribute+commands+events)
/// where each element can be iterated through independently.
///
/// Iteration rules:
///   - Invalid paths will be returned when iteration ends (IDs will be kInvalid* and in particular
///     mEndpointId will be kInvalidEndpointId). See `::kInvalid` constants for entries and
///     can use ::IsValid() to determine if the entry is valid or not.
///   - Global Attributes are NOT returned since they are implied
///   - Any internal iteration errors are just logged (callers do not handle iteration CHIP_ERROR)
///   - Iteration order is NOT guaranteed globally. Only the following is guaranteed:
///     - Complete tree iteration (e.g. when iterating an endpoint, ALL clusters of that endpoint
///       are returned, when iterating over a cluster, all attributes/commands are iterated over)
///     - uniqueness and completeness (iterate over all possible distinct values as long as no
///       internal structural changes occur)
class ProviderMetadataTree
{
public:
    virtual ~ProviderMetadataTree() = default;

    virtual EndpointId FirstEndpoint()                 = 0;
    virtual EndpointId NextEndpoint(EndpointId before) = 0;
    virtual bool EndpointExists(EndpointId id);

    // This iteration describes device types registered on an endpoint
    virtual std::optional<DeviceTypeEntry> FirstDeviceType(EndpointId endpoint)                                  = 0;
    virtual std::optional<DeviceTypeEntry> NextDeviceType(EndpointId endpoint, const DeviceTypeEntry & previous) = 0;

    // This iteration will list all clusters on a given endpoint
    virtual ClusterEntry FirstCluster(EndpointId endpoint)                              = 0;
    virtual ClusterEntry NextCluster(const ConcreteClusterPath & before)                = 0;
    virtual std::optional<ClusterInfo> GetClusterInfo(const ConcreteClusterPath & path) = 0;

    // Attribute iteration and accessors provide cluster-level access over
    // attributes
    virtual AttributeEntry FirstAttribute(const ConcreteClusterPath & cluster)                = 0;
    virtual AttributeEntry NextAttribute(const ConcreteAttributePath & before)                = 0;
    virtual std::optional<AttributeInfo> GetAttributeInfo(const ConcreteAttributePath & path) = 0;

    // Command iteration and accessors provide cluster-level access over commands
    virtual CommandEntry FirstAcceptedCommand(const ConcreteClusterPath & cluster)              = 0;
    virtual CommandEntry NextAcceptedCommand(const ConcreteCommandPath & before)                = 0;
    virtual std::optional<CommandInfo> GetAcceptedCommandInfo(const ConcreteCommandPath & path) = 0;

    // "generated" commands are purely for reporting what types of command ids can be
    // returned as responses.
    virtual ConcreteCommandPath FirstGeneratedCommand(const ConcreteClusterPath & cluster) = 0;
    virtual ConcreteCommandPath NextGeneratedCommand(const ConcreteCommandPath & before)   = 0;
};

} // namespace DataModel
} // namespace app
} // namespace chip
