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

#include <app/server-cluster/ServerClusterContext.h>
#include <app/server-cluster/ServerClusterInterface.h>
#include <lib/support/Span.h>

namespace chip {
namespace app {

/// A proxy for a ServerClusterInterface that adds data version management and attribute change notifications.
///
/// This class wraps an existing ServerClusterInterface instance and is intended to be
/// used as a base class for extending cluster functionality, like adding new attributes
/// or commands to an existing server cluster interface.
///
/// NOTE: class is generally designed to override a SINGLE cluster even though ServerClusterInterface
///       could support multiple server clusters: it maintains a single data version delta that will
///       apply globally whenever an extension attribute is notified as changed.
class ServerClusterExtension : public ServerClusterInterface
{
public:
    explicit ServerClusterExtension(ServerClusterInterface & underlying) : mUnderlying(underlying) {}

    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown() override;
    [[nodiscard]] Span<const ConcreteClusterPath> GetPaths() const override;
    [[nodiscard]] DataVersion GetDataVersion(const ConcreteClusterPath & path) const override;
    [[nodiscard]] BitFlags<DataModel::ClusterQualityFlags> GetClusterFlags(const ConcreteClusterPath & path) const override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;
    void ListAttributeWriteNotification(const ConcreteAttributePath & path, DataModel::ListWriteOperation opType,
                                        FabricIndex accessingFabric) override;
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;
    CHIP_ERROR EventInfo(const ConcreteEventPath & path, DataModel::EventEntry & eventInfo) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;

protected:
    ServerClusterInterface & mUnderlying;
    ServerClusterContext * mContext = nullptr;

    // A data version increment for when the underlying cluster data changes
    DataVersion mVersionDelta = 0;

    /// Mark the given path as changed:
    ///   - calls the underlying context if available (i.e. make sure reporting works)
    ///   - updates internal version delta
    void NotifyAttributeChanged(const AttributePathParams & path);
};

} // namespace app
} // namespace chip
