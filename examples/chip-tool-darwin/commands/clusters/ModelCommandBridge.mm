/*
 *   Copyright (c) 2022 Project CHIP Authors
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

#import <CHIP/CHIPError_Internal.h>

#include "ModelCommandBridge.h"
#include <app/InteractionModelEngine.h>
#include <inttypes.h>

using namespace ::chip;

CHIP_ERROR ModelCommand::RunCommand()
{
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip-tool.command", DISPATCH_QUEUE_SERIAL);

    ChipLogProgress(chipTool, "Sending command to node 0x%" PRIx64, mNodeId);
    [CurrentCommissioner() getConnectedDevice:mNodeId
                                        queue:callbackQueue
                            completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                                if (error) {
                                    CHIP_ERROR err = CHIP_NO_ERROR;
                                    err = [CHIPError errorToCHIPErrorCode:error];
                                    ChipLogError(chipTool, "Error: %s", chip::ErrorStr(err));
                                    SetCommandExitStatus(err);
                                    return;
                                }

                                SendCommand(device, mEndPointId);
                            }];
    return CHIP_NO_ERROR;
}
