/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      HSM based implementation of CHIP crypto primitives
 *      Based on configurations in CHIPCryptoPALHsm_config.h file,
 *      chip crypto apis use either HSM or rollback to software implementation.
 */

#include "CHIPCryptoPALHsm_se05x_utils.h"
#include <lib/core/CHIPEncoding.h>

#define MAX_SHA_ONE_SHOT_DATA_LEN 900
#define NIST256_HEADER_OFFSET 26
#define CRYPTO_KEYPAIR_KEYID_OFFSET 8

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

#if (ENABLE_SE05X_GENERATE_EC_KEY || ENABLE_SE05X_ECDSA_VERIFY)
static CHIP_ERROR parse_se05x_keyid_from_keypair(const P256KeypairContext mKeypair, uint32_t * key_id)
{
    if (0 != memcmp(&mKeypair.mBytes[0], se05x_magic_no, sizeof(se05x_magic_no)))
    {
        return CHIP_ERROR_INTERNAL;
    }

    *key_id += (mKeypair.mBytes[CRYPTO_KEYPAIR_KEYID_OFFSET] << (8 * 3) & 0xFF000000) |
        (mKeypair.mBytes[CRYPTO_KEYPAIR_KEYID_OFFSET + 1] << (8 * 2) & 0x00FF0000) |
        (mKeypair.mBytes[CRYPTO_KEYPAIR_KEYID_OFFSET + 2] << (8 * 1) & 0x0000FF00) |
        (mKeypair.mBytes[CRYPTO_KEYPAIR_KEYID_OFFSET + 3] << (8 * 0) & 0x000000FF);

    return CHIP_NO_ERROR;
}
#endif // #if (ENABLE_SE05X_GENERATE_EC_KEY || ENABLE_SE05X_ECDSA_VERIFY)

P256Keypair::~P256Keypair()
{
    uint32_t keyid = 0;
    if (CHIP_NO_ERROR != parse_se05x_keyid_from_keypair(mKeypair, &keyid))
    {
        Clear();
    }
    else
    {
        // Delete the key in SE
    }
}

CHIP_ERROR P256Keypair::Initialize(ECPKeyTarget key_target)
{
#if !ENABLE_SE05X_GENERATE_EC_KEY
    if (CHIP_NO_ERROR == Initialize_H(this, &mPublicKey, &mKeypair))
    {
        mInitialized = true;
    }
    return error;
#else
    sss_status_t status    = kStatus_SSS_Fail;
    sss_object_t keyObject = { 0 };
    uint8_t pubkey[128]    = {
        0,
    };
    size_t pubKeyLen   = sizeof(pubkey);
    size_t pbKeyBitLen = sizeof(pubkey) * 8;
    uint32_t keyid     = 0;
    uint32_t options   = kKeyObject_Mode_Transient;

    ChipLogDetail(Crypto, "se05x::Generate nist256 key using se05x");

    if (key_target == ECPKeyTarget::ECDH)
    {
        keyid = kKeyId_case_ephemeral_keyid;
    }
    else
    {
        // Add the logic to use different keyid
        keyid   = kKeyId_node_op_keyid_start;
        options = kKeyObject_Mode_Persistent;
    }

    VerifyOrReturnError(se05x_sessionOpen() == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);

    status = sss_key_object_init(&keyObject, &gex_sss_chip_ctx.ks);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    status = sss_key_object_allocate_handle(&keyObject, keyid, kSSS_KeyPart_Pair, kSSS_CipherType_EC_NIST_P, 256, options);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    status = sss_key_store_generate_key(&gex_sss_chip_ctx.ks, &keyObject, 256, 0);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    status = sss_key_store_get_key(&gex_sss_chip_ctx.ks, &keyObject, pubkey, &pubKeyLen, &pbKeyBitLen);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    /* Set the public key */
    P256PublicKey & public_key = const_cast<P256PublicKey &>(Pubkey());
    VerifyOrReturnError(pubKeyLen > NIST256_HEADER_OFFSET, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError((pubKeyLen - NIST256_HEADER_OFFSET) <= kP256_PublicKey_Length, CHIP_ERROR_INTERNAL);
    memcpy((void *) Uint8::to_const_uchar(public_key), pubkey + NIST256_HEADER_OFFSET, pubKeyLen - NIST256_HEADER_OFFSET);

    memcpy(&mKeypair.mBytes[0], se05x_magic_no, sizeof(se05x_magic_no));
    mKeypair.mBytes[CRYPTO_KEYPAIR_KEYID_OFFSET]     = (keyid >> (3 * 8)) & 0x000000FF;
    mKeypair.mBytes[CRYPTO_KEYPAIR_KEYID_OFFSET + 1] = (keyid >> (2 * 8)) & 0x000000FF;
    mKeypair.mBytes[CRYPTO_KEYPAIR_KEYID_OFFSET + 2] = (keyid >> (1 * 8)) & 0x000000FF;
    mKeypair.mBytes[CRYPTO_KEYPAIR_KEYID_OFFSET + 3] = (keyid >> (0 * 8)) & 0x000000FF;

    mInitialized = true;

    return CHIP_NO_ERROR;
#endif // ENABLE_SE05X_GENERATE_EC_KEY
}

CHIP_ERROR P256Keypair::ECDSA_sign_msg(const uint8_t * msg, size_t msg_length, P256ECDSASignature & out_signature) const
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_UNINITIALIZED);

#if !ENABLE_SE05X_GENERATE_EC_KEY
    return ECDSA_sign_msg_H(&mKeypair, msg, msg_length, out_signature);
#else
    CHIP_ERROR error                  = CHIP_ERROR_INTERNAL;
    uint32_t keyid                    = 0;
    sss_asymmetric_t asymm_ctx        = { 0 };
    uint8_t hash[kSHA256_Hash_Length] = {
        0,
    };
    size_t hashLen                                           = sizeof(hash);
    sss_status_t status                                      = kStatus_SSS_Success;
    sss_object_t keyObject                                   = { 0 };
    uint8_t signature_se05x[kMax_ECDSA_Signature_Length_Der] = { 0 };
    size_t signature_se05x_len                               = sizeof(signature_se05x);
    MutableByteSpan out_raw_sig_span(out_signature.Bytes(), out_signature.Capacity());

    if (CHIP_NO_ERROR != parse_se05x_keyid_from_keypair(mKeypair, &keyid))
    {
        ChipLogDetail(Crypto, "ECDSA_sign_msg : Not ref key. Using host for ecdsa sign");
        return ECDSA_sign_msg_H(&mKeypair, msg, msg_length, out_signature);
    }

    VerifyOrReturnError(msg != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(msg_length > 0, CHIP_ERROR_INVALID_ARGUMENT);

    ChipLogDetail(Crypto, "ECDSA_sign_msg: Using se05x for ecdsa sign!");

    error = Hash_SHA256(msg, msg_length, hash);
    SuccessOrExit(error);

    VerifyOrReturnError(se05x_sessionOpen() == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(gex_sss_chip_ctx.ks.session != NULL, CHIP_ERROR_INTERNAL);

    status = sss_key_object_init(&keyObject, &gex_sss_chip_ctx.ks);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_key_object_get_handle(&keyObject, keyid);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_asymmetric_context_init(&asymm_ctx, &gex_sss_chip_ctx.session, &keyObject, kAlgorithm_SSS_SHA256, kMode_SSS_Sign);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_asymmetric_sign_digest(&asymm_ctx, hash, hashLen, signature_se05x, &signature_se05x_len);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    error = EcdsaAsn1SignatureToRaw(kP256_FE_Length, ByteSpan{ signature_se05x, signature_se05x_len }, out_raw_sig_span);
    SuccessOrExit(error);

    out_signature.SetLength(2 * kP256_FE_Length);

    error = CHIP_NO_ERROR;
exit:
    if (asymm_ctx.session != nullptr)
    {
        sss_asymmetric_context_free(&asymm_ctx);
    }
    return error;
#endif // ENABLE_SE05X_GENERATE_EC_KEY
}

CHIP_ERROR P256Keypair::Serialize(P256SerializedKeypair & output) const
{
    const size_t len = output.Length() == 0 ? output.Capacity() : output.Length();
    Encoding::BufferWriter bbuf(output.Bytes(), len);

    if (0 != memcmp(&mKeypair.mBytes[0], se05x_magic_no, sizeof(se05x_magic_no)))
    {
        VerifyOrReturnError(mInitialized, CHIP_ERROR_UNINITIALIZED);
        return Serialize_H(mKeypair, mPublicKey, output);
    }

    /* Set the public key */
    P256PublicKey & public_key = const_cast<P256PublicKey &>(Pubkey());
    bbuf.Put(Uint8::to_uchar(public_key), public_key.Length());

    /* Set the private key se05x_magic_no */
    bbuf.Put(mKeypair.mBytes, kP256_PrivateKey_Length);

    VerifyOrReturnError(bbuf.Fit(), CHIP_ERROR_BUFFER_TOO_SMALL);

    output.SetLength(bbuf.Needed());

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

    privkey = input.ConstBytes() /*Uint8::to_const_uchar(input) */ + public_key.Length();

    if (0 == memcmp(privkey, se05x_magic_no, sizeof(se05x_magic_no)))
    {
        /* se05x_magic_no + KeyID is passed */
        ChipLogDetail(Crypto, "Deserialize: ref key found");
        bbuf.Put(input.Bytes(), public_key.Length());
        VerifyOrReturnError(bbuf.Fit(), CHIP_ERROR_NO_MEMORY);

        memcpy(&mKeypair.mBytes[0], se05x_magic_no, sizeof(se05x_magic_no));
        mKeypair.mBytes[8]  = *(privkey + 8);
        mKeypair.mBytes[9]  = *(privkey + 9);
        mKeypair.mBytes[10] = *(privkey + 10);
        mKeypair.mBytes[11] = *(privkey + 11);
        // ChipLogDetail(Crypto, "Parsed keyId = 0x%02X%02X%02X%02X", mKeypair.mBytes[8], mKeypair.mBytes[9],
        // mKeypair.mBytes[10],mKeypair.mBytes[11]);

        mInitialized = true;

        return CHIP_NO_ERROR;
    }
    else
    {
#if !ENABLE_SE05X_KEY_IMPORT
        if (CHIP_NO_ERROR == (error = Deserialize_H(this, &mPublicKey, &mKeypair, input)))
        {
            mInitialized = true;
        }
        return error;
#else

        sss_object_t sss_object = { 0 };
        sss_status_t sss_status = kStatus_SSS_Fail;
        uint32_t keyid          = 0;
        uint8_t keyid_buffer[4] = { 0 };
        uint8_t key[128]        = { 0 };
        uint8_t header[]        = EC_NIST_P256_KP_HEADER;
        uint8_t pub_header[]    = EC_NIST_P256_KP_PUB_HEADER;
        size_t key_length       = 0;

        memcpy(&key[key_length], header, sizeof(header));
        key_length += sizeof(header);
        if ((privkey[0] & 0x80))
        {
            key[key_length++] = 0x00;
        }
        memcpy(&key[key_length], privkey, kP256_PrivateKey_Length);
        key_length += kP256_PrivateKey_Length;
        memcpy(&key[key_length], pub_header, sizeof(pub_header));
        key_length += sizeof(pub_header);
        memcpy(&key[key_length], input.ConstBytes(), public_key.Length());
        key_length += public_key.Length();

        error = DRBG_get_bytes(keyid_buffer, sizeof(keyid_buffer));
        VerifyOrReturnError(error == CHIP_NO_ERROR, error);

        keyid = (keyid_buffer[3] << (8 * 3)) + (keyid_buffer[2] << (8 * 2)) + (keyid_buffer[1] << (8 * 1)) +
            (keyid_buffer[0] << (8 * 0));

        sss_status = sss_key_object_init(&sss_object, &gex_sss_chip_ctx.ks);
        VerifyOrReturnError(sss_status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);
        sss_status = sss_key_object_allocate_handle(&sss_object, keyid, kSSS_KeyPart_Pair, kSSS_CipherType_EC_NIST_P, 256,
                                                    kKeyObject_Mode_Persistent);
        VerifyOrReturnError(sss_status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);
        sss_status = sss_key_store_set_key(&gex_sss_chip_ctx.ks, &sss_object, key, key_length, 256, NULL, 0);
        VerifyOrReturnError(sss_status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

        return CHIP_NO_ERROR;
#endif
    }
}

CHIP_ERROR P256Keypair::ECDH_derive_secret(const P256PublicKey & remote_public_key, P256ECDHDerivedSecret & out_secret) const
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_UNINITIALIZED);

#if !ENABLE_SE05X_GENERATE_EC_KEY
    return ECDH_derive_secret_H(&mKeypair, remote_public_key, out_secret);
#else
    size_t secret_length = (out_secret.Length() == 0) ? out_secret.Capacity() : out_secret.Length();
    uint32_t keyid       = 0;

    if (CHIP_NO_ERROR != parse_se05x_keyid_from_keypair(mKeypair, &keyid))
    {
        ChipLogDetail(Crypto, "ECDH_derive_secret : Not ref key. Using host for ecdh");
        return ECDH_derive_secret_H(&mKeypair, remote_public_key, out_secret);
    }

    ChipLogDetail(Crypto, "ECDH_derive_secret : Using se05x for ecdh");

    VerifyOrReturnError(se05x_sessionOpen() == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(gex_sss_chip_ctx.ks.session != NULL, CHIP_ERROR_INTERNAL);

    const uint8_t * const rem_pubKey = Uint8::to_const_uchar(remote_public_key);
    const size_t rem_pubKeyLen       = remote_public_key.Length();

    VerifyOrReturnError(gex_sss_chip_ctx.ks.session != nullptr, CHIP_ERROR_INTERNAL);

    const smStatus_t smstatus =
        Se05x_API_ECGenSharedSecret(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx, keyid, rem_pubKey, rem_pubKeyLen,
                                    out_secret.Bytes() /*Uint8::to_uchar(out_secret)*/, &secret_length);
    VerifyOrReturnError(smstatus == SM_OK, CHIP_ERROR_INTERNAL);

    return out_secret.SetLength(secret_length);

#endif // ENABLE_SE05X_GENERATE_EC_KEY
}

/* EC Public key HSM implementation */

CHIP_ERROR SE05X_Set_ECDSA_Public_Key(sss_object_t * keyObject, const uint8_t * key, size_t keylen)
{
    uint8_t public_key[128] = {
        0,
    };
    size_t public_key_len = 0;

    /* ECC NIST-256 Public Key header */
    const uint8_t nist256_header[] = { 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01,
                                       0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00 };

    VerifyOrReturnError(se05x_sessionOpen() == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(gex_sss_chip_ctx.ks.session != NULL, CHIP_ERROR_INTERNAL);

    /* Set public key */
    sss_status_t status = sss_key_object_init(keyObject, &gex_sss_chip_ctx.ks);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    status = sss_key_object_allocate_handle(keyObject, kKeyId_sha256_ecc_pub_keyid, kSSS_KeyPart_Public, kSSS_CipherType_EC_NIST_P,
                                            256, kKeyObject_Mode_Transient);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    VerifyOrReturnError((sizeof(nist256_header) + keylen) <= sizeof(public_key), CHIP_ERROR_INTERNAL);

    memcpy(public_key, nist256_header, sizeof(nist256_header));
    public_key_len = sizeof(nist256_header);
    memcpy(public_key + public_key_len, key, keylen);
    public_key_len = public_key_len + keylen;

    status = sss_key_store_set_key(&gex_sss_chip_ctx.ks, keyObject, public_key, public_key_len, 256, NULL, 0);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR P256PublicKey::ECDSA_validate_msg_signature(const uint8_t * msg, size_t msg_length,
                                                       const P256ECDSASignature & signature) const
{
#if !ENABLE_SE05X_ECDSA_VERIFY
    return ECDSA_validate_msg_signature_H(this, msg, msg_length, signature);
#else
    CHIP_ERROR error           = CHIP_ERROR_INTERNAL;
    sss_status_t status        = kStatus_SSS_Success;
    sss_asymmetric_t asymm_ctx = { 0 };
    uint8_t hash[32]           = {
        0,
    };
    size_t hash_length                                       = sizeof(hash);
    sss_object_t keyObject                                   = { 0 };
    uint8_t signature_se05x[kMax_ECDSA_Signature_Length_Der] = { 0 };
    size_t signature_se05x_len                               = sizeof(signature_se05x);
    MutableByteSpan out_der_sig_span(signature_se05x, signature_se05x_len);

    VerifyOrReturnError(msg != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(msg_length > 0, CHIP_ERROR_INVALID_ARGUMENT);

    ChipLogDetail(Crypto, "ECDSA_validate_msg_signature: Using se05x for ECDSA verify (msg) !");

    VerifyOrReturnError(se05x_sessionOpen() == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(gex_sss_chip_ctx.ks.session != NULL, CHIP_ERROR_INTERNAL);

    error = Hash_SHA256(msg, msg_length, hash);
    SuccessOrExit(error);

    error = SE05X_Set_ECDSA_Public_Key(&keyObject, bytes, kP256_PublicKey_Length);
    SuccessOrExit(error);

    /* ECC Verify */
    status =
        sss_asymmetric_context_init(&asymm_ctx, &gex_sss_chip_ctx.session, &keyObject, kAlgorithm_SSS_SHA256, kMode_SSS_Verify);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    error = EcdsaRawSignatureToAsn1(kP256_FE_Length, ByteSpan{ Uint8::to_const_uchar(signature.ConstBytes()), signature.Length() },
                                    out_der_sig_span);
    SuccessOrExit(error);

    signature_se05x_len = out_der_sig_span.size();

    status = sss_asymmetric_verify_digest(&asymm_ctx, hash, hash_length, (uint8_t *) signature_se05x, signature_se05x_len);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INVALID_SIGNATURE);

    error = CHIP_NO_ERROR;
exit:
    if (asymm_ctx.session != NULL)
    {
        sss_asymmetric_context_free(&asymm_ctx);
    }

    if (keyObject.keyStore->session != NULL)
    {
        sss_key_store_erase_key(&gex_sss_chip_ctx.ks, &keyObject);
    }

    return error;

#endif // ENABLE_SE05X_ECDSA_VERIFY
}

CHIP_ERROR P256PublicKey::ECDSA_validate_hash_signature(const uint8_t * hash, size_t hash_length,
                                                        const P256ECDSASignature & signature) const
{
#if !ENABLE_SE05X_ECDSA_VERIFY
    return ECDSA_validate_hash_signature_H(this, hash, hash_length, signature);
#else
    CHIP_ERROR error                                         = CHIP_ERROR_INTERNAL;
    sss_status_t status                                      = kStatus_SSS_Success;
    sss_asymmetric_t asymm_ctx                               = { 0 };
    sss_object_t keyObject                                   = { 0 };
    uint8_t signature_se05x[kMax_ECDSA_Signature_Length_Der] = { 0 };
    size_t signature_se05x_len                               = sizeof(signature_se05x);
    MutableByteSpan out_der_sig_span(signature_se05x, signature_se05x_len);

    VerifyOrReturnError(hash != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(hash_length > 0, CHIP_ERROR_INVALID_ARGUMENT);

    ChipLogDetail(Crypto, "ECDSA_validate_msg_signature: Using se05x for ECDSA verify (hash) !");

    VerifyOrReturnError(se05x_sessionOpen() == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(gex_sss_chip_ctx.ks.session != NULL, CHIP_ERROR_INTERNAL);

    error = SE05X_Set_ECDSA_Public_Key(&keyObject, bytes, kP256_PublicKey_Length);
    SuccessOrExit(error);

    /* ECC Verify */
    status =
        sss_asymmetric_context_init(&asymm_ctx, &gex_sss_chip_ctx.session, &keyObject, kAlgorithm_SSS_SHA256, kMode_SSS_Verify);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    error = EcdsaRawSignatureToAsn1(kP256_FE_Length, ByteSpan{ Uint8::to_const_uchar(signature.ConstBytes()), signature.Length() },
                                    out_der_sig_span);
    SuccessOrExit(error);

    signature_se05x_len = out_der_sig_span.size();

    status = sss_asymmetric_verify_digest(&asymm_ctx, const_cast<uint8_t *>(hash), hash_length, (uint8_t *) signature_se05x,
                                          signature_se05x_len);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INVALID_SIGNATURE);

    error = CHIP_NO_ERROR;
exit:
    if (asymm_ctx.session != NULL)
    {
        sss_asymmetric_context_free(&asymm_ctx);
    }

    if (keyObject.keyStore->session != NULL)
    {
        sss_key_store_erase_key(&gex_sss_chip_ctx.ks, &keyObject);
    }

    return error;

#endif // ENABLE_SE05X_ECDSA_VERIFY
}

void add_tlv(uint8_t * buf, size_t buf_index, uint8_t tag, size_t len, uint8_t * val)
{
    buf[buf_index++] = (uint8_t) tag;
    buf[buf_index++] = (uint8_t) len;
    if (len > 0 && val != NULL)
    {
        memcpy(&buf[buf_index], val, len);
        buf_index = buf_index + len;
    }
}

/*
 * CSR format used in the below function,
 *
 *
 *    (ASN1_CONSTRUCTED | ASN1_SEQUENCE) LENGTH
 *
 *        (ASN1_CONSTRUCTED | ASN1_SEQUENCE) LENGTH
 *
 *            VERSION ::=  INTEGER  {  v1(0), v2(1), v3(2)  }
 *
 *            (ASN1_CONSTRUCTED | ASN1_SEQUENCE) LENGTH
 *
 *                (ASN1_CONSTRUCTED | ASN1_SET) LENGTH
 *
 *                    (ASN1_CONSTRUCTED | ASN1_SEQUENCE) LENGTH
 *
 *                        (ASN1_OID) LENGTH VALUE(Organisation OID)
 *
 *                        (ASN1_UTF8_STRING) LENGTH VALUE(Subject Str == "CSR")
 *
 *            PUBLIC KEY {WITH HEADER. 91 Bytes}
 *
 *        (ASN1_CONSTRUCTED | ASN1_SEQUENCE) LENGTH
 *
 *            (ASN1_OID) LENGTH VALUE(ECDSA SHA256 OID)
 *
 *            (ASN1_NULL) 0x00
 *
 *        (ASN1_BIT_STRING) LENGTH VALUE(SIGNATURE)
 *
 */

CHIP_ERROR P256Keypair::NewCertificateSigningRequest(uint8_t * csr, size_t & csr_length) const
{
#if !ENABLE_SE05X_GENERATE_EC_KEY
    return NewCertificateSigningRequest_H(&mKeypair, csr, csr_length);
#else
    CHIP_ERROR error           = CHIP_ERROR_INTERNAL;
    sss_status_t status        = kStatus_SSS_Success;
    sss_asymmetric_t asymm_ctx = { 0 };
    sss_object_t keyObject     = { 0 };
    uint32_t keyid             = 0;

    uint8_t data_to_hash[128] = { 0 };
    size_t data_to_hash_len   = sizeof(data_to_hash);
    uint8_t pubkey[128]       = { 0 };
    size_t pubKeyLen          = 0;
    uint8_t hash[32]          = { 0 };
    size_t hash_length        = sizeof(hash);
    uint8_t signature[128]    = { 0 };
    size_t signature_len      = sizeof(signature);

    size_t csr_index    = 0;
    size_t buffer_index = data_to_hash_len;

    uint8_t organisation_oid[3] = { 0x55, 0x04, 0x0a };

    // Version  ::=  INTEGER  {  v1(0), v2(1), v3(2)  }
    uint8_t version[3]       = { 0x02, 0x01, 0x00 };
    uint8_t signature_oid[8] = { 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02 };
    uint8_t nist256_header[] = { 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01,
                                 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00 };

    VerifyOrReturnError(mInitialized, CHIP_ERROR_UNINITIALIZED);

    if (CHIP_NO_ERROR != parse_se05x_keyid_from_keypair(mKeypair, &keyid))
    {
        ChipLogDetail(Crypto, "NewCertificateSigningRequest : Not ref key. Using host for CSR");
        return NewCertificateSigningRequest_H(&mKeypair, csr, csr_length);
    }

    ChipLogDetail(Crypto, "NewCertificateSigningRequest : Using se05x for CSR");

    // No extensions are copied
    buffer_index -= kTlvHeader;
    add_tlv(data_to_hash, buffer_index, (ASN1_CONSTRUCTED | ASN1_CONTEXT_SPECIFIC), 0, NULL);

    // Copy public key (with header)
    {
        P256PublicKey & public_key = const_cast<P256PublicKey &>(Pubkey());

        VerifyOrExit((sizeof(nist256_header) + public_key.Length()) <= sizeof(pubkey), error = CHIP_ERROR_INTERNAL);

        memcpy(pubkey, nist256_header, sizeof(nist256_header));
        pubKeyLen = pubKeyLen + sizeof(nist256_header);

        memcpy((pubkey + pubKeyLen), Uint8::to_uchar(public_key), public_key.Length());
        pubKeyLen = pubKeyLen + public_key.Length();
    }

    buffer_index -= pubKeyLen;
    VerifyOrExit(buffer_index > 0, error = CHIP_ERROR_INTERNAL);
    memcpy((void *) &data_to_hash[buffer_index], pubkey, pubKeyLen);

    // Copy subject (in the current implementation only organisation name info is added) and organisation OID
    buffer_index -= (kTlvHeader + sizeof(SUBJECT_STR) - 1);
    VerifyOrExit(buffer_index > 0, error = CHIP_ERROR_INTERNAL);
    add_tlv(data_to_hash, buffer_index, ASN1_UTF8_STRING, sizeof(SUBJECT_STR) - 1, (uint8_t *) SUBJECT_STR);

    buffer_index -= (kTlvHeader + sizeof(organisation_oid));
    VerifyOrExit(buffer_index > 0, error = CHIP_ERROR_INTERNAL);
    add_tlv(data_to_hash, buffer_index, ASN1_OID, sizeof(organisation_oid), organisation_oid);

    // Add length
    buffer_index -= kTlvHeader;
    // Subject TLV ==> 1 + 1 + len(subject)
    // Org OID TLV ==> 1 + 1 + len(organisation_oid)
    VerifyOrExit(buffer_index > 0, error = CHIP_ERROR_INTERNAL);
    add_tlv(data_to_hash, buffer_index, (ASN1_CONSTRUCTED | ASN1_SEQUENCE),
            ((2 * kTlvHeader) + (sizeof(SUBJECT_STR) - 1) + sizeof(organisation_oid)), NULL);

    buffer_index -= kTlvHeader;
    VerifyOrExit(buffer_index > 0, error = CHIP_ERROR_INTERNAL);
    add_tlv(data_to_hash, buffer_index, (ASN1_CONSTRUCTED | ASN1_SET),
            ((3 * kTlvHeader) + (sizeof(SUBJECT_STR) - 1) + sizeof(organisation_oid)), NULL);

    buffer_index -= kTlvHeader;
    VerifyOrExit(buffer_index > 0, error = CHIP_ERROR_INTERNAL);
    add_tlv(data_to_hash, buffer_index, (ASN1_CONSTRUCTED | ASN1_SEQUENCE),
            ((4 * kTlvHeader) + (sizeof(SUBJECT_STR) - 1) + sizeof(organisation_oid)), NULL);

    buffer_index -= 3;
    VerifyOrExit(buffer_index > 0, error = CHIP_ERROR_INTERNAL);
    memcpy((void *) &data_to_hash[buffer_index], version, sizeof(version));

    buffer_index -= kTlvHeader;
    VerifyOrExit(buffer_index > 0, error = CHIP_ERROR_INTERNAL);
    add_tlv(data_to_hash, buffer_index, (ASN1_CONSTRUCTED | ASN1_SEQUENCE), (data_to_hash_len - buffer_index - kTlvHeader), NULL);

    // TLV data is created by copying from backwards. move it to start of buffer.
    data_to_hash_len = (data_to_hash_len - buffer_index);
    memmove(data_to_hash, (data_to_hash + buffer_index), data_to_hash_len);

    error = Hash_SHA256(data_to_hash, data_to_hash_len, hash);
    SuccessOrExit(error);

    // Sign on hash
    status = sss_key_object_init(&keyObject, &gex_sss_chip_ctx.ks);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_key_object_get_handle(&keyObject, keyid);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_asymmetric_context_init(&asymm_ctx, &gex_sss_chip_ctx.session, &keyObject, kAlgorithm_SSS_SHA256, kMode_SSS_Sign);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_asymmetric_sign_digest(&asymm_ctx, hash, hash_length, Uint8::to_uchar(signature), &signature_len);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    VerifyOrExit((csr_index + 3) <= csr_length, error = CHIP_ERROR_INTERNAL);
    csr[csr_index++] = (ASN1_CONSTRUCTED | ASN1_SEQUENCE);
    if ((data_to_hash_len + 14 + kTlvHeader + signature_len) >= 0x80)
    {
        csr[csr_index++] = 0x81;
    }
    csr[csr_index++] = (uint8_t) (data_to_hash_len + 14 + kTlvHeader + signature_len);

    VerifyOrExit((csr_index + data_to_hash_len) <= csr_length, error = CHIP_ERROR_INTERNAL);
    memcpy((csr + csr_index), data_to_hash, data_to_hash_len);
    csr_index = csr_index + data_to_hash_len;

    // ECDSA SHA256 Signature OID TLV ==> 1 + 1 + len(signature_oid) (8)
    // ASN_NULL ==> 1 + 1
    VerifyOrExit((csr_index + kTlvHeader) <= csr_length, error = CHIP_ERROR_INTERNAL);
    add_tlv(csr, csr_index, (ASN1_CONSTRUCTED | ASN1_SEQUENCE), 0x0C, NULL);
    csr_index = csr_index + kTlvHeader;

    VerifyOrExit((csr_index + sizeof(signature_oid) + kTlvHeader) <= csr_length, error = CHIP_ERROR_INTERNAL);
    add_tlv(csr, csr_index, ASN1_OID, sizeof(signature_oid), signature_oid);
    csr_index = csr_index + kTlvHeader + sizeof(signature_oid);

    VerifyOrExit((csr_index + kTlvHeader) <= csr_length, error = CHIP_ERROR_INTERNAL);
    add_tlv(csr, csr_index, ASN1_NULL, 0x00, NULL);
    csr_index = csr_index + kTlvHeader;

    VerifyOrExit((csr_index + kTlvHeader) <= csr_length, error = CHIP_ERROR_INTERNAL);
    csr[csr_index++] = ASN1_BIT_STRING;
    csr[csr_index++] = (uint8_t) ((signature[0] != 0) ? (signature_len + 1) : (signature_len));

    if (signature[0] != 0)
    {
        VerifyOrExit(csr_index <= csr_length, error = CHIP_ERROR_INTERNAL);
        csr[csr_index++] = 0x00;
        // Increament total count by 1
        csr[2]++;
    }
    VerifyOrExit((csr_index + signature_len) <= csr_length, error = CHIP_ERROR_INTERNAL);
    memcpy(&csr[csr_index], signature, signature_len);

    csr_length = (csr_index + signature_len);

    error = CHIP_NO_ERROR;
exit:
    if (asymm_ctx.session != NULL)
    {
        sss_asymmetric_context_free(&asymm_ctx);
    }

    return error;

#endif // ENABLE_SE05X_GENERATE_EC_KEY
}

} // namespace Crypto
} // namespace chip
