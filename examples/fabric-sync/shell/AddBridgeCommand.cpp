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

#include "AddBridgeCommand.h"

#include <admin/DeviceManager.h>
#include <lib/shell/streamer.h>

using namespace ::chip;

namespace commands {

AddBridgeCommand::AddBridgeCommand(chip::NodeId nodeId, uint32_t setupPINCode, const char * remoteAddr, uint16_t remotePort) :
    mBridgeNodeId(nodeId), mSetupPINCode(setupPINCode), mRemoteAddr(remoteAddr), mRemotePort(remotePort)
{}

void AddBridgeCommand::OnCommissioningComplete(NodeId deviceId, CHIP_ERROR err)
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
        admin::DeviceManager::Instance().SetRemoteBridgeNodeId(mBridgeNodeId);

        ChipLogProgress(NotSpecified, "Successfully paired bridge device: NodeId: " ChipLogFormatX64,
                        ChipLogValueX64(mBridgeNodeId));

        admin::DeviceManager::Instance().UpdateLastUsedNodeId(mBridgeNodeId);
        admin::DeviceManager::Instance().SubscribeRemoteFabricBridge();
        admin::DeviceManager::Instance().InitCommissionerControl();

        // After successful commissioning of the Commissionee, initiate Reverse Commissioning
        // via the Commissioner Control Cluster. However, we must first verify that the
        // remote Fabric-Bridge supports Fabric Synchronization.
        //
        // Note: The Fabric-Admin MUST NOT send the RequestCommissioningApproval command
        // if the remote Fabric-Bridge lacks Fabric Synchronization support.
        DeviceLayer::SystemLayer().ScheduleLambda([]() { admin::DeviceManager::Instance().ReadSupportedDeviceCategories(); });
    }
    else
    {
        ChipLogError(NotSpecified, "Failed to pair bridge device (0x:" ChipLogFormatX64 ") with error: %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(deviceId), err.Format());
    }

    CommandRegistry::Instance().ResetActiveCommand();
}

CHIP_ERROR AddBridgeCommand::RunCommand()
{
    if (admin::DeviceManager::Instance().IsFabricSyncReady())
    {
        // print to console
        fprintf(stderr, "Remote Fabric Bridge has already been configured.\n");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    admin::PairingManager::Instance().SetPairingDelegate(this);

    ChipLogProgress(NotSpecified, "Running AddBridgeCommand with Node ID: %lu, PIN Code: %u, Address: %s, Port: %u", mBridgeNodeId,
                    mSetupPINCode, mRemoteAddr, mRemotePort);

    return admin::DeviceManager::Instance().PairRemoteFabricBridge(mBridgeNodeId, mSetupPINCode, mRemoteAddr, mRemotePort);
}

} // namespace commands
