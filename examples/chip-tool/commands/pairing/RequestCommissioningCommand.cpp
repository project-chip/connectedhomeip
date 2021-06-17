/*
 *   Copyright (c) 2020 Project CHIP Authors
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

#include "RequestCommissioningCommand.h"

#include <support/CodeUtils.h>

using namespace ::chip;

constexpr uint16_t kWaitDurationInSeconds = 3 * 60;

CHIP_ERROR RequestCommissioningCommand::Run(NodeId localId, NodeId remoteId)
{
    //
    // Set this to true first BEFORE we send commands to ensure we don't
    // end up in a situation where the response comes back faster than we can
    // set the variable to true, which will cause it to block indefinitely.
    //
    UpdateWaitForResponse(true);

    {
        chip::DeviceLayer::StackLock lock;

        ReturnErrorOnFailure(mCommissionableNodeController.AdvertiseCommissionableNode());

        // TODO: ReturnErrorOnFailure(mCommissionableNodeController.SendUserDirectedCommissioningRequest(args...));

        ChipLogProgress(chipTool, "Waiting for %d sec", kWaitDurationInSeconds);
    }

    WaitForResponse(kWaitDurationInSeconds);

    return CHIP_NO_ERROR;
}
