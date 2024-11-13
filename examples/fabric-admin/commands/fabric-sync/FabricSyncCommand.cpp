/*
 *   Copyright (c) 2024 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "FabricSyncCommand.h"
#include <commands/common/RemoteDataModelLogger.h>
#include <commands/interactive/InteractiveCommands.h>
#include <device_manager/DeviceManager.h>
#include <setup_payload/ManualSetupPayloadGenerator.h>
#include <thread>
#include <unistd.h>

#if defined(PW_RPC_ENABLED)
#include <rpc/RpcClient.h>
#endif

using namespace ::chip;

namespace admin {

void FabricSyncAddBridgeCommand::OnCommissioningComplete(NodeId deviceId, CHIP_ERROR err)
{
    if (mBridgeNodeId != deviceId)
    {
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(NotSpecified, "Failed to pair non-bridge device (0x:" ChipLogFormatX64 ") with error: %" CHIP_ERROR_FORMAT,
                         ChipLogValueX64(deviceId), err.Format());
        }
        else
        {
            ChipLogProgress(NotSpecified, "Commissioning complete for non-bridge device: NodeId: " ChipLogFormatX64,
                            ChipLogValueX64(deviceId));
        }
        return;
    }

    if (err == CHIP_NO_ERROR)
    {
        DeviceManager::Instance().SetRemoteBridgeNodeId(mBridgeNodeId);
        ChipLogProgress(NotSpecified, "Successfully paired bridge device: NodeId: " ChipLogFormatX64,
                        ChipLogValueX64(mBridgeNodeId));

        DeviceManager::Instance().UpdateLastUsedNodeId(mBridgeNodeId);
        DeviceManager::Instance().SubscribeRemoteFabricBridge();

        if (DeviceManager::Instance().IsLocalBridgeReady())
        {
            // After successful commissioning of the Commissionee, initiate Reverse Commissioning
            // via the Commissioner Control Cluster. However, we must first verify that the
            // remote Fabric-Bridge supports Fabric Synchronization.
            //
            // Note: The Fabric-Admin MUST NOT send the RequestCommissioningApproval command
            // if the remote Fabric-Bridge lacks Fabric Synchronization support.
            DeviceLayer::SystemLayer().ScheduleLambda([]() { DeviceManager::Instance().ReadSupportedDeviceCategories(); });
        }
    }
    else
    {
        ChipLogError(NotSpecified, "Failed to pair bridge device (0x:" ChipLogFormatX64 ") with error: %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(deviceId), err.Format());
    }

    mBridgeNodeId = kUndefinedNodeId;
}

CHIP_ERROR FabricSyncAddBridgeCommand::RunCommand(NodeId remoteId)
{
    if (DeviceManager::Instance().IsFabricSyncReady())
    {
        // print to console
        fprintf(stderr, "Remote Fabric Bridge has already been configured.\n");
        return CHIP_NO_ERROR;
    }

    PairingManager::Instance().SetPairingDelegate(this);

    mBridgeNodeId = remoteId;

    DeviceManager::Instance().PairRemoteFabricBridge(remoteId, mSetupPINCode, reinterpret_cast<const char *>(mRemoteAddr.data()),
                                                     mRemotePort);

    return CHIP_NO_ERROR;
}

void FabricSyncRemoveBridgeCommand::OnDeviceRemoved(NodeId deviceId, CHIP_ERROR err)
{
    if (mBridgeNodeId != deviceId)
    {
        ChipLogProgress(NotSpecified, "An non-bridge device: NodeId: " ChipLogFormatX64 " is removed.", ChipLogValueX64(deviceId));
        return;
    }

    if (err == CHIP_NO_ERROR)
    {
        DeviceManager::Instance().SetRemoteBridgeNodeId(kUndefinedNodeId);
        ChipLogProgress(NotSpecified, "Successfully removed bridge device: NodeId: " ChipLogFormatX64,
                        ChipLogValueX64(mBridgeNodeId));
    }
    else
    {
        ChipLogError(NotSpecified, "Failed to remove bridge device (0x:" ChipLogFormatX64 ") with error: %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(deviceId), err.Format());
    }

    mBridgeNodeId = kUndefinedNodeId;
}

CHIP_ERROR FabricSyncRemoveBridgeCommand::RunCommand()
{
    NodeId bridgeNodeId = DeviceManager::Instance().GetRemoteBridgeNodeId();

    if (bridgeNodeId == kUndefinedNodeId)
    {
        // print to console
        fprintf(stderr, "Remote Fabric Bridge is not configured yet, nothing to remove.\n");
        return CHIP_NO_ERROR;
    }

    mBridgeNodeId = bridgeNodeId;

    PairingManager::Instance().SetPairingDelegate(this);
    DeviceManager::Instance().UnpairRemoteFabricBridge();

    return CHIP_NO_ERROR;
}

void FabricSyncAddLocalBridgeCommand::OnCommissioningComplete(NodeId deviceId, CHIP_ERROR err)
{
    if (mLocalBridgeNodeId != deviceId)
    {
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(NotSpecified, "Failed to pair non-bridge device (0x:" ChipLogFormatX64 ") with error: %" CHIP_ERROR_FORMAT,
                         ChipLogValueX64(deviceId), err.Format());
        }
        else
        {
            ChipLogProgress(NotSpecified, "Commissioning complete for non-bridge device: NodeId: " ChipLogFormatX64,
                            ChipLogValueX64(deviceId));
        }
        return;
    }

    if (err == CHIP_NO_ERROR)
    {
        DeviceManager::Instance().SetLocalBridgeNodeId(mLocalBridgeNodeId);
        DeviceManager::Instance().UpdateLastUsedNodeId(mLocalBridgeNodeId);
        ChipLogProgress(NotSpecified, "Successfully paired local bridge device: NodeId: " ChipLogFormatX64,
                        ChipLogValueX64(mLocalBridgeNodeId));
    }
    else
    {
        ChipLogError(NotSpecified, "Failed to pair local bridge device (0x:" ChipLogFormatX64 ") with error: %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(deviceId), err.Format());
    }

    mLocalBridgeNodeId = kUndefinedNodeId;
}

CHIP_ERROR FabricSyncAddLocalBridgeCommand::RunCommand(NodeId deviceId)
{
    if (DeviceManager::Instance().IsLocalBridgeReady())
    {
        // print to console
        fprintf(stderr, "Local Fabric Bridge has already been configured.\n");
        return CHIP_NO_ERROR;
    }

    PairingManager::Instance().SetPairingDelegate(this);
    mLocalBridgeNodeId = deviceId;

    if (mSetupPINCode.HasValue())
    {
        DeviceManager::Instance().SetLocalBridgeSetupPinCode(mSetupPINCode.Value());
    }
    if (mLocalPort.HasValue())
    {
        DeviceManager::Instance().SetLocalBridgePort(mLocalPort.Value());
    }

    DeviceManager::Instance().PairLocalFabricBridge(deviceId);

    return CHIP_NO_ERROR;
}

void FabricSyncRemoveLocalBridgeCommand::OnDeviceRemoved(NodeId deviceId, CHIP_ERROR err)
{
    if (mLocalBridgeNodeId != deviceId)
    {
        ChipLogProgress(NotSpecified, "A non-bridge device: NodeId: " ChipLogFormatX64 " is removed.", ChipLogValueX64(deviceId));
        return;
    }

    if (err == CHIP_NO_ERROR)
    {
        DeviceManager::Instance().SetLocalBridgeNodeId(kUndefinedNodeId);
        ChipLogProgress(NotSpecified, "Successfully removed local bridge device: NodeId: " ChipLogFormatX64,
                        ChipLogValueX64(mLocalBridgeNodeId));
    }
    else
    {
        ChipLogError(NotSpecified, "Failed to remove local bridge device (0x:" ChipLogFormatX64 ") with error: %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(deviceId), err.Format());
    }

    mLocalBridgeNodeId = kUndefinedNodeId;
}

CHIP_ERROR FabricSyncRemoveLocalBridgeCommand::RunCommand()
{
    NodeId bridgeNodeId = DeviceManager::Instance().GetLocalBridgeNodeId();

    if (bridgeNodeId == kUndefinedNodeId)
    {
        // print to console
        fprintf(stderr, "Local Fabric Bridge is not configured yet, nothing to remove.\n");
        return CHIP_NO_ERROR;
    }

    mLocalBridgeNodeId = bridgeNodeId;

    PairingManager::Instance().SetPairingDelegate(this);
    DeviceManager::Instance().UnpairLocalFabricBridge();

    return CHIP_NO_ERROR;
}

void FabricSyncDeviceCommand::OnCommissioningWindowOpened(NodeId deviceId, CHIP_ERROR err, SetupPayload payload)
{
    ChipLogProgress(NotSpecified, "FabricSyncDeviceCommand::OnCommissioningWindowOpened");

    if (err == CHIP_NO_ERROR)
    {
        char payloadBuffer[kMaxManualCodeLength + 1];
        MutableCharSpan manualCode(payloadBuffer);
        CHIP_ERROR error = ManualSetupPayloadGenerator(payload).payloadDecimalStringRepresentation(manualCode);
        if (error == CHIP_NO_ERROR)
        {
            NodeId nodeId = DeviceManager::Instance().GetNextAvailableNodeId();

            PairingManager::Instance().SetPairingDelegate(this);
            mAssignedNodeId = nodeId;

            usleep(kCommissionPrepareTimeMs * 1000);

            DeviceManager::Instance().PairRemoteDevice(nodeId, payloadBuffer);
        }
        else
        {
            ChipLogError(NotSpecified, "Unable to generate manual code for setup payload: %" CHIP_ERROR_FORMAT, error.Format());
        }
    }
    else
    {
        ChipLogError(NotSpecified,
                     "Failed to open synced device (0x:" ChipLogFormatX64 ") commissioning window: %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(deviceId), err.Format());
    }
}

void FabricSyncDeviceCommand::OnCommissioningComplete(NodeId deviceId, CHIP_ERROR err)
{
    if (mAssignedNodeId != deviceId)
    {
        // Ignore if the deviceId does not match the mAssignedNodeId.
        // This scenario should not occur because no other device should be commissioned during the fabric sync process.
        return;
    }

    if (err == CHIP_NO_ERROR)
    {
        DeviceManager::Instance().AddSyncedDevice(Device(mAssignedNodeId, mRemoteEndpointId));
    }
    else
    {
        ChipLogError(NotSpecified, "Failed to pair synced device (0x:" ChipLogFormatX64 ") with error: %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(deviceId), err.Format());
    }
}

CHIP_ERROR FabricSyncDeviceCommand::RunCommand(EndpointId remoteEndpointId)
{
    if (!DeviceManager::Instance().IsFabricSyncReady())
    {
        // print to console
        fprintf(stderr, "Remote Fabric Bridge is not configured yet.\n");
        return CHIP_NO_ERROR;
    }

    PairingManager::Instance().SetOpenCommissioningWindowDelegate(this);

    DeviceManager::Instance().OpenRemoteDeviceCommissioningWindow(remoteEndpointId);

    return CHIP_NO_ERROR;
}

} // namespace admin
