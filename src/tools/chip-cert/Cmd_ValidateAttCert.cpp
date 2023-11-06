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
 *      This file implements the command handler for the 'chip-cert' tool
 *      that validates a CHIP attestation certificate chain.
 *
 */

#include "chip-cert.h"

#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>

namespace {

using namespace chip;
using namespace chip::ArgParser;
using namespace chip::Credentials;
using namespace chip::Crypto;
using namespace chip::ASN1;

#define CMD_NAME "chip-cert validate-att-cert"

bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg);

// clang-format off
OptionDef gCmdOptionDefs[] =
{
    { "dac",            kArgumentRequired,  'd' },
    { "pai",            kArgumentRequired,  'i' },
    { "paa",            kArgumentRequired,  'a' },
    { }
};

const char * const gCmdOptionHelp =
    "  -d, --dac <file/str>\n"
    "\n"
    "       File or string containing Device Attestation Certificate (DAC) to be validated.\n"
    "       The DAC format is auto-detected and can be any of: X.509 PEM, DER or HEX formats.\n"
    "\n"
    "  -i, --pai <file/str>\n"
    "\n"
    "       File or string containing Product Attestation Intermediate (PAI) Certificate.\n"
    "       The PAI format is auto-detected and can be any of: X.509 PEM, DER or HEX formats.\n"
    "\n"
    "  -a, --paa <file/str>\n"
    "\n"
    "       File or string containing trusted Product Attestation Authority (PAA) Certificate.\n"
    "       The PAA format is auto-detected and can be any of: X.509 PEM, DER or HEX formats.\n"
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
    "Validate a chain of CHIP attestation certificates"
);

OptionSet * gCmdOptionSets[] =
{
    &gCmdOptions,
    &gHelpOptions,
    nullptr
};
// clang-format on

const char * gDACFileNameOrStr = nullptr;
const char * gPAIFileNameOrStr = nullptr;
const char * gPAAFileNameOrStr = nullptr;

bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg)
{
    switch (id)
    {
    case 'd':
        gDACFileNameOrStr = arg;
        break;
    case 'i':
        gPAIFileNameOrStr = arg;
        break;
    case 'a':
        gPAAFileNameOrStr = arg;
        break;
    default:
        PrintArgError("%s: Unhandled option: %s\n", progName, name);
        return false;
    }

    return true;
}

AttestationVerificationResult MapError(CertificateChainValidationResult certificateChainValidationResult)
{
    switch (certificateChainValidationResult)
    {
    case CertificateChainValidationResult::kRootFormatInvalid:
        return AttestationVerificationResult::kPaaFormatInvalid;

    case CertificateChainValidationResult::kRootArgumentInvalid:
        return AttestationVerificationResult::kPaaArgumentInvalid;

    case CertificateChainValidationResult::kICAFormatInvalid:
        return AttestationVerificationResult::kPaiFormatInvalid;

    case CertificateChainValidationResult::kICAArgumentInvalid:
        return AttestationVerificationResult::kPaiArgumentInvalid;

    case CertificateChainValidationResult::kLeafFormatInvalid:
        return AttestationVerificationResult::kDacFormatInvalid;

    case CertificateChainValidationResult::kLeafArgumentInvalid:
        return AttestationVerificationResult::kDacArgumentInvalid;

    case CertificateChainValidationResult::kChainInvalid:
        return AttestationVerificationResult::kDacSignatureInvalid;

    case CertificateChainValidationResult::kNoMemory:
        return AttestationVerificationResult::kNoMemory;

    case CertificateChainValidationResult::kInternalFrameworkError:
        return AttestationVerificationResult::kInternalError;

    default:
        return AttestationVerificationResult::kInternalError;
    }
}

} // namespace

bool Cmd_ValidateAttCert(int argc, char * argv[])
{
    uint8_t dacBuf[kMaxDERCertLength] = { 0 };
    uint8_t paiBuf[kMaxDERCertLength] = { 0 };
    uint8_t paaBuf[kMaxDERCertLength] = { 0 };
    MutableByteSpan dac(dacBuf);
    MutableByteSpan pai(paiBuf);
    MutableByteSpan paa(paaBuf);

    AttestationVerificationResult attestationError = AttestationVerificationResult::kSuccess;

    if (argc == 1)
    {
        gHelpOptions.PrintBriefUsage(stderr);
        return true;
    }

    VerifyOrReturnError(ParseArgs(CMD_NAME, argc, argv, gCmdOptionSets), false);

    if (gDACFileNameOrStr == nullptr)
    {
        fprintf(stderr, "Please specify the DAC certificate using the --dac option.\n");
        return false;
    }

    if (gPAIFileNameOrStr == nullptr)
    {
        fprintf(stderr, "Please specify the PAI certificate using the --pai option.\n");
        return false;
    }

    if (gPAAFileNameOrStr == nullptr)
    {
        fprintf(stderr, "Please specify the PAA certificate using the --paa option.\n");
        return false;
    }

    if (!ReadCertDER(gDACFileNameOrStr, dac))
    {
        fprintf(stderr, "Failed to read DAC Certificate: %s\n", gDACFileNameOrStr);
        return false;
    }
    if (!ReadCertDER(gPAIFileNameOrStr, pai))
    {
        fprintf(stderr, "Failed to read PAI Certificate: %s\n", gPAIFileNameOrStr);
        return false;
    }
    if (!ReadCertDER(gPAAFileNameOrStr, paa))
    {
        fprintf(stderr, "Failed to read PAA Certificate: %s\n", gPAAFileNameOrStr);
        return false;
    }

    // Validate Proper Certificate Format
    VerifyOrExit(VerifyAttestationCertificateFormat(paa, AttestationCertType::kPAA) == CHIP_NO_ERROR,
                 attestationError = AttestationVerificationResult::kPaaFormatInvalid);
    VerifyOrExit(VerifyAttestationCertificateFormat(pai, AttestationCertType::kPAI) == CHIP_NO_ERROR,
                 attestationError = AttestationVerificationResult::kPaiFormatInvalid);
    VerifyOrExit(VerifyAttestationCertificateFormat(dac, AttestationCertType::kDAC) == CHIP_NO_ERROR,
                 attestationError = AttestationVerificationResult::kDacFormatInvalid);

    // Verify certificate is valid at the current time
    VerifyOrExit(IsCertificateValidAtCurrentTime(dac) == CHIP_NO_ERROR,
                 attestationError = AttestationVerificationResult::kDacExpired);

    // Verify that VID and PID in the certificates match.
    {
        AttestationCertVidPid dacVidPid;
        AttestationCertVidPid paiVidPid;
        AttestationCertVidPid paaVidPid;

        VerifyOrExit(ExtractVIDPIDFromX509Cert(dac, dacVidPid) == CHIP_NO_ERROR,
                     attestationError = AttestationVerificationResult::kDacFormatInvalid);
        VerifyOrExit(ExtractVIDPIDFromX509Cert(pai, paiVidPid) == CHIP_NO_ERROR,
                     attestationError = AttestationVerificationResult::kPaiFormatInvalid);
        VerifyOrExit(ExtractVIDPIDFromX509Cert(paa, paaVidPid) == CHIP_NO_ERROR,
                     attestationError = AttestationVerificationResult::kPaaFormatInvalid);

        VerifyOrExit(dacVidPid.mVendorId.HasValue() && dacVidPid.mVendorId == paiVidPid.mVendorId,
                     attestationError = AttestationVerificationResult::kDacVendorIdMismatch);

        if (paaVidPid.mVendorId.HasValue())
        {
            VerifyOrExit(dacVidPid.mVendorId == paaVidPid.mVendorId,
                         attestationError = AttestationVerificationResult::kPaiVendorIdMismatch);
        }

        if (paiVidPid.mProductId.HasValue())
        {
            VerifyOrExit(dacVidPid.mProductId == paiVidPid.mProductId,
                         attestationError = AttestationVerificationResult::kDacProductIdMismatch);
        }

        VerifyOrExit(!paaVidPid.mProductId.HasValue(), attestationError = AttestationVerificationResult::kPaaFormatInvalid);
    }

    // Validate certificate chain.
    chip::Crypto::CertificateChainValidationResult chainValidationResult;
    VerifyOrExit(ValidateCertificateChain(paa.data(), paa.size(), pai.data(), pai.size(), dac.data(), dac.size(),
                                          chainValidationResult) == CHIP_NO_ERROR,
                 attestationError = MapError(chainValidationResult));

exit:
    if (attestationError != AttestationVerificationResult::kSuccess)
    {
        fprintf(stderr, "Attestation Certificates Validation Failed with Error Code: %d\n", static_cast<int>(attestationError));
        return false;
    }

    return true;
}
