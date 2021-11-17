/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/CASESessionManager.h>
#include <app/server/Server.h>
#include <app/util/util.h>
#include <controller/CHIPDeviceControllerFactory.h>
#include <controller/CommissioneeDeviceProxy.h>
#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CHIPArgParser.hpp>
#include <platform/CHIPDeviceLayer.h>
#include <zap-generated/CHIPClientCallbacks.h>
#include <zap-generated/CHIPClusters.h>

#include "BDXDownloader.h"
#include "ExampleOTARequestor.h"

using chip::ByteSpan;
using chip::CASESessionManager;
using chip::CASESessionManagerConfig;
using chip::CharSpan;
using chip::DeviceProxy;
using chip::EndpointId;
using chip::FabricIndex;
using chip::FabricInfo;
using chip::NodeId;
using chip::OnDeviceConnected;
using chip::OnDeviceConnectionFailure;
using chip::PeerId;
using chip::Server;
using chip::VendorId;
using chip::bdx::TransferSession;
using chip::Callback::Callback;
using chip::System::Layer;
using chip::Transport::PeerAddress;
using namespace chip::ArgParser;
using namespace chip::Messaging;
using namespace chip::app::Clusters::OtaSoftwareUpdateProvider::Commands;

// Various cases for when OnConnected callback could be called
enum OnConnectedState
{
    kQueryImage = 0,
    kStartBDX,
};

void OnQueryImageResponse(void * context, const QueryImageResponse::DecodableType & response);
void OnQueryImageFailure(void * context, EmberAfStatus status);
void OnConnected(void * context, chip::DeviceProxy * deviceProxy);
void OnConnectionFailure(void * context, NodeId deviceId, CHIP_ERROR error);
bool HandleOptions(const char * aProgram, OptionSet * aOptions, int aIdentifier, const char * aName, const char * aValue);

// TODO: would be nicer to encapsulate these globals and the callbacks in some sort of class
CASESessionManager * mCASESessionManager = nullptr;
OnConnectedState onConnectedState        = kQueryImage;
ExchangeContext * exchangeCtx            = nullptr;
BdxDownloader bdxDownloader;
Callback<OnDeviceConnected> mOnConnectedCallback(OnConnected, nullptr);
Callback<OnDeviceConnectionFailure> mOnConnectionFailureCallback(OnConnectionFailure, nullptr);

constexpr uint8_t kNodeIdHexStringMaxLen = 16;

constexpr uint16_t kOptionProviderNodeId      = 'n';
constexpr uint16_t kOptionProviderFabricIndex = 'f';
constexpr uint16_t kOptionUdpPort             = 'u';
constexpr uint16_t kOptionDiscriminator       = 'd';
constexpr uint16_t kOptionDelayQuery          = 'q';

NodeId providerNodeId           = 0x0;
FabricIndex providerFabricIndex = 1;
uint16_t requestorSecurePort    = 0;
uint16_t setupDiscriminator     = CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR;
uint16_t delayQueryTimeInSec    = 0;

OptionDef cmdLineOptionsDef[] = {
    { "providerNodeId", chip::ArgParser::kArgumentRequired, kOptionProviderNodeId },
    { "providerFabricIndex", chip::ArgParser::kArgumentRequired, kOptionProviderFabricIndex },
    { "udpPort", chip::ArgParser::kArgumentRequired, kOptionUdpPort },
    { "discriminator", chip::ArgParser::kArgumentRequired, kOptionDiscriminator },
    { "delayQuery", chip::ArgParser::kArgumentRequired, kOptionDelayQuery },
    {},
};

OptionSet cmdLineOptions = { HandleOptions, cmdLineOptionsDef, "PROGRAM OPTIONS",
                             "  -n/--providerNodeId <node ID>\n"
                             "        Node ID of the OTA Provider to connect to (hex format)\n\n"
                             "        This assumes that you've already commissioned the OTA Provider node with chip-tool.\n"
                             "  -f/--providerFabricIndex <fabric index>\n"
                             "        Fabric index of the OTA Provider to connect to. If none is specified, default value is 1.\n\n"
                             "        This assumes that you've already commissioned the OTA Provider node with chip-tool.\n"
                             "  -u/--udpPort <UDP port number>\n"
                             "        UDP Port that the OTA Requestor listens on for secure connections.\n"
                             "  -d/--discriminator <discriminator>\n"
                             "        A 12-bit value used to discern between multiple commissionable CHIP device\n"
                             "        advertisements. If none is specified, default value is 3840.\n"
                             "  -q/--delayQuery <Time in seconds>\n"
                             "        From boot up, the amount of time to wait before triggering the QueryImage\n"
                             "        command. If none or zero is supplied, QueryImage will not be triggered.\n" };

HelpOptions helpOptions("ota-requestor-app", "Usage: ota-requestor-app [options]", "1.0");

OptionSet * allOptions[] = { &cmdLineOptions, &helpOptions, nullptr };

void OnQueryImageResponse(void * context, const QueryImageResponse::DecodableType & response)
{
    ChipLogDetail(SoftwareUpdate, "QueryImageResponse responded with action %" PRIu8, response.status);
    ChipLogDetail(SoftwareUpdate, "  status: %" PRIu8 "", response.status);
    if (response.delayedActionTime.HasValue())
    {
        ChipLogDetail(SoftwareUpdate, "  delayedActionTime: %" PRIu32 "", response.delayedActionTime.Value());
    }
    if (response.imageURI.HasValue())
    {
        ChipLogDetail(SoftwareUpdate, "  imageURI: %.*s", static_cast<int>(response.imageURI.Value().size()),
                      response.imageURI.Value().data());
    }
    if (response.softwareVersion.HasValue())
    {
        ChipLogDetail(SoftwareUpdate, "  softwareVersion: %" PRIu32 "", response.softwareVersion.Value());
    }
    if (response.softwareVersionString.HasValue())
    {
        ChipLogDetail(SoftwareUpdate, "  softwareVersionString: %.*s",
                      static_cast<int>(response.softwareVersionString.Value().size()),
                      response.softwareVersionString.Value().data());
    }
    if (response.updateToken.HasValue())
    {
        ChipLogDetail(SoftwareUpdate, "  updateToken: %zu", response.updateToken.Value().size());
    }
    if (response.userConsentNeeded.HasValue())
    {
        ChipLogDetail(SoftwareUpdate, "  userConsentNeeded: %d", response.userConsentNeeded.Value());
    }
    if (response.metadataForRequestor.HasValue())
    {
        ChipLogDetail(SoftwareUpdate, "  metadataForRequestor: %zu", response.metadataForRequestor.Value().size());
    }

    // Parse out the provider node ID from the ImageURI
    NodeId nodeId = chip::kUndefinedNodeId;
    if (response.imageURI.HasValue())
    {
        CharSpan nodeIdString = response.imageURI.Value().SubSpan(6, kNodeIdHexStringMaxLen);

        uint8_t buffer[kNodeIdHexStringMaxLen];
        if (chip::Encoding::HexToBytes(nodeIdString.data(), nodeIdString.size(), buffer, kNodeIdHexStringMaxLen) == 0)
        {
            ChipLogError(SoftwareUpdate, "Cannot convert provider Node ID: %.*s", static_cast<int>(nodeIdString.size()),
                         nodeIdString.data());
            return;
        }

        nodeId = chip::Encoding::BigEndian::Get64(buffer);
    }

    // Use the same CSM from sending of QueryImage based on the assumption that the provider node ID that will supply the OTA image
    // is on the same fabric as the sender of the QueryImageResponse
    if (mCASESessionManager != nullptr)
    {
        // Establish a CASE session with the parsed node ID to obtain exchange context and session handle for BDX transfer
        onConnectedState = kStartBDX;
        CHIP_ERROR err = mCASESessionManager->FindOrEstablishSession(nodeId, &mOnConnectedCallback, &mOnConnectionFailureCallback);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(SoftwareUpdate, "Cannot establish connection to peer device: %" CHIP_ERROR_FORMAT, err.Format());
        }
    }
}

void OnQueryImageFailure(void * context, EmberAfStatus status)
{
    ChipLogDetail(SoftwareUpdate, "QueryImage failure response %" PRIu8, status);
}

// Called whenever FindOrEstablishSession is successful
void OnConnected(void * context, chip::DeviceProxy * deviceProxy)
{
    switch (onConnectedState)
    {
    case kQueryImage: {
        CHIP_ERROR err = CHIP_NO_ERROR;
        chip::Controller::OtaSoftwareUpdateProviderCluster cluster;
        constexpr EndpointId kOtaProviderEndpoint = 0;

        // These QueryImage params have been chosen arbitrarily
        constexpr VendorId kExampleVendorId                               = VendorId::Common;
        constexpr uint16_t kExampleProductId                              = 77;
        constexpr uint16_t kExampleHWVersion                              = 3;
        constexpr uint16_t kExampleSoftwareVersion                        = 0;
        constexpr EmberAfOTADownloadProtocol kExampleProtocolsSupported[] = { EMBER_ZCL_OTA_DOWNLOAD_PROTOCOL_BDX_SYNCHRONOUS };
        const char locationBuf[]                                          = { 'U', 'S' };
        CharSpan exampleLocation(locationBuf);
        constexpr bool kExampleClientCanConsent = false;
        ByteSpan metadata;

        err = cluster.Associate(deviceProxy, kOtaProviderEndpoint);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(SoftwareUpdate, "Associate() failed: %" CHIP_ERROR_FORMAT, err.Format());
            return;
        }
        QueryImage::Type args;
        args.vendorId           = kExampleVendorId;
        args.productId          = kExampleProductId;
        args.softwareVersion    = kExampleSoftwareVersion;
        args.protocolsSupported = kExampleProtocolsSupported;
        args.hardwareVersion.Emplace(kExampleHWVersion);
        args.location.Emplace(exampleLocation);
        args.requestorCanConsent.Emplace(kExampleClientCanConsent);
        args.metadataForProvider.Emplace(metadata);
        err = cluster.InvokeCommand(args, /* context = */ nullptr, OnQueryImageResponse, OnQueryImageFailure);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(SoftwareUpdate, "QueryImage() failed: %" CHIP_ERROR_FORMAT, err.Format());
        }

        break;
    }
    case kStartBDX: {
        TransferSession::TransferInitData initOptions;
        initOptions.TransferCtlFlags = chip::bdx::TransferControlFlags::kReceiverDrive;
        initOptions.MaxBlockSize     = 1024;
        char testFileDes[9]          = { "test.txt" };
        initOptions.FileDesLength    = static_cast<uint16_t>(strlen(testFileDes));
        initOptions.FileDesignator   = reinterpret_cast<uint8_t *>(testFileDes);

        if (deviceProxy != nullptr)
        {
            chip::Messaging::ExchangeManager * exchangeMgr = deviceProxy->GetExchangeManager();
            chip::Optional<chip::SessionHandle> session    = deviceProxy->GetSecureSession();
            if (exchangeMgr != nullptr && session.HasValue())
            {
                exchangeCtx = exchangeMgr->NewContext(session.Value(), &bdxDownloader);
            }

            if (exchangeCtx == nullptr)
            {
                ChipLogError(BDX, "unable to allocate ec: exchangeMgr=%p sessionExists? %u", exchangeMgr, session.HasValue());
                return;
            }
        }

        bdxDownloader.SetInitialExchange(exchangeCtx);

        // This will kick of a timer which will regularly check for updates to the bdx::TransferSession state machine.
        bdxDownloader.InitiateTransfer(&chip::DeviceLayer::SystemLayer(), chip::bdx::TransferRole::kReceiver, initOptions,
                                       chip::System::Clock::Seconds16(20));
        break;
    }
    default:
        break;
    }
}

void OnConnectionFailure(void * context, NodeId deviceId, CHIP_ERROR error)
{
    ChipLogError(SoftwareUpdate, "failed to connect to 0x%" PRIX64 ": %" CHIP_ERROR_FORMAT, deviceId, error.Format());
}

bool HandleOptions(const char * aProgram, OptionSet * aOptions, int aIdentifier, const char * aName, const char * aValue)
{
    bool retval = true;

    switch (aIdentifier)
    {
    case kOptionProviderNodeId:
        if (1 != sscanf(aValue, "%" PRIX64, &providerNodeId))
        {
            PrintArgError("%s: unable to parse Node ID: %s\n", aProgram, aValue);
        }
        break;
    case kOptionProviderFabricIndex:
        providerFabricIndex = static_cast<uint8_t>(strtol(aValue, NULL, 0));

        if (kOptionProviderFabricIndex == 0)
        {
            PrintArgError("%s: Input ERROR: Fabric Index may not be zero\n", aProgram);
            retval = false;
        }
        break;
    case kOptionUdpPort:
        requestorSecurePort = static_cast<uint16_t>(strtol(aValue, NULL, 0));

        if (requestorSecurePort == 0)
        {
            PrintArgError("%s: Input ERROR: udpPort may not be zero\n", aProgram);
            retval = false;
        }
        break;
    case kOptionDiscriminator:
        setupDiscriminator = static_cast<uint16_t>(strtol(aValue, NULL, 0));

        if (setupDiscriminator > 0xFFF)
        {
            PrintArgError("%s: Input ERROR: setupDiscriminator value %s is out of range \n", aProgram, aValue);
            retval = false;
        }
        break;
    case kOptionDelayQuery:
        delayQueryTimeInSec = static_cast<uint16_t>(strtol(aValue, NULL, 0));
        break;
    default:
        PrintArgError("%s: INTERNAL ERROR: Unhandled option: %s\n", aProgram, aName);
        retval = false;
        break;
    }

    return (retval);
}

CHIP_ERROR SetupCASESessionManager(FabricIndex fabricIndex)
{
    // A previous CASE session had been established
    if (mCASESessionManager != nullptr)
    {
        if (mCASESessionManager->GetFabricInfo()->GetFabricIndex() != fabricIndex)
        {
            // CSM is per fabric so if fabric index does not match the previous session, CSM needs to be set up again
            chip::Platform::Delete(mCASESessionManager);
            mCASESessionManager = nullptr;
        }
        else
        {
            // Fabric index matches so use previous instance
            return CHIP_NO_ERROR;
        }
    }

    // CSM has not been setup so create a new instance of it
    if (mCASESessionManager == nullptr)
    {
        Server * server         = &(Server::GetInstance());
        FabricInfo * fabricInfo = server->GetFabricTable().FindFabricWithIndex(fabricIndex);
        if (fabricInfo == nullptr)
        {
            ChipLogError(SoftwareUpdate, "Did not find fabric for index %d", fabricIndex);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        chip::DeviceProxyInitParams initParams = {
            .sessionManager = &(server->GetSecureSessionManager()),
            .exchangeMgr    = &(server->GetExchangeManager()),
            .idAllocator    = &(server->GetSessionIDAllocator()),
            .fabricInfo     = fabricInfo,
            // TODO: Determine where this should be instantiated
            .imDelegate = chip::Platform::New<chip::Controller::DeviceControllerInteractionModelDelegate>(),
        };

        CASESessionManagerConfig sessionManagerConfig = {
            .sessionInitParams = initParams,
            .dnsCache          = nullptr,
        };

        mCASESessionManager = chip::Platform::New<CASESessionManager>(sessionManagerConfig);
    }

    if (mCASESessionManager == nullptr)
    {
        ChipLogError(SoftwareUpdate, "Failed in creating an instance of CASESessionManager");
        return CHIP_ERROR_NO_MEMORY;
    }

    return CHIP_NO_ERROR;
}

void SendQueryImageCommand(chip::NodeId peerNodeId = providerNodeId, chip::FabricIndex peerFabricIndex = providerFabricIndex)
{
    CHIP_ERROR err = SetupCASESessionManager(peerFabricIndex);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Cannot send QueryImage command without CASESessionManager: %" CHIP_ERROR_FORMAT,
                     err.Format());
    }

    onConnectedState = kQueryImage;
    err = mCASESessionManager->FindOrEstablishSession(peerNodeId, &mOnConnectedCallback, &mOnConnectionFailureCallback);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Cannot establish connection to peer device: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void OnStartDelayTimerHandler(Layer * systemLayer, void * appState)
{
    SendQueryImageCommand();
}

int main(int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (chip::Platform::MemoryInit() != CHIP_NO_ERROR)
    {

        ChipLogError(SoftwareUpdate, "FAILED to initialize memory");
        return 1;
    }

    if (chip::DeviceLayer::PlatformMgr().InitChipStack() != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "FAILED to initialize chip stack");
        return 1;
    }

    if (!chip::ArgParser::ParseArgs(argv[0], argc, argv, allOptions))
    {
        return 1;
    }

    chip::DeviceLayer::ConfigurationMgr().LogDeviceConfig();

    // Set discriminator to user specified value
    ChipLogProgress(SoftwareUpdate, "Setting discriminator to: %d", setupDiscriminator);
    err = chip::DeviceLayer::ConfigurationMgr().StoreSetupDiscriminator(setupDiscriminator);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Setup discriminator setting failed with code: %" CHIP_ERROR_FORMAT, err.Format());
        return 1;
    }

    // Init Data Model and CHIP App Server with user specified UDP port
    Server::GetInstance().Init(nullptr, requestorSecurePort);
    ChipLogProgress(SoftwareUpdate, "Initializing the Application Server. Listening on UDP port %d", requestorSecurePort);

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(chip::Credentials::Examples::GetExampleDACProvider());

    // This will allow ExampleOTARequestor to call SendQueryImageCommand
    ExampleOTARequestor::GetInstance().SetConnectToProviderCallback(SendQueryImageCommand);

    // If a delay is provided, QueryImage after the timer expires
    if (delayQueryTimeInSec > 0)
    {
        chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(delayQueryTimeInSec * 1000),
                                                    OnStartDelayTimerHandler, nullptr);
    }

    chip::DeviceLayer::PlatformMgr().RunEventLoop();

    return 0;
}
