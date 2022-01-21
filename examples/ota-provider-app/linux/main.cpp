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

#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>

#include <app/clusters/ota-provider/ota-provider-delegate.h>
#include <app/clusters/ota-provider/ota-provider.h>
#include <app/server/Server.h>
#include <app/util/util.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/logging/CHIPLogging.h>

#include <ota-provider-common/BdxOtaSender.h>
#include <ota-provider-common/OTAProviderExample.h>

#include <fstream>
#include <iostream>
#include <unistd.h>

using chip::BitFlags;
using chip::app::Clusters::OTAProviderDelegate;
using chip::ArgParser::HelpOptions;
using chip::ArgParser::OptionDef;
using chip::ArgParser::OptionSet;
using chip::ArgParser::PrintArgError;
using chip::bdx::TransferControlFlags;
using chip::Messaging::ExchangeManager;

// TODO: this should probably be done dynamically
constexpr chip::EndpointId kOtaProviderEndpoint = 0;

constexpr uint16_t kOptionFilepath             = 'f';
constexpr uint16_t kOptionQueryImageBehavior   = 'q';
constexpr uint16_t kOptionDelayedActionTimeSec = 'd';

// Global variables used for passing the CLI arguments to the OTAProviderExample object
OTAProviderExample::QueryImageBehaviorType gQueryImageBehavior = OTAProviderExample::kRespondWithUpdateAvailable;
uint32_t gDelayedActionTimeSec                                 = 0;
const char * gOtaFilepath                                      = nullptr;

bool HandleOptions(const char * aProgram, OptionSet * aOptions, int aIdentifier, const char * aName, const char * aValue)
{
    bool retval = true;

    switch (aIdentifier)
    {
    case kOptionFilepath:
        if (0 != access(aValue, R_OK))
        {
            PrintArgError("%s: not permitted to read %s\n", aProgram, aValue);
            retval = false;
        }
        else
        {
            gOtaFilepath = aValue;
        }
        break;
    case kOptionQueryImageBehavior:
        if (aValue == NULL)
        {
            PrintArgError("%s: ERROR: NULL QueryImageBehavior parameter\n", aProgram);
            retval = false;
        }
        else if (strcmp(aValue, "UpdateAvailable") == 0)
        {
            gQueryImageBehavior = OTAProviderExample::kRespondWithUpdateAvailable;
        }
        else if (strcmp(aValue, "Busy") == 0)
        {
            gQueryImageBehavior = OTAProviderExample::kRespondWithBusy;
        }
        else if (strcmp(aValue, "UpdateNotAvailable") == 0)
        {
            gQueryImageBehavior = OTAProviderExample::kRespondWithNotAvailable;
        }
        else
        {
            PrintArgError("%s: ERROR: Invalid QueryImageBehavior parameter:  %s\n", aProgram, aValue);
            retval = false;
        }
        break;
    case kOptionDelayedActionTimeSec:
        gDelayedActionTimeSec = static_cast<uint32_t>(strtol(aValue, NULL, 0));
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
    { "QueryImageBehavior", chip::ArgParser::kArgumentRequired, kOptionQueryImageBehavior },
    { "DelayedActionTimeSec", chip::ArgParser::kArgumentRequired, kOptionDelayedActionTimeSec },
    {},
};

OptionSet cmdLineOptions = { HandleOptions, cmdLineOptionsDef, "PROGRAM OPTIONS",
                             "  -f/--filepath <file>\n"
                             "        Path to a file containing an OTA image.\n"
                             "  -q/--QueryImageBehavior <UpdateAvailable | Busy | UpdateNotAvailable>\n"
                             "        Status value in the Query Image Response\n"
                             "  -d/--DelayedActionTimeSec <time>\n"
                             "        Value in seconds for the DelayedActionTime in the Query Image Response\n"
                             "        and Apply Update Response\n" };

HelpOptions helpOptions("ota-provider-app", "Usage: ota-provider-app [options]", "1.0");

OptionSet * allOptions[] = { &cmdLineOptions, &helpOptions, nullptr };

int main(int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    OTAProviderExample otaProvider;

    if (chip::Platform::MemoryInit() != CHIP_NO_ERROR)
    {
        fprintf(stderr, "FAILED to initialize memory\n");
        return 1;
    }

    if (chip::DeviceLayer::PlatformMgr().InitChipStack() != CHIP_NO_ERROR)
    {
        fprintf(stderr, "FAILED to initialize chip stack\n");
        return 1;
    }

    if (!chip::ArgParser::ParseArgs(argv[0], argc, argv, allOptions))
    {
        return 1;
    }

    chip::DeviceLayer::ConfigurationMgr().LogDeviceConfig();
    chip::Server::GetInstance().Init();

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(chip::Credentials::Examples::GetExampleDACProvider());

    BdxOtaSender * bdxOtaSender = otaProvider.GetBdxOtaSender();
    VerifyOrReturnError(bdxOtaSender != nullptr, 1);
    err = chip::Server::GetInstance().GetExchangeManager().RegisterUnsolicitedMessageHandlerForProtocol(chip::Protocols::BDX::Id,
                                                                                                        bdxOtaSender);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogDetail(SoftwareUpdate, "RegisterUnsolicitedMessageHandler failed: %s", chip::ErrorStr(err));
        return 1;
    }

    ChipLogDetail(SoftwareUpdate, "using OTA file: %s", gOtaFilepath ? gOtaFilepath : "(none)");

    if (gOtaFilepath != nullptr)
    {
        otaProvider.SetOTAFilePath(gOtaFilepath);
    }

    otaProvider.SetQueryImageBehavior(gQueryImageBehavior);
    otaProvider.SetDelayedActionTimeSec(gDelayedActionTimeSec);

    chip::app::Clusters::OTAProvider::SetDelegate(kOtaProviderEndpoint, &otaProvider);

    chip::DeviceLayer::PlatformMgr().RunEventLoop();

    return 0;
}
