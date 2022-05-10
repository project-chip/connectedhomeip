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
#import <CHIP/CHIPDeviceController.h>
#include <commands/common/Command.h>
#include <commands/common/CredentialIssuerCommands.h>
#include <map>
#include <string>

#pragma once

constexpr const char kIdentityAlpha[] = "alpha";
constexpr const char kIdentityBeta[]  = "beta";
constexpr const char kIdentityGamma[] = "gamma";

class CHIPCommandBridge : public Command
{
public:
    CHIPCommandBridge(const char * commandName) : Command(commandName) { AddArgument("commissioner-name", &mCommissionerName); }

    /////////// Command Interface /////////
    CHIP_ERROR Run() override;

    void SetCommandExitStatus(CHIP_ERROR status)
    {
#if CHIP_CONFIG_ERROR_SOURCE
        // If there is a filename in the status makes a copy of the filename as the pointer may be released
        // when the autorelease pool is drained.
        strncpy(mCommandExitStatusFilename, status.GetFile(), sizeof(mCommandExitStatusFilename));
        mCommandExitStatusFilename[sizeof(mCommandExitStatusFilename) - 1] = '\0';
        mCommandExitStatus = chip::ChipError(status.AsInteger(), mCommandExitStatusFilename, status.GetLine());
#else
        mCommandExitStatus = status;
#endif // CHIP_CONFIG_ERROR_SOURCE
        ShutdownCommissioner();
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

    void SetIdentity(const char * identity);

    // This method returns the commissioner instance to be used for running the command.
    CHIPDeviceController * CurrentCommissioner();

    CHIPDeviceController * GetCommissioner(const char * identity);

private:
    CHIP_ERROR InitializeCommissioner(std::string key, chip::FabricId fabricId,
                                      const chip::Credentials::AttestationTrustStore * trustStore);
    CHIP_ERROR ShutdownCommissioner();
    uint16_t CurrentCommissionerIndex();

#if CHIP_CONFIG_ERROR_SOURCE
    char mCommandExitStatusFilename[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];
#endif // CHIP_CONFIG_ERROR_SOURCE
    CHIP_ERROR mCommandExitStatus = CHIP_ERROR_INTERNAL;

    CHIP_ERROR StartWaiting(chip::System::Clock::Timeout seconds);
    void StopWaiting();

    // Our three controllers: alpha, beta, gamma.
    std::map<std::string, CHIPDeviceController *> mControllers;

    // The current controller; the one the current command should be using.
    CHIPDeviceController * mCurrentController;

    std::condition_variable cvWaitingForResponse;
    std::mutex cvWaitingForResponseMutex;
    chip::Optional<char *> mCommissionerName;
    bool mWaitingForResponse{ true };
};
