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

#include "RemoveDeviceCommand.h"

#include <admin/DeviceManager.h>
#include <lib/shell/streamer.h>

using namespace ::chip;

namespace commands {

RemoveDeviceCommand::RemoveDeviceCommand(NodeId nodeId) : mNodeId(nodeId) {}

void RemoveDeviceCommand::OnDeviceRemoved(NodeId deviceId, CHIP_ERROR err)
{
    if (mNodeId != deviceId)
    {
        ChipLogProgress(NotSpecified, "An non-specified device: NodeId: " ChipLogFormatX64 " is removed.",
                        ChipLogValueX64(deviceId));
        return;
    }

    if (err == CHIP_NO_ERROR)
    {
        // print to console
        fprintf(stderr, "Successfully removed device: NodeId: " ChipLogFormatX64 "\n", ChipLogValueX64(mNodeId));
    }
    else
    {
        ChipLogError(NotSpecified, "Failed to remove device (0x:" ChipLogFormatX64 ") with error: %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(deviceId), err.Format());
    }

    CommandRegistry::Instance().ResetActiveCommand();
}

CHIP_ERROR RemoveDeviceCommand::RunCommand()
{
    if (admin::DeviceManager::Instance().IsCurrentBridgeDevice(mNodeId))
    {
        // print to console
        fprintf(stderr, "The specified node ID has been reserved by the Fabric Bridge.\n");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    admin::PairingManager::Instance().SetPairingDelegate(this);

    ChipLogProgress(NotSpecified, "Running RemoveDeviceCommand with Node ID: " ChipLogFormatX64, ChipLogValueX64(mNodeId));

    return admin::PairingManager::Instance().UnpairDevice(mNodeId);
}

} // namespace commands
