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
#include <lib/support/SafeInt.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <platform/internal/DeviceControlServer.h>
#include <platform/internal/DeviceNetworkInfo.h>

namespace chip {
namespace app {
namespace Clusters {
namespace NetworkCommissioning {

namespace {
// For WiFi and Thread scan results, each item will cose ~60 bytes in TLV, thus 16 is a safe upper bound of scan results.
constexpr size_t kMaxNetworksInScanResponse = 16;
constexpr uint32_t kFeatureMapWiFi          = 0b0000'0001;
constexpr uint32_t kFeatureMapThread        = 0b0000'0010;
} // namespace

// TODO: The network commissioning cluster should listen to OnCommissioningComplete event, and start to connect to the network in
// the background.
// TODO: The Reorder network / remove network uses lots of copy operation and should be updated.

void InstanceBase::InvokeCommand(HandlerContext & ctxt)
{
    if (mAsyncCommandHandle.Get() != nullptr)
    {
        // We have a command processing in the backend, reject all incoming commands.
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Busy);
        ctxt.SetCommandHandled();
        return;
    }

    // We only use mAsyncCommandHandle to determine whether we are processing another command, so it is OK to left some values
    // dirty.

    mPath = ctxt.mRequestPath;

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::ScanNetworks::Id:
        HandleCommand<Commands::ScanNetworks::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleScanNetworks(ctx, req); });
        return;
    case Commands::AddOrUpdateWiFiNetwork::Id:
        VerifyOrReturn(mType == NetworkType::kWiFi);
        HandleCommand<Commands::AddOrUpdateWiFiNetwork::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleAddOrUpdateWiFiNetwork(ctx, req); });
        return;
    case Commands::AddOrUpdateThreadNetwork::Id:
        VerifyOrReturn(mType == NetworkType::kThread);
        HandleCommand<Commands::AddOrUpdateThreadNetwork::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleAddOrUpdateThreadNetwork(ctx, req); });
        return;
    case Commands::RemoveNetwork::Id:
        HandleCommand<Commands::RemoveNetwork::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleRemoveNetwork(ctx, req); });
        return;
    case Commands::ConnectNetwork::Id:
        HandleCommand<Commands::ConnectNetwork::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleConnectNetwork(ctx, req); });
        return;
    case Commands::ReorderNetwork::Id:
        HandleCommand<Commands::ReorderNetwork::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleReorderNetwork(ctx, req); });
    }
}

CHIP_ERROR InstanceBase::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case Attributes::MaxNetworks::Id:
        return aEncoder.Encode(mMaxNetworks);
    case Attributes::Networks::Id:
        return HandleReadNetworks(aEncoder);
    case Attributes::ScanMaxTimeSeconds::Id:
        return HandleReadScanMaxTimeSeconds(aEncoder);
    case Attributes::ConnectMaxTimeSeconds::Id:
        return HandleReadConnectMaxTimeSeconds(aEncoder);

    // TODO: Add support to the following attributes
    case Attributes::InterfaceEnabled::Id:
        return aEncoder.Encode(true);
    case Attributes::LastNetworkingStatus::Id:
        return aEncoder.Encode(NetworkCommissioningStatus::kSuccess);
    case Attributes::LastNetworkID::Id:
        return aEncoder.Encode(CharSpan());
    case Attributes::LastConnectErrorValue::Id:
        return aEncoder.Encode(Attributes::LastConnectErrorValue::TypeInfo::Type(0));

    case Attributes::FeatureMap::Id:
        switch (mType)
        {
        case NetworkType::kWiFi:
            return aEncoder.Encode(kFeatureMapWiFi);
        case NetworkType::kThread:
            return aEncoder.Encode(kFeatureMapThread);
        default:
            return aEncoder.Encode(Attributes::FeatureMap::TypeInfo::Type(0));
        }
    default:
        return CHIP_NO_ERROR;
    }
}

CHIP_ERROR InstanceBase::HandleReadNetworks(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([this](const auto & encoder) {
        for (size_t i = 0; i < mMaxNetworks; i++)
        {
            if (mpNetworks[i].mNetworkType == mType)
            {
                NetworkCommissioning::Structs::NetworkInfo::Type tp;
                tp.networkID = ByteSpan(mpNetworks[i].mNetworkID, mpNetworks[i].mNetworkIDLen);
                tp.connected = mpNetworks[i].mEnabled;
                ReturnErrorOnFailure(encoder.Encode(tp));
            }
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR InstanceBase::HandleReadScanMaxTimeSeconds(AttributeValueEncoder & aEncoder)
{
    if (mType == NetworkType::kWiFi)
    {
        return aEncoder.Encode(mpWiFiDelegate->GetScanNetworkTimeoutSeconds());
    }
    else if (mType == NetworkType::kThread)
    {
        return aEncoder.Encode(mpThreadDelegate->GetScanNetworkTimeoutSeconds());
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR InstanceBase::HandleReadConnectMaxTimeSeconds(AttributeValueEncoder & aEncoder)
{
    if (mType == NetworkType::kWiFi)
    {
        return aEncoder.Encode(mpWiFiDelegate->GetConnectNetworkTimeoutSeconds());
    }
    else if (mType == NetworkType::kThread)
    {
        return aEncoder.Encode(mpThreadDelegate->GetConnectNetworkTimeoutSeconds());
    }
    return CHIP_NO_ERROR;
}

void InstanceBase::HandleScanNetworks(HandlerContext & ctx, const Commands::ScanNetworks::DecodableType & req)
{
    switch (mType)
    {
    case NetworkType::kWiFi:
        mAsyncCommandHandle = app::CommandHandler::Handle(&ctx.mCommandHandler);
        mpWiFiDelegate->ScanWiFiNetworks(req.ssid, this);
        return;
    case NetworkType::kThread:
        mAsyncCommandHandle = app::CommandHandler::Handle(&ctx.mCommandHandler);
        mpThreadDelegate->ScanThreadNetworks(this);
        return;
    default:
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
        return;
    }
}

void InstanceBase::HandleAddOrUpdateWiFiNetwork(HandlerContext & ctx, const Commands::AddOrUpdateWiFiNetwork::DecodableType & req)
{
    Commands::NetworkConfigResponse::Type response;
    NetworkCommissioningStatus err = NetworkCommissioningStatus::kSuccess;

    size_t itemIndex = 0;
    for (; itemIndex < mMaxNetworks; itemIndex++)
    {
        if (mpNetworks[itemIndex].mNetworkType == NetworkType::kUndefined)
        {
            break;
        }
        else if (mpNetworks[itemIndex].mNetworkIDLen == req.ssid.size() &&
                 memcmp(mpNetworks[itemIndex].mNetworkID, req.ssid.data(), req.ssid.size()) == 0)
        {
            break;
        }
    }

    VerifyOrExit(itemIndex < mMaxNetworks, err = NetworkCommissioningStatus::kBoundsExceeded);

    VerifyOrExit(req.ssid.size() <= sizeof(mpNetworks[itemIndex].mData.mWiFi.mSSID), err = NetworkCommissioningStatus::kOutOfRange);
    memcpy(mpNetworks[itemIndex].mData.mWiFi.mSSID, req.ssid.data(), req.ssid.size());

    using WiFiSSIDLenType = decltype(mpNetworks[itemIndex].mData.mWiFi.mSSIDLen);
    VerifyOrExit(CanCastTo<WiFiSSIDLenType>(req.ssid.size()), err = NetworkCommissioningStatus::kOutOfRange);
    mpNetworks[itemIndex].mData.mWiFi.mSSIDLen = static_cast<WiFiSSIDLenType>(req.ssid.size());

    VerifyOrExit(req.credentials.size() <= sizeof(mpNetworks[itemIndex].mData.mWiFi.mCredentials),
                 err = NetworkCommissioningStatus::kOutOfRange);
    memcpy(mpNetworks[itemIndex].mData.mWiFi.mCredentials, req.credentials.data(), req.credentials.size());

    using WiFiCredentialsLenType = decltype(mpNetworks[itemIndex].mData.mWiFi.mCredentialsLen);
    VerifyOrExit(CanCastTo<WiFiCredentialsLenType>(req.ssid.size()), err = NetworkCommissioningStatus::kOutOfRange);
    mpNetworks[itemIndex].mData.mWiFi.mCredentialsLen = static_cast<WiFiCredentialsLenType>(req.credentials.size());

    VerifyOrExit(req.ssid.size() <= sizeof(mpNetworks[itemIndex].mNetworkID), err = NetworkCommissioningStatus::kOutOfRange);
    memcpy(mpNetworks[itemIndex].mNetworkID, mpNetworks[itemIndex].mData.mWiFi.mSSID, req.ssid.size());

    using NetworkIDLenType = decltype(mpNetworks[itemIndex].mNetworkIDLen);
    VerifyOrExit(CanCastTo<NetworkIDLenType>(req.ssid.size()), err = NetworkCommissioningStatus::kOutOfRange);
    mpNetworks[itemIndex].mNetworkIDLen = static_cast<NetworkIDLenType>(req.ssid.size());

    mpNetworks[itemIndex].mNetworkType = NetworkType::kWiFi;
    mpNetworks[itemIndex].mEnabled     = false;

    VerifyOrExit(err == NetworkCommissioningStatus::kSuccess, );

    ChipLogDetail(Zcl, "WiFi provisioning data: SSID: %.*s", static_cast<int>(req.ssid.size()), req.ssid.data());
exit:
    response.networkingStatus = err;
    ctx.mCommandHandler.AddResponseData(ctx.mRequestPath, response);
}

void InstanceBase::HandleAddOrUpdateThreadNetwork(HandlerContext & ctx,
                                                  const Commands::AddOrUpdateThreadNetwork::DecodableType & req)
{
    Commands::NetworkConfigResponse::Type response;
    NetworkCommissioningStatus err = NetworkCommissioningStatus::kSuccess;

    Thread::OperationalDataset dataset;
    CHIP_ERROR error = dataset.Init(req.operationalDataset);
    uint8_t extendedPanId[Thread::kSizeExtendedPanId];

    size_t itemIndex = 0;

    if (error != CHIP_NO_ERROR)
    {
        ChipLogDetail(Zcl, "Failed to parse Thread operational dataset: %s", ErrorStr(error));
        err = NetworkCommissioningStatus::kUnknownError;
        ExitNow();
    }

    static_assert(sizeof(NetworkInfo::mNetworkID) >= sizeof(extendedPanId),
                  "Network ID must be larger than Thread extended PAN ID!");
    SuccessOrExit(dataset.GetExtendedPanId(extendedPanId));

    for (; itemIndex < mMaxNetworks; itemIndex++)
    {
        if (mpNetworks[itemIndex].mNetworkType == NetworkType::kUndefined)
        {
            break;
        }
        else if (mpNetworks[itemIndex].mNetworkIDLen == Thread::kSizeExtendedPanId &&
                 memcmp(mpNetworks[itemIndex].mNetworkID, extendedPanId, Thread::kSizeExtendedPanId) == 0)
        {
            break;
        }
    }

    VerifyOrExit(itemIndex < mMaxNetworks, err = NetworkCommissioningStatus::kBoundsExceeded);

    memcpy(mpNetworks[itemIndex].mNetworkID, extendedPanId, sizeof(extendedPanId));
    mpNetworks[itemIndex].mData.mThread = dataset;
    mpNetworks[itemIndex].mNetworkIDLen = sizeof(extendedPanId);
    mpNetworks[itemIndex].mNetworkType  = NetworkType::kThread;
    mpNetworks[itemIndex].mEnabled      = false;

exit:
    response.networkingStatus = err;
    ctx.mCommandHandler.AddResponseData(ctx.mRequestPath, response);
}

void InstanceBase::HandleRemoveNetwork(HandlerContext & ctx, const Commands::RemoveNetwork::DecodableType & req)
{
    Commands::NetworkConfigResponse::Type response;
    NetworkCommissioningStatus err = NetworkCommissioningStatus::kNetworkIDNotFound;

    size_t networkSeq = mMaxNetworks;

    for (networkSeq = 0; networkSeq < mMaxNetworks; networkSeq++)
    {
        if (mpNetworks[networkSeq].mNetworkIDLen == req.networkID.size() &&
            mpNetworks[networkSeq].mNetworkType != NetworkType::kUndefined &&
            memcmp(mpNetworks[networkSeq].mNetworkID, req.networkID.data(), req.networkID.size()) == 0)
        {
            err = NetworkCommissioningStatus::kSuccess;
            break;
        }
    }

    if (err == NetworkCommissioningStatus::kSuccess)
    {
        networkSeq++;
        for (; networkSeq < mMaxNetworks; networkSeq++)
        {
            mpNetworks[networkSeq - 1] = mpNetworks[networkSeq];
        }
        mpNetworks[mMaxNetworks].mNetworkType = NetworkType::kUndefined;
    }

    response.networkingStatus = err;
    ctx.mCommandHandler.AddResponseData(ctx.mRequestPath, response);
}

void InstanceBase::HandleConnectNetwork(HandlerContext & ctx, const Commands::ConnectNetwork::DecodableType & req)
{
    Commands::ConnectNetworkResponse::Type response;
    NetworkCommissioningStatus err = NetworkCommissioningStatus::kNetworkIDNotFound;

    mAsyncCommandHandle = app::CommandHandler::Handle(&ctx.mCommandHandler);

    for (size_t networkSeq = 0; networkSeq < mMaxNetworks; networkSeq++)
    {
        if (mpNetworks[networkSeq].mNetworkIDLen == req.networkID.size() &&
            mpNetworks[networkSeq].mNetworkType != NetworkType::kUndefined &&
            memcmp(mpNetworks[networkSeq].mNetworkID, req.networkID.data(), req.networkID.size()) == 0)
        {
            // TODO: Currently, we cannot figure out the detailed error from network provisioning on DeviceLayer, we should
            // implement this in device layer.

            VerifyOrExit(DoConnectNetwork(&mpNetworks[networkSeq]) == CHIP_NO_ERROR,
                         err = NetworkCommissioningStatus::kUnknownError);
            return;
        }
    }
exit:
    response.networkingStatus = err;
    ctx.mCommandHandler.AddResponseData(ctx.mRequestPath, response);
    mAsyncCommandHandle = nullptr;
}

void InstanceBase::HandleReorderNetwork(HandlerContext & ctx, const Commands::ReorderNetwork::DecodableType & req)
{
    Commands::NetworkConfigResponse::Type response;
    NetworkCommissioningStatus err = NetworkCommissioningStatus::kNetworkIDNotFound;

    size_t networkSeq = mMaxNetworks;

    for (networkSeq = 0; networkSeq < mMaxNetworks; networkSeq++)
    {
        if (mpNetworks[networkSeq].mNetworkIDLen == req.networkID.size() &&
            mpNetworks[networkSeq].mNetworkType != NetworkType::kUndefined &&
            memcmp(mpNetworks[networkSeq].mNetworkID, req.networkID.data(), req.networkID.size()) == 0)
        {
            err = NetworkCommissioningStatus::kSuccess;
            break;
        }
    }

    if (err == NetworkCommissioningStatus::kSuccess)
    {
        size_t target = req.networkIndex;
        if (target >= mMaxNetworks)
        {
            target = mMaxNetworks - 1;
        }
        if (target < networkSeq)
        {
            NetworkInfo network = mpNetworks[networkSeq];
            for (size_t i = networkSeq; i > target; i--)
            {
                mpNetworks[i] = mpNetworks[i - 1];
            }
            mpNetworks[target] = network;
        }
        else if (target > networkSeq)
        {
            NetworkInfo network = mpNetworks[networkSeq];
            for (size_t i = networkSeq; i < target; i++)
            {
                mpNetworks[i] = mpNetworks[i + 1];
            }
            mpNetworks[target] = network;
        }
    }

    response.networkingStatus = err;
    ctx.mCommandHandler.AddResponseData(ctx.mRequestPath, response);
}

CHIP_ERROR InstanceBase::DoConnectNetwork(NetworkInfo * network)
{
    mCurrentProcessingNetworkInfo = network;
    switch (network->mNetworkType)
    {
    case NetworkType::kThread:
        mpThreadDelegate->ConnectThreadNetwork(network->mData.mThread.AsByteSpan(), this);
        return CHIP_NO_ERROR;
    case NetworkType::kWiFi:
        mpWiFiDelegate->ConnectWiFiNetwork(ByteSpan(network->mData.mWiFi.mSSID, network->mData.mWiFi.mSSIDLen),
                                           ByteSpan(network->mData.mWiFi.mCredentials, network->mData.mWiFi.mCredentialsLen), this);
        return CHIP_NO_ERROR;
    case NetworkType::kEthernet:
    case NetworkType::kUndefined:
    default:
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
}

void InstanceBase::PrepareCommand(ConcreteCommandPath path, NetworkInfo * info)
{
    mPath                         = path;
    mCurrentProcessingNetworkInfo = info;
}

void InstanceBase::OnConnectResult(app::Clusters::NetworkCommissioning::NetworkCommissioningStatus commissioningError,
                                   CharSpan errorText, int32_t networkRelatedAssociationStatus)
{
    auto commandHandleRef = std::move(mAsyncCommandHandle);
    auto commandHandle    = commandHandleRef.Get();
    if (commandHandle == nullptr)
    {
        // Oh, this is a response of a network scan from unclean shutdown, ignore it.
        return;
    }

    Commands::ConnectNetworkResponse::Type response;
    response.networkingStatus = commissioningError;
    response.debugText        = errorText;
    response.errorValue       = networkRelatedAssociationStatus;
    commandHandle->AddResponseData(mPath, response);

    if (commissioningError == NetworkCommissioningStatus::kSuccess)
    {
        DeviceLayer::Internal::DeviceControlServer::DeviceControlSvr().ConnectNetworkForOperational(
            ByteSpan(mCurrentProcessingNetworkInfo->mNetworkID, mCurrentProcessingNetworkInfo->mNetworkIDLen));
    }

    mCurrentProcessingNetworkInfo->mEnabled = true;
}

void InstanceBase::OnScanFinished(CHIP_ERROR err, CharSpan debugText,
                                  const Span<Structs::ThreadInterfaceScanResult::Type> & networks)
{
    auto commandHandleRef = std::move(mAsyncCommandHandle);
    auto commandHandle    = commandHandleRef.Get();
    if (commandHandle == nullptr)
    {
        // Oh, this is a response of a network scan from unclean shutdown, ignore it.
        return;
    }

    Commands::ScanNetworksResponse::Type response;
    response.networkingStatus =
        (err == CHIP_NO_ERROR ? NetworkCommissioningStatus::kSuccess : NetworkCommissioningStatus::kUnknownError);
    response.debugText         = debugText;
    response.threadScanResults = networks;
    if (response.threadScanResults.size() > kMaxNetworksInScanResponse)
    {
        response.threadScanResults.reduce_size(kMaxNetworksInScanResponse);
    }
    commandHandle->AddResponseData(mPath, response);
}

void InstanceBase::OnScanFinished(CHIP_ERROR err, CharSpan debugText, const Span<Structs::WiFiInterfaceScanResult::Type> & networks)
{
    auto commandHandleRef = std::move(mAsyncCommandHandle);
    auto commandHandle    = commandHandleRef.Get();
    if (commandHandle == nullptr)
    {
        // Oh, this is a response of a network scan from unclean shutdown, ignore it.
        return;
    }

    Commands::ScanNetworksResponse::Type response;
    response.networkingStatus =
        (err == CHIP_NO_ERROR ? NetworkCommissioningStatus::kSuccess : NetworkCommissioningStatus::kUnknownError);
    response.debugText       = debugText;
    response.wiFiScanResults = networks;
    if (response.wiFiScanResults.size() > kMaxNetworksInScanResponse)
    {
        response.wiFiScanResults.reduce_size(kMaxNetworksInScanResponse);
    }
    commandHandle->AddResponseData(mPath, response);
}

} // namespace NetworkCommissioning
} // namespace Clusters
} // namespace app
} // namespace chip
