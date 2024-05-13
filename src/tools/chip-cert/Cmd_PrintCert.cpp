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
 *      that prints the contents of a CHIP certificate.
 *
 */

#include "chip-cert.h"

namespace {

using namespace chip;
using namespace chip::ArgParser;
using namespace chip::Credentials;
using namespace chip::ASN1;

#define CMD_NAME "chip-cert print-cert"

bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg);
bool HandleNonOptionArgs(const char * progName, int argc, char * const argv[]);

// clang-format off
OptionDef gCmdOptionDefs[] =
{
    { "out",           kArgumentRequired, 'o' },
    { }
};

const char * const gCmdOptionHelp =
    "   -o, --out <file/stdout>\n"
    "\n"
    "       The output printed certificate file name. If not specified\n"
    "       or if specified '-' then output is written to stdout.\n"
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
    "Usage: " CMD_NAME " [<options...>] <file/str>\n",
    CHIP_VERSION_STRING "\n" COPYRIGHT_STRING,
    "Print a CHIP operational certificate.\n"
    "\n"
    "ARGUMENTS\n"
    "\n"
    "  <file/str>\n"
    "\n"
    "       File or string containing a CHIP certificate.\n"
    "\n"
);

OptionSet *gCmdOptionSets[] =
{
    &gCmdOptions,
    &gHelpOptions,
    nullptr
};
// clang-format on

const char * gInFileNameOrStr = nullptr;
const char * gOutFileName     = "-";

bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg)
{
    switch (id)
    {
    case 'o':
        gOutFileName = arg;
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
        PrintArgError("%s: Please specify the name of the certificate to be printed.\n", progName);
        return false;
    }

    if (argc > 1)
    {
        PrintArgError("%s: Unexpected argument: %s\n", progName, argv[1]);
        return false;
    }

    gInFileNameOrStr = argv[0];

    return true;
}

void Indent(FILE * file, int count)
{
    while (count--)
    {
        fputc(' ', file);
    }
}

void PrintHexField(FILE * file, const char * name, int indent, size_t count, const uint8_t * data, size_t countPerRow = 16)
{
    Indent(file, indent);
    indent += fprintf(file, "%s: ", name);

    for (size_t i = 0; i < count; i++)
    {
        if (i != 0 && i != count && i % countPerRow == 0)
        {
            fprintf(file, "\n");
            Indent(file, indent);
        }

        fprintf(file, "%02X ", data[i]);
    }

    fprintf(file, "\n");
}

void PrintEpochTime(FILE * file, const char * name, int indent, uint32_t epochTime)
{
    chip::ASN1::ASN1UniversalTime asn1Time;

    ChipEpochToASN1Time(epochTime, asn1Time);

    Indent(file, indent);
    fprintf(file, "%s: ", name);

    fprintf(file, "0x%08" PRIX32 "  ( %04d/%02d/%02d %02d:%02d:%02d )\n", epochTime, asn1Time.Year, asn1Time.Month, asn1Time.Day,
            asn1Time.Hour, asn1Time.Minute, asn1Time.Second);
}

void PrintDN(FILE * file, const char * name, int indent, const ChipDN * dn)
{
    uint8_t rdnCount = dn->RDNCount();
    char valueStr[128];

    Indent(file, indent);
    indent += fprintf(file, "%s: [[ ", name);

    for (uint8_t i = 0; i < rdnCount; i++)
    {
        if (IsChip64bitDNAttr(dn->rdn[i].mAttrOID))
        {
            snprintf(valueStr, sizeof(valueStr), "%016" PRIX64, dn->rdn[i].mChipVal);
        }
        else if (IsChip32bitDNAttr(dn->rdn[i].mAttrOID))
        {
            snprintf(valueStr, sizeof(valueStr), "%08" PRIX32, static_cast<uint32_t>(dn->rdn[i].mChipVal));
        }
        else
        {
            size_t len = dn->rdn[i].mString.size();
            if (len > sizeof(valueStr) - 1)
            {
                len = sizeof(valueStr) - 1;
            }
            memcpy(valueStr, dn->rdn[i].mString.data(), len);
            valueStr[len] = 0;
        }

        fprintf(file, "%s = %s", chip::ASN1::GetOIDName(dn->rdn[i].mAttrOID), valueStr);
        if (i == rdnCount - 1)
        {
            fprintf(file, " ]]\n");
        }
        else
        {
            fprintf(file, ",\n");
            Indent(file, indent);
        }
    }
}

bool PrintCert(const char * fileName, X509 * cert)
{
    bool res       = true;
    CHIP_ERROR err = CHIP_NO_ERROR;
    FILE * file    = nullptr;
    ChipCertificateData certDataStorage;
    const auto certData = &certDataStorage;
    uint8_t chipCertBuf[kMaxCHIPCertLength];
    MutableByteSpan chipCert(chipCertBuf);
    int indent = 4;

    VerifyOrExit(cert != nullptr, res = false);

    res = OpenFile(fileName, file, true);
    VerifyTrueOrExit(res);

    res = X509ToChipCert(cert, chipCert);
    VerifyTrueOrExit(res);

    err = DecodeChipCert(chipCert, certDataStorage);
    if (err != CHIP_NO_ERROR)
    {
        fprintf(stderr, "Error reading %s: %s\n", fileName, chip::ErrorStr(err));
        ExitNow(res = false);
    }

    fprintf(file, "CHIP Certificate:\n");

    Indent(file, indent);
    fprintf(file, "Signature Algo  : %s\n", GetOIDName(certData->mSigAlgoOID));

    PrintDN(file, "Issuer          ", indent, &certData->mIssuerDN);

    PrintEpochTime(file, "Not Before      ", indent, certData->mNotBeforeTime);

    PrintEpochTime(file, "Not After       ", indent, certData->mNotAfterTime);

    PrintDN(file, "Subject         ", indent, &certData->mSubjectDN);

    Indent(file, indent);
    fprintf(file, "Public Key Algo : %s\n", GetOIDName(certData->mPubKeyAlgoOID));

    Indent(file, indent);
    fprintf(file, "Curve Id        : %s\n", GetOIDName(certData->mPubKeyCurveOID));

    PrintHexField(file, "Public Key      ", indent, certData->mPublicKey.size(), certData->mPublicKey.data());

    Indent(file, indent);
    fprintf(file, "Extensions:\n");

    indent += 4;
    Indent(file, indent);
    fprintf(file, "Is CA            : %s\n", certData->mCertFlags.Has(CertFlags::kIsCA) ? "true" : "false");

    if (certData->mCertFlags.Has(CertFlags::kPathLenConstraintPresent))
    {
        Indent(file, indent);
        fprintf(file, "Path Length Const: %u\n", (unsigned) certData->mPathLenConstraint);
    }

    if (certData->mCertFlags.Has(CertFlags::kExtPresent_KeyUsage))
    {
        Indent(file, indent);
        fprintf(file, "Key Usage        : ");
        if (certData->mKeyUsageFlags.Has(KeyUsageFlags::kDigitalSignature))
        {
            fprintf(file, "DigitalSignature ");
        }
        if (certData->mKeyUsageFlags.Has(KeyUsageFlags::kNonRepudiation))
        {
            fprintf(file, "NonRepudiation ");
        }
        if (certData->mKeyUsageFlags.Has(KeyUsageFlags::kKeyEncipherment))
        {
            fprintf(file, "KeyEncipherment ");
        }
        if (certData->mKeyUsageFlags.Has(KeyUsageFlags::kDataEncipherment))
        {
            fprintf(file, "DataEncipherment ");
        }
        if (certData->mKeyUsageFlags.Has(KeyUsageFlags::kKeyAgreement))
        {
            fprintf(file, "KeyAgreement ");
        }
        if (certData->mKeyUsageFlags.Has(KeyUsageFlags::kKeyCertSign))
        {
            fprintf(file, "KeyCertSign ");
        }
        if (certData->mKeyUsageFlags.Has(KeyUsageFlags::kCRLSign))
        {
            fprintf(file, "CRLSign ");
        }
        if (certData->mKeyUsageFlags.Has(KeyUsageFlags::kEncipherOnly))
        {
            fprintf(file, "EncipherOnly ");
        }
        if (certData->mKeyUsageFlags.Has(KeyUsageFlags::kDecipherOnly))
        {
            fprintf(file, "DecipherOnly ");
        }
        fprintf(file, "\n");
    }

    if (certData->mCertFlags.Has(CertFlags::kExtPresent_ExtendedKeyUsage))
    {
        Indent(file, indent);
        fprintf(file, "Key Purpose      : ");
        if (certData->mKeyPurposeFlags.Has(KeyPurposeFlags::kServerAuth))
        {
            fprintf(file, "ServerAuth ");
        }
        if (certData->mKeyPurposeFlags.Has(KeyPurposeFlags::kClientAuth))
        {
            fprintf(file, "ClientAuth ");
        }
        if (certData->mKeyPurposeFlags.Has(KeyPurposeFlags::kCodeSigning))
        {
            fprintf(file, "CodeSigning ");
        }
        if (certData->mKeyPurposeFlags.Has(KeyPurposeFlags::kEmailProtection))
        {
            fprintf(file, "EmailProtection ");
        }
        if (certData->mKeyPurposeFlags.Has(KeyPurposeFlags::kTimeStamping))
        {
            fprintf(file, "TimeStamping ");
        }
        if (certData->mKeyPurposeFlags.Has(KeyPurposeFlags::kOCSPSigning))
        {
            fprintf(file, "OCSPSigning ");
        }
        fprintf(file, "\n");
    }

    if (certData->mCertFlags.Has(CertFlags::kExtPresent_SubjectKeyId))
    {
        PrintHexField(file, "Subject Key Id   ", indent, certData->mSubjectKeyId.size(), certData->mSubjectKeyId.data(),
                      certData->mSubjectKeyId.size());
    }

    if (certData->mCertFlags.Has(CertFlags::kExtPresent_AuthKeyId))
    {
        PrintHexField(file, "Authority Key Id ", indent, certData->mAuthKeyId.size(), certData->mAuthKeyId.data(),
                      certData->mAuthKeyId.size());
    }

    indent -= 4;
    PrintHexField(file, "Signature       ", indent, certData->mSignature.size(), certData->mSignature.data());

exit:
    CloseFile(file);
    return res;
}

} // namespace

bool Cmd_PrintCert(int argc, char * argv[])
{
    bool res = true;
    std::unique_ptr<X509, void (*)(X509 *)> cert(nullptr, &X509_free);

    if (argc == 1)
    {
        gHelpOptions.PrintBriefUsage(stderr);
        ExitNow(res = true);
    }

    res = ParseArgs(CMD_NAME, argc, argv, gCmdOptionSets, HandleNonOptionArgs);
    VerifyTrueOrExit(res);

    res = ReadCert(gInFileNameOrStr, cert);
    VerifyTrueOrExit(res);

    res = PrintCert(gOutFileName, cert.get());
    VerifyTrueOrExit(res);

exit:
    return res;
}
