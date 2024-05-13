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
