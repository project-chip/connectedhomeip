/**
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/clusters/thread-network-directory-server/ThreadNetworkDirectoryStorage.h>
#include <clusters/ThreadNetworkDirectory/Commands.h>

namespace chip {
namespace app {
namespace Clusters {

class ThreadNetworkDirectoryCluster : public DefaultServerCluster
{
public:
    ThreadNetworkDirectoryCluster(EndpointId endpointId, ThreadNetworkDirectoryStorage & storage);

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;

    // Application-facing accessors: a border router records and retracts the network it is on
    // through the cluster, so subscribers are notified and the data version moves. Each keeps
    // the cluster's invariants on its own.

    // Records a network, replacing an entry with the same Extended PAN ID. The dataset needs the
    // sub-TLVs AddNetwork requires and a newer Active Timestamp than the stored entry; recording
    // the stored dataset again is a no-op.
    CHIP_ERROR AddOrUpdateNetwork(const ThreadNetworkDirectoryStorage::ExtendedPanId & extendedPanId, ByteSpan dataset);

    // Retracts a network. A preference naming it is cleared first; SetPreferredNetwork moves it
    // elsewhere if wanted.
    CHIP_ERROR ForgetNetwork(const ThreadNetworkDirectoryStorage::ExtendedPanId & extendedPanId);

    // Reads PreferredExtendedPanID; empty when null.
    CHIP_ERROR GetPreferredNetwork(std::optional<ThreadNetworkDirectoryStorage::ExtendedPanId> & extendedPanId);

    // Points PreferredExtendedPanID at a listed network, or clears it when given nullptr.
    CHIP_ERROR SetPreferredNetwork(const ThreadNetworkDirectoryStorage::ExtendedPanId * extendedPanId);

private:
    using ExtendedPanId = ThreadNetworkDirectoryStorage::ExtendedPanId;

    // Attribute handling helpers
    ConcreteDataAttributePath PreferredExtendedPanIdPath() const;
    // The sub-TLVs the specification requires of a directory entry.
    static CHIP_ERROR ValidateDatasetForDirectory(ByteSpan dataset, ByteSpan & outExtendedPanId, uint64_t & outActiveTimestamp);
    CHIP_ERROR ReadExtendedPanId(const ConcreteDataAttributePath & aPath, std::optional<ExtendedPanId> & outExPanId);
    CHIP_ERROR ReadPreferredExtendedPanId(const ConcreteDataAttributePath & aPath, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadThreadNetworks(const ConcreteDataAttributePath & aPath, AttributeValueEncoder & aEncoder);
    CHIP_ERROR WritePreferredExtendedPanId(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder);

    // Command handling helpers
    DataModel::ActionReturnStatus HandleAddNetworkRequest(const ThreadNetworkDirectory::Commands::AddNetwork::DecodableType & req);
    DataModel::ActionReturnStatus
    HandleRemoveNetworkRequest(const ThreadNetworkDirectory::Commands::RemoveNetwork::DecodableType & req);
    std::optional<DataModel::ActionReturnStatus>
    HandleOperationalDatasetRequest(CommandHandler & handler,
                                    const ThreadNetworkDirectory::Commands::GetOperationalDataset::DecodableType & req,
                                    const chip::app::ConcreteCommandPath & commandPath);

    ThreadNetworkDirectoryStorage & mStorage;
};

} // namespace Clusters
} // namespace app
} // namespace chip
