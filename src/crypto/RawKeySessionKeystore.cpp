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

namespace chip {
namespace Crypto {

#ifdef ENABLE_HSM_HKDF
using HKDF_sha_crypto = HKDF_shaHSM;
#else
using HKDF_sha_crypto = HKDF_sha;
#endif

enum KeyUsage
{
    kI2RKey                  = 0,
    kR2IKey                  = 1,
    kAttestationChallengeKey = 2,
    kNumSessionKeys          = 3
};

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
    Aes128KeyByteArray keys[kNumSessionKeys];

    static_assert(AttestationChallenge::Capacity() == sizeof(Aes128KeyByteArray), "Unexpected attestation challenge size");

    ReturnErrorOnFailure(
        hkdf.HKDF_SHA256(secret.data(), secret.size(), salt.data(), salt.size(), info.data(), info.size(), keys[0], sizeof(keys)));

    memcpy(i2rKey.AsMutable<Aes128KeyByteArray>(), keys[kI2RKey], sizeof(Aes128KeyByteArray));
    memcpy(r2iKey.AsMutable<Aes128KeyByteArray>(), keys[kR2IKey], sizeof(Aes128KeyByteArray));
    memcpy(attestationChallenge.Bytes(), keys[kAttestationChallengeKey], sizeof(Aes128KeyByteArray));

    return CHIP_NO_ERROR;
}

void RawKeySessionKeystore::DestroyKey(Aes128KeyHandle & key)
{
    ClearSecretData(key.AsMutable<Aes128KeyByteArray>());
}

} // namespace Crypto
} // namespace chip
