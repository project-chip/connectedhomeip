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
#include <lib/support/ThreadOperationalDataset.h>
#include <platform/NetworkCommissioning.h>
#include <platform/internal/DeviceNetworkInfo.h>

namespace chip {
namespace app {
namespace Clusters {
namespace NetworkCommissioning {

// TODO: Use macro to disable some wifi or thread
class InstanceBase : public CommandHandlerInterface,
                     public DeviceLayer::WiFiNetworkCommissioningDelegate::Callback,
                     public DeviceLayer::ThreadNetworkCommissioningDelegate::Callback,
                     public AttributeAccessInterface
{
public:
    enum class NetworkType : uint8_t
    {
        kUndefined = 0,
        kWiFi      = 1,
        kThread    = 2,
        kEthernet  = 3,
    };

    static constexpr size_t kMaxNetworkIDLen = 32;

    struct WiFiNetworkInfo
    {
        uint8_t mSSID[DeviceLayer::Internal::kMaxWiFiSSIDLength + 1];
        uint8_t mSSIDLen;
        uint8_t mCredentials[DeviceLayer::Internal::kMaxWiFiKeyLength];
        uint8_t mCredentialsLen;
    };

    struct NetworkInfo
    {
        uint8_t mNetworkID[kMaxNetworkIDLen];
        uint8_t mNetworkIDLen;
        uint8_t mEnabled;
        NetworkType mNetworkType;
        union NetworkData
        {
            Thread::OperationalDataset mThread;
            WiFiNetworkInfo mWiFi;
        } mData;
    };

    // CommandHandlerInterface
    void InvokeCommand(HandlerContext & ctx) override;
    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

    void HandleScanNetworks(HandlerContext & ctx, const Commands::ScanNetworks::DecodableType & req);
    void HandleAddOrUpdateWiFiNetwork(HandlerContext & ctx, const Commands::AddOrUpdateWiFiNetwork::DecodableType & req);
    void HandleAddOrUpdateThreadNetwork(HandlerContext & ctx, const Commands::AddOrUpdateThreadNetwork::DecodableType & req);
    void HandleRemoveNetwork(HandlerContext & ctx, const Commands::RemoveNetwork::DecodableType & req);
    void HandleConnectNetwork(HandlerContext & ctx, const Commands::ConnectNetwork::DecodableType & req);
    void HandleReorderNetwork(HandlerContext & ctx, const Commands::ReorderNetwork::DecodableType & req);

    CHIP_ERROR DoConnectNetwork(NetworkInfo * network);

    void PrepareCommand(ConcreteCommandPath path, NetworkInfo * info);

    // DeviceLayer::WiFiNetworkCommissioningDelegate::Callback
    void OnConnectWiFiResult(app::Clusters::NetworkCommissioning::NetworkCommissioningStatus commissioningError, CharSpan errorText,
                             int32_t wifiAssociationStatus) override
    {
        OnConnectResult(commissioningError, errorText, wifiAssociationStatus);
    }
    void OnScanFinished(CHIP_ERROR err, CharSpan debugText, const Span<Structs::WiFiInterfaceScanResult::Type> & networks) override;

    // DeviceLayer::ThreadNetworkCommissioningDelegate::Callback
    void OnConnectThreadResult(app::Clusters::NetworkCommissioning::NetworkCommissioningStatus commissioningError,
                               CharSpan errorText, int32_t threadAssociationStatus) override
    {
        OnConnectResult(commissioningError, errorText, threadAssociationStatus);
    }
    void OnScanFinished(CHIP_ERROR err, CharSpan debugText,
                        const Span<Structs::ThreadInterfaceScanResult::Type> & networks) override;

private:
    void OnConnectResult(app::Clusters::NetworkCommissioning::NetworkCommissioningStatus commissioningError, CharSpan errorText,
                         int32_t networkRelatedAssociationStatus);
    CHIP_ERROR HandleReadNetworks(AttributeValueEncoder & aEncoder);
    CHIP_ERROR HandleReadScanMaxTimeSeconds(AttributeValueEncoder & aEncoder);
    CHIP_ERROR HandleReadConnectMaxTimeSeconds(AttributeValueEncoder & aEncoder);

    const NetworkType mType;

    NetworkInfo * mpNetworks;
    Attributes::MaxNetworks::TypeInfo::Type mMaxNetworks;

    DeviceLayer::WiFiNetworkCommissioningDelegate * mpWiFiDelegate;
    DeviceLayer::ThreadNetworkCommissioningDelegate * mpThreadDelegate;

    app::CommandHandler::Handle mAsyncCommandHandle;

    ConcreteCommandPath mPath                   = ConcreteCommandPath(0, 0, 0);
    NetworkInfo * mCurrentProcessingNetworkInfo = nullptr;

public:
    InstanceBase(EndpointId aEndpointId, NetworkInfo * apNetworkInfo, decltype(mMaxNetworks) aMaxNetworks,
                 DeviceLayer::WiFiNetworkCommissioningDelegate * apDelegate) :
        CommandHandlerInterface(Optional<EndpointId>(aEndpointId), Id),
        AttributeAccessInterface(Optional<EndpointId>(aEndpointId), Id), mType(NetworkType::kWiFi), mpNetworks(apNetworkInfo),
        mMaxNetworks(aMaxNetworks), mpWiFiDelegate(apDelegate)
    {}

    InstanceBase(EndpointId aEndpointId, NetworkInfo * apNetworkInfo, decltype(mMaxNetworks) aMaxNetworks,
                 DeviceLayer::ThreadNetworkCommissioningDelegate * apDelegate) :
        CommandHandlerInterface(Optional<EndpointId>(aEndpointId), Id),
        AttributeAccessInterface(Optional<EndpointId>(aEndpointId), Id), mType(NetworkType::kThread), mpNetworks(apNetworkInfo),
        mMaxNetworks(aMaxNetworks), mpThreadDelegate(apDelegate)
    {}

    virtual ~InstanceBase() = default;
};

class SingleNetworkInstance : public InstanceBase
{
public:
    SingleNetworkInstance(EndpointId aEndpointId, DeviceLayer::ThreadNetworkCommissioningDelegate * apDelegate) :
        InstanceBase(aEndpointId, &mNetwork, 1, apDelegate)
    {}

    SingleNetworkInstance(EndpointId aEndpointId, DeviceLayer::WiFiNetworkCommissioningDelegate * apDelegate) :
        InstanceBase(aEndpointId, &mNetwork, 1, apDelegate)
    {}

private:
    NetworkInfo mNetwork;
};

} // namespace NetworkCommissioning
} // namespace Clusters
} // namespace app
} // namespace chip
