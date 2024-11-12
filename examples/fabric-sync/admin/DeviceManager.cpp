/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "DeviceManager.h"

#include <crypto/RandUtils.h>
#include <lib/support/StringBuilder.h>

#include <cstdio>
#include <string>

using namespace chip;

namespace admin {
// Define the static member
DeviceManager DeviceManager::sInstance;

void DeviceManager::Init()
{
    // TODO: (#34113) Init mLastUsedNodeId from chip config file
    mLastUsedNodeId = 1;
    mInitialized    = true;

    ChipLogProgress(NotSpecified, "DeviceManager initialized: last used nodeId " ChipLogFormatX64,
                    ChipLogValueX64(mLastUsedNodeId));
}

NodeId DeviceManager::GetNextAvailableNodeId()
{
    mLastUsedNodeId++;
    VerifyOrDieWithMsg(mLastUsedNodeId < std::numeric_limits<NodeId>::max(), NotSpecified, "No more available NodeIds.");

    return mLastUsedNodeId;
}

void DeviceManager::UpdateLastUsedNodeId(NodeId nodeId)
{
    if (nodeId > mLastUsedNodeId)
    {
        mLastUsedNodeId = nodeId;
        ChipLogProgress(NotSpecified, "Updating last used NodeId to " ChipLogFormatX64, ChipLogValueX64(mLastUsedNodeId));
    }
}

void DeviceManager::SetRemoteBridgeNodeId(chip::NodeId nodeId)
{
    mRemoteBridgeNodeId = nodeId;
}

CHIP_ERROR DeviceManager::PairRemoteFabricBridge(NodeId nodeId, uint32_t setupPINCode, const char * deviceRemoteIp,
                                                 uint16_t deviceRemotePort)
{
    CHIP_ERROR err = PairingManager::Instance().PairDevice(nodeId, setupPINCode, deviceRemoteIp, deviceRemotePort);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified,
                     "Failed to pair remote fabric bridge: Node ID " ChipLogFormatX64 " with error: %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(nodeId), err.Format());
        return err;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceManager::PairRemoteDevice(NodeId nodeId, const char * payload)
{
    CHIP_ERROR err = PairingManager::Instance().PairDeviceWithCode(nodeId, payload);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to pair device: Node ID " ChipLogFormatX64 " with error: %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(nodeId), err.Format());
        return err;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceManager::PairRemoteDevice(NodeId nodeId, uint32_t setupPINCode, const char * deviceRemoteIp,
                                           uint16_t deviceRemotePort)
{
    CHIP_ERROR err = PairingManager::Instance().PairDevice(nodeId, setupPINCode, deviceRemoteIp, deviceRemotePort);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to pair device: Node ID " ChipLogFormatX64 " with error: %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(nodeId), err.Format());
        return err;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceManager::UnpairRemoteFabricBridge()
{
    if (mRemoteBridgeNodeId == kUndefinedNodeId)
    {
        ChipLogError(NotSpecified, "Remote bridge node ID is undefined; cannot unpair device.");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    CHIP_ERROR err = PairingManager::Instance().UnpairDevice(mRemoteBridgeNodeId);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to unpair remote bridge device " ChipLogFormatX64, ChipLogValueX64(mRemoteBridgeNodeId));
        return err;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceManager::UnpairRemoteDevice(NodeId nodeId)
{
    CHIP_ERROR err = PairingManager::Instance().UnpairDevice(nodeId);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to unpair remote device " ChipLogFormatX64, ChipLogValueX64(nodeId));
        return err;
    }

    return CHIP_NO_ERROR;
}

} // namespace admin
