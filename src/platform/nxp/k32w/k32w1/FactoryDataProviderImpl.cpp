/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <crypto/CHIPCryptoPAL.h>
#include <platform/nxp/k32w/k32w1/FactoryDataProviderImpl.h>

namespace chip {
namespace DeviceLayer {

CHIP_ERROR FactoryDataProviderImpl::Init()
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    uint32_t sum     = 0;

#if CHIP_DEVICE_CONFIG_SECURE_DAC_PRIVATE_KEY
    ConvertDacKey();
#endif

    for (uint8_t i = 1; i < FactoryDataProvider::kNumberOfIds; i++)
    {
        sum += maxLengths[i];
    }

    if(sum > kFactoryDataSize)
    {
        ChipLogError(DeviceLayer,
            "Max size of factory data: %lu is bigger than reserved factory data size: %lu",
            sum, kFactoryDataSize
        );
    }

    error = Validate();
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Factory data init failed with: %s", ErrorStr(error));
    }

    return error;
}

CHIP_ERROR FactoryDataProviderImpl::SignWithDacKey(const ByteSpan & messageToSign, MutableByteSpan & outSignBuffer)
{
    Crypto::P256ECDSASignature signature;
    Crypto::P256Keypair keypair;
    Crypto::P256SerializedKeypair serializedKeypair;

    VerifyOrReturnError(IsSpanUsable(outSignBuffer), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSpanUsable(messageToSign), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(outSignBuffer.size() >= signature.Capacity(), CHIP_ERROR_BUFFER_TOO_SMALL);

    /* Get private key of DAC certificate from reserved section */
    uint8_t keyBuf[Crypto::kP256_PrivateKey_Length];
    MutableByteSpan dacPrivateKeySpan(keyBuf);
    uint16_t keySize = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kDacPrivateKeyId, dacPrivateKeySpan.data(), dacPrivateKeySpan.size(), keySize));
    dacPrivateKeySpan.reduce_size(keySize);

    /* Only the private key is used when signing */
    ReturnErrorOnFailure(serializedKeypair.SetLength(Crypto::kP256_PublicKey_Length + dacPrivateKeySpan.size()));
    memcpy(serializedKeypair.Bytes() + Crypto::kP256_PublicKey_Length, dacPrivateKeySpan.data(), dacPrivateKeySpan.size());

    ReturnErrorOnFailure(keypair.Deserialize(serializedKeypair));
    ReturnErrorOnFailure(keypair.ECDSA_sign_msg(messageToSign.data(), messageToSign.size(), signature));

    // TODO: sanitize temporary buffers used to store the private key, so it doesn't leak on the stack?

    return CopySpanToMutableSpan(ByteSpan{ signature.ConstBytes(), signature.Length() }, outSignBuffer);
}

#if CHIP_DEVICE_CONFIG_SECURE_DAC_PRIVATE_KEY
void FactoryDataProviderImpl::ConvertDacKey()
{
    // Get private key from internal flash
    // Send it to S200 and export blob
    // Compute factory data hash
    // Overwrite old hash
    // Overwrite old private key with the blob
}
#endif

} // namespace DeviceLayer
} // namespace chip
