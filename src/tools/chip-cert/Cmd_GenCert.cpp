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
 *      that generates a CHIP certificate.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include "chip-cert.h"

namespace {

using namespace chip::ArgParser;
using namespace chip::Credentials;
using namespace chip::ASN1;

#define CMD_NAME "chip-cert gen-cert"

bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg);

// clang-format off
OptionDef gCmdOptionDefs[] =
{
    { "type",             kArgumentRequired, 't' },
    { "subject-chip-id",  kArgumentRequired, 'i' },
    { "subject-fab-id",   kArgumentRequired, 'f' },
    { "subject-at",       kArgumentRequired, 'a' },
    { "subject-cn-u",     kArgumentRequired, 'c' },
    { "future-ext-sub",   kArgumentRequired, 'x' },
    { "future-ext-info",  kArgumentRequired, '2' },
    { "key",              kArgumentRequired, 'k' },
    { "ca-cert",          kArgumentRequired, 'C' },
    { "ca-key",           kArgumentRequired, 'K' },
    { "out",              kArgumentRequired, 'o' },
    { "out-key",          kArgumentRequired, 'O' },
    { "out-format",       kArgumentRequired, 'F' },
    { "valid-from",       kArgumentRequired, 'V' },
    { "lifetime",         kArgumentRequired, 'l' },
    { }
};

const char * const gCmdOptionHelp =
    "   -t, --type <cert-type>\n"
    "\n"
    "       Certificate type to be generated. Valid certificate type values are:\n"
    "           r - root certificate\n"
    "           c - CA certificate\n"
    "           n - node certificate\n"
    "           f - firmware signing certificate\n"
    "\n"
    "   -i, --subject-chip-id <hex-digits>\n"
    "\n"
    "       Subject DN CHIP Id attribute (in hex). For Node Certificate it is CHIP Node Id attribute.\n"
    "          - for Root certificate it is ChipRootId\n"
    "          - for intermediate CA certificate it is ChipICAId\n"
    "          - for Node certificate it is ChipNodeId\n"
    "          - for Firmware Signing certificate it is ChipFirmwareSigningId\n"
    "\n"
    "   -f, --subject-fab-id <hex-digits>\n"
    "\n"
    "       Subject DN Fabric Id attribute (in hex).\n"
    "\n"
    "   -a, --subject-at <hex-digits>\n"
    "\n"
    "       Subject DN CHIP Authentication Tag (in hex).\n"
    "\n"
    "   -c, --subject-cn-u <string>\n"
    "\n"
    "       Subject DN Common Name attribute encoded as UTF8String.\n"
    "\n"
    "   -x, --future-ext-sub <string>\n"
    "\n"
    "       NID_subject_alt_name extension to be added to the list of certificate extensions.\n"
    "\n"
    "   -2, --future-ext-info <string>\n"
    "\n"
    "       NID_info_access extension to be added to the list of certificate extensions.\n"
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
    "       File containing the public and private keys for the new certificate.\n"
    "       If not specified, a new key pair will be generated.\n"
    "\n"
    "   -o, --out <file>\n"
    "\n"
    "       File to contain the new certificate.\n"
    "\n"
    "   -O, --out-key <file>\n"
    "\n"
    "       File to contain the public/private key for the new certificate.\n"
    "       This option must be specified if the --key option is not.\n"
    "\n"
    "  -F, --out-format <format>\n"
    "\n"
    "       Specifies format of the output certificate and private key.\n"
    "       If not specified, the default base-64 encoded CHIP format is used.\n"
    "       Supported format parametes are:\n"
    "           x509-pem  - X.509 PEM format\n"
    "           x509-der  - X.509 DER format\n"
    "           chip      - raw CHIP TLV format\n"
    "           chip-b64  - base-64 encoded CHIP TLV format (default)\n"
    "\n"
    "   -V, --valid-from <YYYY>-<MM>-<DD> [ <HH>:<MM>:<SS> ]\n"
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

ToolChipDN gSubjectDN;
uint8_t gCertType                    = kCertType_NotSpecified;
bool gSelfSign                       = false;
const char * gCACertFileName         = nullptr;
const char * gCAKeyFileName          = nullptr;
const char * gInKeyFileName          = nullptr;
const char * gOutCertFileName        = nullptr;
const char * gOutKeyFileName         = nullptr;
CertFormat gOutCertFormat            = kCertFormat_Chip_Base64;
KeyFormat gOutKeyFormat              = kKeyFormat_Chip_Base64;
uint32_t gValidDays                  = 0;
FutureExtension gFutureExtensions[3] = { { 0, nullptr } };
uint8_t gFutureExtensionsCount       = 0;
struct tm gValidFrom;

bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint64_t chip64bitAttr;
    OID attrOID;

    switch (id)
    {
    case 't':
        if (strlen(arg) == 1)
        {
            if (*arg == 'n')
            {
                gCertType = kCertType_Node;
            }
            else if (*arg == 'f')
            {
                gCertType = kCertType_FirmwareSigning;
            }
            else if (*arg == 'c')
            {
                gCertType = kCertType_ICA;
            }
            else if (*arg == 'r')
            {
                gCertType = kCertType_Root;
                gSelfSign = true;
            }
        }

        if (gCertType == kCertType_NotSpecified)
        {
            PrintArgError("%s: Invalid value specified for the certificate type: %s\n", progName, arg);
            return false;
        }
        break;

    case 'i':
        if (!ParseChip64bitAttr(arg, chip64bitAttr))
        {
            PrintArgError("%s: Invalid value specified for subject chip id attribute: %s\n", progName, arg);
            return false;
        }

        switch (gCertType)
        {
        case kCertType_Node:
            attrOID = kOID_AttributeType_ChipNodeId;
            break;
        case kCertType_FirmwareSigning:
            attrOID = kOID_AttributeType_ChipFirmwareSigningId;
            break;
        case kCertType_ICA:
            attrOID = kOID_AttributeType_ChipICAId;
            break;
        case kCertType_Root:
            attrOID = kOID_AttributeType_ChipRootId;
            break;
        default:
            PrintArgError("%s: Certificate type argument should be specified prior to subject attribute: %s\n", progName, arg);
            return false;
        }

        err = gSubjectDN.AddAttribute(attrOID, chip64bitAttr);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add subject DN attribute: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;

    case 'a':
        if (!ParseChip64bitAttr(arg, chip64bitAttr))
        {
            PrintArgError("%s: Invalid value specified for the subject authentication tag attribute: %s\n", progName, arg);
            return false;
        }

        if (!gSubjectDN.HasAttr(kOID_AttributeType_ChipAuthTag1))
        {
            attrOID = kOID_AttributeType_ChipAuthTag1;
        }
        else if (!gSubjectDN.HasAttr(kOID_AttributeType_ChipAuthTag2))
        {
            attrOID = kOID_AttributeType_ChipAuthTag2;
        }
        else
        {
            PrintArgError("%s: Too many authentication tag attributes are specified: %s\n", progName, arg);
            return false;
        }

        err = gSubjectDN.AddAttribute(attrOID, chip64bitAttr);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add subject DN attribute: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;

    case 'f':
        if (!ParseChip64bitAttr(arg, chip64bitAttr))
        {
            PrintArgError("%s: Invalid value specified for subject fabric id attribute: %s\n", progName, arg);
            return false;
        }

        err = gSubjectDN.AddAttribute(kOID_AttributeType_ChipFabricId, chip64bitAttr);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add Fabric Id attribute to the subject DN: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;

    case 'c':
        err = gSubjectDN.AddAttribute(kOID_AttributeType_CommonName,
                                      chip::ByteSpan(reinterpret_cast<const uint8_t *>(arg), strlen(arg)));
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add Common Name attribute to the subject DN: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;
    case 'x':
        gFutureExtensions[gFutureExtensionsCount].nid  = NID_subject_alt_name;
        gFutureExtensions[gFutureExtensionsCount].info = arg;
        gFutureExtensionsCount++;
        break;
    case '2':
        gFutureExtensions[gFutureExtensionsCount].nid  = NID_info_access;
        gFutureExtensions[gFutureExtensionsCount].info = arg;
        gFutureExtensionsCount++;
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
    case 'F':
        if (strcmp(arg, "x509-pem") == 0)
        {
            gOutCertFormat = kCertFormat_X509_PEM;
            gOutKeyFormat  = kKeyFormat_X509_PEM;
        }
        else if (strcmp(arg, "x509-der") == 0)
        {
            gOutCertFormat = kCertFormat_X509_DER;
            gOutKeyFormat  = kKeyFormat_X509_DER;
        }
        else if (strcmp(arg, "chip") == 0)
        {
            gOutCertFormat = kCertFormat_Chip_Raw;
            gOutKeyFormat  = kKeyFormat_Chip_Raw;
        }
        else if (strcmp(arg, "chip-b64") == 0)
        {
            gOutCertFormat = kCertFormat_Chip_Base64;
            gOutKeyFormat  = kKeyFormat_Chip_Base64;
        }
        else
        {
            PrintArgError("%s: Invalid value specified for the output format: %s\n", progName, arg);
            return false;
        }
        break;
    case 'V':
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

bool Cmd_GenCert(int argc, char * argv[])
{
    CHIP_ERROR err   = CHIP_NO_ERROR;
    bool res         = true;
    uint8_t certType = kCertType_NotSpecified;
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
        ExitNow(res = true);
    }

    res = ParseArgs(CMD_NAME, argc, argv, gCmdOptionSets);
    VerifyTrueOrExit(res);

    if (gSubjectDN.IsEmpty())
    {
        fprintf(stderr, "Please specify the subject DN attributes.\n");
        ExitNow(res = false);
    }

    err = gSubjectDN.GetCertType(certType);
    if (err != CHIP_NO_ERROR)
    {
        fprintf(stderr, "Invalid certificate subject attribute specified: %s\n", chip::ErrorStr(err));
        ExitNow(res = false);
    }
    if (certType != gCertType)
    {
        fprintf(stderr, "Please specify certificate type that matches subject DN attributes.\n");
        ExitNow(res = false);
    }

    if (gCACertFileName == nullptr && !gSelfSign)
    {
        fprintf(stderr, "Please specify the CA certificate file name using the --ca-cert option.\n");
        ExitNow(res = false);
    }
    else if (gCACertFileName != nullptr && gSelfSign)
    {
        fprintf(stderr, "Please don't specify --ca-cert option for the self signed certificate. \n");
        ExitNow(res = false);
    }

    if (gCACertFileName != nullptr && gCAKeyFileName == nullptr)
    {
        fprintf(stderr, "Please specify the CA key file name using the --ca-key option.\n");
        ExitNow(res = false);
    }

    if (gOutCertFileName == nullptr)
    {
        fprintf(stderr, "Please specify the file name for the new certificate using the --out option.\n");
        ExitNow(res = false);
    }

    if (gInKeyFileName == nullptr && gOutKeyFileName == nullptr)
    {
        fprintf(stderr, "Please specify the file name for the new public/private key using the --out-key option.\n");
        ExitNow(res = false);
    }

    if (gValidDays == 0)
    {
        fprintf(stderr, "Please specify the lifetime (in dys) for the new certificate using the --lifetime option.\n");
        ExitNow(res = false);
    }

    if (strcmp(gOutCertFileName, "-") != 0 && access(gOutCertFileName, R_OK) == 0)
    {
        fprintf(stderr,
                "Output certificate file already exists (%s)\n"
                "To replace the file, please remove it and re-run the command.\n",
                gOutCertFileName);
        ExitNow(res = false);
    }

    if (gOutKeyFileName != nullptr && access(gOutKeyFileName, R_OK) == 0)
    {
        fprintf(stderr,
                "Output key file already exists (%s)\n"
                "To replace the file, please remove it and re-run the command.\n",
                gOutKeyFileName);
        ExitNow(res = false);
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

    if (gSelfSign)
    {
        res = MakeCert(gCertType, &gSubjectDN, newCert.get(), newKey.get(), gValidFrom, gValidDays, gFutureExtensions,
                       gFutureExtensionsCount, newCert.get(), newKey.get());
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

        res = MakeCert(gCertType, &gSubjectDN, caCert.get(), caKey.get(), gValidFrom, gValidDays, gFutureExtensions,
                       gFutureExtensionsCount, newCert.get(), newKey.get());
        VerifyTrueOrExit(res);
    }

    res = WriteCert(gOutCertFileName, newCert.get(), gOutCertFormat);
    VerifyTrueOrExit(res);

    if (gOutKeyFileName != nullptr)
    {
        res = WritePrivateKey(gOutKeyFileName, newKey.get(), gOutKeyFormat);
        VerifyTrueOrExit(res);
    }

exit:
    return res;
}
