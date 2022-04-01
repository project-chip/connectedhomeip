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

#include "LinuxCommissionableDataProvider.h"
#include "Options.h"
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

constexpr TVExampleDeviceType kKnownDeviceTypes[]              = { { "video-player", 35 }, { "dimmable-light", 257 } };
constexpr int kKnownDeviceTypesCount                           = sizeof kKnownDeviceTypes / sizeof *kKnownDeviceTypes;
constexpr uint16_t kOptionDeviceType                           = 't';
constexpr System::Clock::Seconds16 kCommissioningWindowTimeout = System::Clock::Seconds16(3 * 60);
constexpr uint32_t kCommissionerDiscoveryTimeoutInMs           = 5 * 1000;

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

void ReadServerClusters(EndpointId endpointId);
CHIP_ERROR InitBindingHandlers()
{
    auto & server = chip::Server::GetInstance();
    chip::BindingManager::GetInstance().Init(
        { &server.GetFabricTable(), server.GetCASESessionManager(), &server.GetPersistentStorage() });
    return CHIP_NO_ERROR;
}

/**
 * Enters commissioning mode, opens commissioning window, logs onboarding payload.
 * If non-null selectedCommissioner is provided, sends user directed commissioning
 * request to the selectedCommissioner and advertises self as commissionable node over DNS-SD
 */
void PrepareForCommissioning(const Dnssd::DiscoveredNodeData * selectedCommissioner = nullptr)
{
    // DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init("/tmp/chip_tv_casting_kvs");
    DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init(CHIP_CONFIG_KVS_PATH);

    // Enter commissioning mode, open commissioning window
    Server::GetInstance().Init();
    Server::GetInstance().GetFabricTable().DeleteAllFabrics();
    ReturnOnFailure(
        Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow(kCommissioningWindowTimeout));

    // Display onboarding payload
    chip::DeviceLayer::ConfigurationMgr().LogDeviceConfig();

    // Initialize binding handlers
    ReturnOnFailure(InitBindingHandlers());

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    if (selectedCommissioner != nullptr)
    {
        // Send User Directed commissioning request
        ReturnOnFailure(Server::GetInstance().SendUserDirectedCommissioningRequest(chip::Transport::PeerAddress::UDP(
            selectedCommissioner->ipAddress[0], selectedCommissioner->port, selectedCommissioner->interfaceId)));
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

class TargetEndpointInfo
{
public:
    void Initialize(EndpointId endpointId)
    {
        mEndpointId = endpointId;
        for (size_t i = 0; i < kMaxNumberOfClustersPerEndpoint; i++)
        {
            mClusters[i] = kInvalidClusterId;
        }
        mInitialized = true;
    }

    void Reset() { mInitialized = false; }

    bool HasCluster(ClusterId clusterId)
    {
        for (size_t i = 0; i < kMaxNumberOfClustersPerEndpoint; i++)
        {
            if (mClusters[i] == clusterId)
            {
                return true;
            }
        }
        return false;
    }

    bool AddCluster(ClusterId clusterId)
    {
        for (size_t i = 0; i < kMaxNumberOfClustersPerEndpoint; i++)
        {
            if (mClusters[i] == clusterId)
            {
                return true;
            }
            if (mClusters[i] == kInvalidClusterId)
            {
                mClusters[i] = clusterId;
                return true;
            }
        }
        return false;
    }

    bool IsInitialized() { return mInitialized; }

    EndpointId GetEndpointId() const { return mEndpointId; }

    void PrintInfo()
    {
        ChipLogProgress(NotSpecified, "   endpoint=%d", mEndpointId);
        for (size_t i = 0; i < kMaxNumberOfClustersPerEndpoint; i++)
        {
            if (mClusters[i] != kInvalidClusterId)
            {

                ChipLogProgress(NotSpecified, "      cluster=" ChipLogFormatMEI, ChipLogValueMEI(mClusters[i]));
            }
        }
    }

private:
    static constexpr size_t kMaxNumberOfClustersPerEndpoint = 10;
    ClusterId mClusters[kMaxNumberOfClustersPerEndpoint]    = {};
    EndpointId mEndpointId;
    bool mInitialized = false;
};

class TargetVideoPlayerInfo
{
public:
    bool IsInitialized() { return mInitialized; }

    CHIP_ERROR Initialize(NodeId nodeId, FabricIndex fabricIndex)
    {
        mNodeId      = nodeId;
        mFabricIndex = fabricIndex;
        for (auto & endpointInfo : mEndpoints)
        {
            endpointInfo.Reset();
        }

        Server * server           = &(chip::Server::GetInstance());
        chip::FabricInfo * fabric = server->GetFabricTable().FindFabricWithIndex(fabricIndex);
        if (fabric == nullptr)
        {
            ChipLogError(AppServer, "Did not find fabric for index %d", fabricIndex);
            return CHIP_ERROR_INVALID_FABRIC_ID;
        }

        chip::DeviceProxyInitParams initParams = {
            .sessionManager = &(server->GetSecureSessionManager()),
            .exchangeMgr    = &(server->GetExchangeManager()),
            .fabricTable    = &(server->GetFabricTable()),
            .clientPool     = &gCASEClientPool,
        };

        PeerId peerID           = fabric->GetPeerIdForNode(nodeId);
        mOperationalDeviceProxy = chip::Platform::New<chip::OperationalDeviceProxy>(initParams, peerID);

        // TODO: figure out why this doesn't work so that we can remove OperationalDeviceProxy creation above,
        // and remove the FindSecureSessionForNode and SetConnectedSession calls below
        // mOperationalDeviceProxy = server->GetCASESessionManager()->FindExistingSession(nodeId);
        if (mOperationalDeviceProxy == nullptr)
        {
            ChipLogError(AppServer, "Failed in creating an instance of OperationalDeviceProxy");
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        ChipLogError(AppServer, "Created an instance of OperationalDeviceProxy");

        SessionHandle handle = server->GetSecureSessionManager().FindSecureSessionForNode(nodeId);
        mOperationalDeviceProxy->SetConnectedSession(handle);

        mInitialized = true;
        return CHIP_NO_ERROR;
    }

    NodeId GetNodeId() const { return mNodeId; }
    FabricIndex GetFabricIndex() const { return mFabricIndex; }
    OperationalDeviceProxy * GetOperationalDeviceProxy() const { return mOperationalDeviceProxy; }

    TargetEndpointInfo * GetOrAddEndpoint(EndpointId endpointId)
    {
        if (!mInitialized)
        {
            return nullptr;
        }
        TargetEndpointInfo * endpoint = GetEndpoint(endpointId);
        if (endpoint != nullptr)
        {
            return endpoint;
        }
        for (auto & endpointInfo : mEndpoints)
        {
            if (!endpointInfo.IsInitialized())
            {
                endpointInfo.Initialize(endpointId);
                return &endpointInfo;
            }
        }
        return nullptr;
    }

    TargetEndpointInfo * GetEndpoint(EndpointId endpointId)
    {
        if (!mInitialized)
        {
            return nullptr;
        }
        for (auto & endpointInfo : mEndpoints)
        {
            if (endpointInfo.IsInitialized() && endpointInfo.GetEndpointId() == endpointId)
            {
                return &endpointInfo;
            }
        }
        return nullptr;
    }

    bool HasEndpoint(EndpointId endpointId)
    {
        if (!mInitialized)
        {
            return false;
        }
        for (auto & endpointInfo : mEndpoints)
        {
            if (endpointInfo.IsInitialized() && endpointInfo.GetEndpointId() == endpointId)
            {
                return true;
            }
        }
        return false;
    }

    void PrintInfo()
    {
        ChipLogProgress(NotSpecified, " TargetVideoPlayerInfo nodeId=0x" ChipLogFormatX64 " fabric index=%d",
                        ChipLogValueX64(mNodeId), mFabricIndex);
        for (auto & endpointInfo : mEndpoints)
        {
            if (endpointInfo.IsInitialized())
            {
                endpointInfo.PrintInfo();
            }
        }
    }

private:
    static constexpr size_t kMaxNumberOfEndpoints = 5;
    TargetEndpointInfo mEndpoints[kMaxNumberOfEndpoints];
    NodeId mNodeId;
    FabricIndex mFabricIndex;
    OperationalDeviceProxy * mOperationalDeviceProxy;

    bool mInitialized = false;
};
TargetVideoPlayerInfo gTargetVideoPlayerInfo;

void OnDescriptorReadSuccessResponse(void * context, const app::DataModel::DecodableList<ClusterId> & responseList)
{
    TargetEndpointInfo * endpointInfo = static_cast<TargetEndpointInfo *>(context);

    ChipLogProgress(AppServer, "Descriptor: Default Success Response endpoint=%d", endpointInfo->GetEndpointId());

    auto iter = responseList.begin();
    while (iter.Next())
    {
        auto & clusterId = iter.GetValue();
        endpointInfo->AddCluster(clusterId);
    }
    // Always print the target info after handling descriptor read response
    // Even when we get nothing back for any reasons
    gTargetVideoPlayerInfo.PrintInfo();
}

void OnDescriptorReadFailureResponse(void * context, CHIP_ERROR error)
{
    ChipLogError(AppServer, "Descriptor: Default Failure Response: %" CHIP_ERROR_FORMAT, error.Format());
}

void ReadServerClusters(EndpointId endpointId)
{
    OperationalDeviceProxy * operationalDeviceProxy = gTargetVideoPlayerInfo.GetOperationalDeviceProxy();
    if (operationalDeviceProxy == nullptr)
    {
        ChipLogError(AppServer, "Failed in getting an instance of OperationalDeviceProxy");
        return;
    }

    chip::Controller::DescriptorCluster cluster;
    CHIP_ERROR err = cluster.Associate(operationalDeviceProxy, endpointId);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Associate() failed: %" CHIP_ERROR_FORMAT, err.Format());
        return;
    }

    TargetEndpointInfo * endpointInfo = gTargetVideoPlayerInfo.GetOrAddEndpoint(endpointId);

    if (cluster.ReadAttribute<app::Clusters::Descriptor::Attributes::ServerList::TypeInfo>(
            endpointInfo, OnDescriptorReadSuccessResponse, OnDescriptorReadFailureResponse) != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Could not read Descriptor cluster ServerList");
    }

    ChipLogProgress(Controller, "Sent descriptor read for remote endpoint=%d", endpointId);
}

void ReadServerClustersForNode(NodeId nodeId)
{
    for (const auto & binding : BindingTable::GetInstance())
    {
        ChipLogProgress(NotSpecified,
                        "Binding type=%d fab=%d nodeId=0x" ChipLogFormatX64
                        " groupId=%d local endpoint=%d remote endpoint=%d cluster=" ChipLogFormatMEI,
                        binding.type, binding.fabricIndex, ChipLogValueX64(binding.nodeId), binding.groupId, binding.local,
                        binding.remote, ChipLogValueMEI(binding.clusterId.ValueOr(0)));
        if (binding.type == EMBER_UNICAST_BINDING && nodeId == binding.nodeId)
        {
            if (!gTargetVideoPlayerInfo.HasEndpoint(binding.remote))
            {
                ReadServerClusters(binding.remote);
            }
        }
    }
}

void DeviceEventCallback(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    if (event->Type == DeviceLayer::DeviceEventType::kBindingsChangedViaCluster)
    {
        if (gTargetVideoPlayerInfo.IsInitialized())
        {
            ReadServerClustersForNode(gTargetVideoPlayerInfo.GetNodeId());
        }
    }
    else if (event->Type == DeviceLayer::DeviceEventType::kCommissioningComplete)
    {
        ReturnOnFailure(gTargetVideoPlayerInfo.Initialize(event->CommissioningComplete.PeerNodeId,
                                                          event->CommissioningComplete.PeerFabricIndex));

        OperationalDeviceProxy * operationalDeviceProxy = gTargetVideoPlayerInfo.GetOperationalDeviceProxy();
        if (operationalDeviceProxy == nullptr)
        {
            ChipLogError(AppServer, "Failed in getting an instance of OperationalDeviceProxy");
            return;
        }

        ContentLauncherCluster cluster;
        CHIP_ERROR err = cluster.Associate(operationalDeviceProxy, kTvEndpoint);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "Associate() failed: %" CHIP_ERROR_FORMAT, err.Format());
            return;
        }
        LaunchURL::Type request;
        request.contentURL          = chip::CharSpan::fromCharString(kContentUrl);
        request.displayString       = Optional<CharSpan>(chip::CharSpan::fromCharString(kContentDisplayStr));
        request.brandingInformation = Optional<chip::app::Clusters::ContentLauncher::Structs::BrandingInformation::Type>(
            chip::app::Clusters::ContentLauncher::Structs::BrandingInformation::Type());
        cluster.InvokeCommand(request, nullptr, OnContentLauncherSuccessResponse, OnContentLauncherFailureResponse);
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
