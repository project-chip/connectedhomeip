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

#include <app/clusters/ota-provider/ota-provider-delegate.h>
#include <app/clusters/ota-provider/ota-provider.h>
#include <app/server/Server.h>
#include <app/util/util.h>
#include <json/json.h>
#include <ota-provider-common/BdxOtaSender.h>
#include <ota-provider-common/DefaultUserConsentProvider.h>
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

constexpr uint16_t kOptionFilepath             = 'f';
constexpr uint16_t kOptionOtaImageList         = 'o';
constexpr uint16_t kOptionQueryImageStatus     = 'q';
constexpr uint16_t kOptionUserConsentState     = 'u';
constexpr uint16_t kOptionUpdateAction         = 'a';
constexpr uint16_t kOptionDelayedActionTimeSec = 't';
constexpr uint16_t kOptionSoftwareVersion      = 's';
constexpr uint16_t kOptionSoftwareVersionStr   = 'S';
constexpr uint16_t kOptionUserConsentNeeded    = 'c';

OTAProviderExample gOtaProvider;
chip::ota::DefaultUserConsentProvider gUserConsentProvider;

// Global variables used for passing the CLI arguments to the OTAProviderExample object
static OTAProviderExample::QueryImageBehaviorType gQueryImageBehavior = OTAProviderExample::kRespondWithUnknown;
static OTAApplyUpdateAction gOptionUpdateAction                       = OTAApplyUpdateAction::kProceed;
static uint32_t gDelayedActionTimeSec                                 = 0;
static const char * gOtaFilepath                                      = nullptr;
static const char * gOtaImageListFilepath                             = nullptr;
static chip::ota::UserConsentState gUserConsentState                  = chip::ota::UserConsentState::kUnknown;
static bool gUserConsentNeeded                                        = false;
static chip::Optional<uint32_t> gSoftwareVersion;
static const char * gSoftwareVersionString = nullptr;

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
        if (aValue == NULL)
        {
            PrintArgError("%s: ERROR: NULL queryImageStatus parameter\n", aProgram);
            retval = false;
        }
        else if (strcmp(aValue, "updateAvailable") == 0)
        {
            gQueryImageBehavior = OTAProviderExample::kRespondWithUpdateAvailable;
        }
        else if (strcmp(aValue, "busy") == 0)
        {
            gQueryImageBehavior = OTAProviderExample::kRespondWithBusy;
        }
        else if (strcmp(aValue, "updateNotAvailable") == 0)
        {
            gQueryImageBehavior = OTAProviderExample::kRespondWithNotAvailable;
        }
        else
        {
            PrintArgError("%s: ERROR: Invalid queryImageStatus parameter:  %s\n", aProgram, aValue);
            retval = false;
        }
        break;
    case kOptionUpdateAction:
        if (aValue == NULL)
        {
            PrintArgError("%s: ERROR: NULL applyUpdateAction parameter\n", aProgram);
            retval = false;
        }
        else if (strcmp(aValue, "proceed") == 0)
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
    case kOptionDelayedActionTimeSec:
        gDelayedActionTimeSec = static_cast<uint32_t>(strtoul(aValue, NULL, 0));
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
    case kOptionSoftwareVersion:
        gSoftwareVersion.SetValue(static_cast<uint32_t>(strtoul(aValue, NULL, 0)));
        break;
    case kOptionUserConsentNeeded:
        gUserConsentNeeded = true;
        break;
    case kOptionSoftwareVersionStr:
        if (aValue == NULL)
        {
            PrintArgError("%s: ERROR: NULL SoftwareVersionStr parameter\n", aProgram);
            retval = false;
        }
        else if ((strlen(aValue) < 1 || strlen(aValue) > 64))
        {
            PrintArgError("%s: ERROR: SoftwareVersionStr parameter length is out of range \n", aProgram);
            retval = false;
        }
        else
        {
            gSoftwareVersionString = aValue;
        }
        break;
    default:
        PrintArgError("%s: INTERNAL ERROR: Unhandled option: %s\n", aProgram, aName);
        retval = false;
        break;
    }

    return (retval);
}

OptionDef cmdLineOptionsDef[] = {
    { "filepath", chip::ArgParser::kArgumentRequired, kOptionFilepath },
    { "otaImageList", chip::ArgParser::kArgumentRequired, kOptionOtaImageList },
    { "queryImageStatus", chip::ArgParser::kArgumentRequired, kOptionQueryImageStatus },
    { "applyUpdateAction", chip::ArgParser::kArgumentRequired, kOptionUpdateAction },
    { "delayedActionTimeSec", chip::ArgParser::kArgumentRequired, kOptionDelayedActionTimeSec },
    { "userConsentState", chip::ArgParser::kArgumentRequired, kOptionUserConsentState },
    { "softwareVersion", chip::ArgParser::kArgumentRequired, kOptionSoftwareVersion },
    { "softwareVersionStr", chip::ArgParser::kArgumentRequired, kOptionSoftwareVersionStr },
    { "UserConsentNeeded", chip::ArgParser::kNoArgument, kOptionUserConsentNeeded },
    {},
};

OptionSet cmdLineOptions = { HandleOptions, cmdLineOptionsDef, "PROGRAM OPTIONS",
                             "  -f/--filepath <file>\n"
                             "        Path to a file containing an OTA image\n"
                             "  -o/--otaImageList <file>\n"
                             "        Path to a file containing a list of OTA images\n"
                             "  -q/--queryImageStatus <updateAvailable | busy | updateNotAvailable>\n"
                             "        Value for the Status field in the QueryImageResponse\n"
                             "  -a/--applyUpdateAction <proceed | awaitNextAction | discontinue>\n"
                             "        Value for the Action field in the ApplyUpdateResponse\n"
                             "  -t/--delayedActionTimeSec <time>\n"
                             "        Value in seconds for the DelayedActionTime field in the QueryImageResponse\n"
                             "        and ApplyUpdateResponse\n"
                             "  -u/--userConsentState <granted | denied | deferred>\n"
                             "        granted: Status field in QueryImageResponse is set to updateAvailable\n"
                             "        denied: Status field in QueryImageResponse is set to updateNotAvailable\n"
                             "        deferred: Status field in QueryImageResponse is set to busy\n"
                             "        -q/--queryImageStatus overrides this option\n"
                             "  -s/--softwareVersion <version>\n"
                             "        Value for the SoftwareVersion field in the QueryImageResponse\n"
                             "        -o/--otaImageList overrides this option\n"
                             "  -S/--softwareVersionStr <version string>\n"
                             "        Value for the SoftwareVersionString field in the QueryImageResponse\n"
                             "        -o/--otaImageList overrides this option\n"
                             "  -c/--UserConsentNeeded\n"
                             "        If provided, and value of RequestorCanConsent field in QueryImage Command is true,\n"
                             "        then value of UserConsentNeeded field in the QueryImageResponse is set to true.\n"
                             "        Else, value of UserConsentNeeded is false.\n" };

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

    gOtaProvider.SetQueryImageBehavior(gQueryImageBehavior);
    gOtaProvider.SetApplyUpdateAction(gOptionUpdateAction);
    gOtaProvider.SetDelayedActionTimeSec(gDelayedActionTimeSec);
    if (gSoftwareVersion.HasValue())
    {
        gOtaProvider.SetSoftwareVersion(gSoftwareVersion.Value());
    }
    if (gSoftwareVersionString)
    {
        gOtaProvider.SetSoftwareVersionString(gSoftwareVersionString);
    }

    if (gUserConsentState != chip::ota::UserConsentState::kUnknown)
    {
        gUserConsentProvider.SetGlobalUserConsentState(gUserConsentState);
        gOtaProvider.SetUserConsentDelegate(&gUserConsentProvider);
    }

    if (gUserConsentNeeded)
    {
        gOtaProvider.SetUserConsentNeeded(true);
    }

    ChipLogDetail(SoftwareUpdate, "Using ImageList file: %s", gOtaImageListFilepath ? gOtaImageListFilepath : "(none)");

    if (gOtaImageListFilepath != nullptr)
    {
        // Parse JSON file and load the ota candidates
        std::vector<OTAProviderExample::DeviceSoftwareVersionModel> candidates;
        ParseJsonFileAndPopulateCandidates(gOtaImageListFilepath, candidates);
        gOtaProvider.SetOTACandidates(candidates);
    }

    chip::app::Clusters::OTAProvider::SetDelegate(kOtaProviderEndpoint, &gOtaProvider);
}

int main(int argc, char * argv[])
{
    VerifyOrDie(ChipLinuxAppInit(argc, argv, &cmdLineOptions) == 0);
    ChipLinuxAppMainLoop();
    return 0;
}
