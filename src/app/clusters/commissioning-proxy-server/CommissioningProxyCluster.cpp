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

// Masks of all spec-defined bits for each bitmap type.
// Any bit outside these masks is a reserved bit and SHALL be rejected with InvalidCommand.
constexpr uint8_t  kValidTransportBits =
    static_cast<uint8_t>(CapabilitiesBitmap::kBle) |
    static_cast<uint8_t>(CapabilitiesBitmap::kWiFiPAF);
constexpr uint16_t kValidWiFiBandBits =
    static_cast<uint16_t>(WiFiBandBitmap::k2g4) |
    static_cast<uint16_t>(WiFiBandBitmap::k5g);

// Reserved SessionId sentinel for ProxyDisconnectRequest: cancel any ongoing ProxyConnectRequest.
inline constexpr uint16_t kCancelPendingConnectSessionId = 0xFFFF;

CHIP_ERROR CommissioningProxyCluster::Startup(ServerClusterContext & context)
{
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
    switch (request.path.mAttributeId)
    {
    case ScanMaxTime::Id: {
        uint8_t time;
        ReturnErrorOnFailure(decoder.Decode(time));
        mDelegate.SetScanMaxTime(time);
        break;
    }
    case CacheTimeout::Id: {
        uint16_t cacheTimeout;
        ReturnErrorOnFailure(decoder.Decode(cacheTimeout));
        mDelegate.SetCacheTimeout(cacheTimeout);
        break;
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedWrite;
    }

    return NotifyAttributeChangedIfSuccess(request.path.mAttributeId, err);
}

DataModel::ActionReturnStatus CommissioningProxyCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                           AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case FeatureMap::Id:
        return encoder.Encode(mFeatureFlags);

    case Transport::Id:
        return encoder.Encode(GetSupportedTransports());

    case MaxSessions::Id:
        // Current implementation supports a single concurrent proxy session.
        return encoder.Encode(static_cast<uint8_t>(1));

    case ScanMaxTime::Id:
        return encoder.Encode(mDelegate.GetScanMaxTime());

    case MaxCachedResults::Id:
        return encoder.Encode(mDelegate.GetMaxCachedResults());

    case NumCachedResults::Id:
        return encoder.Encode(mDelegate.GetNumCachedResults());

    case CacheTimeout::Id:
        return encoder.Encode(mDelegate.GetCacheTimeout());

    case CachedResults::Id:
        return mDelegate.EncodeCachedResults(encoder);

    case WiFiBand::Id:
        return encoder.Encode(mDelegate.GetSupportedWiFiBands());

    default:
        return Status::UnsupportedAttribute;
    }
}

chip::BitMask<CapabilitiesBitmap> CommissioningProxyCluster::GetSupportedTransports() const
{
    chip::BitMask<CapabilitiesBitmap> supported;
    if (mFeatureFlags.Has(Feature::kWiFiNetworkInterface))
        supported.Set(CapabilitiesBitmap::kWiFiPAF);
    // When Feature::kBleInterface is defined, add kBle here analogously.
    return supported;
}

std::optional<DataModel::ActionReturnStatus> CommissioningProxyCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                          TLV::TLVReader & input_arguments,
                                                                                          CommandHandler * handler)
{
    using namespace Commands;

    switch (request.path.mCommandId)
    {
    case ProxyConnectRequest::Id: {
        auto result = HandleProxyConnectRequest(request, input_arguments, handler);
        // The delegate always sends the response (sync or async via Handle).
        // Return nullopt so the framework does not add a duplicate status.
        if (result.IsSuccess())
            return std::nullopt;
        return result;
    }

    case ProxyDisconnectRequest::Id:
        return HandleProxyDisconnectRequest(request, input_arguments, handler);

    case ProxyScanRequest::Id: {
        auto result = HandleProxyScanRequest(request, input_arguments, handler);
        // Delegate sends ProxyScanResponse asynchronously; return nullopt on
        // success so the framework does not add a duplicate status.
        if (result.IsSuccess())
            return std::nullopt;
        return result;
    }

    case ProxyBackGroundScanStartRequest::Id:
        return HandleProxyBackGroundScanStartRequest(request, input_arguments, handler);

    case ProxyBackGroundScanStopRequest::Id:
        return HandleProxyBackGroundScanStopRequest(request, input_arguments, handler);

    case ProxyMessageRequest::Id: {
        auto result = HandleProxyMessageRequest(request, input_arguments, handler);
        // Delegate sends ProxyMessageResponse asynchronously; return nullopt on
        // success so the framework does not add a duplicate status.
        if (result.IsSuccess())
            return std::nullopt;
        return result;
    }

    default:
        return Status::UnsupportedCommand;
    }
}

DataModel::ActionReturnStatus CommissioningProxyCluster::HandleProxyConnectRequest(const DataModel::InvokeRequest & request,
                                                                                   TLV::TLVReader & input_arguments,
                                                                                   CommandHandler * handler)
{
    Commands::ProxyConnectRequest::DecodableType commandData;
    ReturnErrorOnFailure(DataModel::Decode(input_arguments, commandData));

    // Only a single transport SHALL be selected per spec
    VerifyOrReturnError(HasExactlyOneBitSet(commandData.transport.Raw()), Status::InvalidCommand);

    // The selected transport must be supported by this proxy instance.
    // The supported set is derived from enabled feature flags (same as the Transport attribute).
    // When Feature::kBleInterface is defined, kBle will automatically be accepted here.
    if (!GetSupportedTransports().HasAny(commandData.transport))
    {
        ChipLogError(Zcl, "Commissioning Proxy: requested transport not supported by this instance");
        return Status::InvalidTransportType;
    }

    // WiFiBand is only valid when the WI feature is enabled per spec
    chip::app::Clusters::CommissioningProxy::WiFiBandBitmap wiFiBand =
        static_cast<chip::app::Clusters::CommissioningProxy::WiFiBandBitmap>(0);

    if (commandData.wiFiBand.HasValue())
    {
        VerifyOrReturnError(mFeatureFlags.Has(Feature::kWiFiNetworkInterface), Status::InvalidCommand);

        // WiFiBand must not contain reserved bits.
        VerifyOrReturnError((commandData.wiFiBand.Value().Raw() & ~kValidWiFiBandBits) == 0, Status::InvalidCommand);

        // WiFiBand must be a subset of the bands supported by this proxy.
        auto supportedBands = mDelegate.GetSupportedWiFiBands();
        if ((commandData.wiFiBand.Value().Raw() & ~supportedBands.Raw()) != 0)
        {
            ChipLogError(Zcl, "CommissioningProxy: Requested WiFiBand not in supported bands");
            return Status::InvalidTransportType;
        }

        wiFiBand = static_cast<chip::app::Clusters::CommissioningProxy::WiFiBandBitmap>(
            commandData.wiFiBand.Value().Raw());
    }

    // Delegate SHALL establish the transport connection and call commandObj->AddResponse()
    // with a ProxyConnectResponse containing the sessionId per spec.
    // State transition to kState_CPConnected happens in the delegate's async
    // success callback (OnPafConnectSuccess) once the transport is actually up.
    auto delegateStatus = mDelegate.ProxyConnectRequest(
        commandData.address, commandData.transport, commandData.discriminator,
        commandData.vendorId, commandData.productId, commandData.timeout, wiFiBand, handler, request);

    ReturnErrorOnFailure(DataModel::ActionReturnStatus(delegateStatus).GetUnderlyingError());

    return Status::Success;
}

DataModel::ActionReturnStatus CommissioningProxyCluster::HandleProxyDisconnectRequest(const DataModel::InvokeRequest & request,
                                                                                      TLV::TLVReader & input_arguments,
                                                                                     CommandHandler * handler)
{
    Commands::ProxyDisconnectRequest::DecodableType commandData;
    ReturnErrorOnFailure(DataModel::Decode(input_arguments, commandData));

    ChipLogProgress(Zcl, "HandleProxyDisconnectRequest: sessionId=0x%04x", commandData.sessionId);

    // SessionId=0xFFFF means cancel any ongoing ProxyConnectRequest, not disconnect a session.
    if (commandData.sessionId == kCancelPendingConnectSessionId)
    {
        return mDelegate.CancelPendingConnect();
    }

    auto delegateStatus = mDelegate.ProxyDisconnectRequest(commandData.sessionId);
    ReturnErrorOnFailure(DataModel::ActionReturnStatus(delegateStatus).GetUnderlyingError());

    // Transition cluster state back to disconnected now that the session is gone.
    CHIP_ERROR stateErr = SetCPState(kState_CPDisconnected);
    if (stateErr != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "HandleProxyDisconnectRequest: SetCPState failed: %" CHIP_ERROR_FORMAT, stateErr.Format());
    }

    return Status::Success;
}

DataModel::ActionReturnStatus CommissioningProxyCluster::HandleProxyScanRequest(const DataModel::InvokeRequest & request,
                                                                                TLV::TLVReader & input_arguments,
                                                                                 CommandHandler * handler)
{
    Commands::ProxyScanRequest::DecodableType commandData;
    ReturnErrorOnFailure(DataModel::Decode(input_arguments, commandData));

    // Transport must be non-zero — at least one transport must be selected.
    VerifyOrReturnError(commandData.transport.Raw() != 0, Status::InvalidCommand);

    // Transport must not contain reserved (undefined) bits.
    VerifyOrReturnError((commandData.transport.Raw() & ~kValidTransportBits) == 0, Status::InvalidCommand);

    // Each selected transport must be supported by this proxy instance.
    // The supported set is derived from enabled feature flags (same as the Transport attribute).
    {
        auto supported = GetSupportedTransports();
        if ((commandData.transport.Raw() & ~supported.Raw()) != 0)
        {
            ChipLogError(Zcl, "CommissioningProxy: one or more requested transports not supported by this instance");
            return Status::InvalidTransportType;
        }
    }

    chip::app::Clusters::CommissioningProxy::WiFiBandBitmap wiFiBands =
        static_cast<chip::app::Clusters::CommissioningProxy::WiFiBandBitmap>(0);
    if (commandData.wiFiBands.HasValue())
    {
        // WiFiBands field is only valid when the WI feature is enabled.
        VerifyOrReturnError(mFeatureFlags.Has(Feature::kWiFiNetworkInterface), Status::InvalidCommand);

        // WiFiBands must not contain reserved bits.
        VerifyOrReturnError((commandData.wiFiBands.Value().Raw() & ~kValidWiFiBandBits) == 0, Status::InvalidCommand);

        // WiFiBands must be a subset of the bands supported by this proxy.
        auto supportedBands = mDelegate.GetSupportedWiFiBands();
        if ((commandData.wiFiBands.Value().Raw() & ~supportedBands.Raw()) != 0)
        {
            ChipLogError(Zcl, "CommissioningProxy: Requested WiFiBand not in supported bands");
            return Status::InvalidTransportType;
        }

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
    Commands::ProxyBackGroundScanStartRequest::DecodableType commandData;
    ReturnErrorOnFailure(DataModel::Decode(input_arguments, commandData));

    // Transport must be non-zero — at least one transport must be selected.
    VerifyOrReturnError(commandData.transport.Raw() != 0, Status::InvalidCommand);

    // Transport must not contain reserved (undefined) bits.
    VerifyOrReturnError((commandData.transport.Raw() & ~kValidTransportBits) == 0, Status::InvalidCommand);

    // Each selected transport must be supported by this proxy instance.
    // The supported set is derived from enabled feature flags (same as the Transport attribute).
    {
        auto supported = GetSupportedTransports();
        if ((commandData.transport.Raw() & ~supported.Raw()) != 0)
        {
            ChipLogError(Zcl, "CommissioningProxy: one or more requested transports not supported by this instance");
            return Status::InvalidTransportType;
        }
    }

    WiFiBandBitmap wiFiBands = static_cast<WiFiBandBitmap>(0);
    if (commandData.wiFiBands.HasValue())
    {
        // WiFiBands field is only valid when the WI feature is enabled.
        VerifyOrReturnError(mFeatureFlags.Has(Feature::kWiFiNetworkInterface), Status::InvalidCommand);

        // WiFiBands must not contain reserved bits.
        VerifyOrReturnError((commandData.wiFiBands.Value().Raw() & ~kValidWiFiBandBits) == 0, Status::InvalidCommand);

        // WiFiBands must be a subset of the bands supported by this proxy.
        auto supportedBands = mDelegate.GetSupportedWiFiBands();
        if ((commandData.wiFiBands.Value().Raw() & ~supportedBands.Raw()) != 0)
        {
            ChipLogError(Zcl, "CommissioningProxy: Requested WiFiBand not in supported bands");
            return Status::InvalidTransportType;
        }

        wiFiBands = static_cast<WiFiBandBitmap>(commandData.wiFiBands.Value().Raw());
    }

    chip::FabricIndex fabricIndex = request.subjectDescriptor.fabricIndex;
    chip::NodeId nodeId           = request.subjectDescriptor.subject;

    auto delegateStatus = mDelegate.ProxyBackgroundScanStartRequest(
        commandData.transport, commandData.timeout, wiFiBands, fabricIndex, nodeId, handler, request);
    ReturnErrorOnFailure(DataModel::ActionReturnStatus(delegateStatus).GetUnderlyingError());

    return Status::Success;
}

DataModel::ActionReturnStatus CommissioningProxyCluster::HandleProxyBackGroundScanStopRequest(const DataModel::InvokeRequest & request,
                                                                                               TLV::TLVReader & input_arguments,
                                                                                               CommandHandler * handler)
{
    Commands::ProxyBackGroundScanStopRequest::DecodableType commandData;
    ReturnErrorOnFailure(DataModel::Decode(input_arguments, commandData));

    // transport=0 and no wiFiBands → nothing to stop
    VerifyOrReturnError(commandData.transport.Raw() != 0 || commandData.wiFiBands.HasValue(), Status::InvalidCommand);

    // Reserved bits in transport must be clear
    VerifyOrReturnError((commandData.transport.Raw() & ~kValidTransportBits) == 0, Status::InvalidCommand);

    // Each non-zero transport bit must be in the supported set.
    // transport=0 (band-only stop) skips this check — there are no unsupported bits in zero.
    {
        auto supported = GetSupportedTransports();
        if ((commandData.transport.Raw() & ~supported.Raw()) != 0)
        {
            ChipLogError(Zcl, "CommissioningProxy: one or more requested transports not supported by this instance");
            return Status::InvalidTransportType;
        }
    }

    WiFiBandBitmap wiFiBands = static_cast<WiFiBandBitmap>(0);
    if (commandData.wiFiBands.HasValue())
    {
        // WiFiBands field is only valid when the WI feature is enabled.
        VerifyOrReturnError(mFeatureFlags.Has(Feature::kWiFiNetworkInterface), Status::InvalidCommand);

        // Reserved bits must be clear.
        VerifyOrReturnError((commandData.wiFiBands.Value().Raw() & ~kValidWiFiBandBits) == 0, Status::InvalidCommand);

        wiFiBands = static_cast<WiFiBandBitmap>(commandData.wiFiBands.Value().Raw());
    }

    chip::FabricIndex fabricIndex = request.subjectDescriptor.fabricIndex;
    chip::NodeId nodeId           = request.subjectDescriptor.subject;

    auto delegateStatus = mDelegate.ProxyBackgroundScanStopRequest(
        commandData.transport, wiFiBands, fabricIndex, nodeId);
    ReturnErrorOnFailure(DataModel::ActionReturnStatus(delegateStatus).GetUnderlyingError());

    return Status::Success;
}

DataModel::ActionReturnStatus CommissioningProxyCluster::HandleProxyMessageRequest(const DataModel::InvokeRequest & request,
                                                                                               TLV::TLVReader & input_arguments,
                                                                                               CommandHandler * handler)
{
    Commands::ProxyMessageRequest::DecodableType commandData;
    ReturnErrorOnFailure(DataModel::Decode(input_arguments, commandData));

    chip::Optional<chip::ByteSpan> message;
    if (!commandData.message.IsNull())
    {
        message.SetValue(commandData.message.Value());
    }

    auto delegateStatus = mDelegate.ProxyMessageRequest(
        commandData.sessionId, message, commandData.responseTimeout, handler, request);

    ReturnErrorOnFailure(DataModel::ActionReturnStatus(delegateStatus).GetUnderlyingError());
    return Status::Success;
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

    AttributeListBuilder listBuilder(builder);

    return listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes), mEnabledOptionalAttributes);
}

CHIP_ERROR CommissioningProxyCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                           ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    ReturnErrorOnFailure(builder.AppendElements({
        Commands::ProxyConnectRequest::kMetadataEntry,
        Commands::ProxyDisconnectRequest::kMetadataEntry,
        Commands::ProxyScanRequest::kMetadataEntry,
        Commands::ProxyMessageRequest::kMetadataEntry
    }));

    if (mFeatureFlags.Has(Feature::kBackgroundScan))
    {
        ReturnErrorOnFailure(builder.AppendElements({
            Commands::ProxyBackGroundScanStartRequest::kMetadataEntry,
            Commands::ProxyBackGroundScanStopRequest::kMetadataEntry,
        }));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissioningProxyCluster::SetCPState(CommissioningProxyCluster::State_t state)
{
    mMainCommissioningProxyState = state;
    return CHIP_NO_ERROR;
}

CommissioningProxyCluster::State_t CommissioningProxyCluster::GetCPState(void)
{
    return mMainCommissioningProxyState;
}


} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
