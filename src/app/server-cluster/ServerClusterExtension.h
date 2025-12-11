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

#include <app/ConcreteClusterPath.h>
#include <app/server-cluster/ServerClusterContext.h>
#include <app/server-cluster/ServerClusterInterface.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/Span.h>

namespace chip::app {

/// A proxy for a ServerClusterInterface that adds data version management and attribute change notification
/// capability that works together with an underlying server cluster interface:
///   - a ServerClusterContext received from startup is retained
///   - a data version delta is maintained compared to underlying, to account for extra data
///
/// This class wraps an existing ServerClusterInterface instance and is intended to be
/// used as a base class for extending cluster functionality, like adding new attributes
/// or commands to an existing server cluster interface.
///
/// The class is intended to wrap a SINGLE cluster path for a given interface, even if the interface
/// itself supports multiple paths.
///
/// Expected usage is that the `ServerClusterExtension` is registered while the `underlying` interface
/// is not. This can be used to wrap an existing registered interface like:
///
/// ```
/// ServerClusterInterface *underlying = registry.Get(clusterPath);
/// registry.Unregister(underlying);
/// auto extension = std::make_unique<RegisteredServerCluster<Extension>>(clusterPath, *underlying);
/// registry.Register(extension->Registration());
/// ```
///
/// An extension could be chained like `Extension2(path2, Extension1(path1, underlying))` if more than
/// one extension is desired.
///
/// NOTES:
///   - if changing an attribute (via WriteAttribute or as part of other operations), remember to call
///     NotifyAttributeChanged so that attribute subscriptions work correctly.
class ServerClusterExtension : public ServerClusterInterface
{
public:
    explicit ServerClusterExtension(const ConcreteClusterPath & path, ServerClusterInterface & underlying) :
        mClusterPath(path), mUnderlying(underlying)
    {}

    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown(ClusterShutdownType type) override;
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
    const ConcreteClusterPath mClusterPath;
    ServerClusterInterface & mUnderlying;

    // Cluster context, set on Startup and reset to nullptr on shutdown.
    ServerClusterContext * mContext = nullptr;

    // A data version increment for when the extension's data changes.
    // Since data version is explicitly random to start and wraps, the extension's
    // version is computed as "underlying version + delta".
    DataVersion mVersionDelta = 0;

    /// Mark the given attribute as changed:
    ///   - calls the underlying context if available (i.e. make sure reporting works)
    ///   - updates internal version delta
    void NotifyAttributeChanged(AttributeId id);
};

} // namespace chip::app
