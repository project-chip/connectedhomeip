

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
#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributePathParams.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteClusterPath.h>
#include <app/ConcreteCommandPath.h>
#include <app/data-model-provider/MetadataList.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model/List.h>
#include <lib/support/Span.h>

namespace chip {
namespace app {
namespace DataModel {

class ProviderMetadataTree
{
public:
    virtual ~ProviderMetadataTree() = default;

    using SemanticTag = Clusters::Descriptor::Structs::SemanticTagStruct::Type;

    virtual MetadataList<EndpointEntry> Endpoints() = 0;

    virtual MetadataList<SemanticTag> SemanticTags(EndpointId endpointId)          = 0;
    virtual MetadataList<DeviceTypeEntry> DeviceTypes(EndpointId endpointId)       = 0;
    virtual MetadataList<ClusterId> ClientClusters(EndpointId endpointId)          = 0;
    virtual MetadataList<ServerClusterEntry> ServerClusters(EndpointId endpointId) = 0;

    virtual MetadataList<AttributeEntry> Attributes(const ConcreteClusterPath & path)             = 0;
    virtual MetadataList<CommandId> GeneratedCommands(const ConcreteClusterPath & path)           = 0;
    virtual MetadataList<AcceptedCommandEntry> AcceptedCommands(const ConcreteClusterPath & path) = 0;

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
