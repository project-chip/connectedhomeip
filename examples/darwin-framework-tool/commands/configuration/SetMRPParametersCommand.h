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

#import <Matter/Matter.h>

#include "../common/CHIPCommandBridge.h"

class SetMRPParametersCommand : public CHIPCommandBridge
{
public:
    SetMRPParametersCommand() :
        CHIPCommandBridge("set-mrp-parameters", "Set various MRP parameters.  At least one value must be provided.")
    {
        AddArgument("idle-interval", 0, UINT32_MAX, &mIdleRetransmitMs,
                    "Our MRP idle interval (SII) in milliseconds.  Defaults to current value if not set.");
        AddArgument("active-interval", 0, UINT32_MAX, &mActiveRetransmitMs,
                    "Our MRP active interval (SAI) in milliseconds.  Defaults to current value if not set.");
        AddArgument("active-threshold", 0, UINT32_MAX, &mActiveThresholdMs,
                    "Our MRP active threshold: how long we stay in active mode before transitioning to idle mode.  Defaults to "
                    "current value if not set.");
        AddArgument("additional-retransmit-delay", 0, UINT32_MAX, &mAdditionalRetransmitDelayMs,
                    "Additional delay between retransmits that we do. Defaults to current value if not set.");
    }

protected:
    /////////// CHIPCommandBridge Interface /////////
    CHIP_ERROR RunCommand() override;

    // Our command is synchronous, so no need to wait.
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::kZero; }

    chip::Optional<uint32_t> mIdleRetransmitMs;
    chip::Optional<uint32_t> mActiveRetransmitMs;
    chip::Optional<uint32_t> mActiveThresholdMs;
    chip::Optional<uint64_t> mAdditionalRetransmitDelayMs;
};
