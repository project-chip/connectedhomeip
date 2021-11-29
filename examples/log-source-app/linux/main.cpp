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

#include <app/server/Server.h>
#include <app/util/util.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/logging/CHIPLogging.h>

#include <log-source-common/DiagnosticLogsCommandHandler.h>

#include <fstream>
#include <iostream>
#include <unistd.h>

using chip::BitFlags;
using chip::ArgParser::HelpOptions;
using chip::ArgParser::OptionDef;
using chip::ArgParser::OptionSet;
using chip::ArgParser::PrintArgError;
using chip::Messaging::ExchangeManager;

// TODO: this should probably be done dynamically
constexpr chip::EndpointId kOtaProviderEndpoint = 0;

constexpr uint16_t kOptionFilepath = 'f';

// Arbitrary BDX Transfer Params
constexpr uint32_t kMaxBdxBlockSize                 = 1024;
constexpr chip::System::Clock::Timeout kBdxTimeout  = chip::System::Clock::Seconds16(5 * 60); // Specification mandates >= 5 minutes
constexpr chip::System::Clock::Timeout kBdxPollFreq = chip::System::Clock::Milliseconds32(500);

// Global variables used for passing the CLI arguments to the DiagnosticLogsCommandHandler object
const char * gOtaFilepath = nullptr;

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
                             "  -f/--filepath <file>\n"
                             "        Path to a mock log file.\n" };

HelpOptions helpOptions("log-source-app", "Usage: log-source-app [options]", "1.0");

OptionSet * allOptions[] = { &cmdLineOptions, &helpOptions, nullptr };

static constexpr size_t kMaxLogMessageLength = 512;

DiagnosticLogsCommandHandler & GetLogProvider()
{
    static DiagnosticLogsCommandHandler LogProvider;
    return LogProvider;
}

void LoggingCallback(const char * module, uint8_t category, const char * msg, va_list args)
{
    // Print the log on console for debug
    va_list argsCopy;
    va_copy(argsCopy, args);
    chip::Logging::Platform::LogV(module, category, msg, argsCopy);

    // Feed the log entry into the internal circular buffer
    char buffer1[kMaxLogMessageLength];
    char buffer2[kMaxLogMessageLength];
    int s1 = vsnprintf(buffer1, sizeof(buffer1), msg, args);
    int s2 = snprintf(buffer2, sizeof(buffer2), "%s:%.*s", module, s1, buffer1);
    GetLogProvider().PushLog(chip::ByteSpan(reinterpret_cast<uint8_t *>(buffer2), s2));
}

int main(int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (chip::Platform::MemoryInit() != CHIP_NO_ERROR)
    {
        fprintf(stderr, "FAILED to initialize memory\n");
        return 1;
    }

    chip::Logging::SetLogRedirectCallback(&LoggingCallback);

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

    if (err != CHIP_NO_ERROR)
    {
        ChipLogDetail(AppServer, "RegisterUnsolicitedMessageHandler failed: %s", chip::ErrorStr(err));
        return 1;
    }

    chip::app::InteractionModelEngine::GetInstance()->RegisterCommandHandler(&GetLogProvider());

    chip::DeviceLayer::PlatformMgr().RunEventLoop();

    return 0;
}
