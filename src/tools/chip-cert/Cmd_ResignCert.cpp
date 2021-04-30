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
 *      that re-signs a CHIP certificate.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include <ctype.h>
#include <getopt.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "chip-cert.h"

using namespace chip::ArgParser;
using namespace chip::Credentials;
using namespace chip::ASN1;

#define CMD_NAME "chip-cert resign-cert"

static bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg);

// clang-format off
static OptionDef gCmdOptionDefs[] =
{
    { "cert",       kArgumentRequired, 'c' },
    { "out",        kArgumentRequired, 'o' },
    { "ca-cert",    kArgumentRequired, 'C' },
    { "ca-key",     kArgumentRequired, 'K' },
    { "self",       kNoArgument,       's' },
    { }
};

static const char * const gCmdOptionHelp =
    "  -c, --cert <file>\n"
    "\n"
    "       File containing the certificate to be re-signed.\n"
    "\n"
    "  -o, --out <file>\n"
    "\n"
    "       File to contain the re-signed certificate.\n"
    "\n"
    "  -C, --ca-cert <file>\n"
    "\n"
    "       File containing CA certificate to be used to re-sign the certificate.\n"
    "\n"
    "  -K, --ca-key <file>\n"
    "\n"
    "       File containing CA private key to be used to re-sign the certificate.\n"
    "\n"
    "  -s, --self\n"
    "\n"
    "       Generate a self-signed certificate.\n"
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
    "Usage: " CMD_NAME " [ <options...> ]\n",
    CHIP_VERSION_STRING "\n" COPYRIGHT_STRING,
    "Resign a CHIP certificate using a new CA certificate/key."
);

static OptionSet * gCmdOptionSets[] =
{
    &gCmdOptions,
    &gHelpOptions,
    nullptr
};
// clang-format on

static const char * gInCertFileName  = nullptr;
static const char * gOutCertFileName = nullptr;
static const char * gCACertFileName  = nullptr;
static const char * gCAKeyFileName   = nullptr;
static bool gSelfSign                = false;

bool Cmd_ResignCert(int argc, char * argv[])
{
    bool res         = true;
    X509 * caCert    = nullptr;
    X509 * cert      = nullptr;
    EVP_PKEY * caKey = nullptr;
    CertFormat inCertFmt;

    if (argc == 1)
    {
        gHelpOptions.PrintBriefUsage(stderr);
        ExitNow(res = true);
    }

    res = ParseArgs(CMD_NAME, argc, argv, gCmdOptionSets);
    VerifyTrueOrExit(res);

    if (gInCertFileName == nullptr)
    {
        fprintf(stderr, "Please specify certificate to be resigned using --cert option.\n");
        ExitNow(res = false);
    }

    if (gOutCertFileName == nullptr)
    {
        fprintf(stderr, "Please specify the file name for the new certificate using the --out option.\n");
        ExitNow(res = false);
    }

    if (gCACertFileName == nullptr && !gSelfSign)
    {
        fprintf(stderr,
                "Please specify a CA certificate to be used to sign the new certificate (using\n"
                "the --ca-cert option) or --self to generate a self-signed certificate.\n");
        ExitNow(res = false);
    }
    else if (gCACertFileName != nullptr && gSelfSign)
    {
        fprintf(stderr, "Please specify only one of --ca-cert and --self.\n");
        ExitNow(res = false);
    }

    if (gCAKeyFileName == nullptr)
    {
        fprintf(stderr, "Please specify the CA key file name using the --ca-key option.\n");
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

    res = ReadCert(gInCertFileName, cert, inCertFmt);
    VerifyTrueOrExit(res);

    if (!gSelfSign)
    {
        res = ReadCert(gCACertFileName, caCert);
        VerifyTrueOrExit(res);
    }
    else
    {
        caCert = cert;
    }

    res = ReadPrivateKey(gCAKeyFileName, caKey);
    VerifyTrueOrExit(res);

    res = ResignCert(cert, caCert, caKey);
    VerifyTrueOrExit(res);

    res = WriteCert(gOutCertFileName, cert, inCertFmt);
    VerifyTrueOrExit(res);

exit:
    if (cert != nullptr)
    {
        X509_free(cert);
    }
    if (caCert != nullptr && caCert != cert)
    {
        X509_free(caCert);
    }
    if (caKey != nullptr)
    {
        EVP_PKEY_free(caKey);
    }

    return res;
}

bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg)
{
    switch (id)
    {
    case 'c':
        gInCertFileName = arg;
        break;
    case 'o':
        gOutCertFileName = arg;
        break;
    case 'C':
        gCACertFileName = arg;
        break;
    case 'K':
        gCAKeyFileName = arg;
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
