/*
 *
 * Copyright (c) 2024 Project CHIP Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file
 * HSM based implementation of CHIP crypto primitives
 * Based on configurations in CHIPCryptoPALHsm_config.h file,
 * chip crypto apis use either HSM or rollback to software implementation.
 */

#include "CHIPCryptoPALHsm_utils_trustm.h"
#include "optiga/optiga_util.h"
#include "optiga_crypt.h"
#include "optiga_lib_common.h"
#include "optiga_lib_types.h"
#include <lib/core/CHIPEncoding.h>

#define NIST256_HEADER_OFFSET 26
#define CRYPTO_KEYPAIR_KEYID_OFFSET 4

/* Used for CSR generation */
// Organisation info.
#define SUBJECT_STR "CSR"
#define ASN1_BIT_STRING 0x03
#define ASN1_NULL 0x05
#define ASN1_OID 0x06
#define ASN1_SEQUENCE 0x10
#define ASN1_SET 0x11
#define ASN1_UTF8_STRING 0x0C
#define ASN1_CONSTRUCTED 0x20
#define ASN1_CONTEXT_SPECIFIC 0x80

const uint8_t kTlvHeader = 2;

namespace chip {
namespace Crypto {

#define EC_NIST_P256_KP_HEADER                                                                                                     \
    {                                                                                                                              \
        0x30, 0x81, 0x87, 0x02, 0x01, 0x00, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01, 0x06, 0x08, 0x2A,    \
            0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, 0x04, 0x6D, 0x30, 0x6B, 0x02, 0x01, 0x01, 0x04, 0x20,                        \
    }

#define EC_NIST_P256_KP_PUB_HEADER                                                                                                 \
    {                                                                                                                              \
        0xA1, 0x44, 0x03, 0x42, 0x00,                                                                                              \
    }

#define NIST256_HEADER_LENGTH (26)

extern CHIP_ERROR Initialize_H(P256Keypair * pk, P256PublicKey * mPublicKey, P256KeypairContext * mKeypair);
extern CHIP_ERROR ECDSA_sign_msg_H(P256KeypairContext * mKeypair, const uint8_t * msg, const size_t msg_length,
                                   P256ECDSASignature & out_signature);
extern CHIP_ERROR ECDH_derive_secret_H(P256KeypairContext * mKeypair, const P256PublicKey & remote_public_key,
                                       P256ECDHDerivedSecret & out_secret);
extern CHIP_ERROR NewCertificateSigningRequest_H(P256KeypairContext * mKeypair, uint8_t * out_csr, size_t & csr_length);
extern CHIP_ERROR Deserialize_H(P256Keypair * pk, P256PublicKey * mPublicKey, P256KeypairContext * mKeypair,
                                P256SerializedKeypair & input);
extern CHIP_ERROR Serialize_H(const P256KeypairContext mKeypair, const P256PublicKey mPublicKey, P256SerializedKeypair & output);
extern CHIP_ERROR ECDSA_validate_msg_signature_H(const P256PublicKey * public_key, const uint8_t * msg, const size_t msg_length,
                                                 const P256ECDSASignature & signature);
extern CHIP_ERROR ECDSA_validate_hash_signature_H(const P256PublicKey * public_key, const uint8_t * hash, const size_t hash_length,
                                                  const P256ECDSASignature & signature);

static CHIP_ERROR get_trustm_keyid_from_keypair(const P256KeypairContext mKeypair, uint32_t * key_id)
{
    if (0 != memcmp(&mKeypair.mBytes[0], trustm_magic_no, sizeof(trustm_magic_no)))
    {
        return CHIP_ERROR_INTERNAL;
    }

    *key_id = (mKeypair.mBytes[CRYPTO_KEYPAIR_KEYID_OFFSET]) | (mKeypair.mBytes[CRYPTO_KEYPAIR_KEYID_OFFSET + 1] << 8);
    return CHIP_NO_ERROR;
}

P256Keypair::~P256Keypair()
{
    uint32_t keyid = 0;
    if (CHIP_NO_ERROR != get_trustm_keyid_from_keypair(mKeypair, &keyid))
    {
        Clear();
    }
}

CHIP_ERROR P256Keypair::Initialize(ECPKeyTarget key_target)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;

    uint8_t pubkey[128] = {
        0,
    };
    uint16_t pubKeyLen                = sizeof(pubkey);
    optiga_lib_status_t return_status = OPTIGA_LIB_BUSY;
    P256PublicKey & public_key        = const_cast<P256PublicKey &>(Pubkey());
    optiga_key_usage_t key_usage;
    uint16_t keyid;

    if (key_target == ECPKeyTarget::ECDH)
    {
        keyid = TRUSTM_ECDH_OID_KEY;

        // Trust M ECC 256 Key Gen
        ChipLogDetail(Crypto, "Generating NIST256 key for ECDH!");
        key_usage = OPTIGA_KEY_USAGE_KEY_AGREEMENT;
    }
    else
    {
        error = Initialize_H(this, &mPublicKey, &mKeypair);
        if (CHIP_NO_ERROR == error)
        {
            mInitialized = true;
        }
        return error;
    }
    // Trust M init
    return_status = trustm_Open();
    VerifyOrExit(return_status == OPTIGA_LIB_SUCCESS, error = CHIP_ERROR_INTERNAL);
    return_status = trustm_ecc_keygen(keyid, key_usage, OPTIGA_ECC_CURVE_NIST_P_256, pubkey, &pubKeyLen);

    // Add signature length
    VerifyOrExit(return_status == OPTIGA_LIB_SUCCESS, error = CHIP_ERROR_INTERNAL);

    /* Set the public key */
    VerifyOrExit((size_t) pubKeyLen > NIST256_HEADER_OFFSET, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(((size_t) pubKeyLen - NIST256_HEADER_OFFSET) <= kP256_PublicKey_Length, error = CHIP_ERROR_INTERNAL);
    memcpy((void *) Uint8::to_const_uchar(public_key), pubkey + NIST256_HEADER_OFFSET, pubKeyLen - NIST256_HEADER_OFFSET);

    memcpy(&mKeypair.mBytes[0], trustm_magic_no, sizeof(trustm_magic_no));
    mKeypair.mBytes[CRYPTO_KEYPAIR_KEYID_OFFSET]     = (keyid >> (0 * 8)) & 0xFF;
    mKeypair.mBytes[CRYPTO_KEYPAIR_KEYID_OFFSET + 1] = (keyid >> (1 * 8)) & 0xFF;

    mInitialized = true;
    error        = CHIP_NO_ERROR;

exit:
    if (error != CHIP_NO_ERROR)
    {
        trustm_close();
    }
    return error;
}

CHIP_ERROR P256Keypair::ECDSA_sign_msg(const uint8_t * msg, size_t msg_length, P256ECDSASignature & out_signature) const
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_UNINITIALIZED);
    uint16_t keyid = (mKeypair.mBytes[CRYPTO_KEYPAIR_KEYID_OFFSET]) | (mKeypair.mBytes[CRYPTO_KEYPAIR_KEYID_OFFSET + 1] << 8);

    CHIP_ERROR error                  = CHIP_ERROR_INTERNAL;
    optiga_lib_status_t return_status = OPTIGA_LIB_BUSY;

    uint8_t signature_trustm[kMax_ECDSA_Signature_Length_Der] = { 0 };
    uint16_t signature_trustm_len                             = (uint16_t) kMax_ECDSA_Signature_Length_Der;
    uint8_t digest[32];
    uint8_t digest_length = sizeof(digest);

    memset(&digest[0], 0, sizeof(digest));
    MutableByteSpan out_raw_sig_span(out_signature.Bytes(), out_signature.Capacity());

    VerifyOrReturnError(msg != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(msg_length > 0, CHIP_ERROR_INVALID_ARGUMENT);
    // Trust M Init
    return_status = trustm_Open();
    VerifyOrExit(return_status == OPTIGA_LIB_SUCCESS, error = CHIP_ERROR_INTERNAL);
    // Hash to get the digest
    error = Hash_SHA256(msg, msg_length, &digest[0]);
    SuccessOrExit(error);

    if (keyid == OPTIGA_KEY_ID_E0F0)
    {
        ChipLogDetail(Crypto, "TrustM: ECDSA_sign_msg");

        // Api call to calculate the signature
        return_status = trustm_ecdsa_sign(OPTIGA_KEY_ID_E0F0, digest, digest_length, signature_trustm, &signature_trustm_len);
    }
    else
    {
        // Use the mbedtls based method
        ChipLogDetail(Crypto, "ECDSA sign msg mbedtls");
        return ECDSA_sign_msg_H(&mKeypair, msg, msg_length, out_signature);
    }

    VerifyOrExit(return_status == OPTIGA_LIB_SUCCESS, error = CHIP_ERROR_INTERNAL);

    error = EcdsaAsn1SignatureToRaw(kP256_FE_Length, ByteSpan{ signature_trustm, signature_trustm_len }, out_raw_sig_span);

    SuccessOrExit(error);

    SuccessOrExit(error = out_signature.SetLength(2 * kP256_FE_Length));

    error = CHIP_NO_ERROR;

exit:
    if (error != CHIP_NO_ERROR)
    {
        trustm_close();
    }
    return error;
}

CHIP_ERROR P256Keypair::ECDH_derive_secret(const P256PublicKey & remote_public_key, P256ECDHDerivedSecret & out_secret) const
{
    CHIP_ERROR error                  = CHIP_ERROR_INTERNAL;
    optiga_lib_status_t return_status = OPTIGA_LIB_BUSY;
    size_t secret_length              = (out_secret.Length() == 0) ? out_secret.Capacity() : out_secret.Length();
    uint32_t keyid                    = 0;
    if (CHIP_NO_ERROR != get_trustm_keyid_from_keypair(mKeypair, &keyid))
    {
        ChipLogDetail(Crypto, "ECDH_derive_secret : Host");
        return ECDH_derive_secret_H(&mKeypair, remote_public_key, out_secret);
    }

    ChipLogDetail(Crypto, "TrustM: ECDH_derive_secret");

    const uint8_t * const rem_pubKey = Uint8::to_const_uchar(remote_public_key);
    const size_t rem_pubKeyLen       = remote_public_key.Length();
    uint8_t remote_key[68];
    uint8_t header[3] = { 0x03, 0x42, 0x00 };

    return_status = trustm_Open();
    VerifyOrExit(return_status == OPTIGA_LIB_SUCCESS, error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(rem_pubKeyLen <= (sizeof(remote_key) - sizeof(header)), error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(secret_length <= UINT8_MAX, error = CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrExit(secret_length <= out_secret.Capacity(), error = CHIP_ERROR_BUFFER_TOO_SMALL);

    memcpy(remote_key, &header, sizeof(header));
    memcpy(remote_key + sizeof(header), rem_pubKey, rem_pubKeyLen);

    return_status = trustm_ecdh_derive_secret(static_cast<optiga_key_id_t>(TRUSTM_ECDH_OID_KEY), (uint8_t *) remote_key,
                                              static_cast<uint16_t>(rem_pubKeyLen + sizeof(header)), out_secret.Bytes(),
                                              static_cast<uint8_t>(secret_length));
    VerifyOrExit(return_status == OPTIGA_LIB_SUCCESS, error = CHIP_ERROR_INTERNAL);
    SuccessOrExit(error = out_secret.SetLength(secret_length));

    error = CHIP_NO_ERROR;

exit:
    if (error != CHIP_NO_ERROR)
    {
        trustm_close();
    }
    return error;
}

CHIP_ERROR P256PublicKey::ECDSA_validate_hash_signature(const uint8_t * hash, size_t hash_length,
                                                        const P256ECDSASignature & signature) const
{
#if !ENABLE_TRUSTM_ECDSA_VERIFY
    return ECDSA_validate_hash_signature_H(this, hash, hash_length, signature);
#else
    CHIP_ERROR error                                          = CHIP_ERROR_INTERNAL;
    optiga_lib_status_t return_status                         = OPTIGA_LIB_BUSY;
    uint8_t signature_trustm[kMax_ECDSA_Signature_Length_Der] = { 0 };
    size_t signature_trustm_len                               = sizeof(signature_trustm);
    MutableByteSpan out_der_sig_span(signature_trustm, signature_trustm_len);

    VerifyOrReturnError(hash != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(hash_length > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(hash_length <= UINT8_MAX, CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t hash_length_u8 = static_cast<uint8_t>(hash_length);

    ChipLogDetail(Crypto, "TrustM: ECDSA_validate_hash_signature");

    // Trust M init
    return_status = trustm_Open();
    VerifyOrExit(return_status == OPTIGA_LIB_SUCCESS, error = CHIP_ERROR_INTERNAL);
    error = EcdsaRawSignatureToAsn1(kP256_FE_Length, ByteSpan{ Uint8::to_const_uchar(signature.ConstBytes()), signature.Length() },
                                    out_der_sig_span);
    SuccessOrExit(error);

    signature_trustm_len = out_der_sig_span.size();
    VerifyOrExit(signature_trustm_len <= UINT16_MAX, error = CHIP_ERROR_INTERNAL);
    // ECC verify
    return_status =
        trustm_ecdsa_verify((uint8_t *) hash, hash_length_u8, (uint8_t *) signature_trustm,
                            static_cast<uint16_t>(signature_trustm_len), (uint8_t *) bytes, (uint8_t) kP256_PublicKey_Length);

    VerifyOrExit(return_status == OPTIGA_LIB_SUCCESS, error = CHIP_ERROR_INTERNAL);
    error = CHIP_NO_ERROR;
exit:
    if (error != CHIP_NO_ERROR)
    {
        trustm_close();
    }
    return error;
#endif
}

CHIP_ERROR P256Keypair::Serialize(P256SerializedKeypair & output) const
{
    const size_t len = output.Length() == 0 ? output.Capacity() : output.Length();
    Encoding::BufferWriter bbuf(output.Bytes(), len);
    uint8_t privkey[kP256_PrivateKey_Length] = {
        0,
    };

    if (0 != memcmp(&mKeypair.mBytes[0], trustm_magic_no, sizeof(trustm_magic_no)))
    {
        VerifyOrReturnError(mInitialized, CHIP_ERROR_UNINITIALIZED);
        return Serialize_H(mKeypair, mPublicKey, output);
    }

    /* Set the public key */
    P256PublicKey & public_key = const_cast<P256PublicKey &>(Pubkey());
    bbuf.Put(Uint8::to_uchar(public_key), public_key.Length());

    VerifyOrReturnError(bbuf.Available() == sizeof(privkey), CHIP_ERROR_INTERNAL);

    /* Set the private key trustm_magic_no */
    bbuf.Put(mKeypair.mBytes, kP256_PrivateKey_Length);
    VerifyOrReturnError(bbuf.Fit(), CHIP_ERROR_BUFFER_TOO_SMALL);

    ReturnErrorOnFailure(output.SetLength(bbuf.Needed()));

    return CHIP_NO_ERROR;
}

CHIP_ERROR P256Keypair::Deserialize(P256SerializedKeypair & input)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    const uint8_t * privkey;
    /* Set the public key */
    P256PublicKey & public_key = const_cast<P256PublicKey &>(Pubkey());
    Encoding::BufferWriter bbuf((uint8_t *) Uint8::to_const_uchar(public_key), public_key.Length());

    VerifyOrReturnError(input.Length() == public_key.Length() + kP256_PrivateKey_Length, CHIP_ERROR_INVALID_ARGUMENT);

    privkey = input.ConstBytes() + public_key.Length();

    if (0 == memcmp(privkey, trustm_magic_no, sizeof(trustm_magic_no)))
    {
        /* trustm_magic_no + KeyID is passed */
        ChipLogDetail(Crypto, "Deserialize: key found");
        bbuf.Put(input.Bytes(), public_key.Length());
        VerifyOrReturnError(bbuf.Fit(), CHIP_ERROR_NO_MEMORY);

        memcpy(&mKeypair.mBytes[0], trustm_magic_no, sizeof(trustm_magic_no));

        mKeypair.mBytes[4] = *(privkey + 4);
        mKeypair.mBytes[5] = *(privkey + 5);

        mInitialized = true;

        return CHIP_NO_ERROR;
    }
    else
    {
        if (CHIP_NO_ERROR == (error = Deserialize_H(this, &mPublicKey, &mKeypair, input)))
        {
            mInitialized = true;
        }
        return error;
    }
}

CHIP_ERROR P256PublicKey::ECDSA_validate_msg_signature(const uint8_t * msg, size_t msg_length,
                                                       const P256ECDSASignature & signature) const
{
#if !ENABLE_TRUSTM_ECDSA_VERIFY
    return ECDSA_validate_msg_signature_H(this, msg, msg_length, signature);
#else
    CHIP_ERROR error                                          = CHIP_ERROR_INTERNAL;
    uint8_t signature_trustm[kMax_ECDSA_Signature_Length_Der] = { 0 };
    size_t signature_trustm_len                               = sizeof(signature_trustm);
    uint8_t digest[32];
    uint8_t digest_length = sizeof(digest);
    MutableByteSpan out_der_sig_span(signature_trustm, signature_trustm_len);
    optiga_lib_status_t return_status = OPTIGA_LIB_BUSY;

    VerifyOrReturnError(msg != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(msg_length > 0, CHIP_ERROR_INVALID_ARGUMENT);

    ChipLogDetail(Crypto, "TrustM: ECDSA_validate_msg_signature");

    // Trust M init
    return_status = trustm_Open();
    VerifyOrExit(return_status == OPTIGA_LIB_SUCCESS, error = CHIP_ERROR_INTERNAL);

    error = EcdsaRawSignatureToAsn1(kP256_FE_Length, ByteSpan{ Uint8::to_const_uchar(signature.ConstBytes()), signature.Length() },
                                    out_der_sig_span);
    SuccessOrExit(error);

    signature_trustm_len = out_der_sig_span.size();
    VerifyOrExit(signature_trustm_len <= UINT16_MAX, error = CHIP_ERROR_INTERNAL);
    // Hash to get the digest
    memset(&digest[0], 0, sizeof(digest));
    error = Hash_SHA256(msg, msg_length, &digest[0]);
    SuccessOrExit(error);
    // ECC verify
    return_status =
        trustm_ecdsa_verify(digest, digest_length, (uint8_t *) signature_trustm, static_cast<uint16_t>(signature_trustm_len),
                            (uint8_t *) bytes, (uint8_t) kP256_PublicKey_Length);

    VerifyOrExit(return_status == OPTIGA_LIB_SUCCESS, error = CHIP_ERROR_INTERNAL);
    error = CHIP_NO_ERROR;
exit:
    if (error != CHIP_NO_ERROR)
    {
        trustm_close();
    }
    return error;
#endif
}

CHIP_ERROR P256Keypair::NewCertificateSigningRequest(uint8_t * csr, size_t & csr_length) const
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_UNINITIALIZED);

    ChipLogDetail(Crypto, "NewCertificateSigningRequest : Host");
    return NewCertificateSigningRequest_H(&mKeypair, csr, csr_length);
}

} // namespace Crypto
} // namespace chip
