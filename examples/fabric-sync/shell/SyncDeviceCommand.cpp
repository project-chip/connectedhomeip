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

#include "SyncDeviceCommand.h"

#include <admin/DeviceManager.h>
#include <lib/shell/streamer.h>
#include <setup_payload/ManualSetupPayloadGenerator.h>

using namespace ::chip;

namespace {

// Constants
constexpr uint32_t kCommissionPrepareTimeMs = 500;

} // namespace

namespace commands {

SyncDeviceCommand::SyncDeviceCommand(EndpointId remoteEndpointId) : mRemoteEndpointId(remoteEndpointId) {}

void SyncDeviceCommand::OnCommissioningWindowOpened(NodeId deviceId, CHIP_ERROR err, SetupPayload payload)
{
    ChipLogProgress(NotSpecified, "FabricSyncDeviceCommand::OnCommissioningWindowOpened");

    if (err == CHIP_NO_ERROR)
    {
        char payloadBuffer[admin::kMaxManualCodeLength + 1];
        MutableCharSpan manualCode(payloadBuffer);
        CHIP_ERROR error = ManualSetupPayloadGenerator(payload).payloadDecimalStringRepresentation(manualCode);
        if (error == CHIP_NO_ERROR)
        {
            NodeId nodeId = admin::DeviceManager::Instance().GetNextAvailableNodeId();

            admin::PairingManager::Instance().SetPairingDelegate(this);
            mAssignedNodeId = nodeId;

            usleep(kCommissionPrepareTimeMs * 1000);

            error = admin::PairingManager::Instance().PairDeviceWithCode(nodeId, payloadBuffer);
            if (error != CHIP_NO_ERROR)
            {
                ChipLogError(NotSpecified, "Failed to sync device: Node ID " ChipLogFormatX64 " with error: %" CHIP_ERROR_FORMAT,
                             ChipLogValueX64(nodeId), err.Format());
            }
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

void SyncDeviceCommand::OnCommissioningComplete(NodeId deviceId, CHIP_ERROR err)
{
    if (mAssignedNodeId != deviceId)
    {
        // Ignore if the deviceId does not match the mAssignedNodeId.
        // This scenario should not occur because no other device should be commissioned during the fabric sync process.
        return;
    }

    if (err == CHIP_NO_ERROR)
    {
        admin::DeviceManager::Instance().AddSyncedDevice(admin::SyncedDevice(mAssignedNodeId, mRemoteEndpointId));
    }
    else
    {
        ChipLogError(NotSpecified, "Failed to pair synced device (0x:" ChipLogFormatX64 ") with error: %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(deviceId), err.Format());
    }

    admin::PairingManager::Instance().ResetForNextCommand();
    CommandRegistry::Instance().ResetActiveCommand();
}

CHIP_ERROR SyncDeviceCommand::RunCommand()
{
    if (!admin::DeviceManager::Instance().IsFabricSyncReady())
    {
        // print to console
        Shell::streamer_t * sout = Shell::streamer_get();
        Shell::streamer_printf(sout, "Remote Fabric Bridge has already been configured.\n");

        return CHIP_ERROR_INCORRECT_STATE;
    }

    ChipLogProgress(NotSpecified, "Running SyncDeviceCommand with EndpointId: %u", mRemoteEndpointId);

    admin::PairingManager::Instance().SetOpenCommissioningWindowDelegate(this);
    admin::DeviceManager::Instance().OpenRemoteDeviceCommissioningWindow(mRemoteEndpointId);

    return CHIP_NO_ERROR;
}

} // namespace commands
