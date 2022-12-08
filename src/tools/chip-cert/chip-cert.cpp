/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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
 *      This file implements the CHIP certificate (chip-cert) command line tool.
 *
 *      The 'chip-cert' command line tool is used, primarily, for generating
 *      and manipulating CHIP certificate and private key material.
 *
 */

#include "chip-cert.h"

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
    "Usage: chip <command> [ <args...> ]\n"
    "\n"
    "Commands:\n"
    "\n"
    "    gen-cert -- Generate a CHIP certificate.\n"
    "\n"
    "    convert-cert -- Convert a certificate between CHIP and X509 form.\n"
    "\n"
    "    convert-key -- Convert a private key between CHIP and PEM/DER form.\n"
    "\n"
    "    resign-cert -- Resign a CHIP certificate using a new CA key.\n"
    "\n"
    "    validate-cert -- Validate a CHIP certificate chain.\n"
    "\n"
    "    print-cert -- Print a CHIP certificate.\n"
    "\n"
    "    gen-att-cert -- Generate a CHIP attestation certificate.\n"
    "\n"
    "    validate-att-cert -- Validate a CHIP attestation certificate chain.\n"
    "\n"
    "    gen-cd -- Generate a CHIP certification declaration signed message.\n"
    "\n"
    "    print-cd -- Print a CHIP certification declaration (CD) content.\n"
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
bool PrintVersion()
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
    else if (strcasecmp(argv[1], "gen-cert") == 0 || strcasecmp(argv[1], "gencert") == 0)
    {
        res = Cmd_GenCert(argc - 1, argv + 1);
    }
    else if (strcasecmp(argv[1], "convert-cert") == 0 || strcasecmp(argv[1], "convertcert") == 0)
    {
        res = Cmd_ConvertCert(argc - 1, argv + 1);
    }
    else if (strcasecmp(argv[1], "convert-key") == 0 || strcasecmp(argv[1], "convertkey") == 0)
    {
        res = Cmd_ConvertKey(argc - 1, argv + 1);
    }
    else if (strcasecmp(argv[1], "resign-cert") == 0 || strcasecmp(argv[1], "resigncert") == 0)
    {
        res = Cmd_ResignCert(argc - 1, argv + 1);
    }
    else if (strcasecmp(argv[1], "validate-cert") == 0 || strcasecmp(argv[1], "validatecert") == 0)
    {
        res = Cmd_ValidateCert(argc - 1, argv + 1);
    }
    else if (strcasecmp(argv[1], "print-cert") == 0 || strcasecmp(argv[1], "printcert") == 0)
    {
        res = Cmd_PrintCert(argc - 1, argv + 1);
    }
    else if (strcasecmp(argv[1], "gen-att-cert") == 0 || strcasecmp(argv[1], "genattcert") == 0)
    {
        res = Cmd_GenAttCert(argc - 1, argv + 1);
    }
    else if (strcasecmp(argv[1], "validate-att-cert") == 0 || strcasecmp(argv[1], "validateattcert") == 0)
    {
        res = Cmd_ValidateAttCert(argc - 1, argv + 1);
    }
    else if (strcasecmp(argv[1], "gen-cd") == 0 || strcasecmp(argv[1], "gencd") == 0)
    {
        res = Cmd_GenCD(argc - 1, argv + 1);
    }
    else if (strcasecmp(argv[1], "print-cd") == 0 || strcasecmp(argv[1], "printcd") == 0)
    {
        res = Cmd_PrintCD(argc - 1, argv + 1);
    }
    else
    {
        fprintf(stderr, "Unrecognized command: %s\n", argv[1]);
    }

    return res ? 0 : -1;
}
