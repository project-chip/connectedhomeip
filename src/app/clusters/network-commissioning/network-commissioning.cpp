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
#include <app/data-model/Nullable.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <credentials/CHIPCert.h>
#include <lib/core/CHIPConfig.h>
#include <lib/support/SafeInt.h>
#include <lib/support/SortUtils.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/DeviceControlServer.h>
#include <platform/PlatformManager.h>
#include <platform/internal/DeviceNetworkInfo.h>
#include <tracing/macros.h>

#include <array>
#include <utility>

namespace chip {
namespace app {
namespace Clusters {
namespace NetworkCommissioning {

using namespace Credentials;
using namespace DataModel;
using namespace DeviceLayer::NetworkCommissioning;

namespace {

// For WiFi and Thread scan results, each item will cost ~60 bytes in TLV, thus 15 is a safe upper bound of scan results.
constexpr size_t kMaxNetworksInScanResponse = 15;

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
constexpr size_t kPossessionNonceSize = 32;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

// Note: CHIP_CONFIG_NETWORK_COMMISSIONING_DEBUG_TEXT_BUFFER_SIZE can be 0, this disables debug text
using DebugTextStorage = std::array<char, CHIP_CONFIG_NETWORK_COMMISSIONING_DEBUG_TEXT_BUFFER_SIZE>;

enum ValidWiFiCredentialLength
{
    kOpen      = 0,
    kWEP64     = 5,
    kMinWPAPSK = 8,
    kMaxWPAPSK = 63,
    kWPAPSKHex = 64,
};

template <typename T, typename Func>
static void EnumerateAndRelease(Iterator<T> * iterator, Func func)
{
    if (iterator != nullptr)
    {
        T element;
        while (iterator->Next(element) && func(element) == Loop::Continue)
        {
            /* continue */
        }
        iterator->Release();
    }
}

BitFlags<Feature> WiFiFeatures(WiFiDriver * driver)
{
    BitFlags<Feature> features = Feature::kWiFiNetworkInterface;
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    features.Set(Feature::kPerDeviceCredentials, driver->SupportsPerDeviceCredentials());
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    return features;
}

} // namespace

Instance::Instance(EndpointId aEndpointId, WiFiDriver * apDelegate) :
    CommandHandlerInterface(Optional<EndpointId>(aEndpointId), Id), AttributeAccessInterface(Optional<EndpointId>(aEndpointId), Id),
    mFeatureFlags(WiFiFeatures(apDelegate)), mpWirelessDriver(apDelegate), mpBaseDriver(apDelegate)
{
    mpDriver.Set<WiFiDriver *>(apDelegate);
}

Instance::Instance(EndpointId aEndpointId, ThreadDriver * apDelegate) :
    CommandHandlerInterface(Optional<EndpointId>(aEndpointId), Id), AttributeAccessInterface(Optional<EndpointId>(aEndpointId), Id),
    mFeatureFlags(Feature::kThreadNetworkInterface), mpWirelessDriver(apDelegate), mpBaseDriver(apDelegate)
{
    mpDriver.Set<ThreadDriver *>(apDelegate);
}

Instance::Instance(EndpointId aEndpointId, EthernetDriver * apDelegate) :
    CommandHandlerInterface(Optional<EndpointId>(aEndpointId), Id), AttributeAccessInterface(Optional<EndpointId>(aEndpointId), Id),
    mFeatureFlags(Feature::kEthernetNetworkInterface), mpWirelessDriver(nullptr), mpBaseDriver(apDelegate)
{}

CHIP_ERROR Instance::Init()
{
    ReturnErrorOnFailure(InteractionModelEngine::GetInstance()->RegisterCommandHandler(this));
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

    case Commands::ConnectNetwork::Id: {
        VerifyOrReturn(mFeatureFlags.Has(Feature::kWiFiNetworkInterface) || mFeatureFlags.Has(Feature::kThreadNetworkInterface));
#if CONFIG_NETWORK_LAYER_BLE && !CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
        // If commissionee does not support Concurrent Connections, request the BLE to be stopped.
        // Start the ConnectNetwork, but this will not complete until the BLE is off.
        ChipLogProgress(NetworkProvisioning, "Closing BLE connections due to non-concurrent mode");
        DeviceLayer::DeviceControlServer::DeviceControlSvr().PostCloseAllBLEConnectionsToOperationalNetworkEvent();
#endif
        HandleCommand<Commands::ConnectNetwork::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleConnectNetwork(ctx, req); });
        return;
    }

    case Commands::ReorderNetwork::Id:
        VerifyOrReturn(mFeatureFlags.Has(Feature::kWiFiNetworkInterface) || mFeatureFlags.Has(Feature::kThreadNetworkInterface));
        HandleCommand<Commands::ReorderNetwork::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleReorderNetwork(ctx, req); });
        return;
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    case Commands::QueryIdentity::Id:
        VerifyOrReturn(mFeatureFlags.Has(Feature::kPerDeviceCredentials));
        HandleCommand<Commands::QueryIdentity::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleQueryIdentity(ctx, req); });
        return;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
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
            CHIP_ERROR err = CHIP_NO_ERROR;
            Structs::NetworkInfoStruct::Type networkForEncode;
            EnumerateAndRelease(mpBaseDriver->GetNetworks(), [&](const Network & network) {
                networkForEncode.networkID = ByteSpan(network.networkID, network.networkIDLen);
                networkForEncode.connected = network.connected;

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
                // These fields are both optional and nullable in NetworkInfoStruct.
                // If PDC is supported, the fields are always present but may be null.
                if (mFeatureFlags.Has(Feature::kPerDeviceCredentials))
                {
                    networkForEncode.networkIdentifier = MakeOptional(Nullable<ByteSpan>(network.networkIdentifier));
                    networkForEncode.clientIdentifier  = MakeOptional(Nullable<ByteSpan>(network.clientIdentifier));
                }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

                err = encoder.Encode(networkForEncode);
                return (err == CHIP_NO_ERROR) ? Loop::Continue : Loop::Break;
            });
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

    case Attributes::SupportedWiFiBands::Id:
        VerifyOrReturnError(mFeatureFlags.Has(Feature::kWiFiNetworkInterface), CHIP_NO_ERROR);
        VerifyOrReturnError(mpDriver.Valid(), CHIP_NO_ERROR);
        return aEncoder.Encode(mpDriver.Get<WiFiDriver *>()->GetSupportedWiFiBands());

    case Attributes::SupportedThreadFeatures::Id:
        VerifyOrReturnError(mFeatureFlags.Has(Feature::kThreadNetworkInterface), CHIP_NO_ERROR);
        VerifyOrReturnError(mpDriver.Valid(), CHIP_NO_ERROR);
        return aEncoder.Encode(mpDriver.Get<ThreadDriver *>()->GetSupportedThreadFeatures());

    case Attributes::ThreadVersion::Id:
        VerifyOrReturnError(mFeatureFlags.Has(Feature::kThreadNetworkInterface), CHIP_NO_ERROR);
        VerifyOrReturnError(mpDriver.Valid(), CHIP_NO_ERROR);
        return aEncoder.Encode(mpDriver.Get<ThreadDriver *>()->GetThreadVersion());

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

void Instance::OnNetworkingStatusChange(Status aCommissioningError, Optional<ByteSpan> aNetworkId, Optional<int32_t> aConnectStatus)
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
        response.debugText.SetValue(debugText);
    }
    if (response.networkingStatus == Status::kSuccess)
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

    if (req.ssid.empty() || req.ssid.size() > DeviceLayer::Internal::kMaxWiFiSSIDLength)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::InvalidCommand, "ssid");
        return;
    }

    // Presence of a Network Identity indicates we're configuring for Per-Device Credentials
    if (req.networkIdentity.HasValue())
    {
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
        if (mFeatureFlags.Has(Feature::kWiFiNetworkInterface))
        {
            HandleAddOrUpdateWiFiNetworkWithPDC(ctx, req);
            return;
        }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::InvalidCommand);
        return;
    }

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
    DebugTextStorage debugTextBuffer;
    MutableCharSpan debugText(debugTextBuffer);
    uint8_t outNetworkIndex = 0;
    response.networkingStatus =
        mpDriver.Get<WiFiDriver *>()->AddOrUpdateNetwork(req.ssid, req.credentials, debugText, outNetworkIndex);
    FillDebugTextAndNetworkIndex(response, debugText, outNetworkIndex);
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    if (response.networkingStatus == Status::kSuccess)
    {
        UpdateBreadcrumb(req.breadcrumb);
    }
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
void Instance::HandleAddOrUpdateWiFiNetworkWithPDC(HandlerContext & ctx,
                                                   const Commands::AddOrUpdateWiFiNetwork::DecodableType & req)
{
    // Credentials must be empty when configuring for PDC, it's only present to keep the command shape compatible.
    if (!req.credentials.empty())
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::InvalidCommand, "credentials");
        return;
    }

    auto && networkIdentity = req.networkIdentity.Value(); // presence checked by caller
    if (networkIdentity.size() > kMaxCHIPCompactNetworkIdentityLength ||
        Credentials::ValidateChipNetworkIdentity(networkIdentity) != CHIP_NO_ERROR)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::InvalidCommand, "networkIdentity");
        return;
    }

    if (req.clientIdentifier.HasValue() && req.clientIdentifier.Value().size() != CertificateKeyId::size())
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::InvalidCommand, "clientIdentifier");
        return;
    }

    bool provePossession = req.possessionNonce.HasValue();
    if (provePossession && req.possessionNonce.Value().size() != kPossessionNonceSize)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::InvalidCommand, "possessionNonce");
        return;
    }

    auto err = CHIP_NO_ERROR;
    {
        auto driver = mpDriver.Get<WiFiDriver *>();

        // If the client is requesting re-use of a Client Identity, find the existing network it belongs to
        Optional<uint8_t> clientIdentityNetworkIndex;
        if (req.clientIdentifier.HasValue())
        {
            CertificateKeyId clientIdentifier(req.clientIdentifier.Value().data());
            uint8_t networkIndex = 0;
            EnumerateAndRelease(driver->GetNetworks(), [&](const Network & network) {
                if (network.clientIdentifier.HasValue() && clientIdentifier.data_equal(network.clientIdentifier.Value()))
                {
                    clientIdentityNetworkIndex.SetValue(networkIndex);
                    return Loop::Break;
                }
                networkIndex++;
                return Loop::Continue;
            });
            if (!clientIdentityNetworkIndex.HasValue())
            {
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::NotFound, "clientIdentifier");
                return;
            }
        }

        // Allocate a buffer to hold the client identity, and leave enough room to append the possession nonce if needed.
        chip::Platform::ScopedMemoryBuffer<uint8_t> identityBuffer;
        size_t identityBufferSize = kMaxCHIPCompactNetworkIdentityLength + (provePossession ? kPossessionNonceSize : 0);
        VerifyOrExit(identityBuffer.Alloc(identityBufferSize), /**/);

        // Add/Update the network at the driver level
        MutableByteSpan clientIdentity(identityBuffer.Get(), kMaxCHIPCompactNetworkIdentityLength);
        Optional<P256ECDSASignature> possessionSignature;
        Status status = Status::kUnknownError;
        DebugTextStorage debugTextBuffer;
        MutableCharSpan debugText(debugTextBuffer);
        uint8_t networkIndex;
        SuccessOrExit(err = driver->AddOrUpdateNetworkWithPDC(req.ssid, networkIdentity, clientIdentityNetworkIndex, status,
                                                              debugText, clientIdentity, networkIndex));

        Commands::NetworkConfigResponse::Type response;
        response.networkingStatus = status;
        FillDebugTextAndNetworkIndex(response, debugText, networkIndex);

        if (status == Status::kSuccess)
        {
            response.clientIdentity.SetValue(clientIdentity);

            if (provePossession)
            {
                // PossessionSignature TBS message = (NetworkClientIdentity || PossessionNonce)
                memcpy(clientIdentity.end(), req.possessionNonce.Value().data(), kPossessionNonceSize);
                ByteSpan tbsMessage(clientIdentity.data(), clientIdentity.size() + kPossessionNonceSize);
                SuccessOrExit(err = driver->SignWithClientIdentity(networkIndex, tbsMessage, possessionSignature.Emplace()));
                response.possessionSignature.SetValue(possessionSignature.Value().Span());
            }

            UpdateBreadcrumb(req.breadcrumb);
        }

        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "AddOrUpdateWiFiNetwork with PDC failed: %" CHIP_ERROR_FORMAT, err.Format());
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Failure);
    }
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

void Instance::HandleAddOrUpdateThreadNetwork(HandlerContext & ctx, const Commands::AddOrUpdateThreadNetwork::DecodableType & req)
{
    MATTER_TRACE_SCOPE("HandleAddOrUpdateThreadNetwork", "NetworkCommissioning");

    VerifyOrReturn(CheckFailSafeArmed(ctx));

    Commands::NetworkConfigResponse::Type response;
    DebugTextStorage debugTextBuffer;
    MutableCharSpan debugText(debugTextBuffer);
    uint8_t outNetworkIndex = 0;
    response.networkingStatus =
        mpDriver.Get<ThreadDriver *>()->AddOrUpdateNetwork(req.operationalDataset, debugText, outNetworkIndex);
    FillDebugTextAndNetworkIndex(response, debugText, outNetworkIndex);
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    if (response.networkingStatus == Status::kSuccess)
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
    DebugTextStorage debugTextBuffer;
    MutableCharSpan debugText(debugTextBuffer);
    uint8_t outNetworkIndex   = 0;
    response.networkingStatus = mpWirelessDriver->RemoveNetwork(req.networkID, debugText, outNetworkIndex);
    FillDebugTextAndNetworkIndex(response, debugText, outNetworkIndex);
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    if (response.networkingStatus == Status::kSuccess)
    {
        UpdateBreadcrumb(req.breadcrumb);
    }
}

void Instance::HandleConnectNetwork(HandlerContext & ctx, const Commands::ConnectNetwork::DecodableType & req)
{
    MATTER_TRACE_SCOPE("HandleConnectNetwork", "NetworkCommissioning");
    if (req.networkID.size() > kMaxNetworkIDLen)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::InvalidValue);
        return;
    }

    VerifyOrReturn(CheckFailSafeArmed(ctx));

    mConnectingNetworkIDLen = static_cast<uint8_t>(req.networkID.size());
    memcpy(mConnectingNetworkID, req.networkID.data(), mConnectingNetworkIDLen);
    mAsyncCommandHandle         = CommandHandler::Handle(&ctx.mCommandHandler);
    mCurrentOperationBreadcrumb = req.breadcrumb;

    // In Non-concurrent mode postpone the final execution of ConnectNetwork until the operational
    // network has been fully brought up and kWiFiDeviceAvailable is delivered.
    // mConnectingNetworkIDLen and mConnectingNetworkID contains the received SSID
#if CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
    mpWirelessDriver->ConnectNetwork(req.networkID, this);
#endif
}

void Instance::HandleNonConcurrentConnectNetwork()
{
    ByteSpan nonConcurrentNetworkID = ByteSpan(mConnectingNetworkID, mConnectingNetworkIDLen);
    ChipLogProgress(NetworkProvisioning, "HandleNonConcurrentConnectNetwork() SSID=%s", mConnectingNetworkID);
    mpWirelessDriver->ConnectNetwork(nonConcurrentNetworkID, this);
}

void Instance::HandleReorderNetwork(HandlerContext & ctx, const Commands::ReorderNetwork::DecodableType & req)
{
    MATTER_TRACE_SCOPE("HandleReorderNetwork", "NetworkCommissioning");
    Commands::NetworkConfigResponse::Type response;
    DebugTextStorage debugTextBuffer;
    MutableCharSpan debugText(debugTextBuffer);
    response.networkingStatus = mpWirelessDriver->ReorderNetwork(req.networkID, req.networkIndex, debugText);
    FillDebugTextAndNetworkIndex(response, debugText, req.networkIndex);
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    if (response.networkingStatus == Status::kSuccess)
    {
        UpdateBreadcrumb(req.breadcrumb);
    }
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
void Instance::HandleQueryIdentity(HandlerContext & ctx, const Commands::QueryIdentity::DecodableType & req)
{
    MATTER_TRACE_SCOPE("HandleQueryIdentity", "NetworkCommissioning");

    if (req.keyIdentifier.size() != CertificateKeyId::size())
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::InvalidCommand, "keyIdentifier");
        return;
    }
    CertificateKeyId keyIdentifier(req.keyIdentifier.data());

    bool provePossession = req.possessionNonce.HasValue();
    if (provePossession && req.possessionNonce.Value().size() != kPossessionNonceSize)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::InvalidCommand, "possessionNonce");
        return;
    }

    auto err      = CHIP_NO_ERROR;
    auto status   = Protocols::InteractionModel::Status::Success;
    auto driver   = mpDriver.Get<WiFiDriver *>();
    auto networks = driver->GetNetworks();
    VerifyOrExit(networks != nullptr && networks->Count() > 0, status = Protocols::InteractionModel::Status::NotFound);

    {
        // Allocate a buffer to hold the identity, and leave enough room to append the possession nonce if needed.
        chip::Platform::ScopedMemoryBuffer<uint8_t> identityBuffer;
        size_t identityBufferSize = kMaxCHIPCompactNetworkIdentityLength + (provePossession ? kPossessionNonceSize : 0);
        VerifyOrExit(identityBuffer.Alloc(identityBufferSize), /**/);

        MutableByteSpan identity(identityBuffer.Get(), kMaxCHIPCompactNetworkIdentityLength);
        Optional<P256ECDSASignature> possessionSignature;

        Network network;
        for (uint8_t networkIndex = 0;; networkIndex++)
        {
            VerifyOrExit(networks->Next(network), status = Protocols::InteractionModel::Status::NotFound);

            if (network.clientIdentifier.HasValue() && keyIdentifier.data_equal(network.clientIdentifier.Value()))
            {
                SuccessOrExit(err = driver->GetClientIdentity(networkIndex, identity));
                if (provePossession)
                {
                    // PossessionSignature TBS message = (NetworkClientIdentity || PossessionNonce)
                    memcpy(identity.end(), req.possessionNonce.Value().data(), kPossessionNonceSize);
                    ByteSpan tbsMessage(identity.data(), identity.size() + kPossessionNonceSize);
                    SuccessOrExit(err = driver->SignWithClientIdentity(networkIndex, tbsMessage, possessionSignature.Emplace()));
                }
                break;
            }
            if (!provePossession && // Proof-of-possession is not possible for network identities
                network.networkIdentifier.HasValue() && keyIdentifier.data_equal(network.networkIdentifier.Value()))
            {
                SuccessOrExit(err = driver->GetNetworkIdentity(networkIndex, identity));
                break;
            }
        }

        Commands::QueryIdentityResponse::Type response;
        response.identity = identity;
        if (possessionSignature.HasValue())
        {
            response.possessionSignature.SetValue(possessionSignature.Value().Span());
        }
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    }

exit:
    if (networks != nullptr)
    {
        networks->Release();
    }
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "QueryIdentity failed: %" CHIP_ERROR_FORMAT, err.Format());
        status = Protocols::InteractionModel::Status::Failure;
    }
    if (status != Protocols::InteractionModel::Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
    }
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

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

#if CONFIG_NETWORK_LAYER_BLE && !CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
    ChipLogProgress(NetworkProvisioning, "Non-concurrent mode, ConnectNetworkResponse will NOT be sent");
    // Do not send the ConnectNetworkResponse if in non-concurrent mode
    // Issue #30576 raised to modify CommandHandler to notify it if no response required
#else
    commandHandle->AddResponse(mPath, response);
#endif
    if (commissioningError == Status::kSuccess)
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
    Platform::ScopedMemoryBuffer<ThreadScanResponse> scanResponseArray;
    size_t scanResponseArrayLength = 0;
    uint8_t extendedAddressBuffer[Thread::kSizeExtendedPanId];

    const CommandHandler::InvokeResponseParameters prepareParams(mPath);
    SuccessOrExit(
        err = commandHandle->PrepareInvokeResponseCommand(
            ConcreteCommandPath(mPath.mEndpointId, NetworkCommissioning::Id, Commands::ScanNetworksResponse::Id), prepareParams));
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
        ChipLogError(Zcl, "Failed to encode response: %" CHIP_ERROR_FORMAT, err.Format());
    }
    if (status == Status::kSuccess)
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

    const CommandHandler::InvokeResponseParameters prepareParams(mPath);
    SuccessOrExit(
        err = commandHandle->PrepareInvokeResponseCommand(
            ConcreteCommandPath(mPath.mEndpointId, NetworkCommissioning::Id, Commands::ScanNetworksResponse::Id), prepareParams));
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
        ChipLogError(Zcl, "Failed to encode response: %" CHIP_ERROR_FORMAT, err.Format());
    }
    if (status == Status::kSuccess)
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
    else if (event->Type == DeviceLayer::DeviceEventType::kWiFiDeviceAvailable)
    {
        this_->HandleNonConcurrentConnectNetwork();
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

    if (mFeatureFlags.Has(Feature::kThreadNetworkInterface))
    {
        for (auto && cmd : {
                 ScanNetworks::Id,
                 AddOrUpdateThreadNetwork::Id,
                 RemoveNetwork::Id,
                 ConnectNetwork::Id,
                 ReorderNetwork::Id,
             })
        {
            VerifyOrExit(callback(cmd, context) == Loop::Continue, /**/);
        }
    }
    else if (mFeatureFlags.Has(Feature::kWiFiNetworkInterface))
    {
        for (auto && cmd : {
                 ScanNetworks::Id,
                 AddOrUpdateWiFiNetwork::Id,
                 RemoveNetwork::Id,
                 ConnectNetwork::Id,
                 ReorderNetwork::Id,
             })
        {
            VerifyOrExit(callback(cmd, context) == Loop::Continue, /**/);
        }
    }

    if (mFeatureFlags.Has(Feature::kPerDeviceCredentials))
    {
        VerifyOrExit(callback(QueryIdentity::Id, context) == Loop::Continue, /**/);
    }

exit:
    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::EnumerateGeneratedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context)
{
    using namespace Clusters::NetworkCommissioning::Commands;

    if (mFeatureFlags.HasAny(Feature::kWiFiNetworkInterface, Feature::kThreadNetworkInterface))
    {
        for (auto && cmd : { ScanNetworksResponse::Id, NetworkConfigResponse::Id, ConnectNetworkResponse::Id })
        {
            VerifyOrExit(callback(cmd, context) == Loop::Continue, /**/);
        }
    }

    if (mFeatureFlags.Has(Feature::kPerDeviceCredentials))
    {
        VerifyOrExit(callback(QueryIdentityResponse::Id, context) == Loop::Continue, /**/);
    }

exit:
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

NetworkIterator * NullNetworkDriver::GetNetworks()
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
