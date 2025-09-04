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

#include <app/server-cluster/DefaultServerCluster.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {

/// This class provides a code-driven implementation for the Basic Information cluster,
/// centralizing its logic and state. It is designed as a singleton because the cluster
/// is defined to exist only once per node, specifically on the root endpoint (Endpoint 0).
///
/// As a PlatformManagerDelegate, it automatically hooks into the node's lifecycle to
/// emit the mandatory StartUp and optional ShutDown events, ensuring spec compliance.
///
/// Note on the implementation of the singleton pattern:
/// The constructor is public to allow for a global variable instantiation. This approach
/// can save flash memory compared to a function-static instance, which often requires
/// additional thread-safety mechanisms. The intended usage is via the static
/// `Instance()` method, which returns a reference to the global instance.
class GroupcastCluster : public DefaultServerCluster
{
public:
    static GroupcastCluster & Instance();

    GroupcastCluster();

    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown() override;

    // [[nodiscard]] Span<const ConcreteClusterPath> GetPaths() const override;
    // [[nodiscard]] DataVersion GetDataVersion(const ConcreteClusterPath & path) const override;
    // [[nodiscard]] BitFlags<DataModel::ClusterQualityFlags> GetClusterFlags(const ConcreteClusterPath &) const override;
    // void ListAttributeWriteNotification(const ConcreteAttributePath & path, DataModel::ListWriteOperation opType) {}
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                            AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                AttributeValueDecoder & decoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;
    // CHIP_ERROR EventInfo(const ConcreteEventPath & path, DataModel::EventEntry & eventInfo) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request, chip::TLV::TLVReader & input_arguments, CommandHandler * handler) override;
    // CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
    // CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;
    // bool PathsContains(const ConcreteClusterPath & path);
};

} // namespace Clusters
} // namespace app
} // namespace chip
