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

    using SemanticTag = Clusters::Descriptor::Structs::SemanticTagStruct::Type;

    virtual CHIP_ERROR Endpoints(ListBuilder<EndpointEntry> & builder) = 0;

    virtual CHIP_ERROR SemanticTags(EndpointId endpointId, ListBuilder<SemanticTag> & builder)          = 0;
    virtual CHIP_ERROR DeviceTypes(EndpointId endpointId, ListBuilder<DeviceTypeEntry> & builder)       = 0;
    virtual CHIP_ERROR ClientClusters(EndpointId endpointId, ListBuilder<ClusterId> & builder)          = 0;
    virtual CHIP_ERROR ServerClusters(EndpointId endpointId, ListBuilder<ServerClusterEntry> & builder) = 0;

    /// Attribute lists contain all attributes. This MUST include all global
    /// attributes (See SPEC 7.13 Global Elements / Global Attributes Table).
    /// In particular this MUST contain:
    ///    - AcceptedCommandList::Id
    ///    - AttributeList::Id
    ///    - ClusterRevision::Id
    ///    - FeatureMap::Id
    ///    - GeneratedCommandList::Id
    virtual CHIP_ERROR Attributes(const ConcreteClusterPath & path, ListBuilder<AttributeEntry> & builder)             = 0;
    virtual CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ListBuilder<CommandId> & builder)           = 0;
    virtual CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path, ListBuilder<AcceptedCommandEntry> & builder) = 0;

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

    // "convenience" functions that just return the data and ignore the error
    // This returns the `ListBuilder<..>::TakeBuffer` from their equivalent fuctions as-is,
    // even after an error (e.g. not found would return empty data).
    //
    // Usage of these indicates no error handling (not even logging) and code should
    // consider handling errors instead.
    ReadOnlyBuffer<EndpointEntry> EndpointsIgnoreError();
    ReadOnlyBuffer<ServerClusterEntry> ServerClustersIgnoreError(EndpointId endpointId);
    ReadOnlyBuffer<AttributeEntry> AttributesIgnoreError(const ConcreteClusterPath & path);
};

} // namespace DataModel
} // namespace app
} // namespace chip
