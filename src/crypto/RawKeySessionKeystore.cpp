/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <crypto/RawKeySessionKeystore.h>

#include <lib/support/BufferReader.h>

namespace chip {
namespace Crypto {

using HKDF_sha_crypto = HKDF_sha;

CHIP_ERROR RawKeySessionKeystore::CreateKey(const Aes128KeyByteArray & keyMaterial, Aes128KeyHandle & key)
{
    memcpy(key.AsMutable<Aes128KeyByteArray>(), keyMaterial, sizeof(Aes128KeyByteArray));
    return CHIP_NO_ERROR;
}

CHIP_ERROR RawKeySessionKeystore::DeriveKey(const P256ECDHDerivedSecret & secret, const ByteSpan & salt, const ByteSpan & info,
                                            Aes128KeyHandle & key)
{
    HKDF_sha_crypto hkdf;

    return hkdf.HKDF_SHA256(secret.ConstBytes(), secret.Length(), salt.data(), salt.size(), info.data(), info.size(),
                            key.AsMutable<Aes128KeyByteArray>(), sizeof(Aes128KeyByteArray));
}

CHIP_ERROR RawKeySessionKeystore::DeriveSessionKeys(const ByteSpan & secret, const ByteSpan & salt, const ByteSpan & info,
                                                    Aes128KeyHandle & i2rKey, Aes128KeyHandle & r2iKey,
                                                    AttestationChallenge & attestationChallenge)
{
    HKDF_sha_crypto hkdf;
    uint8_t keyMaterial[2 * sizeof(Aes128KeyByteArray) + AttestationChallenge::Capacity()];

    ReturnErrorOnFailure(hkdf.HKDF_SHA256(secret.data(), secret.size(), salt.data(), salt.size(), info.data(), info.size(),
                                          keyMaterial, sizeof(keyMaterial)));

    Encoding::LittleEndian::Reader reader(keyMaterial, sizeof(keyMaterial));

    return reader.ReadBytes(i2rKey.AsMutable<Aes128KeyByteArray>(), sizeof(Aes128KeyByteArray))
        .ReadBytes(r2iKey.AsMutable<Aes128KeyByteArray>(), sizeof(Aes128KeyByteArray))
        .ReadBytes(attestationChallenge.Bytes(), AttestationChallenge::Capacity())
        .StatusCode();
}

void RawKeySessionKeystore::DestroyKey(Aes128KeyHandle & key)
{
    ClearSecretData(key.AsMutable<Aes128KeyByteArray>());
}

} // namespace Crypto
} // namespace chip
