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

#include "OptigaFactoryDataProvider.h"

#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <wiced_optiga.h>

namespace chip {
namespace DeviceLayer {

using namespace chip::DeviceLayer::Internal;

/*
 * Members functions that implement the DeviceAttestationCredentialsProvider
 */
CHIP_ERROR OptigaFactoryDataProvider::GetDeviceAttestationCert(MutableByteSpan & out_dac_buffer)
{
    uint16_t length = out_dac_buffer.size();
    VerifyOrReturnError(wiced_optiga_read_data(OPTIGA_DAC_OBJECT_ID, OPTIGA_OBJECT_DATA, out_dac_buffer.data(), &length) ==
                            WICED_SUCCESS,
                        CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);
    out_dac_buffer.reduce_size(length);

    return CHIP_NO_ERROR;
}

CHIP_ERROR OptigaFactoryDataProvider::GetProductAttestationIntermediateCert(MutableByteSpan & out_pai_buffer)
{
    uint16_t length = out_pai_buffer.size();
    VerifyOrReturnError(wiced_optiga_read_data(OPTIGA_PAI_CERT_OBJECT_ID, OPTIGA_OBJECT_DATA, out_pai_buffer.data(), &length) ==
                            WICED_SUCCESS,
                        CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);
    out_pai_buffer.reduce_size(length);

    return CHIP_NO_ERROR;
}

CHIP_ERROR OptigaFactoryDataProvider::SignWithDeviceAttestationKey(const ByteSpan & message_to_sign,
                                                                   MutableByteSpan & out_signature_buffer)
{
    VerifyOrReturnError(!out_signature_buffer.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(!message_to_sign.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_signature_buffer.size() >= Crypto::kMax_ECDSA_Signature_Length, CHIP_ERROR_BUFFER_TOO_SMALL);

    uint8_t digest[Crypto::kSHA256_Hash_Length];
    ReturnErrorOnFailure(Crypto::Hash_SHA256(message_to_sign.data(), message_to_sign.size(), digest));
    ByteSpan digest_span(digest);

    constexpr uint8_t kAsn1HeaderLength = 2u;
    constexpr uint8_t kSeqTag           = 0x30u;

    uint8_t asn1_sigature_buffer[Crypto::kMax_ECDSA_Signature_Length_Der];
    MutableByteSpan asn1_signature_span(asn1_sigature_buffer);

    MutableByteSpan optiga_signature_span(asn1_signature_span.SubSpan(kAsn1HeaderLength));
    ReturnErrorOnFailure(SignWithOptigaDeviceAttestationKey(digest_span, optiga_signature_span));

    asn1_signature_span[0] = kSeqTag;
    asn1_signature_span[1] = optiga_signature_span.size();
    asn1_signature_span.reduce_size(kAsn1HeaderLength + optiga_signature_span.size());
    ReturnErrorOnFailure(Crypto::EcdsaAsn1SignatureToRaw(Crypto::kP256_FE_Length, asn1_signature_span, out_signature_buffer));

    return CHIP_NO_ERROR;
}

CHIP_ERROR OptigaFactoryDataProvider::SignWithOptigaDeviceAttestationKey(const ByteSpan & digest, MutableByteSpan & signature)
{
    uint16_t signature_length = signature.size();
    VerifyOrReturnError(wiced_optiga_ecdsa_sign(digest.data(), digest.size(), OPTIGA_DAC_KEY_OBJECT_ID, signature.data(),
                                                &signature_length) == WICED_SUCCESS,
                        CHIP_ERROR_INTERNAL);
    signature.reduce_size(signature_length);

    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
