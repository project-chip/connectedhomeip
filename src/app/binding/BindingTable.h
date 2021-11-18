/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "app/OperationalDeviceProxy.h"
#include "lib/core/DataModelTypes.h"
#include "lib/dnssd/Resolver.h"
#include "lib/support/Pool.h"

namespace chip {
namespace app {

struct BindingTableEntry
{
    BindingTableEntry(EndpointId sourceEndpoint, NodeId peerNodeId, EndpointId peerEndpointId, FabricIndex peerFabricIndex,
                      ClusterId clusterId);

    EndpointId mSourceEndpoint;
    EndpointId mPeerEndpointId;
    PeerId mPeerId;
    GroupId mPeerGroupId; // Destination group, not used
    ClusterId mClusterId;
};

class BindingTable
{
public:
    BindingTable();

    CHIP_ERROR Add(EndpointId sourceEndpoint, NodeId peerNodeId, EndpointId peerEndpointId, FabricIndex peerFabricIndex,
                   ClusterId clusterId);

    CHIP_ERROR Remove(EndpointId sourceEndpoint, NodeId peerNodeId, EndpointId peerEndpointId, FabricIndex peerFabricIndex,
                      ClusterId clusterId);

    BindingTableEntry * FindBindingEntry(PeerId peer);

    BindingTableEntry * FindBindingEntry(NodeId node, FabricIndex fabricIndex);

    BindingTableEntry * FindBindingEntry(EndpointId endpoint, ClusterId cluster);

    OperationalDeviceProxy * FindPeerDevice(PeerId peer);

    OperationalDeviceProxy * FindPeerDevice(NodeId node, FabricIndex fabricIndex);

    CHIP_ERROR TryConnectDevice(OperationalDeviceProxy * device);

    static BindingTable & GetInstance() { return BindingTable::sInstance; }

private:
    static BindingTable sInstance;
    static constexpr int kMaxBinding = 3; // TODO: make it (3 * NUM_SUPPORTED_FABRICS)

    class ResolveDelegate : public Dnssd::ResolverDelegate
    {
    public:
        ResolveDelegate(BindingTable * bindingTable) : mBindingTable(bindingTable) {}

        void OnNodeIdResolved(const Dnssd::ResolvedNodeData & nodeData) override;

        void OnNodeIdResolutionFailed(const PeerId & peerId, CHIP_ERROR error) override;

        void OnNodeDiscoveryComplete(const Dnssd::DiscoveredNodeData & nodeData) override {}

    private:
        BindingTable * mBindingTable;
    };

    static void OnDeviceConnectedFn(void * context, chip::DeviceProxy * device);
    static void OnDeviceConnectionFailureFn(void * context, NodeId deviceId, CHIP_ERROR error);

    BitMapObjectPool<BindingTableEntry, kMaxBinding> mBindingEntries;
    BitMapObjectPool<OperationalDeviceProxy, kMaxBinding> mPeerDevices;
    ResolveDelegate mResolveDelegate;
    Callback::Callback<OnDeviceConnected> mOnDeviceConnected;
    Callback::Callback<OnDeviceConnectionFailure> mOnDeviceConnectionFailure;
};

void MatterBindingAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t mask, uint8_t type,
                                          uint16_t size, uint8_t * value);

} // namespace app
} // namespace chip

// User callback called when a bound cluster is changed. The peer device will be already connected.
extern void MatterBoundAttributeChangedCallback(const chip::app::BindingTableEntry & binding,
                                                chip::OperationalDeviceProxy * peerDevice,
                                                const chip::app::ConcreteAttributePath & attributePath, uint16_t size,
                                                uint8_t * value);
