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

#include <app/FailSafeContext.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server/CommissioningWindowManager.h>
#include <app/server/Dnssd.h>
#include <clusters/OperationalCredentials/ClusterId.h>
#include <credentials/DeviceAttestationCredsProvider.h>

namespace chip {
namespace app {
namespace Clusters {

class OperationalCredentialsCluster : public DefaultServerCluster, chip::FabricTable::Delegate
{
public:
    struct Context
    {
        FabricTable & fabricTable;
        FailSafeContext & failSafeContext;
        SessionManager & sessionManager;
        DnssdServer & dnssdServer;
        CommissioningWindowManager & commissioningWindowManager;
    };

    OperationalCredentialsCluster(EndpointId endpoint, const Context context) :
        DefaultServerCluster({ endpoint, OperationalCredentials::Id }), mOpCredsContext(context){};

    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown() override;

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    // Function used to handle event FailSafeTimerExpired
    static void FailSafeCleanup(const DeviceLayer::ChipDeviceEvent * event, OperationalCredentialsCluster * cluster);

    // FabricTable delegate
    void FabricWillBeRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override;
    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override;
    void OnFabricUpdated(const FabricTable & fabricTable, FabricIndex fabricIndex) override;
    void OnFabricCommitted(const FabricTable & fabricTable, FabricIndex fabricIndex) override;

private:
    const OperationalCredentialsCluster::Context mOpCredsContext;

    FabricTable & GetFabricTable();
    FailSafeContext & GetFailSafeContext();
    Credentials::DeviceAttestationCredentialsProvider * GetDACProvider();
    SessionManager & GetSessionManager();
    DnssdServer & GetDNSSDServer();
    CommissioningWindowManager & GetCommissioningWindowManager();
};

} // namespace Clusters
} // namespace app
} // namespace chip
