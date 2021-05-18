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

#include "chip-cert.h"

namespace {

using namespace chip::ArgParser;
using namespace chip::Credentials;

#define CMD_NAME "chip-cert convert-key"

bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg);
bool HandleNonOptionArgs(const char * progName, int argc, char * argv[]);

// clang-format off
OptionDef gCmdOptionDefs[] =
{
    { "x509-pem",       kNoArgument, 'p' },
    { "x509-der",       kNoArgument, 'x' },
    { "chip",           kNoArgument, 'c' },
    { "chip-b64",       kNoArgument, 'b' },
    { }
};

const char * const gCmdOptionHelp =
    "   -p, --x509-pem\n"
    "\n"
    "       Output the private key in SEC1/RFC-5915 PEM format.\n"
    "\n"
    "   -x, --x509-der\n"
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
    ;

OptionSet gCmdOptions =
{
    HandleOption,
    gCmdOptionDefs,
    "COMMAND OPTIONS",
    gCmdOptionHelp
};

HelpOptions gHelpOptions(
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

OptionSet *gCmdOptionSets[] =
{
    &gCmdOptions,
    &gHelpOptions,
    nullptr
};
// clang-ormat on

const char * gInFileName = nullptr;
const char * gOutFileName = nullptr;
KeyFormat gOutFormat = kKeyFormat_Chip_Base64;


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

} // namespace

bool Cmd_ConvertKey(int argc, char * argv[])
{
    bool res       = true;
    std::unique_ptr<EVP_PKEY,void(*)(EVP_PKEY*)> key(EVP_PKEY_new(), &EVP_PKEY_free);

    if (argc == 1)
    {
        gHelpOptions.PrintBriefUsage(stderr);
        ExitNow(res = true);
    }

    res = ParseArgs(CMD_NAME, argc, argv, gCmdOptionSets, HandleNonOptionArgs);
    VerifyTrueOrExit(res);

    res = InitOpenSSL();
    VerifyTrueOrExit(res);

    res = ReadKey(gInFileName, key.get());
    VerifyTrueOrExit(res);

    res = WritePrivateKey(gOutFileName, key.get(), gOutFormat);
    VerifyTrueOrExit(res);

exit:
    return res;
}
