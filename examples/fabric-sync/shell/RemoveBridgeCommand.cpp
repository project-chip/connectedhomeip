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

#include "RemoveBridgeCommand.h"

#include <admin/DeviceManager.h>
#include <lib/shell/streamer.h>

using namespace ::chip;

namespace commands {

void RemoveBridgeCommand::OnDeviceRemoved(NodeId deviceId, CHIP_ERROR err)
{
    if (mBridgeNodeId != deviceId)
    {
        ChipLogProgress(NotSpecified, "An non-bridge device: NodeId: " ChipLogFormatX64 " is removed.", ChipLogValueX64(deviceId));
        return;
    }

    if (err == CHIP_NO_ERROR)
    {
        admin::DeviceMgr().SetRemoteBridgeNodeId(kUndefinedNodeId);

        // print to console
        fprintf(stderr, "Successfully removed bridge device: NodeId: " ChipLogFormatX64 "\n", ChipLogValueX64(mBridgeNodeId));
    }
    else
    {
        ChipLogError(NotSpecified, "Failed to remove bridge device (0x:" ChipLogFormatX64 ") with error: %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(deviceId), err.Format());
    }

    CommandRegistry::Instance().ResetActiveCommand();
}

CHIP_ERROR RemoveBridgeCommand::RunCommand()
{
    NodeId bridgeNodeId = admin::DeviceMgr().GetRemoteBridgeNodeId();

    if (bridgeNodeId == kUndefinedNodeId)
    {
        // print to console
        fprintf(stderr, "Remote Fabric Bridge is not configured yet, nothing to remove.\n");
        return CHIP_NO_ERROR;
    }

    mBridgeNodeId = bridgeNodeId;

    admin::PairingManager::Instance().SetPairingDelegate(this);

    return admin::DeviceMgr().UnpairRemoteFabricBridge();
}

} // namespace commands
