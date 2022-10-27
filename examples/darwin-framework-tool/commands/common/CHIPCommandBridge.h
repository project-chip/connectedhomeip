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
#import <Matter/Matter.h>
#include <commands/common/Command.h>
#include <commands/common/CredentialIssuerCommands.h>
#include <map>
#include <string>

#include "../provider/OTAProviderDelegate.h"

#pragma once

constexpr const char kIdentityAlpha[] = "alpha";
constexpr const char kIdentityBeta[] = "beta";
constexpr const char kIdentityGamma[] = "gamma";

class CHIPCommandBridge : public Command {
public:
    CHIPCommandBridge(const char * commandName)
        : Command(commandName)
    {
        AddArgument("commissioner-name", &mCommissionerName);
        AddArgument("paa-trust-store-path", &mPaaTrustStorePath,
            "Path to directory holding PAA certificate information.  Can be absolute or relative to the current working "
            "directory.");
    }

    /////////// Command Interface /////////
    CHIP_ERROR Run() override;

    // Will convert error to a CHIP_ERROR and call SetCommandExitStatus with the
    // result.  If a log string is provided, will log that plus the string
    // representation of the CHIP_ERROR.
    void SetCommandExitStatus(NSError * error, const char * logString = nullptr);

    void SetCommandExitStatus(CHIP_ERROR status)
    {
        mCommandExitStatus = status;
        StopWaiting();
    }

    static OTAProviderDelegate * mOTADelegate;

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
    MTRDeviceController * CurrentCommissioner();

    MTRDeviceController * GetCommissioner(const char * identity);

    // Will log the given string and given error (as progress if success, error
    // if failure).
    void LogNSError(const char * logString, NSError * error);

    // Clean up any resources allocated by the command.  Some commands may hold
    // on to resources after Shutdown(), but Cleanup() will guarantee those are
    // cleaned up.
    virtual void Cleanup() {}

    // If true, skip calling Cleanup() when in interactive mode, so the command
    // can keep doing work as needed.  Cleanup() will be called when quitting
    // interactive mode.  This method will be called before Shutdown, so it can
    // use member values that Shutdown will normally reset.
    virtual bool DeferInteractiveCleanup() { return NO; }

    // Execute any deferred cleanups.  Used when exiting interactive mode.
    void ExecuteDeferredCleanups();

    static std::set<CHIPCommandBridge *> sDeferredCleanups;

    void StopCommissioners();

    void RestartCommissioners();

private:
    CHIP_ERROR InitializeCommissioner(
        std::string key, chip::FabricId fabricId, const chip::Credentials::AttestationTrustStore * trustStore);
    void ShutdownCommissioner();
    uint16_t CurrentCommissionerIndex();

    CHIP_ERROR mCommandExitStatus = CHIP_ERROR_INTERNAL;

    CHIP_ERROR StartWaiting(chip::System::Clock::Timeout seconds);
    void StopWaiting();

    CHIP_ERROR MaybeSetUpStack();
    void MaybeTearDownStack();

    CHIP_ERROR GetPAACertsFromFolder(NSArray<NSData *> * __autoreleasing * paaCertsResult);

    // Our three controllers: alpha, beta, gamma.
    static std::map<std::string, MTRDeviceController *> mControllers;

    // The current controller; the one the current command should be using.
    MTRDeviceController * mCurrentController;

    std::condition_variable cvWaitingForResponse;
    std::mutex cvWaitingForResponseMutex;
    chip::Optional<char *> mCommissionerName;
    bool mWaitingForResponse { true };
    static dispatch_queue_t mOTAProviderCallbackQueue;
    chip::Optional<char *> mPaaTrustStorePath;
};
