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
 *      This file implements the command handler for the 'spake2p' tool
 *      that generates Verifier.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include "spake2p.h"

#include <errno.h>

#include <CHIPVersion.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/Base64.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <protocols/secure_channel/PASESession.h>

namespace {

using namespace chip::ArgParser;

#define CMD_NAME "spake2p gen-verifier"

bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg);

// clang-format off
OptionDef gCmdOptionDefs[] =
{
    { "count",           kArgumentRequired, 'c' },
    { "pin-code",        kArgumentRequired, 'p' },
    { "iteration-count", kArgumentRequired, 'i' },
    { "salt-len",        kArgumentRequired, 'l' },
    { "salt",            kArgumentRequired, 's' },
    { "out",             kArgumentRequired, 'o' },
    { }
};

const char * const gCmdOptionHelp =
    "   -c, --count <int>\n"
    "\n"
    "       The number of pin-code/verifier parameter sets to be generated. If not specified,\n"
    "       one set will be generated.\n"
    "\n"
    "   -p, --pin-code <int>\n"
    "\n"
    "       SPAKE2P setup PIN code. The value should be positive integer in range [1..99999998].\n"
    "       If not specified, the PIN code value will be randomly generated.\n"
    "       When count is more than one, only first set will use the specified PIN code value\n"
    "       and others will be randomly generated.\n"
    "       The following PIN codes SHALL NOT be used due to their trivial, insecure nature:\n"
    "          * 00000000\n"
    "          * 11111111\n"
    "          * 22222222\n"
    "          * 33333333\n"
    "          * 44444444\n"
    "          * 55555555\n"
    "          * 66666666\n"
    "          * 77777777\n"
    "          * 88888888\n"
    "          * 99999999\n"
    "          * 12345678\n"
    "          * 87654321\n"
    "\n"
    "   -i, --iteration-count <int>\n"
    "\n"
    "       SPAKE2P PBKDF iteration count. The value should be positive integer in range [1000..100000].\n"
    "\n"
    "   -l, --salt-len <int>\n"
    "\n"
    "       SPAKE2P PBKDF salt input length. The value should be in range [16..32].\n"
    "       If not specified, the 'salt' input should be specified and the length will be\n"
    "       extracted from 'salt'. When both 'salt-len' and 'salt' are specified, the length\n"
    "       should match the length of the specified 'salt' string.\n"
    "\n"
    "   -s, --salt <string>\n"
    "\n"
    "       SPAKE2P PBKDF salt input value. Length of salt string should be in range [16..32].\n"
    "       If not specified, the 'salt-len' input should be specified and the 'salt' velue will be\n"
    "       randomly generated. When 'count' is more than one, only first set will use the specified\n"
    "       'salt' value and others will be randomly generated.\n"
    "\n"
    "   -o, --out <file>\n"
    "\n"
    "       File to contain the generated SPAKE2P PBKDF parameters. Specify '-' for stdout.\n"
    "       The format of the output file is:\n"
    "           Index,PIN Code,Iteration Count,Salt,Verifier\n"
    "           index of the parameter set in the list,'pin-code','iteration-count','salt'(Base-64 encoded),'verifier'(Base-64 encoded)\n"
    "           ....\n"
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
    "Usage: " CMD_NAME " [ <options...> ]\n",
    CHIP_VERSION_STRING "\n" COPYRIGHT_STRING,
    "Generate a CHIP certificate"
);

OptionSet *gCmdOptionSets[] =
{
    &gCmdOptions,
    &gHelpOptions,
    nullptr
};
// clang-format on

uint32_t gCount           = 1;
uint32_t gPinCode         = 0;
uint32_t gIterationCount  = 0;
uint8_t gSaltLen          = 0;
const char * gSalt        = nullptr;
const char * gOutFileName = nullptr;

bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg)
{
    switch (id)
    {
    case 'c':
        if (!ParseInt(arg, gCount) || gCount == 0)
        {
            PrintArgError("%s: Invalid value specified for parameter set count: %s\n", progName, arg);
            return false;
        }
        break;
    case 'p':
        // Specifications sections 5.1.1.6 and 5.1.6.1
        if (!ParseInt(arg, gPinCode) || (gPinCode > chip::kSetupPINCodeMaximumValue) || (gPinCode == 0) || (gPinCode == 11111111) ||
            (gPinCode == 22222222) || (gPinCode == 33333333) || (gPinCode == 44444444) || (gPinCode == 55555555) ||
            (gPinCode == 66666666) || (gPinCode == 77777777) || (gPinCode == 88888888) || (gPinCode == 99999999) ||
            (gPinCode == 12345678) || (gPinCode == 87654321))
        {
            PrintArgError("%s: Invalid value specified for pin-code parameter: %s\n", progName, arg);
            return false;
        }
        break;

    case 'i':
        if (!ParseInt(arg, gIterationCount) ||
            !(gIterationCount >= chip::kPBKDFMinimumIterations && gIterationCount <= chip::kPBKDFMaximumIterations))
        {
            PrintArgError("%s: Invalid value specified for the iteration-count parameter: %s\n", progName, arg);
            return false;
        }
        break;

    case 'l':
        if (!ParseInt(arg, gSaltLen) || !(gSaltLen >= chip::kPBKDFMinimumSaltLen && gSaltLen <= chip::kPBKDFMaximumSaltLen))
        {
            PrintArgError("%s: Invalid value specified for salt length parameter: %s\n", progName, arg);
            return false;
        }
        break;

    case 's':
        gSalt = arg;
        if (!(strlen(gSalt) >= chip::kPBKDFMinimumSaltLen && strlen(gSalt) <= chip::kPBKDFMaximumSaltLen))
        {
            fprintf(stderr, "%s: Invalid legth of the specified salt parameter: %s\n", progName, arg);
            return false;
        }
        break;

    case 'o':
        gOutFileName = arg;
        break;

    default:
        PrintArgError("%s: Unhandled option: %s\n", progName, name);
        return false;
    }

    return true;
}

} // namespace

bool Cmd_GenVerifier(int argc, char * argv[])
{
    FILE * outFile = NULL;

    if (argc == 1)
    {
        gHelpOptions.PrintBriefUsage(stderr);
        return true;
    }

    bool res = ParseArgs(CMD_NAME, argc, argv, gCmdOptionSets);
    VerifyOrReturnError(res, false);

    if (gIterationCount == 0)
    {
        fprintf(stderr, "Please specify the iteration-count parameter.\n");
        return false;
    }

    if (gSalt == nullptr && gSaltLen == 0)
    {
        fprintf(stderr, "Please specify at least one of the 'salt' or 'salt-len' parameters.\n");
        return false;
    }
    else if (gSalt != nullptr && gSaltLen != 0 && gSaltLen != strlen(gSalt))
    {
        fprintf(stderr, "The specified 'salt-len' doesn't match the length of 'salt' parameter.\n");
        return false;
    }
    else if (gSaltLen == 0)
    {
        gSaltLen = static_cast<uint8_t>(strlen(gSalt));
    }

    if (gOutFileName == nullptr)
    {
        fprintf(stderr, "Please specify the output file name, or - for stdout.\n");
        return false;
    }

    if (strcmp(gOutFileName, "-") != 0)
    {
        outFile = fopen(gOutFileName, "w+b");
        if (outFile == NULL)
        {
            fprintf(stderr, "Unable to create file %s\n%s\n", gOutFileName, strerror(errno));
            return false;
        }
    }
    else
    {
        outFile = stdout;
    }

    if (fprintf(outFile, "Index,PIN Code,Iteration Count,Salt,Verifier\n") < 0 || ferror(outFile))
    {
        fprintf(stderr, "Error writing to output file: %s\n", strerror(errno));
        return false;
    }

    for (uint32_t i = 0; i < gCount; i++)
    {
        uint8_t salt[chip::kPBKDFMaximumSaltLen];
        if (gSalt == nullptr)
        {
            CHIP_ERROR err = chip::Crypto::DRBG_get_bytes(salt, gSaltLen);
            if (err != CHIP_NO_ERROR)
            {
                fprintf(stderr, "DRBG_get_bytes() failed.\n");
                return false;
            }
        }
        else
        {
            memcpy(salt, gSalt, gSaltLen);
        }

        chip::PASEVerifier verifier;
        CHIP_ERROR err = chip::PASESession::GeneratePASEVerifier(verifier, gIterationCount, chip::ByteSpan(salt, gSaltLen),
                                                                 (gPinCode == 0), gPinCode);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "GeneratePASEVerifier() failed.\n");
            return false;
        }

        chip::PASEVerifierSerialized serializedVerifier;
        chip::MutableByteSpan serializedVerifierSpan(serializedVerifier);
        err = verifier.Serialize(serializedVerifierSpan);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "PASEVerifier::Serialize() failed.\n");
            return false;
        }

        char saltB64[BASE64_ENCODED_LEN(chip::kPBKDFMaximumSaltLen) + 1];
        uint32_t saltB64Len = chip::Base64Encode32(salt, gSaltLen, saltB64);
        saltB64[saltB64Len] = '\0';

        char verifierB64[BASE64_ENCODED_LEN(chip::kSpake2pSerializedVerifierSize) + 1];
        uint32_t verifierB64Len     = chip::Base64Encode32(serializedVerifier, chip::kSpake2pSerializedVerifierSize, verifierB64);
        verifierB64[verifierB64Len] = '\0';

        if (fprintf(outFile, "%d,%08d,%d,%s,%s\n", i, gPinCode, gIterationCount, saltB64, verifierB64) < 0 || ferror(outFile))
        {
            fprintf(stderr, "Error writing to output file: %s\n", strerror(errno));
            return false;
        }

        // On the next iteration the PIN Code and Salt will be randomly generated.
        gPinCode = 0;
        gSalt    = nullptr;
    }

    return true;
}
