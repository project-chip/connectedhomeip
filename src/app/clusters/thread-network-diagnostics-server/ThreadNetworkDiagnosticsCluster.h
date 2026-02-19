/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "lib/support/BitFlags.h"
#include <app/server-cluster/DefaultServerCluster.h>
#include <platform/DiagnosticDataProvider.h>

namespace chip::app::Clusters {

namespace ThreadNetworkDiagnostics {

constexpr BitFlags<Feature> kFeaturesAll{
    Feature::kErrorCounts,  //
    Feature::kPacketCounts, //
    Feature::kMACCounts,    //
    Feature::kMLECounts     //
};

} // namespace ThreadNetworkDiagnostics

class ThreadNetworkDiagnosticsCluster : public DefaultServerCluster, public DeviceLayer::ThreadDiagnosticsDelegate
{
public:
    enum class ClusterType : uint8_t
    {
        kMinimal,
        kFull
    };

    ThreadNetworkDiagnosticsCluster(EndpointId endpointId, ClusterType clusterType);

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown(ClusterShutdownType type) override;

    // ThreadDiagnosticsDelegate implementation
    void OnConnectionStatusChanged(ThreadNetworkDiagnostics::ConnectionStatusEnum newConnectionStatus) override;
    void OnNetworkFaultChanged(const DeviceLayer::GeneralFaults<DeviceLayer::kMaxNetworkFaults> & previous,
                               const DeviceLayer::GeneralFaults<DeviceLayer::kMaxNetworkFaults> & current) override;

private:
    const ClusterType mClusterType;
};

} // namespace chip::app::Clusters
