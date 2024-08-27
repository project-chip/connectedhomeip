/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "Efr32OpaqueKeypair.h"
#include "em_device.h"
#include <psa/crypto.h>
#include <sl_psa_crypto.h>

#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/SafeInt.h>
using chip::Platform::MemoryCalloc;
using chip::Platform::MemoryFree;

using chip::Crypto::P256ECDHDerivedSecret;
using chip::Crypto::P256ECDSASignature;
using chip::Crypto::P256Keypair;
using chip::Crypto::P256PublicKey;
using chip::Crypto::P256SerializedKeypair;

namespace chip {
namespace DeviceLayer {
namespace Internal {

/*******************************************************************************
 *
 * PSA key ID range for storing Matter Opaque keys
 *
 ******************************************************************************/
#define PSA_KEY_ID_FOR_MATTER_MIN (0x00004400)
#define PSA_KEY_ID_FOR_MATTER_MAX (0x000045FF)
#define PSA_KEY_ID_FOR_MATTER_SIZE (PSA_KEY_ID_FOR_MATTER_MAX - PSA_KEY_ID_FOR_MATTER_MIN + 1)

static_assert((kEFR32OpaqueKeyIdPersistentMax - kEFR32OpaqueKeyIdPersistentMin) < PSA_KEY_ID_FOR_MATTER_SIZE,
              "Not enough PSA range to store all allowed opaque key IDs");

static void _log_PSA_error(psa_status_t status)
{
    if (status != PSA_SUCCESS)
    {
        ChipLogError(Crypto, "PSA error: %ld", status);
    }
}

/*******************************************************************************
 *
 * PSA Crypto backed implementation of EFR32OpaqueKeypair
 *
 ******************************************************************************/

static bool is_opaque_key_valid(EFR32OpaqueKeyId id)
{
    if (id == kEFR32OpaqueKeyIdVolatile)
    {
        return true;
    }
    else if (id >= kEFR32OpaqueKeyIdPersistentMin && id <= (kEFR32OpaqueKeyIdPersistentMin + PSA_KEY_ID_FOR_MATTER_SIZE))
    {
        return true;
    }

    return false;
}

static mbedtls_svc_key_id_t psa_key_id_from_opaque(EFR32OpaqueKeyId id)
{
    if (id == kEFR32OpaqueKeyIdVolatile || !is_opaque_key_valid(id))
    {
        return 0;
    }

    return PSA_KEY_ID_FOR_MATTER_MIN + (id - kEFR32OpaqueKeyIdPersistentMin);
}

static EFR32OpaqueKeyId opaque_key_id_from_psa(mbedtls_svc_key_id_t id)
{
    if (id == 0)
    {
        return kEFR32OpaqueKeyIdVolatile;
    }
    else if (id >= PSA_KEY_ID_FOR_MATTER_MIN && id <= PSA_KEY_ID_FOR_MATTER_MAX)
    {
        return (id + kEFR32OpaqueKeyIdPersistentMin) - PSA_KEY_ID_FOR_MATTER_MIN;
    }
    else
    {
        return kEFR32OpaqueKeyIdUnknown;
    }
}

EFR32OpaqueKeypair::EFR32OpaqueKeypair()
{
    psa_crypto_init();
    // Avoid having a reference to PSA datatypes in the signature of this class
    mContext = MemoryCalloc(1, sizeof(mbedtls_svc_key_id_t));
}

EFR32OpaqueKeypair::~EFR32OpaqueKeypair()
{
    // Free key resources
    if (mContext != nullptr)
    {
        // Delete volatile keys, since nobody else can after we drop the key ID.
        if (!mIsPersistent)
        {
            DestroyKey();
        }

        MemoryFree(mContext);
        mContext = nullptr;
    }
}

CHIP_ERROR EFR32OpaqueKeypair::Load(EFR32OpaqueKeyId opaque_id)
{
    CHIP_ERROR error            = CHIP_NO_ERROR;
    psa_status_t status         = PSA_ERROR_BAD_STATE;
    mbedtls_svc_key_id_t key_id = 0;

    VerifyOrExit(opaque_id != kEFR32OpaqueKeyIdVolatile, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(is_opaque_key_valid(opaque_id), error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mContext, error = CHIP_ERROR_INCORRECT_STATE);

    // If the object contains a volatile key, clean it up before reusing the object storage
    if (mHasKey && !mIsPersistent)
    {
        DestroyKey();
    }

    key_id = psa_key_id_from_opaque(opaque_id);

    status = psa_export_public_key(key_id, mPubkeyRef, mPubkeySize, &mPubkeyLength);

    if (status == PSA_ERROR_DOES_NOT_EXIST)
    {
        error = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
        goto exit;
    }

    VerifyOrExit(status == PSA_SUCCESS, {
        _log_PSA_error(status);
        error = CHIP_ERROR_INTERNAL;
    });

    // Store the key ID and mark the key as valid
    *(mbedtls_svc_key_id_t *) mContext = key_id;
    mHasKey                            = true;
    mIsPersistent                      = true;

exit:
    if (error != CHIP_NO_ERROR)
    {
        memset(mPubkeyRef, 0, mPubkeySize);
    }

    return error;
}

CHIP_ERROR EFR32OpaqueKeypair::Create(EFR32OpaqueKeyId opaque_id, EFR32OpaqueKeyUsages usage)
{
    CHIP_ERROR error            = CHIP_NO_ERROR;
    psa_status_t status         = PSA_ERROR_BAD_STATE;
    psa_key_attributes_t attr   = PSA_KEY_ATTRIBUTES_INIT;
    mbedtls_svc_key_id_t key_id = 0;

    VerifyOrExit(is_opaque_key_valid(opaque_id), error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mContext, error = CHIP_ERROR_INCORRECT_STATE);

    if (opaque_id == kEFR32OpaqueKeyIdVolatile)
    {
        psa_set_key_lifetime(
            &attr,
            PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(PSA_KEY_LIFETIME_VOLATILE, sl_psa_get_most_secure_key_location()));
    }
    else
    {
        psa_key_handle_t key_handle;

        key_id = psa_key_id_from_opaque(opaque_id);

        // Check if the key already exists
        int ret = psa_open_key(key_id, &key_handle);
        if (PSA_SUCCESS == ret)
        {
            // WARNING: Existing key! This is caused by a problem in the key store.
            // The key must be destroyed, otherwhise the device won't recover.
            ChipLogError(Crypto, "WARNING: PSA key recycled: %d / %ld", opaque_id, key_id);
            psa_destroy_key(key_id);
        }

        psa_set_key_id(&attr, key_id);
        psa_set_key_lifetime(
            &attr,
            PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(PSA_KEY_LIFETIME_PERSISTENT, sl_psa_get_most_secure_key_location()));
    }

    switch (usage)
    {
    case EFR32OpaqueKeyUsages::ECDSA_P256_SHA256:
        psa_set_key_type(&attr, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
        psa_set_key_bits(&attr, 256);
        psa_set_key_algorithm(&attr, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
        // Need hash signing permissions because the CSR generation uses sign_hash internally
        psa_set_key_usage_flags(&attr, PSA_KEY_USAGE_SIGN_MESSAGE | PSA_KEY_USAGE_SIGN_HASH);
        break;
    case EFR32OpaqueKeyUsages::ECDH_P256:
        psa_set_key_type(&attr, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
        psa_set_key_bits(&attr, 256);
        psa_set_key_algorithm(&attr, PSA_ALG_ECDH);
        psa_set_key_usage_flags(&attr, PSA_KEY_USAGE_DERIVE);
        break;
    }

    status = psa_generate_key(&attr, &key_id);
    VerifyOrExit(status == PSA_SUCCESS, {
        _log_PSA_error(status);
        error = CHIP_ERROR_INTERNAL;
    });

    // Export the public key
    status = psa_export_public_key(key_id, mPubkeyRef, mPubkeySize, &mPubkeyLength);
    if (PSA_SUCCESS != status)
    {
        _log_PSA_error(status);
        // Key generation succeeded, but pubkey export did not. To avoid
        // memory leaks, delete the generated key before returning the error
        psa_destroy_key(key_id);
        error = CHIP_ERROR_INTERNAL;
        goto exit;
    }

    // Store the key ID and mark the key as valid
    mHasKey       = true;
    mIsPersistent = opaque_id != kEFR32OpaqueKeyIdVolatile;

exit:
    psa_reset_key_attributes(&attr);
    if (mContext)
    {
        if (CHIP_NO_ERROR == error)
        {
            *(mbedtls_svc_key_id_t *) mContext = key_id;
        }
        else
        {
            *(mbedtls_svc_key_id_t *) mContext = 0;
        }
    }
    return error;
}

CHIP_ERROR EFR32OpaqueKeypair::GetPublicKey(uint8_t * output, size_t output_size, size_t * output_length) const
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    VerifyOrExit(mHasKey, error = CHIP_ERROR_INCORRECT_STATE);

    if (output_size >= mPubkeyLength)
    {
        memcpy(output, mPubkeyRef, mPubkeyLength);
        *output_length = mPubkeyLength;
    }
    else
    {
        error = CHIP_ERROR_BUFFER_TOO_SMALL;
    }
exit:
    return error;
}

EFR32OpaqueKeyId EFR32OpaqueKeypair::GetKeyId() const
{
    if (!mHasKey)
    {
        return kEFR32OpaqueKeyIdUnknown;
    }

    if (!mIsPersistent)
    {
        return kEFR32OpaqueKeyIdVolatile;
    }

    return opaque_key_id_from_psa(*(mbedtls_svc_key_id_t *) mContext);
}

CHIP_ERROR EFR32OpaqueKeypair::Sign(const uint8_t * msg, size_t msg_len, uint8_t * output, size_t output_size,
                                    size_t * output_length) const
{
    CHIP_ERROR error    = CHIP_NO_ERROR;
    psa_status_t status = PSA_ERROR_BAD_STATE;

    VerifyOrExit(mContext, error = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mHasKey, error = CHIP_ERROR_INCORRECT_STATE);

    status = psa_sign_message(*(mbedtls_svc_key_id_t *) mContext, PSA_ALG_ECDSA(PSA_ALG_SHA_256), msg, msg_len, output, output_size,
                              output_length);

    VerifyOrExit(status == PSA_SUCCESS, {
        _log_PSA_error(status);
        error = CHIP_ERROR_INTERNAL;
    });

exit:
    return error;
}

CHIP_ERROR EFR32OpaqueKeypair::Derive(const uint8_t * their_key, size_t their_key_len, uint8_t * output, size_t output_size,
                                      size_t * output_length) const
{
    CHIP_ERROR error    = CHIP_NO_ERROR;
    psa_status_t status = PSA_ERROR_BAD_STATE;

    VerifyOrExit(mHasKey, error = CHIP_ERROR_INCORRECT_STATE);

    status = psa_raw_key_agreement(PSA_ALG_ECDH, *(mbedtls_svc_key_id_t *) mContext, their_key, their_key_len, output, output_size,
                                   output_length);

    VerifyOrExit(status == PSA_SUCCESS, {
        _log_PSA_error(status);
        error = CHIP_ERROR_INTERNAL;
    });

exit:
    return error;
}

CHIP_ERROR EFR32OpaqueKeypair::DestroyKey()
{
    CHIP_ERROR error    = CHIP_NO_ERROR;
    psa_status_t status = PSA_ERROR_BAD_STATE;

    VerifyOrExit(mHasKey, error = CHIP_ERROR_INCORRECT_STATE);

    status = psa_destroy_key(*(mbedtls_svc_key_id_t *) mContext);
    VerifyOrExit(status == PSA_SUCCESS, {
        _log_PSA_error(status);
        error = CHIP_ERROR_INTERNAL;
    });

exit:
    mHasKey       = false;
    mIsPersistent = false;
    memset(mPubkeyRef, 0, mPubkeySize);
    if (mContext)
    {
        *(mbedtls_svc_key_id_t *) mContext = 0;
    }
    return error;
}

/*******************************************************************************
 *
 * PSA Crypto backed implementation of EFR32OpaqueP256Keypair
 *
 ******************************************************************************/
EFR32OpaqueP256Keypair::EFR32OpaqueP256Keypair()
{
    mPubkeyRef    = mPubKey.Bytes();
    mPubkeySize   = mPubKey.Length();
    mPubkeyLength = 0;
}

EFR32OpaqueP256Keypair::~EFR32OpaqueP256Keypair() {}

CHIP_ERROR EFR32OpaqueP256Keypair::Initialize(chip::Crypto::ECPKeyTarget key_target)
{
    if (mPubkeyLength > 0)
    {
        // already have a key - ECDH use case where CASESession is calling Initialize()
        return CHIP_NO_ERROR;
    }

    ChipLogError(Crypto, "Initialize() is invalid on opaque keys, use Create() instead");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR EFR32OpaqueP256Keypair::Serialize(P256SerializedKeypair & output) const
{
    ChipLogError(Crypto, "Serialisation is invalid on opaque keys, share the object instead");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR EFR32OpaqueP256Keypair::Deserialize(P256SerializedKeypair & input)
{
    ChipLogError(Crypto, "Serialisation is invalid on opaque keys");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR EFR32OpaqueP256Keypair::NewCertificateSigningRequest(uint8_t * out_csr, size_t & csr_length) const
{
    MutableByteSpan csr(out_csr, csr_length);
    CHIP_ERROR err = GenerateCertificateSigningRequest(this, csr);
    csr_length     = (CHIP_NO_ERROR == err) ? csr.size() : 0;
    return err;
}

CHIP_ERROR EFR32OpaqueP256Keypair::ECDSA_sign_msg(const uint8_t * msg, size_t msg_length, P256ECDSASignature & out_signature) const
{
    CHIP_ERROR error     = CHIP_NO_ERROR;
    size_t output_length = 0;

    VerifyOrExit((msg != nullptr) && (msg_length > 0), error = CHIP_ERROR_INVALID_ARGUMENT);

    error = Sign(msg, msg_length, out_signature.Bytes(), out_signature.Capacity(), &output_length);

    SuccessOrExit(error);
    SuccessOrExit(error = out_signature.SetLength(output_length));
exit:
    return error;
}

CHIP_ERROR EFR32OpaqueP256Keypair::ECDH_derive_secret(const P256PublicKey & remote_public_key,
                                                      P256ECDHDerivedSecret & out_secret) const
{
    CHIP_ERROR error     = CHIP_NO_ERROR;
    size_t output_length = 0;

    error = Derive(Uint8::to_const_uchar(remote_public_key), remote_public_key.Length(), out_secret.Bytes(),
                   (out_secret.Length() == 0) ? out_secret.Capacity() : out_secret.Length(), &output_length);

    SuccessOrExit(error);
    SuccessOrExit(error = out_secret.SetLength(output_length));
exit:
    return error;
}

const P256PublicKey & EFR32OpaqueP256Keypair::Pubkey() const
{
    return mPubKey;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
