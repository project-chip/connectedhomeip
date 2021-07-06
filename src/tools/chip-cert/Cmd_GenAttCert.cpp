/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      that generates attestation certificates.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include "chip-cert.h"

#include <support/SafeInt.h>

namespace {

using namespace chip::ArgParser;
using namespace chip::Credentials;
using namespace chip::ASN1;

#define CMD_NAME "chip-cert gen-att-cert"

bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg);

// clang-format off
OptionDef gCmdOptionDefs[] =
{
    { "type",             kArgumentRequired, 't' },
    { "subject-cn",       kArgumentRequired, 'c' },
    { "subject-vid",      kArgumentRequired, 'V' },
    { "subject-pid",      kArgumentRequired, 'P' },
    { "key",              kArgumentRequired, 'k' },
    { "ca-cert",          kArgumentRequired, 'C' },
    { "ca-key",           kArgumentRequired, 'K' },
    { "out",              kArgumentRequired, 'o' },
    { "out-key",          kArgumentRequired, 'O' },
    { "valid-from",       kArgumentRequired, 'f' },
    { "lifetime",         kArgumentRequired, 'l' },
    { }
};

const char * const gCmdOptionHelp =
    "   -t, --type <att-cert-type>\n"
    "\n"
    "       Attestation certificate type to be generated. Valid certificate type values are:\n"
    "           a - product attestation authority certificate\n"
    "           i - product attestation intermediate certificate\n"
    "           d - device attestation certificate\n"
    "\n"
    "   -c, --subject-cn <string>\n"
    "\n"
    "       Subject DN Common Name attribute encoded as UTF8String.\n"
    "\n"
    "   -V, --subject-vid <hex-digits>\n"
    "\n"
    "       Subject DN CHIP VID attribute (in hex).\n"
    "\n"
    "   -P, --subject-pid <hex-digits>\n"
    "\n"
    "       Subject DN CHIP PID attribute (in hex).\n"
    "\n"
    "   -C, --ca-cert <file>\n"
    "\n"
    "       File containing CA certificate to be used to sign the new certificate.\n"
    "\n"
    "   -K, --ca-key <file>\n"
    "\n"
    "       File containing CA private key to be used to sign the new certificate.\n"
    "\n"
    "   -k, --key <file>\n"
    "\n"
    "       File containing the public and private keys for the new certificate (in an X.509 PEM format).\n"
    "       If not specified, a new key pair will be generated.\n"
    "\n"
    "   -o, --out <file>\n"
    "\n"
    "       File to contain the new certificate (in an X.509 PEM format).\n"
    "\n"
    "   -O, --out-key <file>\n"
    "\n"
    "       File to contain the public/private key for the new certificate (in an X.509 PEM format).\n"
    "       This option must be specified if the --key option is not.\n"
    "\n"
    "   -f, --valid-from <YYYY>-<MM>-<DD> [ <HH>:<MM>:<SS> ]\n"
    "\n"
    "       The start date for the certificate's validity period. If not specified,\n"
    "       the validity period starts on the current day.\n"
    "\n"
    "   -l, --lifetime <days>\n"
    "\n"
    "       The lifetime for the new certificate, in whole days.\n"
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

AttCertType gAttCertType      = kAttCertType_NotSpecified;
const char * gSubjectCN       = nullptr;
uint16_t gSubjectVID          = 0;
uint16_t gSubjectPID          = 0;
const char * gCACertFileName  = nullptr;
const char * gCAKeyFileName   = nullptr;
const char * gInKeyFileName   = nullptr;
const char * gOutCertFileName = nullptr;
const char * gOutKeyFileName  = nullptr;
uint32_t gValidDays           = 0;
struct tm gValidFrom;

bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg)
{
    uint64_t chip64bitAttr;

    switch (id)
    {
    case 't':
        if (strlen(arg) == 1)
        {
            if (*arg == 'd')
            {
                gAttCertType = kAttCertType_DAC;
            }
            else if (*arg == 'i')
            {
                gAttCertType = kAttCertType_PAI;
            }
            else if (*arg == 'a')
            {
                gAttCertType = kAttCertType_PAA;
            }
        }

        if (gAttCertType == kAttCertType_NotSpecified)
        {
            PrintArgError("%s: Invalid value specified for the attestation certificate type: %s\n", progName, arg);
            return false;
        }
        break;
    case 'c':
        gSubjectCN = arg;
        break;
    case 'V':
        if (!ParseChip64bitAttr(arg, chip64bitAttr) || !chip::CanCastTo<uint16_t>(chip64bitAttr))
        {
            PrintArgError("%s: Invalid value specified for the subject VID attribute: %s\n", progName, arg);
            return false;
        }
        gSubjectVID = static_cast<uint16_t>(chip64bitAttr);
        break;
    case 'P':
        if (!ParseChip64bitAttr(arg, chip64bitAttr) || !chip::CanCastTo<uint16_t>(chip64bitAttr))
        {
            PrintArgError("%s: Invalid value specified for the subject PID attribute: %s\n", progName, arg);
            return false;
        }
        gSubjectPID = static_cast<uint16_t>(chip64bitAttr);
        break;
    case 'k':
        gInKeyFileName = arg;
        break;
    case 'C':
        gCACertFileName = arg;
        break;
    case 'K':
        gCAKeyFileName = arg;
        break;
    case 'o':
        gOutCertFileName = arg;
        break;
    case 'O':
        gOutKeyFileName = arg;
        break;
    case 'f':
        if (!ParseDateTime(arg, gValidFrom))
        {
            PrintArgError("%s: Invalid value specified for certificate validity date: %s\n", progName, arg);
            return false;
        }
        break;
    case 'l':
        if (!ParseInt(arg, gValidDays))
        {
            PrintArgError("%s: Invalid value specified for certificate lifetime: %s\n", progName, arg);
            return false;
        }
        break;
    default:
        PrintArgError("%s: Unhandled option: %s\n", progName, name);
        return false;
    }

    return true;
}

} // namespace

bool Cmd_GenAttCert(int argc, char * argv[])
{
    bool res = true;
    std::unique_ptr<X509, void (*)(X509 *)> newCert(X509_new(), &X509_free);
    std::unique_ptr<EVP_PKEY, void (*)(EVP_PKEY *)> newKey(EVP_PKEY_new(), &EVP_PKEY_free);

    {
        time_t now         = time(nullptr);
        gValidFrom         = *gmtime(&now);
        gValidFrom.tm_hour = 0;
        gValidFrom.tm_min  = 0;
        gValidFrom.tm_sec  = 0;
    }

    if (argc == 1)
    {
        gHelpOptions.PrintBriefUsage(stderr);
        return true;
    }

    res = ParseArgs(CMD_NAME, argc, argv, gCmdOptionSets);
    VerifyTrueOrExit(res);

    if (gAttCertType == kAttCertType_NotSpecified)
    {
        fprintf(stderr, "Please specify attestation certificate type.\n");
        return false;
    }
    else if (gAttCertType == kAttCertType_DAC)
    {
        if (gSubjectVID == 0 || gSubjectPID == 0)
        {
            fprintf(stderr, "Please specify VID and PID subject DN attributes.\n");
            return false;
        }
    }
    else if (gAttCertType == kAttCertType_PAI)
    {
        if (gSubjectVID == 0)
        {
            fprintf(stderr, "Please specify VID subject DN attributes.\n");
            return false;
        }
    }
    else if (gAttCertType == kAttCertType_PAA)
    {
        if (gSubjectVID != 0 || gSubjectPID != 0)
        {
            fprintf(stderr, "VID & PID SHALL NOT specify subject DN attributes.\n");
            return false;
        }
    }

    if (gCACertFileName == nullptr && gAttCertType != kAttCertType_PAA)
    {
        fprintf(stderr, "Please specify the CA certificate file name using the --ca-cert option.\n");
        return false;
    }
    else if (gCACertFileName != nullptr && gAttCertType == kAttCertType_PAA)
    {
        fprintf(stderr, "Please don't specify --ca-cert option for the self signed certificate. \n");
        return false;
    }

    if (gCACertFileName != nullptr && gCAKeyFileName == nullptr)
    {
        fprintf(stderr, "Please specify the CA key file name using the --ca-key option.\n");
        return false;
    }

    if (gOutCertFileName == nullptr)
    {
        fprintf(stderr, "Please specify the file name for the new certificate using the --out option.\n");
        return false;
    }

    if (gInKeyFileName == nullptr && gOutKeyFileName == nullptr)
    {
        fprintf(stderr, "Please specify the file name for the new public/private key using the --out-key option.\n");
        return false;
    }

    if (gValidDays == 0)
    {
        fprintf(stderr, "Please specify the lifetime (in dys) for the new certificate using the --lifetime option.\n");
        return false;
    }

    if (access(gOutCertFileName, R_OK) == 0)
    {
        fprintf(stderr,
                "Output certificate file already exists (%s)\n"
                "To replace the file, please remove it and re-run the command.\n",
                gOutCertFileName);
        return false;
    }

    if (gOutKeyFileName != nullptr && access(gOutKeyFileName, R_OK) == 0)
    {
        fprintf(stderr,
                "Output key file already exists (%s)\n"
                "To replace the file, please remove it and re-run the command.\n",
                gOutKeyFileName);
        return false;
    }

    res = InitOpenSSL();
    VerifyTrueOrExit(res);

    if (gInKeyFileName != nullptr)
    {
        res = ReadKey(gInKeyFileName, newKey.get());
        VerifyTrueOrExit(res);
    }
    else
    {
        res = GenerateKeyPair(newKey.get());
        VerifyTrueOrExit(res);
    }

    if (gAttCertType == kAttCertType_PAA)
    {
        res = MakeAttCert(gAttCertType, gSubjectCN, gSubjectVID, gSubjectPID, newCert.get(), newKey.get(), gValidFrom, gValidDays,
                          newCert.get(), newKey.get());
        VerifyTrueOrExit(res);
    }
    else
    {
        std::unique_ptr<X509, void (*)(X509 *)> caCert(X509_new(), &X509_free);
        std::unique_ptr<EVP_PKEY, void (*)(EVP_PKEY *)> caKey(EVP_PKEY_new(), &EVP_PKEY_free);

        res = ReadCert(gCACertFileName, caCert.get());
        VerifyTrueOrExit(res);

        res = ReadKey(gCAKeyFileName, caKey.get());
        VerifyTrueOrExit(res);

        res = MakeAttCert(gAttCertType, gSubjectCN, gSubjectVID, gSubjectPID, caCert.get(), caKey.get(), gValidFrom, gValidDays,
                          newCert.get(), newKey.get());
        VerifyTrueOrExit(res);
    }

    res = WriteCert(gOutCertFileName, newCert.get(), kCertFormat_X509_PEM);
    VerifyTrueOrExit(res);

    if (gOutKeyFileName != nullptr)
    {
        res = WritePrivateKey(gOutKeyFileName, newKey.get(), kKeyFormat_X509_PEM);
        VerifyTrueOrExit(res);
    }

exit:
    return res;
}
