/*
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

#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>

#include <app/clusters/diagnostic-logs-server/diagnostic-logs-server.h>
#include <app/server/Server.h>
#include <app/util/util.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/logging/CHIPLogging.h>

#include <fstream>
#include <iostream>
#include <unistd.h>

using chip::BitFlags;
using chip::ArgParser::HelpOptions;
using chip::ArgParser::OptionDef;
using chip::ArgParser::OptionSet;
using chip::ArgParser::PrintArgError;
using chip::Messaging::ExchangeManager;

bool HandleOptions(const char * aProgram, OptionSet * aOptions, int aIdentifier, const char * aName, const char * aValue)
{
    // No option yet
    return true;
}

OptionDef cmdLineOptionsDef[] = {
    {},
};

OptionSet cmdLineOptions = { HandleOptions, cmdLineOptionsDef, "PROGRAM OPTIONS" };

HelpOptions helpOptions("log-source-app", "Usage: log-source-app [options]", "1.0");

OptionSet * allOptions[] = { &cmdLineOptions, &helpOptions, nullptr };

static constexpr size_t kMaxLogMessageLength = 512;

DiagnosticLogsCommandHandler & GetLogProvider()
{
    static DiagnosticLogsCommandHandler LogProvider;
    return LogProvider;
}

void ENFORCE_FORMAT(3, 0) LoggingCallback(const char * module, uint8_t category, const char * msg, va_list args)
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

    size_t len = chip::CanCastTo<size_t>(s2) ? static_cast<size_t>(s2) : SIZE_MAX;
    GetLogProvider().PushLog(chip::ByteSpan(reinterpret_cast<uint8_t *>(buffer2), len));
}

int main(int argc, char * argv[])
{
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
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    chip::Server::GetInstance().Init(initParams);

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(chip::Credentials::Examples::GetExampleDACProvider());

    chip::app::InteractionModelEngine::GetInstance()->RegisterCommandHandler(&GetLogProvider());

    chip::DeviceLayer::PlatformMgr().RunEventLoop();

    return 0;
}
