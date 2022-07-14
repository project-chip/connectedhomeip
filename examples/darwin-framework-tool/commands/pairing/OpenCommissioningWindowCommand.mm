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
 */

#import <Matter/Matter.h>

#include "OpenCommissioningWindowCommand.h"

CHIP_ERROR OpenCommissioningWindowCommand::RunCommand()
{
    auto * controller = CurrentCommissioner();
    NSError * error;
    __block NSString * pairingCode;
    if (mCommissioningWindowOption == 0) {
        [controller openPairingWindow:mNodeId duration:mCommissioningWindowTimeoutMs error:&error];
    } else {
        pairingCode = [controller openPairingWindowWithPIN:mNodeId
                                                  duration:mCommissioningWindowTimeoutMs
                                             discriminator:mDiscriminator
                                                  setupPIN:[MTRSetupPayload generateRandomPIN]
                                                     error:&error];
    }

    if (error != nil) {
        SetCommandExitStatus(error);
        return CHIP_NO_ERROR;
    }

    // TODO: Those should be async operations and we should not claim to
    // be done until they complete.  As things stand, we have no idea
    // how to tell when we're done, so just set a timer for slightly
    // less than our command timeout to call SetCommandExitStatus.
    mWorkQueue = dispatch_queue_create("com.chip.open_commissioning_window", DISPATCH_QUEUE_SERIAL);
    mTimer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, mWorkQueue);
    auto * self = this;
    dispatch_source_set_event_handler(mTimer, ^{
        dispatch_source_cancel(mTimer);
        mTimer = nil;
        mWorkQueue = nil;
        if (pairingCode != nil) {
            ChipLogProgress(chipTool, "Setup code: %s\n", [pairingCode UTF8String]);
        }
        self->SetCommandExitStatus(CHIP_NO_ERROR);
    });
    dispatch_source_set_timer(
        mTimer, dispatch_time(DISPATCH_TIME_NOW, (GetWaitDuration().count() - 2000) * NSEC_PER_MSEC), DISPATCH_TIME_FOREVER, 0);
    dispatch_resume(mTimer);

    return CHIP_NO_ERROR;
}
