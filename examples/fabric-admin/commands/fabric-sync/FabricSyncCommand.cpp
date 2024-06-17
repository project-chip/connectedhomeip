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
#include <setup_payload/ManualSetupPayloadGenerator.h>
#include <thread>
#include <unistd.h>

#if defined(PW_RPC_ENABLED)
#include <rpc/RpcClient.h>
#endif

using namespace ::chip;

namespace {

// Constants
constexpr uint32_t kCommissionPrepareTimeMs = 500;
constexpr uint16_t kMaxManaulCodeLength     = 21;
constexpr uint16_t kSubscribeMinInterval    = 0;
constexpr uint16_t kSubscribeMaxInterval    = 60;

} // namespace

CHIP_ERROR FabricSyncAddDeviceCommand::RunCommand(NodeId remoteId)
{
#if defined(PW_RPC_ENABLED)
    AddSynchronizedDevice(remoteId);
    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

void FabricSyncDeviceCommand::OnCommissioningWindowOpened(NodeId deviceId, CHIP_ERROR err, chip::SetupPayload payload)
{
    if (err == CHIP_NO_ERROR)
    {
        char payloadBuffer[kMaxManaulCodeLength + 1];
        MutableCharSpan manualCode(payloadBuffer);
        CHIP_ERROR error = ManualSetupPayloadGenerator(payload).payloadDecimalStringRepresentation(manualCode);
        if (error == CHIP_NO_ERROR)
        {
            char command[kMaxCommandSize];
            NodeId nodeId = 2; // TODO: need to switch to dynamically assigned ID
            snprintf(command, sizeof(command), "pairing code %ld %s", nodeId, payloadBuffer);

            PairingCommand * pairingCommand = static_cast<PairingCommand *>(CommandMgr().GetCommandByName("pairing", "code"));

            if (pairingCommand == nullptr)
            {
                ChipLogError(NotSpecified, "Pairing code command is not available");
                return;
            }

            pairingCommand->RegisterCommissioningDelegate(this);
            mAssignedNodeId = nodeId;

            usleep(kCommissionPrepareTimeMs * 1000);

            PushCommand(command);
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

void FabricSyncDeviceCommand::OnCommissioningComplete(chip::NodeId deviceId, CHIP_ERROR err)
{
    if (mAssignedNodeId != deviceId)
    {
        // Ignore if the deviceId does not match the mAssignedNodeId.
        // This scenario should not occur because no other device should be commissioned during the fabric sync process.
        return;
    }

    if (err == CHIP_NO_ERROR)
    {
        // TODO: AddSyncedDevice
    }
    else
    {
        ChipLogError(NotSpecified, "Failed to pair synced device (0x:" ChipLogFormatX64 ") with error: %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(deviceId), err.Format());
    }
}

CHIP_ERROR FabricSyncDeviceCommand::RunCommand(EndpointId remoteId)
{
    char command[kMaxCommandSize];
    NodeId bridgeNodeId = 1; // TODO: need to switch to configured ID
    snprintf(command, sizeof(command), "pairing open-commissioning-window %ld %d %d %d %d %d", bridgeNodeId, remoteId,
             kEnhancedCommissioningMethod, kWindowTimeout, kIteration, kDiscriminator);

    OpenCommissioningWindowCommand * openCommand =
        static_cast<OpenCommissioningWindowCommand *>(CommandMgr().GetCommandByName("pairing", "open-commissioning-window"));

    if (openCommand == nullptr)
    {
        return CHIP_ERROR_UNINITIALIZED;
    }

    openCommand->RegisterDelegate(this);

    PushCommand(command);

    return CHIP_NO_ERROR;
}
