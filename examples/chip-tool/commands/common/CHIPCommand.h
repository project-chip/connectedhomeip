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
#include <commands/common/CredentialIssuerCommands.h>
#include <commands/example/ExampleCredentialIssuerCommands.h>

#pragma once

class PersistentStorage;

constexpr const char kIdentityAlpha[] = "alpha";
constexpr const char kIdentityBeta[]  = "beta";
constexpr const char kIdentityGamma[] = "gamma";
// The null fabric commissioner is a commissioner that isn't on a fabric.
// This is a legal configuration in which the commissioner delegates
// operational communication and invocation of the commssioning complete
// command to a separate on-fabric administrator node.
//
// The null-fabric-commissioner identity is provided here to demonstrate the
// commissioner portion of such an architecture.  The null-fabric-commissioner
// can carry a commissioning flow up until the point of operational channel
// (CASE) communcation.
constexpr const char kIdentityNull[] = "null-fabric-commissioner";

class CHIPCommand : public Command
{
public:
    using ChipDevice             = ::chip::DeviceProxy;
    using ChipDeviceCommissioner = ::chip::Controller::DeviceCommissioner;
    using ChipDeviceController   = ::chip::Controller::DeviceController;
    using IPAddress              = ::chip::Inet::IPAddress;
    using NodeId                 = ::chip::NodeId;
    using PeerId                 = ::chip::PeerId;
    using PeerAddress            = ::chip::Transport::PeerAddress;

    CHIPCommand(const char * commandName, CredentialIssuerCommands * credIssuerCmds) :
        Command(commandName), mCredIssuerCmds(credIssuerCmds)
    {
        AddArgument("commissioner-name", &mCommissionerName);
#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
        AddArgument("trace_file", &mTraceFile);
        AddArgument("trace_log", 0, 1, &mTraceLog);
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
        AddArgument("ble-adapter", 0, UINT64_MAX, &mBleAdapterId);
    }

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
    virtual chip::System::Clock::Timeout GetWaitDuration() const = 0;

    // Shut down the command, in case any work needs to be done after the event
    // loop has been stopped.
    virtual void Shutdown() {}

    PersistentStorage mDefaultStorage;
    PersistentStorage mCommissionerStorage;
    chip::SimpleFabricStorage mFabricStorage;
    CredentialIssuerCommands * mCredIssuerCmds;

    std::string GetIdentity();
    void SetIdentity(const char * name);

    // This method returns the commissioner instance to be used for running the command.
    // The default commissioner instance name is "alpha", but it can be overridden by passing
    // --identity "instance name" when running a command.
    ChipDeviceCommissioner & CurrentCommissioner();

private:
    CHIP_ERROR InitializeCommissioner(std::string key, chip::FabricId fabricId);
    CHIP_ERROR ShutdownCommissioner(std::string key);
    chip::FabricId CurrentCommissionerId();
    std::map<std::string, std::unique_ptr<ChipDeviceCommissioner>> mCommissioners;
    chip::Optional<char *> mCommissionerName;
    chip::Optional<uint16_t> mBleAdapterId;

    static void RunQueuedCommand(intptr_t commandArg);

    CHIP_ERROR mCommandExitStatus = CHIP_ERROR_INTERNAL;

    CHIP_ERROR StartWaiting(chip::System::Clock::Timeout seconds);
    void StopWaiting();

#if CONFIG_USE_SEPARATE_EVENTLOOP
    std::condition_variable cvWaitingForResponse;
    std::mutex cvWaitingForResponseMutex;
    bool mWaitingForResponse{ true };
#endif // CONFIG_USE_SEPARATE_EVENTLOOP

    void StartTracing();
    void StopTracing();

#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
    chip::Optional<char *> mTraceFile;
    chip::Optional<bool> mTraceLog;
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
};
