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

#ifdef CONFIG_USE_LOCAL_STORAGE
#include "../../config/PersistentStorage.h"
#endif // CONFIG_USE_LOCAL_STORAGE

#include "Command.h"

#include <commands/common/CredentialIssuerCommands.h>
#include <commands/example/ExampleCredentialIssuerCommands.h>
#include <credentials/GroupDataProviderImpl.h>
#include <credentials/PersistentStorageOpCertStore.h>
#include <crypto/PersistentStorageOperationalKeystore.h>

#pragma once

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
    using ChipDeviceCommissioner = ::chip::Controller::DeviceCommissioner;
    using ChipDeviceController   = ::chip::Controller::DeviceController;
    using IPAddress              = ::chip::Inet::IPAddress;
    using NodeId                 = ::chip::NodeId;
    using PeerId                 = ::chip::PeerId;
    using PeerAddress            = ::chip::Transport::PeerAddress;

    static constexpr uint16_t kMaxGroupsPerFabric    = 5;
    static constexpr uint16_t kMaxGroupKeysPerFabric = 8;

    CHIPCommand(const char * commandName, CredentialIssuerCommands * credIssuerCmds, const char * helpText = nullptr) :
        Command(commandName, helpText), mCredIssuerCmds(credIssuerCmds)
    {
        AddArgument("paa-trust-store-path", &mPaaTrustStorePath,
                    "Path to directory holding PAA certificate information.  Can be absolute or relative to the current working "
                    "directory.");
        AddArgument("cd-trust-store-path", &mCDTrustStorePath,
                    "Path to directory holding CD certificate information.  Can be absolute or relative to the current working "
                    "directory.");
        AddArgument("commissioner-name", &mCommissionerName,
                    "Name of fabric to use. Valid values are \"alpha\", \"beta\", \"gamma\", and integers greater than or equal to "
                    "4.  The default if not specified is \"alpha\".");
        AddArgument("commissioner-nodeid", 0, UINT64_MAX, &mCommissionerNodeId,
                    "The node id to use for chip-tool.  If not provided, kTestControllerNodeId (112233, 0x1B669) will be used.");
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

    // Execute any deferred cleanups.  Used when exiting interactive mode.
    static void ExecuteDeferredCleanups(intptr_t ignored);

#ifdef CONFIG_USE_LOCAL_STORAGE
    PersistentStorage mDefaultStorage;
    // TODO: It's pretty weird that we re-init mCommissionerStorage for every
    // identity without shutting it down or something in between...
    PersistentStorage mCommissionerStorage;
#endif // CONFIG_USE_LOCAL_STORAGE
    chip::PersistentStorageOperationalKeystore mOperationalKeystore;
    chip::Credentials::PersistentStorageOpCertStore mOpCertStore;

    static chip::Credentials::GroupDataProviderImpl sGroupDataProvider;
    CredentialIssuerCommands * mCredIssuerCmds;

    std::string GetIdentity();
    CHIP_ERROR GetCommissionerNodeId(std::string identity, chip::NodeId * nodeId);
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
    };

    // InitializeCommissioner uses various members, so can't be static.  This is
    // obviously a little odd, since the commissioners are then shared across
    // multiple commands in interactive mode...
    CHIP_ERROR InitializeCommissioner(const CommissionerIdentity & identity, chip::FabricId fabricId);
    void ShutdownCommissioner(const CommissionerIdentity & key);
    chip::FabricId CurrentCommissionerId();

    static std::map<CommissionerIdentity, std::unique_ptr<ChipDeviceCommissioner>> mCommissioners;
    static std::set<CHIPCommand *> sDeferredCleanups;

    chip::Optional<char *> mCommissionerName;
    chip::Optional<chip::NodeId> mCommissionerNodeId;
    chip::Optional<uint16_t> mBleAdapterId;
    chip::Optional<char *> mPaaTrustStorePath;
    chip::Optional<char *> mCDTrustStorePath;
    chip::Optional<bool> mUseMaxSizedCerts;
    chip::Optional<bool> mOnlyAllowTrustedCdKeys;

    // Cached trust store so commands other than the original startup command
    // can spin up commissioners as needed.
    static const chip::Credentials::AttestationTrustStore * sTrustStore;

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
    chip::Optional<bool> mTraceDecode;
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
};
