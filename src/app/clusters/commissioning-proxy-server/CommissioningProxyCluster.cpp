/*
 *
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

#include <app/clusters/commissioning-proxy-server/CommissioningProxyCluster.h>

#include <app/data-model/Decode.h>
#include <app/data-model/Encode.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/CommissioningProxy/Attributes.h>
#include <clusters/CommissioningProxy/Commands.h>
#include <clusters/CommissioningProxy/Metadata.h>
#include <clusters/CommissioningProxy/Structs.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeContext.h>
#include <platform/CommissionableDataProvider.h> // for kMaxDiscriminatorValue
#include <protocols/interaction_model/StatusCode.h>
#include <system/SystemClock.h>
#include <system/SystemPacketBuffer.h>

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
constexpr uint8_t kValidTransportBits = static_cast<uint8_t>(CapabilitiesBitmap::kBle) |
    static_cast<uint8_t>(CapabilitiesBitmap::kWiFiPAF) | static_cast<uint8_t>(CapabilitiesBitmap::kNtl);
constexpr uint16_t kValidWiFiBandBits = static_cast<uint16_t>(WiFiBandBitmap::k2g4) | static_cast<uint16_t>(WiFiBandBitmap::k5g);

CHIP_ERROR CommissioningProxyCluster::Startup(ServerClusterContext & context)
{
    return DefaultServerCluster::Startup(context);
}

DataModel::ActionReturnStatus CommissioningProxyCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                        AttributeValueDecoder & decoder)
{
    // These writable attributes are stored by the cluster so that change-reporting is
    // always the cluster's responsibility. Only emit a change report when the stored
    // value actually changes, to avoid redundant reports.
    switch (request.path.mAttributeId)
    {
    case ScanMaxTime::Id: {
        uint8_t time;
        ReturnErrorOnFailure(decoder.Decode(time));
        // Spec: ScanMaxTime has constraint "min 1"; reject 0 with ConstraintError.
        VerifyOrReturnError(time >= 1, Status::ConstraintError);
        // No change → no write, no report.
        VerifyOrReturnValue(time != mScanMaxTime, Status::Success);
        mScanMaxTime = time;
        break;
    }
    case CacheTimeout::Id: {
        uint16_t cacheTimeout;
        ReturnErrorOnFailure(decoder.Decode(cacheTimeout));
        // Spec: CacheTimeout has constraint "min 1"; reject 0 with ConstraintError.
        VerifyOrReturnError(cacheTimeout >= 1, Status::ConstraintError);
        // No change → no write, no report.
        VerifyOrReturnValue(cacheTimeout != mCacheTimeout, Status::Success);
        mCacheTimeout = cacheTimeout;
        break;
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedWrite;
    }

    NotifyAttributeChanged(request.path.mAttributeId);
    return Status::Success;
}

DataModel::ActionReturnStatus CommissioningProxyCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                       AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Globals::Attributes::ClusterRevision::Id:
        // Per spec ClusterRevision SHALL be the highest revision number in the
        // Revision History table; sourced from the cluster's generated Metadata
        // (CommissioningProxy::kRevision in Metadata.h).
        return encoder.Encode(kRevision);

    case FeatureMap::Id:
        return encoder.Encode(mFeatureFlags);

    case Transport::Id:
        return encoder.Encode(GetSupportedTransports());

    case MaxSessions::Id:
        return encoder.Encode(mMaxSessions);

    case ScanMaxTime::Id:
        return encoder.Encode(mScanMaxTime);

    case MaxCachedResults::Id:
        return encoder.Encode(mMaxCachedResults);

    case NumCachedResults::Id:
        return encoder.Encode(mScanCache.Count());

    case CacheTimeout::Id:
        return encoder.Encode(mCacheTimeout);

    case CachedResults::Id:
        return mScanCache.Encode(encoder);

    case WiFiBand::Id:
        return encoder.Encode(mSupportedWiFiBands);

    default:
        return Status::UnsupportedAttribute;
    }
}

chip::BitMask<CapabilitiesBitmap> CommissioningProxyCluster::GetSupportedTransports() const
{
    // A transport is supported iff a driver is registered for it.
    chip::BitMask<CapabilitiesBitmap> supported;
    for (size_t i = 0; i < mTransportCount; i++)
    {
        supported.Set(mTransports[i]->GetTransportType());
    }
    return supported;
}

CommissioningProxyTransport * CommissioningProxyCluster::FindTransport(CapabilitiesBitmap bit) const
{
    for (size_t i = 0; i < mTransportCount; i++)
    {
        if (mTransports[i]->GetTransportType() == bit)
        {
            return mTransports[i];
        }
    }
    return nullptr;
}

std::optional<DataModel::ActionReturnStatus> CommissioningProxyCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                      TLV::TLVReader & input_arguments,
                                                                                      CommandHandler * handler)
{
    using namespace Commands;

    // Each handler returns its own std::optional result: std::nullopt when the
    // delegate has taken ownership of the response (sync or async AddResponse) so
    // the framework must not add a duplicate status, or a concrete status otherwise.
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

std::optional<DataModel::ActionReturnStatus>
CommissioningProxyCluster::HandleProxyConnectRequest(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
                                                     CommandHandler * handler)
{
    Commands::ProxyConnectRequest::DecodableType commandData;
    ReturnErrorOnFailure(commandData.Decode(input_arguments, request.GetAccessingFabricIndex()));

    // Only a single transport SHALL be selected per spec
    VerifyOrReturnError(HasExactlyOneBitSet(commandData.transport.Raw()), Status::InvalidCommand);

    // Spec: Discriminator field constraint is "0 to 4095" (12-bit); any other
    // field being invalid SHALL return InvalidCommand. Matches the validation in
    // AdministratorCommissioning / JointFabricAdministrator.
    VerifyOrReturnError(commandData.discriminator <= kMaxDiscriminatorValue, Status::InvalidCommand);

    // The selected transport must be supported by this proxy instance.
    if (!GetSupportedTransports().HasAny(commandData.transport))
    {
        ChipLogError(Zcl, "Commissioning Proxy: requested transport not supported by this instance");
        return Status::InvalidTransportType;
    }

    // WiFiBand is meaningful only with the WiFiPAF transport per spec; if the
    // selected transport is not kWiFiPAF, the field must be absent.
    if (commandData.wiFiBand.HasValue() && commandData.transport != CapabilitiesBitmap::kWiFiPAF)
    {
        ChipLogError(Zcl, "Commissioning Proxy: WiFiBand provided with non-WiFiPAF transport");
        return Status::InvalidCommand;
    }

    // WiFiBand is only valid when the WI feature is enabled per spec.
    if (commandData.wiFiBand.HasValue())
    {
        VerifyOrReturnError(mFeatureFlags.Has(Feature::kWiFiNetworkInterface), Status::InvalidCommand);

        // WiFiBand must not contain reserved bits.
        VerifyOrReturnError((commandData.wiFiBand.Value().Raw() & ~kValidWiFiBandBits) == 0, Status::InvalidCommand);

        // WiFiBand must be a subset of the bands supported by this proxy.
        if ((commandData.wiFiBand.Value().Raw() & ~mSupportedWiFiBands.Raw()) != 0)
        {
            ChipLogError(Zcl, "CommissioningProxy: Requested WiFiBand not in supported bands");
            return Status::InvalidTransportType;
        }
    }

    // Spec: "If MaxSessions are in use, a RESOURCE_EXHAUSTED status SHALL be
    // returned."  Enforced here so the check covers established sessions and any
    // in-flight connect across all transports (GetActiveSessionCount()).
    if (GetActiveSessionCount() >= mMaxSessions)
    {
        ChipLogError(Zcl, "Commissioning Proxy: MaxSessions reached (%u/%u)", GetActiveSessionCount(), mMaxSessions);
        return Status::ResourceExhausted;
    }

    // The transport driver establishes the connection, allocates/registers a session
    // via Sessions(), and calls handler->AddResponse() with the ProxyConnectResponse
    // (sync or async). The state transition to kState_CPConnected happens in the
    // driver's success path once the transport is actually up.
    CommissioningProxyTransport * transport = FindTransport(static_cast<CapabilitiesBitmap>(commandData.transport.Raw()));
    VerifyOrReturnValue(transport != nullptr, Status::InvalidTransportType);

    auto status = transport->Connect(handler, request, commandData.discriminator, commandData.timeout);

    // On error, surface the status; on success the driver owns the response, so
    // return nullopt to avoid a duplicate framework status.
    VerifyOrReturnValue(status == Status::Success, DataModel::ActionReturnStatus(status));
    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus>
CommissioningProxyCluster::HandleProxyDisconnectRequest(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
                                                        CommandHandler * handler)
{
    Commands::ProxyDisconnectRequest::DecodableType commandData;
    ReturnErrorOnFailure(commandData.Decode(input_arguments, request.GetAccessingFabricIndex()));

    // A null SessionID SHALL cancel any ongoing ProxyConnectRequest for the invoking fabric, not disconnect a session.
    if (commandData.sessionID.IsNull())
    {
        ChipLogProgress(Zcl, "HandleProxyDisconnectRequest: sessionID=null (cancel pending connect)");
        return CancelPendingConnect(request.subjectDescriptor.fabricIndex);
    }

    const uint16_t sessionId = commandData.sessionID.Value();
    ChipLogProgress(Zcl, "HandleProxyDisconnectRequest: sessionID=0x%04x", sessionId);

    auto info = mSessions.Find(sessionId);
    VerifyOrReturnValue(info.has_value(), Status::NotFound);
    // Per spec, a session may only be disconnected by the fabric that owns it.
    VerifyOrReturnValue(request.subjectDescriptor.fabricIndex == info->fabricIndex, Status::NotFound);

    const CapabilitiesBitmap sessTransport = info->transport;
    if (CommissioningProxyTransport * transport = FindTransport(sessTransport))
    {
        auto s = transport->Disconnect(sessionId);
        // On failure, leave the session intact and surface the error.
        VerifyOrReturnValue(s == Status::Success, DataModel::ActionReturnStatus(s));
    }

    // Drop the session record (and any pending ProxyMessage, without answering it).
    mSessions.RemoveSession(sessionId);

    // With MaxSessions > 1 several sessions may be open at once. Only transition the
    // cluster back to disconnected, and notify transports, once the last session is
    // gone; otherwise the proxy would report disconnected while sessions are active.
    if (GetActiveSessionCount() == 0)
    {
        for (size_t i = 0; i < mTransportCount; i++)
        {
            mTransports[i]->OnAllSessionsClosed();
        }

        CHIP_ERROR stateErr = SetCPState(kState_CPDisconnected);
        if (stateErr != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "HandleProxyDisconnectRequest: SetCPState failed: %" CHIP_ERROR_FORMAT, stateErr.Format());
        }
    }

    return Status::Success;
}

std::optional<DataModel::ActionReturnStatus>
CommissioningProxyCluster::HandleProxyScanRequest(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
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

    if (commandData.wiFiBands.HasValue())
    {
        // WiFiBands field is only valid when the WI feature is enabled.
        VerifyOrReturnError(mFeatureFlags.Has(Feature::kWiFiNetworkInterface), Status::InvalidCommand);

        // WiFiBands must not contain reserved bits.
        VerifyOrReturnError((commandData.wiFiBands.Value().Raw() & ~kValidWiFiBandBits) == 0, Status::InvalidCommand);

        // WiFiBands must be a subset of the bands supported by this proxy.
        if ((commandData.wiFiBands.Value().Raw() & ~mSupportedWiFiBands.Raw()) != 0)
        {
            ChipLogError(Zcl, "CommissioningProxy: Requested WiFiBand not in supported bands");
            return Status::InvalidTransportType;
        }
    }

    // A ProxyScanRequest MAY select multiple transports: scan each requested,
    // registered transport in parallel and aggregate into one ProxyScanResponse.
    if (mScanAggregator.InProgress())
    {
        return Status::Busy;
    }

    const uint8_t scanMaxTime = mScanMaxTime;
    mScanAggregator.Begin(handler, request.path, scanMaxTime);

    // First non-success from a sub-scan that could not start; only returned if no
    // sub-scan starts at all.
    auto firstError = Status::Success;
    for (size_t i = 0; i < mTransportCount; i++)
    {
        CommissioningProxyTransport * transport = mTransports[i];
        if (!commandData.transport.Has(transport->GetTransportType()))
        {
            continue;
        }
        auto s = transport->Scan(scanMaxTime);
        if (s == Status::Success)
        {
            mScanAggregator.AddPendingContributor();
        }
        else if (firstError == Status::Success)
        {
            firstError = s;
        }
    }

    if (mScanAggregator.PendingContributors() == 0)
    {
        ChipLogError(Zcl, "CommissioningProxy: no requested transport scan could be started");
        mScanAggregator.Abort();
        return (firstError == Status::Success) ? DataModel::ActionReturnStatus(Status::Failure)
                                               : DataModel::ActionReturnStatus(firstError);
    }

    // The aggregator owns the exchange now; keep it alive for the full scan window.
    handler->FlushAcksRightAwayOnSlowCommand();
    if (auto * exchange = handler->GetExchangeContext())
    {
        exchange->SetResponseTimeout(System::Clock::Seconds16(static_cast<uint16_t>(scanMaxTime) + 5));
    }

    // If every started sub-scan already reported synchronously, emit now.
    mScanAggregator.MaybeEmitIfComplete();

    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus>
CommissioningProxyCluster::HandleProxyBackGroundScanStartRequest(const DataModel::InvokeRequest & request,
                                                                 TLV::TLVReader & input_arguments, CommandHandler * handler)
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

    chip::BitMask<WiFiBandBitmap> wiFiBands;
    if (commandData.wiFiBands.HasValue())
    {
        // WiFiBands field is only valid when the WI feature is enabled.
        VerifyOrReturnError(mFeatureFlags.Has(Feature::kWiFiNetworkInterface), Status::InvalidCommand);

        // WiFiBands must not contain reserved bits.
        VerifyOrReturnError((commandData.wiFiBands.Value().Raw() & ~kValidWiFiBandBits) == 0, Status::InvalidCommand);

        // WiFiBands must be a subset of the bands supported by this proxy.
        if ((commandData.wiFiBands.Value().Raw() & ~mSupportedWiFiBands.Raw()) != 0)
        {
            ChipLogError(Zcl, "CommissioningProxy: Requested WiFiBand not in supported bands");
            return Status::InvalidTransportType;
        }

        wiFiBands = commandData.wiFiBands.Value();
    }

    const FabricIndex fabricIndex = request.subjectDescriptor.fabricIndex;
    const NodeId nodeId           = request.subjectDescriptor.subject;

    // A background scan MAY select multiple transports: start each requested,
    // registered transport with its own transport-local per-fabric record.
    auto result = Status::Success;
    for (size_t i = 0; i < mTransportCount; i++)
    {
        CommissioningProxyTransport * transport = mTransports[i];
        if (!commandData.transport.Has(transport->GetTransportType()))
        {
            continue;
        }
        auto s = transport->BgScanStart(commandData.timeout, wiFiBands, fabricIndex, nodeId);
        if (s != Status::Success && result == Status::Success)
        {
            result = s;
        }
    }

    return result;
}

std::optional<DataModel::ActionReturnStatus>
CommissioningProxyCluster::HandleProxyBackGroundScanStopRequest(const DataModel::InvokeRequest & request,
                                                                TLV::TLVReader & input_arguments, CommandHandler * handler)
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

    chip::BitMask<WiFiBandBitmap> wiFiBands;
    if (commandData.wiFiBands.HasValue())
    {
        // WiFiBands field is only valid when the WI feature is enabled.
        VerifyOrReturnError(mFeatureFlags.Has(Feature::kWiFiNetworkInterface), Status::InvalidCommand);

        // Reserved bits must be clear.
        VerifyOrReturnError((commandData.wiFiBands.Value().Raw() & ~kValidWiFiBandBits) == 0, Status::InvalidCommand);

        wiFiBands = commandData.wiFiBands.Value();
    }

    const FabricIndex fabricIndex = request.subjectDescriptor.fabricIndex;
    const NodeId nodeId           = request.subjectDescriptor.subject;

    // Fan the stop out to every registered transport with the full request mask;
    // each matches it against its own per-fabric record (a driver may partially
    // stop, e.g. one band of several) and returns NotFound if nothing matched.
    // NotFound is returned to the commissioner only if no transport matched at all.
    bool matched = false;
    auto err     = Status::Success;
    for (size_t i = 0; i < mTransportCount; i++)
    {
        auto s = mTransports[i]->BgScanStop(commandData.transport, wiFiBands, fabricIndex, nodeId);
        if (s != Status::NotFound)
        {
            matched = true;
            if (s != Status::Success)
            {
                err = s;
            }
        }
    }

    return matched ? err : Status::NotFound;
}

std::optional<DataModel::ActionReturnStatus>
CommissioningProxyCluster::HandleProxyMessageRequest(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
                                                     CommandHandler * handler)
{
    Commands::ProxyMessageRequest::DecodableType commandData;
    ReturnErrorOnFailure(commandData.Decode(input_arguments, request.GetAccessingFabricIndex()));

    const uint16_t sessionId = commandData.sessionID;

    auto info = mSessions.Find(sessionId);
    VerifyOrReturnValue(info.has_value(), Status::NotFound);
    // A session's messages may only be forwarded by the fabric that owns it.
    VerifyOrReturnValue(request.subjectDescriptor.fabricIndex == info->fabricIndex, Status::NotFound);

    // Per spec: a null/empty message is the Commissioner polling for a queued
    // response. Answer immediately with a null message.
    if (commandData.message.IsNull() || commandData.message.Value().empty())
    {
        Commands::ProxyMessageResponse::Type pollResponse;
        pollResponse.sessionID = sessionId;
        pollResponse.message.SetNull();
        handler->AddResponse(request.path, pollResponse);
        return std::nullopt;
    }

    CommissioningProxyTransport * transport = FindTransport(info->transport);
    VerifyOrReturnValue(transport != nullptr, Status::Failure);

    System::PacketBufferHandle buf =
        System::PacketBufferHandle::NewWithData(commandData.message.Value().data(), commandData.message.Value().size());
    VerifyOrReturnValue(!buf.IsNull(), Status::Failure);

    // Per spec ResponseTimeout=0: forward best-effort and answer immediately.
    if (commandData.responseTimeout == 0)
    {
        CHIP_ERROR sendErr = transport->SendMessage(sessionId, std::move(buf));
        if (sendErr != CHIP_NO_ERROR)
        {
            ChipLogDetail(Zcl, "ProxyMessageRequest(ResponseTimeout=0): SendMessage failed: %" CHIP_ERROR_FORMAT, sendErr.Format());
        }
        Commands::ProxyMessageResponse::Type immediate;
        immediate.sessionID = sessionId;
        immediate.message.SetNull();
        handler->AddResponse(request.path, immediate);
        return std::nullopt;
    }

    // Keep the exchange open until the commissionee replies (or the timeout fires).
    auto begin = mSessions.BeginMessage(sessionId, handler, request, commandData.responseTimeout);
    VerifyOrReturnValue(begin == Status::Success, DataModel::ActionReturnStatus(begin));

    CHIP_ERROR err = transport->SendMessage(sessionId, std::move(buf));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "ProxyMessageRequest: SendMessage failed: %" CHIP_ERROR_FORMAT, err.Format());
        mSessions.AbortPending(sessionId);
        return DataModel::ActionReturnStatus(Status::Failure);
    }

    // Reply is delivered asynchronously via Sessions().DispatchMessageResponse().
    return std::nullopt;
}

Protocols::InteractionModel::Status CommissioningProxyCluster::CancelPendingConnect(FabricIndex fabricIndex)
{
    // A null-SessionID ProxyDisconnectRequest cancels the invoking fabric's own
    // pending connect(s). Every transport is offered the cancel (no early return):
    // the spec says null cancels *any* ongoing connect for the fabric, and a foreign
    // connect on one transport must not mask this fabric's connect on another.
    //   Success        — a connect owned by this fabric was cancelled
    //   NotFound       — a connect is pending but owned by a different fabric
    //   InvalidInState — no connect pending on that transport
    bool cancelledOwn      = false;
    bool sawForeignPending = false;
    for (size_t i = 0; i < mTransportCount; i++)
    {
        auto s = mTransports[i]->CancelPendingConnect(fabricIndex);
        if (s == Status::Success)
        {
            cancelledOwn = true;
        }
        else if (s == Status::NotFound)
        {
            sawForeignPending = true;
        }
    }
    if (cancelledOwn)
    {
        return Status::Success;
    }
    return sawForeignPending ? Status::NotFound : Status::InvalidInState;
}

CHIP_ERROR CommissioningProxyCluster::Attributes(const ConcreteClusterPath & path,
                                                 ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    static constexpr DataModel::AttributeEntry optionalAttributes[] = {
        MaxCachedResults::kMetadataEntry, NumCachedResults::kMetadataEntry, CacheTimeout::kMetadataEntry,
        CachedResults::kMetadataEntry,    WiFiBand::kMetadataEntry,
    };

    AttributeListBuilder listBuilder(builder);

    return listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes), mEnabledOptionalAttributes);
}

CHIP_ERROR CommissioningProxyCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                       ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    ReturnErrorOnFailure(
        builder.AppendElements({ Commands::ProxyConnectRequest::kMetadataEntry, Commands::ProxyDisconnectRequest::kMetadataEntry,
                                 Commands::ProxyScanRequest::kMetadataEntry, Commands::ProxyMessageRequest::kMetadataEntry }));

    if (mFeatureFlags.Has(Feature::kBackgroundScan))
    {
        ReturnErrorOnFailure(builder.AppendElements({
            Commands::ProxyBackGroundScanStartRequest::kMetadataEntry,
            Commands::ProxyBackGroundScanStopRequest::kMetadataEntry,
        }));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissioningProxyCluster::GeneratedCommands(const ConcreteClusterPath & path,
                                                        ReadOnlyBufferBuilder<CommandId> & builder)
{
    // The three response commands the server can generate. All are mandatory and
    // independent of the feature flags (the BackgroundScan commands respond with a
    // plain status, not a generated command).
    return builder.AppendElements({
        Commands::ProxyConnectResponse::Id,
        Commands::ProxyScanResponse::Id,
        Commands::ProxyMessageResponse::Id,
    });
}

CHIP_ERROR CommissioningProxyCluster::SetCPState(CommissioningProxyCluster::State state)
{
    mMainCommissioningProxyState = state;
    return CHIP_NO_ERROR;
}

CommissioningProxyCluster::State CommissioningProxyCluster::GetCPState()
{
    return mMainCommissioningProxyState;
}

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
