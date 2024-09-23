/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <credentials/attestation_verifier/TestDACRevocationDelegateImpl.h>
#include <lib/support/Base64.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/logging/CHIPLogging.h>

#include <fstream>
#include <json/json.h>

using namespace chip::Crypto;

namespace chip {
namespace Credentials {

namespace {
CHIP_ERROR BytesToHexStr(const ByteSpan & bytes, MutableCharSpan & outHexStr)
{
    Encoding::HexFlags flags = Encoding::HexFlags::kUppercase;
    ReturnErrorOnFailure(BytesToHex(bytes.data(), bytes.size(), outHexStr.data(), outHexStr.size(), flags));
    outHexStr.reduce_size(2 * bytes.size());
    return CHIP_NO_ERROR;
}
} // anonymous namespace

CHIP_ERROR TestDACRevocationDelegateImpl::SetDeviceAttestationRevocationSetPath(std::string_view path)
{
    VerifyOrReturnError(path.empty() != true, CHIP_ERROR_INVALID_ARGUMENT);
    mDeviceAttestationRevocationSetPath = path;
    return CHIP_NO_ERROR;
}

void TestDACRevocationDelegateImpl::ClearDeviceAttestationRevocationSetPath()
{
    // clear the string_view
    mDeviceAttestationRevocationSetPath = mDeviceAttestationRevocationSetPath.substr(0, 0);
}

// This method parses the below JSON Scheme
// [
//   {
//     "type": "revocation_set",
//     "issuer_subject_key_id": "<issuer subject key ID as uppercase hex, 20 bytes>",
//     "issuer_name": "<ASN.1 SEQUENCE of Issuer of the CRL as base64>",
//     "revoked_serial_numbers: [
//       "serial1 bytes as base64",
//       "serial2 bytes as base64"
//     ]
//   }
// ]
//
bool TestDACRevocationDelegateImpl::IsEntryInRevocationSet(const CharSpan & akidHexStr, const CharSpan & issuerNameBase64Str,
                                                           const CharSpan & serialNumberHexStr)
{
    std::ifstream file(mDeviceAttestationRevocationSetPath.c_str());
    if (!file.is_open())
    {
        ChipLogError(NotSpecified, "Failed to open file: %s", mDeviceAttestationRevocationSetPath.c_str());
        return false;
    }

    // Parse the JSON data incrementally
    Json::CharReaderBuilder readerBuilder;
    Json::Value jsonData;
    std::string errs;

    bool parsingSuccessful = Json::parseFromStream(readerBuilder, file, &jsonData, &errs);

    // Close the file as it's no longer needed
    file.close();

    if (!parsingSuccessful)
    {
        ChipLogError(NotSpecified, "Failed to parse JSON: %s", errs.c_str());
        return false;
    }

    std::string issuerName   = std::string(issuerNameBase64Str.data(), issuerNameBase64Str.size());
    std::string serialNumber = std::string(serialNumberHexStr.data(), serialNumberHexStr.size());
    std::string akid         = std::string(akidHexStr.data(), akidHexStr.size());

    for (const auto & revokedSet : jsonData)
    {
        if (revokedSet["issuer_name"].asString() != issuerName)
        {
            continue;
        }
        if (revokedSet["issuer_subject_key_id"].asString() != akid)
        {
            continue;
        }
        for (const auto & revokedSerialNumber : revokedSet["revoked_serial_numbers"])
        {
            if (revokedSerialNumber.asString() == serialNumber)
            {
                return true;
            }
        }
    }
    return false;
}

CHIP_ERROR TestDACRevocationDelegateImpl::GetAKIDHexStr(const ByteSpan & certDer, MutableCharSpan & outAKIDHexStr)
{
    uint8_t akidBuf[kAuthorityKeyIdentifierLength];
    MutableByteSpan akid(akidBuf);

    ReturnErrorOnFailure(ExtractAKIDFromX509Cert(certDer, akid));

    return BytesToHexStr(akid, outAKIDHexStr);
}

CHIP_ERROR TestDACRevocationDelegateImpl::GetSerialNumberHexStr(const ByteSpan & certDer, MutableCharSpan & outSerialNumberHexStr)
{
    uint8_t serialNumberBuf[kMaxCertificateSerialNumberLength] = { 0 };
    MutableByteSpan serialNumber(serialNumberBuf);

    ReturnErrorOnFailure(ExtractSerialNumberFromX509Cert(certDer, serialNumber));
    return BytesToHexStr(serialNumber, outSerialNumberHexStr);
}

CHIP_ERROR TestDACRevocationDelegateImpl::GetIssuerNameBase64Str(const ByteSpan & certDer,
                                                                 MutableCharSpan & outIssuerNameBase64String)
{
    uint8_t issuerBuf[kMaxCertificateDistinguishedNameLength] = { 0 };
    MutableByteSpan issuer(issuerBuf);

    ReturnErrorOnFailure(ExtractIssuerFromX509Cert(certDer, issuer));
    VerifyOrReturnError(outIssuerNameBase64String.size() >= BASE64_ENCODED_LEN(issuer.size()), CHIP_ERROR_BUFFER_TOO_SMALL);

    uint16_t encodedLen = Base64Encode(issuer.data(), static_cast<uint16_t>(issuer.size()), outIssuerNameBase64String.data());
    outIssuerNameBase64String.reduce_size(encodedLen);
    return CHIP_NO_ERROR;
}

bool TestDACRevocationDelegateImpl::IsCertificateRevoked(const ByteSpan & certDer)
{
    static constexpr uint32_t maxIssuerBase64Len = BASE64_ENCODED_LEN(kMaxCertificateDistinguishedNameLength);

    char issuerNameBuffer[maxIssuerBase64Len]                            = { 0 };
    char serialNumberHexStrBuffer[2 * kMaxCertificateSerialNumberLength] = { 0 };
    char akidHexStrBuffer[2 * kAuthorityKeyIdentifierLength]             = { 0 };

    MutableCharSpan issuerName(issuerNameBuffer);
    MutableCharSpan serialNumber(serialNumberHexStrBuffer);
    MutableCharSpan akid(akidHexStrBuffer);

    VerifyOrReturnValue(CHIP_NO_ERROR == GetIssuerNameBase64Str(certDer, issuerName), false);
    ChipLogDetail(NotSpecified, "Issuer: %.*s", static_cast<int>(issuerName.size()), issuerName.data());

    VerifyOrReturnValue(CHIP_NO_ERROR == GetSerialNumberHexStr(certDer, serialNumber), false);
    ChipLogDetail(NotSpecified, "Serial Number: %.*s", static_cast<int>(serialNumber.size()), serialNumber.data());

    VerifyOrReturnValue(CHIP_NO_ERROR == GetAKIDHexStr(certDer, akid), false);
    ChipLogDetail(NotSpecified, "AKID: %.*s", static_cast<int>(akid.size()), akid.data());

    // TODO: Cross-validate the CRLSignerCertificate and CRLSignerDelegator per spec: #34587

    return IsEntryInRevocationSet(akid, issuerName, serialNumber);
}

void TestDACRevocationDelegateImpl::CheckForRevokedDACChain(
    const DeviceAttestationVerifier::AttestationInfo & info,
    Callback::Callback<DeviceAttestationVerifier::OnAttestationInformationVerification> * onCompletion)
{
    AttestationVerificationResult attestationError = AttestationVerificationResult::kSuccess;

    if (mDeviceAttestationRevocationSetPath.empty())
    {

        onCompletion->mCall(onCompletion->mContext, info, attestationError);
    }

    ChipLogDetail(NotSpecified, "Checking for revoked DAC in %s", mDeviceAttestationRevocationSetPath.c_str());

    if (IsCertificateRevoked(info.dacDerBuffer))
    {
        ChipLogProgress(NotSpecified, "Found revoked DAC in %s", mDeviceAttestationRevocationSetPath.c_str());
        attestationError = AttestationVerificationResult::kDacRevoked;
    }

    ChipLogDetail(NotSpecified, "Checking for revoked PAI in %s", mDeviceAttestationRevocationSetPath.c_str());

    if (IsCertificateRevoked(info.paiDerBuffer))
    {
        ChipLogProgress(NotSpecified, "Found revoked PAI in %s", mDeviceAttestationRevocationSetPath.c_str());

        if (attestationError == AttestationVerificationResult::kDacRevoked)
        {
            attestationError = AttestationVerificationResult::kPaiAndDacRevoked;
        }
        else
        {
            attestationError = AttestationVerificationResult::kPaiRevoked;
        }
    }

    onCompletion->mCall(onCompletion->mContext, info, attestationError);
}

} // namespace Credentials
} // namespace chip
