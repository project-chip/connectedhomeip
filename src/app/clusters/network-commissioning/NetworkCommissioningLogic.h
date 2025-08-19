/*
 *    Copyright (c) 2021-2025 Project CHIP Authors
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

#include <app/AttributeValueEncoder.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model/Nullable.h>
#include <clusters/NetworkCommissioning/Attributes.h>
#include <clusters/NetworkCommissioning/Commands.h>
#include <lib/core/CHIPError.h>
#include <lib/support/IntrusiveList.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <lib/support/Variant.h>
#include <platform/NetworkCommissioning.h>
#include <platform/PlatformManager.h>
#include <platform/internal/DeviceNetworkInfo.h>

#include <optional>

namespace chip {
namespace app {
namespace Clusters {

// NetworkCommissioningLogic inherits privately from this class to participate in NetworkCommissioningLogic::sInstances
#if CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
class NetworkCommissioningLogicListNode : public IntrusiveListNodeBase<>
{
};
#else
class NetworkCommissioningLogicListNode
{
};
#endif

// This contains the NetworkCommissioning logic for a cluster
//
// TODO: this originates as a copy of the AttributeAccessInterface / CommandHandlerInterface
//       based implementation, so the integrations are significantly separated out
class NetworkCommissioningLogic : private NetworkCommissioningLogicListNode,
                                  public DeviceLayer::NetworkCommissioning::Internal::BaseDriver::NetworkStatusChangeCallback,
                                  public DeviceLayer::NetworkCommissioning::Internal::WirelessDriver::ConnectCallback,
                                  public DeviceLayer::NetworkCommissioning::WiFiDriver::ScanCallback,
                                  public DeviceLayer::NetworkCommissioning::ThreadDriver::ScanCallback
{
public:
    using ThreadDriver   = DeviceLayer::NetworkCommissioning::ThreadDriver;
    using WiFiDriver     = DeviceLayer::NetworkCommissioning::WiFiDriver;
    using EthernetDriver = DeviceLayer::NetworkCommissioning::EthernetDriver;

    CHIP_ERROR Init();
    void Shutdown();

    // BaseDriver::NetworkStatusChangeCallback
    void OnNetworkingStatusChange(DeviceLayer::NetworkCommissioning::Status aCommissioningError, Optional<ByteSpan> aNetworkId,
                                  Optional<int32_t> aConnectStatus) override;

    // WirelessDriver::ConnectCallback
    void OnResult(DeviceLayer::NetworkCommissioning::Status commissioningError, CharSpan errorText,
                  int32_t interfaceStatus) override;

    // WiFiDriver::ScanCallback
    void OnFinished(DeviceLayer::NetworkCommissioning::Status err, CharSpan debugText,
                    DeviceLayer::NetworkCommissioning::WiFiScanResponseIterator * networks) override;

    // ThreadDriver::ScanCallback
    void OnFinished(DeviceLayer::NetworkCommissioning::Status err, CharSpan debugText,
                    DeviceLayer::NetworkCommissioning::ThreadScanResponseIterator * networks) override;

    // Actual handlers of the commands
    std::optional<DataModel::ActionReturnStatus>
    HandleScanNetworks(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                       const NetworkCommissioning::Commands::ScanNetworks::DecodableType & req);
    std::optional<DataModel::ActionReturnStatus>
    HandleAddOrUpdateWiFiNetwork(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                 const NetworkCommissioning::Commands::AddOrUpdateWiFiNetwork::DecodableType & req);
    std::optional<DataModel::ActionReturnStatus>
    HandleAddOrUpdateThreadNetwork(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                   const NetworkCommissioning::Commands::AddOrUpdateThreadNetwork::DecodableType & req);
    std::optional<DataModel::ActionReturnStatus>
    HandleRemoveNetwork(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                        const NetworkCommissioning::Commands::RemoveNetwork::DecodableType & req);
    std::optional<DataModel::ActionReturnStatus>
    HandleConnectNetwork(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                         const NetworkCommissioning::Commands::ConnectNetwork::DecodableType & req);
    std::optional<DataModel::ActionReturnStatus>
    HandleReorderNetwork(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                         const NetworkCommissioning::Commands::ReorderNetwork::DecodableType & req);
    std::optional<DataModel::ActionReturnStatus> HandleNonConcurrentConnectNetwork();

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    std::optional<DataModel::ActionReturnStatus>
    HandleQueryIdentity(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                        const NetworkCommissioning::Commands::QueryIdentity::DecodableType & req);

    std::optional<DataModel::ActionReturnStatus>
    HandleAddOrUpdateWiFiNetworkWithPDC(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                        const NetworkCommissioning::Commands::AddOrUpdateWiFiNetwork::DecodableType & req);
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

    // Attribute handling
    CHIP_ERROR SetInterfaceEnabled(bool enabled) { return mpBaseDriver->SetEnabled(enabled); }
    uint8_t GetMaxNetworks() const { return mpBaseDriver->GetMaxNetworks(); }
    uint8_t GetScanNetworkTimeoutSeconds() const
    {
        return mpWirelessDriver != nullptr ? mpWirelessDriver->GetScanNetworkTimeoutSeconds() : 0;
    }
    uint8_t GetConnectNetworkTimeoutSeconds() const
    {
        return mpWirelessDriver != nullptr ? mpWirelessDriver->GetConnectNetworkTimeoutSeconds() : 0;
    }
    bool GetInterfaceEnabled() const { return mpBaseDriver->GetEnabled(); }
    NetworkCommissioning::Attributes::LastNetworkingStatus::TypeInfo::Type GetLastNetworkingStatus() const
    {
        return mLastNetworkingStatusValue;
    }
    NetworkCommissioning::Attributes::LastConnectErrorValue::TypeInfo::Type GetLastConnectErrorValue() const
    {
        return mLastConnectErrorValue;
    }
    ByteSpan GetLastNetworkID() const { return { mLastNetworkID, mLastNetworkIDLen }; }

    uint16_t GetThreadVersion() const
    {
#if (CHIP_DEVICE_CONFIG_ENABLE_THREAD)
        if (mFeatureFlags.Has(NetworkCommissioning::Feature::kThreadNetworkInterface))
        {
            return mpDriver.Get<ThreadDriver *>()->GetThreadVersion();
        }
#endif
        return 0;
    }

    BitMask<DeviceLayer::NetworkCommissioning::ThreadCapabilities> GetThreadCapabilities() const
    {
#if (CHIP_DEVICE_CONFIG_ENABLE_THREAD)
        if (mFeatureFlags.Has(NetworkCommissioning::Feature::kThreadNetworkInterface))
        {
            return mpDriver.Get<ThreadDriver *>()->GetSupportedThreadFeatures();
        }
#endif
        return {};
    }

    CHIP_ERROR EncodeSupportedThreadFeatures(AttributeValueEncoder & encoder) const;
    CHIP_ERROR EncodeNetworks(AttributeValueEncoder & encoder) const;
    CHIP_ERROR EncodeSupportedWiFiBands(AttributeValueEncoder & encoder) const;

    const BitFlags<NetworkCommissioning::Feature> & Features() const { return mFeatureFlags; }

    // Command handling
    bool IsProcessingAsyncCommand() const { return mAsyncCommandHandle.IsValid(); }

private:
    static void OnPlatformEventHandler(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg);
    void OnCommissioningComplete();
    void OnFailSafeTimerExpired();
#if !CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
    void SendNonConcurrentConnectNetworkResponse();
#endif

// TODO: This could be guarded by a separate multi-interface condition instead
#if CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
    class NetworkInstanceList : public IntrusiveList<NetworkCommissioningLogicListNode>
    {
    public:
        ~NetworkInstanceList() { this->Clear(); }
    };

    static NetworkInstanceList sInstances;
#endif

    EndpointId mEndpointId = kInvalidEndpointId;
    const BitFlags<NetworkCommissioning::Feature> mFeatureFlags;

    DeviceLayer::NetworkCommissioning::Internal::WirelessDriver * const mpWirelessDriver;
    DeviceLayer::NetworkCommissioning::Internal::BaseDriver * const mpBaseDriver;

    Variant<WiFiDriver *, ThreadDriver *> mpDriver;

    app::CommandHandler::Handle mAsyncCommandHandle;
    ConcreteCommandPath mAsyncCommandPath;

    // Last* attributes
    // Setting these values don't have to care about parallel requests, since we will reject other requests when there is
    // another request ongoing. These values can be updated via OnNetworkingStatusChange callback, ScanCallback::OnFinished and
    // ConnectCallback::OnResult.
    NetworkCommissioning::Attributes::LastNetworkingStatus::TypeInfo::Type mLastNetworkingStatusValue;
    NetworkCommissioning::Attributes::LastConnectErrorValue::TypeInfo::Type mLastConnectErrorValue;
    uint8_t mConnectingNetworkID[DeviceLayer::NetworkCommissioning::kMaxNetworkIDLen];
    uint8_t mConnectingNetworkIDLen = 0;
    uint8_t mLastNetworkID[DeviceLayer::NetworkCommissioning::kMaxNetworkIDLen];
    uint8_t mLastNetworkIDLen = 0;
    Optional<uint64_t> mCurrentOperationBreadcrumb;
    bool mScanningWasDirected = false;

    void SetLastNetworkingStatusValue(NetworkCommissioning::Attributes::LastNetworkingStatus::TypeInfo::Type networkingStatusValue);
    void SetLastConnectErrorValue(NetworkCommissioning::Attributes::LastConnectErrorValue::TypeInfo::Type connectErrorValue);
    void SetLastNetworkId(ByteSpan lastNetworkId);
    void ReportNetworksListChanged() const;

#if CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
    // Disconnect if the current connection is not in the Networks list
    void DisconnectLingeringConnection();
#endif

    // Commits the breadcrumb value saved in mCurrentOperationBreadcrumb to the breadcrumb attribute in GeneralCommissioning
    // cluster. Will set mCurrentOperationBreadcrumb to NullOptional.
    void CommitSavedBreadcrumb();

    // Sets the breadcrumb attribute in GeneralCommissioning cluster, no-op when breadcrumbValue is NullOptional.
    void UpdateBreadcrumb(const Optional<uint64_t> & breadcrumbValue);

public:
    NetworkCommissioningLogic(EndpointId aEndpointId, WiFiDriver * apDelegate);
    NetworkCommissioningLogic(EndpointId aEndpointId, ThreadDriver * apDelegate);
    NetworkCommissioningLogic(EndpointId aEndpointId, EthernetDriver * apDelegate);
    virtual ~NetworkCommissioningLogic()
    {
#if CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
        if (IsInList())
        {
            sInstances.Remove(this);
        }
#endif
    }
};

} // namespace Clusters
} // namespace app
} // namespace chip
