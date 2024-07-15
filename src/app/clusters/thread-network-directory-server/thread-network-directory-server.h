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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/clusters/thread-network-directory-server/ThreadNetworkDirectoryStorage.h>
#include <lib/core/CHIPError.h>

#include <optional>

namespace chip {
namespace app {
namespace Clusters {

class ThreadNetworkDirectoryServer : private AttributeAccessInterface, private CommandHandlerInterface
{
public:
    /*
     * Returns a default instance of this server cluster. Only available if there is
     * exactly one instance of this cluster defined in the application zap file.
     * Must not be called before the data model has been initialized.
     *
     * To support more than one cluster instance, or to customize how the cluster server
     * is instantiated, the application should override the callback function
     * emberAfThreadNetworkDirectoryClusterServerInitCallback(chip::EndpointId).
     */
    static ThreadNetworkDirectoryServer & DefaultInstance();

    ThreadNetworkDirectoryServer(EndpointId endpoint, ThreadNetworkDirectoryStorage & storage);
    ~ThreadNetworkDirectoryServer();

    CHIP_ERROR Init();

    ThreadNetworkDirectoryServer(ThreadNetworkDirectoryServer const &)             = delete;
    ThreadNetworkDirectoryServer & operator=(ThreadNetworkDirectoryServer const &) = delete;

private:
    using ExtendedPanId = ThreadNetworkDirectoryStorage::ExtendedPanId;

    EndpointId GetEndpointId() { return AttributeAccessInterface::GetEndpointId().Value(); }

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;
    void InvokeCommand(HandlerContext & handlerContext) override;

    CHIP_ERROR ReadExtendedPanId(const ConcreteDataAttributePath & aPath, std::optional<ExtendedPanId> & outExPanId);
    CHIP_ERROR ReadPreferredExtendedPanId(const ConcreteDataAttributePath & aPath, AttributeValueEncoder & aEncoder);
    CHIP_ERROR WritePreferredExtendedPanId(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder);
    CHIP_ERROR ReadThreadNetworks(const ConcreteDataAttributePath & aPath, AttributeValueEncoder & aEncoder);

    void HandleAddNetworkRequest(HandlerContext & ctx, const ThreadNetworkDirectory::Commands::AddNetwork::DecodableType & req);
    void HandleRemoveNetworkRequest(HandlerContext & ctx,
                                    const ThreadNetworkDirectory::Commands::RemoveNetwork::DecodableType & req);
    void HandleOperationalDatasetRequest(HandlerContext & ctx,
                                         const ThreadNetworkDirectory::Commands::GetOperationalDataset::DecodableType & req);

    ThreadNetworkDirectoryStorage & mStorage;
};

} // namespace Clusters
} // namespace app
} // namespace chip
