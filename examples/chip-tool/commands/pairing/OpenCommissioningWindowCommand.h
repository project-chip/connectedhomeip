/*
 *   Copyright (c) 2021-2022 Project CHIP Authors
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

#pragma once

#include "../common/CHIPCommand.h"

#include <controller/CommissioningWindowOpener.h>
#include <lib/support/CHIPMem.h>

class OpenCommissioningWindowCommand : public CHIPCommand
{
public:
    OpenCommissioningWindowCommand(CredentialIssuerCommands * credIssuerCommands) :
        CHIPCommand("open-commissioning-window", credIssuerCommands),
        mOnOpenCommissioningWindowCallback(OnOpenCommissioningWindowResponse, this),
        mOnOpenBasicCommissioningWindowCallback(OnOpenBasicCommissioningWindowResponse, this)
    {
        AddArgument("node-id", 0, UINT64_MAX, &mNodeId, "Node to send command to.");
        AddArgument("option", 0, 2, &mCommissioningWindowOption,
                    "1 to use Enhanced Commissioning Method.\n  0 to use Basic Commissioning Method.");
        AddArgument("window-timeout", 0, UINT16_MAX, &mCommissioningWindowTimeout,
                    "Time, in seconds, before the commissioning window closes.");
        AddArgument("iteration", chip::Crypto::kSpake2p_Min_PBKDF_Iterations, chip::Crypto::kSpake2p_Max_PBKDF_Iterations,
                    &mIteration, "Number of PBKDF iterations to use to derive the verifier.  Ignored if 'option' is 0.");
        AddArgument("discriminator", 0, 4095, &mDiscriminator, "Discriminator to use for advertising.  Ignored if 'option' is 0.");
        AddArgument("timeout", 0, UINT16_MAX, &mTimeout, "Time, in seconds, before this command is considered to have timed out.");
    }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;
    // We issue multiple data model operations for this command, and the default
    // timeout for those is 10 seconds, so default to 20 seconds.
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(mTimeout.ValueOr(20)); }

private:
    NodeId mNodeId;
    chip::Controller::CommissioningWindowOpener::CommissioningWindowOption mCommissioningWindowOption;
    uint16_t mCommissioningWindowTimeout;
    uint32_t mIteration;
    uint16_t mDiscriminator;

    chip::Optional<uint16_t> mTimeout;

    chip::Platform::UniquePtr<chip::Controller::CommissioningWindowOpener> mWindowOpener;

    static void OnOpenCommissioningWindowResponse(void * context, NodeId deviceId, CHIP_ERROR status, chip::SetupPayload payload);
    static void OnOpenBasicCommissioningWindowResponse(void * context, NodeId deviceId, CHIP_ERROR status);

    chip::Callback::Callback<chip::Controller::OnOpenCommissioningWindow> mOnOpenCommissioningWindowCallback;
    chip::Callback::Callback<chip::Controller::OnOpenBasicCommissioningWindow> mOnOpenBasicCommissioningWindowCallback;
};
