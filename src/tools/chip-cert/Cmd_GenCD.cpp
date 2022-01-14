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
 *      that generates a CHIP Certification Declaration.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include "chip-cert.h"

#include <credentials/CertificationDeclaration.h>

namespace {

using namespace chip;
using namespace chip::ArgParser;
using namespace chip::Credentials;
using namespace chip::Crypto;

#define CMD_NAME "chip-cert gen-cd"

bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg);

// clang-format off
OptionDef gCmdOptionDefs[] =
{
    { "key",                   kArgumentRequired, 'K' },
    { "cert",                  kArgumentRequired, 'C' },
    { "out",                   kArgumentRequired, 'O' },
    { "format-version",        kArgumentRequired, 'f' },
    { "vendor-id",             kArgumentRequired, 'V' },
    { "product-id",            kArgumentRequired, 'p' },
    { "device-type-id",        kArgumentRequired, 'd' },
    { "certificate-id",        kArgumentRequired, 'c' },
    { "security-level",        kArgumentRequired, 'l' },
    { "security-info",         kArgumentRequired, 'i' },
    { "version-number",        kArgumentRequired, 'n' },
    { "certification-type",    kArgumentRequired, 't' },
    { "dac-origin-vendor-id",  kArgumentRequired, 'o' },
    { "dac-origin-product-id", kArgumentRequired, 'r' },
    { }
};

const char * const gCmdOptionHelp =
    "   -K, --key <file>\n"
    "\n"
    "       File containing private key to be used to sign the Certification Declaration.\n"
    "\n"
    "   -C, --cert <file>\n"
    "\n"
    "       File containing certificate associated with the private key that is used\n"
    "       to sign the Certification Declaration. The Subject Key Identifier in the\n"
    "       certificate will be included in the signed Certification Declaration message.\n"
    "\n"
    "   -O, --out <file>\n"
    "\n"
    "       File to contain the signed Certification Declaration message.\n"
    "\n"
    "   -f, --format-version <int>\n"
    "\n"
    "       Format Version.\n"
    "\n"
    "   -V, --vendor-id <hex-digits>\n"
    "\n"
    "       Vendor Id (VID) in hex.\n"
    "\n"
    "   -p, --product-id <hex-digits>\n"
    "\n"
    "       Product Id (PID) in hex. Maximum 100 PID values can be specified.\n"
    "       Each PID value should have it's own -p or --product-id option selector.\n"
    "\n"
    "   -d, --device-type-id <hex-digits>\n"
    "\n"
    "       Device Type Id in hex.\n"
    "\n"
    "   -c, --certificate-id <string>\n"
    "\n"
    "       Certificate Id encoded as UTF8 string.\n"
    "\n"
    "   -l, --security-level <hex-digits>\n"
    "\n"
    "       Security Level in hex.\n"
    "\n"
    "   -i, --security-info <hex-digits>\n"
    "\n"
    "       Security Information in hex.\n"
    "\n"
    "   -n, --version-number <hex-digits>\n"
    "\n"
    "       Version Number in hex.\n"
    "\n"
    "   -t, --certification-type <int>\n"
    "\n"
    "       Certification Type. Valid values are:\n"
    "           0 - Development and Test (default)\n"
    "           1 - Provisional\n"
    "           2 - Official\n"
    "\n"
    "   -o, --dac-origin-vendor-id <hex-digits>\n"
    "\n"
    "       DAC Origin Vendor Id in hex.\n"
    "\n"
    "   -r, --dac-origin-product-id <hex-digits>\n"
    "\n"
    "       DAC Origin Product Id in hex.\n"
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
    "Generate CD CMS Signed Message"
);

OptionSet *gCmdOptionSets[] =
{
    &gCmdOptions,
    &gHelpOptions,
    nullptr
};
// clang-format on

CertificationElements gCertElements = { 0 };
const char * gCertFileName          = nullptr;
const char * gKeyFileName           = nullptr;
const char * gSignedCDFileName      = nullptr;

bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg)
{
    switch (id)
    {
    case 'C':
        gCertFileName = arg;
        break;
    case 'K':
        gKeyFileName = arg;
        break;
    case 'O':
        gSignedCDFileName = arg;
        break;
    case 'f':
        if (!ParseInt(arg, gCertElements.FormatVersion, 16))
        {
            PrintArgError("%s: Invalid value specified for Format Version: %s\n", progName, arg);
            return false;
        }
        break;
    case 'V':
        if (!ParseInt(arg, gCertElements.VendorId, 16) || gCertElements.VendorId == 0)
        {
            PrintArgError("%s: Invalid value specified for Vendor Id: %s\n", progName, arg);
            return false;
        }
        break;
    case 'p':
        if (gCertElements.ProductIdsCount == ArraySize(gCertElements.ProductIds))
        {
            PrintArgError("%s: Too many Product Ids are specified: %s\n", progName, arg);
            return false;
        }
        if (!ParseInt(arg, gCertElements.ProductIds[gCertElements.ProductIdsCount], 16) ||
            gCertElements.ProductIds[gCertElements.ProductIdsCount] == 0)
        {
            PrintArgError("%s: Invalid value specified for Product Id: %s\n", progName, arg);
            return false;
        }
        gCertElements.ProductIdsCount++;
        break;
    case 'd':
        if (!ParseInt(arg, gCertElements.DeviceTypeId, 16))
        {
            PrintArgError("%s: Invalid value specified for Device Type Id: %s\n", progName, arg);
            return false;
        }
        break;
    case 'c':
        if (strlen(arg) != kCertificateIdLength)
        {
            PrintArgError("%s: Invalid value specified for Certificate Id: %s\n", progName, arg);
            return false;
        }
        memcpy(gCertElements.CertificateId, arg, strlen(arg));
        gCertElements.CertificateId[kCertificateIdLength] = '\0';
        break;
    case 'l':
        if (!ParseInt(arg, gCertElements.SecurityLevel, 16))
        {
            PrintArgError("%s: Invalid value specified for Security Level: %s\n", progName, arg);
            return false;
        }
        break;
    case 'i':
        if (!ParseInt(arg, gCertElements.SecurityInformation, 16))
        {
            PrintArgError("%s: Invalid value specified for Security Information: %s\n", progName, arg);
            return false;
        }
        break;
    case 'n':
        if (!ParseInt(arg, gCertElements.VersionNumber, 16))
        {
            PrintArgError("%s: Invalid value specified for Version Number: %s\n", progName, arg);
            return false;
        }
        break;
    case 't':
        if (!ParseInt(arg, gCertElements.CertificationType) || gCertElements.CertificationType > 2)
        {
            PrintArgError("%s: Invalid value specified for Certification Type: %s\n", progName, arg);
            return false;
        }
        break;
    case 'o':
        if (!ParseInt(arg, gCertElements.DACOriginVendorId, 16) || gCertElements.DACOriginVendorId == 0)
        {
            PrintArgError("%s: Invalid value specified for DAC Origin Vendor Id: %s\n", progName, arg);
            return false;
        }
        gCertElements.DACOriginVIDandPIDPresent = true;
        break;
    case 'r':
        if (!ParseInt(arg, gCertElements.DACOriginProductId, 16) || gCertElements.DACOriginProductId == 0)
        {
            PrintArgError("%s: Invalid value specified for DAC Origin Product Id: %s\n", progName, arg);
            return false;
        }
        gCertElements.DACOriginVIDandPIDPresent = true;
        break;
    default:
        PrintArgError("%s: Unhandled option: %s\n", progName, name);
        return false;
    }

    return true;
}

} // namespace

bool Cmd_GenCD(int argc, char * argv[])
{
    if (argc == 1)
    {
        gHelpOptions.PrintBriefUsage(stderr);
        return true;
    }

    VerifyOrReturnError(ParseArgs(CMD_NAME, argc, argv, gCmdOptionSets), false);

    if (gKeyFileName == nullptr)
    {
        fprintf(stderr, "Please specify the signing private key file name using the --key option.\n");
        return false;
    }

    if (gCertFileName == nullptr)
    {
        fprintf(stderr, "Please specify the signing certificate file name using the --cert option.\n");
        return false;
    }

    if (gSignedCDFileName == nullptr)
    {
        fprintf(stderr, "Please specify the file name for the signed Certification Declaration using the --out option.\n");
        return false;
    }

    if (gCertElements.VendorId == 0 || gCertElements.ProductIdsCount == 0 || gCertElements.DeviceTypeId == 0 ||
        strlen(gCertElements.CertificateId) == 0 || gCertElements.VersionNumber == 0)
    {
        fprintf(stderr, "Please specify all mandatory CD elements.\n");
        return false;
    }

    if (gCertElements.DACOriginVIDandPIDPresent && (gCertElements.DACOriginVendorId == 0 || gCertElements.DACOriginProductId == 0))
    {
        fprintf(stderr, "The DAC Origin Vendor Id and Product Id SHALL be specified together.\n");
        return false;
    }

    if (strcmp(gSignedCDFileName, "-") != 0 && access(gSignedCDFileName, R_OK) == 0)
    {
        fprintf(stderr,
                "Output signed CD file already exists (%s)\n"
                "To replace the file, please remove it and re-run the command.\n",
                gSignedCDFileName);
        return false;
    }

    {
        std::unique_ptr<X509, void (*)(X509 *)> cert(X509_new(), &X509_free);
        std::unique_ptr<EVP_PKEY, void (*)(EVP_PKEY *)> key(EVP_PKEY_new(), &EVP_PKEY_free);

        VerifyOrReturnError(ReadCert(gCertFileName, cert.get()), false);
        VerifyOrReturnError(ReadKey(gKeyFileName, key.get()), false);

        // Extract the subject key id from the X509 certificate.
        ByteSpan signerKeyId;
        {
            const ASN1_OCTET_STRING * skidString = X509_get0_subject_key_id(cert.get());
            VerifyOrReturnError(skidString != nullptr, false);
            VerifyOrReturnError(CanCastTo<size_t>(skidString->length), false);
            signerKeyId = ByteSpan(skidString->data, static_cast<size_t>(skidString->length));
        }

        // Initialize P256Keypair from EVP_PKEY.
        P256Keypair keypair;
        {
            P256SerializedKeypair serializedKeypair;
            VerifyOrReturnError(SerializeKeyPair(key.get(), serializedKeypair), false);
            VerifyOrReturnError(keypair.Deserialize(serializedKeypair) == CHIP_NO_ERROR, false);
        }

        // Encode CD TLV content.
        uint8_t encodedCDBuf[kCertificationElements_TLVEncodedMaxLength];
        MutableByteSpan encodedCD(encodedCDBuf);
        VerifyOrReturnError(EncodeCertificationElements(gCertElements, encodedCD) == CHIP_NO_ERROR, false);

        // Sign CD.
        uint8_t signedMessageBuf[kMaxCMSSignedCDMessage];
        MutableByteSpan signedMessage(signedMessageBuf);
        VerifyOrReturnError(CMS_Sign(encodedCD, signerKeyId, keypair, signedMessage) == CHIP_NO_ERROR, false);

        // Write to file.
        {
            FILE * file = nullptr;

            VerifyOrReturnError(OpenFile(gSignedCDFileName, file, true), false);

            if (fwrite(signedMessage.data(), 1, signedMessage.size(), file) != signedMessage.size())
            {
                fprintf(stderr, "Unable to write to %s: %s\n", gSignedCDFileName, strerror(ferror(file) ? errno : ENOSPC));
                return false;
            }
        }
    }
    return true;
}
