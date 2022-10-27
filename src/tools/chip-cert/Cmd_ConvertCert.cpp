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
 *      This file implements the command handler for the 'chip-cert' tool
 *      that converts a CHIP certificate between CHIP TLV and X.509
 *      formats.
 *
 */

#include "chip-cert.h"

namespace {

using namespace chip::ArgParser;
using namespace chip::Credentials;

#define CMD_NAME "chip-cert convert-cert"

bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg);
bool HandleNonOptionArgs(const char * progName, int argc, char * const argv[]);

// clang-format off
OptionDef gCmdOptionDefs[] =
{
    { "x509-pem",   kNoArgument, 'p' },
    { "x509-der",   kNoArgument, 'd' },
    { "x509-hex",   kNoArgument, 'X' },
    { "chip",       kNoArgument, 'c' },
    { "chip-hex",   kNoArgument, 'x' },
    { "chip-b64",   kNoArgument, 'b' },
    { }
};

const char * const gCmdOptionHelp =
    "  -p, --x509-pem\n"
    "\n"
    "       Output certificate in X.509 PEM format.\n"
    "\n"
    "  -d, --x509-der\n"
    "\n"
    "       Output certificate in X.509 DER format.\n"
    "\n"
    "  -X, --x509-hex\n"
    "\n"
    "       Output certificate in X.509 DER hex encoded format.\n"
    "\n"
    "  -c, --chip\n"
    "\n"
    "       Output certificate in raw CHIP TLV format.\n"
    "\n"
    "  -x, --chip-hex\n"
    "\n"
    "       Output certificate in CHIP TLV hexadecimal format.\n"
    "\n"
    "  -b --chip-b64\n"
    "\n"
    "       Output certificate in CHIP TLV base-64 encoded format.\n"
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
    "Usage: " CMD_NAME " [ <options...> ] <in-file/str> <out-file/stdout>\n",
    CHIP_VERSION_STRING "\n" COPYRIGHT_STRING,
    "Convert operational certificate between CHIP and X.509 formats.\n"
    "\n"
    "ARGUMENTS\n"
    "\n"
    "  <in-file/str>\n"
    "\n"
    "       File or string containing certificate to be converted.\n"
    "       The format of the input certificate is auto-detected and can be any of:\n"
    "       X.509 PEM, X.509 DER, X.509 HEX, CHIP base-64, CHIP raw TLV or CHIP HEX.\n"
    "\n"
    "  <out-file/stdout>\n"
    "\n"
    "       The output certificate file name, or '-' to write to stdout.\n"
    "\n"
);

OptionSet * gCmdOptionSets[] =
{
    &gCmdOptions,
    &gHelpOptions,
    nullptr
};
// clang-format on

const char * gInFileNameOrStr = nullptr;
const char * gOutFileName     = nullptr;
CertFormat gOutCertFormat     = kCertFormat_Default;

bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg)
{
    switch (id)
    {
    case 'p':
        gOutCertFormat = kCertFormat_X509_PEM;
        break;
    case 'd':
        gOutCertFormat = kCertFormat_X509_DER;
        break;
    case 'X':
        gOutCertFormat = kCertFormat_X509_Hex;
        break;
    case 'x':
        gOutCertFormat = kCertFormat_Chip_Hex;
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

bool HandleNonOptionArgs(const char * progName, int argc, char * const argv[])
{
    if (argc == 0)
    {
        PrintArgError("%s: Please specify the name of the input certificate file or the certificate string.\n", progName);
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

    gInFileNameOrStr = argv[0];
    gOutFileName     = argv[1];

    return true;
}

} // namespace

bool Cmd_ConvertCert(int argc, char * argv[])
{
    bool res = true;
    std::unique_ptr<X509, void (*)(X509 *)> cert(X509_new(), &X509_free);

    if (argc == 1)
    {
        gHelpOptions.PrintBriefUsage(stderr);
        ExitNow(res = true);
    }

    res = ParseArgs(CMD_NAME, argc, argv, gCmdOptionSets, HandleNonOptionArgs);
    VerifyTrueOrExit(res);

    res = InitOpenSSL();
    VerifyTrueOrExit(res);

    res = ReadCert(gInFileNameOrStr, cert.get());
    VerifyTrueOrExit(res);

    res = WriteCert(gOutFileName, cert.get(), gOutCertFormat);
    VerifyTrueOrExit(res);

exit:
    return res;
}
