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
#include <app/server-cluster/ServerClusterInterface.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <lib/support/Span.h>
#include <optional>
#include <vector>

namespace chip {
namespace app {

/**
 * @brief An implementation of ServerClusterInterface that relies on the
 *        Ember ZCL Application Framework (and its code generation from ZAP)
 *        to handle attribute storage, command dispatching, and metadata.
 *
 * This class acts as a bridge between the abstract ServerClusterInterface and the
 * concrete Ember ZCL functions and data structures. It is implemented as a temporary
 * thin wrapper to enable us to use the CodeDrivenDataModelProvider while we migrate off of ember..
 *
 * Expected Usage:
 * - Instantiate with one or more ConcreteClusterPath objects that this cluster instance serves.
 * - Register with a data model provider (e.g., CodegenDataModelProvider or CodeDrivenDataModelProvider).
 * - The provider will call Startup() and Shutdown().
 *
 * Thread Safety: Assumes that interactions (ReadAttribute, WriteAttribute, InvokeCommand) are serialized by the calling
 * DataModel::Provider, typically on the Matter event loop. Internal state modifications (like data version) are handled by Ember's
 * mechanisms, which are generally expected to be called from the Matter event loop.
 */
class ServerClusterShim : public ServerClusterInterface
{
public:
    ServerClusterShim(std::initializer_list<ConcreteClusterPath> paths) : mPaths(paths) {}
    ~ServerClusterShim() override;

    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown(ClusterShutdownType shutdownType) override;

    DataVersion GetDataVersion(const ConcreteClusterPath & path) const override;
    BitFlags<DataModel::ClusterQualityFlags> GetClusterFlags(const ConcreteClusterPath &) const override
    {
        return {}; // Not supported by Ember
    }
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    Span<const ConcreteClusterPath> GetPaths() const override
    {
        return Span<const ConcreteClusterPath>(mPaths.data(), mPaths.size());
    };

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;

    void ListAttributeWriteNotification(const ConcreteAttributePath & aPath, DataModel::ListWriteOperation opType,
                                        FabricIndex accessingFabric) override;

    CHIP_ERROR EventInfo(const ConcreteEventPath & path, DataModel::EventEntry & eventInfo) override;

private:
    ServerClusterContext * mContext = nullptr;
    const std::vector<ConcreteClusterPath> mPaths;
};

} // namespace app
} // namespace chip
