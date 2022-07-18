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
#include <psa/crypto.h>
#include "em_device.h"

// Includes needed for certificate parsing
#if defined(MBEDTLS_X509_CRT_PARSE_C)
#include <mbedtls/x509_crt.h>
#endif // defined(MBEDTLS_X509_CRT_PARSE_C)
#include <mbedtls/oid.h>
#include <mbedtls/x509.h>
#include <mbedtls/x509_csr.h>
#include <mbedtls/md.h>

#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/SafeInt.h>
#include <lib/support/CHIPMem.h>
using chip::Platform::MemoryCalloc;
using chip::Platform::MemoryFree;

using chip::Crypto::P256PublicKey;
using chip::Crypto::P256Keypair;
using chip::Crypto::P256ECDSASignature;
using chip::Crypto::P256ECDHDerivedSecret;
using chip::Crypto::P256SerializedKeypair;

namespace chip {
namespace DeviceLayer {
namespace Internal {

/*******************************************************************************
 *
 * PSA key ID range for storing Matter Opaque keys
 *
 ******************************************************************************/
#define PSA_KEY_ID_FOR_MATTER_MIN   (0x00004400)
#define PSA_KEY_ID_FOR_MATTER_MAX   (0x000045FF)
#define PSA_KEY_ID_FOR_MATTER_SIZE  (PSA_KEY_ID_FOR_MATTER_MAX - PSA_KEY_ID_FOR_MATTER_MIN + 1)

static_assert((kEFR32OpaqueKeyIdPersistentMax - kEFR32OpaqueKeyIdPersistentMin) < PSA_KEY_ID_FOR_MATTER_SIZE, "Not enough PSA range to store all allowed opaque key IDs");

#if defined(SEMAILBOX_PRESENT) && (_SILICON_LABS_SECURITY_FEATURE == _SILICON_LABS_SECURITY_FEATURE_VAULT)
#define PSA_CRYPTO_LOCATION_FOR_DEVICE PSA_KEY_LOCATION_SL_SE_OPAQUE
#elif defined(CRYPTOACC_PRESENT) && defined(SEPUF_PRESENT) && defined(SL_TRUSTZONE_NONSECURE)
#define PSA_CRYPTO_LOCATION_FOR_DEVICE PSA_KEY_LOCATION_SL_CRYPTOACC_OPAQUE
#else
#define PSA_CRYPTO_LOCATION_FOR_DEVICE PSA_KEY_LOCATION_LOCAL_STORAGE
#endif

static void _log_mbedTLS_error(int error_code)
{
    if (error_code != 0)
    {
#if defined(MBEDTLS_ERROR_C)
        char error_str[MAX_ERROR_STR_LEN];
        mbedtls_strerror(error_code, error_str, sizeof(error_str));
        ChipLogError(Crypto, "mbedTLS error: %s", error_str);
#else
        // Error codes defined in 16-bit negative hex numbers. Ease lookup by printing likewise
        ChipLogError(Crypto, "mbedTLS error: -0x%04X", -static_cast<uint16_t>(error_code));
#endif
    }
}

static void _log_PSA_error(psa_status_t status)
{
    if (status != 0)
    {
        // Error codes defined in 16-bit negative hex numbers. Ease lookup by printing likewise
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
    // Avoid having a reference to PSA datatypes in the signature of this class
    mContext = MemoryCalloc(1, sizeof(mbedtls_svc_key_id_t));
}

EFR32OpaqueKeypair::~EFR32OpaqueKeypair()
{
    // Free dynamic resource
    if (mContext != nullptr)
    {
        MemoryFree(mContext);
        mContext = nullptr;
    }
}

CHIP_ERROR EFR32OpaqueKeypair::Load(EFR32OpaqueKeyId key_id)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    psa_status_t status = PSA_ERROR_BAD_STATE;

    VerifyOrExit(key_id != kEFR32OpaqueKeyIdVolatile, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(is_opaque_key_valid(key_id), error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mContext, error = CHIP_ERROR_INCORRECT_STATE);

    psa_crypto_init();

    // If the object contains a volatile key, clean it up before reusing the object storage
    if (mHasKey && !mIsPersistent)
    {
        Delete();
    }

    status = psa_export_public_key(psa_key_id_from_opaque(key_id),
                                   mPubkeyRef, mPubkeySize, &mPubkeyLength);

    if (status == PSA_ERROR_DOES_NOT_EXIST)
    {
        error = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
        goto exit;
    }

    VerifyOrExit(status == PSA_SUCCESS, { _log_PSA_error(status); error = CHIP_ERROR_INTERNAL; });

    // Store the key ID and mark the key as valid
    *(mbedtls_svc_key_id_t*) mContext = psa_key_id_from_opaque(key_id);
    mHasKey = true;
    mIsPersistent = true;

exit:
    if (error != CHIP_NO_ERROR)
    {
        memset(mPubkeyRef, 0, mPubkeySize);
    }

    return error;
}

CHIP_ERROR EFR32OpaqueKeypair::Create(EFR32OpaqueKeyId key_id, EFR32OpaqueKeyUsages usage)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    psa_status_t status = PSA_ERROR_BAD_STATE;
    psa_key_attributes_t attr = PSA_KEY_ATTRIBUTES_INIT;

    VerifyOrExit(is_opaque_key_valid(key_id), error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mContext, error = CHIP_ERROR_INCORRECT_STATE);

    psa_crypto_init();

    if (key_id == kEFR32OpaqueKeyIdVolatile)
    {
        psa_set_key_lifetime(&attr, PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(
                                        PSA_KEY_LIFETIME_VOLATILE,
                                        PSA_CRYPTO_LOCATION_FOR_DEVICE));
    }
    else
    {
        psa_set_key_lifetime(&attr, PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(
                                        PSA_KEY_LIFETIME_PERSISTENT,
                                        PSA_CRYPTO_LOCATION_FOR_DEVICE));
        psa_set_key_id(&attr, psa_key_id_from_opaque(key_id));
    }

    switch (usage)
    {
        case EFR32OpaqueKeyUsages::ECDSA_P256_SHA256:
            psa_set_key_type(&attr, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
            psa_set_key_bits(&attr, 256);
            psa_set_key_algorithm(&attr, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
            // Need hash signing permissions because the CSR generation uses sign_hash internally
            psa_set_key_usage_flags(&attr, PSA_KEY_USAGE_SIGN_MESSAGE |
                                           PSA_KEY_USAGE_SIGN_HASH);
            break;
        case EFR32OpaqueKeyUsages::ECDH_P256:
            psa_set_key_type(&attr, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
            psa_set_key_bits(&attr, 256);
            psa_set_key_algorithm(&attr, PSA_ALG_ECDH);
            psa_set_key_usage_flags(&attr, PSA_KEY_USAGE_DERIVE);
            break;
    }

    status = psa_generate_key(&attr, (mbedtls_svc_key_id_t*) mContext);
    VerifyOrExit(status == PSA_SUCCESS, { _log_PSA_error(status); error = CHIP_ERROR_INTERNAL; });

    // Export the public key
    status = psa_export_public_key(*(mbedtls_svc_key_id_t*) mContext,
                                   mPubkeyRef, mPubkeySize, &mPubkeyLength);

    if (status != PSA_SUCCESS)
    {
        _log_PSA_error(status);
        // Key generation succeeded, but pubkey export did not. To avoid
        // memory leaks, delete the generated key before returning the error
        psa_destroy_key(*(mbedtls_svc_key_id_t*) mContext);
        error = CHIP_ERROR_INTERNAL;
        goto exit;
    }

    // Store the key ID and mark the key as valid
    mHasKey = true;
    mIsPersistent = key_id != kEFR32OpaqueKeyIdVolatile;

exit:
    psa_reset_key_attributes(&attr);

    if (error != CHIP_NO_ERROR && mContext)
    {
        *(mbedtls_svc_key_id_t*) mContext = 0;
    }
    return error;
}

CHIP_ERROR EFR32OpaqueKeypair::GetPublicKey(uint8_t* output, size_t output_size, size_t * output_length) const
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

    return opaque_key_id_from_psa(*(mbedtls_svc_key_id_t*) mContext);
}

CHIP_ERROR EFR32OpaqueKeypair::Sign(const uint8_t* msg, size_t msg_len,
                    uint8_t* output, size_t output_size, size_t * output_length) const
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    psa_status_t status = PSA_ERROR_BAD_STATE;

    VerifyOrExit(mHasKey, error = CHIP_ERROR_INCORRECT_STATE);

    status = psa_sign_message(
                *(mbedtls_svc_key_id_t*) mContext,
                PSA_ALG_ECDSA(PSA_ALG_SHA_256),
                msg, msg_len,
                output, output_size, output_length);

    VerifyOrExit(status == PSA_SUCCESS, { _log_PSA_error(status); error = CHIP_ERROR_INTERNAL; });

exit:
    return error;
}

CHIP_ERROR EFR32OpaqueKeypair::Derive(const uint8_t* their_key, size_t their_key_len,
                      uint8_t* output, size_t output_size, size_t * output_length) const
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    psa_status_t status = PSA_ERROR_BAD_STATE;

    VerifyOrExit(mHasKey, error = CHIP_ERROR_INCORRECT_STATE);

    status = psa_raw_key_agreement(
                PSA_ALG_ECDH,
                *(mbedtls_svc_key_id_t*) mContext,
                their_key, their_key_len,
                output, output_size, output_length);

    VerifyOrExit(status == PSA_SUCCESS, { _log_PSA_error(status); error = CHIP_ERROR_INTERNAL; });

exit:
    return error;
}

CHIP_ERROR EFR32OpaqueKeypair::Delete()
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    psa_status_t status = PSA_ERROR_BAD_STATE;

    VerifyOrExit(mHasKey, error = CHIP_ERROR_INCORRECT_STATE);

    status = psa_destroy_key(*(mbedtls_svc_key_id_t*) mContext);
    VerifyOrExit(status == PSA_SUCCESS, { _log_PSA_error(status); error = CHIP_ERROR_INTERNAL; });

exit:
    mHasKey = false;
    mIsPersistent = false;
    memset(mPubkeyRef, 0, mPubkeySize);
    if (mContext)
    {
        *(mbedtls_svc_key_id_t*) mContext = 0;
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
    mPubkeyRef = mPubKey.Bytes();
    mPubkeySize = mPubKey.Length();
    mPubkeyLength = 0;
}

EFR32OpaqueP256Keypair::~EFR32OpaqueP256Keypair()
{

}

CHIP_ERROR EFR32OpaqueP256Keypair::Initialize()
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
    // Copied from mbedTLS, since x.509 is part of the TLS layer.
#if defined(MBEDTLS_X509_CSR_WRITE_C)
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;
    size_t out_length;

    mbedtls_x509write_csr csr;
    mbedtls_x509write_csr_init(&csr);

    VerifyOrExit(mHasKey, error = CHIP_ERROR_INCORRECT_STATE);

    psa_crypto_init();

    // Use PSA key to generate CSR
    mbedtls_pk_context pk;
    mbedtls_pk_init(&pk);
    result = mbedtls_pk_setup_opaque(&pk, *((mbedtls_svc_key_id_t*)mContext));
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(pk.MBEDTLS_PRIVATE(pk_info) != nullptr, error = CHIP_ERROR_INTERNAL);

    mbedtls_x509write_csr_set_key(&csr, &pk);

    mbedtls_x509write_csr_set_md_alg(&csr, MBEDTLS_MD_SHA256);

    // TODO: mbedTLS CSR parser fails if the subject name is not set (or if empty).
    //       CHIP Spec doesn't specify the subject name that can be used.
    //       Figure out the correct value and update this code.
    result = mbedtls_x509write_csr_set_subject_name(&csr, "O=CSR");
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_x509write_csr_der(&csr, out_csr, csr_length, nullptr, nullptr);
    VerifyOrExit(result > 0, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(CanCastTo<size_t>(result), error = CHIP_ERROR_INTERNAL);

    out_length = static_cast<size_t>(result);
    result     = 0;
    VerifyOrExit(out_length <= csr_length, error = CHIP_ERROR_INTERNAL);

    if (csr_length != out_length)
    {
        // mbedTLS API writes the CSR at the end of the provided buffer.
        // Let's move it to the start of the buffer.
        size_t offset = csr_length - out_length;
        memmove(out_csr, &out_csr[offset], out_length);
    }

    csr_length = out_length;

exit:
    mbedtls_x509write_csr_free(&csr);

    _log_mbedTLS_error(result);
    return error;
#else
    ChipLogError(Crypto, "MBEDTLS_X509_CSR_WRITE_C is not enabled. CSR cannot be created");
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif
}

CHIP_ERROR EFR32OpaqueP256Keypair::ECDSA_sign_msg(const uint8_t * msg, size_t msg_length, P256ECDSASignature & out_signature) const
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    size_t output_length = 0;

    error = Sign(msg, msg_length, out_signature.Bytes(), out_signature.Capacity(), &output_length);

    SuccessOrExit(error);
    SuccessOrExit(out_signature.SetLength(output_length));
exit:
    return error;
}

CHIP_ERROR EFR32OpaqueP256Keypair::ECDH_derive_secret(const P256PublicKey & remote_public_key, P256ECDHDerivedSecret & out_secret) const
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    size_t output_length = 0;

    error = Derive(Uint8::to_const_uchar(remote_public_key), remote_public_key.Length(),
                   Uint8::to_uchar(out_secret),
                   (out_secret.Length() == 0) ? out_secret.Capacity() : out_secret.Length(),
                   &output_length);

    SuccessOrExit(error);
    SuccessOrExit(out_secret.SetLength(output_length));
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
