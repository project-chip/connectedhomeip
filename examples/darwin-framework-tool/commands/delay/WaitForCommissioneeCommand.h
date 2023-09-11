/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include "../common/CHIPCommandBridge.h"
#include <app/OperationalSessionSetup.h>
#include <lib/core/CHIPCallback.h>

NS_ASSUME_NONNULL_BEGIN
@interface MTRDeviceTestDelegate : NSObject <MTRDeviceDelegate>
@property CHIPCommandBridge * commandBridge;
- (instancetype)initWithCommandBridge:(CHIPCommandBridge *)commandBridge;
- (instancetype)init NS_UNAVAILABLE;
@end
NS_ASSUME_NONNULL_END

class WaitForCommissioneeCommand : public CHIPCommandBridge {
public:
    WaitForCommissioneeCommand()
        : CHIPCommandBridge("wait-for-commissionee")
        , mDeviceDelegate([[MTRDeviceTestDelegate alloc] initWithCommandBridge:this])
    {
        AddArgument("nodeId", 0, UINT64_MAX, &mNodeId);
        AddArgument("expire-existing-session", 0, 1, &mExpireExistingSession);
        AddArgument(
            "timeout", 0, UINT64_MAX, &mTimeoutSecs, "Time, in seconds, before this command is considered to have timed out.");
    }

    /////////// CHIPCommandBridge Interface /////////
    CHIP_ERROR RunCommand() override;
    chip::System::Clock::Timeout GetWaitDuration() const override
    {
        return chip::System::Clock::Seconds16(mTimeoutSecs.ValueOr(10));
    }

private:
    chip::NodeId mNodeId;
    chip::Optional<uint16_t> mTimeoutSecs;
    chip::Optional<bool> mExpireExistingSession;
    id<MTRDeviceDelegate> _Nullable mDeviceDelegate;
};
