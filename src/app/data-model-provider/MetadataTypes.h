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

#include "app/data-model-provider/MetadataList.h"
#include "lib/support/Span.h"
#include <cstdint>
#include <optional>

#include <access/Privilege.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributePathParams.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteClusterPath.h>
#include <app/ConcreteCommandPath.h>
#include <app/data-model/List.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitFlags.h>
#include <lib/support/BitMask.h>

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

struct EndpointInfo
{
    // kInvalidEndpointId if there is no explicit parent endpoint (which means the parent is endpoint 0,
    // for endpoints other than endpoint 0).
    EndpointId parentId;
    EndpointCompositionPattern compositionPattern;

    explicit EndpointInfo(EndpointId parent) : parentId(parent), compositionPattern(EndpointCompositionPattern::kFullFamily) {}
    EndpointInfo(EndpointId parent, EndpointCompositionPattern pattern) : parentId(parent), compositionPattern(pattern) {}
};

struct EndpointEntry
{
    EndpointId id;
    EndpointInfo info;

    bool IsValid() const { return id != kInvalidEndpointId; }
    static const EndpointEntry kInvalid;
};

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

    // This iteration will list all the endpoints in the data model
    virtual EndpointEntry FirstEndpoint()                              = 0;
    virtual EndpointEntry NextEndpoint(EndpointId before)              = 0;
    virtual std::optional<EndpointInfo> GetEndpointInfo(EndpointId id) = 0;
    virtual bool EndpointExists(EndpointId id);

    // This iteration describes device types registered on an endpoint
    virtual std::optional<DeviceTypeEntry> FirstDeviceType(EndpointId endpoint)                                  = 0;
    virtual std::optional<DeviceTypeEntry> NextDeviceType(EndpointId endpoint, const DeviceTypeEntry & previous) = 0;


    // This iteration will list all server clusters on a given endpoint
    virtual ClusterEntry FirstServerCluster(EndpointId endpoint)                              = 0;
    virtual ClusterEntry NextServerCluster(const ConcreteClusterPath & before)                = 0;
    virtual std::optional<ClusterInfo> GetServerClusterInfo(const ConcreteClusterPath & path) = 0;

    // This iteration will list all client clusters on a given endpoint
    // As the client cluster is only a client without any attributes/commands,
    // these functions only return the cluster path.
    virtual ConcreteClusterPath FirstClientCluster(EndpointId endpoint)               = 0;
    virtual ConcreteClusterPath NextClientCluster(const ConcreteClusterPath & before) = 0;

    // Attribute iteration and accessors provide cluster-level access over
    // attributes
    virtual AttributeEntry FirstAttribute(const ConcreteClusterPath & cluster)                = 0;
    virtual AttributeEntry NextAttribute(const ConcreteAttributePath & before)                = 0;
    virtual std::optional<AttributeInfo> GetAttributeInfo(const ConcreteAttributePath & path) = 0;

    /// List items. TODO: convert ALL items above to the new format

    using SemanticTag = Clusters::Descriptor::Structs::SemanticTagStruct::Type;

    /// List all the generated commands for the given path
    virtual MetadataList<CommandId> GeneratedCommands(const ConcreteClusterPath & path)           = 0;
    virtual MetadataList<AcceptedCommandEntry> AcceptedCommands(const ConcreteClusterPath & path) = 0;
    virtual MetadataList<SemanticTag> SemanticTags(EndpointId endpointId)                         = 0;

    /// Workaround function to report attribute change.
    ///
    /// When this is invoked, the caller is expected to increment the cluster data version, and the attribute path
    /// should be marked as `dirty` by the data model provider listener so that the reporter can notify the subscriber
    /// of attribute changes.
    /// This function should be invoked when attribute managed by attribute access interface is modified but not
    /// through an actual Write interaction.
    /// For example, if the LastNetworkingStatus attribute changes because the NetworkCommissioning driver detects a
    /// network connection status change and calls SetLastNetworkingStatusValue(). The data model provider can recognize
    /// this change by invoking this function at the point of change.
    ///
    /// This is a workaround function as we cannot notify the attribute change to the data model provider. The provider
    /// should own its data and versions.
    ///
    /// TODO: We should remove this function when the AttributeAccessInterface/CommandHandlerInterface is able to report
    /// the attribute changes.
    virtual void Temporary_ReportAttributeChanged(const AttributePathParams & path) = 0;
};

} // namespace DataModel
} // namespace app
} // namespace chip
