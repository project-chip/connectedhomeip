/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include "DeviceAttestationCredsExample.h"

#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>

// TODO: Remove once the Attestation Credentials storage mechanism is updated.
namespace chip {
namespace TestCerts {
extern const ByteSpan sTestCert_DAC_FFF1_8000_0004_Cert;
extern const ByteSpan sTestCert_PAI_FFF1_8000_Cert;
extern const ByteSpan sTestCert_DAC_FFF1_8000_0004_PublicKey;
extern const ByteSpan sTestCert_DAC_FFF1_8000_0004_PrivateKey;
} // namespace TestCerts
} // namespace chip

namespace chip {
namespace Credentials {
namespace Examples {

namespace {

// TODO: This should be moved to a method of P256Keypair
CHIP_ERROR LoadKeypairFromRaw(ByteSpan private_key, ByteSpan public_key, Crypto::P256Keypair & keypair)
{
    Crypto::P256SerializedKeypair serialized_keypair;
    ReturnErrorOnFailure(serialized_keypair.SetLength(private_key.size() + public_key.size()));
    memcpy(serialized_keypair.Bytes(), public_key.data(), public_key.size());
    memcpy(serialized_keypair.Bytes() + public_key.size(), private_key.data(), private_key.size());
    return keypair.Deserialize(serialized_keypair);
}

class ExampleDACProvider : public DeviceAttestationCredentialsProvider
{
public:
    CHIP_ERROR GetCertificationDeclaration(MutableByteSpan & out_cd_buffer) override;
    CHIP_ERROR GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer) override;
    CHIP_ERROR GetDeviceAttestationCert(MutableByteSpan & out_dac_buffer) override;
    CHIP_ERROR GetProductAttestationIntermediateCert(MutableByteSpan & out_pai_buffer) override;
    CHIP_ERROR SignWithDeviceAttestationKey(const ByteSpan & digest_to_sign, MutableByteSpan & out_signature_buffer) override;
};

CHIP_ERROR ExampleDACProvider::GetDeviceAttestationCert(MutableByteSpan & out_dac_buffer)
{
    return CopySpanToMutableSpan(TestCerts::sTestCert_DAC_FFF1_8000_0004_Cert, out_dac_buffer);
}

CHIP_ERROR ExampleDACProvider::GetProductAttestationIntermediateCert(MutableByteSpan & out_pai_buffer)
{
    return CopySpanToMutableSpan(TestCerts::sTestCert_PAI_FFF1_8000_Cert, out_pai_buffer);
}

CHIP_ERROR ExampleDACProvider::GetCertificationDeclaration(MutableByteSpan & out_cd_buffer)
{
    // -> format_version = 1
    // -> vendor_id = 0xFFF1
    // -> product_id_array = [ 0x8000 ]
    // -> device_type_id = 0x1234
    // -> certificate_id = "ZIG20141ZB330001-24"
    // -> security_level = 0
    // -> security_information = 0
    // -> version_number = 0x2694
    // -> certification_type = 0
    // -> dac_origin_vendor_id is not present
    // -> dac_origin_product_id is not present
    constexpr uint8_t kCertificationDeclaration[] = {
        0x30, 0x81, 0xe8, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x07, 0x02, 0xa0, 0x81, 0xda, 0x30, 0x81, 0xd7,
        0x02, 0x01, 0x03, 0x31, 0x0d, 0x30, 0x0b, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x30, 0x45,
        0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x07, 0x01, 0xa0, 0x38, 0x04, 0x36, 0x15, 0x24, 0x00, 0x01, 0x25,
        0x01, 0xf1, 0xff, 0x36, 0x02, 0x05, 0x00, 0x80, 0x18, 0x25, 0x03, 0x34, 0x12, 0x2c, 0x04, 0x13, 0x5a, 0x49, 0x47, 0x32,
        0x30, 0x31, 0x34, 0x31, 0x5a, 0x42, 0x33, 0x33, 0x30, 0x30, 0x30, 0x31, 0x2d, 0x32, 0x34, 0x24, 0x05, 0x00, 0x24, 0x06,
        0x00, 0x25, 0x07, 0x94, 0x26, 0x24, 0x08, 0x00, 0x18, 0x31, 0x7c, 0x30, 0x7a, 0x02, 0x01, 0x03, 0x80, 0x14, 0x62, 0xfa,
        0x82, 0x33, 0x59, 0xac, 0xfa, 0xa9, 0x96, 0x3e, 0x1c, 0xfa, 0x14, 0x0a, 0xdd, 0xf5, 0x04, 0xf3, 0x71, 0x60, 0x30, 0x0b,
        0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d,
        0x04, 0x03, 0x02, 0x04, 0x46, 0x30, 0x44, 0x02, 0x20, 0x43, 0xa6, 0x3f, 0x2b, 0x94, 0x3d, 0xf3, 0x3c, 0x38, 0xb3, 0xe0,
        0x2f, 0xca, 0xa7, 0x5f, 0xe3, 0x53, 0x2a, 0xeb, 0xbf, 0x5e, 0x63, 0xf5, 0xbb, 0xdb, 0xc0, 0xb1, 0xf0, 0x1d, 0x3c, 0x4f,
        0x60, 0x02, 0x20, 0x4c, 0x1a, 0xbf, 0x5f, 0x18, 0x07, 0xb8, 0x18, 0x94, 0xb1, 0x57, 0x6c, 0x47, 0xe4, 0x72, 0x4e, 0x4d,
        0x96, 0x6c, 0x61, 0x2e, 0xd3, 0xfa, 0x25, 0xc1, 0x18, 0xc3, 0xf2, 0xb3, 0xf9, 0x03, 0x69
    };

    return CopySpanToMutableSpan(ByteSpan{ kCertificationDeclaration }, out_cd_buffer);
}

CHIP_ERROR ExampleDACProvider::GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer)
{
    // TODO: We need a real example FirmwareInformation to be populated.
    out_firmware_info_buffer.reduce_size(0);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ExampleDACProvider::SignWithDeviceAttestationKey(const ByteSpan & digest_to_sign, MutableByteSpan & out_signature_buffer)
{
    Crypto::P256ECDSASignature signature;
    Crypto::P256Keypair keypair;

    VerifyOrReturnError(IsSpanUsable(out_signature_buffer), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSpanUsable(digest_to_sign), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_signature_buffer.size() >= signature.Capacity(), CHIP_ERROR_BUFFER_TOO_SMALL);

    // In a non-exemplary implementation, the public key is not needed here. It is used here merely because
    // Crypto::P256Keypair is only (currently) constructable from raw keys if both private/public keys are present.
    ReturnErrorOnFailure(LoadKeypairFromRaw(TestCerts::sTestCert_DAC_FFF1_8000_0004_PrivateKey,
                                            TestCerts::sTestCert_DAC_FFF1_8000_0004_PublicKey, keypair));
    ReturnErrorOnFailure(keypair.ECDSA_sign_hash(digest_to_sign.data(), digest_to_sign.size(), signature));

    return CopySpanToMutableSpan(ByteSpan{ signature.ConstBytes(), signature.Length() }, out_signature_buffer);
}

} // namespace

DeviceAttestationCredentialsProvider * GetExampleDACProvider()
{
    static ExampleDACProvider example_dac_provider;

    return &example_dac_provider;
}

} // namespace Examples
} // namespace Credentials
} // namespace chip
