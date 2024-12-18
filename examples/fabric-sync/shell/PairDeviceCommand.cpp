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

#include "PairDeviceCommand.h"

#include <admin/DeviceManager.h>
#include <lib/shell/streamer.h>

using namespace ::chip;

namespace commands {

PairDeviceCommand::PairDeviceCommand(chip::NodeId nodeId, const char * payload, bool enableICDRegistration) :
    mNodeId(nodeId), mPayload(payload), mEnableICDRegistration(enableICDRegistration)
{}

void PairDeviceCommand::OnCommissioningComplete(NodeId deviceId, CHIP_ERROR err)
{
    if (mNodeId != deviceId)
    {
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(NotSpecified,
                         "Failed to pair non-specified device (0x:" ChipLogFormatX64 ") with error: %" CHIP_ERROR_FORMAT,
                         ChipLogValueX64(deviceId), err.Format());
        }
        else
        {
            ChipLogProgress(NotSpecified, "Commissioning complete for non-specified device: NodeId: " ChipLogFormatX64,
                            ChipLogValueX64(deviceId));
        }
        return;
    }

    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(NotSpecified, "Successfully paired device: NodeId: " ChipLogFormatX64, ChipLogValueX64(mNodeId));

        admin::DeviceManager::Instance().UpdateLastUsedNodeId(mNodeId);
    }
    else
    {
        ChipLogError(NotSpecified, "Failed to pair device (0x:" ChipLogFormatX64 ") with error: %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(deviceId), err.Format());
    }

    admin::PairingManager::Instance().ResetForNextCommand();
    CommandRegistry::Instance().ResetActiveCommand();
}

CHIP_ERROR PairDeviceCommand::RunCommand()
{
    if (admin::DeviceManager::Instance().IsCurrentBridgeDevice(mNodeId))
    {
        // print to console
        fprintf(stderr, "The specified node ID has been reserved by the Fabric Bridge.\n");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ChipLogProgress(NotSpecified, "Running PairDeviceCommand with Node ID: " ChipLogFormatX64 ", Code: %s",
                    ChipLogValueX64(mNodeId), mPayload);

    admin::PairingManager::Instance().SetPairingDelegate(this);

    return admin::PairingManager::Instance().PairDeviceWithCode(mNodeId, mPayload, mEnableICDRegistration);
}

} // namespace commands
