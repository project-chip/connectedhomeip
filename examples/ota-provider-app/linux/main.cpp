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
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/RandUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <ota-provider-common/BdxOtaSender.h>
#include <ota-provider-common/OTAProviderExample.h>

#include <fstream>
#include <iostream>
#include <unistd.h>

using chip::BitFlags;
using chip::app::clusters::OTAProviderDelegate;
using chip::ArgParser::HelpOptions;
using chip::ArgParser::OptionDef;
using chip::ArgParser::OptionSet;
using chip::ArgParser::PrintArgError;
using chip::bdx::TransferControlFlags;
using chip::Messaging::ExchangeManager;

// TODO: this should probably be done dynamically
constexpr chip::EndpointId kOtaProviderEndpoint = 0;

constexpr uint16_t kOptionFilepath = 'f';
const char * gOtaFilepath          = nullptr;

// Arbitrary BDX Transfer Params
constexpr uint32_t kMaxBdxBlockSize = 1024;
constexpr uint32_t kBdxTimeoutMs    = 5 * 60 * 1000; // OTA Spec mandates >= 5 minutes
constexpr uint32_t kBdxPollFreqMs   = 500;

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
    default:
        PrintArgError("%s: INTERNAL ERROR: Unhandled option: %s\n", aProgram, aName);
        retval = false;
        break;
    }

    return (retval);
}

OptionDef cmdLineOptionsDef[] = {
    { "filepath", chip::ArgParser::kArgumentRequired, kOptionFilepath },
    {},
};

OptionSet cmdLineOptions = { HandleOptions, cmdLineOptionsDef, "PROGRAM OPTIONS",
                             "  -f <file>\n"
                             "  --filepath <file>\n"
                             "        Path to a file containing an OTA image.\n" };

HelpOptions helpOptions("ota-provider-app", "Usage: ota-provider-app [options]", "1.0");

OptionSet * allOptions[] = { &cmdLineOptions, &helpOptions, nullptr };

int main(int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    OTAProviderExample otaProvider;
    BdxOtaSender bdxServer;
    ExchangeManager * exchangeMgr;

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
    InitServer();

    exchangeMgr = chip::ExchangeManager();
    err         = exchangeMgr->RegisterUnsolicitedMessageHandlerForProtocol(chip::Protocols::BDX::Id, &bdxServer);
    VerifyOrReturnError(err == CHIP_NO_ERROR, 1);

    ChipLogDetail(SoftwareUpdate, "using OTA file: %s", gOtaFilepath ? gOtaFilepath : "(none)");

    if (gOtaFilepath != nullptr)
    {
        otaProvider.SetOTAFilePath(gOtaFilepath);
        bdxServer.SetFilepath(gOtaFilepath);
    }

    chip::app::clusters::OTAProvider::SetDelegate(kOtaProviderEndpoint, &otaProvider);

    BitFlags<TransferControlFlags> bdxFlags;
    bdxFlags.Set(TransferControlFlags::kReceiverDrive);
    err = bdxServer.PrepareForTransfer(&chip::DeviceLayer::SystemLayer, chip::bdx::TransferRole::kSender, bdxFlags,
                                       kMaxBdxBlockSize, kBdxTimeoutMs, kBdxPollFreqMs);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(BDX, "failed to init BDX server: %s", chip::ErrorStr(err));
    }

    chip::DeviceLayer::PlatformMgr().RunEventLoop();

    return 0;
}
