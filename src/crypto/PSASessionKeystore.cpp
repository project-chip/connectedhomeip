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

#include <psa/crypto.h>

namespace chip {
namespace Crypto {

namespace {

class KeyAttributesBase
{
public:
    KeyAttributesBase(psa_key_type_t type, psa_algorithm_t algorithm, psa_key_usage_t usageFlags, size_t bits)
    {
        psa_set_key_type(&mAttrs, type);
        psa_set_key_algorithm(&mAttrs, algorithm);
        psa_set_key_usage_flags(&mAttrs, usageFlags);
        psa_set_key_bits(&mAttrs, bits);
        GetPSAKeyAllocator().UpdateKeyAttributes(mAttrs);
    }

    ~KeyAttributesBase() { psa_reset_key_attributes(&mAttrs); }

    const psa_key_attributes_t & Get() { return mAttrs; }

private:
    psa_key_attributes_t mAttrs = PSA_KEY_ATTRIBUTES_INIT;
};

class AesKeyAttributes : public KeyAttributesBase
{
public:
    AesKeyAttributes() :
        KeyAttributesBase(PSA_KEY_TYPE_AES, PSA_ALG_AEAD_WITH_AT_LEAST_THIS_LENGTH_TAG(PSA_ALG_CCM, 8),
                          PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT | PSA_KEY_USAGE_COPY,
                          CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES * 8)
    {}
};

class HmacKeyAttributes : public KeyAttributesBase
{
public:
    HmacKeyAttributes() :
        KeyAttributesBase(PSA_KEY_TYPE_HMAC, PSA_ALG_HMAC(PSA_ALG_SHA_256), PSA_KEY_USAGE_SIGN_MESSAGE | PSA_KEY_USAGE_COPY,
                          CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES * 8)
    {}
};

class HkdfKeyAttributes : public KeyAttributesBase
{
public:
    HkdfKeyAttributes() : KeyAttributesBase(PSA_KEY_TYPE_DERIVE, PSA_ALG_HKDF(PSA_ALG_SHA_256), PSA_KEY_USAGE_DERIVE, 0) {}
};

#if CHIP_CONFIG_ENABLE_ICD_CIP
void SetKeyId(Symmetric128BitsKeyHandle & key, psa_key_id_t newKeyId)
{
    auto & KeyId = key.AsMutable<psa_key_id_t>();

    KeyId = newKeyId;
}
#endif
} // namespace

CHIP_ERROR PSASessionKeystore::CreateKey(const Symmetric128BitsKeyByteArray & keyMaterial, Aes128KeyHandle & key)
{
    // Destroy the old key if already allocated
    DestroyKey(key);

    AesKeyAttributes attrs;
    psa_status_t status =
        psa_import_key(&attrs.Get(), keyMaterial, sizeof(Symmetric128BitsKeyByteArray), &key.AsMutable<psa_key_id_t>());
    LogPsaError(status);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PSASessionKeystore::CreateKey(const Symmetric128BitsKeyByteArray & keyMaterial, Hmac128KeyHandle & key)
{
    // Destroy the old key if already allocated
    DestroyKey(key);

    HmacKeyAttributes attrs;
    psa_status_t status =
        psa_import_key(&attrs.Get(), keyMaterial, sizeof(Symmetric128BitsKeyByteArray), &key.AsMutable<psa_key_id_t>());
    LogPsaError(status);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PSASessionKeystore::CreateKey(const ByteSpan & keyMaterial, HkdfKeyHandle & key)
{
    // Destroy the old key if already allocated
    psa_destroy_key(key.As<psa_key_id_t>());

    HkdfKeyAttributes attrs;
    psa_status_t status = psa_import_key(&attrs.Get(), keyMaterial.data(), keyMaterial.size(), &key.AsMutable<psa_key_id_t>());
    LogPsaError(status);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PSASessionKeystore::DeriveKey(const P256ECDHDerivedSecret & secret, const ByteSpan & salt, const ByteSpan & info,
                                         Aes128KeyHandle & key)
{
    PsaKdf kdf;
    ReturnErrorOnFailure(kdf.Init(secret.Span(), salt, info));

    AesKeyAttributes attrs;

    return kdf.DeriveKey(attrs.Get(), key.AsMutable<psa_key_id_t>());
}

CHIP_ERROR PSASessionKeystore::DeriveSessionKeys(const ByteSpan & secret, const ByteSpan & salt, const ByteSpan & info,
                                                 Aes128KeyHandle & i2rKey, Aes128KeyHandle & r2iKey,
                                                 AttestationChallenge & attestationChallenge)
{
    PsaKdf kdf;
    ReturnErrorOnFailure(kdf.Init(secret, salt, info));

    return DeriveSessionKeys(kdf, i2rKey, r2iKey, attestationChallenge);
}

CHIP_ERROR PSASessionKeystore::DeriveSessionKeys(const HkdfKeyHandle & hkdfKey, const ByteSpan & salt, const ByteSpan & info,
                                                 Aes128KeyHandle & i2rKey, Aes128KeyHandle & r2iKey,
                                                 AttestationChallenge & attestationChallenge)
{
    PsaKdf kdf;
    ReturnErrorOnFailure(kdf.Init(hkdfKey, salt, info));

    return DeriveSessionKeys(kdf, i2rKey, r2iKey, attestationChallenge);
}

CHIP_ERROR PSASessionKeystore::DeriveSessionKeys(PsaKdf & kdf, Aes128KeyHandle & i2rKey, Aes128KeyHandle & r2iKey,
                                                 AttestationChallenge & attestationChallenge)
{
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

void PSASessionKeystore::DestroyKey(Symmetric128BitsKeyHandle & key)
{
    auto & keyId = key.AsMutable<psa_key_id_t>();

    psa_destroy_key(keyId);
    keyId = 0;
}

void PSASessionKeystore::DestroyKey(HkdfKeyHandle & key)
{
    auto & keyId = key.AsMutable<psa_key_id_t>();

    psa_destroy_key(keyId);
    keyId = PSA_KEY_ID_NULL;
}

#if CHIP_CONFIG_ENABLE_ICD_CIP
CHIP_ERROR PSASessionKeystore::PersistICDKey(Symmetric128BitsKeyHandle & key)
{
    CHIP_ERROR err             = CHIP_NO_ERROR;
    psa_key_id_t newKeyId      = PSA_KEY_ID_NULL;
    psa_key_attributes_t attrs = PSA_KEY_ATTRIBUTES_INIT;

    psa_get_key_attributes(key.As<psa_key_id_t>(), &attrs);

    // Exit early if key is already persistent
    if (psa_get_key_lifetime(&attrs) == PSA_KEY_LIFETIME_PERSISTENT)
    {
        psa_reset_key_attributes(&attrs);
        return CHIP_NO_ERROR;
    }

    newKeyId = GetPSAKeyAllocator().AllocateICDKeyId();
    VerifyOrExit(PSA_KEY_ID_NULL != newKeyId, err = CHIP_ERROR_INTERNAL);

    psa_set_key_lifetime(&attrs, PSA_KEY_LIFETIME_PERSISTENT);
    psa_set_key_id(&attrs, newKeyId);
    GetPSAKeyAllocator().UpdateKeyAttributes(attrs);

    VerifyOrExit(psa_copy_key(key.As<psa_key_id_t>(), &attrs, &newKeyId) == PSA_SUCCESS, err = CHIP_ERROR_INTERNAL);

exit:
    DestroyKey(key);
    psa_reset_key_attributes(&attrs);

    if (err == CHIP_NO_ERROR)
    {
        SetKeyId(key, newKeyId);
    }

    return err;
}
#endif

} // namespace Crypto
} // namespace chip
