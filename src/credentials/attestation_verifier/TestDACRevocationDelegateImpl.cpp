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

CHIP_ERROR BytesToHexStr(const ByteSpan & bytes, std::string & outHexStr)
{
    size_t hexLength = bytes.size() * 2;
    outHexStr.resize(hexLength);

    Encoding::HexFlags flags = Encoding::HexFlags::kUppercase;
    return BytesToHex(bytes.data(), bytes.size(), &outHexStr[0], hexLength, flags);
}

} // anonymous namespace

CHIP_ERROR TestDACRevocationDelegateImpl::SetDeviceAttestationRevocationSetPath(std::string_view path)
{
    VerifyOrReturnError(path.empty() != true, CHIP_ERROR_INVALID_ARGUMENT);
    mDeviceAttestationRevocationSetPath = path;
    return CHIP_NO_ERROR;
}

CHIP_ERROR TestDACRevocationDelegateImpl::SetDeviceAttestationRevocationData(const std::string & jsonData)
{
    mRevocationData = jsonData;
    return CHIP_NO_ERROR;
}

void TestDACRevocationDelegateImpl::ClearDeviceAttestationRevocationSetPath()
{
    // clear the string_view
    mDeviceAttestationRevocationSetPath = mDeviceAttestationRevocationSetPath.substr(0, 0);
}

void TestDACRevocationDelegateImpl::ClearDeviceAttestationRevocationData()
{
    mRevocationData.clear();
}

// Check if issuer and AKID matches with the crl signer OR crl signer delegator's subject and SKID
bool TestDACRevocationDelegateImpl::CrossValidateCert(const Json::Value & revokedSet, const std::string & akidHexStr,
                                                      const std::string & issuerNameBase64Str)
{
    std::string certBase64;
    [[maybe_unused]] std::string certType;

    if (revokedSet.isMember("crl_signer_delegator"))
    {
        certBase64 = revokedSet["crl_signer_delegator"].asString();
        certType   = "CRL Signer delegator";
    }
    else
    {
        certBase64 = revokedSet["crl_signer_cert"].asString();
        certType   = "CRL Signer";
    }

    uint8_t certDerBuf[kMax_x509_Certificate_Length] = { 0 };
    MutableByteSpan certDER(certDerBuf);

    // Verify we have enough room to store the decoded certificate
    size_t maxDecodeLen = BASE64_MAX_DECODED_LEN(certBase64.size());
    VerifyOrReturnValue(certDER.size() >= maxDecodeLen, false);

    uint16_t derLen = Base64Decode(certBase64.c_str(), static_cast<uint16_t>(certBase64.size()), certDER.data());
    VerifyOrReturnValue(derLen != UINT16_MAX, false);
    certDER.reduce_size(derLen);

    std::string subject;
    std::string keyId;

    VerifyOrReturnValue(CHIP_NO_ERROR == GetSubjectNameBase64Str(certDER, subject), false);
    VerifyOrReturnValue(CHIP_NO_ERROR == GetSKIDHexStr(certDER, keyId), false);

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
//     "crl_signer_cert": "<base64 encoded DER certificate>",
//     "crl_signer_delegator": "<base64 encoded DER certificate>",
//   }
// ]
//
bool TestDACRevocationDelegateImpl::IsEntryInRevocationSet(const std::string & akidHexStr, const std::string & issuerNameBase64Str,
                                                           const std::string & serialNumberHexStr)
{
    Json::CharReaderBuilder readerBuilder;
    Json::Value jsonData;
    std::string errs;

    // Try direct data first, then fall back to file

    if (!mRevocationData.empty())
    {
        std::istringstream jsonStream(!mRevocationData.empty() ? mRevocationData : "[]");
        if (!Json::parseFromStream(readerBuilder, jsonStream, &jsonData, &errs))
        {
            ChipLogError(NotSpecified, "Failed to parse JSON data: %s", errs.c_str());
            return false;
        }
    }
    else if (!mDeviceAttestationRevocationSetPath.empty())
    {
        std::ifstream file(mDeviceAttestationRevocationSetPath.c_str());
        if (!file.is_open())
        {
            ChipLogError(NotSpecified, "Failed to open file: %s", mDeviceAttestationRevocationSetPath.c_str());
            return false;
        }

        bool parsingSuccessful = Json::parseFromStream(readerBuilder, file, &jsonData, &errs);
        file.close();

        if (!parsingSuccessful)
        {
            ChipLogError(NotSpecified, "Failed to parse JSON from file: %s", errs.c_str());
            return false;
        }
    }
    else
    {
        // No revocation data available
        return false;
    }

    // 6.2.4.2. Determining Revocation Status of an Entity
    for (const auto & revokedSet : jsonData)
    {
        if (revokedSet["issuer_name"].asString() != issuerNameBase64Str)
        {
            continue;
        }
        if (revokedSet["issuer_subject_key_id"].asString() != akidHexStr)
        {
            continue;
        }

        // 4.a cross validate PAI with crl signer OR crl signer delegator
        // 4.b cross validate DAC with crl signer OR crl signer delegator
        VerifyOrReturnValue(CrossValidateCert(revokedSet, akidHexStr, issuerNameBase64Str), false);

        // 4.c check if serial number is revoked
        for (const auto & revokedSerialNumber : revokedSet["revoked_serial_numbers"])
        {
            if (revokedSerialNumber.asString() == serialNumberHexStr)
            {
                return true;
            }
        }
    }
    return false;
}

CHIP_ERROR TestDACRevocationDelegateImpl::GetKeyIDHexStr(const ByteSpan & certDer, std::string & outKeyIDHexStr,
                                                         KeyIdType keyIdType)
{
    static_assert(kAuthorityKeyIdentifierLength == kSubjectKeyIdentifierLength, "AKID and SKID length mismatch");

    uint8_t keyIdBuf[kAuthorityKeyIdentifierLength];
    MutableByteSpan keyId(keyIdBuf);

    switch (keyIdType)
    {
    case KeyIdType::kAKID:
        ReturnErrorOnFailure(ExtractAKIDFromX509Cert(certDer, keyId));
        break;

    case KeyIdType::kSKID:
        ReturnErrorOnFailure(ExtractSKIDFromX509Cert(certDer, keyId));
        break;

    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return BytesToHexStr(keyId, outKeyIDHexStr);
}

CHIP_ERROR TestDACRevocationDelegateImpl::GetAKIDHexStr(const ByteSpan & certDer, std::string & outAKIDHexStr)
{
    return GetKeyIDHexStr(certDer, outAKIDHexStr, KeyIdType::kAKID);
}

CHIP_ERROR TestDACRevocationDelegateImpl::GetSKIDHexStr(const ByteSpan & certDer, std::string & outSKIDHexStr)
{
    return GetKeyIDHexStr(certDer, outSKIDHexStr, KeyIdType::kSKID);
}

CHIP_ERROR TestDACRevocationDelegateImpl::GetSerialNumberHexStr(const ByteSpan & certDer, std::string & outSerialNumberHexStr)
{
    uint8_t serialNumberBuf[kMaxCertificateSerialNumberLength] = { 0 };
    MutableByteSpan serialNumber(serialNumberBuf);

    ReturnErrorOnFailure(ExtractSerialNumberFromX509Cert(certDer, serialNumber));
    return BytesToHexStr(serialNumber, outSerialNumberHexStr);
}

CHIP_ERROR TestDACRevocationDelegateImpl::GetRDNBase64Str(const ByteSpan & certDer, std::string & outRDNBase64String,
                                                          RDNType rdnType)
{
    uint8_t rdnBuf[kMaxCertificateDistinguishedNameLength] = { 0 };
    MutableByteSpan rdn(rdnBuf);

    switch (rdnType)
    {
    case RDNType::kIssuer:
        ReturnErrorOnFailure(ExtractIssuerFromX509Cert(certDer, rdn));
        break;

    case RDNType::kSubject:
        ReturnErrorOnFailure(ExtractSubjectFromX509Cert(certDer, rdn));
        break;

    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // calculate the b64 length needed
    size_t b64LenNeeded = BASE64_ENCODED_LEN(rdn.size());

    // Ensure string has enough capacity for base64 encoded data
    outRDNBase64String.resize(b64LenNeeded);

    uint16_t encodedLen = Base64Encode(rdn.data(), static_cast<uint16_t>(rdn.size()), &outRDNBase64String[0]);
    outRDNBase64String.resize(encodedLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR TestDACRevocationDelegateImpl::GetIssuerNameBase64Str(const ByteSpan & certDer, std::string & outIssuerNameBase64String)
{
    return GetRDNBase64Str(certDer, outIssuerNameBase64String, RDNType::kIssuer);
}

CHIP_ERROR TestDACRevocationDelegateImpl::GetSubjectNameBase64Str(const ByteSpan & certDer,
                                                                  std::string & outSubjectNameBase64String)
{
    return GetRDNBase64Str(certDer, outSubjectNameBase64String, RDNType::kSubject);
}

// @param certDer Certificate, in DER format, to check for revocation
bool TestDACRevocationDelegateImpl::IsCertificateRevoked(const ByteSpan & certDer)
{
    std::string serialNumber;
    std::string akid;
    std::string issuerName;

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

    if (mDeviceAttestationRevocationSetPath.empty() && mRevocationData.empty())
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
