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

CHIP_ERROR FactoryDataProvider::Init()
{
    return CHIP_NO_ERROR;
}

/*
 * Members functions that implement the CommissionableDataProvider
 */
CHIP_ERROR FactoryDataProvider::GetSetupDiscriminator(uint16_t & setupDiscriminator)
{
    return CYW30739Config::ReadConfigValue(CYW30739Config::kConfigKey_SetupDiscriminator, setupDiscriminator);
}

CHIP_ERROR FactoryDataProvider::SetSetupDiscriminator(uint16_t setupDiscriminator)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR FactoryDataProvider::GetSpake2pIterationCount(uint32_t & iterationCount)
{
    return CYW30739Config::ReadConfigValue(CYW30739Config::kConfigKey_Spake2pIterationCount, iterationCount);
}

CHIP_ERROR FactoryDataProvider::GetSpake2pSalt(MutableByteSpan & saltBuf)
{
    size_t read_size;
    ReturnErrorOnFailure(
        CYW30739Config::ReadConfigValueBin(CYW30739Config::kConfigKey_Spake2pSalt, saltBuf.data(), saltBuf.size(), read_size));
    saltBuf.reduce_size(read_size);
    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetSpake2pVerifier(MutableByteSpan & verifierBuf, size_t & outVerifierLen)
{
    ReturnErrorOnFailure(CYW30739Config::ReadConfigValueBin(CYW30739Config::kConfigKey_Spake2pVerifier, verifierBuf.data(),
                                                            verifierBuf.size(), outVerifierLen));
    verifierBuf.reduce_size(outVerifierLen);
    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetSetupPasscode(uint32_t & setupPasscode)
{
    return CYW30739Config::ReadConfigValue(CYW30739Config::kConfigKey_SetupPinCode, setupPasscode);
}

CHIP_ERROR FactoryDataProvider::SetSetupPasscode(uint32_t setupPasscode)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

/*
 * Members functions that implement the DeviceAttestationCredentialsProvider
 */
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

/*
 * Members functions that implement the DeviceInstanceInfoProvider
 */
CHIP_ERROR FactoryDataProvider::GetVendorName(char * buf, size_t bufSize)
{
    size_t read_size;
    return CYW30739Config::ReadConfigValueStr(CYW30739Config::kConfigKey_VendorName, buf, bufSize, read_size);
}

CHIP_ERROR FactoryDataProvider::GetVendorId(uint16_t & vendorId)
{
    return CYW30739Config::ReadConfigValue(CYW30739Config::kConfigKey_VendorId, vendorId);
}

CHIP_ERROR FactoryDataProvider::GetProductName(char * buf, size_t bufSize)
{
    size_t read_size;
    return CYW30739Config::ReadConfigValueStr(CYW30739Config::kConfigKey_ProductName, buf, bufSize, read_size);
}

CHIP_ERROR FactoryDataProvider::GetProductId(uint16_t & productId)
{
    return CYW30739Config::ReadConfigValue(CYW30739Config::kConfigKey_ProductId, productId);
}

CHIP_ERROR FactoryDataProvider::GetPartNumber(char * buf, size_t bufSize)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR FactoryDataProvider::GetProductURL(char * buf, size_t bufSize)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR FactoryDataProvider::GetProductLabel(char * buf, size_t bufSize)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR FactoryDataProvider::GetSerialNumber(char * buf, size_t bufSize)
{
    size_t read_size;
    return CYW30739Config::ReadConfigValueStr(CYW30739Config::kConfigKey_SerialNum, buf, bufSize, read_size);
}

CHIP_ERROR FactoryDataProvider::GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR FactoryDataProvider::GetHardwareVersion(uint16_t & hardwareVersion)
{
    return CYW30739Config::ReadConfigValue(CYW30739Config::kConfigKey_HardwareVersion, hardwareVersion);
}

CHIP_ERROR FactoryDataProvider::GetHardwareVersionString(char * buf, size_t bufSize)
{
    size_t read_size;
    return CYW30739Config::ReadConfigValueStr(CYW30739Config::kConfigKey_HardwareVersionString, buf, bufSize, read_size);
}

CHIP_ERROR FactoryDataProvider::GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan)
{
    ChipError err = CHIP_ERROR_WRONG_KEY_TYPE;
#if CHIP_ENABLE_ROTATING_DEVICE_ID && defined(CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID)
    if (chip::DeviceLayer::ConfigurationMgr().GetRotatingDeviceIdUniqueId(uniqueIdSpan) != CHIP_NO_ERROR)
    {
        static_assert(ConfigurationManager::kRotatingDeviceIDUniqueIDLength >=
                          ConfigurationManager::kMinRotatingDeviceIDUniqueIDLength,
                      "Length of unique ID for rotating device ID is smaller than minimum.");

        constexpr uint8_t uniqueId[] = CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID;

        ReturnErrorCodeIf(sizeof(uniqueId) > uniqueIdSpan.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
        ReturnErrorCodeIf(sizeof(uniqueId) != ConfigurationManager::kRotatingDeviceIDUniqueIDLength, CHIP_ERROR_BUFFER_TOO_SMALL);
        memcpy(uniqueIdSpan.data(), uniqueId, sizeof(uniqueId));
        uniqueIdSpan.reduce_size(sizeof(uniqueId));
    }
    return CHIP_NO_ERROR;
#endif
    return err;
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
