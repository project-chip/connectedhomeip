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

#include "FactoryDataProvider.h"

#include <mbedtls/pk.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

namespace chip {
namespace DeviceLayer {

using namespace chip::DeviceLayer::Internal;

CHIP_ERROR FactoryDataProvider::GetCertificationDeclaration(MutableByteSpan & out_cd_buffer)
{
    size_t read_size;
    ReturnErrorOnFailure(CYW30739Config::ReadConfigValueBin(CYW30739Config::kConfigKey_CertDeclaration, out_cd_buffer.data(),
                                                            out_cd_buffer.size(), read_size));
    out_cd_buffer.reduce_size(read_size);
    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer)
{
    out_firmware_info_buffer.reduce_size(0);
    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetDeviceAttestationCert(MutableByteSpan & out_dac_buffer)
{
    size_t read_size;
    ReturnErrorOnFailure(CYW30739Config::ReadConfigValueBin(CYW30739Config::kConfigKey_DAC, out_dac_buffer.data(),
                                                            out_dac_buffer.size(), read_size));
    out_dac_buffer.reduce_size(read_size);
    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetProductAttestationIntermediateCert(MutableByteSpan & out_pai_buffer)
{
    size_t read_size;
    ReturnErrorOnFailure(CYW30739Config::ReadConfigValueBin(CYW30739Config::kConfigKey_PAICert, out_pai_buffer.data(),
                                                            out_pai_buffer.size(), read_size));
    out_pai_buffer.reduce_size(read_size);
    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::SignWithDeviceAttestationKey(const ByteSpan & message_to_sign,
                                                             MutableByteSpan & out_signature_buffer)
{
    Crypto::P256ECDSASignature signature;
    Crypto::P256Keypair keypair;

    VerifyOrReturnError(!out_signature_buffer.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(!message_to_sign.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_signature_buffer.size() >= signature.Capacity(), CHIP_ERROR_BUFFER_TOO_SMALL);

    uint8_t dac_key_buffer[128];
    MutableByteSpan dac_key_span(dac_key_buffer);
    ReturnErrorOnFailure(GetDeviceAttestationCertKey(dac_key_span));
    ReturnErrorOnFailure(LoadKeypairFromDer(dac_key_span, keypair));
    ReturnErrorOnFailure(keypair.ECDSA_sign_msg(message_to_sign.data(), message_to_sign.size(), signature));

    return CopySpanToMutableSpan(ByteSpan{ signature.ConstBytes(), signature.Length() }, out_signature_buffer);
}

CHIP_ERROR FactoryDataProvider::GetDeviceAttestationCertKey(MutableByteSpan & out_key_buffer)
{
    size_t read_size;
    ReturnErrorOnFailure(CYW30739Config::ReadConfigValueBin(CYW30739Config::kConfigKey_DACKey, out_key_buffer.data(),
                                                            out_key_buffer.size(), read_size));
    out_key_buffer.reduce_size(read_size);
    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::LoadKeypairFromDer(const ByteSpan & der_buffer, Crypto::P256Keypair & keypair)
{
    CHIP_ERROR error   = CHIP_NO_ERROR;
    int mbedtls_result = 0;
    mbedtls_pk_context pk;
    mbedtls_ecp_keypair * ecp;
    Crypto::P256SerializedKeypair serializedKeypair;
    MutableByteSpan public_key(serializedKeypair.Bytes(), Crypto::kP256_PublicKey_Length);
    MutableByteSpan private_key(serializedKeypair.Bytes() + Crypto::kP256_PublicKey_Length, Crypto::kP256_PrivateKey_Length);

    mbedtls_pk_init(&pk);
    mbedtls_result = mbedtls_pk_parse_key(&pk, der_buffer.data(), der_buffer.size(), nullptr, 0);
    VerifyOrExit(mbedtls_result == 0, error = CHIP_ERROR_INTERNAL);

    ecp = mbedtls_pk_ec(pk);
    VerifyOrExit(ecp != nullptr, error = CHIP_ERROR_INTERNAL);

    size_t key_length;
    mbedtls_result = mbedtls_ecp_point_write_binary(&ecp->grp, &ecp->Q, MBEDTLS_ECP_PF_UNCOMPRESSED, &key_length, public_key.data(),
                                                    public_key.size());
    VerifyOrExit(mbedtls_result == 0 && key_length == Crypto::kP256_PublicKey_Length, error = CHIP_ERROR_INTERNAL);

    mbedtls_result = mbedtls_ecp_write_key(ecp, private_key.data(), private_key.size());
    VerifyOrExit(mbedtls_result == 0, error = CHIP_ERROR_INTERNAL);

    SuccessOrExit(error = serializedKeypair.SetLength(public_key.size() + private_key.size()));
    SuccessOrExit(error = keypair.Deserialize(serializedKeypair));

exit:
    if (mbedtls_result != 0)
    {
        ChipLogError(Crypto, "mbedtls result: 0x%04x", mbedtls_result);
    }
    mbedtls_pk_free(&pk);
    return error;
}

} // namespace DeviceLayer
} // namespace chip
