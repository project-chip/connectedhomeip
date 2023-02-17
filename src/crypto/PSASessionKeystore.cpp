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

#include "PSASessionKeystore.h"

#include <crypto/CHIPCryptoPALPSA.h>

#include <psa/crypto.h>

namespace chip {
namespace Crypto {

namespace {

class AesKeyAttributes
{
public:
    AesKeyAttributes()
    {
        constexpr psa_algorithm_t kAlgorithm = PSA_ALG_AEAD_WITH_AT_LEAST_THIS_LENGTH_TAG(PSA_ALG_CCM, 8);

        psa_set_key_type(&mAttrs, PSA_KEY_TYPE_AES);
        psa_set_key_algorithm(&mAttrs, kAlgorithm);
        psa_set_key_usage_flags(&mAttrs, PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT);
        psa_set_key_bits(&mAttrs, CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES * 8);
    }

    ~AesKeyAttributes() { psa_reset_key_attributes(&mAttrs); }

    const psa_key_attributes_t & Get() { return mAttrs; }

private:
    psa_key_attributes_t mAttrs = PSA_KEY_ATTRIBUTES_INIT;
};

} // namespace

CHIP_ERROR PSASessionKeystore::CreateKey(const Aes128KeyByteArray & keyMaterial, Aes128KeyHandle & key)
{
    // Destroy the old key if already allocated
    psa_destroy_key(key.As<psa_key_id_t>());

    AesKeyAttributes attrs;
    psa_status_t status = psa_import_key(&attrs.Get(), keyMaterial, sizeof(Aes128KeyByteArray), &key.AsMutable<psa_key_id_t>());
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PSASessionKeystore::DeriveKey(const P256ECDHDerivedSecret & secret, const ByteSpan & salt, const ByteSpan & info,
                                         Aes128KeyHandle & key)
{
    PsaKdf kdf;
    ReturnErrorOnFailure(kdf.Init(PSA_ALG_HKDF(PSA_ALG_SHA_256), secret.Span(), salt, info));

    AesKeyAttributes attrs;

    return kdf.DeriveKey(attrs.Get(), key.AsMutable<psa_key_id_t>());
}

CHIP_ERROR PSASessionKeystore::DeriveSessionKeys(const ByteSpan & secret, const ByteSpan & salt, const ByteSpan & info,
                                                 Aes128KeyHandle & i2rKey, Aes128KeyHandle & r2iKey,
                                                 AttestationChallenge & attestationChallenge)
{
    PsaKdf kdf;
    ReturnErrorOnFailure(kdf.Init(PSA_ALG_HKDF(PSA_ALG_SHA_256), secret, salt, info));

    CHIP_ERROR error;
    AesKeyAttributes attrs;

    SuccessOrExit(error = kdf.DeriveKey(attrs.Get(), i2rKey.AsMutable<psa_key_id_t>()));
    SuccessOrExit(error = kdf.DeriveKey(attrs.Get(), r2iKey.AsMutable<psa_key_id_t>()));
    SuccessOrExit(error = kdf.DeriveBytes(MutableByteSpan(attestationChallenge.Bytes(), AttestationChallenge::Capacity())));

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
