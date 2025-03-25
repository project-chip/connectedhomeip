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

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/data-model/Nullable.h>
#include <lib/support/IntrusiveList.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <lib/support/Variant.h>
#include <platform/NetworkCommissioning.h>
#include <platform/PlatformManager.h>
#include <platform/internal/DeviceNetworkInfo.h>

namespace chip {
namespace app {
namespace Clusters {
namespace NetworkCommissioning {

// Instance inherits privately from this class to participate in Instance::sInstances
class InstanceListNode : public IntrusiveListNodeBase<>
{
};

// TODO: Use macro to disable some wifi or thread
class Instance : public CommandHandlerInterface,
                 public AttributeAccessInterface,
#if CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
                 private InstanceListNode,
#endif
                 public DeviceLayer::NetworkCommissioning::Internal::BaseDriver::NetworkStatusChangeCallback,
                 public DeviceLayer::NetworkCommissioning::Internal::WirelessDriver::ConnectCallback,
                 public DeviceLayer::NetworkCommissioning::WiFiDriver::ScanCallback,
                 public DeviceLayer::NetworkCommissioning::ThreadDriver::ScanCallback
{
public:
    /**
     * Register will register the network commissioning instance to the attribute and command dispatching route.
     */
    CHIP_ERROR Init();
    void Shutdown();

    // CommandHandlerInterface
    void InvokeCommand(HandlerContext & ctx) override;
    CHIP_ERROR EnumerateAcceptedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context) override;
    CHIP_ERROR EnumerateGeneratedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context) override;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

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

private:
    static void OnPlatformEventHandler(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg);
    void OnCommissioningComplete();
    void OnFailSafeTimerExpired();
#if !CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
    void SendNonConcurrentConnectNetworkResponse();
#endif

// TODO: This could be guarded by a separate multi-interface condition instead
#if CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
    class NetworkInstanceList : public IntrusiveList<InstanceListNode>
    {
    public:
        ~NetworkInstanceList() { this->Clear(); }
    };

    static NetworkInstanceList sInstances;
#endif

    EndpointId mEndpointId = kInvalidEndpointId;
    const BitFlags<Feature> mFeatureFlags;

    DeviceLayer::NetworkCommissioning::Internal::WirelessDriver * const mpWirelessDriver;
    DeviceLayer::NetworkCommissioning::Internal::BaseDriver * const mpBaseDriver;

    Variant<DeviceLayer::NetworkCommissioning::WiFiDriver *, DeviceLayer::NetworkCommissioning::ThreadDriver *> mpDriver;

    app::CommandHandler::Handle mAsyncCommandHandle;

    ConcreteCommandPath mPath = ConcreteCommandPath(0, 0, 0);

    // Last* attributes
    // Setting these values don't have to care about parallel requests, since we will reject other requests when there is another
    // request ongoing.
    // These values can be updated via OnNetworkingStatusChange callback, ScanCallback::OnFinished and ConnectCallback::OnResult.
    Attributes::LastNetworkingStatus::TypeInfo::Type mLastNetworkingStatusValue;
    Attributes::LastConnectErrorValue::TypeInfo::Type mLastConnectErrorValue;
    uint8_t mConnectingNetworkID[DeviceLayer::NetworkCommissioning::kMaxNetworkIDLen];
    uint8_t mConnectingNetworkIDLen = 0;
    uint8_t mLastNetworkID[DeviceLayer::NetworkCommissioning::kMaxNetworkIDLen];
    uint8_t mLastNetworkIDLen = 0;
    Optional<uint64_t> mCurrentOperationBreadcrumb;
    bool mScanningWasDirected = false;

    void SetLastNetworkingStatusValue(Attributes::LastNetworkingStatus::TypeInfo::Type networkingStatusValue);
    void SetLastConnectErrorValue(Attributes::LastConnectErrorValue::TypeInfo::Type connectErrorValue);
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

    // Actual handlers of the commands
    void HandleScanNetworks(HandlerContext & ctx, const Commands::ScanNetworks::DecodableType & req);
    void HandleAddOrUpdateWiFiNetwork(HandlerContext & ctx, const Commands::AddOrUpdateWiFiNetwork::DecodableType & req);
    void HandleAddOrUpdateWiFiNetworkWithPDC(HandlerContext & ctx, const Commands::AddOrUpdateWiFiNetwork::DecodableType & req);
    void HandleAddOrUpdateThreadNetwork(HandlerContext & ctx, const Commands::AddOrUpdateThreadNetwork::DecodableType & req);
    void HandleRemoveNetwork(HandlerContext & ctx, const Commands::RemoveNetwork::DecodableType & req);
    void HandleConnectNetwork(HandlerContext & ctx, const Commands::ConnectNetwork::DecodableType & req);
    void HandleReorderNetwork(HandlerContext & ctx, const Commands::ReorderNetwork::DecodableType & req);
    void HandleNonConcurrentConnectNetwork(void);
    void HandleQueryIdentity(HandlerContext & ctx, const Commands::QueryIdentity::DecodableType & req);

public:
    Instance(EndpointId aEndpointId, DeviceLayer::NetworkCommissioning::WiFiDriver * apDelegate);
    Instance(EndpointId aEndpointId, DeviceLayer::NetworkCommissioning::ThreadDriver * apDelegate);
    Instance(EndpointId aEndpointId, DeviceLayer::NetworkCommissioning::EthernetDriver * apDelegate);
    virtual ~Instance()
    {
#if CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
        if (IsInList())
        {
            sInstances.Remove(this);
        }
#endif
    }
};

// NetworkDriver for the devices that don't have / don't need a real network driver.
class NullNetworkDriver : public DeviceLayer::NetworkCommissioning::EthernetDriver
{
public:
    uint8_t GetMaxNetworks() override;

    DeviceLayer::NetworkCommissioning::NetworkIterator * GetNetworks() override;

    bool GetEnabled() override;

    virtual ~NullNetworkDriver() = default;
};

} // namespace NetworkCommissioning
} // namespace Clusters
} // namespace app
} // namespace chip
