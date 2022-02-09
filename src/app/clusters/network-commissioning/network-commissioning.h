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
#include <lib/support/Variant.h>
#include <platform/NetworkCommissioning.h>
#include <platform/PlatformManager.h>
#include <platform/internal/DeviceNetworkInfo.h>

namespace chip {
namespace app {
namespace Clusters {
namespace NetworkCommissioning {

// TODO: Use macro to disable some wifi or thread
class Instance : public CommandHandlerInterface,
                 public AttributeAccessInterface,
                 public DeviceLayer::NetworkCommissioning::Internal::WirelessDriver::ConnectCallback,
                 public DeviceLayer::NetworkCommissioning::WiFiDriver::ScanCallback,
                 public DeviceLayer::NetworkCommissioning::ThreadDriver::ScanCallback
{
public:
    /**
     * Register will register the network commissioning instance to the attribute and command dispatching route.
     */
    CHIP_ERROR Init();
    CHIP_ERROR Shutdown();

    // CommandHandlerInterface
    void InvokeCommand(HandlerContext & ctx) override;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

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
    static void _OnCommissioningComplete(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg);
    void OnCommissioningComplete(CHIP_ERROR err);

    const BitFlags<NetworkCommissioningFeature> mFeatureFlags;

    DeviceLayer::NetworkCommissioning::Internal::WirelessDriver * const mpWirelessDriver;
    DeviceLayer::NetworkCommissioning::Internal::BaseDriver * const mpBaseDriver;

    Variant<DeviceLayer::NetworkCommissioning::WiFiDriver *, DeviceLayer::NetworkCommissioning::ThreadDriver *> mpDriver;

    app::CommandHandler::Handle mAsyncCommandHandle;

    ConcreteCommandPath mPath = ConcreteCommandPath(0, 0, 0);

    // Actual handlers of the commands
    void HandleScanNetworks(HandlerContext & ctx, const Commands::ScanNetworks::DecodableType & req);
    void HandleAddOrUpdateWiFiNetwork(HandlerContext & ctx, const Commands::AddOrUpdateWiFiNetwork::DecodableType & req);
    void HandleAddOrUpdateThreadNetwork(HandlerContext & ctx, const Commands::AddOrUpdateThreadNetwork::DecodableType & req);
    void HandleRemoveNetwork(HandlerContext & ctx, const Commands::RemoveNetwork::DecodableType & req);
    void HandleConnectNetwork(HandlerContext & ctx, const Commands::ConnectNetwork::DecodableType & req);
    void HandleReorderNetwork(HandlerContext & ctx, const Commands::ReorderNetwork::DecodableType & req);

public:
    Instance(EndpointId aEndpointId, DeviceLayer::NetworkCommissioning::WiFiDriver * apDelegate) :
        CommandHandlerInterface(Optional<EndpointId>(aEndpointId), Id),
        AttributeAccessInterface(Optional<EndpointId>(aEndpointId), Id),
        mFeatureFlags(NetworkCommissioningFeature::kWiFiNetworkInterface), mpWirelessDriver(apDelegate), mpBaseDriver(apDelegate)
    {
        mpDriver.Set<DeviceLayer::NetworkCommissioning::WiFiDriver *>(apDelegate);
    }

    Instance(EndpointId aEndpointId, DeviceLayer::NetworkCommissioning::ThreadDriver * apDelegate) :
        CommandHandlerInterface(Optional<EndpointId>(aEndpointId), Id),
        AttributeAccessInterface(Optional<EndpointId>(aEndpointId), Id),
        mFeatureFlags(NetworkCommissioningFeature::kThreadNetworkInterface), mpWirelessDriver(apDelegate), mpBaseDriver(apDelegate)
    {
        mpDriver.Set<DeviceLayer::NetworkCommissioning::ThreadDriver *>(apDelegate);
    }

    Instance(EndpointId aEndpointId, DeviceLayer::NetworkCommissioning::EthernetDriver * apDelegate) :
        CommandHandlerInterface(Optional<EndpointId>(aEndpointId), Id),
        AttributeAccessInterface(Optional<EndpointId>(aEndpointId), Id),
        mFeatureFlags(NetworkCommissioningFeature::kEthernetNetworkInterface), mpWirelessDriver(nullptr), mpBaseDriver(apDelegate)
    {}

    virtual ~Instance() = default;
};

} // namespace NetworkCommissioning
} // namespace Clusters
} // namespace app
} // namespace chip
