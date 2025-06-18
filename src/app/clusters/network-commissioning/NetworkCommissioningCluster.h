/*
 *    Copyright (c) 2021-2025 Project CHIP Authors
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

#include <app/clusters/network-commissioning/NetworkCommissioningLogic.h>
#include <app/server-cluster/DefaultServerCluster.h>

namespace chip {
namespace app {
namespace Clusters {

/// Integration of Network Commissioning logic within the Matter data model
///
/// Translates between matter calls and Network commissioning logic
class NetworkCommissioningCluster : public DefaultServerCluster
{
public:
    NetworkCommissioningCluster(EndpointId endpointId, DeviceLayer::NetworkCommissioning::WiFiDriver * driver) :
        DefaultServerCluster({ endpointId, NetworkCommissioning::Id }), mLogic(endpointId, driver)
    {}
    NetworkCommissioningCluster(EndpointId endpointId, DeviceLayer::NetworkCommissioning::ThreadDriver * driver) :
        DefaultServerCluster({ endpointId, NetworkCommissioning::Id }), mLogic(endpointId, driver)
    {}
    NetworkCommissioningCluster(EndpointId endpointId, DeviceLayer::NetworkCommissioning::EthernetDriver * driver) :
        DefaultServerCluster({ endpointId, NetworkCommissioning::Id }), mLogic(endpointId, driver)
    {}

    CHIP_ERROR Init() { return mLogic.Init(); }

    // Undo of the init. Separate name as `Shutdown` has meaning for a server cluster
    void Deinit() { mLogic.Shutdown(); }

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

private:
    NetworkCommissioningLogic mLogic;
};

} // namespace Clusters
} // namespace app
} // namespace chip
