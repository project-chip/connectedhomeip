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

namespace {

void CheckFabricBridgeSynchronizationSupport(intptr_t ignored)
{
    DeviceMgr().ReadSupportedDeviceCategories();
}

} // namespace

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
        DeviceMgr().SetRemoteBridgeNodeId(mBridgeNodeId);
        ChipLogProgress(NotSpecified, "Successfully paired bridge device: NodeId: " ChipLogFormatX64,
                        ChipLogValueX64(mBridgeNodeId));

        DeviceMgr().SubscribeRemoteFabricBridge();

        if (DeviceMgr().IsLocalBridgeReady())
        {
            // After successful commissioning of the Commissionee, initiate Reverse Commissioning
            // via the Commissioner Control Cluster. However, we must first verify that the
            // remote Fabric-Bridge supports Fabric Synchronization.
            //
            // Note: The Fabric-Admin MUST NOT send the RequestCommissioningApproval command
            // if the remote Fabric-Bridge lacks Fabric Synchronization support.
            DeviceLayer::PlatformMgr().ScheduleWork(CheckFabricBridgeSynchronizationSupport, 0);
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
    if (DeviceMgr().IsFabricSyncReady())
    {
        // print to console
        fprintf(stderr, "Remote Fabric Bridge has already been configured.");
        return CHIP_NO_ERROR;
    }

    PairingCommand * pairingCommand = static_cast<PairingCommand *>(CommandMgr().GetCommandByName("pairing", "already-discovered"));

    if (pairingCommand == nullptr)
    {
        ChipLogError(NotSpecified, "Pairing already-discovered command is not available");
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    pairingCommand->RegisterCommissioningDelegate(this);
    mBridgeNodeId = remoteId;

    DeviceMgr().PairRemoteFabricBridge(remoteId, reinterpret_cast<const char *>(mRemoteAddr.data()));

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
        DeviceMgr().SetRemoteBridgeNodeId(kUndefinedNodeId);
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
    NodeId bridgeNodeId = DeviceMgr().GetRemoteBridgeNodeId();

    if (bridgeNodeId == kUndefinedNodeId)
    {
        // print to console
        fprintf(stderr, "Remote Fabric Bridge is not configured yet, nothing to remove.");
        return CHIP_NO_ERROR;
    }

    mBridgeNodeId = bridgeNodeId;

    PairingCommand * pairingCommand = static_cast<PairingCommand *>(CommandMgr().GetCommandByName("pairing", "unpair"));

    if (pairingCommand == nullptr)
    {
        ChipLogError(NotSpecified, "Pairing unpair command is not available");
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    pairingCommand->RegisterPairingDelegate(this);

    DeviceMgr().UnpairRemoteFabricBridge();

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
        DeviceMgr().SetLocalBridgeNodeId(mLocalBridgeNodeId);
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
    if (DeviceMgr().IsLocalBridgeReady())
    {
        // print to console
        fprintf(stderr, "Local Fabric Bridge has already been configured.");
        return CHIP_NO_ERROR;
    }

    PairingCommand * pairingCommand = static_cast<PairingCommand *>(CommandMgr().GetCommandByName("pairing", "already-discovered"));
    VerifyOrDie(pairingCommand != nullptr);

    pairingCommand->RegisterCommissioningDelegate(this);
    mLocalBridgeNodeId = deviceId;

    DeviceMgr().PairLocalFabricBridge(deviceId);

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
        DeviceMgr().SetLocalBridgeNodeId(kUndefinedNodeId);
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
    NodeId bridgeNodeId = DeviceMgr().GetLocalBridgeNodeId();

    if (bridgeNodeId == kUndefinedNodeId)
    {
        // print to console
        fprintf(stderr, "Local Fabric Bridge is not configured yet, nothing to remove.");
        return CHIP_NO_ERROR;
    }

    mLocalBridgeNodeId = bridgeNodeId;

    PairingCommand * pairingCommand = static_cast<PairingCommand *>(CommandMgr().GetCommandByName("pairing", "unpair"));

    if (pairingCommand == nullptr)
    {
        ChipLogError(NotSpecified, "Pairing unpair command is not available");
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    pairingCommand->RegisterPairingDelegate(this);

    DeviceMgr().UnpairLocalFabricBridge();

    return CHIP_NO_ERROR;
}

void FabricSyncDeviceCommand::OnCommissioningWindowOpened(NodeId deviceId, CHIP_ERROR err, chip::SetupPayload payload)
{
    ChipLogProgress(NotSpecified, "FabricSyncDeviceCommand::OnCommissioningWindowOpened");

    if (err == CHIP_NO_ERROR)
    {
        char payloadBuffer[kMaxManualCodeLength + 1];
        MutableCharSpan manualCode(payloadBuffer);
        CHIP_ERROR error = ManualSetupPayloadGenerator(payload).payloadDecimalStringRepresentation(manualCode);
        if (error == CHIP_NO_ERROR)
        {
            NodeId nodeId = DeviceMgr().GetNextAvailableNodeId();

            PairingCommand * pairingCommand = static_cast<PairingCommand *>(CommandMgr().GetCommandByName("pairing", "code"));

            if (pairingCommand == nullptr)
            {
                ChipLogError(NotSpecified, "Pairing code command is not available");
                return;
            }

            pairingCommand->RegisterCommissioningDelegate(this);
            mAssignedNodeId = nodeId;

            usleep(kCommissionPrepareTimeMs * 1000);

            DeviceMgr().PairRemoteDevice(nodeId, payloadBuffer);
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
        DeviceMgr().AddSyncedDevice(Device(mAssignedNodeId, mRemoteEndpointId));
    }
    else
    {
        ChipLogError(NotSpecified, "Failed to pair synced device (0x:" ChipLogFormatX64 ") with error: %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(deviceId), err.Format());
    }
}

CHIP_ERROR FabricSyncDeviceCommand::RunCommand(EndpointId remoteId)
{
    if (!DeviceMgr().IsFabricSyncReady())
    {
        // print to console
        fprintf(stderr, "Remote Fabric Bridge is not configured yet.");
        return CHIP_NO_ERROR;
    }

    OpenCommissioningWindowCommand * openCommand =
        static_cast<OpenCommissioningWindowCommand *>(CommandMgr().GetCommandByName("pairing", "open-commissioning-window"));

    if (openCommand == nullptr)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    openCommand->RegisterDelegate(this);

    DeviceMgr().OpenRemoteDeviceCommissioningWindow(remoteId);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FabricAutoSyncCommand::RunCommand(bool enableAutoSync)
{
    DeviceMgr().EnableAutoSync(enableAutoSync);

    // print to console
    fprintf(stderr, "Auto Fabric Sync is %s.\n", enableAutoSync ? "enabled" : "disabled");
    fprintf(stderr,
            "WARNING: The auto-sync command is currently under development and may contain bugs. Use it at your own risk.\n");

    return CHIP_NO_ERROR;
}
