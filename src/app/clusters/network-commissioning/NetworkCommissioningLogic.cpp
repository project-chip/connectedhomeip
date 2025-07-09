/*
 *    Copyright (c) 2021-2025 Project CHIP Authors
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
#include "NetworkCommissioningLogic.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandler.h>
#include <app/CommandHandlerInterface.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/ConcreteCommandPath.h>
#include <app/clusters/general-commissioning-server/general-commissioning-server.h>
#include <app/clusters/network-commissioning/ThreadScanResponse.h>
#include <app/clusters/network-commissioning/WifiScanResponse.h>
#include <app/data-model/Nullable.h>
#include <app/reporting/reporting.h>
#include <app/server/Server.h>
#include <credentials/CHIPCert.h>
#include <lib/core/CHIPConfig.h>
#include <lib/support/SafeInt.h>
#include <lib/support/SortUtils.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <optional>
#include <platform/CHIPDeviceConfig.h>
#include <platform/ConnectivityManager.h>
#include <platform/DeviceControlServer.h>
#include <platform/PlatformManager.h>
#include <platform/internal/DeviceNetworkInfo.h>
#include <protocols/interaction_model/StatusCode.h>
#include <tracing/macros.h>

#include <array>
#include <utility>

namespace chip {
namespace app {
namespace Clusters {

using namespace Credentials;
using namespace DataModel;
using namespace DeviceLayer::NetworkCommissioning;
using namespace chip::app::Clusters::NetworkCommissioning;

namespace {

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

template <typename T>
static size_t CountAndRelease(Iterator<T> * iterator)
{
    size_t count = 0;
    if (iterator != nullptr)
    {
        count = iterator->Count();
        iterator->Release();
    }

    return count;
}

BitFlags<Feature> WiFiFeatures(WiFiDriver * driver)
{
    BitFlags<Feature> features = Feature::kWiFiNetworkInterface;
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    features.Set(Feature::kPerDeviceCredentials, driver->SupportsPerDeviceCredentials());
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    return features;
}

/// Performs an auto-release of the given item, generally an `Iterator` type
/// like Wifi or Thread scan results.
template <typename T>
class AutoRelease
{
public:
    AutoRelease(T * iterator) : mValue(iterator) {}
    ~AutoRelease()
    {
        if (mValue != nullptr)
        {
            mValue->Release();
        }
    }

private:
    T * mValue;
};

/// Convenience macro to auto-create a variable for you to release the given name at
/// the exit of the current scope.
#define DEFER_AUTO_RELEASE(name) AutoRelease autoRelease##__COUNTER__(name)

} // namespace

#if CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
NetworkCommissioningLogic::NetworkInstanceList NetworkCommissioningLogic::sInstances;
#endif

NetworkCommissioningLogic::NetworkCommissioningLogic(EndpointId aEndpointId, WiFiDriver * apDelegate) :
    mEndpointId(aEndpointId), mFeatureFlags(WiFiFeatures(apDelegate)), mpWirelessDriver(apDelegate), mpBaseDriver(apDelegate)
{
    mpDriver.Set<WiFiDriver *>(apDelegate);
}

NetworkCommissioningLogic::NetworkCommissioningLogic(EndpointId aEndpointId, ThreadDriver * apDelegate) :
    mEndpointId(aEndpointId), mFeatureFlags(Feature::kThreadNetworkInterface), mpWirelessDriver(apDelegate),
    mpBaseDriver(apDelegate)
{
    mpDriver.Set<ThreadDriver *>(apDelegate);
}

NetworkCommissioningLogic::NetworkCommissioningLogic(EndpointId aEndpointId, EthernetDriver * apDelegate) :
    mEndpointId(aEndpointId), mFeatureFlags(Feature::kEthernetNetworkInterface), mpWirelessDriver(nullptr), mpBaseDriver(apDelegate)
{}

CHIP_ERROR NetworkCommissioningLogic::Init()
{
    ReturnErrorOnFailure(DeviceLayer::PlatformMgrImpl().AddEventHandler(OnPlatformEventHandler, reinterpret_cast<intptr_t>(this)));
    ReturnErrorOnFailure(mpBaseDriver->Init(this));
    mLastNetworkingStatusValue.SetNull();
    mLastConnectErrorValue.SetNull();
    mLastNetworkIDLen = 0;
#if CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
    if (!sInstances.Contains(this))
    {
        sInstances.PushBack(this);
    }
#endif
    return CHIP_NO_ERROR;
}

void NetworkCommissioningLogic::Shutdown()
{
#if CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
    if (sInstances.Contains(this))
    {
        sInstances.Remove(this);
    }
#endif
    mpBaseDriver->Shutdown();
}

#if !CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
void NetworkCommissioningLogic::SendNonConcurrentConnectNetworkResponse()
{
    auto commandHandleRef = std::move(mAsyncCommandHandle);
    auto commandHandle    = commandHandleRef.Get();
    if (commandHandle == nullptr)
    {
        return;
    }

#if CONFIG_NETWORK_LAYER_BLE
    DeviceLayer::ConnectivityMgr().GetBleLayer()->IndicateBleClosing();
#endif // CONFIG_NETWORK_LAYER_BLE
    ChipLogProgress(NetworkProvisioning, "Non-concurrent mode. Send ConnectNetworkResponse(Success)");
    Commands::ConnectNetworkResponse::Type response;
    response.networkingStatus = NetworkCommissioning::Status::kSuccess;
    commandHandle->AddResponse(mAsyncCommandPath, response);
}
#endif // CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION

void NetworkCommissioningLogic::SetLastNetworkingStatusValue(Attributes::LastNetworkingStatus::TypeInfo::Type networkingStatusValue)
{
    if (mLastNetworkingStatusValue.Update(networkingStatusValue))
    {
        MatterReportingAttributeChangeCallback(mEndpointId, Clusters::NetworkCommissioning::Id,
                                               Attributes::LastNetworkingStatus::TypeInfo::GetAttributeId());
    }
}

void NetworkCommissioningLogic::SetLastConnectErrorValue(Attributes::LastConnectErrorValue::TypeInfo::Type connectErrorValue)
{
    if (mLastConnectErrorValue.Update(connectErrorValue))
    {
        MatterReportingAttributeChangeCallback(mEndpointId, Clusters::NetworkCommissioning::Id,
                                               Attributes::LastConnectErrorValue::TypeInfo::GetAttributeId());
    }
}

void NetworkCommissioningLogic::SetLastNetworkId(ByteSpan lastNetworkId)
{
    ByteSpan prevLastNetworkId{ mLastNetworkID, mLastNetworkIDLen };
    VerifyOrReturn(lastNetworkId.size() <= kMaxNetworkIDLen);
    VerifyOrReturn(!prevLastNetworkId.data_equal(lastNetworkId));

    memcpy(mLastNetworkID, lastNetworkId.data(), lastNetworkId.size());
    mLastNetworkIDLen = static_cast<uint8_t>(lastNetworkId.size());
    MatterReportingAttributeChangeCallback(mEndpointId, Clusters::NetworkCommissioning::Id,
                                           Attributes::LastNetworkID::TypeInfo::GetAttributeId());
}

void NetworkCommissioningLogic::ReportNetworksListChanged() const
{
    MatterReportingAttributeChangeCallback(mEndpointId, Clusters::NetworkCommissioning::Id,
                                           Attributes::Networks::TypeInfo::GetAttributeId());
}

void NetworkCommissioningLogic::OnNetworkingStatusChange(Status aCommissioningError, Optional<ByteSpan> aNetworkId,
                                                         Optional<int32_t> aConnectStatus)
{
    if (aNetworkId.HasValue())
    {
        if (aNetworkId.Value().size() > kMaxNetworkIDLen)
        {
            ChipLogError(DeviceLayer, "Overly large network ID received when calling OnNetworkingStatusChange");
        }
        else
        {
            SetLastNetworkId(aNetworkId.Value());
        }
    }

    SetLastNetworkingStatusValue(MakeNullable(aCommissioningError));
    if (aConnectStatus.HasValue())
    {
        SetLastConnectErrorValue(MakeNullable(aConnectStatus.Value()));
    }
    else
    {
        SetLastConnectErrorValue(NullNullable);
    }
}

std::optional<ActionReturnStatus> NetworkCommissioningLogic::HandleScanNetworks(CommandHandler & handler,
                                                                                const ConcreteCommandPath & commandPath,
                                                                                const Commands::ScanNetworks::DecodableType & req)
{
    MATTER_TRACE_SCOPE("HandleScanNetwork", "NetworkCommissioning");

    mScanningWasDirected = false;
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
            // Clients should never use too large a SSID.
            SetLastNetworkingStatusValue(MakeNullable(Status::kUnknownError));
            return Protocols::InteractionModel::Status::ConstraintError;
        }

        mScanningWasDirected        = !ssid.empty();
        mCurrentOperationBreadcrumb = req.breadcrumb;
        mAsyncCommandPath           = commandPath;
        mAsyncCommandHandle         = CommandHandler::Handle(&handler);
        handler.FlushAcksRightAwayOnSlowCommand();
        mpDriver.Get<WiFiDriver *>()->ScanNetworks(ssid, this);
        return std::nullopt;
    }

    if (mFeatureFlags.Has(Feature::kThreadNetworkInterface))
    {
        // NOTE: the following lines were commented out due to issue #32875. In short, a popular
        // commissioner is passing a null SSID argument and this logic breaks interoperability as a result.
        // The spec has some inconsistency on this which also needs to be fixed. The commissioner maker is
        // fixing its code and will return to un-comment this code, with that work tracked by Issue #32887.
        //
        // SSID present on Thread violates the `[WI]` conformance.
        // if (req.ssid.HasValue())
        // {
        //     return Protocols::InteractionModel::Status::InvalidCommand;
        // }

        mCurrentOperationBreadcrumb = req.breadcrumb;
        mAsyncCommandPath           = commandPath;
        mAsyncCommandHandle         = CommandHandler::Handle(&handler);
        handler.FlushAcksRightAwayOnSlowCommand();
        mpDriver.Get<ThreadDriver *>()->ScanNetworks(this);
        return std::nullopt;
    }

    return Protocols::InteractionModel::Status::UnsupportedCommand;
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

std::optional<ActionReturnStatus> EnsureFailsafeIsArmed(FabricIndex fabricIndex)
{
    auto & failSafeContext = chip::Server::GetInstance().GetFailSafeContext();

    if (!failSafeContext.IsFailSafeArmed(fabricIndex))
    {
        return Protocols::InteractionModel::Status::FailsafeRequired;
    }

    return std::nullopt;
}

/// Returns with an error status (FailsafeRequired) if the failsafe
/// is not armed for the given fabric index.
///
/// This just wraps EnsureFailsafeIsArmed with a one-liner for check & return.
#define RETURN_ERROR_STATUS_IF_FAILSAFE_NOT_ARMED(fabricIndex)                                                                     \
    if (std::optional<ActionReturnStatus> status = EnsureFailsafeIsArmed(fabricIndex); status.has_value())                         \
    {                                                                                                                              \
        return status;                                                                                                             \
    }                                                                                                                              \
    (void) 0

} // namespace

std::optional<ActionReturnStatus>
NetworkCommissioningLogic::HandleAddOrUpdateWiFiNetwork(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                                        const Commands::AddOrUpdateWiFiNetwork::DecodableType & req)
{
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION || CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
    MATTER_TRACE_SCOPE("HandleAddOrUpdateWiFiNetwork", "NetworkCommissioning");

    RETURN_ERROR_STATUS_IF_FAILSAFE_NOT_ARMED(handler.GetAccessingFabricIndex());

    if (req.ssid.empty() || req.ssid.size() > DeviceLayer::Internal::kMaxWiFiSSIDLength)
    {
        handler.AddStatus(commandPath, Protocols::InteractionModel::Status::ConstraintError, "ssid");
        return std::nullopt;
    }

    // Presence of a Network Identity indicates we're configuring for Per-Device Credentials
    if (req.networkIdentity.HasValue())
    {
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
        if (mFeatureFlags.Has(Feature::kWiFiNetworkInterface))
        {
            HandleAddOrUpdateWiFiNetworkWithPDC(handler, commandPath, req);
            return;
        }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
        return Protocols::InteractionModel::Status::InvalidCommand;
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
                return Protocols::InteractionModel::Status::ConstraintError;
            }
        }
    }
    else
    {
        // Invalid length
        return Protocols::InteractionModel::Status::ConstraintError;
    }

    Commands::NetworkConfigResponse::Type response;
    DebugTextStorage debugTextBuffer;
    MutableCharSpan debugText(debugTextBuffer);
    uint8_t outNetworkIndex = 0;
    response.networkingStatus =
        mpDriver.Get<WiFiDriver *>()->AddOrUpdateNetwork(req.ssid, req.credentials, debugText, outNetworkIndex);
    FillDebugTextAndNetworkIndex(response, debugText, outNetworkIndex);
    handler.AddResponse(commandPath, response);
    if (response.networkingStatus == Status::kSuccess)
    {
        UpdateBreadcrumb(req.breadcrumb);
        ReportNetworksListChanged();
    }
    return std::nullopt;
#else
    return Protocols::InteractionModel::Status::InvalidCommand;
#endif
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
std::optional<ActionReturnStatus>
NetworkCommissioningLogic::HandleAddOrUpdateWiFiNetworkWithPDC(HandlerContext & handler, const ConcreteCommandPath & commandPath,
                                                               const Commands::AddOrUpdateWiFiNetwork::DecodableType & req)
{
    // Credentials must be empty when configuring for PDC, it's only present to keep the command shape compatible.
    if (!req.credentials.empty())
    {
        handler.AddStatus(commandPath, Protocols::InteractionModel::Status::ConstraintError, "credentials");
        return std::nullopt;
    }

    auto && networkIdentity = req.networkIdentity.Value(); // presence checked by caller
    if (networkIdentity.size() > kMaxCHIPCompactNetworkIdentityLength ||
        Credentials::ValidateChipNetworkIdentity(networkIdentity) != CHIP_NO_ERROR)
    {
        handler.AddStatus(commandPath, Protocols::InteractionModel::Status::ConstraintError, "networkIdentity");
        return std::nullopt;
    }

    if (req.clientIdentifier.HasValue() && req.clientIdentifier.Value().size() != CertificateKeyId::size())
    {
        handler.AddStatus(commandPath, Protocols::InteractionModel::Status::ConstraintError, "clientIdentifier");
        return std::nullopt;
    }

    bool provePossession = req.possessionNonce.HasValue();
    if (provePossession && req.possessionNonce.Value().size() != kPossessionNonceSize)
    {
        handler.AddStatus(commandPath, Protocols::InteractionModel::Status::ConstraintError, "possessionNonce");
        return std::nullopt;
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
                handler.AddStatus(commandPath, Protocols::InteractionModel::Status::NotFound, "clientIdentifier");
                return std::nullopt;
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

            ReportNetworksListChanged();
            UpdateBreadcrumb(req.breadcrumb);
        }

        handler.AddResponse(commandPath, response);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "AddOrUpdateWiFiNetwork with PDC failed: %" CHIP_ERROR_FORMAT, err.Format());
        return Protocols::InteractionModel::Status::Failure;
    }
    return std::nullopt;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

std::optional<ActionReturnStatus>
NetworkCommissioningLogic::HandleAddOrUpdateThreadNetwork(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                                          const Commands::AddOrUpdateThreadNetwork::DecodableType & req)
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD

    MATTER_TRACE_SCOPE("HandleAddOrUpdateThreadNetwork", "NetworkCommissioning");

    RETURN_ERROR_STATUS_IF_FAILSAFE_NOT_ARMED(handler.GetAccessingFabricIndex());

    Commands::NetworkConfigResponse::Type response;
    DebugTextStorage debugTextBuffer;
    MutableCharSpan debugText(debugTextBuffer);
    uint8_t outNetworkIndex = 0;
    response.networkingStatus =
        mpDriver.Get<ThreadDriver *>()->AddOrUpdateNetwork(req.operationalDataset, debugText, outNetworkIndex);
    FillDebugTextAndNetworkIndex(response, debugText, outNetworkIndex);
    handler.AddResponse(commandPath, response);
    if (response.networkingStatus == Status::kSuccess)
    {
        ReportNetworksListChanged();
        UpdateBreadcrumb(req.breadcrumb);
    }
    return std::nullopt;
#else
    return Protocols::InteractionModel::Status::InvalidCommand;
#endif
}

void NetworkCommissioningLogic::UpdateBreadcrumb(const Optional<uint64_t> & breadcrumb)
{
    VerifyOrReturn(breadcrumb.HasValue());
    GeneralCommissioning::SetBreadcrumb(breadcrumb.Value());
}

void NetworkCommissioningLogic::CommitSavedBreadcrumb()
{
    // We rejected the command when there is another ongoing command, so mCurrentOperationBreadcrumb reflects the breadcrumb
    // argument in the only background command.
    UpdateBreadcrumb(mCurrentOperationBreadcrumb);
    mCurrentOperationBreadcrumb.ClearValue();
}

std::optional<ActionReturnStatus> NetworkCommissioningLogic::HandleRemoveNetwork(CommandHandler & handler,
                                                                                 const ConcreteCommandPath & commandPath,
                                                                                 const Commands::RemoveNetwork::DecodableType & req)
{
    MATTER_TRACE_SCOPE("HandleRemoveNetwork", "NetworkCommissioning");

    RETURN_ERROR_STATUS_IF_FAILSAFE_NOT_ARMED(handler.GetAccessingFabricIndex());

    Commands::NetworkConfigResponse::Type response;
    DebugTextStorage debugTextBuffer;
    MutableCharSpan debugText(debugTextBuffer);
    uint8_t outNetworkIndex   = 0;
    response.networkingStatus = mpWirelessDriver->RemoveNetwork(req.networkID, debugText, outNetworkIndex);
    FillDebugTextAndNetworkIndex(response, debugText, outNetworkIndex);
    handler.AddResponse(commandPath, response);
    if (response.networkingStatus == Status::kSuccess)
    {
        ReportNetworksListChanged();
        UpdateBreadcrumb(req.breadcrumb);

        // If no networks are left, clear-out errors;
        if (CountAndRelease(mpBaseDriver->GetNetworks()) == 0)
        {
            SetLastNetworkId(ByteSpan{});
            SetLastConnectErrorValue(NullNullable);
            SetLastNetworkingStatusValue(NullNullable);
        }
    }
    return std::nullopt;
}

std::optional<ActionReturnStatus>
NetworkCommissioningLogic::HandleConnectNetwork(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                                const Commands::ConnectNetwork::DecodableType & req)
{
    MATTER_TRACE_SCOPE("HandleConnectNetwork", "NetworkCommissioning");
    if (req.networkID.size() > kMaxNetworkIDLen)
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }

    RETURN_ERROR_STATUS_IF_FAILSAFE_NOT_ARMED(handler.GetAccessingFabricIndex());

    mConnectingNetworkIDLen = static_cast<uint8_t>(req.networkID.size());
    memcpy(mConnectingNetworkID, req.networkID.data(), mConnectingNetworkIDLen);
    mAsyncCommandPath           = commandPath;
    mAsyncCommandHandle         = CommandHandler::Handle(&handler);
    mCurrentOperationBreadcrumb = req.breadcrumb;

#if CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
    // Per spec, lingering connections on any other interfaces need to be disconnected at this point.
    for (auto & node : sInstances)
    {
        auto instance = static_cast<NetworkCommissioningLogic *>(&node);
        if (instance != this)
        {
            instance->DisconnectLingeringConnection();
        }
    }

    mpWirelessDriver->ConnectNetwork(req.networkID, this);
#else
    // In Non-concurrent mode postpone the final execution of ConnectNetwork until the operational
    // network has been fully brought up and kOperationalNetworkStarted is delivered.
    // mConnectingNetworkIDLen and mConnectingNetworkID contain the received SSID
    // As per spec, send the ConnectNetworkResponse(Success) prior to releasing the commissioning channel
    SendNonConcurrentConnectNetworkResponse();
#endif
    return std::nullopt;
}

std::optional<ActionReturnStatus> NetworkCommissioningLogic::HandleNonConcurrentConnectNetwork()
{
    ByteSpan nonConcurrentNetworkID = ByteSpan(mConnectingNetworkID, mConnectingNetworkIDLen);
    ChipLogProgress(NetworkProvisioning, "Non-concurrent mode, Connect to Network SSID=%.*s", mConnectingNetworkIDLen,
                    mConnectingNetworkID);
    mpWirelessDriver->ConnectNetwork(nonConcurrentNetworkID, this);
    return std::nullopt;
}

std::optional<ActionReturnStatus>
NetworkCommissioningLogic::HandleReorderNetwork(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                                const Commands::ReorderNetwork::DecodableType & req)
{
    MATTER_TRACE_SCOPE("HandleReorderNetwork", "NetworkCommissioning");
    Commands::NetworkConfigResponse::Type response;
    DebugTextStorage debugTextBuffer;
    MutableCharSpan debugText(debugTextBuffer);
    response.networkingStatus = mpWirelessDriver->ReorderNetwork(req.networkID, req.networkIndex, debugText);
    FillDebugTextAndNetworkIndex(response, debugText, req.networkIndex);
    handler.AddResponse(commandPath, response);
    if (response.networkingStatus == Status::kSuccess)
    {
        ReportNetworksListChanged();
        UpdateBreadcrumb(req.breadcrumb);
    }
    return std::nullopt;
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
std::optional<ActionReturnStatus> NetworkCommissioningLogic::HandleQueryIdentity(CommandHandler & handler,
                                                                                 const ConcreteCommandPath & commandPath,
                                                                                 const Commands::QueryIdentity::DecodableType & req)
{
    MATTER_TRACE_SCOPE("HandleQueryIdentity", "NetworkCommissioning");

    if (req.keyIdentifier.size() != CertificateKeyId::size())
    {
        handler.AddStatus(commandPath, Protocols::InteractionModel::Status::ConstraintError, "keyIdentifier");
        return std::nullopt;
    }
    CertificateKeyId keyIdentifier(req.keyIdentifier.data());

    bool provePossession = req.possessionNonce.HasValue();
    if (provePossession && req.possessionNonce.Value().size() != kPossessionNonceSize)
    {
        handler.AddStatus(commandPath, Protocols::InteractionModel::Status::ConstraintError, "possessionNonce");
        return std::nullopt;
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
        handler.AddResponse(commandPath, response);
    }

exit:
    if (networks != nullptr)
    {
        networks->Release();
    }
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "QueryIdentity failed: %" CHIP_ERROR_FORMAT, err.Format());
        return Protocols::InteractionModel::Status::Failure;
    }
    if (status != Protocols::InteractionModel::Status::Success)
    {
        return status;
    }
    // response was sent if error is CHIP_NO_ERROR
    return std::nullopt;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

#if CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
void NetworkCommissioningLogic::DisconnectLingeringConnection()
{
    bool haveConnectedNetwork = false;
    EnumerateAndRelease(mpBaseDriver->GetNetworks(), [&](const Network & network) {
        if (network.connected)
        {
            haveConnectedNetwork = true;
            return Loop::Break;
        }
        return Loop::Continue;
    });

    // If none of the configured networks is `connected`, we may have a
    // lingering connection to a different network that we need to disconnect.
    // Note: The driver may or may not be actually connected
    if (!haveConnectedNetwork)
    {
        LogErrorOnFailure(mpWirelessDriver->DisconnectFromNetwork());
    }
}
#endif

void NetworkCommissioningLogic::OnResult(Status commissioningError, CharSpan debugText, int32_t interfaceStatus)
{
    auto commandHandleRef = std::move(mAsyncCommandHandle);

    // In Non-concurrent mode the commandHandle will be null here, the ConnectNetworkResponse
    // has already been sent and the BLE will have been stopped, however the other functionality
    // is still required
#if CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
    auto commandHandle = commandHandleRef.Get();
    if (commandHandle == nullptr)
    {
        // When the platform shut down, interaction model engine will invalidate all commandHandle to avoid dangling references.
        // We may receive the callback after it and should make it noop.
        return;
    }
#endif // CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION

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
        SetLastConnectErrorValue(NullNullable);
    }
    else
    {
        response.errorValue.SetNonNull(interfaceStatus);
        SetLastConnectErrorValue(MakeNullable(interfaceStatus));
    }

    SetLastNetworkId(ByteSpan{ mConnectingNetworkID, mConnectingNetworkIDLen });
    SetLastNetworkingStatusValue(MakeNullable(commissioningError));

#if CONFIG_NETWORK_LAYER_BLE && !CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
    ChipLogProgress(NetworkProvisioning, "Non-concurrent mode, ConnectNetworkResponse will NOT be sent");
    // Do not send the ConnectNetworkResponse if in non-concurrent mode
    // TODO(#30576) raised to modify CommandHandler to notify it if no response required
    // -----> Is this required here: commandHandle->FinishCommand();
#else
    commandHandle->AddResponse(mAsyncCommandPath, response);
#endif // CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION

    if (commissioningError == Status::kSuccess)
    {
        CommitSavedBreadcrumb();
    }
}

void NetworkCommissioningLogic::OnFinished(Status status, CharSpan debugText, ThreadScanResponseIterator * networks)
{
    DEFER_AUTO_RELEASE(networks);

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    auto commandHandleRef = std::move(mAsyncCommandHandle);
    auto commandHandle    = commandHandleRef.Get();
    if (commandHandle == nullptr)
    {
        // When the platform shuts down, interaction model engine will invalidate all commandHandle to avoid dangling references.
        // We may receive the callback after it and should make it noop.
        return;
    }

    SetLastNetworkingStatusValue(MakeNullable(status));

    ThreadScanResponseToTLV responseBuilder(status, debugText, networks);
    commandHandle->AddResponse(mAsyncCommandPath, Commands::ScanNetworksResponse::Id, responseBuilder);

    if (status == Status::kSuccess)
    {
        CommitSavedBreadcrumb();
    }
#endif
}

void NetworkCommissioningLogic::OnFinished(Status status, CharSpan debugText, WiFiScanResponseIterator * networks)
{
    DEFER_AUTO_RELEASE(networks);

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION || CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
    auto commandHandleRef = std::move(mAsyncCommandHandle);
    auto commandHandle    = commandHandleRef.Get();
    if (commandHandle == nullptr)
    {
        // When the platform shuts down, interaction model engine will invalidate all commandHandle to avoid dangling references.
        // We may receive the callback after it and should make it noop.
        return;
    }

    // If drivers are failing to respond NetworkNotFound on empty results, force it for them.
    bool resultsMissing = !networks || (networks->Count() == 0);
    if ((status == Status::kSuccess) && mScanningWasDirected && resultsMissing)
    {
        status = Status::kNetworkNotFound;
    }

    SetLastNetworkingStatusValue(MakeNullable(status));

    WifiScanResponseToTLV responseBuilder(status, debugText, networks);
    commandHandle->AddResponse(mAsyncCommandPath, Commands::ScanNetworksResponse::Id, responseBuilder);

    if (status == Status::kSuccess)
    {
        CommitSavedBreadcrumb();
    }
#endif
}

void NetworkCommissioningLogic::OnPlatformEventHandler(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    auto this_ = reinterpret_cast<NetworkCommissioningLogic *>(arg);

    if (event->Type == DeviceLayer::DeviceEventType::kCommissioningComplete)
    {
        this_->OnCommissioningComplete();
    }
    else if (event->Type == DeviceLayer::DeviceEventType::kFailSafeTimerExpired)
    {
        this_->OnFailSafeTimerExpired();
    }
    else if ((event->Type == DeviceLayer::DeviceEventType::kWiFiDeviceAvailable) ||
             (event->Type == DeviceLayer::DeviceEventType::kOperationalNetworkStarted))

    {
        // In Non-Concurrent mode connect the operational channel, as BLE has been stopped
        this_->HandleNonConcurrentConnectNetwork();
    }
}

void NetworkCommissioningLogic::OnCommissioningComplete()
{
    VerifyOrReturn(mpWirelessDriver != nullptr);

    ChipLogDetail(Zcl, "Commissioning complete, notify platform driver to persist network credentials.");
    mpWirelessDriver->CommitConfiguration();
}

void NetworkCommissioningLogic::OnFailSafeTimerExpired()
{
    VerifyOrReturn(mpWirelessDriver != nullptr);

    ChipLogDetail(Zcl, "Failsafe timeout, tell platform driver to revert network credentials.");
    mpWirelessDriver->RevertConfiguration();
    mAsyncCommandHandle.Release();

    // Mark the network list changed since `mpWirelessDriver->RevertConfiguration()` may have updated it.
    ReportNetworksListChanged();

    // If no networks are left, clear-out errors;
    if (mpBaseDriver && (CountAndRelease(mpBaseDriver->GetNetworks()) == 0))
    {
        SetLastNetworkId(ByteSpan{});
        SetLastConnectErrorValue(NullNullable);
        SetLastNetworkingStatusValue(NullNullable);
    }
}

CHIP_ERROR NetworkCommissioningLogic::EncodeNetworks(AttributeValueEncoder & listEncoder) const
{
    return listEncoder.EncodeList([this](const auto & encoder) {
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
}

CHIP_ERROR NetworkCommissioningLogic::EncodeSupportedWiFiBands(AttributeValueEncoder & listEncoder) const
{
#if (CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION || CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP)
    if (mFeatureFlags.Has(Feature::kWiFiNetworkInterface))
    {
        return listEncoder.EncodeList([this](const auto & encoder) {
            uint32_t bands = mpDriver.Get<WiFiDriver *>()->GetSupportedWiFiBandsMask();

            // Extract every band from the bitmap of supported bands, starting positionally on the right.
            for (uint32_t band_bit_pos = 0; band_bit_pos < std::numeric_limits<uint32_t>::digits; ++band_bit_pos)
            {
                auto band_mask = static_cast<uint32_t>(1UL << band_bit_pos);
                if ((bands & band_mask) != 0)
                {
                    ReturnErrorOnFailure(encoder.Encode(static_cast<WiFiBandEnum>(band_bit_pos)));
                }
            }
            return CHIP_NO_ERROR;
        });
    }
#endif
    // attribute available IFF wifi supported (see above)
    return CHIP_ERROR_INCORRECT_STATE;
}

} // namespace Clusters
} // namespace app
} // namespace chip
