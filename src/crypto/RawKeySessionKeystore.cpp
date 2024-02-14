/*
 *    Copyright (c) 2023 Project CHIP Authors
 *    All rights reserved.
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

#include <crypto/RawKeySessionKeystore.h>

#include <lib/support/BufferReader.h>

#include <cstdint>

namespace chip {
namespace Crypto {

// The underlying representation of the HKDF key handle
struct RawHkdfKeyHandle
{
    ByteSpan Span() const { return ByteSpan(data, size); }

    // Cap the data size so that the entire structure fits in the opaque context of the HKDF key handle.
    static constexpr size_t kMaxDataSize = std::min<size_t>(CHIP_CONFIG_HKDF_KEY_HANDLE_CONTEXT_SIZE - sizeof(uint8_t), UINT8_MAX);

    uint8_t data[kMaxDataSize];
    uint8_t size;
};

CHIP_ERROR RawKeySessionKeystore::CreateKey(const Symmetric128BitsKeyByteArray & keyMaterial, Aes128KeyHandle & key)
{
    memcpy(key.AsMutable<Symmetric128BitsKeyByteArray>(), keyMaterial, sizeof(Symmetric128BitsKeyByteArray));
    return CHIP_NO_ERROR;
}

CHIP_ERROR RawKeySessionKeystore::CreateKey(const Symmetric128BitsKeyByteArray & keyMaterial, Hmac128KeyHandle & key)
{
    memcpy(key.AsMutable<Symmetric128BitsKeyByteArray>(), keyMaterial, sizeof(Symmetric128BitsKeyByteArray));
    return CHIP_NO_ERROR;
}

CHIP_ERROR RawKeySessionKeystore::CreateKey(const ByteSpan & keyMaterial, HkdfKeyHandle & key)
{
    RawHkdfKeyHandle & rawKey = key.AsMutable<RawHkdfKeyHandle>();

    VerifyOrReturnError(keyMaterial.size() <= sizeof(rawKey.data), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(rawKey.data, keyMaterial.data(), keyMaterial.size());
    rawKey.size = static_cast<uint8_t>(keyMaterial.size());

    return CHIP_NO_ERROR;
}

CHIP_ERROR RawKeySessionKeystore::DeriveKey(const P256ECDHDerivedSecret & secret, const ByteSpan & salt, const ByteSpan & info,
                                            Aes128KeyHandle & key)
{
    HKDF_sha hkdf;

    return hkdf.HKDF_SHA256(secret.ConstBytes(), secret.Length(), salt.data(), salt.size(), info.data(), info.size(),
                            key.AsMutable<Symmetric128BitsKeyByteArray>(), sizeof(Symmetric128BitsKeyByteArray));
}

CHIP_ERROR RawKeySessionKeystore::DeriveSessionKeys(const ByteSpan & secret, const ByteSpan & salt, const ByteSpan & info,
                                                    Aes128KeyHandle & i2rKey, Aes128KeyHandle & r2iKey,
                                                    AttestationChallenge & attestationChallenge)
{
    HKDF_sha hkdf;
    uint8_t keyMaterial[2 * sizeof(Symmetric128BitsKeyByteArray) + AttestationChallenge::Capacity()];

    ReturnErrorOnFailure(hkdf.HKDF_SHA256(secret.data(), secret.size(), salt.data(), salt.size(), info.data(), info.size(),
                                          keyMaterial, sizeof(keyMaterial)));

    Encoding::LittleEndian::Reader reader(keyMaterial, sizeof(keyMaterial));

    return reader.ReadBytes(i2rKey.AsMutable<Symmetric128BitsKeyByteArray>(), sizeof(Symmetric128BitsKeyByteArray))
        .ReadBytes(r2iKey.AsMutable<Symmetric128BitsKeyByteArray>(), sizeof(Symmetric128BitsKeyByteArray))
        .ReadBytes(attestationChallenge.Bytes(), AttestationChallenge::Capacity())
        .StatusCode();
}

CHIP_ERROR RawKeySessionKeystore::DeriveSessionKeys(const HkdfKeyHandle & hkdfKey, const ByteSpan & salt, const ByteSpan & info,
                                                    Aes128KeyHandle & i2rKey, Aes128KeyHandle & r2iKey,
                                                    AttestationChallenge & attestationChallenge)
{
    return DeriveSessionKeys(hkdfKey.As<RawHkdfKeyHandle>().Span(), salt, info, i2rKey, r2iKey, attestationChallenge);
}

void RawKeySessionKeystore::DestroyKey(Symmetric128BitsKeyHandle & key)
{
    ClearSecretData(key.AsMutable<Symmetric128BitsKeyByteArray>());
}

void RawKeySessionKeystore::DestroyKey(HkdfKeyHandle & key)
{
    RawHkdfKeyHandle & rawKey = key.AsMutable<RawHkdfKeyHandle>();

    ClearSecretData(rawKey.data);
    rawKey.size = 0;
}

} // namespace Crypto
} // namespace chip
