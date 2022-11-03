/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
