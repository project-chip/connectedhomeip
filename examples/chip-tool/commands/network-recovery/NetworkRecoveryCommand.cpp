/*
 *   Copyright (c) 2021 Project CHIP Authors
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

void NetworkRecoveryCommandBase::OnNetworkRecoverDiscover(uint64_t recoveryId)
{
    ChipLogProgress(chipTool, "found recoverable device:%lu", recoveryId);
}

void NetworkRecoveryCommandBase::OnNetworkRecoverComplete(NodeId deviceId, CHIP_ERROR error)
{
    if (error == CHIP_NO_ERROR)
    {
        ChipLogProgress(chipTool, "recovery complete for device:%lu", deviceId);
    }
    else
    {
        ChipLogError(chipTool, "recovery failed for device:%lu", deviceId);
    }
    SetCommandExitStatus(CHIP_NO_ERROR);
}

CHIP_ERROR NetworkRecoveryDiscoverCommand::RunCommand()
{
    mCommissioner = &CurrentCommissioner();
    mCommissioner->RegisterNetworkRecoverDelegate(this);
    return mCommissioner->DiscoverRecoverableNodes();
}

CHIP_ERROR NetworkRecoveryRecoverCommand::RunCommand()
{
    mCommissioner = &CurrentCommissioner();
    mCommissioner->RegisterNetworkRecoverDelegate(this);
    chip::Controller::WiFiCredentials wiFiCreds(mSSID, mPassword);
    return mCommissioner->RecoverNode(mNodeId, mRecoveryId, wiFiCreds, mBreadcrumb);
}