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
#pragma once

#include <credentials/DeviceAttestationCredsProvider.h>
#include <lib/core/Optional.h>
#include <lib/support/ScopedMemoryBuffer.h>

#include <istream>

namespace chip {
namespace Credentials {
namespace Examples {

struct TestHarnessDACProviderData
{
    chip::Optional<ByteSpan> dacCert;
    chip::Optional<ByteSpan> dacPrivateKey;
    chip::Optional<ByteSpan> dacPublicKey;
    chip::Optional<ByteSpan> pqcDacCertMlDsa44;
    chip::Optional<ByteSpan> pqcDacCertMlDsa65;
    chip::Optional<ByteSpan> paiCert;
    chip::Optional<ByteSpan> pqcPaiCertMlDsa44;
    chip::Optional<ByteSpan> pqcPaiCertMlDsa65;
    chip::Optional<ByteSpan> certificationDeclaration;
    chip::Optional<ByteSpan> firmwareInformation;
    chip::Optional<CharSpan> description;
    chip::Optional<bool> isSuccessCase;
    chip::Optional<uint16_t> pid;
    // The ML-DSA suffix identifies the PAA/chain, not necessarily the PAI public key.
    // JSON keys: pai_profile_ml_dsa_44 / pai_profile_ml_dsa_65; values: 0 = ECDSA,
    // 1 = ML-DSA-44, 2 = ML-DSA-65. Set these to the provisioned PAI key algorithm.
    // Defaults preserve the interpretation of existing homogeneous issuer fixtures.
    DeviceAttestationCertProfile paiProfileMlDsa44 = DeviceAttestationCertProfile::kMlDsa44;
    DeviceAttestationCertProfile paiProfileMlDsa65 = DeviceAttestationCertProfile::kMlDsa65;
};

class TestHarnessDACProvider : public DeviceAttestationCredentialsProvider
{
public:
    // Capability is derived from loaded credentials; no separate PQC opt-in is needed.
    TestHarnessDACProvider();

    CHIP_ERROR GetCertificationDeclaration(MutableByteSpan & out_cd_buffer) override;
    CHIP_ERROR GetDeviceAttestationCertForProfile(DeviceAttestationCertProfile profile, MutableByteSpan & out_dac_buffer) override;
    CHIP_ERROR GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer) override;
    CHIP_ERROR GetDeviceAttestationCert(MutableByteSpan & out_dac_buffer) override;
    CHIP_ERROR GetProductAttestationIntermediateCertForProfile(DeviceAttestationCertProfile profile,
                                                               MutableByteSpan & out_pai_buffer) override;
    CHIP_ERROR GetProductAttestationIntermediateCert(MutableByteSpan & out_pai_buffer) override;
    CHIP_ERROR SignWithDeviceAttestationKey(const ByteSpan & message_to_sign, MutableByteSpan & out_signature_buffer) override;
    DeviceAttestationProfileSupport GetDeviceAttestationProfileSupport() const override;
    DeviceAttestationCertProfile GetPreferredDeviceAttestationChainProfile() const override;
    CHIP_ERROR GetDeviceAttestationDocumentSegment(DeviceAttestationDocumentType documentType, DeviceAttestationCertProfile profile,
                                                   size_t offset, MutableByteSpan & out_document_buffer,
                                                   size_t & out_document_size) override;
    CharSpan GetDescription() { return mDescription; }
    bool IsSuccessCase() { return mIsSuccessCase; }
    uint16_t GetPid() { return mPid; }

    void Init(const char * filepath);
    /// Own the decoded JSON data, bounded by each field's size limit. Errors leave the current credentials unchanged.
    CHIP_ERROR Init(std::istream & json);
    /// Borrow data without allocating; its backing storage must outlive use by this provider.
    void Init(const TestHarnessDACProviderData & data);

private:
    Platform::ScopedMemoryBuffer<uint8_t> mJsonStorage;
    ByteSpan mDacCert;
    ByteSpan mDacPrivateKey;
    ByteSpan mDacPublicKey;
    ByteSpan mPqcDacCertMlDsa44;
    ByteSpan mPqcDacCertMlDsa65;
    ByteSpan mPaiCert;
    ByteSpan mPqcPaiCertMlDsa44;
    ByteSpan mPqcPaiCertMlDsa65;
    ByteSpan mCertificationDeclaration;
    ByteSpan mFirmwareInformation;
    CharSpan mDescription;
    DeviceAttestationProfileSupport mProfileSupport;
    bool mIsSuccessCase;
    uint16_t mPid;
};

} // namespace Examples
} // namespace Credentials
} // namespace chip
