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

#include <app/AttributePathParams.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteClusterPath.h>
#include <app/ConcreteCommandPath.h>
#include <app/ConcreteEventPath.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model/List.h>
#include <clusters/Descriptor/Structs.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <lib/support/Span.h>

namespace chip {
namespace app {
namespace DataModel {

/// Provides metadata information for a data model
///
/// The data model can be viewed as a tree of endpoint/cluster/(attribute+commands+events)
/// where each element can be iterated through independently.
class ProviderMetadataTree
{
public:
    virtual ~ProviderMetadataTree() = default;

    using SemanticTag = Clusters::Descriptor::Structs::SemanticTagStruct::Type;

    virtual CHIP_ERROR Endpoints(ReadOnlyBufferBuilder<EndpointEntry> & builder) = 0;

    virtual CHIP_ERROR SemanticTags(EndpointId endpointId, ReadOnlyBufferBuilder<SemanticTag> & builder)          = 0;
    virtual CHIP_ERROR DeviceTypes(EndpointId endpointId, ReadOnlyBufferBuilder<DeviceTypeEntry> & builder)       = 0;
    virtual CHIP_ERROR ClientClusters(EndpointId endpointId, ReadOnlyBufferBuilder<ClusterId> & builder)          = 0;
    virtual CHIP_ERROR ServerClusters(EndpointId endpointId, ReadOnlyBufferBuilder<ServerClusterEntry> & builder) = 0;
#if CHIP_CONFIG_USE_ENDPOINT_UNIQUE_ID
    virtual CHIP_ERROR EndpointUniqueID(EndpointId endpointId, MutableCharSpan & EndpointUniqueId) = 0;
#endif

    /// Fetch event metadata for a specific event
    ///
    /// This metadata is used for event validation, specifically ACL at this time. Method is generally
    /// expected to return required access data for events.
    ///
    /// - Implementations MUST return valid event permission values for known events.
    /// - Implementations MAY choose to default to return a default kView when events are unknown,
    ///   in order to save on processing complexity and not deny event subscriptions
    ///   (even if specific events may never be generated).
    ///
    /// No explicit CHIP_ERROR values beyond CHIP_NO_ERROR (i.e. success) are defined. Returning failure
    /// from this method essentially means "This event is known as not supported by this provider" and
    /// the caller is not required to make any more differentiation beyond that, nor is the implementation
    /// required to return specific CHIP_ERROR values (like invalid endpoint/cluster/...)
    virtual CHIP_ERROR EventInfo(const ConcreteEventPath & path, EventEntry & eventInfo) = 0;

    /// Attribute lists contain all attributes. This MUST include all global
    /// attributes (See SPEC 7.13 Global Elements / Global Attributes Table).
    /// In particular this MUST contain:
    ///    - AcceptedCommandList::Id
    ///    - AttributeList::Id
    ///    - ClusterRevision::Id
    ///    - FeatureMap::Id
    ///    - GeneratedCommandList::Id
    virtual CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<AttributeEntry> & builder)   = 0;
    virtual CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) = 0;
    virtual CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                        ReadOnlyBufferBuilder<AcceptedCommandEntry> & builder)                         = 0;

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
    // This returns the `ReadOnlyBufferBuilder<..>::TakeBuffer` from their equivalent fuctions as-is,
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
