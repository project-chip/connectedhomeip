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
#include <set>
#include <string>

#include "../provider/OTAProviderDelegate.h"

inline constexpr char kIdentityAlpha[] = "alpha";
inline constexpr char kIdentityBeta[] = "beta";
inline constexpr char kIdentityGamma[] = "gamma";
inline constexpr char kControllerIdPrefix[] = "8DCADB14-AF1F-45D0-B084-00000000000";

class CHIPCommandBridge : public Command {
public:
    CHIPCommandBridge(const char * commandName, const char * helpText = nullptr)
        : Command(commandName, helpText)
    {
        AddArgument("commissioner-name", &mCommissionerName);
        AddArgument("commissioner-nodeId", 0, UINT64_MAX, &mCommissionerNodeId,
            "Sets the commissioner node ID of the given "
            "commissioner-name. Interactive mode will only set a single commissioner on the inital command. "
            "The commissioner node ID will be persisted until a different one is specified.");
        AddArgument("commissioner-shared-storage", 0, 1, &mCommissionerSharedStorage,
            "Use a shared storage instance instead of individual storage for each commissioner. Default is false.");
        AddArgument("paa-trust-store-path", &mPaaTrustStorePath,
            "Path to directory holding PAA certificate information.  Can be absolute or relative to the current working "
            "directory.");
        AddArgument(
            "storage-directory", &mStorageDirectory, "This option does nothing. It is here for API compatibility with chip-tool.");
        AddArgument("commissioner-vendor-id", 0, UINT16_MAX, &mCommissionerVendorId,
            "The vendor id to use for darwin-framework-tool. If not provided, chip::VendorId::TestVendor1 (65521, 0xFFF1) will be "
            "used.");
        AddArgument("use-xpc", &mUseXPC, "Use a controller that will connect to an XPC endpoint instead of talking to devices directly. If a string argument is provided, it must identify a Mach service name that can be used to connect to a remote endpoint. If no argument is provided, a local endpoint will be used.");
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

    static NSNumber * GetCommissionerFabricId(const char * identity);

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
    NSNumber * CurrentCommissionerFabricId();

    MTRDeviceController * GetCommissioner(const char * identity);

    // Returns the MTRBaseDevice for the specified node ID.
    // Will utilize an existing PASE connection if the device is being commissioned.
    MTRBaseDevice * BaseDeviceWithNodeId(chip::NodeId nodeId);

    // Returns the MTRDevice for the specified node ID.
    // Will utilize an existing PASE connection if the device is being commissioned.
    MTRDevice * DeviceWithNodeId(chip::NodeId nodeId);

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

    void SuspendOrResumeCommissioners();

    MTRDevice * GetLastUsedDevice();

private:
    CHIP_ERROR InitializeCommissioner(
        std::string key, chip::FabricId fabricId, const chip::Credentials::AttestationTrustStore * trustStore);
    void ShutdownCommissioner();
    uint16_t CurrentCommissionerIndex();

    CHIP_ERROR mCommandExitStatus = CHIP_ERROR_INTERNAL;

    CHIP_ERROR StartWaiting(chip::System::Clock::Timeout seconds);
    void StopWaiting();

    CHIP_ERROR MaybeSetUpStack();
    CHIP_ERROR SetUpStackWithSharedStorage(NSArray<NSData *> * productAttestationAuthorityCertificates);
    CHIP_ERROR SetUpStackWithPerControllerStorage(NSArray<NSData *> * productAttestationAuthorityCertificates);
    void MaybeTearDownStack();

    CHIP_ERROR GetPAACertsFromFolder(NSArray<NSData *> * __autoreleasing * paaCertsResult);

    // Our three controllers: alpha, beta, gamma.
    static std::map<std::string, MTRDeviceController *> mControllers;

    // The current controller; the one the current command should be using.
    MTRDeviceController * mCurrentController;

    static bool sUseSharedStorage;
    chip::Optional<bool> mCommissionerSharedStorage;

    std::condition_variable cvWaitingForResponse;
    std::mutex cvWaitingForResponseMutex;
    chip::Optional<char *> mCommissionerName;
    chip::Optional<uint64_t> mCommissionerNodeId;
    bool mWaitingForResponse { true };
    static dispatch_queue_t mOTAProviderCallbackQueue;
    chip::Optional<char *> mPaaTrustStorePath;
    chip::Optional<chip::VendorId> mCommissionerVendorId;
    std::string mCurrentIdentity;
    chip::Optional<chip::app::DataModel::Nullable<char *>> mUseXPC;
};
