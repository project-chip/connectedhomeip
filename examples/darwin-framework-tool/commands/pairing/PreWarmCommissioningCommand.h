/*
 *   Copyright (c) 2022-2023 Project CHIP Authors
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
