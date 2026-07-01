/*
 *
 *    Copyright (c) 2020-2022, 2025 Project CHIP Authors
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

/**
 *    @file
 *    PSA Crypto based implementation of CHIP crypto primitives (host fallback)
 */

#include <crypto/CHIPCryptoPAL.h>
#include <crypto/CHIPCryptoPALmbedTLS.h>

#include <psa/crypto.h>
#include <type_traits>

#include <mbedtls/error.h>
#include <mbedtls/md.h>
#include <mbedtls/oid.h>
#include <mbedtls/pk.h>
#include <mbedtls/pkcs5.h>
#include <mbedtls/sha256.h>
#include <mbedtls/version.h>
#include <mbedtls/x509.h>
#include <mbedtls/x509_csr.h>
#if defined(MBEDTLS_X509_CRT_PARSE_C)
#include <mbedtls/x509_crt.h>
#endif // defined(MBEDTLS_X509_CRT_PARSE_C)

#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/SafePointerCast.h>
#include <lib/support/logging/CHIPLogging.h>

#include <string.h>

namespace chip {
namespace Crypto {

// Structure to hold PSA key ID inside P256KeypairContext
struct PsaP256KeyContext
{
    psa_key_id_t key_id;
};

static_assert(sizeof(PsaP256KeyContext) <= sizeof(P256KeypairContext), "PsaP256KeyContext must fit in P256KeypairContext");

static inline PsaP256KeyContext * to_psa_context(P256KeypairContext * context)
{
    return SafePointerCast<PsaP256KeyContext *>(context);
}

#if MBEDTLS_VERSION_NUMBER < 0x04000000
static inline const mbedtls_ecp_keypair * to_const_keypair(const P256KeypairContext * context)
{
    return SafePointerCast<const mbedtls_ecp_keypair *>(context);
}
#endif

static inline const PsaP256KeyContext * to_const_psa_context(const P256KeypairContext * context)
{
    return SafePointerCast<const PsaP256KeyContext *>(context);
}

#if MBEDTLS_VERSION_NUMBER < 0x04000000
static int CryptoRNG(void * ctxt, uint8_t * out_buffer, size_t out_length)
{
    return (chip::Crypto::DRBG_get_bytes(out_buffer, out_length) == CHIP_NO_ERROR) ? 0 : 1;
}
#endif

CHIP_ERROR Initialize_H(P256Keypair * pk, P256PublicKey * mPublicKey, P256KeypairContext * mKeypair)
{
    CHIP_ERROR error    = CHIP_NO_ERROR;
    psa_status_t result = PSA_SUCCESS;
    size_t pubkey_size  = 0;
    psa_key_id_t key_id = PSA_KEY_ID_NULL;

    pk->Clear();

    // Initialize the context
    PsaP256KeyContext * ctx = to_psa_context(mKeypair);
    memset(ctx, 0, sizeof(PsaP256KeyContext));

    // Set up PSA key attributes for P-256
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
    psa_set_key_bits(&attributes, kP256_PrivateKey_Length * 8);
    psa_set_key_usage_flags(&attributes,
                            PSA_KEY_USAGE_SIGN_HASH | PSA_KEY_USAGE_VERIFY_HASH | PSA_KEY_USAGE_DERIVE | PSA_KEY_USAGE_EXPORT);
    psa_set_key_algorithm(&attributes, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
    psa_set_key_enrollment_algorithm(&attributes, PSA_ALG_ECDH);
    psa_set_key_lifetime(&attributes, PSA_KEY_LIFETIME_VOLATILE);

    // Generate the key pair
    result = psa_generate_key(&attributes, &key_id);
    VerifyOrExit(result == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);

    // Export the public key
    result = psa_export_public_key(key_id, Uint8::to_uchar(mPublicKey->Bytes()), mPublicKey->Length(), &pubkey_size);
    VerifyOrExit(result == PSA_SUCCESS, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(pubkey_size == mPublicKey->Length(), error = CHIP_ERROR_INVALID_ARGUMENT);

    // Store the key ID in the context
    ctx->key_id = key_id;
    key_id      = PSA_KEY_ID_NULL;

exit:
    if (key_id != PSA_KEY_ID_NULL)
    {
        psa_destroy_key(key_id);
    }
    psa_reset_key_attributes(&attributes);

    _log_mbedTLS_error(result);
    return error;
}

CHIP_ERROR ECDSA_sign_msg_H(P256KeypairContext * mKeypair, const uint8_t * msg, const size_t msg_length,
                            P256ECDSASignature & out_signature)
{
    VerifyOrReturnError((msg != nullptr) && (msg_length > 0), CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t digest[kSHA256_Hash_Length];
    memset(&digest[0], 0, sizeof(digest));
    ReturnErrorOnFailure(Hash_SHA256(msg, msg_length, &digest[0]));

    CHIP_ERROR error        = CHIP_NO_ERROR;
    psa_status_t result     = PSA_SUCCESS;
    size_t signature_length = 0;

    const PsaP256KeyContext * ctx = to_const_psa_context(mKeypair);

    // PSA sign hash - directly produces signature in r||s format
    result = psa_sign_hash(ctx->key_id, PSA_ALG_ECDSA(PSA_ALG_SHA_256), digest, sizeof(digest), out_signature.Bytes(),
                           out_signature.Capacity(), &signature_length);

    VerifyOrExit(result == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(signature_length == kP256_ECDSA_Signature_Length_Raw, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(out_signature.SetLength(signature_length) == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

exit:
    _log_mbedTLS_error(result);
    return error;
}

CHIP_ERROR ECDH_derive_secret_H(P256KeypairContext * mKeypair, const P256PublicKey & remote_public_key,
                                P256ECDHDerivedSecret & out_secret)
{
    CHIP_ERROR error              = CHIP_NO_ERROR;
    psa_status_t result           = PSA_SUCCESS;
    size_t secret_length          = 0;
    const PsaP256KeyContext * ctx = to_const_psa_context(mKeypair);

    // Perform ECDH key agreement directly with peer public key bytes
    result = psa_raw_key_agreement(PSA_ALG_ECDH, ctx->key_id, Uint8::to_const_uchar(remote_public_key), remote_public_key.Length(),
                                   out_secret.Bytes(), out_secret.Capacity(), &secret_length);
    VerifyOrExit(result == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);
    TEMPORARY_RETURN_IGNORED out_secret.SetLength(secret_length);

exit:
    _log_mbedTLS_error(result);
    return error;
}

CHIP_ERROR NewCertificateSigningRequest_H(P256KeypairContext * mKeypair, uint8_t * out_csr, size_t & csr_length)
{
#if defined(MBEDTLS_X509_CSR_WRITE_C)
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;
    size_t out_length;

    const PsaP256KeyContext * ctx = to_const_psa_context(mKeypair);

    mbedtls_x509write_csr csr;
    mbedtls_x509write_csr_init(&csr);

    // Wrap PSA key ID in an mbedtls_pk_context using opaque key setup
    mbedtls_pk_context pk;
    mbedtls_pk_init(&pk);

    // In Mbed TLS 4, we use mbedtls_pk_wrap_psa to wrap PSA key
#if MBEDTLS_VERSION_NUMBER < 0x04000000
    result = mbedtls_pk_setup_opaque(&pk, ctx->key_id);
#else
    result = mbedtls_pk_wrap_psa(&pk, ctx->key_id);
#endif
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    mbedtls_x509write_csr_set_key(&csr, &pk);
    mbedtls_x509write_csr_set_md_alg(&csr, MBEDTLS_MD_SHA256);
    result = mbedtls_x509write_csr_set_subject_name(&csr, "O=CSR");
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

#if MBEDTLS_VERSION_NUMBER < 0x04000000
    result = mbedtls_x509write_csr_der(&csr, out_csr, csr_length, CryptoRNG, nullptr);
#else
    result = mbedtls_x509write_csr_der(&csr, out_csr, csr_length);
#endif
    VerifyOrExit(result > 0, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(CanCastTo<size_t>(result), error = CHIP_ERROR_INTERNAL);

    out_length = static_cast<size_t>(result);
    result     = 0;
    VerifyOrExit(out_length <= csr_length, error = CHIP_ERROR_INTERNAL);

    if (csr_length != out_length)
    {
        // mbedTLS API writes the CSR at the end of the provided buffer.
        size_t offset = csr_length - out_length;
        memmove(out_csr, &out_csr[offset], out_length);
    }

    csr_length = out_length;

exit:
    mbedtls_pk_free(&pk);
    mbedtls_x509write_csr_free(&csr);

    _log_mbedTLS_error(result);
    return error;
#else
    ChipLogError(Crypto, "MBEDTLS_X509_CSR_WRITE_C is not enabled. CSR cannot be created...");
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif
}

CHIP_ERROR Serialize_H(const P256KeypairContext mKeypair, const P256PublicKey mPublicKey, P256SerializedKeypair & output)
{
    const PsaP256KeyContext * ctx = to_const_psa_context(&mKeypair);

    size_t len = output.Length() == 0 ? output.Capacity() : output.Length();
    Encoding::BufferWriter bbuf(output.Bytes(), len);
    uint8_t privkey[kP256_PrivateKey_Length];
    CHIP_ERROR error    = CHIP_NO_ERROR;
    psa_status_t result = PSA_SUCCESS;
    size_t privkey_size = 0;

    // Write the public key to the output buffer
    bbuf.Put(mPublicKey, mPublicKey.Length());
    VerifyOrExit(bbuf.Available() == sizeof(privkey), error = CHIP_ERROR_INTERNAL);

    // Export private key from PSA key storage
    result = psa_export_key(ctx->key_id, Uint8::to_uchar(privkey), sizeof(privkey), &privkey_size);
    VerifyOrExit(result == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(privkey_size == kP256_PrivateKey_Length, error = CHIP_ERROR_INTERNAL);

    bbuf.Put(privkey, sizeof(privkey));
    VerifyOrExit(bbuf.Fit(), error = CHIP_ERROR_BUFFER_TOO_SMALL);
    TEMPORARY_RETURN_IGNORED output.SetLength(bbuf.Needed());
exit:
    ClearSecretData(privkey, sizeof(privkey));
    _log_mbedTLS_error(result);
    return error;
}

CHIP_ERROR Deserialize_H(P256Keypair * pk, P256PublicKey * mPublicKey, P256KeypairContext * mKeypair, P256SerializedKeypair & input)
{
    Encoding::BufferWriter bbuf(*mPublicKey, mPublicKey->Length());

    psa_status_t result             = PSA_SUCCESS;
    CHIP_ERROR error                = CHIP_NO_ERROR;
    psa_key_id_t key_id             = PSA_KEY_ID_NULL;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    PsaP256KeyContext * ctx         = NULL;
    const uint8_t * privkey         = NULL;

    VerifyOrExit(input.Length() == mPublicKey->Length() + kP256_PrivateKey_Length, error = CHIP_ERROR_INVALID_ARGUMENT);

    // Extract private key bytes
    privkey = input.ConstBytes() + mPublicKey->Length();

    pk->Clear();

    // Initialize the context
    ctx = to_psa_context(mKeypair);
    memset(ctx, 0, sizeof(PsaP256KeyContext));

    // Set up PSA key attributes for importing the private key
    psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
    // psa_set_key_bits(&attributes, 256);
    psa_set_key_bits(&attributes, kP256_PrivateKey_Length * 8);
    psa_set_key_usage_flags(&attributes,
                            PSA_KEY_USAGE_SIGN_HASH | PSA_KEY_USAGE_VERIFY_HASH | PSA_KEY_USAGE_DERIVE | PSA_KEY_USAGE_EXPORT);

    psa_set_key_algorithm(&attributes, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
    // psa_set_key_enrollment_algorithm(&attributes, PSA_ALG_ECDH);
    psa_set_key_lifetime(&attributes, PSA_KEY_LIFETIME_VOLATILE);

    // Import the raw private key scalar into PSA
    result = psa_import_key(&attributes, privkey, kP256_PrivateKey_Length, &key_id);
    psa_reset_key_attributes(&attributes);
    VerifyOrExit(result == PSA_SUCCESS, error = CHIP_ERROR_INVALID_ARGUMENT);

    // Extract public key from serialized input
    bbuf.Put(input.ConstBytes(), mPublicKey->Length());
    VerifyOrExit(bbuf.Fit(), error = CHIP_ERROR_NO_MEMORY);

    // Store key ID in context - ownership transferred
    ctx->key_id = key_id;
    key_id      = PSA_KEY_ID_NULL;

exit:
    if (key_id != PSA_KEY_ID_NULL)
    {
        psa_destroy_key(key_id);
    }
    _log_mbedTLS_error(result);
    return error;
}

} // namespace Crypto
} // namespace chip
