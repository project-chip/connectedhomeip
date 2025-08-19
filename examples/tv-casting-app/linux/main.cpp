/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <signal.h>

#include "commands/clusters/SubscriptionsCommands.h"
#include "commands/common/Commands.h"
#include "commands/example/ExampleCredentialIssuerCommands.h"
#include <zap-generated/cluster/Commands.h>

#include "CastingServer.h"
#include "CastingUtils.h"
#if defined(ENABLE_CHIP_SHELL)
#include "CastingShellCommands.h"
#include <lib/shell/Engine.h> // nogncheck
#include <thread>
#endif

#include "LinuxCommissionableDataProvider.h"
#include "Options.h"
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <data-model-providers/codegen/Instance.h>
#include <platform/TestOnlyCommissionableDataProvider.h>

using namespace chip;
using chip::ArgParser::HelpOptions;
using chip::ArgParser::OptionDef;
using chip::ArgParser::OptionSet;

#if defined(ENABLE_CHIP_SHELL)
using chip::Shell::Engine;
#endif

CHIP_ERROR InitCommissionableDataProvider(LinuxCommissionableDataProvider & provider, LinuxDeviceOptions & options)
{
    chip::Optional<uint32_t> setupPasscode;

    if (options.payload.setUpPINCode != 0)
    {
        setupPasscode.SetValue(options.payload.setUpPINCode);
    }
    else if (!options.spake2pVerifier.HasValue())
    {
        uint32_t defaultTestPasscode = 0;
        chip::DeviceLayer::TestOnlyCommissionableDataProvider TestOnlyCommissionableDataProvider;
        VerifyOrDie(TestOnlyCommissionableDataProvider.GetSetupPasscode(defaultTestPasscode) == CHIP_NO_ERROR);

        ChipLogError(Support,
                     "*** WARNING: Using temporary passcode %u due to no neither --passcode or --spake2p-verifier-base64 "
                     "given on command line. This is temporary and will disappear. Please update your scripts "
                     "to explicitly configure onboarding credentials. ***",
                     static_cast<unsigned>(defaultTestPasscode));
        setupPasscode.SetValue(defaultTestPasscode);
        options.payload.setUpPINCode = defaultTestPasscode;
    }
    else
    {
        // Passcode is 0, so will be ignored, and verifier will take over. Onboarding payload
        // printed for debug will be invalid, but if the onboarding payload had been given
        // properly to the commissioner later, PASE will succeed.
    }

    // Default to minimum PBKDF iterations
    uint32_t spake2pIterationCount = chip::Crypto::kSpake2p_Min_PBKDF_Iterations;
    if (options.spake2pIterations != 0)
    {
        spake2pIterationCount = options.spake2pIterations;
    }
    ChipLogError(Support, "PASE PBKDF iterations set to %u", static_cast<unsigned>(spake2pIterationCount));

    return provider.Init(options.spake2pVerifier, options.spake2pSalt, spake2pIterationCount, setupPasscode,
                         options.payload.discriminator.GetLongValue());
}

// To hold SPAKE2+ verifier, discriminator, passcode
LinuxCommissionableDataProvider gCommissionableDataProvider;

// For shell and command line processing of commands
ExampleCredentialIssuerCommands gCredIssuerCommands;
Commands gCommands;

CHIP_ERROR ProcessClusterCommand(int argc, char ** argv)
{
    if (!CastingServer::GetInstance()->GetActiveTargetVideoPlayer()->IsInitialized())
    {
        CastingServer::GetInstance()->SetDefaultFabricIndex(OnConnectionSuccess, OnConnectionFailure, OnNewOrUpdatedEndpoint);
    }
    gCommands.Run(argc, argv);
    return CHIP_NO_ERROR;
}

void StopMainEventLoop()
{
    Server::GetInstance().GenerateShutDownEvent();
    DeviceLayer::SystemLayer().ScheduleLambda([]() { DeviceLayer::PlatformMgr().StopEventLoopTask(); });
}

void StopSignalHandler(int /* signal */)
{
#if defined(ENABLE_CHIP_SHELL)
    Engine::Root().StopMainLoop();
#endif
    StopMainEventLoop();
}

int main(int argc, char * argv[])
{
    // This file is built/run only if chip_casting_simplified = 0
    ChipLogProgress(AppServer, "chip_casting_simplified = 0");

#if defined(ENABLE_CHIP_SHELL)
    /* Block SIGINT and SIGTERM. Other threads created by the main thread
     * will inherit the signal mask. Then we can explicitly unblock signals
     * in the shell thread to handle them, so the read(stdin) call can be
     * interrupted by a signal. */
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &set, nullptr);
#endif

    VerifyOrDie(CHIP_NO_ERROR == chip::Platform::MemoryInit());
    VerifyOrDie(CHIP_NO_ERROR == chip::DeviceLayer::PlatformMgr().InitChipStack());

#if defined(ENABLE_CHIP_SHELL)
    Engine::Root().Init();
    Shell::RegisterCastingCommands();
    std::thread shellThread([]() {
        sigset_t set_;
        sigemptyset(&set_);
        sigaddset(&set_, SIGINT);
        sigaddset(&set_, SIGTERM);
        // Unblock SIGINT and SIGTERM, so that the shell thread can handle
        // them - we need read() call to be interrupted.
        pthread_sigmask(SIG_UNBLOCK, &set_, nullptr);
        Engine::Root().RunMainLoop();
        StopMainEventLoop();
    });
#endif

    CHIP_ERROR err = CHIP_NO_ERROR;

    DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init(CHIP_CONFIG_KVS_PATH);

    // Init the commissionable data provider based on command line options
    // to handle custom verifiers, discriminators, etc.
    err = InitCommissionableDataProvider(gCommissionableDataProvider, LinuxDeviceOptions::GetInstance());
    SuccessOrExit(err);
    DeviceLayer::SetCommissionableDataProvider(&gCommissionableDataProvider);

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(chip::Credentials::Examples::GetExampleDACProvider());

    // Initialize device attestation verifier from a constant version
    {
        // TODO: Replace testingRootStore with a AttestationTrustStore that has the necessary official PAA roots available
        const chip::Credentials::AttestationTrustStore * testingRootStore = chip::Credentials::GetTestAttestationTrustStore();
        chip::Credentials::DeviceAttestationRevocationDelegate * kDeviceAttestationRevocationNotChecked = nullptr;
        SetDeviceAttestationVerifier(GetDefaultDACVerifier(testingRootStore, kDeviceAttestationRevocationNotChecked));
    }

    SuccessOrExit(err = CastingServer::GetInstance()->PreInit());

    // Enter commissioning mode, open commissioning window
    static chip::CommonCaseDeviceServerInitParams initParams;
    VerifyOrDie(CHIP_NO_ERROR == initParams.InitializeStaticResourcesBeforeServerInit());
    initParams.dataModelProvider = app::CodegenDataModelProviderInstance(initParams.persistentStorageDelegate);
    VerifyOrDie(CHIP_NO_ERROR == chip::Server::GetInstance().Init(initParams));

    if (argc > 1)
    {
        ChipLogProgress(AppServer, "Command line parameters detected. Skipping auto-start.");
    }
    else if (ConnectToCachedVideoPlayer() == CHIP_NO_ERROR)
    {
        ChipLogProgress(AppServer, "Skipping commissioner discovery / User directed commissioning flow.");
    }
    else
    {
        // Send discover commissioners request
        SuccessOrExit(err = CastingServer::GetInstance()->DiscoverCommissioners());

        // Give commissioners some time to respond and then ScheduleWork to initiate commissioning
        DeviceLayer::SystemLayer().StartTimer(
            chip::System::Clock::Milliseconds32(kCommissionerDiscoveryTimeoutInMs),
            [](System::Layer *, void *) { chip::DeviceLayer::PlatformMgr().ScheduleWork(InitCommissioningFlow); }, nullptr);
    }

    registerClusters(gCommands, &gCredIssuerCommands);
    registerCommandsSubscriptions(gCommands, &gCredIssuerCommands);

    if (argc > 1)
    {
        // if there are command-line arguments, then automatically start server
        ProcessClusterCommand(argc, argv);
    }

    {
        struct sigaction sa = {};
        sa.sa_handler       = StopSignalHandler;
        sa.sa_flags         = SA_RESETHAND;
        sigaction(SIGINT, &sa, nullptr);
        sigaction(SIGTERM, &sa, nullptr);
    }

    DeviceLayer::PlatformMgr().RunEventLoop();

exit:

#if defined(ENABLE_CHIP_SHELL)
    shellThread.join();
#endif

    chip::Server::GetInstance().Shutdown();
    chip::DeviceLayer::PlatformMgr().Shutdown();

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to run TV Casting App: %s", ErrorStr(err));
        // End the program with non zero error code to indicate an error.
        return 1;
    }
    return 0;
}
