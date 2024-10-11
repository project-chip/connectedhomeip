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
#include <app/clusters/ota-requestor/DefaultOTARequestor.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorStorage.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorUserConsent.h>
#include <app/clusters/ota-requestor/ExtendedOTARequestorDriver.h>
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
using chip::PeerId;
using chip::Server;
using chip::VendorId;
using chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum;
using chip::Callback::Callback;
using chip::System::Layer;
using chip::Transport::PeerAddress;
using namespace chip;
using namespace chip::app;
using namespace chip::ArgParser;
using namespace chip::DeviceLayer;
using namespace chip::Messaging;
using namespace chip::app::Clusters::OtaSoftwareUpdateProvider::Commands;

class CustomOTARequestorDriver : public DeviceLayer::ExtendedOTARequestorDriver
{
public:
    bool CanConsent() override;
    void UpdateDownloaded() override;
    void UpdateConfirmed(System::Clock::Seconds32 delay) override;
    static void AppliedNotifyUpdateTimer(System::Layer * systemLayer, void * appState);
    void SendNotifyUpdateApplied();
};

DefaultOTARequestor gRequestorCore;
DefaultOTARequestorStorage gRequestorStorage;
CustomOTARequestorDriver gRequestorUser;
BDXDownloader gDownloader;
OTAImageProcessorImpl gImageProcessor;
chip::ota::DefaultOTARequestorUserConsent gUserConsentProvider;
static chip::ota::UserConsentState gUserConsentState = chip::ota::UserConsentState::kUnknown;

bool HandleOptions(const char * aProgram, OptionSet * aOptions, int aIdentifier, const char * aName, const char * aValue);

constexpr uint16_t kOptionAutoApplyImage       = 'a';
constexpr uint16_t kOptionRequestorCanConsent  = 'c';
constexpr uint16_t kOptionDisableNotify        = 'd';
constexpr uint16_t kOptionOtaDownloadPath      = 'f';
constexpr uint16_t kOptionPeriodicQueryTimeout = 'p';
constexpr uint16_t kOptionUserConsentState     = 'u';
constexpr uint16_t kOptionWatchdogTimeout      = 'w';
constexpr uint16_t kSkipExecImageFile          = 's';
constexpr size_t kMaxFilePathSize              = 256;

uint32_t gPeriodicQueryTimeoutSec = 0;
uint32_t gWatchdogTimeoutSec      = 0;
chip::Optional<bool> gRequestorCanConsent;
static char gOtaDownloadPath[kMaxFilePathSize] = "/tmp/test.bin";
bool gAutoApplyImage                           = false;
bool gSendNotifyUpdateApplied                  = true;
bool gSkipExecImageFile                        = false;

OptionDef cmdLineOptionsDef[] = {
    { "autoApplyImage", chip::ArgParser::kNoArgument, kOptionAutoApplyImage },
    { "requestorCanConsent", chip::ArgParser::kArgumentRequired, kOptionRequestorCanConsent },
    { "disableNotifyUpdateApplied", chip::ArgParser::kNoArgument, kOptionDisableNotify },
    { "otaDownloadPath", chip::ArgParser::kArgumentRequired, kOptionOtaDownloadPath },
    { "periodicQueryTimeout", chip::ArgParser::kArgumentRequired, kOptionPeriodicQueryTimeout },
    { "userConsentState", chip::ArgParser::kArgumentRequired, kOptionUserConsentState },
    { "watchdogTimeout", chip::ArgParser::kArgumentRequired, kOptionWatchdogTimeout },
    { "skipExecImageFile", chip::ArgParser::kNoArgument, kSkipExecImageFile },
    {},
};

// Options for various test scenarios
OptionSet cmdLineOptions = {
    HandleOptions, cmdLineOptionsDef, "PROGRAM OPTIONS",
    "  -a, --autoApplyImage\n"
    "       If supplied, apply the image immediately after download.\n"
    "       Otherwise, the OTA update is complete after image download.\n"
    "  -c, --requestorCanConsent <true | false>\n"
    "       Value for the RequestorCanConsent field in the QueryImage command.\n"
    "       If not supplied, the value is determined by the driver.\n"
    "  -d, --disableNotifyUpdateApplied\n"
    "       If supplied, disable sending of the NotifyUpdateApplied command.\n"
    "       Otherwise, after successfully loading into the updated image, send the NotifyUpdateApplied command.\n"
    "  -f, --otaDownloadPath <file path>\n"
    "       If supplied, the OTA image is downloaded to the given fully-qualified file-path.\n"
    "       Otherwise, the default location for the downloaded image is at /tmp/test.bin\n"
    "  -p, --periodicQueryTimeout <time in seconds>\n"
    "       The periodic time interval to wait before attempting to query a provider from the default OTA provider list.\n"
    "       If none or zero is supplied, the timeout is determined by the driver.\n"
    "  -u, --userConsentState <granted | denied | deferred>\n"
    "       Represents the current user consent status when the OTA Requestor is acting as a user consent\n"
    "       delegate. This value is only applicable if value of the UserConsentNeeded field in the\n"
    "       QueryImageResponse is set to true. This value is used for the first attempt to\n"
    "       download. For all subsequent queries, the value of granted will be used.\n"
    "       granted: Authorize OTA requestor to download an OTA image\n"
    "       denied: Forbid OTA requestor to download an OTA image\n"
    "       deferred: Defer obtaining user consent\n"
    "  -w, --watchdogTimeout <time in seconds>\n"
    "       Maximum amount of time allowed for an OTA download before the process is cancelled and state reset to idle.\n"
    "       If none or zero is supplied, the timeout is determined by the driver.\n"
    "  -s, --skipExecImageFile\n"
    "       To only check Notify Update Applied Command, skip the Image File execution.\n"
};

OptionSet * allOptions[] = { &cmdLineOptions, nullptr };

// Network commissioning
namespace {
constexpr EndpointId kNetworkCommissioningEndpointSecondary = 0xFFFE;
} // namespace

bool CustomOTARequestorDriver::CanConsent()
{
    return gRequestorCanConsent.ValueOr(DeviceLayer::ExtendedOTARequestorDriver::CanConsent());
}

void CustomOTARequestorDriver::UpdateDownloaded()
{
    if (gAutoApplyImage || gSkipExecImageFile)
    {
        // Let the default driver take further action to apply the image.
        // All member variables will be implicitly reset upon loading into the new image.
        DefaultOTARequestorDriver::UpdateDownloaded();
    }
    else
    {
        // Download complete but we're not going to apply image, so reset provider retry counter.
        mProviderRetryCount = 0;

        // Reset to put the state back to idle to allow the next OTA update to occur
        gRequestorCore.Reset();
    }
}

void CustomOTARequestorDriver::UpdateConfirmed(System::Clock::Seconds32 delay)
{
    if (gSkipExecImageFile)
    {
        // Just waiting delay time
        VerifyOrDie(SystemLayer().StartTimer(std::chrono::duration_cast<System::Clock::Timeout>(delay), AppliedNotifyUpdateTimer,
                                             this) == CHIP_NO_ERROR);
    }
    else
    {
        // Let the default driver take further action to apply the image.
        // All member variables will be implicitly reset upon loading into the new image.
        DefaultOTARequestorDriver::UpdateConfirmed(delay);
    }
}

void CustomOTARequestorDriver::AppliedNotifyUpdateTimer(System::Layer * systemLayer, void * appState)
{
    CustomOTARequestorDriver * driver = reinterpret_cast<CustomOTARequestorDriver *>(appState);
    driver->SendNotifyUpdateApplied();
}

void CustomOTARequestorDriver::SendNotifyUpdateApplied()
{
    VerifyOrDie(mRequestor != nullptr);
    mRequestor->NotifyUpdateApplied();
    // After sending Noficy Update Applied command, so reset provider retry counter.
    mProviderRetryCount = 0;

    // Reset to put the state back to idle to allow the next OTA update to occur
    gRequestorCore.Reset();
}

static void InitOTARequestor(void)
{
    // Set the global instance of the OTA requestor core component
    SetRequestorInstance(&gRequestorCore);

    // Periodic query timeout must be set prior to the driver being initialized
    gRequestorUser.SetPeriodicQueryTimeout(gPeriodicQueryTimeoutSec);

    // Watchdog timeout can be set any time before a query image is sent
    gRequestorUser.SetWatchdogTimeout(gWatchdogTimeoutSec);
    gRequestorUser.SetSendNotifyUpdateApplied(gSendNotifyUpdateApplied);

    gRequestorStorage.Init(chip::Server::GetInstance().GetPersistentStorage());
    gRequestorCore.Init(chip::Server::GetInstance(), gRequestorStorage, gRequestorUser, gDownloader);
    gRequestorUser.Init(&gRequestorCore, &gImageProcessor);

    gImageProcessor.SetOTAImageFile(gOtaDownloadPath);
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
        if (strcmp(aValue, "true") == 0)
        {
            gRequestorCanConsent.SetValue(true);
        }
        else if (strcmp(aValue, "false") == 0)
        {
            gRequestorCanConsent.SetValue(false);
        }
        else
        {
            ChipLogError(SoftwareUpdate, "%s: ERROR: Invalid requestorCanConsent parameter: %s\n", aProgram, aValue);
            retval = false;
        }
        break;
    case kOptionOtaDownloadPath:
        chip::Platform::CopyString(gOtaDownloadPath, aValue);
        break;
    case kOptionUserConsentState:
        if (strcmp(aValue, "granted") == 0)
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
            ChipLogError(SoftwareUpdate, "%s: ERROR: Invalid UserConsent parameter: %s\n", aProgram, aValue);
            retval = false;
        }
        break;
    case kOptionAutoApplyImage:
        gAutoApplyImage = true;
        break;
    case kOptionWatchdogTimeout:
        gWatchdogTimeoutSec = static_cast<uint32_t>(strtoul(aValue, NULL, 0));
        break;
    case kOptionDisableNotify:
        // By default, NotifyUpdateApplied should always be sent. In the presence of this option, disable sending of the command.
        gSendNotifyUpdateApplied = false;
        break;
    case kSkipExecImageFile:
        gSkipExecImageFile = true;
        break;
    default:
        ChipLogError(SoftwareUpdate, "%s: INTERNAL ERROR: Unhandled option: %s\n", aProgram, aName);
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

void ApplicationShutdown() {}

int main(int argc, char * argv[])
{
    VerifyOrDie(ChipLinuxAppInit(argc, argv, &cmdLineOptions, MakeOptional(kNetworkCommissioningEndpointSecondary)) == 0);
    ChipLinuxAppMainLoop();

    // If the event loop had been stopped due to an update being applied, boot into the new image
    if (gRequestorCore.GetCurrentUpdateState() == OTAUpdateStateEnum::kApplying)
    {
        if (kMaxFilePathSize <= strlen(kImageExecPath))
        {
            ChipLogError(SoftwareUpdate, "Buffer too small for the new image file path: %s", kImageExecPath);
            return -1;
        }

        argv[0] = kImageExecPath;
        execv(argv[0], argv);

        // If successfully executing the new image, execv should not return
        ChipLogError(SoftwareUpdate, "The OTA image is invalid");
    }
    return 0;
}
