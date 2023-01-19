/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/core/Optional.h>
#include <lib/core/TLVTypes.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConnectivityManager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ThreadNetworkDiagnostics;
using namespace chip::app::Clusters::ThreadNetworkDiagnostics::Attributes;
using namespace chip::DeviceLayer;

namespace {

class ThreadDiagosticsAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the ThreadNetworkDiagnostics cluster on all endpoints.
    ThreadDiagosticsAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), ThreadNetworkDiagnostics::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
};

ThreadDiagosticsAttrAccess gAttrAccess;

CHIP_ERROR ThreadDiagosticsAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    if (aPath.mClusterId != ThreadNetworkDiagnostics::Id)
    {
        // We shouldn't have been called at all.
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    switch (aPath.mAttributeId)
    {
    case ThreadNetworkDiagnostics::Attributes::NeighborTable::Id:
    case ThreadNetworkDiagnostics::Attributes::RouteTable::Id:
    case ThreadNetworkDiagnostics::Attributes::SecurityPolicy::Id:
    case ThreadNetworkDiagnostics::Attributes::OperationalDatasetComponents::Id:
    case ThreadNetworkDiagnostics::Attributes::ActiveNetworkFaultsList::Id:
    case ThreadNetworkDiagnostics::Attributes::Channel::Id:
    case ThreadNetworkDiagnostics::Attributes::RoutingRole::Id:
    case ThreadNetworkDiagnostics::Attributes::NetworkName::Id:
    case ThreadNetworkDiagnostics::Attributes::PanId::Id:
    case ThreadNetworkDiagnostics::Attributes::ExtendedPanId::Id:
    case ThreadNetworkDiagnostics::Attributes::MeshLocalPrefix::Id:
    case ThreadNetworkDiagnostics::Attributes::PartitionId::Id:
    case ThreadNetworkDiagnostics::Attributes::Weighting::Id:
    case ThreadNetworkDiagnostics::Attributes::DataVersion::Id:
    case ThreadNetworkDiagnostics::Attributes::StableDataVersion::Id:
    case ThreadNetworkDiagnostics::Attributes::LeaderRouterId::Id:
    case ThreadNetworkDiagnostics::Attributes::OverrunCount::Id:
    case ThreadNetworkDiagnostics::Attributes::DetachedRoleCount::Id:
    case ThreadNetworkDiagnostics::Attributes::ChildRoleCount::Id:
    case ThreadNetworkDiagnostics::Attributes::RouterRoleCount::Id:
    case ThreadNetworkDiagnostics::Attributes::LeaderRoleCount::Id:
    case ThreadNetworkDiagnostics::Attributes::AttachAttemptCount::Id:
    case ThreadNetworkDiagnostics::Attributes::PartitionIdChangeCount::Id:
    case ThreadNetworkDiagnostics::Attributes::BetterPartitionAttachAttemptCount::Id:
    case ThreadNetworkDiagnostics::Attributes::ParentChangeCount::Id:
    case ThreadNetworkDiagnostics::Attributes::TxTotalCount::Id:
    case ThreadNetworkDiagnostics::Attributes::TxUnicastCount::Id:
    case ThreadNetworkDiagnostics::Attributes::TxBroadcastCount::Id:
    case ThreadNetworkDiagnostics::Attributes::TxAckRequestedCount::Id:
    case ThreadNetworkDiagnostics::Attributes::TxAckedCount::Id:
    case ThreadNetworkDiagnostics::Attributes::TxNoAckRequestedCount::Id:
    case ThreadNetworkDiagnostics::Attributes::TxDataCount::Id:
    case ThreadNetworkDiagnostics::Attributes::TxDataPollCount::Id:
    case ThreadNetworkDiagnostics::Attributes::TxBeaconCount::Id:
    case ThreadNetworkDiagnostics::Attributes::TxBeaconRequestCount::Id:
    case ThreadNetworkDiagnostics::Attributes::TxOtherCount::Id:
    case ThreadNetworkDiagnostics::Attributes::TxRetryCount::Id:
    case ThreadNetworkDiagnostics::Attributes::TxDirectMaxRetryExpiryCount::Id:
    case ThreadNetworkDiagnostics::Attributes::TxIndirectMaxRetryExpiryCount::Id:
    case ThreadNetworkDiagnostics::Attributes::TxErrCcaCount::Id:
    case ThreadNetworkDiagnostics::Attributes::TxErrAbortCount::Id:
    case ThreadNetworkDiagnostics::Attributes::TxErrBusyChannelCount::Id:
    case ThreadNetworkDiagnostics::Attributes::RxTotalCount::Id:
    case ThreadNetworkDiagnostics::Attributes::RxUnicastCount::Id:
    case ThreadNetworkDiagnostics::Attributes::RxBroadcastCount::Id:
    case ThreadNetworkDiagnostics::Attributes::RxDataCount::Id:
    case ThreadNetworkDiagnostics::Attributes::RxDataPollCount::Id:
    case ThreadNetworkDiagnostics::Attributes::RxBeaconCount::Id:
    case ThreadNetworkDiagnostics::Attributes::RxBeaconRequestCount::Id:
    case ThreadNetworkDiagnostics::Attributes::RxOtherCount::Id:
    case ThreadNetworkDiagnostics::Attributes::RxAddressFilteredCount::Id:
    case ThreadNetworkDiagnostics::Attributes::RxDestAddrFilteredCount::Id:
    case ThreadNetworkDiagnostics::Attributes::RxDuplicatedCount::Id:
    case ThreadNetworkDiagnostics::Attributes::RxErrNoFrameCount::Id:
    case ThreadNetworkDiagnostics::Attributes::RxErrUnknownNeighborCount::Id:
    case ThreadNetworkDiagnostics::Attributes::RxErrInvalidSrcAddrCount::Id:
    case ThreadNetworkDiagnostics::Attributes::RxErrSecCount::Id:
    case ThreadNetworkDiagnostics::Attributes::RxErrFcsCount::Id:
    case ThreadNetworkDiagnostics::Attributes::RxErrOtherCount::Id:
    case ThreadNetworkDiagnostics::Attributes::ActiveTimestamp::Id:
    case ThreadNetworkDiagnostics::Attributes::PendingTimestamp::Id:
    case ThreadNetworkDiagnostics::Attributes::Delay::Id:
    case ThreadNetworkDiagnostics::Attributes::ChannelPage0Mask::Id:
        return ConnectivityMgr().WriteThreadNetworkDiagnosticAttributeToTlv(aPath.mAttributeId, aEncoder);
    default:
        break;
    }

    return CHIP_NO_ERROR;
}

} // anonymous namespace

bool emberAfThreadNetworkDiagnosticsClusterResetCountsCallback(app::CommandHandler * commandObj,
                                                               const app::ConcreteCommandPath & commandPath,
                                                               const Commands::ResetCounts::DecodableType & commandData)
{
    ConnectivityMgr().ResetThreadNetworkDiagnosticsCounts();
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

void MatterThreadNetworkDiagnosticsPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gAttrAccess);
}
