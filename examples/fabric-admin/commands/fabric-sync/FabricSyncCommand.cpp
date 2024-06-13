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

constexpr uint16_t kRetryIntervalS      = 3;
constexpr uint16_t kMaxManaulCodeLength = 11;

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
            char command[64];
            snprintf(command, sizeof(command), "pairing code 3 %s", payloadBuffer);
            PushCommand(command);
        }
        else
        {
            ChipLogError(NotSpecified, "Unable to generate manual code for setup payload: %" CHIP_ERROR_FORMAT, error.Format());
        }
    }
}

CHIP_ERROR FabricSyncDeviceCommand::RunCommand(EndpointId remoteId)
{
    char command[64];
    snprintf(command, sizeof(command), "pairing open-commissioning-window 1 %d 1 300 1000 3840", remoteId);

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
