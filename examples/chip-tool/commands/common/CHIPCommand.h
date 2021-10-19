/*
 *   Copyright (c) 2021 Project CHIP Authors
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

#include "../../config/PersistentStorage.h"
#include "Command.h"

#pragma once

class PersistentStorage;

class CHIPCommand : public Command
{
public:
    using ChipDevice             = ::chip::Controller::Device;
    using ChipDeviceCommissioner = ::chip::Controller::DeviceCommissioner;
    using ChipDeviceController   = ::chip::Controller::DeviceController;
    using ChipSerializedDevice   = ::chip::Controller::SerializedDevice;
    using IPAddress              = ::chip::Inet::IPAddress;
    using NodeId                 = ::chip::NodeId;
    using PeerAddress            = ::chip::Transport::PeerAddress;

    CHIPCommand(const char * commandName) : Command(commandName) {}

    /////////// Command Interface /////////
    CHIP_ERROR Run() override;

    void SetCommandExitStatus(CHIP_ERROR status)
    {
        mCommandExitStatus = status;
        StopWaiting();
    }

protected:
    // Will be called in a setting in which it's safe to touch the CHIP
    // stack. The rules for Run() are as follows:
    //
    // 1) If error is returned, Run() must not call SetCommandExitStatus.
    // 2) If success is returned Run() must either have called
    //    SetCommandExitStatus() or scheduled async work that will do that.
    virtual CHIP_ERROR RunCommand() = 0;

    // Get the wait duration, in seconds, before the command times out.
    virtual uint16_t GetWaitDurationInSeconds() const = 0;

    // Shut down the command, in case any work needs to be done after the event
    // loop has been stopped.
    virtual void Shutdown() {}

    ChipDeviceCommissioner mController;
    PersistentStorage mStorage;

private:
    static void RunQueuedCommand(intptr_t commandArg);

    CHIP_ERROR mCommandExitStatus = CHIP_ERROR_INTERNAL;
    chip::Controller::ExampleOperationalCredentialsIssuer mOpCredsIssuer;

    CHIP_ERROR StartWaiting(uint16_t seconds);
    void StopWaiting();

#if CONFIG_USE_SEPARATE_EVENTLOOP
    std::condition_variable cvWaitingForResponse;
    std::mutex cvWaitingForResponseMutex;
    bool mWaitingForResponse{ true };
#endif // CONFIG_USE_SEPARATE_EVENTLOOP
};
