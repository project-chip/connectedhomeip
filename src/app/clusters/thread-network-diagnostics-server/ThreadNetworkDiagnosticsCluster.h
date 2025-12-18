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
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/ThreadNetworkDiagnostics/Enums.h>
#include <platform/DeviceInfoProvider.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/GeneralFaults.h>

namespace chip::app::Clusters {

class ThreadNetworkDiagnosticsCluster : public DefaultServerCluster, public DeviceLayer::ThreadDiagnosticsDelegate
{
public:
    struct StartupConfiguration
    {
        bool activeTs{};
        bool pendingTs{};
        bool delay{};
    };

    ThreadNetworkDiagnosticsCluster(EndpointId endpointId, const BitFlags<ThreadNetworkDiagnostics::Feature> features,
                                    const StartupConfiguration & config);

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown() override;

    void OnConnectionStatusChanged(ThreadNetworkDiagnostics::ConnectionStatusEnum newConnectionStatus) override;
    void OnNetworkFaultChanged(const DeviceLayer::GeneralFaults<DeviceLayer::kMaxNetworkFaults> & previous,
                               const DeviceLayer::GeneralFaults<DeviceLayer::kMaxNetworkFaults> & current) override;

private:
    const BitFlags<ThreadNetworkDiagnostics::Feature> mFeatures;

    bool mActiveTs{};
    bool mPendingTs{};
    bool mDelay{};

    std::optional<DataModel::ActionReturnStatus>
    HandleResetCounts(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                      const ThreadNetworkDiagnostics::Commands::ResetCounts::DecodableType & commandData);
};

} // namespace chip::app::Clusters
