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

#pragma once

#include "../common/CHIPCommand.h"
#include <app/OperationalSessionSetup.h>
#include <lib/core/CHIPCallback.h>
#include <lib/core/DataModelTypes.h>

class CloseSessionCommand : public CHIPCommand
{
public:
    CloseSessionCommand(CredentialIssuerCommands * credIssuerCommands) :
        CHIPCommand("close-session", credIssuerCommands), mOnDeviceConnectedCallback(OnDeviceConnectedFn, this),
        mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this)
    {
        AddArgument("destination-id", 0, UINT64_MAX, &mDestinationId);
        AddArgument("timeout", 0, UINT64_MAX, &mTimeoutSecs,
                    "Time, in seconds, before this command is considered to have timed out.");
    }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;
    chip::System::Clock::Timeout GetWaitDuration() const override
    {
        return chip::System::Clock::Seconds16(mTimeoutSecs.ValueOr(10));
    }

private:
    chip::NodeId mDestinationId;
    chip::Optional<uint16_t> mTimeoutSecs;

    static void OnDeviceConnectedFn(void * context, chip::Messaging::ExchangeManager & exchangeMgr,
                                    chip::SessionHandle & sessionHandle);
    static void OnDeviceConnectionFailureFn(void * context, const chip::ScopedNodeId & peerId, CHIP_ERROR error);

    // Try to send the action CloseSession status report.
    CHIP_ERROR CloseSession(chip::Messaging::ExchangeManager & exchangeMgr, const chip::SessionHandle & sessionHandle);

    chip::Callback::Callback<chip::OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;
};
