/*
 *   Copyright (c) 2025 Project CHIP Authors
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

#include "NetworkRecoveryCommand.h"
#include <commands/common/DeviceScanner.h>
#include <commands/common/RemoteDataModelLogger.h>
#include <lib/support/BytesToHex.h>

using namespace ::chip;

void NetworkRecoveryCommandBase::OnNetworkRecoverDiscover(std::list<uint64_t> recoveryIds)
{
    ChipLogProgress(chipTool, "Find recoverable devices:");
    for (const auto & recoveryId : recoveryIds)
    {
        ChipLogProgress(chipTool, "0x" ChipLogFormatX64, ChipLogValueX64(recoveryId));
        (void) recoveryId; // Avoid unused variable warning in case logging is disabled
    }
    SetCommandExitStatus(CHIP_NO_ERROR);
}

void NetworkRecoveryCommandBase::OnNetworkRecoverComplete(NodeId deviceId, CHIP_ERROR error)
{
    if (error == CHIP_NO_ERROR)
    {
        ChipLogProgress(chipTool, "Recovery complete for devicice " ChipLogFormatX64, ChipLogValueX64(deviceId));
    }
    else
    {
        ChipLogError(chipTool, "Recovery failed for device " ChipLogFormatX64, ChipLogValueX64(deviceId));
    }
    SetCommandExitStatus(error);
}

CHIP_ERROR NetworkRecoveryDiscoverCommand::RunCommand()
{
    uint16_t timeout = mTimeout.ValueOr(30);
    ChipLogProgress(chipTool, "Start scanning recoverable nodes, timeout:%d", timeout);
    mCommissioner = &CurrentCommissioner();
    mCommissioner->RegisterNetworkRecoverDelegate(this);
    return mCommissioner->DiscoverRecoverableNodes(timeout);
}

CHIP_ERROR NetworkRecoveryRecoverCommand::RunCommand()
{
    CHIP_ERROR err;
    mCommissioner = &CurrentCommissioner();
    mCommissioner->RegisterNetworkRecoverDelegate(this);
    if (mOperationalDataset.empty())
    {
        chip::Controller::WiFiCredentials wiFiCreds(mSSID, mPassword);
        err = mCommissioner->RecoverNode(mNodeId, mRecoveryId, wiFiCreds, mBreadcrumb);
    }
    else
    {
        err = mCommissioner->RecoverNode(mNodeId, mRecoveryId, mOperationalDataset, mBreadcrumb);
    }
    return err;
}
