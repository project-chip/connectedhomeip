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

#include "app/binding/BindingTable.h"

#include <app-common/zap-generated/attribute-type.h>
#include <app/server/Server.h>
#include <lib/support/logging/CHIPLogging.h>

namespace {

chip::FabricInfo * GetFabricInfoFromIndex(chip::FabricIndex fabricIndex)
{
    return chip::Server::GetInstance().GetFabricTable().FindFabricWithIndex(fabricIndex);
}

chip::PeerId BuildPeerId(chip::NodeId nodeId, chip::FabricIndex fabricIndex)
{
    return GetFabricInfoFromIndex(fabricIndex)->GetPeerIdForNode(nodeId);
}

}; // namespace

namespace chip {
namespace app {

BindingTable BindingTable::sInstance;

BindingTableEntry::BindingTableEntry(EndpointId sourceEndpoint, NodeId peerNodeId, EndpointId peerEndpointId,
                                     FabricIndex peerFabricIndex, ClusterId clusterId) :
    mSourceEndpoint(sourceEndpoint),
    mPeerEndpointId(peerEndpointId), mPeerId(BuildPeerId(peerNodeId, peerFabricIndex)), mClusterId(clusterId)
{}

BindingTable::BindingTable() :
    mResolveDelegate(this), mOnDeviceConnected(OnDeviceConnectedFn, this),
    mOnDeviceConnectionFailure(OnDeviceConnectionFailureFn, this)
{}

CHIP_ERROR BindingTable::Add(EndpointId sourceEndpoint, NodeId peerNodeId, EndpointId peerEndpointId, FabricIndex peerFabricIndex,
                             ClusterId clusterId)
{
    BindingTableEntry * entry =
        mBindingEntries.CreateObject(sourceEndpoint, peerNodeId, peerEndpointId, peerFabricIndex, clusterId);
    if (entry == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    if (FindPeerDevice(entry->mPeerId) == nullptr)
    {
        DeviceProxyInitParams initParams = { &Server::GetInstance().GetSecureSessionManager(),
                                             &Server::GetInstance().GetExchangeManager(),
                                             &Server::GetInstance().GetSessionIDAllocator(),
                                             GetFabricInfoFromIndex(peerFabricIndex) };
        OperationalDeviceProxy * device  = mPeerDevices.CreateObject(initParams, entry->mPeerId);
        if (device == nullptr)
        {
            mBindingEntries.ReleaseObject(entry);
            return CHIP_ERROR_NO_MEMORY;
        }
        return TryConnectDevice(device);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR BindingTable::Remove(EndpointId sourceEndpoint, NodeId peerNodeId, EndpointId peerEndpointId,
                                FabricIndex peerFabricIndex, ClusterId clusterId)
{
    BindingTableEntry * removeEntry = nullptr;
    int peerNumBindings             = 0;
    PeerId peer                     = BuildPeerId(peerNodeId, peerFabricIndex);
    mBindingEntries.ForEachActiveObject(
        [&removeEntry, &peerNumBindings, sourceEndpoint, peer, peerEndpointId, clusterId](BindingTableEntry * entry) {
            if (entry->mSourceEndpoint == sourceEndpoint && entry->mPeerId == peer && entry->mPeerEndpointId == peerEndpointId &&
                entry->mClusterId == clusterId)
            {
                removeEntry = entry;
            }
            if (entry->mPeerId == peer)
            {
                peerNumBindings++;
            }
            return true;
        });
    if (removeEntry == nullptr)
    {
        return CHIP_ERROR_KEY_NOT_FOUND;
    }
    mBindingEntries.ReleaseObject(removeEntry);
    // Last binding with the peer, close the connection.
    if (peerNumBindings == 1)
    {
        OperationalDeviceProxy * device = FindPeerDevice(peer);
        if (device)
        {
            mPeerDevices.ReleaseObject(device);
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR BindingTable::TryConnectDevice(OperationalDeviceProxy * device)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (device->IsConnected())
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    if (device->IsConnecting())
    {
        return CHIP_NO_ERROR;
    }
    Dnssd::Resolver::Instance().SetResolverDelegate(&mResolveDelegate);
    CHIP_ERROR ret = Dnssd::Resolver::Instance().ResolveNodeId(device->GetPeerId(), Inet::IPAddressType::kAny);
    return ret;
}

BindingTableEntry * BindingTable::FindBindingEntry(NodeId node, FabricIndex fabricIndex)
{
    return FindBindingEntry(BuildPeerId(node, fabricIndex));
}

BindingTableEntry * BindingTable::FindBindingEntry(PeerId peer)
{
    BindingTableEntry * foundEntry = nullptr;
    mBindingEntries.ForEachActiveObject([&foundEntry, peer](BindingTableEntry * entry) {
        if (entry->mPeerId == peer)
        {
            foundEntry = entry;
            return false;
        }
        return true;
    });
    return foundEntry;
}

BindingTableEntry * BindingTable::FindBindingEntry(EndpointId endpoint, ClusterId cluster)
{
    BindingTableEntry * foundEntry = nullptr;
    mBindingEntries.ForEachActiveObject([&foundEntry, endpoint, cluster](BindingTableEntry * entry) {
        printf("source endpoint %d, cluster %d\n", entry->mSourceEndpoint, entry->mClusterId);
        if (entry->mSourceEndpoint == endpoint && entry->mClusterId == cluster)
        {
            foundEntry = entry;
            return false;
        }
        return true;
    });
    return foundEntry;
}

OperationalDeviceProxy * BindingTable::FindPeerDevice(NodeId node, FabricIndex fabricIndex)
{
    return FindPeerDevice(BuildPeerId(node, fabricIndex));
}

OperationalDeviceProxy * BindingTable::FindPeerDevice(PeerId peer)
{
    OperationalDeviceProxy * foundDevice = nullptr;
    mPeerDevices.ForEachActiveObject([&foundDevice, peer](OperationalDeviceProxy * device) {
        if (device->GetPeerId() == peer)
        {
            foundDevice = device;
            return false;
        }
        return true;
    });
    return foundDevice;
}

void BindingTable::ResolveDelegate::OnNodeIdResolved(const Dnssd::ResolvedNodeData & nodeData)
{
    OperationalDeviceProxy * device = mBindingTable->FindPeerDevice(nodeData.mPeerId);
    if (nodeData.mNumIPs > 0 && device != nullptr && !device->IsConnected() && !device->IsConnecting())
    {
        ChipLogProgress(AppServer, "BindingTable: NodeId resolved for 0xdevice %" PRIX64, nodeData.mPeerId.GetNodeId());
        Transport::PeerAddress addr(nodeData.mAddress[0], Transport::Type::kUdp);
        addr.SetInterface(nodeData.mInterfaceId);
        device->UpdateDeviceData(addr, nodeData.mMrpRetryIntervalIdle, nodeData.mMrpRetryIntervalActive);
        device->Connect(&mBindingTable->mOnDeviceConnected, &mBindingTable->mOnDeviceConnectionFailure);
    }
}

void BindingTable::ResolveDelegate::OnNodeIdResolutionFailed(const PeerId & peerId, CHIP_ERROR error)
{
    ChipLogError(AppServer, "Failed to resolve device 0x%" PRIX64, peerId.GetNodeId());
}

void BindingTable::OnDeviceConnectedFn(void * context, chip::DeviceProxy * device)
{
    ChipLogProgress(AppServer, "Connected to device 0x%" PRIX64, device->GetDeviceId());
}

void BindingTable::OnDeviceConnectionFailureFn(void * context, NodeId deviceId, CHIP_ERROR error)
{
    ChipLogError(AppServer, "Failed to connect to device 0x%" PRIX64, deviceId);
}

void MatterBindingAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t mask, uint8_t type,
                                          uint16_t size, uint8_t * value)
{
    printf("Find binding entry for endpoint %d cluster %d\n", attributePath.mEndpointId, attributePath.mClusterId);
    BindingTableEntry * entry = BindingTable::GetInstance().FindBindingEntry(attributePath.mEndpointId, attributePath.mClusterId);
    if (entry)
    {
        OperationalDeviceProxy * device = BindingTable::GetInstance().FindPeerDevice(entry->mPeerId);
        if (device && device->IsConnected())
        {
            MatterBoundAttributeChangedCallback(*entry, device, attributePath, size, value);
        }
    }
}

} // namespace app
} // namespace chip

// Weak implementation for devices without binding
__attribute__((weak)) void MatterBoundAttributeChangedCallback(const chip::app::BindingTableEntry & binding,
                                                               chip::OperationalDeviceProxy * peerDevice,
                                                               const chip::app::ConcreteAttributePath & attributePath,
                                                               uint16_t size, uint8_t * value)
{}
