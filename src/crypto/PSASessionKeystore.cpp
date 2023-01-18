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

#include <crypto/PSASessionKeystore.h>

#include <psa/crypto.h>

namespace chip {
namespace Crypto {

enum KeyUsage
{
    kI2RKey                  = 0,
    kR2IKey                  = 1,
    kAttestationChallengeKey = 2,
    kNumSessionKeys          = 3
};

CHIP_ERROR PSASessionKeystore::CreateKey(const Aes128KeyByteArray & keyMaterial, Aes128KeyHandle & key)
{
    constexpr psa_algorithm_t kAlgorithm = PSA_ALG_AEAD_WITH_AT_LEAST_THIS_LENGTH_TAG(PSA_ALG_CCM, 8);
    CHIP_ERROR error                     = CHIP_NO_ERROR;
    psa_status_t status                  = PSA_SUCCESS;
    psa_key_id_t keyId                   = 0;
    psa_key_attributes_t attrs           = PSA_KEY_ATTRIBUTES_INIT;

    // Destroy the old key if already allocated
    psa_destroy_key(key.As<psa_key_id_t>());

    psa_set_key_type(&attrs, PSA_KEY_TYPE_AES);
    psa_set_key_algorithm(&attrs, kAlgorithm);
    psa_set_key_usage_flags(&attrs, PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT);

    status = psa_import_key(&attrs, keyMaterial, sizeof(Aes128KeyByteArray), &keyId);
    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);

    key.AsMutable<psa_key_id_t>() = keyId;

exit:
    psa_reset_key_attributes(&attrs);

    return error;
}

CHIP_ERROR PSASessionKeystore::DeriveKey(const P256ECDHDerivedSecret & secret, const ByteSpan & salt, const ByteSpan & info,
                                         Aes128KeyHandle & key)
{
    HKDF_sha hkdf;
    Aes128KeyByteArray sr2k;

    ReturnErrorOnFailure(hkdf.HKDF_SHA256(secret.ConstBytes(), secret.Length(), salt.data(), salt.size(), info.data(), info.size(),
                                          sr2k, sizeof(Aes128KeyByteArray)));

    return CreateKey(sr2k, key);
}

CHIP_ERROR PSASessionKeystore::DeriveSessionKeys(const ByteSpan & secret, const ByteSpan & salt, const ByteSpan & info,
                                                 Aes128KeyHandle & i2rKey, Aes128KeyHandle & r2iKey,
                                                 AttestationChallenge & attestationChallenge)
{
    HKDF_sha hkdf;
    Aes128KeyByteArray keys[kNumSessionKeys];

    static_assert(AttestationChallenge::Capacity() == sizeof(Aes128KeyByteArray), "Unexpected attestation challenge size");

    ReturnErrorOnFailure(
        hkdf.HKDF_SHA256(secret.data(), secret.size(), salt.data(), salt.size(), info.data(), info.size(), keys[0], sizeof(keys)));

    CHIP_ERROR error;
    SuccessOrExit(error = CreateKey(keys[kI2RKey], i2rKey));
    SuccessOrExit(error = CreateKey(keys[kR2IKey], r2iKey));
    memcpy(attestationChallenge.Bytes(), keys[kAttestationChallengeKey], sizeof(Aes128KeyByteArray));

exit:
    if (error != CHIP_NO_ERROR)
    {
        DestroyKey(i2rKey);
        DestroyKey(r2iKey);
    }

    return error;
}

void PSASessionKeystore::DestroyKey(Aes128KeyHandle & key)
{
    auto & keyId = key.AsMutable<psa_key_id_t>();

    psa_destroy_key(keyId);
    keyId = 0;
}

} // namespace Crypto
} // namespace chip
