/*
 *   Copyright (c) 2025 Project CHIP Authors
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

#ifdef CONFIG_USE_LOCAL_STORAGE
#include <controller/ExamplePersistentStorage.h>
#endif // CONFIG_USE_LOCAL_STORAGE

#include "Command.h"

#include <TracingCommandLineArgument.h>
#include <commands/common/CredentialIssuerCommands.h>
#include <commands/example/ExampleCredentialIssuerCommands.h>
#include <credentials/GroupDataProviderImpl.h>
#include <credentials/PersistentStorageOpCertStore.h>
#include <crypto/PersistentStorageOperationalKeystore.h>
#include <crypto/RawKeySessionKeystore.h>

#include <string>

inline constexpr char kIdentityAlpha[] = "alpha";
inline constexpr char kIdentityBeta[]  = "beta";
inline constexpr char kIdentityGamma[] = "gamma";
// The null fabric commissioner is a commissioner that isn't on a fabric.
// This is a legal configuration in which the commissioner delegates
// operational communication and invocation of the commssioning complete
// command to a separate on-fabric administrator node.
//
// The null-fabric-commissioner identity is provided here to demonstrate the
// commissioner portion of such an architecture.  The null-fabric-commissioner
// can carry a commissioning flow up until the point of operational channel
// (CASE) communcation.
inline constexpr char kIdentityNull[] = "null-fabric-commissioner";

constexpr uint16_t kMaxCommandSize = 384;

class CHIPCommand : public Command
{
public:
    using ChipDeviceCommissioner = ::chip::Controller::DeviceCommissioner;
    using ChipDeviceController   = ::chip::Controller::DeviceController;
    using IPAddress              = ::chip::Inet::IPAddress;
    using NodeId                 = ::chip::NodeId;
    using PeerId                 = ::chip::PeerId;
    using PeerAddress            = ::chip::Transport::PeerAddress;

    static constexpr uint16_t kMaxGroupsPerFabric    = 50;
    static constexpr uint16_t kMaxGroupKeysPerFabric = 25;

    CHIPCommand(const char * commandName, CredentialIssuerCommands * credIssuerCmds, const char * helpText = nullptr) :
        Command(commandName, helpText), mCredIssuerCmds(credIssuerCmds)
    {
        AddArgument("log-file-path", &mLogFilePath,
                    "Path to the log file where the output is redirected.  Can be absolute or relative to the current working "
                    "directory.");
        AddArgument("paa-trust-store-path", &mPaaTrustStorePath,
                    "Path to directory holding PAA certificate information.  Can be absolute or relative to the current working "
                    "directory.");
        AddArgument("cd-trust-store-path", &mCDTrustStorePath,
                    "Path to directory holding CD certificate information.  Can be absolute or relative to the current working "
                    "directory.");
        AddArgument("commissioner-name", &mCommissionerName,
                    "Name of fabric to use. Valid values are \"alpha\", \"beta\", \"gamma\", and integers greater than or equal to "
                    "4.  The default if not specified is \"alpha\".");
        AddArgument(
            "commissioner-nodeid", 0, UINT64_MAX, &mCommissionerNodeId,
            "The node id to use for camera-controller.  If not provided, kTestControllerNodeId (112233, 0x1B669) will be used.");
        AddArgument("use-max-sized-certs", 0, 1, &mUseMaxSizedCerts,
                    "Maximize the size of operational certificates. If not provided or 0 (\"false\"), normally sized operational "
                    "certificates are generated.");
        AddArgument("only-allow-trusted-cd-keys", 0, 1, &mOnlyAllowTrustedCdKeys,
                    "Only allow trusted CD verifying keys (disallow test keys). If not provided or 0 (\"false\"), untrusted CD "
                    "verifying keys are allowed. If 1 (\"true\"), test keys are disallowed.");
#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
        AddArgument("trace_file", &mTraceFile);
        AddArgument("trace_log", 0, 1, &mTraceLog);
        AddArgument("trace_decode", 0, 1, &mTraceDecode);
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
        AddArgument("trace-to", &mTraceTo, "Trace destinations, comma-separated (" SUPPORTED_COMMAND_LINE_TRACING_TARGETS ")");
        AddArgument("ble-adapter", 0, UINT16_MAX, &mBleAdapterId);
        AddArgument("storage-directory", &mStorageDirectory,
                    "Directory to place camera-controller's storage files in.  Defaults to $TMPDIR, with fallback to /tmp");
        AddArgument("commissioner-vendor-id", 0, UINT16_MAX, &mCommissionerVendorId,
                    "The vendor id to use for camera-controller. If not provided, chip::VendorId::TestVendor1 (65521, 0xFFF1) will "
                    "be used.");
    }

    /////////// Command Interface /////////
    CHIP_ERROR Run() override;

    void SetCommandExitStatus(CHIP_ERROR status)
    {
        mCommandExitStatus = status;
        // In interactive mode the stack is not shut down once a command is ended.
        // That means calling `ErrorStr(err)` from the main thread when command
        // completion is signaled may race since `ErrorStr` uses a static sErrorStr
        // buffer for computing the error string.  Call it here instead.
        if (IsInteractive() && CHIP_NO_ERROR != status)
        {
            ChipLogError(NotSpecified, "Run command failure: %s", chip::ErrorStr(status));
        }
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

    // Shut down the command.  After a Shutdown call the command object is ready
    // to be used for another command invocation.
    virtual void Shutdown() { ResetArguments(); }

    // Clean up any resources allocated by the command.  Some commands may hold
    // on to resources after Shutdown(), but Cleanup() will guarantee those are
    // cleaned up.
    virtual void Cleanup() {}

    // If true, skip calling Cleanup() when in interactive mode, so the command
    // can keep doing work as needed.  Cleanup() will be called when quitting
    // interactive mode.  This method will be called before Shutdown, so it can
    // use member values that Shutdown will normally reset.
    virtual bool DeferInteractiveCleanup() { return false; }

    // If true, the controller will be created with server capabilities enabled,
    // such as advertising operational nodes over DNS-SD and accepting incoming
    // CASE sessions.
    virtual bool NeedsOperationalAdvertising() { return mAdvertiseOperational; }

    // Execute any deferred cleanups.  Used when exiting interactive mode.
    static void ExecuteDeferredCleanups(intptr_t ignored);

#ifdef CONFIG_USE_LOCAL_STORAGE
    PersistentStorage mDefaultStorage;
    // TODO: It's pretty weird that we re-init mCommissionerStorage for every
    // identity without shutting it down or something in between...
    PersistentStorage mCommissionerStorage;
#endif // CONFIG_USE_LOCAL_STORAGE
    chip::Optional<char *> mLogFilePath;
    chip::PersistentStorageOperationalKeystore mOperationalKeystore;
    chip::Credentials::PersistentStorageOpCertStore mOpCertStore;
    static chip::Crypto::RawKeySessionKeystore sSessionKeystore;

    static chip::Credentials::GroupDataProviderImpl sGroupDataProvider;
    CredentialIssuerCommands * mCredIssuerCmds;

    std::string GetIdentity();
    CHIP_ERROR GetIdentityNodeId(std::string identity, chip::NodeId * nodeId);
    CHIP_ERROR GetIdentityRootCertificate(std::string identity, chip::ByteSpan & span);
    void SetIdentity(const char * name);

    // This method returns the commissioner instance to be used for running the command.
    // The default commissioner instance name is "alpha", but it can be overridden by passing
    // --identity "instance name" when running a command.
    ChipDeviceCommissioner & CurrentCommissioner();

    ChipDeviceCommissioner & GetCommissioner(std::string identity);

private:
    CHIP_ERROR MaybeSetUpStack();
    void MaybeTearDownStack();

    CHIP_ERROR EnsureCommissionerForIdentity(std::string identity);

    // Commissioners are keyed by name and local node id.
    struct CommissionerIdentity
    {
        bool operator<(const CommissionerIdentity & other) const
        {
            return mName < other.mName || (mName == other.mName && mLocalNodeId < other.mLocalNodeId);
        }
        std::string mName;
        chip::NodeId mLocalNodeId;
        uint8_t mRCAC[chip::Controller::kMaxCHIPDERCertLength] = {};
        uint8_t mICAC[chip::Controller::kMaxCHIPDERCertLength] = {};
        uint8_t mNOC[chip::Controller::kMaxCHIPDERCertLength]  = {};

        size_t mRCACLen;
        size_t mICACLen;
        size_t mNOCLen;
    };

    // InitializeCommissioner uses various members, so can't be static.  This is
    // obviously a little odd, since the commissioners are then shared across
    // multiple commands in interactive mode...
    CHIP_ERROR InitializeCommissioner(CommissionerIdentity & identity, chip::FabricId fabricId);
    void ShutdownCommissioner(const CommissionerIdentity & key);
    chip::FabricId CurrentCommissionerId();

    static std::map<CommissionerIdentity, std::unique_ptr<ChipDeviceCommissioner>> mCommissioners;
    static std::set<CHIPCommand *> sDeferredCleanups;

    chip::Optional<char *> mCommissionerName;
    chip::Optional<chip::NodeId> mCommissionerNodeId;
    chip::Optional<chip::VendorId> mCommissionerVendorId;
    chip::Optional<uint16_t> mBleAdapterId;
    chip::Optional<char *> mPaaTrustStorePath;
    chip::Optional<char *> mCDTrustStorePath;
    chip::Optional<bool> mUseMaxSizedCerts;
    chip::Optional<bool> mOnlyAllowTrustedCdKeys;

    // Cached trust store so commands other than the original startup command
    // can spin up commissioners as needed.
    static const chip::Credentials::AttestationTrustStore * sTrustStore;

    static void RunQueuedCommand(intptr_t commandArg);
    typedef decltype(RunQueuedCommand) MatterWorkCallback;
    static void RunCommandCleanup(intptr_t commandArg);

    // Do cleanup after a commmand is done running.  Must happen with the
    // Matter stack locked.
    void CleanupAfterRun();

    // Run the given callback on the Matter thread.  Return whether we managed
    // to successfully dispatch it to the Matter thread.  If we did, *timedOut
    // will be set to whether we timed out or whether our mWaitingForResponse
    // got set to false by the callback itself.
    CHIP_ERROR RunOnMatterQueue(MatterWorkCallback callback, chip::System::Clock::Timeout timeout, bool * timedOut);

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
    chip::Optional<bool> mTraceDecode;
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED

    chip::CommandLineApp::TracingSetup mTracingSetup;
    chip::Optional<std::vector<std::string>> mTraceTo;
};
