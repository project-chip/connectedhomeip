/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      This file implements the command handler for the 'chip-cert' tool
 *      that converts a CHIP certificate between CHIP TLV and X.509
 *      formats.
 *
 */

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "chip-cert.h"

using namespace chip::ArgParser;
using namespace chip::Credentials;

#define CMD_NAME "chip-cert convert-cert"

static bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg);
static bool HandleNonOptionArgs(const char * progName, int argc, char * argv[]);

// clang-format off
static OptionDef gCmdOptionDefs[] =
{
    { "x509",       kNoArgument, 'p' },
    { "x509-pem",   kNoArgument, 'p' }, // alias for --x509
    { "x509-der",   kNoArgument, 'x' },
    { "chip",       kNoArgument, 'c' },
    { "chip-b64",   kNoArgument, 'b' },
    { }
};

static const char * const gCmdOptionHelp =
    "  -p, --x509, --x509-pem\n"
    "\n"
    "       Output certificate in X.509 PEM format.\n"
    "\n"
    "  -x, --x509-der\n"
    "\n"
    "       Output certificate in X.509 DER format.\n"
    "\n"
    "  -c, --chip\n"
    "\n"
    "       Output certificate in raw CHIP TLV format.\n"
    "\n"
    "  -b --chip-b64\n"
    "\n"
    "       Output certificate in CHIP TLV base-64 encoded format.\n"
    "\n"
    ;

static OptionSet gCmdOptions =
{
    HandleOption,
    gCmdOptionDefs,
    "COMMAND OPTIONS",
    gCmdOptionHelp
};

static HelpOptions gHelpOptions(
    CMD_NAME,
    "Usage: " CMD_NAME " [ <options...> ] <in-file> <out-file>\n",
    CHIP_VERSION_STRING "\n" COPYRIGHT_STRING,
    "Convert a certificate between CHIP and X509 forms.\n"
    "\n"
    "ARGUMENTS\n"
    "\n"
    "  <in-file>\n"
    "\n"
    "       The input certificate file name, or - to read from stdin. The\n"
    "       format of the input certificate is auto-detected and can be any\n"
    "       of: X.509 PEM, X.509 DER, CHIP base-64 or CHIP raw TLV.\n"
    "\n"
    "  <out-file>\n"
    "\n"
    "       The output certificate file name, or - to write to stdout.\n"
    "\n"
);

static OptionSet * gCmdOptionSets[] =
{
    &gCmdOptions,
    &gHelpOptions,
    nullptr
};
// clang-format on

static const char * gInFileName  = nullptr;
static const char * gOutFileName = nullptr;
static CertFormat gOutCertFormat = kCertFormat_Chip_Base64;

bool Cmd_ConvertCert(int argc, char * argv[])
{
    bool res    = true;
    X509 * cert = nullptr;

    if (argc == 1)
    {
        gHelpOptions.PrintBriefUsage(stderr);
        ExitNow(res = true);
    }

    res = ParseArgs(CMD_NAME, argc, argv, gCmdOptionSets, HandleNonOptionArgs);
    VerifyTrueOrExit(res);

    res = InitOpenSSL();
    VerifyTrueOrExit(res);

    res = ReadCert(gInFileName, cert);
    VerifyTrueOrExit(res);

    res = WriteCert(gOutFileName, cert, gOutCertFormat);
    VerifyTrueOrExit(res);

exit:
    if (cert != nullptr)
    {
        X509_free(cert);
    }

    return res;
}

bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg)
{
    switch (id)
    {
    case 'p':
        gOutCertFormat = kCertFormat_X509_PEM;
        break;
    case 'x':
        gOutCertFormat = kCertFormat_X509_DER;
        break;
    case 'b':
        gOutCertFormat = kCertFormat_Chip_Base64;
        break;
    case 'c':
        gOutCertFormat = kCertFormat_Chip_Raw;
        break;
    default:
        PrintArgError("%s: Unhandled option: %s\n", progName, name);
        return false;
    }

    return true;
}

bool HandleNonOptionArgs(const char * progName, int argc, char * argv[])
{
    if (argc == 0)
    {
        PrintArgError("%s: Please specify the name of the input certificate file, or - for stdin.\n", progName);
        return false;
    }

    if (argc == 1)
    {
        PrintArgError("%s: Please specify the name of the output certificate file, or - for stdout\n", progName);
        return false;
    }

    if (argc > 2)
    {
        PrintArgError("%s: Unexpected argument: %s\n", progName, argv[2]);
        return false;
    }

    gInFileName  = argv[0];
    gOutFileName = argv[1];

    return true;
}
