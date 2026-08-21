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
#include <lib/core/CHIPError.h>

#include <optional>

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

    // Application-facing accessors. An application that knows of a network by
    // other means — a border router knows its own — can record and retract it
    // here. Going through the cluster rather than around it to the storage is
    // what makes the change visible: subscribers to ThreadNetworks are told,
    // and the cluster's data version moves.
    //
    // Each of these leaves the cluster's invariants intact on its own, so an
    // application can call them in any order without arranging a moment where
    // PreferredExtendedPanID names a network that is not in the list.

    // Records a network, replacing any entry with the same Extended PAN ID.
    // The dataset must carry the sub-TLVs the specification requires of an
    // entry, the same set AddNetwork checks.
    CHIP_ERROR AddOrUpdateNetwork(const ThreadNetworkDirectoryStorage::ExtendedPanId & extendedPanId, ByteSpan dataset);

    // Retracts a network. If PreferredExtendedPanID names it, the preference
    // is cleared first: null means no preference, which is always legal, and
    // an application that wants to move the preference elsewhere calls
    // SetPreferredNetwork afterwards.
    CHIP_ERROR ForgetNetwork(const ThreadNetworkDirectoryStorage::ExtendedPanId & extendedPanId);

    // Reads PreferredExtendedPanID; empty when null.
    CHIP_ERROR GetPreferredNetwork(std::optional<ThreadNetworkDirectoryStorage::ExtendedPanId> & extendedPanId);

    // Points PreferredExtendedPanID at a network, which must already be in the
    // list, or clears it when given nothing.
    CHIP_ERROR SetPreferredNetwork(const ThreadNetworkDirectoryStorage::ExtendedPanId * extendedPanId);

private:
    using ExtendedPanId = ThreadNetworkDirectoryStorage::ExtendedPanId;

    // Attribute handling helpers
    ConcreteDataAttributePath PreferredExtendedPanIdPath() const;
    // The sub-TLVs the specification requires of a directory entry.
    static CHIP_ERROR ValidateDatasetForDirectory(ByteSpan dataset, ByteSpan & outExtendedPanId);
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
