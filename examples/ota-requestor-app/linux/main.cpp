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

#include <app/OperationalDeviceProxy.h>
#include <app/server/Server.h>
#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/support/CHIPArgParser.hpp>
#include <platform/CHIPDeviceLayer.h>

#include "LinuxOTAImageProcessor.h"
#include "LinuxOTARequestorDriver.h"
#include "app/clusters/ota-requestor/BDXDownloader.h"
#include "app/clusters/ota-requestor/OTARequestor.h"

using chip::BDXDownloader;
using chip::ByteSpan;
using chip::CharSpan;
using chip::DeviceProxy;
using chip::EndpointId;
using chip::FabricIndex;
using chip::LinuxOTAImageProcessor;
using chip::NodeId;
using chip::OnDeviceConnected;
using chip::OnDeviceConnectionFailure;
using chip::OTAImageProcessorParams;
using chip::PeerId;
using chip::Server;
using chip::VendorId;
using chip::Callback::Callback;
using chip::Inet::IPAddress;
using chip::System::Layer;
using chip::Transport::PeerAddress;
using namespace chip::ArgParser;
using namespace chip::Messaging;
using namespace chip::app::Clusters::OtaSoftwareUpdateProvider::Commands;

OTARequestor requestorCore;
LinuxOTARequestorDriver requestorUser;
BDXDownloader downloader;
LinuxOTAImageProcessor imageProcessor;

bool HandleOptions(const char * aProgram, OptionSet * aOptions, int aIdentifier, const char * aName, const char * aValue);
void OnStartDelayTimerHandler(Layer * systemLayer, void * appState);

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

    // Initialize and interconnect the Requestor and Image Processor objects -- START
    SetRequestorInstance(&requestorCore);

    // Connect the Requestor and Requestor Driver objects
    requestorCore.SetOtaRequestorDriver(&requestorUser);

    // WARNING: this is probably not realistic to know such details of the image or to even have an OTADownloader instantiated at
    // the beginning of program execution. We're using hardcoded values here for now since this is a reference application.
    // TODO: instatiate and initialize these values when QueryImageResponse tells us an image is available
    // TODO: add API for OTARequestor to pass QueryImageResponse info to the application to use for OTADownloader init
    OTAImageProcessorParams ipParams;
    ipParams.imageFile = CharSpan("test.txt");
    imageProcessor.SetOTAImageProcessorParams(ipParams);
    imageProcessor.SetOTADownloader(&downloader);

    // Connect the Downloader and Image Processor objects
    downloader.SetImageProcessorDelegate(&imageProcessor);

    requestorCore.SetBDXDownloader(&downloader);
    // Initialize and interconnect the Requestor and Image Processor objects -- END

    // Pass the IP Address to the OTARequestor object: Use of explicit IP address is temporary
    // until the Exchange Layer implements address resolution
    {
        IPAddress ipAddr;
        IPAddress::FromString(ipAddress, ipAddr);
        requestorCore.SetIpAddress(ipAddr);
    }

    // Test Mode operation: If a delay is provided, QueryImage after the timer expires
    if (delayQueryTimeInSec > 0)
    {
        // In this mode Provider node ID and fabric idx must be supplied explicitly from program args
        requestorCore.TestModeSetProviderParameters(providerNodeId, providerFabricIndex);

        chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(delayQueryTimeInSec * 1000),
                                                    OnStartDelayTimerHandler, nullptr);
    }

    chip::DeviceLayer::PlatformMgr().RunEventLoop();

    return 0;
}

// Test mode operation
void OnStartDelayTimerHandler(Layer * systemLayer, void * appState)
{
    static_cast<OTARequestor *>(GetRequestorInstance())->TriggerImmediateQuery();
}
