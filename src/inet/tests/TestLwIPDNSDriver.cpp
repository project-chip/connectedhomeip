/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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

/**
 *    @file
 *      This file implements a process to effect a functional test for
 *      LwIP's Domain Name Service (DNS) interface.
 *
 */

#include <inet/InetConfig.h>
#include <lib/core/CHIPConfig.h>

#include <stdint.h>
#include <string.h>

#include <sys/time.h>

#include <CHIPVersion.h>

#include <inet/InetArgParser.h>

#include "TestInetCommon.h"
#include "TestInetCommonOptions.h"
#include "TestSetupFaultInjection.h"
#include "TestSetupSignalling.h"

using namespace chip;
using namespace chip::Inet;

#define TOOL_NAME "TestLwIPDNS"

static bool HandleNonOptionArgs(const char * progName, int argc, char * const argv[]);

static const char * sDNSServerAddr = nullptr;

// clang-format off
static ArgParser::HelpOptions gHelpOptions(TOOL_NAME,
                                           "Usage: " TOOL_NAME " [<options...>] <hostname> <dns-server-address>\n",
                                           CHIP_VERSION_STRING "\n" CHIP_TOOL_COPYRIGHT);

static ArgParser::OptionSet * gToolOptionSets[] =
{
    &gNetworkOptions,
    &gFaultInjectionOptions,
    &gHelpOptions,
    nullptr
};
// clang-format on

int main(int argc, char * argv[])
{
    SetSIGUSR1Handler();

    if (argc == 1)
    {
        gHelpOptions.PrintBriefUsage(stderr);
        exit(EXIT_FAILURE);
    }

    if (!ParseArgs(TOOL_NAME, argc, argv, gToolOptionSets, HandleNonOptionArgs))
    {
        exit(EXIT_FAILURE);
    }

    InitSystemLayer();

    InitNetwork();

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    TestLwIPDNS(sDNSServerAddr);
#else
    fprintf(stderr, "Please assert CHIP_SYSTEM_CONFIG_USE_LWIP to use this test.\n");
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

    ShutdownNetwork();

    ShutdownSystemLayer();

    return (EXIT_SUCCESS);
}

static bool HandleNonOptionArgs(const char * progName, int argc, char * const argv[])
{
    if (argc < 2)
    {
        printf("TestDNS: Missing %s argument\n", argc == 0 ? "<hostname>" : "<dns-server-address>");
        return false;
    }

    if (argc > 2)
    {
        printf("Unexpected argument: %s\n", argv[1]);
    }

    sDNSServerAddr = argv[1];

    return true;
}
