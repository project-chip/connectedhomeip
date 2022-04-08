/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
 *      This file implements the 'spake2p' command line tool.
 */

#include "spake2p.h"

namespace chip {
namespace Logging {
namespace Platform {

void LogV(const char * module, uint8_t category, const char * msg, va_list v) {}

} // namespace Platform
} // namespace Logging
} // namespace chip

namespace {

// clang-format off
const char * const sHelp =
    "Usage: spake2p <command> [ <args...> ]\n"
    "\n"
    "Commands:\n"
    "\n"
    "    gen-verifier -- Generate SPAKE2P parameters.\n"
    "\n"
    "    version -- Print the program version and exit.\n"
    "\n";
// clang-format on

/**
 * Print to standard output the program version information.
 *
 * @return Unconditionally returns true.
 *
 */
bool PrintVersion(void)
{
    printf("chip " CHIP_VERSION_STRING "\n" COPYRIGHT_STRING);

    return true;
}

} // namespace

extern "C" int main(int argc, char * argv[])
{
    bool res = false;

    chip::Platform::MemoryInit();

    if (argc == 1)
    {
        fprintf(stderr, "Please specify a command, or 'help' for help.\n");
    }
    else if (strcasecmp(argv[1], "help") == 0 || strcasecmp(argv[1], "--help") == 0 || strcasecmp(argv[1], "-h") == 0)
    {
        res = (fputs(sHelp, stdout) != EOF);
    }
    else if (strcasecmp(argv[1], "version") == 0 || strcasecmp(argv[1], "--version") == 0 || strcasecmp(argv[1], "-v") == 0)
    {
        res = PrintVersion();
    }
    else if (strcasecmp(argv[1], "gen-verifier") == 0 || strcasecmp(argv[1], "genverifier") == 0)
    {
        res = Cmd_GenVerifier(argc - 1, argv + 1);
    }
    else
    {
        fprintf(stderr, "Unrecognized command: %s\n", argv[1]);
    }

    return res ? 0 : -1;
}
