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

#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/ThreadNetworkDiagnostics/Metadata.h>
#include <platform/DiagnosticDataProvider.h>

namespace chip::app::Clusters {

namespace ThreadNetworkDiagnostics {

constexpr uint32_t kFeaturesAll = static_cast<uint32_t>(Feature::kErrorCounts) | static_cast<uint32_t>(Feature::kPacketCounts) |
    static_cast<uint32_t>(Feature::kMACCounts) | static_cast<uint32_t>(Feature::kMLECounts);

static constexpr DataModel::AttributeEntry kFullAttributes[] = {
    // Mandatory Attributes
    Attributes::Channel::kMetadataEntry,                      //
    Attributes::RoutingRole::kMetadataEntry,                  //
    Attributes::NetworkName::kMetadataEntry,                  //
    Attributes::PanId::kMetadataEntry,                        //
    Attributes::ExtendedPanId::kMetadataEntry,                //
    Attributes::MeshLocalPrefix::kMetadataEntry,              //
    Attributes::NeighborTable::kMetadataEntry,                //
    Attributes::RouteTable::kMetadataEntry,                   //
    Attributes::PartitionId::kMetadataEntry,                  //
    Attributes::Weighting::kMetadataEntry,                    //
    Attributes::DataVersion::kMetadataEntry,                  //
    Attributes::StableDataVersion::kMetadataEntry,            //
    Attributes::LeaderRouterId::kMetadataEntry,               //
    Attributes::SecurityPolicy::kMetadataEntry,               //
    Attributes::ChannelPage0Mask::kMetadataEntry,             //
    Attributes::OperationalDatasetComponents::kMetadataEntry, //
    Attributes::ActiveNetworkFaultsList::kMetadataEntry,      //
    Attributes::ExtAddress::kMetadataEntry,                   //
    Attributes::Rloc16::kMetadataEntry,                       //
    // Optional Attributes
    Attributes::ActiveTimestamp::kMetadataEntry,                   //
    Attributes::PendingTimestamp::kMetadataEntry,                  //
    Attributes::Delay::kMetadataEntry,                             //
    Attributes::DetachedRoleCount::kMetadataEntry,                 //
    Attributes::ChildRoleCount::kMetadataEntry,                    //
    Attributes::RouterRoleCount::kMetadataEntry,                   //
    Attributes::LeaderRoleCount::kMetadataEntry,                   //
    Attributes::AttachAttemptCount::kMetadataEntry,                //
    Attributes::PartitionIdChangeCount::kMetadataEntry,            //
    Attributes::BetterPartitionAttachAttemptCount::kMetadataEntry, //
    Attributes::ParentChangeCount::kMetadataEntry,                 //
    Attributes::TxTotalCount::kMetadataEntry,                      //
    Attributes::TxUnicastCount::kMetadataEntry,                    //
    Attributes::TxBroadcastCount::kMetadataEntry,                  //
    Attributes::TxAckRequestedCount::kMetadataEntry,               //
    Attributes::TxAckedCount::kMetadataEntry,                      //
    Attributes::TxNoAckRequestedCount::kMetadataEntry,             //
    Attributes::TxDataCount::kMetadataEntry,                       //
    Attributes::TxDataPollCount::kMetadataEntry,                   //
    Attributes::TxBeaconCount::kMetadataEntry,                     //
    Attributes::TxBeaconRequestCount::kMetadataEntry,              //
    Attributes::TxOtherCount::kMetadataEntry,                      //
    Attributes::TxRetryCount::kMetadataEntry,                      //
    Attributes::TxDirectMaxRetryExpiryCount::kMetadataEntry,       //
    Attributes::TxIndirectMaxRetryExpiryCount::kMetadataEntry,     //
    Attributes::TxErrCcaCount::kMetadataEntry,                     //
    Attributes::TxErrAbortCount::kMetadataEntry,                   //
    Attributes::TxErrBusyChannelCount::kMetadataEntry,             //
    Attributes::RxTotalCount::kMetadataEntry,                      //
    Attributes::RxUnicastCount::kMetadataEntry,                    //
    Attributes::RxBroadcastCount::kMetadataEntry,                  //
    Attributes::RxDataCount::kMetadataEntry,                       //
    Attributes::RxDataPollCount::kMetadataEntry,                   //
    Attributes::RxBeaconCount::kMetadataEntry,                     //
    Attributes::RxBeaconRequestCount::kMetadataEntry,              //
    Attributes::RxOtherCount::kMetadataEntry,                      //
    Attributes::RxAddressFilteredCount::kMetadataEntry,            //
    Attributes::RxDestAddrFilteredCount::kMetadataEntry,           //
    Attributes::RxDuplicatedCount::kMetadataEntry,                 //
    Attributes::RxErrNoFrameCount::kMetadataEntry,                 //
    Attributes::RxErrUnknownNeighborCount::kMetadataEntry,         //
    Attributes::RxErrInvalidSrcAddrCount::kMetadataEntry,          //
    Attributes::RxErrSecCount::kMetadataEntry,                     //
    Attributes::RxErrFcsCount::kMetadataEntry,                     //
    Attributes::RxErrOtherCount::kMetadataEntry,                   //
    Attributes::OverrunCount::kMetadataEntry                       //
};

constexpr uint16_t kOptionalAttributesBegin = 19;
constexpr uint16_t kOptionalAttributesEnd   = 64;

} // namespace ThreadNetworkDiagnostics

class ThreadNetworkDiagnosticsCluster : public DefaultServerCluster, public DeviceLayer::ThreadDiagnosticsDelegate
{
public:
    enum class ClusterType
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
