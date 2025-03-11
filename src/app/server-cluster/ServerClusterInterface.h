/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/AttributeValueDecoder.h>
#include <app/AttributeValueEncoder.h>
#include <app/CommandHandler.h>
#include <app/ConcreteClusterPath.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model-provider/MetadataList.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model-provider/OperationTypes.h>
#include <app/server-cluster/ServerClusterContext.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitFlags.h>

namespace chip {
namespace app {

/// Handles cluster interactions for a specific cluster id.
///
/// A `ServerClusterInterface` instance is associated with a single endpointId and represents
/// a cluster that exists at a given `endpointId/clusterId` path.
///
/// Provides metadata as well as interaction processing (attribute read/write and command handling).
class ServerClusterInterface
{
public:
    virtual ~ServerClusterInterface() = default;

    /// Starts up the server cluster interface.
    ///
    /// The `context` lifetime must be guaranteed to last
    /// until `Shutdown` is called.
    virtual CHIP_ERROR Startup(ServerClusterContext * context) = 0;

    /// A shutdown will always be paired with a corresponding Startup.
    virtual void Shutdown() = 0;

    ///////////////////////////////////// Cluster Metadata Support //////////////////////////////////////////////////

    /// The path to this cluster instance.
    ///
    /// This path (endpointid,clusterid) is expected to remain constant once the server
    /// cluster interface is in use.
    [[nodiscard]] virtual ConcreteClusterPath GetPath() const = 0;

    /// Gets the data version for this cluster instance.
    ///
    /// Every cluster instance must have a data version.
    ///
    /// SPEC - 7.10.3. Cluster Data Version
    ///   A cluster data version is a metadata increment-only counter value, maintained for each cluster instance.
    ///   [...]
    ///   A cluster data version SHALL increment or be set (wrap) to zero if incrementing would exceed its
    ///   maximum value. A cluster data version SHALL be maintained for each cluster instance.
    ///   [...]
    ///   A cluster data version SHALL be incremented if any attribute data changes.
    [[nodiscard]] virtual DataVersion GetDataVersion() const = 0;

    [[nodiscard]] virtual BitFlags<DataModel::ClusterQualityFlags> GetClusterFlags() const = 0;

    ///////////////////////////////////// Attribute Support ////////////////////////////////////////////////////////

    ///   Indicates the start/end of a series of list operations. This function will be called either before the first
    ///   Write operation or after the last one of a series of consecutive attribute data values received for the same attribute.
    ///
    ///   1) This function will be called if the client tries to set a nullable list attribute to null.
    ///   2) This function will only be called at the beginning and end of a series of consecutive attribute data
    ///   blocks for the same attribute, no matter what list operations those data blocks represent.
    ///   3) The opType argument indicates the type of notification (Start, Failure, Success).
    virtual void ListAttributeWriteNotification(const ConcreteAttributePath & aPath, DataModel::ListWriteOperation opType) {}

    /// Reads the value of an existing attribute.
    ///
    /// ReadAttribute MUST be done on an "existent" attribute path: only on attributes that are
    /// returned in an `Attributes` call for this cluster.  ReadAttribute is not expected to perform
    /// that verification; the caller is responsible for it.
    ///
    /// `request.path` is expected to have `GetClusterId` as the cluster id as well as an attribute that is
    /// included in an `Attributes` call.
    ///
    /// This MUST HANDLE the following global attributes:
    ///   - FeatureMap::Id
    ///   - ClusterRevision::Id
    ///
    /// This function WILL NOT be called for attributes that can be derived from cluster metadata.
    /// Specifically this WILL NOT be called (and does not need to implement handling for) the
    /// following attribute IDs:
    ///     - AcceptedCommandList::Id
    ///     - AttributeList::Id
    ///     - GeneratedCommandList::Id
    virtual DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                        AttributeValueEncoder & encoder) = 0;

    /// Writes a value to an existing attribute.
    ///
    /// WriteAttribute MUST be done on an "existent" attribute path: only on attributes that are
    /// returned in an `Attributes` call for this cluster.  WriteAttribute is not expected to perform
    /// that verification; the caller is responsible for it.
    ///
    /// `request.path` is expected to have `GetClusterId` as the cluster id as well as an attribute that is
    /// included in a `Attributes` call.
    virtual DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                         AttributeValueDecoder & decoder) = 0;

    /// Retrieves the list of attributes supported by this cluster.
    ///
    /// Attribute list MUST contain global attributes.
    ///
    /// Specifically these attributes MUST always exist in the list for all clusters:
    ///     - ClusterRevision::Id
    ///     - FeatureMap::Id
    ///     - AcceptedCommandList::Id
    ///     - AttributeList::Id
    ///     - GeneratedCommandList::Id
    /// See SPEC 7.13 Global Elements: `Global Attributes` table
    virtual CHIP_ERROR Attributes(const ConcreteClusterPath & path,
                                  DataModel::ListBuilder<DataModel::AttributeEntry> & builder) = 0;

    ///////////////////////////////////// Command Support /////////////////////////////////////////////////////////

    /// Handles the invocation of a command.
    ///
    /// `handler` is used to send back the response.
    ///    - returning `nullopt` means that return value was placed in handler directly.
    ///      This includes cases where command handling and value return will be done asynchronously.
    ///    - returning a value other than Success implies an error reply (error and data are mutually exclusive)
    ///
    /// InvokeCommand MUST be done on an "existent" attribute path: only on commands that are
    /// returned in an `AcceptedCommand` call for this cluster.
    ///
    /// Return value expectations:
    ///   - if a response has been placed into `handler` then std::nullopt MUST be returned. In particular
    ///     note that CHIP_NO_ERROR is NOT the same as std::nullopt:
    ///        > CHIP_NO_ERROR means handler had no status set and we expect the caller to AddStatus(success)
    ///        > std::nullopt means that handler has added an appropriate data/status response
    ///   - if a value is returned (not nullopt) then the handler response MUST NOT be filled. The caller
    ///     will then issue `handler->AddStatus(request.path, <return_value>->GetStatusCode())`. This is a
    ///     convenience to make writing Invoke calls easier.
    virtual std::optional<DataModel::ActionReturnStatus>
    InvokeCommand(const DataModel::InvokeRequest & request, chip::TLV::TLVReader & input_arguments, CommandHandler * handler) = 0;

    /// Retrieves a list of commands accepted by this cluster.
    ///
    /// Returning `CHIP_NO_ERROR` without adding anything to the `builder` list is expected
    /// if no commands are supported by the cluster.
    virtual CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                        DataModel::ListBuilder<DataModel::AcceptedCommandEntry> & builder) = 0;

    /// Retrieves a list of commands generated by this cluster.
    ///
    /// Returning `CHIP_NO_ERROR` without adding anything to the `builder` list is expected
    /// if no commands are generated by processing accepted commands.
    virtual CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, DataModel::ListBuilder<CommandId> & builder) = 0;
};

} // namespace app
} // namespace chip
