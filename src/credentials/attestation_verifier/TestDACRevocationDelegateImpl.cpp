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

#include <algorithm>
#include <fstream>
#include <json/json.h>

using namespace chip::Crypto;

namespace chip {
namespace Credentials {

namespace {

static constexpr uint32_t kMaxIssuerBase64Len = BASE64_ENCODED_LEN(kMaxCertificateDistinguishedNameLength);

CHIP_ERROR BytesToHexStr(const ByteSpan & bytes, MutableCharSpan & outHexStr)
{
    Encoding::HexFlags flags = Encoding::HexFlags::kUppercase;
    ReturnErrorOnFailure(BytesToHex(bytes.data(), bytes.size(), outHexStr.data(), outHexStr.size(), flags));
    outHexStr.reduce_size(2 * bytes.size());
    return CHIP_NO_ERROR;
}

CHIP_ERROR X509_PemToDer(const std::string & pemCert, MutableByteSpan & derCert)
{
    std::string beginMarker = "-----BEGIN CERTIFICATE-----";
    std::string endMarker   = "-----END CERTIFICATE-----";

    std::size_t beginPos = pemCert.find(beginMarker);
    VerifyOrReturnError(beginPos != std::string::npos, CHIP_ERROR_INVALID_ARGUMENT);

    std::size_t endPos = pemCert.find(endMarker);
    VerifyOrReturnError(endPos != std::string::npos, CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrReturnError(beginPos < endPos, CHIP_ERROR_INVALID_ARGUMENT);

    // Extract content between markers
    std::string plainB64Str = pemCert.substr(beginPos + beginMarker.length(), endPos - (beginPos + beginMarker.length()));

    // Remove all newline characters '\n' and '\r'
    plainB64Str.erase(std::remove(plainB64Str.begin(), plainB64Str.end(), '\n'), plainB64Str.end());
    plainB64Str.erase(std::remove(plainB64Str.begin(), plainB64Str.end(), '\r'), plainB64Str.end());

    VerifyOrReturnError(!plainB64Str.empty(), CHIP_ERROR_INVALID_ARGUMENT);

    // Verify we have enough room to store the decoded certificate
    size_t maxDecodeLen = BASE64_MAX_DECODED_LEN(plainB64Str.size());
    VerifyOrReturnError(derCert.size() >= maxDecodeLen, CHIP_ERROR_BUFFER_TOO_SMALL);

    // decode b64
    uint16_t derLen = Base64Decode(plainB64Str.c_str(), static_cast<uint16_t>(plainB64Str.size()), derCert.data());
    VerifyOrReturnError(derLen != UINT16_MAX, CHIP_ERROR_INVALID_ARGUMENT);

    derCert.reduce_size(derLen);

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

// outSubject is subject encoded as base64 string
// outKeyId is SKID encoded as hex string
CHIP_ERROR TestDACRevocationDelegateImpl::GetSubjectAndKeyIdFromPEMCert(const std::string & certPEM, std::string & outSubject,
                                                                        std::string & outKeyId)
{
    // buffers and spans for storing crl signer delegator OR crl signer cert info
    char subjectBuf[kMaxIssuerBase64Len]             = { 0 };
    char skidBuf[2 * kAuthorityKeyIdentifierLength]  = { 0 };
    uint8_t certDerBuf[kMax_x509_Certificate_Length] = { 0 };

    MutableCharSpan subject(subjectBuf);
    MutableCharSpan keyId(skidBuf);
    MutableByteSpan certDER(certDerBuf);

    ReturnLogErrorOnFailure(X509_PemToDer(certPEM, certDER));
    ReturnErrorOnFailure(GetSubjectNameBase64Str(certDER, subject));
    ReturnErrorOnFailure(GetSKIDHexStr(certDER, keyId));

    outSubject = std::string(subject.data(), subject.size());
    outKeyId   = std::string(keyId.data(), keyId.size());

    return CHIP_NO_ERROR;
}

// Check if issuer and AKID matches with the crl signer OR crl signer delegator's subject and SKID
bool TestDACRevocationDelegateImpl::CrossValidateCert(const Json::Value & revokedSet, const std::string & akidHexStr,
                                                      const std::string & issuerNameBase64Str)
{
    std::string certPEM;
    [[maybe_unused]] std::string certType;

    if (revokedSet.isMember("crl_signer_delegator"))
    {
        certPEM  = revokedSet["crl_signer_delegator"].asString();
        certType = "CRL Signer delegator";
    }
    else
    {
        certPEM  = revokedSet["crl_signer_cert"].asString();
        certType = "CRL Signer";
    }

    std::string subject; // crl signer or crl signer delegator subject
    std::string keyId;   // crl signer or crl signer delegator SKID
    VerifyOrReturnValue(CHIP_NO_ERROR == GetSubjectAndKeyIdFromPEMCert(certPEM, subject, keyId), false);

    ChipLogDetail(NotSpecified, "%s: Subject: %s", certType.c_str(), subject.c_str());
    ChipLogDetail(NotSpecified, "%s: SKID: %s", certType.c_str(), keyId.c_str());

    return (akidHexStr == keyId && issuerNameBase64Str == subject);
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
//     "crl_signer_cert": "<PEM incoded CRL signer certificate>",
//     "crl_signer_delegator": <PEM incoded CRL signer delegator certificate>,
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

    // 6.2.4.2. Determining Revocation Status of an Entity
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

        // 4.a cross validate PAI with crl signer OR crl signer delegator
        // 4.b cross validate DAC with crl signer OR crl signer delegator
        VerifyOrReturnValue(CrossValidateCert(revokedSet, akid, issuerName), false);

        // 4.c check if serial number is revoked
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

CHIP_ERROR TestDACRevocationDelegateImpl::GetKeyIDHexStr(const ByteSpan & certDer, MutableCharSpan & outKeyIDHexStr, bool isAKID)
{
    static_assert(kAuthorityKeyIdentifierLength == kSubjectKeyIdentifierLength, "AKID and SKID length mismatch");

    uint8_t keyIdBuf[kAuthorityKeyIdentifierLength];
    MutableByteSpan keyId(keyIdBuf);

    if (isAKID)
    {
        ReturnErrorOnFailure(ExtractAKIDFromX509Cert(certDer, keyId));
    }
    else
    {
        ReturnErrorOnFailure(ExtractSKIDFromX509Cert(certDer, keyId));
    }

    return BytesToHexStr(keyId, outKeyIDHexStr);
}

CHIP_ERROR TestDACRevocationDelegateImpl::GetAKIDHexStr(const ByteSpan & certDer, MutableCharSpan & outAKIDHexStr)
{
    return GetKeyIDHexStr(certDer, outAKIDHexStr, true);
}

CHIP_ERROR TestDACRevocationDelegateImpl::GetSKIDHexStr(const ByteSpan & certDer, MutableCharSpan & outSKIDHexStr)
{
    return GetKeyIDHexStr(certDer, outSKIDHexStr, false /* isAKID */);
}

CHIP_ERROR TestDACRevocationDelegateImpl::GetSerialNumberHexStr(const ByteSpan & certDer, MutableCharSpan & outSerialNumberHexStr)
{
    uint8_t serialNumberBuf[kMaxCertificateSerialNumberLength] = { 0 };
    MutableByteSpan serialNumber(serialNumberBuf);

    ReturnErrorOnFailure(ExtractSerialNumberFromX509Cert(certDer, serialNumber));
    return BytesToHexStr(serialNumber, outSerialNumberHexStr);
}

CHIP_ERROR TestDACRevocationDelegateImpl::GetRDNBase64Str(const ByteSpan & certDer, MutableCharSpan & outRDNBase64String,
                                                          bool isIssuer)
{
    uint8_t rdnBuf[kMaxCertificateDistinguishedNameLength] = { 0 };
    MutableByteSpan rdn(rdnBuf);

    if (isIssuer)
    {
        ReturnErrorOnFailure(ExtractIssuerFromX509Cert(certDer, rdn));
    }
    else
    {
        ReturnErrorOnFailure(ExtractSubjectFromX509Cert(certDer, rdn));
    }

    VerifyOrReturnError(outRDNBase64String.size() >= BASE64_ENCODED_LEN(rdn.size()), CHIP_ERROR_BUFFER_TOO_SMALL);

    uint16_t encodedLen = Base64Encode(rdn.data(), static_cast<uint16_t>(rdn.size()), outRDNBase64String.data());
    outRDNBase64String.reduce_size(encodedLen);
    return CHIP_NO_ERROR;
}

CHIP_ERROR TestDACRevocationDelegateImpl::GetIssuerNameBase64Str(const ByteSpan & certDer,
                                                                 MutableCharSpan & outIssuerNameBase64String)
{
    return GetRDNBase64Str(certDer, outIssuerNameBase64String, true /* isIssuer */);
}

CHIP_ERROR TestDACRevocationDelegateImpl::GetSubjectNameBase64Str(const ByteSpan & certDer,
                                                                  MutableCharSpan & outSubjectNameBase64String)
{
    return GetRDNBase64Str(certDer, outSubjectNameBase64String, false /* isIssuer */);
}

// @param certDer Certificate, in DER format, to check for revocation
bool TestDACRevocationDelegateImpl::IsCertificateRevoked(const ByteSpan & certDer)
{
    char issuerNameBuffer[kMaxIssuerBase64Len]                           = { 0 };
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
        return;
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
