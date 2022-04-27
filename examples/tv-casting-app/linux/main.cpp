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

#include "CastingServer.h"
#include "CastingUtils.h"
#include "LinuxCommissionableDataProvider.h"
#include "Options.h"
#include "TargetEndpointInfo.h"
#include "TargetVideoPlayerInfo.h"
#include "app/clusters/bindings/BindingManager.h"
#include <app/OperationalDeviceProxy.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <controller/CHIPCommissionableNodeController.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/SafeInt.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/ConfigurationManager.h>
#include <platform/DeviceControlServer.h>
#include <platform/TestOnlyCommissionableDataProvider.h>
#include <system/SystemClock.h>
#include <system/SystemLayer.h>
#include <transport/raw/PeerAddress.h>
#include <zap-generated/CHIPClusters.h>

#if defined(ENABLE_CHIP_SHELL)
#include "CastingShellCommands.h"
#include <lib/shell/Engine.h>
#include <thread>
#endif

#include <list>
#include <string>

using namespace chip;
using namespace chip::Controller;
using namespace chip::Credentials;
using chip::ArgParser::HelpOptions;
using chip::ArgParser::OptionDef;
using chip::ArgParser::OptionSet;
using namespace chip::app::Clusters::ContentLauncher::Commands;

#if defined(ENABLE_CHIP_SHELL)
using chip::Shell::Engine;
#endif
struct TVExampleDeviceType
{
    const char * name;
    uint16_t id;
};

Dnssd::DiscoveryFilter gDiscoveryFilter           = Dnssd::DiscoveryFilter();
constexpr TVExampleDeviceType kKnownDeviceTypes[] = { { "video-player", 35 }, { "dimmable-light", 257 } };
constexpr int kKnownDeviceTypesCount              = sizeof kKnownDeviceTypes / sizeof *kKnownDeviceTypes;
constexpr uint16_t kOptionDeviceType              = 't';

// TODO: Accept these values over CLI
const char * kContentUrl        = "https://www.test.com/videoid";
const char * kContentDisplayStr = "Test video";

CommissionableNodeController gCommissionableNodeController;
chip::System::SocketWatchToken gToken;
bool gInited = false;

bool HandleOptions(const char * aProgram, OptionSet * aOptions, int aIdentifier, const char * aName, const char * aValue)
{
    switch (aIdentifier)
    {
    case kOptionDeviceType: {
        char * endPtr;
        long deviceType = strtol(aValue, &endPtr, 10);
        if (*endPtr == '\0' && deviceType > 0 && CanCastTo<uint16_t>(deviceType))
        {
            gDiscoveryFilter = Dnssd::DiscoveryFilter(Dnssd::DiscoveryFilterType::kDeviceType, static_cast<uint16_t>(deviceType));
            return true;
        }

        for (int i = 0; i < kKnownDeviceTypesCount; i++)
        {
            if (strcasecmp(aValue, kKnownDeviceTypes[i].name) == 0)
            {
                gDiscoveryFilter = Dnssd::DiscoveryFilter(Dnssd::DiscoveryFilterType::kDeviceType, kKnownDeviceTypes[i].id);
                return true;
            }
        }

        ChipLogError(AppServer, "%s: INTERNAL ERROR: Unhandled option value: %s %s", aProgram, aName, aValue);
        return false;
    }
    default:
        ChipLogError(AppServer, "%s: INTERNAL ERROR: Unhandled option: %s", aProgram, aName);
        return false;
    }
}

OptionDef cmdLineOptionsDef[] = {
    { "device-type", chip::ArgParser::kArgumentRequired, kOptionDeviceType },
    {},
};

OptionSet cmdLineOptions = { HandleOptions, cmdLineOptionsDef, "PROGRAM OPTIONS",
                             "  -t <commissioner device type>\n"
                             "  --device-type <commissioner device type>\n"
                             "        Device type of the commissioner to discover and request commissioning from. Specify value as "
                             "a decimal integer or a known text representation. Defaults to all device types\n" };

HelpOptions helpOptions("tv-casting-app", "Usage: tv-casting-app [options]", "1.0");

OptionSet * allOptions[] = { &cmdLineOptions, &helpOptions, nullptr };

void DeviceEventCallback(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    if (event->Type == DeviceLayer::DeviceEventType::kBindingsChangedViaCluster)
    {
        if (CastingServer::GetInstance()->GetTargetVideoPlayerInfo()->IsInitialized())
        {
            CastingServer::GetInstance()->ReadServerClustersForNode(
                CastingServer::GetInstance()->GetTargetVideoPlayerInfo()->GetNodeId());
        }
    }
    else if (event->Type == DeviceLayer::DeviceEventType::kCommissioningComplete)
    {
        ReturnOnFailure(CastingServer::GetInstance()->GetTargetVideoPlayerInfo()->Initialize(
            event->CommissioningComplete.PeerNodeId, event->CommissioningComplete.PeerFabricIndex));

        CastingServer::GetInstance()->ContentLauncherLaunchURL(kContentUrl, kContentDisplayStr);
    }
}

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
                         options.payload.discriminator);
}

// To hold SPAKE2+ verifier, discriminator, passcode
LinuxCommissionableDataProvider gCommissionableDataProvider;

int main(int argc, char * argv[])
{
#if defined(ENABLE_CHIP_SHELL)
    Engine::Root().Init();
    std::thread shellThread([]() { Engine::Root().RunMainLoop(); });
    Shell::RegisterCastingCommands();
#endif

    CHIP_ERROR err = CHIP_NO_ERROR;

    SuccessOrExit(err = chip::Platform::MemoryInit());
    SuccessOrExit(err = chip::DeviceLayer::PlatformMgr().InitChipStack());

    // Init the commissionable data provider based on command line options
    // to handle custom verifiers, discriminators, etc.
    err = InitCommissionableDataProvider(gCommissionableDataProvider, LinuxDeviceOptions::GetInstance());
    SuccessOrExit(err);
    DeviceLayer::SetCommissionableDataProvider(&gCommissionableDataProvider);

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

    // Initialize device attestation verifier from a constant version
    {
        // TODO: Replace testingRootStore with a AttestationTrustStore that has the necessary official PAA roots available
        const chip::Credentials::AttestationTrustStore * testingRootStore = chip::Credentials::GetTestAttestationTrustStore();
        SetDeviceAttestationVerifier(GetDefaultDACVerifier(testingRootStore));
    }

    if (!chip::ArgParser::ParseArgs(argv[0], argc, argv, allOptions))
    {
        return 1;
    }

    // Send discover commissioners request
    SuccessOrExit(err = CastingServer::GetInstance()->DiscoverCommissioners());

    // Give commissioners some time to respond and then ScheduleWork to initiate commissioning
    DeviceLayer::SystemLayer().StartTimer(
        chip::System::Clock::Milliseconds32(kCommissionerDiscoveryTimeoutInMs),
        [](System::Layer *, void *) { chip::DeviceLayer::PlatformMgr().ScheduleWork(InitCommissioningFlow); }, nullptr);

    // Add callback to send Content casting commands after commissioning completes
    chip::DeviceLayer::PlatformMgrImpl().AddEventHandler(DeviceEventCallback, 0);

    DeviceLayer::PlatformMgr().RunEventLoop();
exit:
#if defined(ENABLE_CHIP_SHELL)
    shellThread.join();
#endif
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to run TV Casting App: %s", ErrorStr(err));
        // End the program with non zero error code to indicate an error.
        return 1;
    }
    return 0;
}
