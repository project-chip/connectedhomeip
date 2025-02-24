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

#include "UnprovisionedOptigaFactoryDataProvider.h"

#include <optiga/common/optiga_lib_common.h>
#include <wiced_optiga.h>

using namespace chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {

CHIP_ERROR UnprovisionedOptigaFactoryDataProvider::Init()
{
    ReturnErrorOnFailure(OptigaFactoryDataProvider::Init());

    if (CYW30739Config::ConfigValueExists(CYW30739Config::kConfigKey_ProvisioningDAC))
    {
        if (!ChipError::IsSuccess(CheckProvisionedDataValidity()))
        {
            ReturnErrorOnFailure(ProvisionDataOnce());
        }

        CYW30739Config::ClearConfigValue(CYW30739Config::kConfigKey_ProvisioningDAC);
        CYW30739Config::ClearConfigValue(CYW30739Config::kConfigKey_ProvisioningPAICert);
        CYW30739Config::ClearConfigValue(CYW30739Config::kConfigKey_ProvisioningSecret);
        CYW30739Config::ClearConfigValue(CYW30739Config::kConfigKey_ProvisioningSecretMetaData);
        CYW30739Config::ClearConfigValue(CYW30739Config::kConfigKey_ProvisioningDACMetaData);
        CYW30739Config::ClearConfigValue(CYW30739Config::kConfigKey_ProvisioningDACKeyMetaData);
        CYW30739Config::ClearConfigValue(CYW30739Config::kConfigKey_ProvisioningManifest);
        CYW30739Config::ClearConfigValue(CYW30739Config::kConfigKey_ProvisioningFragment);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR UnprovisionedOptigaFactoryDataProvider::ProvisionDataOnce()
{
    constexpr struct
    {
        CYW30739Config::Key key;
        uint16_t object_id;
        uint8_t object_type;
    } configs[] = {
        { CYW30739Config::kConfigKey_ProvisioningDAC, OPTIGA_DAC_OBJECT_ID, OPTIGA_OBJECT_DATA },
        { CYW30739Config::kConfigKey_ProvisioningPAICert, OPTIGA_PAI_CERT_OBJECT_ID, OPTIGA_OBJECT_DATA },
        { CYW30739Config::kConfigKey_ProvisioningSecret, OPTIGA_SECRET_OBJECT_ID, OPTIGA_OBJECT_DATA },
        { CYW30739Config::kConfigKey_ProvisioningSecretMetaData, OPTIGA_SECRET_OBJECT_ID, OPTIGA_OBJECT_METADATA },
        { CYW30739Config::kConfigKey_ProvisioningDACMetaData, OPTIGA_DAC_OBJECT_ID, OPTIGA_OBJECT_METADATA },
        { CYW30739Config::kConfigKey_ProvisioningDACKeyMetaData, OPTIGA_DAC_KEY_OBJECT_ID, OPTIGA_OBJECT_METADATA },
    };

    ChipLogProgress(DeviceLayer, "Provisioning Optiga data");

    for (size_t i = 0; i < MATTER_ARRAY_SIZE(configs); i++)
    {
        ProvisionDataFromConfig(configs[i].key, configs[i].object_id, configs[i].object_type);
    }

    ProvisionProtectedData();

    return CHIP_NO_ERROR;
}

CHIP_ERROR UnprovisionedOptigaFactoryDataProvider::ProvisionDataFromConfig(CYW30739Config::Key key, uint16_t object_id,
                                                                           uint8_t object_type)
{
    size_t read_size;
    uint8_t buf[Credentials::kMaxDERCertLength];
    MutableByteSpan buf_span(buf);
    ReturnErrorOnFailure(CYW30739Config::ReadConfigValueBin(key, buf_span.data(), buf_span.size(), read_size));
    buf_span.reduce_size(read_size);

    VerifyOrReturnError(wiced_optiga_write_data(object_id, object_type, buf_span.data(), buf_span.size()) == WICED_SUCCESS,
                        CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR UnprovisionedOptigaFactoryDataProvider::CheckProvisionedDataValidity()
{
    uint8_t digest_buf[Crypto::kSHA256_Hash_Length];
    MutableByteSpan digest_span(digest_buf);
    ReturnErrorOnFailure(Crypto::DRBG_get_bytes(digest_span.data(), digest_span.size()));

    uint8_t signature_buffer[Crypto::kMax_ECDSA_Signature_Length_Der];
    MutableByteSpan signature_span(signature_buffer);
    ReturnErrorOnFailure(SignWithOptigaDeviceAttestationKey(digest_span, signature_span));

    ReturnErrorOnFailure(VerifyOptigaSignature(digest_span, signature_span, OPTIGA_DAC_OBJECT_ID));

    Crypto::P256PublicKey dac_public_key;
    ReturnErrorOnFailure(GetDeviceAttestationCertPublicKey(dac_public_key));
    ReturnErrorOnFailure(VerifyOptigaSignature(digest_span, signature_span, dac_public_key));

    return CHIP_NO_ERROR;
}

CHIP_ERROR UnprovisionedOptigaFactoryDataProvider::ProvisionProtectedData()
{
    constexpr size_t kMaxManifestLength = 256;
    uint8_t buf[kMaxManifestLength];
    size_t read_size;

    /* Update the manifest */
    MutableByteSpan manifest_span(buf);
    ReturnErrorOnFailure(CYW30739Config::ReadConfigValueBin(CYW30739Config::kConfigKey_ProvisioningManifest, manifest_span.data(),
                                                            manifest_span.size(), read_size));
    manifest_span.reduce_size(read_size);

    VerifyOrReturnError(wiced_optiga_protected_update_start(1, manifest_span.data(), manifest_span.size()) == WICED_SUCCESS,
                        CHIP_ERROR_INTERNAL);

    /* Update the fragment */
    MutableByteSpan fragment_span(buf);
    ReturnErrorOnFailure(CYW30739Config::ReadConfigValueBin(CYW30739Config::kConfigKey_ProvisioningFragment, fragment_span.data(),
                                                            fragment_span.size(), read_size));
    fragment_span.reduce_size(read_size);

    VerifyOrReturnError(wiced_optiga_protected_update_final(fragment_span.data(), fragment_span.size()) == WICED_SUCCESS,
                        CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR UnprovisionedOptigaFactoryDataProvider::VerifyOptigaSignature(MutableByteSpan & digest, MutableByteSpan & signature,
                                                                         uint16_t public_key)
{
    VerifyOrReturnError(wiced_optiga_ecdsa_verify(digest.data(), digest.size(), signature.data(), signature.size(),
                                                  OPTIGA_CRYPT_OID_DATA, &public_key) == WICED_SUCCESS,
                        CHIP_ERROR_INVALID_SIGNATURE);

    return CHIP_NO_ERROR;
}

CHIP_ERROR UnprovisionedOptigaFactoryDataProvider::VerifyOptigaSignature(MutableByteSpan & digest, MutableByteSpan & signature,
                                                                         Crypto::P256PublicKey & public_key)
{
    VerifyOrReturnError(public_key.Length() == Crypto::kP256_PublicKey_Length, CHIP_ERROR_INVALID_PUBLIC_KEY);

    constexpr uint8_t kOptigaDerBitstringTag                                                      = 0x03;
    constexpr uint8_t kOptigaDerAdditionalLength                                                  = 0x01;
    constexpr uint8_t kOptigaDerNumUnusedBits                                                     = 0x00;
    constexpr size_t kOptigaPublicKeyHeaderLength                                                 = 3;
    static uint8_t host_public_key[kOptigaPublicKeyHeaderLength + Crypto::kP256_PublicKey_Length] = {
        [0] = kOptigaDerBitstringTag,
        [1] = kOptigaDerAdditionalLength + Crypto::kP256_PublicKey_Length,
        [2] = kOptigaDerNumUnusedBits,
    };
    memcpy(host_public_key + kOptigaPublicKeyHeaderLength, public_key, public_key.Length());
    const public_key_from_host_t public_key_from_host = {
        .public_key = host_public_key,
        .length     = sizeof(host_public_key),
        .key_type   = OPTIGA_ECC_CURVE_NIST_P_256,
    };
    VerifyOrReturnError(wiced_optiga_ecdsa_verify(digest.data(), digest.size(), signature.data(), signature.size(),
                                                  OPTIGA_CRYPT_HOST_DATA, &public_key_from_host) == WICED_SUCCESS,
                        CHIP_ERROR_INVALID_SIGNATURE);

    return CHIP_NO_ERROR;
}

CHIP_ERROR UnprovisionedOptigaFactoryDataProvider::GetDeviceAttestationCertPublicKey(Crypto::P256PublicKey & public_key)
{
    size_t read_size;
    uint8_t dac_buf[Credentials::kMaxDERCertLength];
    MutableByteSpan dac_span(dac_buf);
    ReturnErrorOnFailure(CYW30739Config::ReadConfigValueBin(CYW30739Config::kConfigKey_ProvisioningDAC, dac_span.data(),
                                                            dac_span.size(), read_size));
    dac_span.reduce_size(read_size);

    ReturnErrorOnFailure(ExtractPubkeyFromX509Cert(dac_span, public_key));

    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
