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
 *      that re-signs a CHIP certificate.
 *
 */

#include "chip-cert.h"

namespace {

using namespace chip::ArgParser;
using namespace chip::Credentials;
using namespace chip::ASN1;

#define CMD_NAME "chip-cert resign-cert"

bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg);

// clang-format off
OptionDef gCmdOptionDefs[] =
{
    { "cert",       kArgumentRequired, 'c' },
    { "out",        kArgumentRequired, 'o' },
    { "ca-cert",    kArgumentRequired, 'C' },
    { "ca-key",     kArgumentRequired, 'K' },
    { "self",       kNoArgument,       's' },
    { }
};

const char * const gCmdOptionHelp =
    "  -c, --cert <file/str>\n"
    "\n"
    "       File or string containing the certificate to be re-signed.\n"
    "\n"
    "  -o, --out <file/stdout>\n"
    "\n"
    "       File to contain the re-signed certificate.\n"
    "       If specified '-' then output is written to stdout.\n"
    "\n"
    "  -C, --ca-cert <file/str>\n"
    "\n"
    "       File or string containing CA certificate to be used to re-sign the certificate.\n"
    "\n"
    "  -K, --ca-key <file/str>\n"
    "\n"
    "       File or string containing CA private key to be used to re-sign the certificate.\n"
    "\n"
    "  -s, --self\n"
    "\n"
    "       Generate a self-signed certificate.\n"
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
    "Resign a CHIP certificate using a new CA certificate/key."
);

OptionSet * gCmdOptionSets[] =
{
    &gCmdOptions,
    &gHelpOptions,
    nullptr
};
// clang-format on

const char * gInCertFileNameOrStr = nullptr;
const char * gOutCertFileName     = nullptr;
const char * gCACertFileNameOrStr = nullptr;
const char * gCAKeyFileNameOrStr  = nullptr;
bool gSelfSign                    = false;

bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg)
{
    switch (id)
    {
    case 'c':
        gInCertFileNameOrStr = arg;
        break;
    case 'o':
        gOutCertFileName = arg;
        break;
    case 'C':
        gCACertFileNameOrStr = arg;
        break;
    case 'K':
        gCAKeyFileNameOrStr = arg;
        break;
    case 's':
        gSelfSign = true;
        break;
    default:
        PrintArgError("%s: Unhandled option: %s\n", progName, name);
        return false;
    }

    return true;
}

} // namespace

bool Cmd_ResignCert(int argc, char * argv[])
{
    bool res = true;
    CertFormat inCertFmt;
    std::unique_ptr<X509, void (*)(X509 *)> cert(nullptr, &X509_free);
    std::unique_ptr<EVP_PKEY, void (*)(EVP_PKEY *)> caKey(EVP_PKEY_new(), &EVP_PKEY_free);

    if (argc == 1)
    {
        gHelpOptions.PrintBriefUsage(stderr);
        ExitNow(res = true);
    }

    res = ParseArgs(CMD_NAME, argc, argv, gCmdOptionSets);
    VerifyTrueOrExit(res);

    if (gInCertFileNameOrStr == nullptr)
    {
        fprintf(stderr, "Please specify certificate to be resigned using --cert option.\n");
        ExitNow(res = false);
    }

    if (gOutCertFileName == nullptr)
    {
        fprintf(stderr, "Please specify the file name for the new certificate using the --out option.\n");
        ExitNow(res = false);
    }

    if (gCACertFileNameOrStr == nullptr && !gSelfSign)
    {
        fprintf(stderr,
                "Please specify a CA certificate to be used to sign the new certificate (using\n"
                "the --ca-cert option) or --self to generate a self-signed certificate.\n");
        ExitNow(res = false);
    }
    else if (gCACertFileNameOrStr != nullptr && gSelfSign)
    {
        fprintf(stderr, "Please specify only one of --ca-cert and --self.\n");
        ExitNow(res = false);
    }

    if (gCAKeyFileNameOrStr == nullptr)
    {
        fprintf(stderr, "Please specify the CA key using the --ca-key option.\n");
        ExitNow(res = false);
    }

    if (access(gOutCertFileName, R_OK) == 0)
    {
        fprintf(stderr,
                "Output certificate file already exists (%s)\n"
                "To replace the file, please remove it and re-run the command.\n",
                gOutCertFileName);
        ExitNow(res = false);
    }

    res = InitOpenSSL();
    VerifyTrueOrExit(res);

    res = ReadCert(gInCertFileNameOrStr, cert, inCertFmt);
    VerifyTrueOrExit(res);

    res = ReadKey(gCAKeyFileNameOrStr, caKey);
    VerifyTrueOrExit(res);

    if (!gSelfSign)
    {
        std::unique_ptr<X509, void (*)(X509 *)> caCert(nullptr, &X509_free);

        res = ReadCert(gCACertFileNameOrStr, caCert);
        VerifyTrueOrExit(res);

        res = ResignCert(cert.get(), caCert.get(), caKey.get());
        VerifyTrueOrExit(res);
    }
    else
    {
        res = ResignCert(cert.get(), cert.get(), caKey.get());
        VerifyTrueOrExit(res);
    }

    res = WriteCert(gOutCertFileName, cert.get(), inCertFmt);
    VerifyTrueOrExit(res);

exit:
    return res;
}
