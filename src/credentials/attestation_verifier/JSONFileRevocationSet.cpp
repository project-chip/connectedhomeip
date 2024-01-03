/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include "JSONFileRevocationSet.h"

#include <lib/support/Base64.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/ScopedBuffer.h>

#include <cstdlib>
#include <fstream>
#include <iostream>

namespace chip {
namespace Credentials {

JSONFileRevocationSet::JSONFileRevocationSet(const char * revocationSetPath)
{
    VerifyOrReturn(revocationSetPath != nullptr);

    if (revocationSetPath != nullptr)
    {
        LoadJSONRevocationSet(revocationSetPath, mRevocationSet);
        VerifyOrReturn(revocationSetCount());
    }

    mIsInitialized = true;
}

void LoadJSONRevocationSet(const char * revocationSetPath, Json::Value & revocationSet)
{
    VerifyOrReturn(revocationSetPath != nullptr);

    std::ifstream ifs(revocationSetPath);
    Json::Reader reader;
    reader.parse(ifs, revocationSet);
}

AttestationVerificationResult JSONFileRevocationSet::IsCertificateRevoked(bool isPaa, Crypto::AttestationCertVidPid vidPidUnderTest,
                                                                          ByteSpan issuer, ByteSpan authorityKeyId,
                                                                          ByteSpan serialNumber) const
{
    for (size_t revocation_set_idx = 0; revocation_set_idx < mRevocationSet.size(); ++revocation_set_idx)
    {
        Json::Value type = mRevocationSet[static_cast<int>(revocation_set_idx)]["type"];

        // 1.
        if (type.asString().compare("revocation_set") == 0)
        {
            Json::Value jsonCrlIssuerSubjectKeyId = mRevocationSet[static_cast<int>(revocation_set_idx)]["issuer_subject_key_id"];
            Json::Value jsonCrlIssuerName         = mRevocationSet[static_cast<int>(revocation_set_idx)]["issuer_name"];
            Json::Value jsonCrlRevokedSerialNumbers =
                mRevocationSet[static_cast<int>(revocation_set_idx)]["revoked_serial_numbers"];

            uint8_t crlIssuerSubjectKeyIdBuf[Crypto::kAuthorityKeyIdentifierLength] = { 0 };
            ByteSpan crlIssuerSubjectKeyId(crlIssuerSubjectKeyIdBuf);

            VerifyOrReturnError(Encoding::HexToBytes(jsonCrlIssuerSubjectKeyId.asString().c_str(),
                                                     jsonCrlIssuerSubjectKeyId.asString().size(), crlIssuerSubjectKeyIdBuf,
                                                     sizeof(crlIssuerSubjectKeyIdBuf)) == Crypto::kAuthorityKeyIdentifierLength,
                                AttestationVerificationResult::kInvalidArgument);

            size_t crlSignerCertificateMaxLength = BASE64_MAX_DECODED_LEN(jsonCrlIssuerName.asString().size());

            Platform::ScopedMemoryBuffer<uint8_t> crlSignerCertificate;

            ReturnErrorCodeIf(!crlSignerCertificate.Alloc(crlSignerCertificateMaxLength), AttestationVerificationResult::kNoMemory);

            // 2.
            size_t crlSignerCertificateLength =
                Base64Decode(jsonCrlIssuerName.asString().c_str(), jsonCrlIssuerName.asString().size(), crlSignerCertificate.Get());
            VerifyOrReturnError(crlSignerCertificateLength > 0 && crlSignerCertificateLength != UINT16_MAX,
                                AttestationVerificationResult::kInternalError);

            // 3.
            if (isPaa)
            {
                Crypto::AttestationCertVidPid vid;

                Crypto::ExtractVIDPIDFromAttributeString(
                    Crypto::DNAttrType::kCommonName, ByteSpan(crlSignerCertificate.Get(), crlSignerCertificateLength), vid, vid);

                if (vid.mVendorId.HasValue())
                {
                    if (vid.mVendorId.Value() != vidPidUnderTest.mVendorId.Value())
                    {
                        // VID does not match. Stop further processing and continue to next entry.
                        continue;
                    }
                }
            }
            // 4.
            else
            {
                Crypto::AttestationCertVidPid vidPid;

                Crypto::ExtractVIDPIDFromAttributeString(Crypto::DNAttrType::kCommonName,
                                                         ByteSpan(crlSignerCertificate.Get(), crlSignerCertificateLength), vidPid,
                                                         vidPid);

                if (vidPid.mVendorId.HasValue() && vidPid.mVendorId.Value() != vidPidUnderTest.mVendorId.Value())
                {
                    // VID does not match. Stop further processing and continue to next entry.
                    continue;
                }

                if (vidPid.mProductId.HasValue())
                {
                    if (vidPid.mProductId.Value() != vidPidUnderTest.mProductId.Value())
                    {
                        // PID does not match. Stop further processing and continue to next entry.
                        continue;
                    }
                }
            }

            // 7.a Perform CRLFile validation
            if (authorityKeyId.data_equal(crlIssuerSubjectKeyId) == false)
            {
                continue;
            }

            // TODO: 7.b

            // TODO: 8.

            // 9. && 10.
            for (int serial_number_idx = 0; serial_number_idx < static_cast<int>(jsonCrlRevokedSerialNumbers.size());
                 ++serial_number_idx)
            {
                size_t revokedSerialNumberMaxLength =
                    BASE64_MAX_DECODED_LEN(jsonCrlRevokedSerialNumbers[serial_number_idx].asString().size());

                Platform::ScopedMemoryBuffer<uint8_t> revokedSerialNumber;

                ReturnErrorCodeIf(!revokedSerialNumber.Alloc(revokedSerialNumberMaxLength),
                                  AttestationVerificationResult::kNoMemory);

                size_t revokedSerialNumberLength =
                    Base64Decode(jsonCrlRevokedSerialNumbers[serial_number_idx].asString().c_str(),
                                 jsonCrlRevokedSerialNumbers[serial_number_idx].asString().size(), revokedSerialNumber.Get());
                VerifyOrReturnError(revokedSerialNumberLength > 0 && revokedSerialNumberLength != UINT16_MAX,
                                    AttestationVerificationResult::kInternalError);

                uint8_t crlRevokedSerialNumberBuf[Crypto::kMaxCertificateSerialNumberLength] = { 0 };
                ByteSpan crlSerialNumber(crlRevokedSerialNumberBuf);

                VerifyOrReturnError(Encoding::HexToBytes(reinterpret_cast<char *>(revokedSerialNumber.Get()),
                                                         revokedSerialNumberLength, crlRevokedSerialNumberBuf,
                                                         sizeof(crlRevokedSerialNumberBuf)) == sizeof(crlRevokedSerialNumberBuf),
                                    AttestationVerificationResult::kInvalidArgument);

                if (serialNumber.data_equal(crlSerialNumber))
                {
                    return AttestationVerificationResult::kPaaRevoked;
                }
            }
        }
    }

    return AttestationVerificationResult::kSuccess;
}

JSONFileRevocationSet::~JSONFileRevocationSet()
{
    Cleanup();
}

void JSONFileRevocationSet::Cleanup()
{
    mRevocationSet.clear();
    mIsInitialized = false;
}

} // namespace Credentials
} // namespace chip
