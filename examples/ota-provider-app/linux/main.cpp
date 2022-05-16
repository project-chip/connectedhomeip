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

#include <app/clusters/ota-provider/DefaultOTAProviderUserConsent.h>
#include <app/clusters/ota-provider/ota-provider-delegate.h>
#include <app/clusters/ota-provider/ota-provider.h>
#include <app/server/Server.h>
#include <app/util/util.h>
#include <json/json.h>
#include <ota-provider-common/BdxOtaSender.h>
#include <ota-provider-common/OTAProviderExample.h>

#include "AppMain.h"

#include <fstream>
#include <iostream>
#include <unistd.h>

using chip::BitFlags;
using chip::app::Clusters::OTAProviderDelegate;
using chip::ArgParser::OptionDef;
using chip::ArgParser::OptionSet;
using chip::ArgParser::PrintArgError;
using chip::bdx::TransferControlFlags;
using chip::Messaging::ExchangeManager;
using namespace chip::app::Clusters::OtaSoftwareUpdateProvider;

// TODO: this should probably be done dynamically
constexpr chip::EndpointId kOtaProviderEndpoint = 0;

constexpr uint16_t kOptionUpdateAction              = 'a';
constexpr uint16_t kOptionUserConsentNeeded         = 'c';
constexpr uint16_t kOptionFilepath                  = 'f';
constexpr uint16_t kOptionImageUri                  = 'i';
constexpr uint16_t kOptionOtaImageList              = 'o';
constexpr uint16_t kOptionDelayedApplyActionTimeSec = 'p';
constexpr uint16_t kOptionQueryImageStatus          = 'q';
constexpr uint16_t kOptionDelayedQueryActionTimeSec = 't';
constexpr uint16_t kOptionUserConsentState          = 'u';
constexpr uint16_t kOptionIgnoreQueryImage          = 'x';
constexpr uint16_t kOptionIgnoreApplyUpdate         = 'y';
constexpr uint16_t kOptionPollInterval              = 'P';

OTAProviderExample gOtaProvider;
chip::ota::DefaultOTAProviderUserConsent gUserConsentProvider;

// Global variables used for passing the CLI arguments to the OTAProviderExample object
static OTAQueryStatus gQueryImageStatus              = OTAQueryStatus::kUpdateAvailable;
static OTAApplyUpdateAction gOptionUpdateAction      = OTAApplyUpdateAction::kProceed;
static uint32_t gDelayedQueryActionTimeSec           = 0;
static uint32_t gDelayedApplyActionTimeSec           = 0;
static const char * gOtaFilepath                     = nullptr;
static const char * gOtaImageListFilepath            = nullptr;
static const char * gImageUri                        = nullptr;
static chip::ota::UserConsentState gUserConsentState = chip::ota::UserConsentState::kUnknown;
static bool gUserConsentNeeded                       = false;
static uint32_t gIgnoreQueryImageCount               = 0;
static uint32_t gIgnoreApplyUpdateCount              = 0;
static uint32_t gPollInterval                        = 0;

// Parses the JSON filepath and extracts DeviceSoftwareVersionModel parameters
static bool ParseJsonFileAndPopulateCandidates(const char * filepath,
                                               std::vector<OTAProviderExample::DeviceSoftwareVersionModel> & candidates)
{
    bool ret = false;
    Json::Value root;
    Json::CharReaderBuilder builder;
    JSONCPP_STRING errs;
    std::ifstream ifs;

    builder["collectComments"] = true; // allow C/C++ type comments in JSON file
    ifs.open(filepath);

    if (!ifs.good())
    {
        ChipLogError(SoftwareUpdate, "Error opening ifstream with file: \"%s\"", filepath);
        return ret;
    }

    if (!parseFromStream(builder, ifs, &root, &errs))
    {
        ChipLogError(SoftwareUpdate, "Error parsing JSON from file: \"%s\"", filepath);
        return ret;
    }

    const Json::Value devSofVerModValue = root["deviceSoftwareVersionModel"];
    if (!devSofVerModValue || !devSofVerModValue.isArray())
    {
        ChipLogError(SoftwareUpdate, "Error: Key deviceSoftwareVersionModel not found or its value is not of type Array");
    }
    else
    {
        for (auto iter : devSofVerModValue)
        {
            OTAProviderExample::DeviceSoftwareVersionModel candidate;
            candidate.vendorId        = static_cast<chip::VendorId>(iter.get("vendorId", 1).asUInt());
            candidate.productId       = static_cast<uint16_t>(iter.get("productId", 1).asUInt());
            candidate.softwareVersion = static_cast<uint32_t>(iter.get("softwareVersion", 10).asUInt64());
            strncpy(candidate.softwareVersionString, iter.get("softwareVersionString", "1.0.0").asCString(),
                    OTAProviderExample::SW_VER_STR_MAX_LEN);
            candidate.cDVersionNumber              = static_cast<uint16_t>(iter.get("cDVersionNumber", 0).asUInt());
            candidate.softwareVersionValid         = iter.get("softwareVersionValid", true).asBool() ? true : false;
            candidate.minApplicableSoftwareVersion = static_cast<uint32_t>(iter.get("minApplicableSoftwareVersion", 0).asUInt64());
            candidate.maxApplicableSoftwareVersion =
                static_cast<uint32_t>(iter.get("maxApplicableSoftwareVersion", 1000).asUInt64());
            strncpy(candidate.otaURL, iter.get("otaURL", "https://test.com").asCString(), OTAProviderExample::OTA_URL_MAX_LEN);
            candidates.push_back(candidate);
            ret = true;
        }
    }
    return ret;
}

bool HandleOptions(const char * aProgram, OptionSet * aOptions, int aIdentifier, const char * aName, const char * aValue)
{
    bool retval = true;
    static bool kOptionFilepathSelected;
    static bool kOptionOtaImageListSelected;

    switch (aIdentifier)
    {
    case kOptionFilepath:
        kOptionFilepathSelected = true;
        if (0 != access(aValue, R_OK))
        {
            PrintArgError("%s: not permitted to read %s\n", aProgram, aValue);
            retval = false;
        }
        else if (kOptionOtaImageListSelected)
        {
            PrintArgError("%s: Cannot have both OptionOtaImageList and kOptionOtaFilepath \n", aProgram);
            retval = false;
        }
        else
        {
            gOtaFilepath = aValue;
        }
        break;
    case kOptionImageUri:
        gImageUri = aValue;
        break;
    case kOptionOtaImageList:
        kOptionOtaImageListSelected = true;
        if (0 != access(aValue, R_OK))
        {
            PrintArgError("%s: not permitted to read %s\n", aProgram, aValue);
            retval = false;
        }
        else if (kOptionFilepathSelected)
        {
            PrintArgError("%s: Cannot have both OptionOtaImageList and kOptionOtaFilepath \n", aProgram);
            retval = false;
        }
        else
        {
            gOtaImageListFilepath = aValue;
        }
        break;
    case kOptionQueryImageStatus:
        if (strcmp(aValue, "updateAvailable") == 0)
        {
            gQueryImageStatus = OTAQueryStatus::kUpdateAvailable;
        }
        else if (strcmp(aValue, "busy") == 0)
        {
            gQueryImageStatus = OTAQueryStatus::kBusy;
        }
        else if (strcmp(aValue, "updateNotAvailable") == 0)
        {
            gQueryImageStatus = OTAQueryStatus::kNotAvailable;
        }
        else
        {
            PrintArgError("%s: ERROR: Invalid queryImageStatus parameter:  %s\n", aProgram, aValue);
            retval = false;
        }
        break;
    case kOptionIgnoreQueryImage:
        gIgnoreQueryImageCount = static_cast<uint32_t>(strtoul(aValue, NULL, 0));
        break;
    case kOptionIgnoreApplyUpdate:
        gIgnoreApplyUpdateCount = static_cast<uint32_t>(strtoul(aValue, NULL, 0));
        break;
    case kOptionUpdateAction:
        if (strcmp(aValue, "proceed") == 0)
        {
            gOptionUpdateAction = OTAApplyUpdateAction::kProceed;
        }
        else if (strcmp(aValue, "awaitNextAction") == 0)
        {
            gOptionUpdateAction = OTAApplyUpdateAction::kAwaitNextAction;
        }
        else if (strcmp(aValue, "discontinue") == 0)
        {
            gOptionUpdateAction = OTAApplyUpdateAction::kDiscontinue;
        }
        else
        {
            PrintArgError("%s: ERROR: Invalid applyUpdateAction parameter:  %s\n", aProgram, aValue);
            retval = false;
        }
        break;
    case kOptionDelayedQueryActionTimeSec:
        gDelayedQueryActionTimeSec = static_cast<uint32_t>(strtoul(aValue, NULL, 0));
        break;
    case kOptionDelayedApplyActionTimeSec:
        gDelayedApplyActionTimeSec = static_cast<uint32_t>(strtoul(aValue, NULL, 0));
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
            PrintArgError("%s: ERROR: Invalid UserConsent parameter:  %s\n", aProgram, aValue);
            retval = false;
        }
        break;
    case kOptionUserConsentNeeded:
        gUserConsentNeeded = true;
        break;
    case kOptionPollInterval:
        gPollInterval = static_cast<uint32_t>(strtoul(aValue, NULL, 0));
        break;

    default:
        PrintArgError("%s: INTERNAL ERROR: Unhandled option: %s\n", aProgram, aName);
        retval = false;
        break;
    }

    return (retval);
}

OptionDef cmdLineOptionsDef[] = {
    { "applyUpdateAction", chip::ArgParser::kArgumentRequired, kOptionUpdateAction },
    { "userConsentNeeded", chip::ArgParser::kNoArgument, kOptionUserConsentNeeded },
    { "filepath", chip::ArgParser::kArgumentRequired, kOptionFilepath },
    { "imageUri", chip::ArgParser::kArgumentRequired, kOptionImageUri },
    { "otaImageList", chip::ArgParser::kArgumentRequired, kOptionOtaImageList },
    { "delayedApplyActionTimeSec", chip::ArgParser::kArgumentRequired, kOptionDelayedApplyActionTimeSec },
    { "queryImageStatus", chip::ArgParser::kArgumentRequired, kOptionQueryImageStatus },
    { "delayedQueryActionTimeSec", chip::ArgParser::kArgumentRequired, kOptionDelayedQueryActionTimeSec },
    { "userConsentState", chip::ArgParser::kArgumentRequired, kOptionUserConsentState },
    { "ignoreQueryImage", chip::ArgParser::kArgumentRequired, kOptionIgnoreQueryImage },
    { "ignoreApplyUpdate", chip::ArgParser::kArgumentRequired, kOptionIgnoreApplyUpdate },
    { "pollInterval", chip::ArgParser::kArgumentRequired, kOptionPollInterval },
    {},
};

OptionSet cmdLineOptions = { HandleOptions, cmdLineOptionsDef, "PROGRAM OPTIONS",
                             "  -a, --applyUpdateAction <proceed | awaitNextAction | discontinue>\n"
                             "        Value for the Action field in the first ApplyUpdateResponse.\n"
                             "        For all subsequent responses, the value of proceed will be used.\n"
                             "  -c, --userConsentNeeded\n"
                             "        If supplied, value of the UserConsentNeeded field in the QueryImageResponse\n"
                             "        is set to true. This is only applicable if value of the RequestorCanConsent\n"
                             "        field in QueryImage Command is true.\n"
                             "        Otherwise, value of the UserConsentNeeded field is false.\n"
                             "  -f, --filepath <file path>\n"
                             "        Path to a file containing an OTA image\n"
                             "  -i, --imageUri <uri>\n"
                             "        Value for the ImageURI field in the QueryImageResponse.\n"
                             "        If none is supplied, a valid URI is generated.\n"
                             "  -o, --otaImageList <file path>\n"
                             "        Path to a file containing a list of OTA images\n"
                             "  -p, --delayedApplyActionTimeSec <time in seconds>\n"
                             "        Value for the DelayedActionTime field in the first ApplyUpdateResponse.\n"
                             "        For all subsequent responses, the value of zero will be used.\n"
                             "  -q, --queryImageStatus <updateAvailable | busy | updateNotAvailable>\n"
                             "        Value for the Status field in the first QueryImageResponse.\n"
                             "        For all subsequent responses, the value of updateAvailable will be used.\n"
                             "  -t, --delayedQueryActionTimeSec <time in seconds>\n"
                             "        Value for the DelayedActionTime field in the first QueryImageResponse.\n"
                             "        For all subsequent responses, the value of zero will be used.\n"
                             "  -u, --userConsentState <granted | denied | deferred>\n"
                             "        The user consent state for the first QueryImageResponse. For all subsequent\n"
                             "        responses, the value of granted will be used.\n"
                             "        Note that --queryImageStatus overrides this option.\n"
                             "        granted: Status field in the first QueryImageResponse is set to updateAvailable\n"
                             "        denied: Status field in the first QueryImageResponse is set to updateNotAvailable\n"
                             "        deferred: Status field in the first QueryImageResponse is set to busy\n"
                             "  -x, --ignoreQueryImage <ignore count>\n"
                             "        The number of times to ignore the QueryImage Command and not send a response.\n"
                             "  -y, --ignoreApplyUpdate <ignore count>\n"
                             "        The number of times to ignore the ApplyUpdateRequest Command and not send a response.\n"
                             "  -P, --pollInterval <time in milliseconds>\n"
                             "        Poll interval for the BDX transfer \n" };

OptionSet * allOptions[] = { &cmdLineOptions, nullptr };

void ApplicationInit()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    BdxOtaSender * bdxOtaSender = gOtaProvider.GetBdxOtaSender();
    VerifyOrReturn(bdxOtaSender != nullptr);
    err = chip::Server::GetInstance().GetExchangeManager().RegisterUnsolicitedMessageHandlerForProtocol(chip::Protocols::BDX::Id,
                                                                                                        bdxOtaSender);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogDetail(SoftwareUpdate, "RegisterUnsolicitedMessageHandler failed: %s", chip::ErrorStr(err));
        return;
    }

    ChipLogDetail(SoftwareUpdate, "Using OTA file: %s", gOtaFilepath ? gOtaFilepath : "(none)");

    if (gOtaFilepath != nullptr)
    {
        gOtaProvider.SetOTAFilePath(gOtaFilepath);
    }

    if (gImageUri != nullptr)
    {
        gOtaProvider.SetImageUri(gImageUri);
    }

    gOtaProvider.SetIgnoreQueryImageCount(gIgnoreQueryImageCount);
    gOtaProvider.SetIgnoreApplyUpdateCount(gIgnoreApplyUpdateCount);
    gOtaProvider.SetQueryImageStatus(gQueryImageStatus);
    gOtaProvider.SetApplyUpdateAction(gOptionUpdateAction);
    gOtaProvider.SetDelayedQueryActionTimeSec(gDelayedQueryActionTimeSec);
    gOtaProvider.SetDelayedApplyActionTimeSec(gDelayedApplyActionTimeSec);

    if (gUserConsentState != chip::ota::UserConsentState::kUnknown)
    {
        gUserConsentProvider.SetGlobalUserConsentState(gUserConsentState);
        gOtaProvider.SetUserConsentDelegate(&gUserConsentProvider);
    }

    if (gUserConsentNeeded)
    {
        gOtaProvider.SetUserConsentNeeded(true);
    }

    if (gPollInterval != 0)
    {
        gOtaProvider.SetPollInterval(gPollInterval);
    }

    ChipLogDetail(SoftwareUpdate, "Using ImageList file: %s", gOtaImageListFilepath ? gOtaImageListFilepath : "(none)");

    if (gOtaImageListFilepath != nullptr)
    {
        // Parse JSON file and load the ota candidates
        std::vector<OTAProviderExample::DeviceSoftwareVersionModel> candidates;
        ParseJsonFileAndPopulateCandidates(gOtaImageListFilepath, candidates);
        gOtaProvider.SetOTACandidates(candidates);
    }

    if ((gOtaFilepath == nullptr) && (gOtaImageListFilepath == nullptr))
    {
        ChipLogError(SoftwareUpdate, "Either an OTA file or image list file must be specified");
        chipDie();
    }

    chip::app::Clusters::OTAProvider::SetDelegate(kOtaProviderEndpoint, &gOtaProvider);
}

int main(int argc, char * argv[])
{
    VerifyOrDie(ChipLinuxAppInit(argc, argv, &cmdLineOptions) == 0);
    ChipLinuxAppMainLoop();
    return 0;
}
