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

#include "AppMain.h"
#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorStorage.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorUserConsentProvider.h>
#include <app/clusters/ota-requestor/ExtendedOTARequestorDriver.h>
#include <app/clusters/ota-requestor/OTARequestor.h>
#include <platform/Linux/OTAImageProcessorImpl.h>

using chip::BDXDownloader;
using chip::ByteSpan;
using chip::CharSpan;
using chip::EndpointId;
using chip::FabricIndex;
using chip::GetRequestorInstance;
using chip::NodeId;
using chip::OnDeviceConnected;
using chip::OnDeviceConnectionFailure;
using chip::OTADownloader;
using chip::OTAImageProcessorImpl;
using chip::OTARequestor;
using chip::PeerId;
using chip::Server;
using chip::VendorId;
using chip::Callback::Callback;
using chip::System::Layer;
using chip::Transport::PeerAddress;
using namespace chip;
using namespace chip::ArgParser;
using namespace chip::Messaging;
using namespace chip::app::Clusters::OtaSoftwareUpdateProvider::Commands;

class CustomOTARequestorDriver : public DeviceLayer::ExtendedOTARequestorDriver
{
public:
    bool CanConsent() override;
};

OTARequestor gRequestorCore;
DefaultOTARequestorStorage gRequestorStorage;
CustomOTARequestorDriver gRequestorUser;
BDXDownloader gDownloader;
OTAImageProcessorImpl gImageProcessor;
chip::ota::DefaultOTARequestorUserConsentProvider gUserConsentProvider;
static chip::ota::UserConsentState gUserConsentState = chip::ota::UserConsentState::kUnknown;

bool HandleOptions(const char * aProgram, OptionSet * aOptions, int aIdentifier, const char * aName, const char * aValue);

constexpr uint16_t kOptionUserConsentState     = 'u';
constexpr uint16_t kOptionPeriodicQueryTimeout = 'p';
constexpr uint16_t kOptionRequestorCanConsent  = 'c';
constexpr uint16_t kOptionOtaDownloadPath      = 'f';
constexpr size_t kMaxFilePathSize              = 256;

uint32_t gPeriodicQueryTimeoutSec = (24 * 60 * 60);
chip::Optional<bool> gRequestorCanConsent;
static char gOtaDownloadPath[kMaxFilePathSize] = "/tmp/test.bin";

OptionDef cmdLineOptionsDef[] = {
    { "periodicQueryTimeout", chip::ArgParser::kArgumentRequired, kOptionPeriodicQueryTimeout },
    { "requestorCanConsent", chip::ArgParser::kNoArgument, kOptionRequestorCanConsent },
    { "otaDownloadPath", chip::ArgParser::kArgumentRequired, kOptionOtaDownloadPath },
    { "userConsentState", chip::ArgParser::kArgumentRequired, kOptionUserConsentState },
    {},
};

OptionSet cmdLineOptions = { HandleOptions, cmdLineOptionsDef, "PROGRAM OPTIONS",
                             "  -p/--periodicQueryTimeout <Time in seconds>\n"
                             "        Periodic timeout for querying providers in the default OTA provider list\n"
                             "        If none or zero is supplied the timeout is set to every 24 hours. \n"
                             "  -c/--requestorCanConsent\n"
                             "        If supplied, the RequestorCanConsent field of the QueryImage command is set to "
                             "true.\n"
                             "        Otherwise, the value is determined by the driver.\n "
                             "  -f/--otaDownloadPath <file path>\n"
                             "        If supplied, the OTA image is downloaded to the given fully-qualified file-path.\n"
                             "        Otherwise, the value defaults to /tmp/test.bin.\n "
                             "  -u/--userConsentState <granted | denied | deferred>\n"
                             "        The user consent state for the first QueryImage command. For all\n"
                             "        subsequent commands, the value of granted will be used.\n"
                             "        granted: Authorize OTA requestor to download an OTA image\n"
                             "        denied: Forbid OTA requestor to download an OTA image\n"
                             "        deferred: Defer obtaining user consent \n" };

OptionSet * allOptions[] = { &cmdLineOptions, nullptr };

bool CustomOTARequestorDriver::CanConsent()
{
    return gRequestorCanConsent.ValueOr(DeviceLayer::ExtendedOTARequestorDriver::CanConsent());
}

static void InitOTARequestor(void)
{
    // Set the global instance of the OTA requestor core component
    SetRequestorInstance(&gRequestorCore);

    // Periodic query timeout must be set prior to requestor being initialized
    gRequestorUser.SetPeriodicQueryTimeout(gPeriodicQueryTimeoutSec);

    gRequestorStorage.Init(chip::Server::GetInstance().GetPersistentStorage());
    gRequestorCore.Init(chip::Server::GetInstance(), gRequestorStorage, gRequestorUser, gDownloader);
    gRequestorUser.Init(&gRequestorCore, &gImageProcessor);

    gImageProcessor.SetOTAImageFile(CharSpan::fromCharString(gOtaDownloadPath));
    gImageProcessor.SetOTADownloader(&gDownloader);

    // Set the image processor instance used for handling image being downloaded
    gDownloader.SetImageProcessorDelegate(&gImageProcessor);

    if (gUserConsentState != chip::ota::UserConsentState::kUnknown)
    {
        gUserConsentProvider.SetUserConsentState(gUserConsentState);
        gRequestorUser.SetUserConsentDelegate(&gUserConsentProvider);
    }
}

bool HandleOptions(const char * aProgram, OptionSet * aOptions, int aIdentifier, const char * aName, const char * aValue)
{
    bool retval = true;

    switch (aIdentifier)
    {
    case kOptionPeriodicQueryTimeout:
        gPeriodicQueryTimeoutSec = static_cast<uint32_t>(strtoul(aValue, NULL, 0));
        break;
    case kOptionRequestorCanConsent:
        gRequestorCanConsent.SetValue(true);
        break;
    case kOptionOtaDownloadPath:
        chip::Platform::CopyString(gOtaDownloadPath, aValue);
        break;
    case kOptionUserConsentState:
        if (aValue == NULL)
        {
            PrintArgError("%s: ERROR: NULL UserConsent parameter\n", aProgram);
            retval = false;
        }
        else if (strcmp(aValue, "granted") == 0)
        {
            gUserConsentState = chip::ota::UserConsentState::kGranted;
        }
        else if (strcmp(aValue, "denied") == 0)
        {
            gUserConsentState = chip::ota::UserConsentState::kDenied;
        }
        else if (strcmp(aValue, "deferred") == 0)
        {
            gUserConsentState = chip::ota::UserConsentState::kObtaining;
        }
        else
        {
            PrintArgError("%s: ERROR: Invalid UserConsent parameter:  %s\n", aProgram, aValue);
            retval = false;
        }
        break;
    default:
        PrintArgError("%s: INTERNAL ERROR: Unhandled option: %s\n", aProgram, aName);
        retval = false;
        break;
    }

    return (retval);
}

void ApplicationInit()
{
    // Initialize all OTA download components
    InitOTARequestor();
}

int main(int argc, char * argv[])
{
    VerifyOrDie(ChipLinuxAppInit(argc, argv, &cmdLineOptions) == 0);
    ChipLinuxAppMainLoop();
    return 0;
}
