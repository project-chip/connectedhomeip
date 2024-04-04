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

#include "DeviceAttestationCredsExampleTrustM.h"
#include <CHIPCryptoPALHsm_config_trustm.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>

#ifdef ENABLE_TRUSTM_DEVICE_ATTESTATION
#include <CHIPCryptoPALHsm_utils_trustm.h>

/* Device attestation key ids for Trust M */
#define DEV_ATTESTATION_KEY_ID 0xE0F0
#define DEV_ATTESTATION_CERT_ID 0xE0E0
#define PAI_CERT_ID 0xE0E8
#define CERT_DECLARATION_ID 0xF1E0

namespace chip {
namespace Credentials {
namespace Examples {

namespace {

class ExampleTrustMDACProvider : public DeviceAttestationCredentialsProvider
{
public:
    CHIP_ERROR GetCertificationDeclaration(MutableByteSpan & out_cd_buffer) override;
    CHIP_ERROR GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer) override;
    CHIP_ERROR GetDeviceAttestationCert(MutableByteSpan & out_dac_buffer) override;
    CHIP_ERROR GetProductAttestationIntermediateCert(MutableByteSpan & out_pai_buffer) override;
    CHIP_ERROR SignWithDeviceAttestationKey(const ByteSpan & message_to_sign, MutableByteSpan & out_signature_buffer) override;
};

CHIP_ERROR ExampleTrustMDACProvider::GetDeviceAttestationCert(MutableByteSpan & out_dac_buffer)
{
    uint16_t buflen = (uint16_t) out_dac_buffer.size();
    ChipLogDetail(Crypto, "Get DAC certificate from trustm");
    ReturnErrorOnFailure(trustmGetCertificate(DEV_ATTESTATION_CERT_ID, out_dac_buffer.data(), &buflen));
    out_dac_buffer.reduce_size(buflen);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ExampleTrustMDACProvider::GetProductAttestationIntermediateCert(MutableByteSpan & out_pai_buffer)
{
    uint16_t buflen = (uint16_t) out_pai_buffer.size();
    ChipLogDetail(Crypto, "Get PAI certificate from trustm");
    ReturnErrorOnFailure(trustmGetCertificate(PAI_CERT_ID, out_pai_buffer.data(), &buflen));
    out_pai_buffer.reduce_size(buflen);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ExampleTrustMDACProvider::GetCertificationDeclaration(MutableByteSpan & out_cd_buffer)
{
    //-> format_version = 1
    //-> vendor_id = 0xFFF1
    //-> product_id_array = [ 0x8000, 0x8001, 0x8002, 0x8003, 0x8004, 0x8005, 0x8006, 0x8007, 0x8008, 0x8009, 0x800A, 0x800B,
    // 0x800C, 0x800D, 0x800E, 0x800F, 0x8010, 0x8011, 0x8012, 0x8013, 0x8014, 0x8015, 0x8016, 0x8017, 0x8018, 0x8019, 0x801A,
    // 0x801B, 0x801C, 0x801D, 0x801E, 0x801F, 0x8020, 0x8021, 0x8022, 0x8023, 0x8024, 0x8025, 0x8026, 0x8027, 0x8028, 0x8029,
    // 0x802A, 0x802B, 0x802C, 0x802D, 0x802E, 0x802F, 0x8030, 0x8031, 0x8032, 0x8033, 0x8034, 0x8035, 0x8036, 0x8037, 0x8038,
    // 0x8039, 0x803A, 0x803B, 0x803C, 0x803D, 0x803E, 0x803F, 0x8040, 0x8041, 0x8042, 0x8043, 0x8044, 0x8045, 0x8046, 0x8047,
    // 0x8048, 0x8049, 0x804A, 0x804B, 0x804C, 0x804D, 0x804E, 0x804F, 0x8050, 0x8051, 0x8052, 0x8053, 0x8054, 0x8055, 0x8056,
    // 0x8057, 0x8058, 0x8059, 0x805A, 0x805B, 0x805C, 0x805D, 0x805E, 0x805F, 0x8060, 0x8061, 0x8062, 0x8063 ]
    //-> device_type_id = 0x0016
    //-> certificate_id = "ZIG20142ZB330003-24"
    //-> security_level = 0
    //-> security_information = 0
    //-> version_number = 0x2694
    //-> certification_type = 0
    //-> dac_origin_vendor_id is not present
    //-> dac_origin_product_id is not present
    size_t buflen = out_cd_buffer.size();
    ChipLogDetail(Crypto, "Get certificate declaration from trustm");
    ReturnErrorOnFailure(trustmGetCertificate(CERT_DECLARATION_ID, out_cd_buffer.data(), (uint16_t *) &buflen));
    out_cd_buffer.reduce_size(buflen);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ExampleTrustMDACProvider::GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer)
{
    // TODO: We need a real example FirmwareInformation to be populated.
    out_firmware_info_buffer.reduce_size(0);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ExampleTrustMDACProvider::SignWithDeviceAttestationKey(const ByteSpan & message_to_sign,
                                                                  MutableByteSpan & out_signature_buffer)
{
    Crypto::P256ECDSASignature signature;
    Crypto::P256Keypair keypair;
    Crypto::P256SerializedKeypair serialized_keypair;

    ChipLogDetail(Crypto, "Sign using DA key from trustm");

    VerifyOrReturnError(IsSpanUsable(out_signature_buffer), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSpanUsable(message_to_sign), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_signature_buffer.size() >= signature.Capacity(), CHIP_ERROR_BUFFER_TOO_SMALL);

    serialized_keypair.SetLength(Crypto::kP256_PublicKey_Length + Crypto::kP256_PrivateKey_Length);

    memset(serialized_keypair.Bytes(), 0, Crypto::kP256_PublicKey_Length);
    memcpy(serialized_keypair.Bytes() + Crypto::kP256_PublicKey_Length, trustm_magic_no, sizeof(trustm_magic_no));
    memcpy(serialized_keypair.Bytes() + (Crypto::kP256_PublicKey_Length + sizeof(trustm_magic_no)), DA_KEY_ID, 2);

    ReturnErrorOnFailure(keypair.Deserialize(serialized_keypair));

    ReturnErrorOnFailure(keypair.ECDSA_sign_msg(message_to_sign.data(), message_to_sign.size(), signature));

    return CopySpanToMutableSpan(ByteSpan{ signature.ConstBytes(), signature.Length() }, out_signature_buffer);
}

} // namespace

DeviceAttestationCredentialsProvider * GetExampleTrustMDACProvider()
{
    static ExampleTrustMDACProvider example_dac_provider;

    return &example_dac_provider;
}

} // namespace Examples
} // namespace Credentials
} // namespace chip

#endif // #ifdef ENABLE_TRUSTM_DEVICE_ATTESTATION
