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
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitFlags.h>

namespace chip {
namespace app {

/// Handles cluster interactions for a specific cluster id.
///
/// A `ServerClusterInterface` is generally associated with a single endpointId and represents
/// a cluster that exists at a given `endpointId/clusterId` path.
///
/// Provides metadata as well as interaction processing (attribute read/write and command handling).
class ServerClusterInterface
{
public:
    virtual ~ServerClusterInterface() = default;

    ///////////////////////////////////// Cluster Metadata Support //////////////////////////////////////////////////
    [[nodiscard]] virtual ClusterId GetClusterId() const = 0;

    // Every cluster instance must have a data version.
    //
    // SPEC - 7.10.3. Cluster Data Version
    //   A cluster data version is a metadata increment-only counter value, maintained for each cluster instance.
    //   [...]
    //   A cluster data version SHALL increment or be set (wrap) to zero
    //   if incrementing would exceed its maximum value. A cluster data version
    //   SHALL be maintained for each cluster instance.
    //   [...]
    //   A cluster data version SHALL be incremented if any attribute data changes.
    //
    [[nodiscard]] virtual DataVersion GetDataVersion() const = 0;

    [[nodiscard]] virtual BitFlags<DataModel::ClusterQualityFlags> GetClusterFlags() const = 0;

    ///////////////////////////////////// Attribute Support ////////////////////////////////////////////////////////

    /// ReadAttribute MUST be done on an "existent" attribute path: only on attributes that are
    /// returned in an `Attributes` call for this cluster.
    ///
    /// `request.path` is expected to have `GetClusterId` as the cluster id as well as an attribute that is
    /// included in a `Attributes` call.
    ///
    /// This MUST HANDLE the following global attributes:
    ///   - FeatureMap::Id
    ///   - ClusterRevision::Id
    ///
    /// This function WILL NOT be called for attributes that can be built out of cluster metadata.
    /// Specifically this WILL NOT be called (and does not need to implement handling for) the
    /// following attribute IDs:
    ///     - AcceptedCommandList::Id
    ///     - AttributeList::Id
    ///     - GeneratedCommandList::Id
    virtual DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                        AttributeValueEncoder & encoder) = 0;

    /// WriteAttribute MUST be done on an "existent" attribute path: only on attributes that are
    /// returned in an `Attributes` call for this cluster.
    ///
    /// `request.path` is expected to have `GetClusterId` as the cluster id as well as an attribute that is
    /// included in a `Attributes` call.
    virtual DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                         AttributeValueDecoder & decoder) = 0;

    /// Attribute list MUST contain global attributes.
    ///
    /// Specifically these attributes MUST always exist in the list for all clusters:
    ///     - ClusterRevision::Id
    ///     - FeatureMap::Id
    ///     - AcceptedCommandList::Id
    ///     - AttributeList::Id
    ///     - GeneratedCommandList::Id
    /// See SPEC 7.13 Global Elements: `Global Attributes` table
    ///
    virtual CHIP_ERROR Attributes(const ConcreteClusterPath & path,
                                  DataModel::ListBuilder<DataModel::AttributeEntry> & builder) = 0;

    ///////////////////////////////////// Command Support /////////////////////////////////////////////////////////

    virtual std::optional<DataModel::ActionReturnStatus>
    InvokeCommand(const DataModel::InvokeRequest & request, chip::TLV::TLVReader & input_arguments, CommandHandler * handler) = 0;

    virtual CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                        DataModel::ListBuilder<DataModel::AcceptedCommandEntry> & builder) = 0;

    virtual CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, DataModel::ListBuilder<CommandId> & builder) = 0;
};

} // namespace app
} // namespace chip
