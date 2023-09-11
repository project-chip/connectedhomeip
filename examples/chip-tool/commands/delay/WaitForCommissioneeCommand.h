/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include "../common/CHIPCommand.h"
#include <app/OperationalSessionSetup.h>
#include <lib/core/CHIPCallback.h>

class WaitForCommissioneeCommand : public CHIPCommand
{
public:
    WaitForCommissioneeCommand(CredentialIssuerCommands * credIssuerCommands) :
        CHIPCommand("wait-for-commissionee", credIssuerCommands, "Establish a CASE session to the provided node id."),
        mOnDeviceConnectedCallback(OnDeviceConnectedFn, this), mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this)
    {
        AddArgument("nodeId", 0, UINT64_MAX, &mNodeId);
        AddArgument("expire-existing-session", 0, 1, &mExpireExistingSession);
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
    chip::NodeId mNodeId;
    chip::Optional<uint16_t> mTimeoutSecs;
    chip::Optional<bool> mExpireExistingSession;

    static void OnDeviceConnectedFn(void * context, chip::Messaging::ExchangeManager & exchangeMgr,
                                    const chip::SessionHandle & sessionHandle);
    static void OnDeviceConnectionFailureFn(void * context, const chip::ScopedNodeId & peerId, CHIP_ERROR error);

    chip::Callback::Callback<chip::OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;
};
