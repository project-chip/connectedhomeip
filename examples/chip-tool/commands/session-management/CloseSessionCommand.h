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

namespace detail {

class SessionManagementCommand : public CHIPCommand
{
public:
    SessionManagementCommand(const char * commandName, CredentialIssuerCommands * credIssuerCommands, const char * helpText) :
        CHIPCommand(commandName, credIssuerCommands, helpText)
    {
        AddArgument("node-id", 0, UINT64_MAX, &mDestinationNodeId);
    }

protected:
    chip::NodeId mDestinationNodeId;
};

} // namespace detail

class SendCloseSessionCommand : public detail::SessionManagementCommand
{
public:
    SendCloseSessionCommand(CredentialIssuerCommands * credIssuerCommands) :
        detail::SessionManagementCommand("send-close-session", credIssuerCommands,
                                         "Sends a CloseSession message to the given node id."),
        mOnDeviceConnectedCallback(OnDeviceConnectedFn, this), mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this)
    {
        AddArgument("timeout", 0, UINT64_MAX, &mTimeoutSecs,
                    "Time, in seconds, before this command is considered to have timed out.");
        AddArgument("evict-local-session", 0, 1, &mEvictLocalSession,
                    "If true, evicts the local session after sending the message. If false, leaves it around. Defaults to true.");
    }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;
    chip::System::Clock::Timeout GetWaitDuration() const override
    {
        return chip::System::Clock::Seconds16(mTimeoutSecs.ValueOr(10));
    }

private:
    chip::Optional<uint16_t> mTimeoutSecs;
    chip::Optional<bool> mEvictLocalSession;

    static void OnDeviceConnectedFn(void * context, chip::Messaging::ExchangeManager & exchangeMgr,
                                    const chip::SessionHandle & sessionHandle);
    static void OnDeviceConnectionFailureFn(void * context, const chip::ScopedNodeId & peerId, CHIP_ERROR error);

    // Try to send the CloseSession status report.
    CHIP_ERROR CloseSession(chip::Messaging::ExchangeManager & exchangeMgr, const chip::SessionHandle & sessionHandle);

    chip::Callback::Callback<chip::OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;
};

class EvictLocalCASESessionsCommand : public detail::SessionManagementCommand
{
public:
    EvictLocalCASESessionsCommand(CredentialIssuerCommands * credIssuerCommands) :
        detail::SessionManagementCommand("expire-case-sessions", credIssuerCommands,
                                         "Expires (evicts) all local CASE sessions to the given node id.")
    {}

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;
    chip::System::Clock::Timeout GetWaitDuration() const override
    {
        // This command does all its work synchronously, so it really does not matter too much.
        return chip::System::Clock::Seconds16(5);
    }
};
