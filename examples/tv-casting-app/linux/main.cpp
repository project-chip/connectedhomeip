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

#include <app/OperationalDeviceProxy.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <controller/CHIPCommissionableNodeController.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/DeviceAttestationVerifier.h>
#include <credentials/examples/DefaultDeviceAttestationVerifier.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/SafeInt.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConfigurationManager.h>
#include <platform/DeviceControlServer.h>
#include <system/SystemLayer.h>
#include <transport/raw/PeerAddress.h>
#include <zap-generated/CHIPClusters.h>

#include <list>
#include <string>

using namespace chip;
using namespace chip::Controller;
using namespace chip::Credentials;
using chip::ArgParser::HelpOptions;
using chip::ArgParser::OptionDef;
using chip::ArgParser::OptionSet;
using namespace chip::app::Clusters::ContentLauncher::Commands;

struct TVExampleDeviceType
{
    const char * name;
    uint16_t id;
};

constexpr TVExampleDeviceType kKnownDeviceTypes[]    = { { "video-player", 35 }, { "dimmable-light", 257 } };
constexpr int kKnownDeviceTypesCount                 = sizeof kKnownDeviceTypes / sizeof *kKnownDeviceTypes;
constexpr uint16_t kOptionDeviceType                 = 't';
constexpr uint16_t kCommissioningWindowTimeoutInSec  = 3 * 60;
constexpr uint32_t kCommissionerDiscoveryTimeoutInMs = 5 * 1000;

// TODO: Accept these values over CLI
const char * kContentUrl         = "https://www.test.com/videoid";
const char * kContentDisplayStr  = "Test video";
constexpr EndpointId kTvEndpoint = 1;

CommissionableNodeController gCommissionableNodeController;
chip::System::SocketWatchToken gToken;
Dnssd::DiscoveryFilter gDiscoveryFilter = Dnssd::DiscoveryFilter();

CASEClientPool<CHIP_CONFIG_DEVICE_MAX_ACTIVE_CASE_CLIENTS> gCASEClientPool;

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
        else
        {
            for (int i = 0; i < kKnownDeviceTypesCount; i++)
            {
                if (strcasecmp(aValue, kKnownDeviceTypes[i].name) == 0)
                {
                    gDiscoveryFilter = Dnssd::DiscoveryFilter(Dnssd::DiscoveryFilterType::kDeviceType, kKnownDeviceTypes[i].id);
                    return true;
                }
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

/**
 * Enters commissioning mode, opens commissioning window, logs onboarding payload.
 * If non-null selectedCommissioner is provided, sends user directed commissioning
 * request to the selectedCommissioner and advertises self as commissionable node over DNS-SD
 */
void PrepareForCommissioning(const Dnssd::DiscoveredNodeData * selectedCommissioner = nullptr)
{
    // Enter commissioning mode, open commissioning window
    Server::GetInstance().Init();
    Server::GetInstance().GetFabricTable().DeleteAllFabrics();
    ReturnOnFailure(
        Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow(kCommissioningWindowTimeoutInSec));

    // Display onboarding payload
    chip::DeviceLayer::ConfigurationMgr().LogDeviceConfig();

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    if (selectedCommissioner != nullptr)
    {
        // Advertise self as Commissionable Node over mDNS
        app::DnssdServer::Instance().StartServer(Dnssd::CommissioningMode::kEnabledBasic);

        // Send User Directed commissioning request
        ReturnOnFailure(Server::GetInstance().SendUserDirectedCommissioningRequest(chip::Transport::PeerAddress::UDP(
            selectedCommissioner->ipAddress[0], selectedCommissioner->port, selectedCommissioner->interfaceId[0])));
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
}

/**
 * Accepts user input of selected commissioner and calls PrepareForCommissioning with
 * the selected commissioner
 */
void RequestUserDirectedCommissioning(System::SocketEvents events, intptr_t data)
{
    // Accept user selection for commissioner to request commissioning from.
    // Assuming kernel has line buffering, this will unblock on '\n' character
    // on stdin i.e. when user hits 'Enter'
    int selectedCommissionerNumber = CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES;
    scanf("%d", &selectedCommissionerNumber);
    printf("%d\n", selectedCommissionerNumber);
    chip::DeviceLayer::SystemLayerSockets().StopWatchingSocket(&gToken);

    const Dnssd::DiscoveredNodeData * selectedCommissioner =
        gCommissionableNodeController.GetDiscoveredCommissioner(selectedCommissionerNumber - 1);
    VerifyOrReturn(selectedCommissioner != nullptr, ChipLogError(AppServer, "No such commissioner!"));
    PrepareForCommissioning(selectedCommissioner);
}

void InitCommissioningFlow(intptr_t commandArg)
{
    int commissionerCount = 0;

    // Display discovered commissioner TVs to ask user to select one
    for (int i = 0; i < CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES; i++)
    {
        const Dnssd::DiscoveredNodeData * commissioner = gCommissionableNodeController.GetDiscoveredCommissioner(i);
        if (commissioner != nullptr)
        {
            ChipLogProgress(AppServer, "Discovered Commissioner #%d", ++commissionerCount);
            commissioner->LogDetail();
        }
    }

    if (commissionerCount > 0)
    {
        ChipLogProgress(AppServer, "%d commissioner(s) discovered. Select one (by number# above) to request commissioning from: ",
                        commissionerCount);

        // Setup for async/non-blocking user input from stdin
        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        VerifyOrReturn(fcntl(0, F_SETFL, flags | O_NONBLOCK) == 0,
                       ChipLogError(AppServer, "Could not set non-blocking mode for user input!"));
        ReturnOnFailure(chip::DeviceLayer::SystemLayerSockets().StartWatchingSocket(STDIN_FILENO, &gToken));
        ReturnOnFailure(
            chip::DeviceLayer::SystemLayerSockets().SetCallback(gToken, RequestUserDirectedCommissioning, (intptr_t) NULL));
        ReturnOnFailure(chip::DeviceLayer::SystemLayerSockets().RequestCallbackOnPendingRead(gToken));
    }
    else
    {
        ChipLogError(AppServer, "No commissioner discovered, commissioning must be initiated manually!");
        PrepareForCommissioning();
    }
}

void OnContentLauncherSuccessResponse(void * context, const LaunchResponse::DecodableType & response)
{
    ChipLogProgress(AppServer, "ContentLauncher: Default Success Response");
}

void OnContentLauncherFailureResponse(void * context, CHIP_ERROR error)
{
    ChipLogError(AppServer, "ContentLauncher: Default Failure Response: %" CHIP_ERROR_FORMAT, error.Format());
}

void DeviceEventCallback(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    if (event->Type == DeviceLayer::DeviceEventType::kCommissioningComplete)
    {
        chip::NodeId tvNodeId             = chip::DeviceLayer::DeviceControlServer::DeviceControlSvr().GetPeerNodeId();
        chip::FabricIndex peerFabricIndex = chip::DeviceLayer::DeviceControlServer::DeviceControlSvr().GetFabricIndex();

        Server * server           = &(chip::Server::GetInstance());
        chip::FabricInfo * fabric = server->GetFabricTable().FindFabricWithIndex(peerFabricIndex);
        if (fabric == nullptr)
        {
            ChipLogError(AppServer, "Did not find fabric for index %d", peerFabricIndex);
            return;
        }

        chip::DeviceProxyInitParams initParams = {
            .sessionManager = &(server->GetSecureSessionManager()),
            .exchangeMgr    = &(server->GetExchangeManager()),
            .idAllocator    = &(server->GetSessionIDAllocator()),
            .fabricTable    = &(server->GetFabricTable()),
            .clientPool     = &gCASEClientPool,
        };

        PeerId peerID = fabric->GetPeerIdForNode(tvNodeId);
        chip::OperationalDeviceProxy * operationalDeviceProxy =
            chip::Platform::New<chip::OperationalDeviceProxy>(initParams, peerID);
        if (operationalDeviceProxy == nullptr)
        {
            ChipLogError(AppServer, "Failed in creating an instance of OperationalDeviceProxy");
            return;
        }

        SessionHandle handle = server->GetSecureSessionManager().FindSecureSessionForNode(tvNodeId);
        operationalDeviceProxy->SetConnectedSession(handle);

        ContentLauncherCluster cluster;
        CHIP_ERROR err = cluster.Associate(operationalDeviceProxy, kTvEndpoint);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "Associate() failed: %" CHIP_ERROR_FORMAT, err.Format());
            return;
        }
        LaunchURLRequest::Type request;
        request.contentURL          = chip::CharSpan::fromCharString(kContentUrl);
        request.displayString       = Optional<CharSpan>(chip::CharSpan::fromCharString(kContentDisplayStr));
        request.brandingInformation = Optional<chip::app::Clusters::ContentLauncher::Structs::BrandingInformation::Type>(
            chip::app::Clusters::ContentLauncher::Structs::BrandingInformation::Type());
        cluster.InvokeCommand(request, nullptr, OnContentLauncherSuccessResponse, OnContentLauncherFailureResponse);
    }
}

int main(int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    SuccessOrExit(err = chip::Platform::MemoryInit());
    SuccessOrExit(err = chip::DeviceLayer::PlatformMgr().InitChipStack());

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
    SuccessOrExit(err = gCommissionableNodeController.DiscoverCommissioners(gDiscoveryFilter));

    // Give commissioners some time to respond and then ScheduleWork to initiate commissioning
    DeviceLayer::SystemLayer().StartTimer(
        chip::System::Clock::Milliseconds32(kCommissionerDiscoveryTimeoutInMs),
        [](System::Layer *, void *) { chip::DeviceLayer::PlatformMgr().ScheduleWork(InitCommissioningFlow); }, nullptr);

    // Add callback to send Content casting commands after commissioning completes
    chip::DeviceLayer::PlatformMgrImpl().AddEventHandler(DeviceEventCallback, 0);

    DeviceLayer::PlatformMgr().RunEventLoop();
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to run TV Casting App: %s", ErrorStr(err));
        // End the program with non zero error code to indicate an error.
        return 1;
    }
    return 0;
}
