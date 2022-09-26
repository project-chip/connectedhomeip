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
 *      that validates a CHIP certificate.
 *
 */

#include "chip-cert.h"
#include <credentials/CHIPCertificateSet.h>

#include "vector"

namespace {

using namespace chip;
using namespace chip::ArgParser;
using namespace chip::Credentials;
using namespace chip::ASN1;

#define CMD_NAME "chip-cert validate-cert"

bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg);
bool HandleNonOptionArgs(const char * progName, int argc, char * const argv[]);

// clang-format off
OptionDef gCmdOptionDefs[] =
{
    { "cert",           kArgumentRequired,  'c' },
    { "trusted-cert",   kArgumentRequired,  't' },
    { }
};

const char * const gCmdOptionHelp =
    "  -c, --cert <file/str>\n"
    "\n"
    "       File or string containing an untrusted CHIP certificate to be used during\n"
    "       validation. Usually, it is Intermediate CA certificate (ICAC).\n"
    "\n"
    "  -t, --trusted-cert <file/str>\n"
    "\n"
    "       File or string containing a trusted CHIP certificate to be used during\n"
    "       validation. Usually, it is trust anchor root certificate (RCAC).\n"
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
    "Usage: " CMD_NAME " [ <options...> ] <file/str>\n",
    CHIP_VERSION_STRING "\n" COPYRIGHT_STRING,
    "Validate a chain of CHIP certificates.\n"
    "\n"
    "ARGUMENTS\n"
    "\n"
    "  <file/str>\n"
    "\n"
    "      File or string containing the certificate to be validated.\n"
    "      The formats of all input certificates are auto-detected and can be any of:\n"
    "      X.509 PEM, X.509 DER, X.509 HEX, CHIP base-64, CHIP raw TLV or CHIP HEX.\n"
    "\n"
);

OptionSet * gCmdOptionSets[] =
{
    &gCmdOptions,
    &gHelpOptions,
    nullptr
};
// clang-format on

enum
{
    kMaxCerts = 16,
};

const char * gTargetCertFileName         = nullptr;
const char * gCACertFileNames[kMaxCerts] = { nullptr };
bool gCACertIsTrusted[kMaxCerts]         = { false };
size_t gNumCertFileNames                 = 0;

bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg)
{
    switch (id)
    {
    case 'c':
    case 't':
        if (gNumCertFileNames >= kMaxCerts)
        {
            PrintArgError("%s: Too many certificate files\n", progName);
            return false;
        }
        gCACertFileNames[gNumCertFileNames]   = arg;
        gCACertIsTrusted[gNumCertFileNames++] = (id == 't');
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
        PrintArgError("%s: Please specify the name of the certificate to be validated.\n", progName);
        return false;
    }

    if (argc > 1)
    {
        PrintArgError("%s: Unexpected argument: %s\n", progName, argv[1]);
        return false;
    }

    gTargetCertFileName = argv[0];

    return true;
}

} // namespace

bool Cmd_ValidateCert(int argc, char * argv[])
{
    bool res       = true;
    CHIP_ERROR err = CHIP_NO_ERROR;
    ChipCertificateSet certSet;
    const ChipCertificateData * certToBeValidated = nullptr;
    const ChipCertificateData * validatedCert     = nullptr;
    ValidationContext context;
    uint8_t certsBuf[kMaxCerts * kMaxCHIPCertLength];
    MutableByteSpan chipCert[kMaxCerts];

    context.Reset();

    if (argc == 1)
    {
        gHelpOptions.PrintBriefUsage(stderr);
        ExitNow(res = true);
    }

    res = ParseArgs(CMD_NAME, argc, argv, gCmdOptionSets, HandleNonOptionArgs);
    VerifyTrueOrExit(res);

    err = certSet.Init(kMaxCerts);
    if (err != CHIP_NO_ERROR)
    {
        fprintf(stderr, "Failed to initialize certificate set: %s\n", chip::ErrorStr(err));
        ExitNow(res = false);
    }

    for (size_t i = 0; i < gNumCertFileNames; i++)
    {
        chipCert[i] = MutableByteSpan(&certsBuf[i * kMaxCHIPCertLength], kMaxCHIPCertLength);
        res         = LoadChipCert(gCACertFileNames[i], gCACertIsTrusted[i], certSet, chipCert[i]);
        VerifyTrueOrExit(res);
    }

    chipCert[gNumCertFileNames] = MutableByteSpan(&certsBuf[gNumCertFileNames * kMaxCHIPCertLength], kMaxCHIPCertLength);
    res                         = LoadChipCert(gTargetCertFileName, false, certSet, chipCert[gNumCertFileNames]);
    VerifyTrueOrExit(res);

    certToBeValidated = certSet.GetLastCert();

    context.Reset();
    uint32_t currentTime;
    res = chip::UnixEpochToChipEpochTime(static_cast<uint32_t>(time(nullptr)), currentTime);
    context.mEffectiveTime.Set<CurrentChipEpochTime>(currentTime);
    context.mRequiredKeyUsages.Set(KeyUsageFlags::kDigitalSignature);
    VerifyTrueOrExit(res);

    err = certSet.FindValidCert(certToBeValidated->mSubjectDN, certToBeValidated->mSubjectKeyId, context, &validatedCert);
    if (err != CHIP_NO_ERROR)
    {
        fprintf(stderr, "Failed certificate chain validation: %s\n", chip::ErrorStr(err));
        ExitNow(res = false);
    }

exit:
    certSet.Release();
    return res;
}
