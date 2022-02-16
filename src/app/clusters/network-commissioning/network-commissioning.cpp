/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "network-commissioning.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandlerInterface.h>
#include <app/InteractionModelEngine.h>
#include <app/util/attribute-storage.h>
#include <lib/support/SafeInt.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <platform/DeviceControlServer.h>
#include <platform/PlatformManager.h>
#include <platform/internal/DeviceNetworkInfo.h>

namespace chip {
namespace app {
namespace Clusters {
namespace NetworkCommissioning {

using namespace DeviceLayer::NetworkCommissioning;

namespace {
// For WiFi and Thread scan results, each item will cose ~60 bytes in TLV, thus 15 is a safe upper bound of scan results.
constexpr size_t kMaxNetworksInScanResponse = 15;

NetworkCommissioningStatus ToClusterObjectEnum(Status status)
{
    // clang-format off
    static_assert(to_underlying(NetworkCommissioningStatus::kSuccess               ) == to_underlying(Status::kSuccess               ), "kSuccess value mismatch.");
    static_assert(to_underlying(NetworkCommissioningStatus::kOutOfRange            ) == to_underlying(Status::kOutOfRange            ), "kOutOfRange value mismatch.");
    static_assert(to_underlying(NetworkCommissioningStatus::kBoundsExceeded        ) == to_underlying(Status::kBoundsExceeded        ), "kBoundsExceeded value mismatch.");
    static_assert(to_underlying(NetworkCommissioningStatus::kNetworkIDNotFound     ) == to_underlying(Status::kNetworkIDNotFound     ), "kNetworkIDNotFound value mismatch.");
    static_assert(to_underlying(NetworkCommissioningStatus::kDuplicateNetworkID    ) == to_underlying(Status::kDuplicateNetworkID    ), "kDuplicateNetworkID value mismatch.");
    static_assert(to_underlying(NetworkCommissioningStatus::kNetworkNotFound       ) == to_underlying(Status::kNetworkNotFound       ), "kNetworkNotFound value mismatch.");
    static_assert(to_underlying(NetworkCommissioningStatus::kRegulatoryError       ) == to_underlying(Status::kRegulatoryError       ), "kRegulatoryError value mismatch.");
    static_assert(to_underlying(NetworkCommissioningStatus::kAuthFailure           ) == to_underlying(Status::kAuthFailure           ), "kAuthFailure value mismatch.");
    static_assert(to_underlying(NetworkCommissioningStatus::kUnsupportedSecurity   ) == to_underlying(Status::kUnsupportedSecurity   ), "kUnsupportedSecurity value mismatch.");
    static_assert(to_underlying(NetworkCommissioningStatus::kOtherConnectionFailure) == to_underlying(Status::kOtherConnectionFailure), "kOtherConnectionFailure value mismatch.");
    static_assert(to_underlying(NetworkCommissioningStatus::kIPV6Failed            ) == to_underlying(Status::kIPV6Failed            ), "kIPV6Failed value mismatch.");
    static_assert(to_underlying(NetworkCommissioningStatus::kIPBindFailed          ) == to_underlying(Status::kIPBindFailed          ), "kIPBindFailed value mismatch.");
    static_assert(to_underlying(NetworkCommissioningStatus::kUnknownError          ) == to_underlying(Status::kUnknownError          ), "kUnknownError value mismatch.");
    // clang-format on
    return static_cast<NetworkCommissioningStatus>(to_underlying(status));
}

NetworkCommissioning::WiFiBand ToClusterObjectEnum(DeviceLayer::NetworkCommissioning::WiFiBand band)
{
    using ClusterObject     = NetworkCommissioning::WiFiBand;
    using PlatfromInterface = DeviceLayer::NetworkCommissioning::WiFiBand;

    static_assert(to_underlying(ClusterObject::k2g4) == to_underlying(PlatfromInterface::k2g4), "k2g4 value mismatch.");
    static_assert(to_underlying(ClusterObject::k3g65) == to_underlying(PlatfromInterface::k3g65), "k3g65 value mismatch.");
    static_assert(to_underlying(ClusterObject::k5g) == to_underlying(PlatfromInterface::k5g), "k5g value mismatch.");
    static_assert(to_underlying(ClusterObject::k6g) == to_underlying(PlatfromInterface::k6g), "k6g value mismatch.");
    static_assert(to_underlying(ClusterObject::k60g) == to_underlying(PlatfromInterface::k60g), "k60g value mismatch.");

    return static_cast<ClusterObject>(to_underlying(band));
}

} // namespace

CHIP_ERROR Instance::Init()
{
    ReturnErrorOnFailure(chip::app::InteractionModelEngine::GetInstance()->RegisterCommandHandler(this));
    VerifyOrReturnError(registerAttributeAccessOverride(this), CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(
        DeviceLayer::PlatformMgrImpl().AddEventHandler(_OnCommissioningComplete, reinterpret_cast<intptr_t>(this)));
    ReturnErrorOnFailure(mpBaseDriver->Init());
    mLastNetworkingStatusValue.SetNull();
    mLastConnectErrorValue.SetNull();
    mLastNetworkIDLen = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::Shutdown()
{
    ReturnErrorOnFailure(mpBaseDriver->Shutdown());
    return CHIP_NO_ERROR;
}

void Instance::InvokeCommand(HandlerContext & ctxt)
{
    if (mAsyncCommandHandle.Get() != nullptr)
    {
        // We have a command processing in the backend, reject all incoming commands.
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Busy);
        ctxt.SetCommandHandled();
        return;
    }

    // Since mPath is used for building the response command, and we have checked that we are not pending the response of another
    // command above. So it is safe to set the mPath here and not clear it when return.
    mPath = ctxt.mRequestPath;

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::ScanNetworks::Id:
        VerifyOrReturn(mFeatureFlags.Has(NetworkCommissioningFeature::kWiFiNetworkInterface) ||
                       mFeatureFlags.Has(NetworkCommissioningFeature::kThreadNetworkInterface));
        HandleCommand<Commands::ScanNetworks::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleScanNetworks(ctx, req); });
        return;

    case Commands::AddOrUpdateWiFiNetwork::Id:
        VerifyOrReturn(mFeatureFlags.Has(NetworkCommissioningFeature::kWiFiNetworkInterface));
        HandleCommand<Commands::AddOrUpdateWiFiNetwork::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleAddOrUpdateWiFiNetwork(ctx, req); });
        return;

    case Commands::AddOrUpdateThreadNetwork::Id:
        VerifyOrReturn(mFeatureFlags.Has(NetworkCommissioningFeature::kThreadNetworkInterface));
        HandleCommand<Commands::AddOrUpdateThreadNetwork::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleAddOrUpdateThreadNetwork(ctx, req); });
        return;

    case Commands::RemoveNetwork::Id:
        VerifyOrReturn(mFeatureFlags.Has(NetworkCommissioningFeature::kWiFiNetworkInterface) ||
                       mFeatureFlags.Has(NetworkCommissioningFeature::kThreadNetworkInterface));
        HandleCommand<Commands::RemoveNetwork::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleRemoveNetwork(ctx, req); });
        return;

    case Commands::ConnectNetwork::Id:
        VerifyOrReturn(mFeatureFlags.Has(NetworkCommissioningFeature::kWiFiNetworkInterface) ||
                       mFeatureFlags.Has(NetworkCommissioningFeature::kThreadNetworkInterface));
        HandleCommand<Commands::ConnectNetwork::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleConnectNetwork(ctx, req); });
        return;

    case Commands::ReorderNetwork::Id:
        VerifyOrReturn(mFeatureFlags.Has(NetworkCommissioningFeature::kWiFiNetworkInterface) ||
                       mFeatureFlags.Has(NetworkCommissioningFeature::kThreadNetworkInterface));
        HandleCommand<Commands::ReorderNetwork::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleReorderNetwork(ctx, req); });
        return;
    }
}

CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case Attributes::MaxNetworks::Id:
        return aEncoder.Encode(mpBaseDriver->GetMaxNetworks());

    case Attributes::Networks::Id:
        return aEncoder.EncodeList([this](const auto & encoder) {
            auto networks  = mpBaseDriver->GetNetworks();
            CHIP_ERROR err = CHIP_NO_ERROR;
            Structs::NetworkInfo::Type networkForEncode;
            NetworkCommissioning::Network network;
            for (; networks != nullptr && networks->Next(network);)
            {
                networkForEncode.networkID = ByteSpan(network.networkID, network.networkIDLen);
                networkForEncode.connected = network.connected;
                SuccessOrExit(err = encoder.Encode(networkForEncode));
            }
        exit:
            if (networks != nullptr)
            {
                networks->Release();
            }

            return err;
        });

    case Attributes::ScanMaxTimeSeconds::Id:
        if (mpWirelessDriver != nullptr)
        {
            return aEncoder.Encode(mpWirelessDriver->GetScanNetworkTimeoutSeconds());
        }
        return CHIP_NO_ERROR;

    case Attributes::ConnectMaxTimeSeconds::Id:
        if (mpWirelessDriver != nullptr)
        {
            return aEncoder.Encode(mpWirelessDriver->GetConnectNetworkTimeoutSeconds());
        }
        return CHIP_NO_ERROR;

    case Attributes::InterfaceEnabled::Id:
        return aEncoder.Encode(mpBaseDriver->GetEnabled());

    case Attributes::LastNetworkingStatus::Id:
        return aEncoder.Encode(mLastNetworkingStatusValue);

    case Attributes::LastNetworkID::Id:
        if (mLastNetworkIDLen == 0)
        {
            return aEncoder.EncodeNull();
        }
        else
        {
            return aEncoder.Encode(ByteSpan(mLastNetworkID, mLastNetworkIDLen));
        }

    case Attributes::LastConnectErrorValue::Id:
        return aEncoder.Encode(mLastConnectErrorValue);

    case Attributes::FeatureMap::Id:
        return aEncoder.Encode(mFeatureFlags);

    default:
        return CHIP_NO_ERROR;
    }
}

CHIP_ERROR Instance::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    switch (aPath.mAttributeId)
    {
    case Attributes::InterfaceEnabled::Id:
        bool value;
        ReturnErrorOnFailure(aDecoder.Decode(value));
        return mpBaseDriver->SetEnabled(value);
    default:
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
}

void Instance::HandleScanNetworks(HandlerContext & ctx, const Commands::ScanNetworks::DecodableType & req)
{

    if (mFeatureFlags.Has(NetworkCommissioningFeature::kWiFiNetworkInterface))
    {
        mAsyncCommandHandle = app::CommandHandler::Handle(&ctx.mCommandHandler);
        mpDriver.Get<WiFiDriver *>()->ScanNetworks(req.ssid, this);
    }
    else if (mFeatureFlags.Has(NetworkCommissioningFeature::kThreadNetworkInterface))
    {
        mAsyncCommandHandle = app::CommandHandler::Handle(&ctx.mCommandHandler);
        mpDriver.Get<ThreadDriver *>()->ScanNetworks(this);
    }
    else
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
}

void Instance::HandleAddOrUpdateWiFiNetwork(HandlerContext & ctx, const Commands::AddOrUpdateWiFiNetwork::DecodableType & req)
{
    Commands::NetworkConfigResponse::Type response;
    response.networkingStatus = ToClusterObjectEnum(mpDriver.Get<WiFiDriver *>()->AddOrUpdateNetwork(req.ssid, req.credentials));
    ctx.mCommandHandler.AddResponseData(ctx.mRequestPath, response);
}

void Instance::HandleAddOrUpdateThreadNetwork(HandlerContext & ctx, const Commands::AddOrUpdateThreadNetwork::DecodableType & req)
{
    Commands::NetworkConfigResponse::Type response;
    response.networkingStatus = ToClusterObjectEnum(mpDriver.Get<ThreadDriver *>()->AddOrUpdateNetwork(req.operationalDataset));
    ctx.mCommandHandler.AddResponseData(ctx.mRequestPath, response);
}

void Instance::HandleRemoveNetwork(HandlerContext & ctx, const Commands::RemoveNetwork::DecodableType & req)
{
    Commands::NetworkConfigResponse::Type response;
    response.networkingStatus = ToClusterObjectEnum(mpWirelessDriver->RemoveNetwork(req.networkID));
    ctx.mCommandHandler.AddResponseData(ctx.mRequestPath, response);
}

void Instance::HandleConnectNetwork(HandlerContext & ctx, const Commands::ConnectNetwork::DecodableType & req)
{
    if (req.networkID.size() > DeviceLayer::NetworkCommissioning::kMaxNetworkIDLen)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::InvalidValue);
        return;
    }

    mConnectingNetworkIDLen = static_cast<uint8_t>(req.networkID.size());
    memcpy(mConnectingNetworkID, req.networkID.data(), mConnectingNetworkIDLen);

    mAsyncCommandHandle = app::CommandHandler::Handle(&ctx.mCommandHandler);
    mpWirelessDriver->ConnectNetwork(req.networkID, this);
}

void Instance::HandleReorderNetwork(HandlerContext & ctx, const Commands::ReorderNetwork::DecodableType & req)
{
    Commands::NetworkConfigResponse::Type response;
    response.networkingStatus = ToClusterObjectEnum(mpWirelessDriver->ReorderNetwork(req.networkID, req.networkIndex));
    ctx.mCommandHandler.AddResponseData(ctx.mRequestPath, response);
}

void Instance::OnResult(Status commissioningError, CharSpan errorText, int32_t interfaceStatus)
{
    auto commandHandleRef = std::move(mAsyncCommandHandle);
    auto commandHandle    = commandHandleRef.Get();
    if (commandHandle == nullptr)
    {
        // When the platform shutted down, interaction model engine will invalidate all commandHandle to avoid dangling references.
        // We may receive the callback after it and should make it noop.
        return;
    }

    Commands::ConnectNetworkResponse::Type response;
    response.networkingStatus = ToClusterObjectEnum(commissioningError);
    response.debugText        = errorText;
    response.errorValue       = interfaceStatus;
    commandHandle->AddResponseData(mPath, response);

    mLastNetworkIDLen = mConnectingNetworkIDLen;
    memcpy(mLastNetworkID, mConnectingNetworkID, mLastNetworkIDLen);
    mLastNetworkingStatusValue.SetNonNull(ToClusterObjectEnum(commissioningError));
    mLastConnectErrorValue.SetNonNull(interfaceStatus);

    if (commissioningError == Status::kSuccess)
    {
        // TODO: Pass the actual network id to device control server.
        DeviceLayer::DeviceControlServer::DeviceControlSvr().ConnectNetworkForOperational(ByteSpan());
    }
}

void Instance::OnFinished(Status status, CharSpan debugText, ThreadScanResponseIterator * networks)
{
    CHIP_ERROR err        = CHIP_NO_ERROR;
    auto commandHandleRef = std::move(mAsyncCommandHandle);
    auto commandHandle    = commandHandleRef.Get();
    if (commandHandle == nullptr)
    {
        // When the platform shutted down, interaction model engine will invalidate all commandHandle to avoid dangling references.
        // We may receive the callback after it and should make it noop.
        return;
    }

    mLastNetworkingStatusValue.SetNonNull(ToClusterObjectEnum(status));
    mLastConnectErrorValue.SetNull();
    mLastNetworkIDLen = 0;

    TLV::TLVWriter * writer;
    TLV::TLVType listContainerType;
    ThreadScanResponse scanResponse;
    size_t networksEncoded = 0;

    SuccessOrExit(err = commandHandle->PrepareCommand(
                      ConcreteCommandPath(mPath.mEndpointId, NetworkCommissioning::Id, Commands::ScanNetworksResponse::Id)));
    VerifyOrExit((writer = commandHandle->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    SuccessOrExit(err = writer->Put(TLV::ContextTag(to_underlying(Commands::ScanNetworksResponse::Fields::kNetworkingStatus)),
                                    ToClusterObjectEnum(status)));
    SuccessOrExit(err = DataModel::Encode(
                      *writer, TLV::ContextTag(to_underlying(Commands::ScanNetworksResponse::Fields::kDebugText)), debugText));
    SuccessOrExit(
        err = writer->StartContainer(TLV::ContextTag(to_underlying(Commands::ScanNetworksResponse::Fields::kThreadScanResults)),
                                     TLV::TLVType::kTLVType_Array, listContainerType));

    for (; networks != nullptr && networks->Next(scanResponse) && networksEncoded < kMaxNetworksInScanResponse; networksEncoded++)
    {
        Structs::ThreadInterfaceScanResult::Type result;
        result.panId           = scanResponse.panId;
        result.extendedPanId   = scanResponse.extendedPanId;
        result.networkName     = CharSpan(scanResponse.networkName, scanResponse.networkNameLen);
        result.channel         = scanResponse.channel;
        result.version         = scanResponse.version;
        result.extendedAddress = scanResponse.extendedAddress;
        result.rssi            = scanResponse.rssi;
        result.lqi             = scanResponse.lqi;
        SuccessOrExit(err = DataModel::Encode(*writer, TLV::AnonymousTag(), result));
    }

    SuccessOrExit(err = writer->EndContainer(listContainerType));
    SuccessOrExit(err = commandHandle->FinishCommand());

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response: %s", err.AsString());
    }
    networks->Release();
}

void Instance::OnFinished(Status status, CharSpan debugText, WiFiScanResponseIterator * networks)
{
    CHIP_ERROR err        = CHIP_NO_ERROR;
    auto commandHandleRef = std::move(mAsyncCommandHandle);
    auto commandHandle    = commandHandleRef.Get();
    if (commandHandle == nullptr)
    {
        // When the platform shutted down, interaction model engine will invalidate all commandHandle to avoid dangling references.
        // We may receive the callback after it and should make it noop.
        return;
    }

    mLastNetworkingStatusValue.SetNonNull(ToClusterObjectEnum(status));
    mLastConnectErrorValue.SetNull();
    mLastNetworkIDLen = 0;

    TLV::TLVWriter * writer;
    TLV::TLVType listContainerType;
    WiFiScanResponse scanResponse;
    size_t networksEncoded = 0;

    SuccessOrExit(err = commandHandle->PrepareCommand(
                      ConcreteCommandPath(mPath.mEndpointId, NetworkCommissioning::Id, Commands::ScanNetworksResponse::Id)));
    VerifyOrExit((writer = commandHandle->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    SuccessOrExit(err = writer->Put(TLV::ContextTag(to_underlying(Commands::ScanNetworksResponse::Fields::kNetworkingStatus)),
                                    ToClusterObjectEnum(status)));
    SuccessOrExit(err = DataModel::Encode(
                      *writer, TLV::ContextTag(to_underlying(Commands::ScanNetworksResponse::Fields::kDebugText)), debugText));
    SuccessOrExit(
        err = writer->StartContainer(TLV::ContextTag(to_underlying(Commands::ScanNetworksResponse::Fields::kWiFiScanResults)),
                                     TLV::TLVType::kTLVType_Array, listContainerType));

    for (; networks != nullptr && networks->Next(scanResponse) && networksEncoded < kMaxNetworksInScanResponse; networksEncoded++)
    {
        Structs::WiFiInterfaceScanResult::Type result;
        result.security = scanResponse.security;
        result.ssid     = ByteSpan(scanResponse.ssid, scanResponse.ssidLen);
        result.bssid    = ByteSpan(scanResponse.bssid, sizeof(scanResponse.bssid));
        result.channel  = scanResponse.channel;
        result.wiFiBand = ToClusterObjectEnum(scanResponse.wiFiBand);
        result.rssi     = scanResponse.rssi;
        SuccessOrExit(err = DataModel::Encode(*writer, TLV::AnonymousTag(), result));
    }

    SuccessOrExit(err = writer->EndContainer(listContainerType));
    SuccessOrExit(err = commandHandle->FinishCommand());

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response: %s", err.AsString());
    }
    if (networks != nullptr)
    {
        networks->Release();
    }
}

void Instance::_OnCommissioningComplete(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    Instance * this_ = reinterpret_cast<Instance *>(arg);
    VerifyOrReturn(event->Type == DeviceLayer::DeviceEventType::kCommissioningComplete);
    this_->OnCommissioningComplete(event->CommissioningComplete.status);
}

void Instance::OnCommissioningComplete(CHIP_ERROR err)
{
    VerifyOrReturn(mpWirelessDriver != nullptr);

    if (err == CHIP_NO_ERROR)
    {
        ChipLogDetail(Zcl, "Commissioning complete, notify platform driver to persist network credentails.");
        mpWirelessDriver->CommitConfiguration();
    }
    else
    {
        ChipLogDetail(Zcl, "Failsafe timeout, tell platform driver to revert network credentails.");
        mpWirelessDriver->RevertConfiguration();
    }
}

bool NullNetworkDriver::GetEnabled()
{
    // Disable the interface and it cannot be enabled since there are no physical interfaces.
    return false;
}

uint8_t NullNetworkDriver::GetMaxNetworks()
{
    // The minimal value of MaxNetworks should be 1 per spec.
    return 1;
}

DeviceLayer::NetworkCommissioning::NetworkIterator * NullNetworkDriver::GetNetworks()
{
    // Instance::Read accepts nullptr as an empty NetworkIterator.
    return nullptr;
}

} // namespace NetworkCommissioning
} // namespace Clusters
} // namespace app
} // namespace chip
