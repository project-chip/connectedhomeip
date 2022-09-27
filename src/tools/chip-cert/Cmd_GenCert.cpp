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
    { "type",                kArgumentRequired, 't' },
    { "subject-chip-id",     kArgumentRequired, 'i' },
    { "subject-fab-id",      kArgumentRequired, 'f' },
    { "subject-cat",         kArgumentRequired, 'a' },
    { "subject-cn-u",        kArgumentRequired, 'c' },
    { "subject-cn-p",        kArgumentRequired, 'b' },
    { "subject-su-u",        kArgumentRequired, 'd' },
    { "subject-su-p",        kArgumentRequired, 'e' },
    { "subject-sn-u",        kArgumentRequired, 'g' },
    { "subject-sn-p",        kArgumentRequired, 'j' },
    { "subject-co-u",        kArgumentRequired, 'm' },
    { "subject-co-p",        kArgumentRequired, 'n' },
    { "subject-ln-u",        kArgumentRequired, 'q' },
    { "subject-ln-p",        kArgumentRequired, 'r' },
    { "subject-pn-u",        kArgumentRequired, 's' },
    { "subject-pn-p",        kArgumentRequired, 'u' },
    { "subject-on-u",        kArgumentRequired, 'w' },
    { "subject-on-p",        kArgumentRequired, 'y' },
    { "subject-un-u",        kArgumentRequired, 'z' },
    { "subject-un-p",        kArgumentRequired, 'U' },
    { "subject-ti-u",        kArgumentRequired, 'W' },
    { "subject-ti-p",        kArgumentRequired, 'S' },
    { "subject-na-u",        kArgumentRequired, 'T' },
    { "subject-na-p",        kArgumentRequired, 'A' },
    { "subject-gn-u",        kArgumentRequired, 'B' },
    { "subject-gn-p",        kArgumentRequired, 'D' },
    { "subject-in-u",        kArgumentRequired, 'G' },
    { "subject-in-p",        kArgumentRequired, 'H' },
    { "subject-gq-u",        kArgumentRequired, 'J' },
    { "subject-gq-p",        kArgumentRequired, 'L' },
    { "subject-dq-u",        kArgumentRequired, 'M' },
    { "subject-dq-p",        kArgumentRequired, 'N' },
    { "subject-ps-u",        kArgumentRequired, 'P' },
    { "subject-ps-p",        kArgumentRequired, 'Q' },
    { "subject-dc-i",        kArgumentRequired, 'R' },
    { "path-len-constraint", kArgumentRequired, 'p' },
    { "future-ext-sub",      kArgumentRequired, 'x' },
    { "future-ext-info",     kArgumentRequired, '2' },
    { "key",                 kArgumentRequired, 'k' },
    { "ca-cert",             kArgumentRequired, 'C' },
    { "ca-key",              kArgumentRequired, 'K' },
    { "out",                 kArgumentRequired, 'o' },
    { "out-key",             kArgumentRequired, 'O' },
    { "out-format",          kArgumentRequired, 'F' },
    { "valid-from",          kArgumentRequired, 'V' },
    { "lifetime",            kArgumentRequired, 'l' },
#if CHIP_CONFIG_INTERNAL_FLAG_GENERATE_OP_CERT_TEST_CASES
    { "ignore-error",        kNoArgument,       'I' },
    { "error-type",          kArgumentRequired, 'E' },
#endif
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
    "       Subject DN CHIP Id attribute in hexadecimal format with upto 8 octets with or without '0x' prefix.\n"
    "          - for Root certificate it is ChipRootId\n"
    "          - for intermediate CA certificate it is ChipICAId\n"
    "          - for Node certificate it is ChipNodeId. The value should be in a range [1, 0xFFFFFFEFFFFFFFFF]\n"
    "          - for Firmware Signing certificate it is ChipFirmwareSigningId\n"
    "\n"
    "   -f, --subject-fab-id <hex-digits>\n"
    "\n"
    "       Subject DN Fabric Id attribute in hexadecimal format with upto 8 octets with or without '0x' prefix.\n"
    "       The value should be different from 0.\n"
    "\n"
    "   -a, --subject-cat <hex-digits>\n"
    "\n"
    "       Subject DN CHIP CASE Authentication Tag in hexadecimal format with upto 4 octets with or without '0x' prefix.\n"
    "       The version subfield (lower 16 bits) should be different from 0.\n"
    "\n"
    "   Variety of DN attributes are also supported and can be added to the subject DN of the certificate.\n"
    "   These attributes can be encoded as UTF8String, PrintableString or IA5String as specified below:\n"
    "\n"
    "   -<c/b>, --subject-cn-<u/p> <string>  - commonName attribute             <UTF8String/PrintableString>\n"
    "   -<d/e>, --subject-su-<u/p> <string>  - surname attribute                <UTF8String/PrintableString>\n"
    "   -<g/j>, --subject-sn-<u/p> <string>  - serialNumber attribute           <UTF8String/PrintableString>\n"
    "   -<m/n>, --subject-co-<u/p> <string>  - countryName attribute            <UTF8String/PrintableString>\n"
    "   -<q/r>, --subject-ln-<u/p> <string>  - localityName attribute           <UTF8String/PrintableString>\n"
    "   -<s/u>, --subject-pn-<u/p> <string>  - stateOrProvinceName attribute    <UTF8String/PrintableString>\n"
    "   -<w/y>, --subject-on-<u/p> <string>  - organizationName attribute       <UTF8String/PrintableString>\n"
    "   -<z/U>, --subject-un-<u/p> <string>  - organizationalUnitName attribute <UTF8String/PrintableString>\n"
    "   -<V/S>, --subject-ti-<u/p> <string>  - title attribute                  <UTF8String/PrintableString>\n"
    "   -<T/A>, --subject-na-<u/p> <string>  - name attribute                   <UTF8String/PrintableString>\n"
    "   -<B/D>, --subject-gn-<u/p> <string>  - givenName attribute              <UTF8String/PrintableString>\n"
    "   -<G/H>, --subject-in-<u/p> <string>  - initials attribute               <UTF8String/PrintableString>\n"
    "   -<J/L>, --subject-gq-<u/p> <string>  - generationQualifier attribute    <UTF8String/PrintableString>\n"
    "   -<M/N>, --subject-dq-<u/p> <string>  - dnQualifier attribute            <UTF8String/PrintableString>\n"
    "   -<P/Q>, --subject-ps-<u/p> <string>  - pseudonym attribute              <UTF8String/PrintableString>\n"
    "   -R,     --subject-dc-i     <string>  - domainComponent attribute        <IA5String>\n"
    "\n"
    "       Subject DN Common Name attribute encoded as UTF8String.\n"
    "\n"
    "   -p, --path-len-constraint <int>\n"
    "\n"
    "       Path length constraint to be included in the basic constraint extension.\n"
    "       If not specified, the path length constraint is not included in the extension.\n"
    "\n"
    "   -x, --future-ext-sub <string>\n"
    "\n"
    "       NID_subject_alt_name extension to be added to the list of certificate extensions.\n"
    "\n"
    "   -2, --future-ext-info <string>\n"
    "\n"
    "       NID_info_access extension to be added to the list of certificate extensions.\n"
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
    "       File or string containing the public and private keys for the new certificate.\n"
    "       If not specified, a new key pair will be generated.\n"
    "\n"
    "   -o, --out <file/stdout>\n"
    "\n"
    "       File to contain the new certificate.\n"
    "       If specified '-' then output is written to stdout.\n"
    "\n"
    "   -O, --out-key <file/stdout>\n"
    "\n"
    "       File to contain the public/private key for the new certificate.\n"
    "       This option must be specified if the --key option is not.\n"
    "       If specified '-' then output is written to stdout.\n"
    "\n"
    "  -F, --out-format <format>\n"
    "\n"
    "       Specifies format of the output certificate and private key.\n"
    "       If not specified, the default base-64 encoded CHIP format is used.\n"
    "       Supported format parametes are:\n"
    "           x509-pem  - X.509 PEM format\n"
    "           x509-der  - X.509 DER raw format\n"
    "           x509-hex  - X.509 DER hex encoded format\n"
    "           chip      - raw CHIP TLV format\n"
    "           chip-b64  - base-64 encoded CHIP TLV format (default)\n"
    "           chip-hex  - hex encoded CHIP TLV format\n"
    "\n"
    "   -V, --valid-from <YYYY>-<MM>-<DD> [ <HH>:<MM>:<SS> ]\n"
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
#if CHIP_CONFIG_INTERNAL_FLAG_GENERATE_OP_CERT_TEST_CASES
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
    "           cert-oversized                   - Certificate size will exceed it's muximum supported size, which is\n"
    "                                              400 bytes for the CHIP TLV encoded cert and 600 bytes for DER encoded cert.\n"
    "           cert-version                     - Certificate version will be set to v2 instead of required v3.\n"
    "           serial-number-missing            - Certificate won't have required serialNumber field.\n"
    "           sig-algo                         - Use ecdsa-with-SHA1 signature algorithm instead of required ecdsa-with-SHA256.\n"
    "           issuer-missing                   - Certificate won't have required Issuer field.\n"
    "           validity-not-before-missing      - Certificate won't have required validity not-before field.\n"
    "           validity-not-after-missing       - Certificate won't have required validity not-after field.\n"
    "           validity-wrong                   - Certificate will have validity not-before and not-after values switched,\n"
    "                                              where not-before will have greater value than not-after.\n"
    "           subject-missing                  - Certificate won't have required Subject field.\n"
    "           subject-node-id-missing          - Subject won't have NodeId attribute.\n"
    "           subject-node-id-invalid          - Subject will include invalid NodeId value.\n"
    "           subject-node-id-twice            - Subject will include two NodeId attributes.\n"
    "           subject-fabric-id-missing        - Subject won't have FabricId attribute.\n"
    "           subject-fabric-id-invalid        - Subject will include invalid FabricId value.\n"
    "           subject-fabric-id-twice          - Subject will include two FabricId attributes.\n"
    "           subject-fabric-id-mismatch       - The FabricId in the subject won't match FabricId in the issuer field.\n"
    "           subject-cat-invalid              - Subject will include invalid CASE Authenticated Tag (CAT) value.\n"
    "           sig-curve                        - Use secp256k1 curve to generate certificate signature instead of\n"
    "                                              required secp256r1 (aka prime256v1).\n"
    "           publickey                        - Error will be injected in one of the bytes of the public key value.\n"
    "                                              required secp256r1 (aka prime256v1).\n"
    "           ext-basic-missing                - Certificate won't have required Basic Constraint extension.\n"
    "           ext-basic-critical-missing       - Basic Constraint extension won't have critical field.\n"
    "           ext-basic-critical-wrong         - Basic Constraint extension will be marked as non-critical.\n"
    "           ext-basic-ca-missing             - Basic Constraint extension won't have cA field.\n"
    "           ext-basic-ca-wrong               - Basic Constraint extension cA field will be set to TRUE for DAC\n"
    "                                              and to FALSE for PAI and PAA.\n"
    "           ext-basic-pathlen-presence-wrong - Basic Constraint extension will include pathLen field for NOC.\n"
    "           ext-basic-pathlen0               - Basic Constraint extension pathLen field will be set to 0.\n"
    "           ext-basic-pathlen1               - Basic Constraint extension pathLen field will be set to 1.\n"
    "           ext-basic-pathlen2               - Basic Constraint extension pathLen field will be set to 2.\n"
    "           ext-key-usage-missing            - Certificate won't have required Key Usage extension.\n"
    "           ext-key-usage-critical-missing   - Key Usage extension won't have critical field.\n"
    "           ext-key-usage-critical-wrong     - Key Usage extension will be marked as non-critical.\n"
    "           ext-key-usage-dig-sig            - Key Usage extension digitalSignature flag won't be set for NOC\n"
    "                                              and will be set for ICAC/RCAC.\n"
    "           ext-key-usage-key-cert-sign      - Key Usage extension keyCertSign flag will be set for NOC\n"
    "                                              and won't be set for ICAC/RCAC.\n"
    "           ext-key-usage-crl-sign           - Key Usage extension cRLSign flag will be set for NOC\n"
    "                                              and won't set for ICAC/RCAC.\n"
    "           ext-akid-missing                 - Certificate won't have required Authority Key ID extension.\n"
    "           ext-skid-missing                 - Certificate won't have required Subject Key ID extension.\n"
    "           ext-extended-key-usage-missing   - Certificate won't have required Extended Key Usage extension.\n"
    "           signature                        - Error will be injected in one of the bytes of the signature value.\n"
   "\n"
#endif // CHIP_CONFIG_INTERNAL_FLAG_GENERATE_OP_CERT_TEST_CASES
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
uint8_t gCertType                           = kCertType_NotSpecified;
int gPathLengthConstraint                   = kPathLength_NotSpecified;
bool gSelfSign                              = false;
const char * gCACertFileNameOrStr           = nullptr;
const char * gCAKeyFileNameOrStr            = nullptr;
const char * gInKeyFileNameOrStr            = nullptr;
const char * gOutCertFileName               = nullptr;
const char * gOutKeyFileName                = nullptr;
CertFormat gOutCertFormat                   = kCertFormat_Default;
KeyFormat gOutKeyFormat                     = kKeyFormat_Default;
uint32_t gValidDays                         = kCertValidDays_Undefined;
FutureExtensionWithNID gFutureExtensions[3] = { { 0, nullptr } };
uint8_t gFutureExtensionsCount              = 0;
struct tm gValidFrom;
CertStructConfig gCertConfig;

bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint64_t chip64bitAttr;
    uint32_t chip32bitAttr;

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
        if (!ParseInt(arg, chip64bitAttr, 16))
        {
            PrintArgError("%s: Invalid value specified for subject chip id attribute: %s\n", progName, arg);
            return false;
        }

        switch (gCertType)
        {
        case kCertType_Node:
            if (gCertConfig.IsSubjectNodeIdValid() && !chip::IsOperationalNodeId(chip64bitAttr))
            {
                PrintArgError("%s: Invalid value specified for chip node-id attribute: %s\n", progName, arg);
                return false;
            }
            if (gCertConfig.IsSubjectNodeIdPresent())
            {
                if (gCertConfig.IsSubjectNodeIdValid())
                {
                    err = gSubjectDN.AddAttribute_MatterNodeId(chip64bitAttr);
                }
                else
                {
                    err = gSubjectDN.AddAttribute_MatterNodeId(chip::kMaxOperationalNodeId + 10);
                }
                if ((err == CHIP_NO_ERROR) && gCertConfig.IsSubjectNodeIdRepeatsTwice())
                {
                    err = gSubjectDN.AddAttribute_MatterNodeId(chip64bitAttr + 1);
                }
            }
            break;
        case kCertType_FirmwareSigning:
            err = gSubjectDN.AddAttribute_MatterFirmwareSigningId(chip64bitAttr);
            break;
        case kCertType_ICA:
            err = gSubjectDN.AddAttribute_MatterICACId(chip64bitAttr);
            break;
        case kCertType_Root:
            err = gSubjectDN.AddAttribute_MatterRCACId(chip64bitAttr);
            break;
        default:
            PrintArgError("%s: Certificate type argument should be specified prior to subject attribute: %s\n", progName, arg);
            return false;
        }

        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add subject DN attribute: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;

    case 'a':
        if (!ParseInt(arg, chip32bitAttr, 16) || !chip::IsValidCASEAuthTag(chip32bitAttr))
        {
            PrintArgError("%s: Invalid value specified for the subject CASE Authenticated Tag (CAT) attribute: %s\n", progName,
                          arg);
            return false;
        }

        err = gSubjectDN.AddAttribute_MatterCASEAuthTag(chip32bitAttr);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add subject DN attribute: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;

    case 'p':
        if (!ParseInt(arg, gPathLengthConstraint))
        {
            PrintArgError("%s: Invalid value specified for path length constraint: %s\n", progName, arg);
            return false;
        }
        break;
    case 'f':
        if (!ParseInt(arg, chip64bitAttr, 16) || !chip::IsValidFabricId(chip64bitAttr))
        {
            PrintArgError("%s: Invalid value specified for subject fabric id attribute: %s\n", progName, arg);
            return false;
        }

        if (gCertConfig.IsSubjectFabricIdPresent())
        {
            if (gCertConfig.IsSubjectFabricIdValid())
            {
                if (gCertConfig.IsSubjectFabricIdMismatch())
                {
                    err = gSubjectDN.AddAttribute_MatterFabricId(chip64bitAttr + 1);
                }
                else
                {
                    err = gSubjectDN.AddAttribute_MatterFabricId(chip64bitAttr);
                }
            }
            else
            {
                err = gSubjectDN.AddAttribute_MatterFabricId(chip::kUndefinedFabricId);
            }
            if ((err == CHIP_NO_ERROR) && gCertConfig.IsSubjectFabricIdRepeatsTwice())
            {
                err = gSubjectDN.AddAttribute_MatterFabricId(chip64bitAttr + 10);
            }
        }
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add Fabric Id attribute to the subject DN: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;

    case 'c':
        err = gSubjectDN.AddAttribute_CommonName(chip::CharSpan::fromCharString(arg), false);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add Common Name attribute to the subject DN: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;
    case 'b':
        err = gSubjectDN.AddAttribute_CommonName(chip::CharSpan::fromCharString(arg), true);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add Common Name attribute to the subject DN: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;
    case 'd':
        err = gSubjectDN.AddAttribute_Surname(chip::CharSpan::fromCharString(arg), false);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add Surname attribute to the subject DN: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;
    case 'e':
        err = gSubjectDN.AddAttribute_Surname(chip::CharSpan::fromCharString(arg), true);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add Surname attribute to the subject DN: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;
    case 'g':
        err = gSubjectDN.AddAttribute_SerialNumber(chip::CharSpan::fromCharString(arg), false);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add Serial Number attribute to the subject DN: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;
    case 'j':
        err = gSubjectDN.AddAttribute_SerialNumber(chip::CharSpan::fromCharString(arg), true);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add Serial Number attribute to the subject DN: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;
    case 'm':
        err = gSubjectDN.AddAttribute_CountryName(chip::CharSpan::fromCharString(arg), false);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add Country Name attribute to the subject DN: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;
    case 'n':
        err = gSubjectDN.AddAttribute_CountryName(chip::CharSpan::fromCharString(arg), true);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add Country Name attribute to the subject DN: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;
    case 'q':
        err = gSubjectDN.AddAttribute_LocalityName(chip::CharSpan::fromCharString(arg), false);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add Locality Name attribute to the subject DN: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;
    case 'r':
        err = gSubjectDN.AddAttribute_LocalityName(chip::CharSpan::fromCharString(arg), true);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add Locality Name attribute to the subject DN: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;
    case 's':
        err = gSubjectDN.AddAttribute_StateOrProvinceName(chip::CharSpan::fromCharString(arg), false);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add stateOrProvinceName attribute to the subject DN: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;
    case 'u':
        err = gSubjectDN.AddAttribute_StateOrProvinceName(chip::CharSpan::fromCharString(arg), true);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add stateOrProvinceName attribute to the subject DN: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;
    case 'w':
        err = gSubjectDN.AddAttribute_OrganizationName(chip::CharSpan::fromCharString(arg), false);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add Organization Name attribute to the subject DN: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;
    case 'y':
        err = gSubjectDN.AddAttribute_OrganizationName(chip::CharSpan::fromCharString(arg), true);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add Organization Name attribute to the subject DN: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;
    case 'z':
        err = gSubjectDN.AddAttribute_OrganizationalUnitName(chip::CharSpan::fromCharString(arg), false);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add Organizational Unit Name attribute to the subject DN: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;
    case 'U':
        err = gSubjectDN.AddAttribute_OrganizationalUnitName(chip::CharSpan::fromCharString(arg), true);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add Organizational Unit Name attribute to the subject DN: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;
    case 'W':
        err = gSubjectDN.AddAttribute_Title(chip::CharSpan::fromCharString(arg), false);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add Title attribute to the subject DN: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;
    case 'S':
        err = gSubjectDN.AddAttribute_Title(chip::CharSpan::fromCharString(arg), true);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add Title attribute to the subject DN: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;
    case 'T':
        err = gSubjectDN.AddAttribute_Name(chip::CharSpan::fromCharString(arg), false);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add Name attribute to the subject DN: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;
    case 'A':
        err = gSubjectDN.AddAttribute_Name(chip::CharSpan::fromCharString(arg), true);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add Name attribute to the subject DN: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;
    case 'B':
        err = gSubjectDN.AddAttribute_GivenName(chip::CharSpan::fromCharString(arg), false);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add Given Name attribute to the subject DN: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;
    case 'D':
        err = gSubjectDN.AddAttribute_GivenName(chip::CharSpan::fromCharString(arg), true);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add Given Name attribute to the subject DN: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;
    case 'G':
        err = gSubjectDN.AddAttribute_Initials(chip::CharSpan::fromCharString(arg), false);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add Initials attribute to the subject DN: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;
    case 'H':
        err = gSubjectDN.AddAttribute_Initials(chip::CharSpan::fromCharString(arg), true);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add Initials attribute to the subject DN: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;
    case 'J':
        err = gSubjectDN.AddAttribute_GenerationQualifier(chip::CharSpan::fromCharString(arg), false);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add Generation Qualifier attribute to the subject DN: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;
    case 'L':
        err = gSubjectDN.AddAttribute_GenerationQualifier(chip::CharSpan::fromCharString(arg), true);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add Generation Qualifier attribute to the subject DN: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;
    case 'M':
        err = gSubjectDN.AddAttribute_DNQualifier(chip::CharSpan::fromCharString(arg), false);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add DN Qualifier attribute to the subject DN: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;
    case 'N':
        err = gSubjectDN.AddAttribute_DNQualifier(chip::CharSpan::fromCharString(arg), true);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add DN Qualifier attribute to the subject DN: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;
    case 'P':
        err = gSubjectDN.AddAttribute_Pseudonym(chip::CharSpan::fromCharString(arg), false);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add Pseudonym attribute to the subject DN: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;
    case 'Q':
        err = gSubjectDN.AddAttribute_Pseudonym(chip::CharSpan::fromCharString(arg), true);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add Pseudonym attribute to the subject DN: %s\n", chip::ErrorStr(err));
            return false;
        }
        break;
    case 'R':
        err = gSubjectDN.AddAttribute_DomainComponent(chip::CharSpan::fromCharString(arg), true);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add Domain Component attribute to the subject DN: %s\n", chip::ErrorStr(err));
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
        else if (strcmp(arg, "x509-hex") == 0)
        {
            gOutCertFormat = kCertFormat_X509_Hex;
            gOutKeyFormat  = kKeyFormat_X509_Hex;
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
        else if (strcmp(arg, "chip-hex") == 0)
        {
            gOutCertFormat = kCertFormat_Chip_Hex;
            gOutKeyFormat  = kKeyFormat_Chip_Hex;
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
#if CHIP_CONFIG_INTERNAL_FLAG_GENERATE_OP_CERT_TEST_CASES
    case 'I':
        gCertConfig.EnableErrorTestCase();
        break;
    case 'E':
        if (strcmp(arg, "cert-oversized") == 0)
        {
            gCertConfig.SetCertOversized();
        }
        else if (strcmp(arg, "cert-version") == 0)
        {
            gCertConfig.SetCertVersionWrong();
        }
        else if (strcmp(arg, "serial-number-missing") == 0)
        {
            gCertConfig.SetSerialNumberMissing();
        }
        else if (strcmp(arg, "sig-algo") == 0)
        {
            gCertConfig.SetSigAlgoWrong();
        }
        else if (strcmp(arg, "issuer-missing") == 0)
        {
            gCertConfig.SetIssuerMissing();
        }
        else if (strcmp(arg, "validity-not-before-missing") == 0)
        {
            gCertConfig.SetValidityNotBeforeMissing();
        }
        else if (strcmp(arg, "validity-not-after-missing") == 0)
        {
            gCertConfig.SetValidityNotAfterMissing();
        }
        else if (strcmp(arg, "validity-wrong") == 0)
        {
            gCertConfig.SetValidityWrong();
        }
        else if (strcmp(arg, "subject-missing") == 0)
        {
            gCertConfig.SetSubjectMissing();
        }
        else if (strcmp(arg, "subject-node-id-missing") == 0)
        {
            gCertConfig.SetSubjectNodeIdMissing();
        }
        else if (strcmp(arg, "subject-node-id-invalid") == 0)
        {
            gCertConfig.SetSubjectNodeIdInvalid();
        }
        else if (strcmp(arg, "subject-node-id-twice") == 0)
        {
            gCertConfig.SetSubjectNodeIdTwice();
        }
        else if (strcmp(arg, "subject-fabric-id-missing") == 0)
        {
            gCertConfig.SetSubjectFabricIdMissing();
        }
        else if (strcmp(arg, "subject-fabric-id-invalid") == 0)
        {
            gCertConfig.SetSubjectFabricIdInvalid();
        }
        else if (strcmp(arg, "subject-fabric-id-twice") == 0)
        {
            gCertConfig.SetSubjectFabricIdTwice();
        }
        else if (strcmp(arg, "subject-fabric-id-mismatch") == 0)
        {
            gCertConfig.SetSubjectFabricIdMismatch();
        }
        else if (strcmp(arg, "subject-cat-invalid") == 0)
        {
            gCertConfig.SetSubjectCATInvalid();
        }
        else if (strcmp(arg, "sig-curve") == 0)
        {
            gCertConfig.SetSigCurveWrong();
        }
        else if (strcmp(arg, "publickey") == 0)
        {
            gCertConfig.SetPublicKeyError();
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
        else if (strcmp(arg, "ext-extended-key-usage-missing") == 0)
        {
            gCertConfig.SetExtensionExtendedKeyUsageMissing();
        }
        else if (strcmp(arg, "signature") == 0)
        {
            gCertConfig.SetSignatureError();
        }
        else if (strcmp(arg, "no-error") != 0)
        {
            PrintArgError("%s: Invalid value specified for the error type: %s\n", progName, arg);
            return false;
        }
        break;
#endif // CHIP_CONFIG_INTERNAL_FLAG_GENERATE_OP_CERT_TEST_CASES
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
    std::unique_ptr<X509, void (*)(X509 *)> caCert(X509_new(), &X509_free);
    std::unique_ptr<EVP_PKEY, void (*)(EVP_PKEY *)> caKey(EVP_PKEY_new(), &EVP_PKEY_free);
    X509 * caCertPtr    = nullptr;
    EVP_PKEY * caKeyPtr = nullptr;

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

    if (gCertConfig.IsErrorTestCaseEnabled())
    {
        fprintf(stderr,
                "WARNING get-cert: The ignor-error option is set. This option makes it possible to generate invalid "
                "certificates.\n");
    }

    if (gSubjectDN.IsEmpty())
    {
        fprintf(stderr, "Please specify the subject DN attributes.\n");
        ExitNow(res = false);
    }

    if (gCertConfig.IsCertOversized())
    {
        // Largest CN attribute supported by ASN1 library is 64 bytes.
        const char * cn = "Common Name Subject DN Attribute for the Oversize Error Testcase";
        for (int i = 0; i < 3; i++)
        {
            err = gSubjectDN.AddAttribute_CommonName(chip::CharSpan::fromCharString(cn), false);
            if (err != CHIP_NO_ERROR)
            {
                fprintf(stderr, "Failed to add large-size CN attribute for Oversized testcase: %s\n", chip::ErrorStr(err));
                ExitNow(res = false);
            }
        }
    }

    if (!gCertConfig.IsSubjectCATValid())
    {
        err = gSubjectDN.AddAttribute_MatterCASEAuthTag(0xABCD0000);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to add Invalid CAT to the Subject DN: %s\n", chip::ErrorStr(err));
            ExitNow(res = false);
        }
    }

    if (!gCertConfig.IsErrorTestCaseEnabled())
    {
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
    }

    if (gCACertFileNameOrStr == nullptr && !gSelfSign)
    {
        fprintf(stderr, "Please specify the CA certificate using the --ca-cert option.\n");
        ExitNow(res = false);
    }
    else if (gCACertFileNameOrStr != nullptr && gSelfSign)
    {
        fprintf(stderr, "Please don't specify --ca-cert option for the self signed certificate. \n");
        ExitNow(res = false);
    }

    if (gCACertFileNameOrStr != nullptr && gCAKeyFileNameOrStr == nullptr)
    {
        fprintf(stderr, "Please specify the CA key file name using the --ca-key option.\n");
        ExitNow(res = false);
    }

    if (gOutCertFileName == nullptr)
    {
        fprintf(stderr, "Please specify the file name for the new certificate using the --out option.\n");
        ExitNow(res = false);
    }

    if (gInKeyFileNameOrStr == nullptr && gOutKeyFileName == nullptr)
    {
        fprintf(stderr, "Please specify the file name for the new public/private key using the --out-key option.\n");
        ExitNow(res = false);
    }

    if (gValidDays == kCertValidDays_Undefined)
    {
        fprintf(stderr, "Please specify the lifetime (in dys) for the new certificate using the --lifetime option.\n");
        ExitNow(res = false);
    }

    if (gPathLengthConstraint != kPathLength_NotSpecified &&
        (gCertType == kCertType_Node || gCertType == kCertType_FirmwareSigning))
    {
        fprintf(stderr, "Path length constraint shouldn't be specified for the leaf certificate.\n");
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

    if (gSelfSign)
    {
        caCertPtr = newCert.get();
        caKeyPtr  = newKey.get();
    }
    else
    {
        res = ReadCert(gCACertFileNameOrStr, caCert.get());
        VerifyTrueOrExit(res);

        res = ReadKey(gCAKeyFileNameOrStr, caKey);
        VerifyTrueOrExit(res);

        caCertPtr = caCert.get();
        caKeyPtr  = caKey.get();
    }

    res = MakeCert(gCertType, &gSubjectDN, caCertPtr, caKeyPtr, gValidFrom, gValidDays, gPathLengthConstraint, gFutureExtensions,
                   gFutureExtensionsCount, newCert.get(), newKey.get(), gCertConfig);
    VerifyTrueOrExit(res);

    if (gCertConfig.IsErrorTestCaseEnabled() && IsChipCertFormat(gOutCertFormat))
    {
        uint32_t chipCertBufLen                = kMaxCHIPCertLength + gCertConfig.GetExtraCertLength();
        std::unique_ptr<uint8_t[]> chipCertBuf = std::unique_ptr<uint8_t[]>(new uint8_t[chipCertBufLen]);
        chip::MutableByteSpan chipCert(chipCertBuf.get(), chipCertBufLen);
        err = MakeCertChipTLV(gCertType, &gSubjectDN, caCertPtr, caKeyPtr, gValidFrom, gValidDays, gPathLengthConstraint,
                              gFutureExtensions, gFutureExtensionsCount, newCert.get(), newKey.get(), gCertConfig, chipCert);
        VerifyTrueOrExit(err == CHIP_NO_ERROR);

        res = WriteChipCert(gOutCertFileName, chipCert, gOutCertFormat);
        VerifyTrueOrExit(res);
    }
    else
    {
        res = WriteCert(gOutCertFileName, newCert.get(), gOutCertFormat);
        VerifyTrueOrExit(res);
    }

    if (gOutKeyFileName != nullptr)
    {
        res = WriteKey(gOutKeyFileName, newKey.get(), gOutKeyFormat);
        VerifyTrueOrExit(res);
    }

exit:
    return res;
}
