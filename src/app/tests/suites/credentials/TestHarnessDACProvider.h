/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <credentials/DeviceAttestationCredsProvider.h>
#include <lib/core/Optional.h>

namespace chip {
namespace Credentials {
namespace Examples {

struct TestHarnessDACProviderData
{
    chip::Optional<ByteSpan> dacCert;
    chip::Optional<ByteSpan> dacPrivateKey;
    chip::Optional<ByteSpan> dacPublicKey;
    chip::Optional<ByteSpan> paiCert;
    chip::Optional<ByteSpan> certificationDeclaration;
    chip::Optional<ByteSpan> firmwareInformation;
    chip::Optional<CharSpan> description;
    chip::Optional<bool> isSuccessCase;
};

class TestHarnessDACProvider : public DeviceAttestationCredentialsProvider
{
public:
    TestHarnessDACProvider();

    CHIP_ERROR GetCertificationDeclaration(MutableByteSpan & out_cd_buffer) override;
    CHIP_ERROR GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer) override;
    CHIP_ERROR GetDeviceAttestationCert(MutableByteSpan & out_dac_buffer) override;
    CHIP_ERROR GetProductAttestationIntermediateCert(MutableByteSpan & out_pai_buffer) override;
    CHIP_ERROR SignWithDeviceAttestationKey(const ByteSpan & message_to_sign, MutableByteSpan & out_signature_buffer) override;
    CharSpan GetDescription() { return mDescription; }
    bool IsSuccessCase() { return mIsSuccessCase; }

    void Init(const char * filepath);
    void Init(const TestHarnessDACProviderData & data);

private:
    ByteSpan mDacCert;
    ByteSpan mDacPrivateKey;
    ByteSpan mDacPublicKey;
    ByteSpan mPaiCert;
    ByteSpan mCertificationDeclaration;
    ByteSpan mFirmwareInformation;
    CharSpan mDescription;
    bool mIsSuccessCase;
};

} // namespace Examples
} // namespace Credentials
} // namespace chip
