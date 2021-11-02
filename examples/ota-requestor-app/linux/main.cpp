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
#include <app/device/OperationalDeviceProxy.h>
#include <app/server/Server.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/support/CHIPArgParser.hpp>
#include <platform/CHIPDeviceLayer.h>
#include <zap-generated/CHIPClientCallbacks.h>
#include <zap-generated/CHIPClusters.h>

#include "BDXDownloader.h"
#include "ExampleOTARequestor.h"

using chip::ByteSpan;
using chip::CharSpan;
using chip::EndpointId;
using chip::FabricIndex;
using chip::NodeId;
using chip::Server;
using chip::VendorId;
using chip::bdx::TransferSession;
using chip::Callback::Callback;
using chip::Inet::IPAddress;
using chip::System::Layer;
using chip::Transport::PeerAddress;
using namespace chip::ArgParser;
using namespace chip::Messaging;
using namespace chip::app::device;

void OnQueryImageResponse(void * context, uint8_t status, uint32_t delayedActionTime, CharSpan imageURI, uint32_t softwareVersion,
                          CharSpan softwareVersionString, ByteSpan updateToken, bool userConsentNeeded,
                          ByteSpan metadataForRequestor);
void OnQueryImageFailure(void * context, uint8_t status);
void OnConnected(void * context, OperationalDeviceProxy * operationalDeviceProxy);
void OnConnectionFailure(void * context, OperationalDeviceProxy * operationalDeviceProxy, CHIP_ERROR error);
bool HandleOptions(const char * aProgram, OptionSet * aOptions, int aIdentifier, const char * aName, const char * aValue);

// TODO: would be nicer to encapsulate these globals and the callbacks in some sort of class
OperationalDeviceProxy gOperationalDeviceProxy;
ExchangeContext * exchangeCtx = nullptr;
BdxDownloader bdxDownloader;
Callback<OtaSoftwareUpdateProviderClusterQueryImageResponseCallback> mQueryImageResponseCallback(OnQueryImageResponse, nullptr);
Callback<DefaultFailureCallback> mOnQueryFailureCallback(OnQueryImageFailure, nullptr);
Callback<OnOperationalDeviceConnected> mOnConnectedCallback(OnConnected, nullptr);
Callback<OnOperationalDeviceConnectionFailure> mOnConnectionFailureCallback(OnConnectionFailure, nullptr);

constexpr uint16_t kOptionProviderNodeId      = 'n';
constexpr uint16_t kOptionProviderFabricIndex = 'f';
constexpr uint16_t kOptionUdpPort             = 'u';
constexpr uint16_t kOptionDiscriminator       = 'd';
constexpr uint16_t kOptionIPAddress           = 'i';
constexpr uint16_t kOptionDelayQuery          = 'q';

const char * ipAddress          = NULL;
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
    // TODO: This can be removed once OperationalDeviceProxy can resolve the IP Address from Node ID
    { "ipaddress", chip::ArgParser::kArgumentRequired, kOptionIPAddress },
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
                             "  -i/--ipaddress <IP Address>\n"
                             "        The IP Address of the OTA Provider to connect to. This value must be supplied.\n"
                             "  -q/--delayQuery <Time in seconds>\n"
                             "        From boot up, the amount of time to wait before triggering the QueryImage\n"
                             "        command. If none or zero is supplied, QueryImage will not be triggered.\n" };

HelpOptions helpOptions("ota-requestor-app", "Usage: ota-requestor-app [options]", "1.0");

OptionSet * allOptions[] = { &cmdLineOptions, &helpOptions, nullptr };

void OnQueryImageResponse(void * context, uint8_t status, uint32_t delayedActionTime, CharSpan imageURI, uint32_t softwareVersion,
                          CharSpan softwareVersionString, ByteSpan updateToken, bool userConsentNeeded,
                          ByteSpan metadataForRequestor)
{
    ChipLogDetail(SoftwareUpdate, "QueryImageResponse responded with action %" PRIu8, status);

    TransferSession::TransferInitData initOptions;
    initOptions.TransferCtlFlags = chip::bdx::TransferControlFlags::kReceiverDrive;
    initOptions.MaxBlockSize     = 1024;
    char testFileDes[9]          = { "test.txt" };
    initOptions.FileDesLength    = static_cast<uint16_t>(strlen(testFileDes));
    initOptions.FileDesignator   = reinterpret_cast<uint8_t *>(testFileDes);

    {
        chip::Messaging::ExchangeManager * exchangeMgr = gOperationalDeviceProxy.GetDevice().GetExchangeManager();
        chip::Optional<chip::SessionHandle> session    = gOperationalDeviceProxy.GetDevice().GetSecureSession();
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
}

void OnQueryImageFailure(void * context, uint8_t status)
{
    ChipLogDetail(SoftwareUpdate, "QueryImage failure response %" PRIu8, status);
}

void OnConnected(void * context, OperationalDeviceProxy * operationalDeviceProxy)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::Controller::OtaSoftwareUpdateProviderCluster cluster;
    constexpr EndpointId kOtaProviderEndpoint = 0;

    chip::Callback::Cancelable * successCallback = mQueryImageResponseCallback.Cancel();
    chip::Callback::Cancelable * failureCallback = mOnQueryFailureCallback.Cancel();

    // These QueryImage params have been chosen arbitrarily
    constexpr VendorId kExampleVendorId        = VendorId::Common;
    constexpr uint16_t kExampleProductId       = 77;
    constexpr uint16_t kExampleHWVersion       = 3;
    constexpr uint16_t kExampleSoftwareVersion = 0;
    constexpr uint8_t kExampleProtocolsSupported =
        EMBER_ZCL_OTA_DOWNLOAD_PROTOCOL_BDX_SYNCHRONOUS; // TODO: support this as a list once ember adds list support
    const char locationBuf[] = { 'U', 'S' };
    CharSpan exampleLocation(locationBuf);
    constexpr bool kExampleClientCanConsent = false;
    ByteSpan metadata;

    err = cluster.Associate(&(operationalDeviceProxy->GetDevice()), kOtaProviderEndpoint);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Associate() failed: %" CHIP_ERROR_FORMAT, err.Format());
        return;
    }
    err = cluster.QueryImage(successCallback, failureCallback, kExampleVendorId, kExampleProductId, kExampleSoftwareVersion,
                             kExampleProtocolsSupported, kExampleHWVersion, exampleLocation, kExampleClientCanConsent, metadata);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "QueryImage() failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void OnConnectionFailure(void * context, OperationalDeviceProxy * operationalDeviceProxy, CHIP_ERROR error)
{
    ChipLogError(SoftwareUpdate, "failed to connect: %" CHIP_ERROR_FORMAT, error.Format());
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
    case kOptionIPAddress:
        ipAddress = aValue;
        ChipLogError(SoftwareUpdate, "IP Address = %s", aValue);
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

void SendQueryImageCommand(chip::NodeId peerNodeId = providerNodeId, chip::FabricIndex peerFabricIndex = providerFabricIndex)
{
    // Explicitly calling UpdateAddress() should not be needed once OperationalDeviceProxy can resolve IP address from node ID and
    // fabric index
    IPAddress ipAddr;
    IPAddress::FromString(ipAddress, ipAddr);
    PeerAddress addr = PeerAddress::UDP(ipAddr, CHIP_PORT);
    gOperationalDeviceProxy.UpdateAddress(addr);

    Server * server                             = &(Server::GetInstance());
    OperationalDeviceProxyInitParams initParams = {
        .sessionManager = &(server->GetSecureSessionManager()),
        .exchangeMgr    = &(server->GetExchangeManager()),
        .idAllocator    = &(server->GetSessionIDAllocator()),
        .fabricsTable   = &(server->GetFabricTable()),
    };

    CHIP_ERROR err = CHIP_NO_ERROR;
    gOperationalDeviceProxy.Init(peerNodeId, peerFabricIndex, initParams);
    err = gOperationalDeviceProxy.Connect(&mOnConnectedCallback, &mOnConnectionFailureCallback);
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
