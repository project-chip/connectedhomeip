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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandlerInterface.h>
#include <app/InteractionModelEngine.h>
#include <app/clusters/general-commissioning-server/general-commissioning-server.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <lib/support/SafeInt.h>
#include <lib/support/SortUtils.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <platform/DeviceControlServer.h>
#include <platform/PlatformManager.h>
#include <platform/internal/DeviceNetworkInfo.h>
#include <tracing/macros.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::NetworkCommissioning;

namespace chip {
namespace app {
namespace Clusters {
namespace NetworkCommissioning {

using namespace DeviceLayer::NetworkCommissioning;

namespace {
// For WiFi and Thread scan results, each item will cose ~60 bytes in TLV, thus 15 is a safe upper bound of scan results.
constexpr size_t kMaxNetworksInScanResponse = 15;

enum ValidWiFiCredentialLength
{
    kOpen      = 0,
    kWEP64     = 5,
    kMinWPAPSK = 8,
    kMaxWPAPSK = 63,
    kWPAPSKHex = 64,
};

} // namespace

CHIP_ERROR Instance::Init()
{
    ReturnErrorOnFailure(chip::app::InteractionModelEngine::GetInstance()->RegisterCommandHandler(this));
    VerifyOrReturnError(registerAttributeAccessOverride(this), CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(DeviceLayer::PlatformMgrImpl().AddEventHandler(OnPlatformEventHandler, reinterpret_cast<intptr_t>(this)));
    ReturnErrorOnFailure(mpBaseDriver->Init(this));
    mLastNetworkingStatusValue.SetNull();
    mLastConnectErrorValue.SetNull();
    mLastNetworkIDLen = 0;
    return CHIP_NO_ERROR;
}

void Instance::Shutdown()
{
    mpBaseDriver->Shutdown();
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
        VerifyOrReturn(mFeatureFlags.Has(Feature::kWiFiNetworkInterface) || mFeatureFlags.Has(Feature::kThreadNetworkInterface));
        HandleCommand<Commands::ScanNetworks::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleScanNetworks(ctx, req); });
        return;

    case Commands::AddOrUpdateWiFiNetwork::Id:
        VerifyOrReturn(mFeatureFlags.Has(Feature::kWiFiNetworkInterface));
        HandleCommand<Commands::AddOrUpdateWiFiNetwork::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleAddOrUpdateWiFiNetwork(ctx, req); });
        return;

    case Commands::AddOrUpdateThreadNetwork::Id:
        VerifyOrReturn(mFeatureFlags.Has(Feature::kThreadNetworkInterface));
        HandleCommand<Commands::AddOrUpdateThreadNetwork::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleAddOrUpdateThreadNetwork(ctx, req); });
        return;

    case Commands::RemoveNetwork::Id:
        VerifyOrReturn(mFeatureFlags.Has(Feature::kWiFiNetworkInterface) || mFeatureFlags.Has(Feature::kThreadNetworkInterface));
        HandleCommand<Commands::RemoveNetwork::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleRemoveNetwork(ctx, req); });
        return;

    case Commands::ConnectNetwork::Id:
        VerifyOrReturn(mFeatureFlags.Has(Feature::kWiFiNetworkInterface) || mFeatureFlags.Has(Feature::kThreadNetworkInterface));
        HandleCommand<Commands::ConnectNetwork::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleConnectNetwork(ctx, req); });
        return;

    case Commands::ReorderNetwork::Id:
        VerifyOrReturn(mFeatureFlags.Has(Feature::kWiFiNetworkInterface) || mFeatureFlags.Has(Feature::kThreadNetworkInterface));
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
            Structs::NetworkInfoStruct::Type networkForEncode;
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

void Instance::OnNetworkingStatusChange(NetworkCommissioning::Status aCommissioningError, Optional<ByteSpan> aNetworkId,
                                        Optional<int32_t> aConnectStatus)
{
    if (aNetworkId.HasValue() && aNetworkId.Value().size() > kMaxNetworkIDLen)
    {
        ChipLogError(DeviceLayer, "Invalid network id received when calling OnNetworkingStatusChange");
        return;
    }
    mLastNetworkingStatusValue.SetNonNull(aCommissioningError);
    if (aNetworkId.HasValue())
    {
        memcpy(mLastNetworkID, aNetworkId.Value().data(), aNetworkId.Value().size());
        mLastNetworkIDLen = static_cast<uint8_t>(aNetworkId.Value().size());
    }
    else
    {
        mLastNetworkIDLen = 0;
    }
    if (aConnectStatus.HasValue())
    {
        mLastConnectErrorValue.SetNonNull(aConnectStatus.Value());
    }
    else
    {
        mLastConnectErrorValue.SetNull();
    }
}

void Instance::HandleScanNetworks(HandlerContext & ctx, const Commands::ScanNetworks::DecodableType & req)
{
    MATTER_TRACE_SCOPE("HandleScanNetwork", "NetworkCommissioning");
    if (mFeatureFlags.Has(Feature::kWiFiNetworkInterface))
    {
        ByteSpan ssid;
        if (req.ssid.HasValue())
        {
            const auto & nullableSSID = req.ssid.Value();
            if (!nullableSSID.IsNull())
            {
                ssid = nullableSSID.Value();
                if (ssid.empty())
                {
                    // Normalize empty span value to null ByteSpan.
                    // Spec 7.17.1. Empty string is an equivalent of null.
                    ssid = ByteSpan();
                }
            }
        }
        if (ssid.size() > DeviceLayer::Internal::kMaxWiFiSSIDLength)
        {
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::InvalidCommand);
            return;
        }
        mCurrentOperationBreadcrumb = req.breadcrumb;
        mAsyncCommandHandle         = CommandHandler::Handle(&ctx.mCommandHandler);
        ctx.mCommandHandler.FlushAcksRightAwayOnSlowCommand();
        mpDriver.Get<WiFiDriver *>()->ScanNetworks(ssid, this);
    }
    else if (mFeatureFlags.Has(Feature::kThreadNetworkInterface))
    {
        mCurrentOperationBreadcrumb = req.breadcrumb;
        mAsyncCommandHandle         = CommandHandler::Handle(&ctx.mCommandHandler);
        ctx.mCommandHandler.FlushAcksRightAwayOnSlowCommand();
        mpDriver.Get<ThreadDriver *>()->ScanNetworks(this);
    }
    else
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
}

namespace {

void FillDebugTextAndNetworkIndex(Commands::NetworkConfigResponse::Type & response, MutableCharSpan debugText, uint8_t networkIndex)
{
    if (!debugText.empty())
    {
        response.debugText.SetValue(CharSpan(debugText.data(), debugText.size()));
    }
    if (response.networkingStatus == NetworkCommissioningStatusEnum::kSuccess)
    {
        response.networkIndex.SetValue(networkIndex);
    }
}

bool CheckFailSafeArmed(CommandHandlerInterface::HandlerContext & ctx)
{
    auto & failSafeContext = chip::Server::GetInstance().GetFailSafeContext();

    if (failSafeContext.IsFailSafeArmed(ctx.mCommandHandler.GetAccessingFabricIndex()))
    {
        return true;
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::FailsafeRequired);
    return false;
}

} // namespace

void Instance::HandleAddOrUpdateWiFiNetwork(HandlerContext & ctx, const Commands::AddOrUpdateWiFiNetwork::DecodableType & req)
{
    MATTER_TRACE_SCOPE("HandleAddOrUpdateWiFiNetwork", "NetworkCommissioning");

    VerifyOrReturn(CheckFailSafeArmed(ctx));

    // Spec 11.8.8.4
    // Valid Credentials length are:
    // - 0 bytes: Unsecured (open) connection
    // - 5 bytes: WEP-64 passphrase
    // - 10 hexadecimal ASCII characters: WEP-64 40-bit hex raw PSK
    // - 13 bytes: WEP-128 passphrase
    // - 26 hexadecimal ASCII characters: WEP-128 104-bit hex raw PSK
    // - 8..63 bytes: WPA/WPA2/WPA3 passphrase
    // - 64 bytes: WPA/WPA2/WPA3 raw hex PSK
    // Note 10 hex WEP64 and 13 bytes / 26 hex WEP128 passphrase are covered by 8~63 bytes WPA passphrase, so we don't check WEP64
    // hex and WEP128 passphrase.
    if (req.credentials.size() == ValidWiFiCredentialLength::kOpen || req.credentials.size() == ValidWiFiCredentialLength::kWEP64 ||
        (req.credentials.size() >= ValidWiFiCredentialLength::kMinWPAPSK &&
         req.credentials.size() <= ValidWiFiCredentialLength::kMaxWPAPSK))
    {
        // Valid length, the credentials can have any characters.
    }
    else if (req.credentials.size() == ValidWiFiCredentialLength::kWPAPSKHex)
    {
        for (size_t d = 0; d < req.credentials.size(); d++)
        {
            if (!isxdigit(req.credentials.data()[d]))
            {
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::InvalidCommand);
                return;
            }
        }
    }
    else
    {
        // Invalid length
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::InvalidCommand);
        return;
    }

    Commands::NetworkConfigResponse::Type response;
    MutableCharSpan debugText;
#if CHIP_CONFIG_NETWORK_COMMISSIONING_DEBUG_TEXT_BUFFER_SIZE
    char debugTextBuffer[CHIP_CONFIG_NETWORK_COMMISSIONING_DEBUG_TEXT_BUFFER_SIZE];
    debugText = MutableCharSpan(debugTextBuffer);
#endif
    uint8_t outNetworkIndex = 0;
    response.networkingStatus =
        mpDriver.Get<WiFiDriver *>()->AddOrUpdateNetwork(req.ssid, req.credentials, debugText, outNetworkIndex);
    FillDebugTextAndNetworkIndex(response, debugText, outNetworkIndex);
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    if (response.networkingStatus == NetworkCommissioningStatusEnum::kSuccess)
    {
        UpdateBreadcrumb(req.breadcrumb);
    }
}

void Instance::HandleAddOrUpdateThreadNetwork(HandlerContext & ctx, const Commands::AddOrUpdateThreadNetwork::DecodableType & req)
{
    MATTER_TRACE_SCOPE("HandleAddOrUpdateThreadNetwork", "NetworkCommissioning");

    VerifyOrReturn(CheckFailSafeArmed(ctx));

    Commands::NetworkConfigResponse::Type response;
    MutableCharSpan debugText;
#if CHIP_CONFIG_NETWORK_COMMISSIONING_DEBUG_TEXT_BUFFER_SIZE
    char debugTextBuffer[CHIP_CONFIG_NETWORK_COMMISSIONING_DEBUG_TEXT_BUFFER_SIZE];
    debugText = MutableCharSpan(debugTextBuffer);
#endif
    uint8_t outNetworkIndex = 0;
    response.networkingStatus =
        mpDriver.Get<ThreadDriver *>()->AddOrUpdateNetwork(req.operationalDataset, debugText, outNetworkIndex);
    FillDebugTextAndNetworkIndex(response, debugText, outNetworkIndex);
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    if (response.networkingStatus == NetworkCommissioningStatusEnum::kSuccess)
    {
        UpdateBreadcrumb(req.breadcrumb);
    }
}

void Instance::UpdateBreadcrumb(const Optional<uint64_t> & breadcrumb)
{
    VerifyOrReturn(breadcrumb.HasValue());
    GeneralCommissioning::SetBreadcrumb(breadcrumb.Value());
}

void Instance::CommitSavedBreadcrumb()
{
    // We rejected the command when there is another ongoing command, so mCurrentOperationBreadcrumb reflects the breadcrumb
    // argument in the only background command.
    UpdateBreadcrumb(mCurrentOperationBreadcrumb);
    mCurrentOperationBreadcrumb.ClearValue();
}

void Instance::HandleRemoveNetwork(HandlerContext & ctx, const Commands::RemoveNetwork::DecodableType & req)
{
    MATTER_TRACE_SCOPE("HandleRemoveNetwork", "NetworkCommissioning");

    VerifyOrReturn(CheckFailSafeArmed(ctx));

    Commands::NetworkConfigResponse::Type response;
    MutableCharSpan debugText;
#if CHIP_CONFIG_NETWORK_COMMISSIONING_DEBUG_TEXT_BUFFER_SIZE
    char debugTextBuffer[CHIP_CONFIG_NETWORK_COMMISSIONING_DEBUG_TEXT_BUFFER_SIZE];
    debugText = MutableCharSpan(debugTextBuffer);
#endif
    uint8_t outNetworkIndex   = 0;
    response.networkingStatus = mpWirelessDriver->RemoveNetwork(req.networkID, debugText, outNetworkIndex);
    FillDebugTextAndNetworkIndex(response, debugText, outNetworkIndex);
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    if (response.networkingStatus == NetworkCommissioningStatusEnum::kSuccess)
    {
        UpdateBreadcrumb(req.breadcrumb);
    }
}

void Instance::HandleConnectNetwork(HandlerContext & ctx, const Commands::ConnectNetwork::DecodableType & req)
{
    MATTER_TRACE_SCOPE("HandleConnectNetwork", "NetworkCommissioning");
    if (req.networkID.size() > DeviceLayer::NetworkCommissioning::kMaxNetworkIDLen)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::InvalidValue);
        return;
    }

    VerifyOrReturn(CheckFailSafeArmed(ctx));

    mConnectingNetworkIDLen = static_cast<uint8_t>(req.networkID.size());
    memcpy(mConnectingNetworkID, req.networkID.data(), mConnectingNetworkIDLen);
    mAsyncCommandHandle         = CommandHandler::Handle(&ctx.mCommandHandler);
    mCurrentOperationBreadcrumb = req.breadcrumb;
    mpWirelessDriver->ConnectNetwork(req.networkID, this);
}

void Instance::HandleReorderNetwork(HandlerContext & ctx, const Commands::ReorderNetwork::DecodableType & req)
{
    MATTER_TRACE_SCOPE("HandleReorderNetwork", "NetworkCommissioning");
    Commands::NetworkConfigResponse::Type response;
    MutableCharSpan debugText;
#if CHIP_CONFIG_NETWORK_COMMISSIONING_DEBUG_TEXT_BUFFER_SIZE
    char debugTextBuffer[CHIP_CONFIG_NETWORK_COMMISSIONING_DEBUG_TEXT_BUFFER_SIZE];
    debugText = MutableCharSpan(debugTextBuffer);
#endif
    response.networkingStatus = mpWirelessDriver->ReorderNetwork(req.networkID, req.networkIndex, debugText);
    FillDebugTextAndNetworkIndex(response, debugText, req.networkIndex);
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    if (response.networkingStatus == NetworkCommissioningStatusEnum::kSuccess)
    {
        UpdateBreadcrumb(req.breadcrumb);
    }
}

void Instance::OnResult(Status commissioningError, CharSpan debugText, int32_t interfaceStatus)
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
    response.networkingStatus = commissioningError;
    if (!debugText.empty())
    {
        response.debugText.SetValue(debugText);
    }
    if (commissioningError == Status::kSuccess)
    {
        DeviceLayer::DeviceControlServer::DeviceControlSvr().PostConnectedToOperationalNetworkEvent(
            ByteSpan(mLastNetworkID, mLastNetworkIDLen));
        mLastConnectErrorValue.SetNull();
    }
    else
    {
        response.errorValue.SetNonNull(interfaceStatus);
        mLastConnectErrorValue.SetNonNull(interfaceStatus);
    }

    mLastNetworkIDLen = mConnectingNetworkIDLen;
    memcpy(mLastNetworkID, mConnectingNetworkID, mLastNetworkIDLen);
    mLastNetworkingStatusValue.SetNonNull(commissioningError);

    commandHandle->AddResponse(mPath, response);
    if (commissioningError == NetworkCommissioningStatusEnum::kSuccess)
    {
        CommitSavedBreadcrumb();
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

    mLastNetworkingStatusValue.SetNonNull(status);
    mLastConnectErrorValue.SetNull();
    mLastNetworkIDLen = 0;

    TLV::TLVWriter * writer;
    TLV::TLVType listContainerType;
    ThreadScanResponse scanResponse;
    chip::Platform::ScopedMemoryBuffer<ThreadScanResponse> scanResponseArray;
    size_t scanResponseArrayLength = 0;
    uint8_t extendedAddressBuffer[Thread::kSizeExtendedPanId];

    SuccessOrExit(err = commandHandle->PrepareCommand(
                      ConcreteCommandPath(mPath.mEndpointId, NetworkCommissioning::Id, Commands::ScanNetworksResponse::Id)));
    VerifyOrExit((writer = commandHandle->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    SuccessOrExit(err = writer->Put(TLV::ContextTag(Commands::ScanNetworksResponse::Fields::kNetworkingStatus), status));
    if (debugText.size() != 0)
    {
        SuccessOrExit(
            err = DataModel::Encode(*writer, TLV::ContextTag(Commands::ScanNetworksResponse::Fields::kDebugText), debugText));
    }
    SuccessOrExit(err = writer->StartContainer(TLV::ContextTag(Commands::ScanNetworksResponse::Fields::kThreadScanResults),
                                               TLV::TLVType::kTLVType_Array, listContainerType));

    // If no network was found, we encode an empty list, don't call a zero-sized alloc.
    if (networks->Count() > 0)
    {
        VerifyOrExit(scanResponseArray.Alloc(chip::min(networks->Count(), kMaxNetworksInScanResponse)), err = CHIP_ERROR_NO_MEMORY);
        for (; networks != nullptr && networks->Next(scanResponse);)
        {
            if ((scanResponseArrayLength == kMaxNetworksInScanResponse) &&
                (scanResponseArray[scanResponseArrayLength - 1].rssi > scanResponse.rssi))
            {
                continue;
            }

            bool isDuplicated = false;

            for (size_t i = 0; i < scanResponseArrayLength; i++)
            {
                if ((scanResponseArray[i].panId == scanResponse.panId) &&
                    (scanResponseArray[i].extendedPanId == scanResponse.extendedPanId))
                {
                    if (scanResponseArray[i].rssi < scanResponse.rssi)
                    {
                        scanResponseArray[i] = scanResponseArray[--scanResponseArrayLength];
                    }
                    else
                    {
                        isDuplicated = true;
                    }
                    break;
                }
            }

            if (isDuplicated)
            {
                continue;
            }

            if (scanResponseArrayLength < kMaxNetworksInScanResponse)
            {
                scanResponseArrayLength++;
            }
            scanResponseArray[scanResponseArrayLength - 1] = scanResponse;
            Sorting::InsertionSort(
                scanResponseArray.Get(), scanResponseArrayLength,
                [](const ThreadScanResponse & a, const ThreadScanResponse & b) -> bool { return a.rssi > b.rssi; });
        }

        for (size_t i = 0; i < scanResponseArrayLength; i++)
        {
            Structs::ThreadInterfaceScanResultStruct::Type result;
            Encoding::BigEndian::Put64(extendedAddressBuffer, scanResponseArray[i].extendedAddress);
            result.panId           = scanResponseArray[i].panId;
            result.extendedPanId   = scanResponseArray[i].extendedPanId;
            result.networkName     = CharSpan(scanResponseArray[i].networkName, scanResponseArray[i].networkNameLen);
            result.channel         = scanResponseArray[i].channel;
            result.version         = scanResponseArray[i].version;
            result.extendedAddress = ByteSpan(extendedAddressBuffer);
            result.rssi            = scanResponseArray[i].rssi;
            result.lqi             = scanResponseArray[i].lqi;

            SuccessOrExit(err = DataModel::Encode(*writer, TLV::AnonymousTag(), result));
        }
    }

    SuccessOrExit(err = writer->EndContainer(listContainerType));
    SuccessOrExit(err = commandHandle->FinishCommand());

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response: %s", err.AsString());
    }
    if (status == NetworkCommissioningStatusEnum::kSuccess)
    {
        CommitSavedBreadcrumb();
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

    mLastNetworkingStatusValue.SetNonNull(status);
    mLastConnectErrorValue.SetNull();
    mLastNetworkIDLen = 0;

    TLV::TLVWriter * writer;
    TLV::TLVType listContainerType;
    WiFiScanResponse scanResponse;
    size_t networksEncoded = 0;

    SuccessOrExit(err = commandHandle->PrepareCommand(
                      ConcreteCommandPath(mPath.mEndpointId, NetworkCommissioning::Id, Commands::ScanNetworksResponse::Id)));
    VerifyOrExit((writer = commandHandle->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    SuccessOrExit(err = writer->Put(TLV::ContextTag(Commands::ScanNetworksResponse::Fields::kNetworkingStatus), status));
    if (debugText.size() != 0)
    {
        SuccessOrExit(
            err = DataModel::Encode(*writer, TLV::ContextTag(Commands::ScanNetworksResponse::Fields::kDebugText), debugText));
    }
    SuccessOrExit(err = writer->StartContainer(TLV::ContextTag(Commands::ScanNetworksResponse::Fields::kWiFiScanResults),
                                               TLV::TLVType::kTLVType_Array, listContainerType));

    for (; networks != nullptr && networks->Next(scanResponse) && networksEncoded < kMaxNetworksInScanResponse; networksEncoded++)
    {
        Structs::WiFiInterfaceScanResultStruct::Type result;
        result.security = scanResponse.security;
        result.ssid     = ByteSpan(scanResponse.ssid, scanResponse.ssidLen);
        result.bssid    = ByteSpan(scanResponse.bssid, sizeof(scanResponse.bssid));
        result.channel  = scanResponse.channel;
        result.wiFiBand = scanResponse.wiFiBand;
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
    if (status == NetworkCommissioningStatusEnum::kSuccess)
    {
        CommitSavedBreadcrumb();
    }
    if (networks != nullptr)
    {
        networks->Release();
    }
}

void Instance::OnPlatformEventHandler(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    Instance * this_ = reinterpret_cast<Instance *>(arg);

    if (event->Type == DeviceLayer::DeviceEventType::kCommissioningComplete)
    {
        this_->OnCommissioningComplete();
    }
    else if (event->Type == DeviceLayer::DeviceEventType::kFailSafeTimerExpired)
    {
        this_->OnFailSafeTimerExpired();
    }
}

void Instance::OnCommissioningComplete()
{
    VerifyOrReturn(mpWirelessDriver != nullptr);

    ChipLogDetail(Zcl, "Commissioning complete, notify platform driver to persist network credentials.");
    mpWirelessDriver->CommitConfiguration();
}

void Instance::OnFailSafeTimerExpired()
{
    VerifyOrReturn(mpWirelessDriver != nullptr);

    ChipLogDetail(Zcl, "Failsafe timeout, tell platform driver to revert network credentials.");
    mpWirelessDriver->RevertConfiguration();
    mAsyncCommandHandle.Release();
}

CHIP_ERROR Instance::EnumerateAcceptedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context)
{
    using namespace Clusters::NetworkCommissioning::Commands;

    constexpr CommandId acceptedCommandsListWiFi[] = {
        ScanNetworks::Id, AddOrUpdateWiFiNetwork::Id, RemoveNetwork::Id, ConnectNetwork::Id, ReorderNetwork::Id,
    };
    constexpr CommandId acceptedCommandsListThread[] = {
        ScanNetworks::Id, AddOrUpdateThreadNetwork::Id, RemoveNetwork::Id, ConnectNetwork::Id, ReorderNetwork::Id,
    };

    if (mFeatureFlags.Has(Feature::kThreadNetworkInterface))
    {
        for (const auto & cmd : acceptedCommandsListThread)
        {
            if (callback(cmd, context) != Loop::Continue)
            {
                break;
            }
        }

        return CHIP_NO_ERROR;
    }

    if (mFeatureFlags.Has(Feature::kWiFiNetworkInterface))
    {
        for (const auto & cmd : acceptedCommandsListWiFi)
        {
            if (callback(cmd, context) != Loop::Continue)
            {
                break;
            }
        }

        return CHIP_NO_ERROR;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::EnumerateGeneratedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context)
{
    using namespace Clusters::NetworkCommissioning::Commands;

    constexpr CommandId generatedCommandsListWireless[] = { ScanNetworksResponse::Id, NetworkConfigResponse::Id,
                                                            ConnectNetworkResponse::Id };

    if (mFeatureFlags.HasAny(Feature::kWiFiNetworkInterface, Feature::kThreadNetworkInterface))
    {
        for (const auto & cmd : generatedCommandsListWireless)
        {
            if (callback(cmd, context) != Loop::Continue)
            {
                break;
            }
        }
    }

    return CHIP_NO_ERROR;
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

void MatterNetworkCommissioningPluginServerInitCallback()
{
    // Nothing to do, the server init routine will be done in Instance::Init()
}
