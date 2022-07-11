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

#include "ModelCommandBridge.h"
#include <inttypes.h>

#import <Matter/Matter.h>

using namespace ::chip;

CHIP_ERROR ModelCommand::RunCommand()
{
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip-tool.command", DISPATCH_QUEUE_SERIAL);

    MTRDeviceController * commissioner = CurrentCommissioner();
    ChipLogProgress(chipTool, "Sending command to node 0x" ChipLogFormatX64, ChipLogValueX64(mNodeId));
    [commissioner getBaseDevice:mNodeId
                          queue:callbackQueue
              completionHandler:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
                  if (error != nil) {
                      SetCommandExitStatus(error, "Error getting connected device");
                      return;
                  }

                  CHIP_ERROR err;
                  if (device == nil) {
                      err = CHIP_ERROR_INTERNAL;
                  } else {
                      err = SendCommand(device, mEndPointId);
                  }

                  if (err != CHIP_NO_ERROR) {
                      ChipLogError(chipTool, "Error: %s", chip::ErrorStr(err));
                      SetCommandExitStatus(err);
                      return;
                  }
              }];
    return CHIP_NO_ERROR;
}

void ModelCommand::Shutdown() { ResetArguments(); }
