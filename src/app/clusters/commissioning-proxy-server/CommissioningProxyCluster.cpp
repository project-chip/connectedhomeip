/*
 *
 *    Copyright (c) 2023-2026 Project CHIP Authors
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

#include <app/clusters/commissioning-proxy-server/CommissioningProxyCluster.h>

#include <app/data-model/Decode.h>
#include <app/data-model/Encode.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/CommissioningProxy/Attributes.h>
#include <clusters/CommissioningProxy/Commands.h>
#include <clusters/CommissioningProxy/Metadata.h>
#include <clusters/CommissioningProxy/Structs.h>
#include <lib/support/logging/CHIPLogging.h>
#include <protocols/interaction_model/StatusCode.h>

using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

using namespace CommissioningProxy::Attributes;

template <typename T>
constexpr bool HasExactlyOneBitSet(T v)
{
    static_assert(std::is_integral_v<T>, "Integral type required");
    return v != 0 && (v & (v - 1)) == 0;
}


CHIP_ERROR CommissioningProxyCluster::Startup(ServerClusterContext & context)
{
    ChipLogProgress(Zcl, "===SHM %s() EndpointId - delegate has %d, cluster has %d", 
        __func__, mDelegate.GetEndpointId(), mPath.mEndpointId);

    if (mDelegate.GetEndpointId() != mPath.mEndpointId)
    {
        ChipLogError(Zcl, "Commissioning Proxy: EndpointId mismatch - delegate has %d, cluster has %d", mDelegate.GetEndpointId(),
                     mPath.mEndpointId);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return DefaultServerCluster::Startup(context);
}

DataModel::ActionReturnStatus CommissioningProxyCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                        AttributeValueDecoder & decoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ChipLogProgress(Zcl, "===SHM %s()", __func__);
    switch (request.path.mAttributeId)
    {
    case ScanMaxTime::Id:
        uint8_t time;
        ReturnErrorOnFailure(decoder.Decode(time));
        mDelegate.SetScanMaxTime(time);
        break;
    default:
        return Protocols::InteractionModel::Status::UnsupportedWrite;    
    }

    //return NotifyAttributeChangedIfSuccess(request.path.mAttributeId, WriteImpl(request, decoder));
    return NotifyAttributeChangedIfSuccess(request.path.mAttributeId, err);
}

DataModel::ActionReturnStatus CommissioningProxyCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                           AttributeValueEncoder & encoder)
{
    ChipLogProgress(Zcl, "===SHM %s()", __func__);
    switch (request.path.mAttributeId)
    {
    case FeatureMap::Id:
        return encoder.Encode(mFeatureFlags);

    case ScanMaxTime::Id:
        return encoder.Encode(mDelegate.GetScanMaxTime());

    default:
        return Status::UnsupportedAttribute;
    }
}

std::optional<DataModel::ActionReturnStatus> CommissioningProxyCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                          TLV::TLVReader & input_arguments,
                                                                                          CommandHandler * handler)
{
    using namespace Commands;
    ChipLogProgress(Zcl, "===SHM %s()", __func__);

    switch (request.path.mCommandId)
    {
    case ProxyConnectRequest::Id:
        return HandleProxyConnectRequest(request, input_arguments, handler);

    case ProxyDisconnectRequest::Id:
        return HandleProxyDisconnectRequest(request, input_arguments, handler);

    case ProxyScanRequest::Id:
        return HandleProxyScanRequest(request, input_arguments, handler);

    case ProxyBackGroundScanStartRequest::Id:
        return HandleProxyBackGroundScanStartRequest(request, input_arguments, handler);

    case ProxyBackGroundScanStopRequest::Id:
        return HandleProxyBackGroundScanStopRequest(request, input_arguments, handler);

    case ProxyMessageRequest::Id:
        return HandleProxyMessageRequest(request, input_arguments, handler);

    default:
        return Status::UnsupportedCommand;
    }
}

DataModel::ActionReturnStatus CommissioningProxyCluster::HandleProxyConnectRequest(const DataModel::InvokeRequest & request,
                                                                                   TLV::TLVReader & input_arguments,
                                                                                   CommandHandler * handler)
{
    ChipLogProgress(Zcl, "===SHM %s()", __func__);

    Commands::ProxyConnectRequest::DecodableType commandData;
    ReturnErrorOnFailure(DataModel::Decode(input_arguments, commandData));

    // Only a single valid transport allowed
    VerifyOrReturnError(HasExactlyOneBitSet(commandData.transport.Raw()), Status::InvalidCommand);

    // Default to zero if optional field is missing
    chip::app::Clusters::CommissioningProxy::WiFiBandBitmap wiFiBand =
        static_cast<chip::app::Clusters::CommissioningProxy::WiFiBandBitmap>(0);

    if (commandData.wiFiBand.HasValue())
    {
        wiFiBand = static_cast<chip::app::Clusters::CommissioningProxy::WiFiBandBitmap>(
            commandData.wiFiBand.Value().Raw());
    }

    // Cluster state is owned by the cluster; access it via the bound server pointer.
    ChipLogProgress(NotSpecified, "=== %s() State:%u WiFiBand:%u", __func__, (uint8_t)GetCPState(), (uint8_t)wiFiBand);
    CHIP_ERROR err = SetCPState(CommissioningProxyCluster::kState_CPConnected);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Commissioning Proxy SetCPState() Failed");
        return chip::Protocols::InteractionModel::Status::Failure;
    }
    ChipLogProgress(NotSpecified, "=== %s() State:%u", __func__, (uint8_t)GetCPState());

    ReturnErrorOnFailure(
        DataModel::ActionReturnStatus(mDelegate.ProxyConnectRequest(
            commandData.address, commandData.transport, commandData.discriminator,
            commandData.vendorId, commandData.productId, wiFiBand, handler, request))
            .GetUnderlyingError());

    return Status::UnsupportedCommand;
}

DataModel::ActionReturnStatus CommissioningProxyCluster::HandleProxyDisconnectRequest(const DataModel::InvokeRequest & request,
                                                                                      TLV::TLVReader & input_arguments,
                                                                                     CommandHandler * handler)
{
    return Status::UnsupportedCommand;
}

DataModel::ActionReturnStatus CommissioningProxyCluster::HandleProxyScanRequest(const DataModel::InvokeRequest & request,
                                                                                TLV::TLVReader & input_arguments,
                                                                                 CommandHandler * handler)
{
    ChipLogProgress(Zcl, "===SHM %s()", __func__);

    Commands::ProxyScanRequest::DecodableType commandData;
    ReturnErrorOnFailure(DataModel::Decode(input_arguments, commandData));

    chip::app::Clusters::CommissioningProxy::WiFiBandBitmap wiFiBands =
        static_cast<chip::app::Clusters::CommissioningProxy::WiFiBandBitmap>(0);

    if (commandData.wiFiBands.HasValue())
    {
        wiFiBands = static_cast<chip::app::Clusters::CommissioningProxy::WiFiBandBitmap>(
            commandData.wiFiBands.Value().Raw());
    }

    ReturnErrorOnFailure(
        DataModel::ActionReturnStatus(mDelegate.ProxyScanRequest(
            commandData.transport, wiFiBands, handler, request))
            .GetUnderlyingError());

    return Status::Success;
}

DataModel::ActionReturnStatus CommissioningProxyCluster::HandleProxyBackGroundScanStartRequest(const DataModel::InvokeRequest & request,
                                                                                               TLV::TLVReader & input_arguments,
                                                                                               CommandHandler * handler)
{
    return Status::UnsupportedCommand;
}

DataModel::ActionReturnStatus CommissioningProxyCluster::HandleProxyBackGroundScanStopRequest(const DataModel::InvokeRequest & request,
                                                                                               TLV::TLVReader & input_arguments,
                                                                                               CommandHandler * handler)
{
    return Status::UnsupportedCommand;
}

DataModel::ActionReturnStatus CommissioningProxyCluster::HandleProxyMessageRequest(const DataModel::InvokeRequest & request,
                                                                                               TLV::TLVReader & input_arguments,
                                                                                               CommandHandler * handler)
{
    return Status::UnsupportedCommand;
}

CHIP_ERROR CommissioningProxyCluster::Attributes(const ConcreteClusterPath & path,
                                                     ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    static constexpr DataModel::AttributeEntry optionalAttributes[] = {
        MaxCachedResults::kMetadataEntry,
        NumCachedResults::kMetadataEntry,
        CacheTimeout::kMetadataEntry,
        CachedResults::kMetadataEntry,
        WiFiBand::kMetadataEntry,
    };

    ChipLogProgress(Zcl, "===SHM %s()", __func__);
    AttributeListBuilder listBuilder(builder);

    return listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes), mEnabledOptionalAttributes);
}

CHIP_ERROR CommissioningProxyCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                           ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    ChipLogProgress(Zcl, "===SHM %s()", __func__);
    ReturnErrorOnFailure(builder.AppendElements({
        Commands::ProxyConnectRequest::kMetadataEntry,
        Commands::ProxyDisconnectRequest::kMetadataEntry,
        Commands::ProxyScanRequest::kMetadataEntry,
        Commands::ProxyMessageRequest::kMetadataEntry
    }));

    if (mFeatureFlags.Has(Feature::kBackgroundScan))
    {
        ChipLogProgress(Zcl, "===SHM %s() Feature::KBackgroundScan", __func__);
        ReturnErrorOnFailure(builder.AppendElements({
            Commands::ProxyBackGroundScanStartRequest::kMetadataEntry,
            Commands::ProxyBackGroundScanStopRequest::kMetadataEntry,
        }));
    }
    ChipLogProgress(Zcl, "===SHM %s() CHIP_NO_ERROR", __func__);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissioningProxyCluster::SetCPState(CommissioningProxyCluster::State_t state)
{
    ChipLogProgress(AppServer, "===SHM %s(), state=%d", __func__, uint8_t(state));
    mMainCommissioningProxyState = state;
    return CHIP_NO_ERROR;
}

CommissioningProxyCluster::State_t CommissioningProxyCluster::GetCPState(void)
{
    ChipLogProgress(AppServer, "===SHM %s(), state=%d", __func__, uint8_t(mMainCommissioningProxyState));
    return mMainCommissioningProxyState;
}

//bool CommissioningProxyManager::IsCPConnected()
//{
//   ChipLogProgress(AppServer, "===SHM %s()", __func__);
//    return mMainCommissioningProxyState == kState_CPConnected;
//}

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
