/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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

#include <lib/support/SafeInt.h>

namespace {

using namespace chip;
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
    { "vid-pid-as-cn",    kNoArgument,       'a' },
    { "key",              kArgumentRequired, 'k' },
    { "ca-cert",          kArgumentRequired, 'C' },
    { "ca-key",           kArgumentRequired, 'K' },
    { "out",              kArgumentRequired, 'o' },
    { "out-key",          kArgumentRequired, 'O' },
    { "valid-from",       kArgumentRequired, 'f' },
    { "lifetime",         kArgumentRequired, 'l' },
#if CHIP_CONFIG_INTERNAL_FLAG_GENERATE_DA_TEST_CASES
    { "ignore-error",     kNoArgument,       'I' },
    { "error-type",       kArgumentRequired, 'E' },
#endif
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
    "   -a, --vid-pid-as-cn\n"
    "\n"
    "       Encode Matter VID and PID parameters as Common Name attributes in the Subject DN.\n"
    "       If not specified then by default the VID and PID fields are encoded using\n"
    "       Matter specific OIDs.\n"
    "\n"
    "   -C, --ca-cert <file/str>\n"
    "\n"
    "       File or string containing CA certificate to be used to sign the new certificate.\n"
    "\n"
    "   -K, --ca-key <file/str>\n"
    "\n"
    "       File or string containing CA private key to be used to sign the new certificate.\n"
    "\n"
    "   -k, --key <file/str>\n"
    "\n"
    "       File or string containing the public and private keys for the new certificate (in an X.509 PEM format).\n"
    "       If not specified, a new key pair will be generated.\n"
    "\n"
    "   -o, --out <file/stdout>\n"
    "\n"
    "       File to contain the new certificate (in an X.509 PEM format).\n"
    "       If specified '-' then output is written to stdout.\n"
    "\n"
    "   -O, --out-key <file/stdout>\n"
    "\n"
    "       File to contain the public/private key for the new certificate (in an X.509 PEM format).\n"
    "       This option must be specified if the --key option is not.\n"
    "       If specified '-' then output is written to stdout.\n"
    "\n"
    "   -f, --valid-from <YYYY>-<MM>-<DD> [ <HH>:<MM>:<SS> ]\n"
    "\n"
    "       The start date for the certificate's validity period. If not specified,\n"
    "       the validity period starts on the current day.\n"
    "\n"
    "   -l, --lifetime <days>\n"
    "\n"
    "       The lifetime for the new certificate, in whole days. Use special value\n"
    "       4294967295 to indicate that certificate doesn't have well defined\n"
    "       expiration date\n"
    "\n"
#if CHIP_CONFIG_INTERNAL_FLAG_GENERATE_DA_TEST_CASES
    "   -I, --ignore-error\n"
    "\n"
    "       Ignore some input parameters error.\n"
    "       WARNING: This option makes it possible to circumvent attestation certificate\n"
    "       structure requirement. This is required for negative testing of the attestation flow.\n"
    "       Because of this it SHOULD NEVER BE ENABLED IN PRODUCTION BUILDS.\n"
    "\n"
    "   -E, --error-type <error-type>\n"
    "\n"
    "       When specified injects specific error into the structure of generated attestation certificate.\n"
    "       Note that 'ignore-error' option MUST be specified for this error injection to take effect.\n"
    "       Supported error types that can be injected are:\n"
    "           no-error                         - No error to inject.\n"
    "           cert-version                     - Certificate version will be set to v2 instead of required v3.\n"
    "           sig-algo                         - Use ecdsa-with-SHA1 signature algorithm instead of required ecdsa-with-SHA256.\n"
    "           issuer-vid                       - TODO\n"
    "           issuer-pid                       - TODO\n"
    "           subject-vid                      - TODO\n"
    "           subject-vid-mismatch             - The VID value in the subject field won't match VID in the issuer field.\n"
    "           subject-pid                      - TODO\n"
    "           subject-pid-mismatch             - The PID value in the subject field won't match PID in the issuer field.\n"
    "           sig-curve                        - Use secp256k1 curve to generate certificate signature instead of\n"
    "                                              required secp256r1 (aka prime256v1).\n"
    "           ext-basic-missing                - Certificate won't have required Basic Constraint extension.\n"
    "           ext-basic-critical-missing       - Basic Constraint extension won't have critical field.\n"
    "           ext-basic-critical-wrong         - Basic Constraint extension will be marked as non-critical.\n"
    "           ext-basic-ca-missing             - Basic Constraint extension won't have cA field.\n"
    "           ext-basic-ca-wrong               - Basic Constraint extension cA field will be set to TRUE for DAC\n"
    "                                              and to FALSE for PAI and PAA.\n"
    "           ext-basic-pathlen-presence-wrong - Basic Constraint extension will include pathLen field for DAC\n"
    "                                              and won't have pathLen field for PAI and PAA.\n"
    "           ext-basic-pathlen0               - Basic Constraint extension pathLen field will be set to 0.\n"
    "           ext-basic-pathlen1               - Basic Constraint extension pathLen field will be set to 1.\n"
    "           ext-basic-pathlen2               - Basic Constraint extension pathLen field will be set to 2.\n"
    "           ext-key-usage-missing            - Certificate won't have required Key Usage extension.\n"
    "           ext-key-usage-critical-missing   - Key Usage extension won't have critical field.\n"
    "           ext-key-usage-critical-wrong     - Key Usage extension will be marked as non-critical.\n"
    "           ext-key-usage-dig-sig            - Key Usage extension digitalSignature flag won't be set for DAC\n"
    "                                              and will be set for PAI/PAA.\n"
    "           ext-key-usage-key-cert-sign      - Key Usage extension keyCertSign flag will be set for DAC\n"
    "                                              and won't be set for PAI/PAA.\n"
    "           ext-key-usage-crl-sign           - Key Usage extension cRLSign flag will be set for DAC\n"
    "                                              and won't set for PAI/PAA.\n"
    "           ext-akid-missing                 - Certificate won't have required Authority Key ID extension.\n"
    "           ext-skid-missing                 - Certificate won't have required Subject Key ID extension.\n"
    "           ext-extended-key-usage           - Certificate will include optional Extended Key Usage extension.\n"
    "           ext-authority-info-access        - Certificate will include optional Authority Information Access extension.\n"
    "           ext-subject-alt-name             - Certificate will include optional Subject Alternative Name extension.\n"
    "\n"
#endif // CHIP_CONFIG_INTERNAL_FLAG_GENERATE_DA_TEST_CASES
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
    "Generate a CHIP Attestation certificate"
);

OptionSet *gCmdOptionSets[] =
{
    &gCmdOptions,
    &gHelpOptions,
    nullptr
};
// clang-format on

AttCertType gAttCertType          = kAttCertType_NotSpecified;
const char * gSubjectCN           = nullptr;
uint16_t gSubjectVID              = VendorId::NotSpecified;
uint16_t gSubjectPID              = 0;
bool gEncodeVIDandPIDasCN         = false;
const char * gCACertFileNameOrStr = nullptr;
const char * gCAKeyFileNameOrStr  = nullptr;
const char * gInKeyFileNameOrStr  = nullptr;
const char * gOutCertFileName     = nullptr;
const char * gOutKeyFileName      = nullptr;
uint32_t gValidDays               = kCertValidDays_Undefined;
struct tm gValidFrom;
CertStructConfig gCertConfig;

bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg)
{
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
        if (!ParseInt(arg, gSubjectVID, 16))
        {
            PrintArgError("%s: Invalid value specified for the subject VID attribute: %s\n", progName, arg);
            return false;
        }
        break;
    case 'P':
        if (!ParseInt(arg, gSubjectPID, 16))
        {
            PrintArgError("%s: Invalid value specified for the subject PID attribute: %s\n", progName, arg);
            return false;
        }
        break;
    case 'a':
        gEncodeVIDandPIDasCN = true;
        break;
    case 'k':
        gInKeyFileNameOrStr = arg;
        break;
    case 'C':
        gCACertFileNameOrStr = arg;
        break;
    case 'K':
        gCAKeyFileNameOrStr = arg;
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
#if CHIP_CONFIG_INTERNAL_FLAG_GENERATE_DA_TEST_CASES
    case 'I':
        gCertConfig.EnableErrorTestCase();
        break;
    case 'E':
        if (strcmp(arg, "cert-version") == 0)
        {
            gCertConfig.SetCertVersionWrong();
        }
        else if (strcmp(arg, "sig-algo") == 0)
        {
            gCertConfig.SetSigAlgoWrong();
        }
        else if (strcmp(arg, "subject-vid-mismatch") == 0)
        {
            gCertConfig.SetSubjectVIDMismatch();
        }
        else if (strcmp(arg, "subject-pid-mismatch") == 0)
        {
            gCertConfig.SetSubjectPIDMismatch();
        }
        else if (strcmp(arg, "sig-curve") == 0)
        {
            gCertConfig.SetSigCurveWrong();
        }
        else if (strcmp(arg, "ext-basic-missing") == 0)
        {
            gCertConfig.SetExtensionBasicMissing();
        }
        else if (strcmp(arg, "ext-basic-critical-missing") == 0)
        {
            gCertConfig.SetExtensionBasicCriticalMissing();
        }
        else if (strcmp(arg, "ext-basic-critical-wrong") == 0)
        {
            gCertConfig.SetExtensionBasicCriticalWrong();
        }
        else if (strcmp(arg, "ext-basic-ca-missing") == 0)
        {
            gCertConfig.SetExtensionBasicCAMissing();
        }
        else if (strcmp(arg, "ext-basic-ca-wrong") == 0)
        {
            gCertConfig.SetExtensionBasicCAWrong();
        }
        else if (strcmp(arg, "ext-basic-pathlen-presence-wrong") == 0)
        {
            gCertConfig.SetExtensionBasicPathLenPresenceWrong();
        }
        else if (strcmp(arg, "ext-basic-pathlen0") == 0)
        {
            gCertConfig.SetExtensionBasicPathLen0();
        }
        else if (strcmp(arg, "ext-basic-pathlen1") == 0)
        {
            gCertConfig.SetExtensionBasicPathLen1();
        }
        else if (strcmp(arg, "ext-basic-pathlen2") == 0)
        {
            gCertConfig.SetExtensionBasicPathLen2();
        }
        else if (strcmp(arg, "ext-key-usage-missing") == 0)
        {
            gCertConfig.SetExtensionKeyUsageMissing();
        }
        else if (strcmp(arg, "ext-key-usage-critical-missing") == 0)
        {
            gCertConfig.SetExtensionKeyUsageCriticalMissing();
        }
        else if (strcmp(arg, "ext-key-usage-critical-wrong") == 0)
        {
            gCertConfig.SetExtensionKeyUsageCriticalWrong();
        }
        else if (strcmp(arg, "ext-key-usage-dig-sig") == 0)
        {
            gCertConfig.SetExtensionKeyUsageDigitalSigWrong();
        }
        else if (strcmp(arg, "ext-key-usage-key-cert-sign") == 0)
        {
            gCertConfig.SetExtensionKeyUsageKeyCertSignWrong();
        }
        else if (strcmp(arg, "ext-key-usage-crl-sign") == 0)
        {
            gCertConfig.SetExtensionKeyUsageCRLSignWrong();
        }
        else if (strcmp(arg, "ext-akid-missing") == 0)
        {
            gCertConfig.SetExtensionAKIDMissing();
        }
        else if (strcmp(arg, "ext-skid-missing") == 0)
        {
            gCertConfig.SetExtensionSKIDMissing();
        }
        else if (strcmp(arg, "ext-extended-key-usage") == 0)
        {
            gCertConfig.SetExtensionExtendedKeyUsagePresent();
        }
        else if (strcmp(arg, "ext-authority-info-access") == 0)
        {
            gCertConfig.SetExtensionAuthorityInfoAccessPresent();
        }
        else if (strcmp(arg, "ext-subject-alt-name") == 0)
        {
            gCertConfig.SetExtensionSubjectAltNamePresent();
        }
        else if (strcmp(arg, "no-error") != 0)
        {
            PrintArgError("%s: Invalid value specified for the error type: %s\n", progName, arg);
            return false;
        }
        break;
#endif // CHIP_CONFIG_INTERNAL_FLAG_GENERATE_DA_TEST_CASES
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

    if (gCertConfig.IsErrorTestCaseEnabled())
    {
        fprintf(stderr,
                "WARNING get-att-cert: The ignor-error option is set. This option makes it possible to generate invalid "
                "certificates.\n");
    }

    if (gAttCertType == kAttCertType_NotSpecified)
    {
        fprintf(stderr, "Please specify attestation certificate type.\n");
        return false;
    }
    if (!gCertConfig.IsErrorTestCaseEnabled())
    {
        if (gAttCertType == kAttCertType_DAC)
        {
            if (gSubjectVID == VendorId::NotSpecified || gSubjectPID == 0)
            {
                fprintf(stderr, "Please specify VID and PID subject DN attributes.\n");
                return false;
            }
        }
        if (gAttCertType == kAttCertType_PAI)
        {
            if (gSubjectVID == VendorId::NotSpecified)
            {
                fprintf(stderr, "Please specify VID subject DN attributes.\n");
                return false;
            }
        }
        if (gAttCertType == kAttCertType_PAA)
        {
            if (gSubjectPID != 0)
            {
                fprintf(stderr, "VID & PID SHALL NOT specify subject DN attributes.\n");
                return false;
            }
        }
    }

    if (gCACertFileNameOrStr == nullptr && gAttCertType != kAttCertType_PAA)
    {
        fprintf(stderr, "Please specify the CA certificate file name using the --ca-cert option.\n");
        return false;
    }

    if (gCACertFileNameOrStr != nullptr && gAttCertType == kAttCertType_PAA)
    {
        fprintf(stderr, "Please don't specify --ca-cert option for the self signed certificate. \n");
        return false;
    }

    if (gCACertFileNameOrStr != nullptr && gCAKeyFileNameOrStr == nullptr)
    {
        fprintf(stderr, "Please specify the CA key file name using the --ca-key option.\n");
        return false;
    }

    if (gOutCertFileName == nullptr)
    {
        fprintf(stderr, "Please specify the file name for the new certificate using the --out option.\n");
        return false;
    }

    if (gInKeyFileNameOrStr == nullptr && gOutKeyFileName == nullptr)
    {
        fprintf(stderr, "Please specify the file name for the new public/private key using the --out-key option.\n");
        return false;
    }

    if (gValidDays == kCertValidDays_Undefined)
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

    if (gInKeyFileNameOrStr != nullptr)
    {
        res = ReadKey(gInKeyFileNameOrStr, newKey);
        VerifyTrueOrExit(res);
    }
    else
    {
        if (gCertConfig.IsSigCurveWrong())
        {
            res = GenerateKeyPair_Secp256k1(newKey.get());
            VerifyTrueOrExit(res);
        }
        else
        {
            res = GenerateKeyPair(newKey.get());
            VerifyTrueOrExit(res);
        }
    }

    if (gAttCertType == kAttCertType_PAA)
    {
        res = MakeAttCert(gAttCertType, gSubjectCN, gSubjectVID, gSubjectPID, gEncodeVIDandPIDasCN, newCert.get(), newKey.get(),
                          gValidFrom, gValidDays, newCert.get(), newKey.get(), gCertConfig);
        VerifyTrueOrExit(res);
    }
    else
    {
        std::unique_ptr<X509, void (*)(X509 *)> caCert(X509_new(), &X509_free);
        std::unique_ptr<EVP_PKEY, void (*)(EVP_PKEY *)> caKey(EVP_PKEY_new(), &EVP_PKEY_free);

        res = ReadCert(gCACertFileNameOrStr, caCert.get());
        VerifyTrueOrExit(res);

        res = ReadKey(gCAKeyFileNameOrStr, caKey, gCertConfig.IsErrorTestCaseEnabled());
        VerifyTrueOrExit(res);

        res = MakeAttCert(gAttCertType, gSubjectCN, gSubjectVID, gSubjectPID, gEncodeVIDandPIDasCN, caCert.get(), caKey.get(),
                          gValidFrom, gValidDays, newCert.get(), newKey.get(), gCertConfig);
        VerifyTrueOrExit(res);
    }

    res = WriteCert(gOutCertFileName, newCert.get(), kCertFormat_X509_PEM);
    VerifyTrueOrExit(res);

    if (gOutKeyFileName != nullptr)
    {
        res = WriteKey(gOutKeyFileName, newKey.get(), kKeyFormat_X509_PEM);
        VerifyTrueOrExit(res);
    }

exit:
    return res;
}
