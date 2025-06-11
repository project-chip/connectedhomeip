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

#include "LiveViewCommands.h"
#include <commands/interactive/InteractiveCommands.h>
#include <device-manager/DeviceManager.h>

using namespace chip;

namespace {} // namespace

CHIP_ERROR LiveViewStartCommand::RunCommand()
{
    ChipLogProgress(Camera, "Run LiveViewStartCommand");

    if (mPeerNodeId == chip::kUndefinedNodeId)
    {
        ChipLogError(Camera, "Missing --node-id");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Use provided stream usage or default to 3 (LiveView)
    uint8_t streamUsage = mStreamUsage.HasValue() ? mStreamUsage.Value() : 3;

    return camera::DeviceManager::Instance().AllocateVideoStream(mPeerNodeId, streamUsage);
}

CHIP_ERROR LiveViewStopCommand::RunCommand()
{
    ChipLogProgress(Camera, "Run LiveViewStopCommand");

    camera::DeviceManager::Instance().StopVideoStream(mVideoStreamID);

    return camera::DeviceManager::Instance().DeallocateVideoStream(mPeerNodeId, mVideoStreamID);
}
