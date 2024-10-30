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
 */

#pragma once

#import "../common/DeviceDelegate.h"
#import <Matter/Matter.h>

#include <commands/common/Command.h>

class SetUpDeviceCommand : public CHIPCommandBridge {
public:
    SetUpDeviceCommand()
        : CHIPCommandBridge("device", "Creates and configures an instance of a device.")
    {
        AddArgument("node-id", 0, UINT64_MAX, &mNodeId, "The Node ID of the device instance to create.");
        AddArgument("pretend-thread-enabled", 0, 1, &mPretendThreadEnabled,
            "When the device is configured using an MTRDevice (via -use-mtr-device), instructs the MTRDevice to treat the "
            "target device as a Thread device.");
        AddArgument("max-interval", 0, UINT32_MAX, &mMaxIntervalForSubscription,
            "When the device is configured using an MTRDevice (via --use-mtr-device), configure the maximum interval for the "
            "delegate subscription.");
    }

protected:
    /////////// CHIPCommandBridge Interface /////////
    CHIP_ERROR RunCommand() override
    {
        __auto_type * controller = CurrentCommissioner();
        VerifyOrReturnError(nil != controller, CHIP_ERROR_INCORRECT_STATE);

        __auto_type * device = [MTRDevice deviceWithNodeID:@(mNodeId) controller:controller];
        VerifyOrReturnError(nil != device, CHIP_ERROR_INCORRECT_STATE);

        __auto_type * delegate = ConfigureDelegate();
        __auto_type queue = dispatch_queue_create("com.chip.devicedelegate", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
        [device addDelegate:delegate queue:queue];

        mDelegate = delegate;
        SetCommandExitStatus(CHIP_NO_ERROR);
        return CHIP_NO_ERROR;
    }

    // Our command is synchronous, so no need to wait.
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::kZero; }

private:
    DeviceDelegate * ConfigureDelegate()
    {
        __auto_type * delegate = [[DeviceDelegate alloc] init];

        if (mPretendThreadEnabled.ValueOr(false)) {
            [delegate setPretendThreadEnabled:YES];
        }

        if (mMaxIntervalForSubscription.HasValue()) {
            [delegate setMaxIntervalForSubscription:@(mMaxIntervalForSubscription.Value())];
        }

        return delegate;
    }

    DeviceDelegate * mDelegate;

    chip::NodeId mNodeId;
    chip::Optional<bool> mPretendThreadEnabled;
    chip::Optional<uint32_t> mMaxIntervalForSubscription;
};
