/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "../common/CHIPCommandBridge.h"

/**
 * This command blocks the event loop processing for a given amount of time.
 *
 * For example when the event loop is blocked the messages coming-in will not be acked,
 * forcing a retransmission on the other side.
 *
 */

class SleepCommand : public CHIPCommandBridge
{
public:
    SleepCommand() : CHIPCommandBridge("sleep")
    {
        AddArgument("duration-in-ms", 0, UINT32_MAX, &mDurationInMs,
                    "Block the event loop processing for duration-in-ms milliseconds.");
    }

    /////////// CHIPCommandBridge Interface /////////
    CHIP_ERROR RunCommand() override;
    chip::System::Clock::Timeout GetWaitDuration() const override
    {
        // The allowed duration of this method is at least as long as the time specified for blocking the
        // event loop. In order to not fail on some small delays in processing some extra time before
        // failing is added.
        constexpr uint16_t mExtraTimeForFailure = 1000;

        return chip::System::Clock::Milliseconds32(mDurationInMs + mExtraTimeForFailure);
    }

private:
    uint32_t mDurationInMs;
};
