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

#include <app/CASEDeviceManager.h>
#include <lib/address_resolve/AddressResolve.h>

namespace chip {

CHIP_ERROR CASEDeviceManager::Init(chip::System::Layer * systemLayer, const CASEDeviceManagerConfig & params)
{
    ReturnErrorOnFailure(params.deviceInitParams.Validate());
    mConfig = params;
    return AddressResolve::Resolver::Instance().Init(systemLayer);
}

void CASEDeviceManager::FindOrInitializeDevice(PeerId peerId, Callback::Callback<OnDeviceConnected> * onConnection,
                                               Callback::Callback<OnDeviceConnectionFailure> * onFailure)
{
    ChipLogDetail(CASEDeviceManager, "FindOrInitializeDevice: PeerId = " ChipLogFormatX64 ":" ChipLogFormatX64,
                  ChipLogValueX64(peerId.GetCompressedFabricId()), ChipLogValueX64(peerId.GetNodeId()));

    OperationalDeviceProxy * device = FindExistingDevice(peerId);
    if (device == nullptr)
    {
        ChipLogDetail(CASEDeviceManager, "FindOrInitializeDevice: No existing OperationalDeviceProxy instance found");

        device = mConfig.devicePool->Allocate(mConfig.deviceInitParams, peerId);

        if (device == nullptr)
        {
            if (onFailure != nullptr)
            {
                onFailure->mCall(onFailure->mContext, peerId, CHIP_ERROR_NO_MEMORY);
            }
            return;
        }
    }

    device->Connect(onConnection, onFailure);
    if (!device->IsConnected() && !device->IsConnecting() && !device->IsResolvingAddress())
    {
        // This device is not making progress toward anything.  It will have
        // notified the consumer about the failure already via the provided
        // callbacks, if any.
        //
        // Release the peer rather than the pointer in case the failure handler
        // has already released the device.
        ReleaseDevice(peerId);
    }
}

void CASEDeviceManager::ReleaseDevice(PeerId peerId)
{
    ReleaseDevice(FindExistingDevice(peerId));
}

void CASEDeviceManager::ReleaseDevicesForFabric(FabricIndex fabricIndex)
{
    mConfig.devicePool->ReleaseDevicesForFabric(fabricIndex);
}

void CASEDeviceManager::ReleaseAllDevices()
{
    mConfig.devicePool->ReleaseAllDevices();
}

CHIP_ERROR CASEDeviceManager::GetPeerAddress(PeerId peerId, Transport::PeerAddress & addr)
{
    OperationalDeviceProxy * device = FindExistingDevice(peerId);
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_NOT_CONNECTED);
    addr = device->GetPeerAddress();
    return CHIP_NO_ERROR;
}

OperationalDeviceProxy * CASEDeviceManager::FindExistingDevice(PeerId peerId) const
{
    return mConfig.devicePool->FindDevice(peerId);
}

void CASEDeviceManager::ReleaseDevice(OperationalDeviceProxy * device) const
{
    if (device != nullptr)
    {
        mConfig.devicePool->Release(device);
    }
}

} // namespace chip
