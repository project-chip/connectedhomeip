/*
 * SPDX-FileCopyrightText: 2022-2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#import <Matter/Matter.h>

#include "../common/CHIPCommandBridge.h"

#import "MTRError_Utils.h"

class PreWarmCommissioningCommand : public CHIPCommandBridge {
public:
    PreWarmCommissioningCommand()
        : CHIPCommandBridge("pre-warm-commissioning")
    {
    }

protected:
    /////////// CHIPCommandBridge Interface /////////
    CHIP_ERROR RunCommand() override
    {
        auto * controller = CurrentCommissioner();
        [controller preWarmCommissioningSession];

        // In interactive mode, we don't want to block the UI until the end of `GetWaitDuration`. So returns early.
        if (IsInteractive()) {
            SetCommandExitStatus(CHIP_NO_ERROR);
        }
        return CHIP_NO_ERROR;
    }

    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(30); }
};
