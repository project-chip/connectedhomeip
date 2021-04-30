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
 *      that converts a CHIP private key between CHIP serialized and
 *      PEM/DER formats.
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

#define CMD_NAME "chip-cert convert-key"

static bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg);
static bool HandleNonOptionArgs(const char * progName, int argc, char * argv[]);

// clang-format off
static OptionDef gCmdOptionDefs[] =
{
    { "der",            kNoArgument, 'x' },
    { "pem",            kNoArgument, 'p' },
    { "chip",           kNoArgument, 'c' },
    { "chip-b64",       kNoArgument, 'b' },
    { "pkcs8-der",      kNoArgument, 'X' },
    { "pkcs8-pem",      kNoArgument, 'P' },
    { }
};

static const char * const gCmdOptionHelp =
    "   -p, --pem\n"
    "\n"
    "       Output the private key in SEC1/RFC-5915 PEM format.\n"
    "\n"
    "   -x, --der\n"
    "\n"
    "       Output the private key in SEC1/RFC-5915 DER format. \n"
    "\n"
    "   -c, --chip\n"
    "\n"
    "       Output the private key in raw CHIP serialized format.\n"
    "\n"
    "   -b, --chip-b64\n"
    "\n"
    "       Output the private key in base-64 encoded CHIP serialized format.\n"
    "       This is the default.\n"
    "\n"
    "   -P, --pkcs8-pem\n"
    "\n"
    "       Output the private key in PKCS#8 PEM format.\n"
    "\n"
    "   -X, --pkcs8-der\n"
    "\n"
    "       Output the private key in PKCS#8 DER format.\n"
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
    "Convert a private key between CHIP and PEM/DER forms."
    "\n"
    "ARGUMENTS\n"
    "\n"
    "   <in-file>\n"
    "\n"
    "       The input private key file name, or - to read from stdin. The\n"
    "       format of the input key is auto-detected and can be any\n"
    "       of: PEM, DER, CHIP base-64 or CHIP raw.\n"
    "\n"
    "   <out-file>\n"
    "\n"
    "       The output private key file name, or - to write to stdout.\n"
    "\n"
);

static OptionSet *gCmdOptionSets[] =
{
    &gCmdOptions,
    &gHelpOptions,
    nullptr
};
// clang-ormat on

const char * gInFileName = nullptr;
const char * gOutFileName = nullptr;
KeyFormat gOutFormat = kKeyFormat_Chip_Base64;

bool Cmd_ConvertKey(int argc, char * argv[])
{
    bool res       = true;
    EVP_PKEY * key = nullptr;

    if (argc == 1)
    {
        gHelpOptions.PrintBriefUsage(stderr);
        ExitNow(res = true);
    }

    res = ParseArgs(CMD_NAME, argc, argv, gCmdOptionSets, HandleNonOptionArgs);
    VerifyTrueOrExit(res);

    res = InitOpenSSL();
    VerifyTrueOrExit(res);

    res = ReadPrivateKey(gInFileName, key);
    VerifyTrueOrExit(res);

    res = WritePrivateKey(gOutFileName, key, gOutFormat);
    VerifyTrueOrExit(res);

exit:
    if (key != nullptr)
    {
        EVP_PKEY_free(key);
    }

    return res;
}

bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg)
{
    switch (id)
    {
    case 'p':
        gOutFormat = kKeyFormat_X509_PEM;
        break;
    case 'x':
        gOutFormat = kKeyFormat_X509_DER;
        break;
    case 'b':
        gOutFormat = kKeyFormat_Chip_Base64;
        break;
    case 'c':
        gOutFormat = kKeyFormat_Chip_Raw;
        break;
    case 'P':
        gOutFormat = kKeyFormat_X509_PEM_PKCS8;
        break;
    case 'X':
        gOutFormat = kKeyFormat_X509_DER_PKCS8;
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
        PrintArgError("%s: Please specify the name of the input key file, or - for stdin.\n", progName);
        return false;
    }

    if (argc == 1)
    {
        PrintArgError("%s: Please specify the name of the output key file, or - for stdout\n", progName);
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
