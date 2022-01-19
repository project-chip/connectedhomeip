/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include <app/OperationalDeviceManager.h>
#include <platform/CHIPDeviceLayer.h>

namespace {

struct ConnectionErrorContext
{
    chip::OperationalDeviceProxy * device;
    CHIP_ERROR error;
};

} // namespace

namespace chip {

CHIP_ERROR OperationalDeviceManager::AcquireDevice(PeerId peerId, Callback::Callback<OnDeviceConnected> * onConnection,
                                                   Callback::Callback<OnDeviceConnectionFailure> * onFailure)
{
    Dnssd::ResolvedNodeData resolutionData;

    bool nodeIDWasResolved = (mConfig.dnsCache != nullptr && mConfig.dnsCache->Lookup(peerId, resolutionData) == CHIP_NO_ERROR);

    OperationalDeviceProxy * device = FindActiveDevice(peerId);
    if (device == nullptr)
    {
        if (nodeIDWasResolved)
        {
            device = mConfig.devicePool->Allocate(mConfig.sessionInitParams, peerId, resolutionData);
        }
        else
        {
            device = mConfig.devicePool->Allocate(mConfig.sessionInitParams, peerId);
        }

        if (device == nullptr)
        {
            onFailure->mCall(onFailure->mContext, nullptr, peerId, CHIP_ERROR_NO_MEMORY);
            return CHIP_ERROR_NO_MEMORY;
        }
        device->EnqueueConnectionCallbacks(&mOnConnectedCallback, &mOnConnectionFailureCallback);
        return device->Connect(onConnection, onFailure, mConfig.dnsResolver);
    }
    else
    {
        OperationalDeviceProxy * newDevice = mConfig.devicePool->Allocate(mConfig.sessionInitParams, peerId);
        if (newDevice == nullptr)
        {
            onFailure->mCall(onFailure->mContext, nullptr, peerId, CHIP_ERROR_NO_MEMORY);
            return CHIP_ERROR_NO_MEMORY;
        }
        // Try to reuse the session
        if (device->IsConnected())
        {
            newDevice->SetConnectedSession(device->GetSecureSession().Value());
            onConnection->mCall(onConnection->mContext, newDevice);
        }
        // Otherwise wait for the outgoing CASESession to be connected
    }
    return CHIP_NO_ERROR;
}

void OperationalDeviceManager::ReleaseDevice(OperationalDeviceProxy * device)
{
    if (device != nullptr)
    {
        mConfig.devicePool->Release(device);
    }
}

static Loop NotifyDeviceConnected(const void * context, OperationalDeviceProxy * device)
{
    const OperationalDeviceProxy * connectedDevice = static_cast<const OperationalDeviceProxy *>(context);
    if (device != connectedDevice && !device->IsActive() && device->GetPeerId() == connectedDevice->GetPeerId())
    {
        device->SetConnectedSession(connectedDevice->GetSecureSession().Value());
    }
    return Loop::Continue;
}

void OperationalDeviceManager::HandleDeviceConnected(void * context, OperationalDeviceProxy * device)
{
    OperationalDeviceManager * manager = static_cast<OperationalDeviceManager *>(context);
    if (!manager->mNotifyingConnectionSucess)
    {
        manager->mNotifyingConnectionSucess = true;
        manager->mConfig.devicePool->ForEachDevice(device, NotifyDeviceConnected);
        manager->mNotifyingConnectionSucess = false;
    }
}

static Loop NotifyConnectionFailure(const void * context, OperationalDeviceProxy * device)
{
    const ConnectionErrorContext * errorContext = static_cast<const ConnectionErrorContext *>(context);
    if (device != errorContext->device && device->IsActive() && device->GetPeerId() == errorContext->device->GetPeerId())
    {
        device->NotifyConnectionFailure(errorContext->error);
    }
    return Loop::Continue;
}

void OperationalDeviceManager::HandleDeviceConnectionFailure(void * context, OperationalDeviceProxy * device, PeerId peerId,
                                                             CHIP_ERROR error)
{
    ConnectionErrorContext errorContext = {
        .device = device,
        .error  = error,
    };
    OperationalDeviceManager * manager = static_cast<OperationalDeviceManager *>(context);
    manager->mConfig.devicePool->ForEachDevice(&errorContext, NotifyConnectionFailure);
}

CHIP_ERROR OperationalDeviceManager::ResolveDeviceAddress(FabricInfo * fabric, NodeId nodeId)
{
    return mConfig.dnsResolver->ResolveNodeId(fabric->GetPeerIdForNode(nodeId), Inet::IPAddressType::kAny,
                                              Dnssd::Resolver::CacheBypass::On);
}

static Loop NotifyResolvedNodeData(const void * context, OperationalDeviceProxy * device)
{
    const Dnssd::ResolvedNodeData * nodeData = static_cast<const Dnssd::ResolvedNodeData *>(context);
    LogErrorOnFailure(device->UpdateDeviceData(device->ToPeerAddress(*nodeData), nodeData->GetMRPConfig()));
    return Loop::Continue;
}

void OperationalDeviceManager::OnNodeIdResolved(const Dnssd::ResolvedNodeData & nodeData)
{
    ChipLogProgress(Controller, "Address resolved for node: 0x" ChipLogFormatX64, ChipLogValueX64(nodeData.mPeerId.GetNodeId()));

    if (mConfig.dnsCache != nullptr)
    {
        LogErrorOnFailure(mConfig.dnsCache->Insert(nodeData));
    }

    mConfig.devicePool->ForEachDevice(&nodeData, NotifyResolvedNodeData);
}

void OperationalDeviceManager::OnNodeIdResolutionFailed(const PeerId & peer, CHIP_ERROR error)
{
    ChipLogError(Controller, "Error resolving node id: %s", ErrorStr(error));
}

CHIP_ERROR OperationalDeviceManager::GetPeerAddress(PeerId peerId, Transport::PeerAddress & addr)
{
    if (mConfig.dnsCache != nullptr)
    {
        Dnssd::ResolvedNodeData resolutionData;
        ReturnErrorOnFailure(mConfig.dnsCache->Lookup(peerId, resolutionData));
        addr = OperationalDeviceProxy::ToPeerAddress(resolutionData);
        return CHIP_NO_ERROR;
    }

    OperationalDeviceProxy * device = FindActiveDevice(peerId);
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_NOT_CONNECTED);
    addr = device->GetPeerAddress();
    return CHIP_NO_ERROR;
}

OperationalDeviceProxy * OperationalDeviceManager::FindActiveDevice(PeerId peerId)
{
    return mConfig.devicePool->FindFirstActiveDevice(peerId);
}

} // namespace chip
