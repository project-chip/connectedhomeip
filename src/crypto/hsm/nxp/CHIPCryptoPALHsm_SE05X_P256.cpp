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

#include "CHIPCryptoPALHsm_SE05X_utils.h"
#include <core/CHIPEncoding.h>

#if ENABLE_HSM_GENERATE_EC_KEY

#define MAX_SHA_ONE_SHOT_DATA_LEN 900
#define NIST256_HEADER_OFFSET 26

namespace chip {
namespace Crypto {

P256KeypairHSM::~P256KeypairHSM()
{
    if (keyid != kKeyId_NotInitialized)
    {
        if (provisioned_key == false)
        {
            ChipLogDetail(Crypto, "Deleting key with id - %x !", keyid);
            se05x_delete_key(keyid);
        }
        else
        {
            ChipLogDetail(Crypto, "Provisioned key ! Not deleting key in HSM");
        }
    }
}

CHIP_ERROR P256KeypairHSM::Initialize()
{
    sss_object_t keyObject = { 0 };
    uint8_t pubkey[128]    = {
        0,
    };
    constexpr size_t pubKeyLen   = sizeof(pubkey);
    constexpr size_t pbKeyBitLen = sizeof(pubkey) * 8;

    if (keyid == 0)
    {
        ChipLogDetail(Crypto, "Keyid not set !. Set key id using 'SetKeyId' member class !");
        return CHIP_ERROR_INTERNAL;
    }

    se05x_sessionOpen();

    sss_status_t status = sss_key_object_init(&keyObject, &gex_sss_chip_ctx.ks);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    if (provisioned_key == false)
    {

        status = sss_key_object_allocate_handle(&keyObject, keyid, kSSS_KeyPart_Pair, kSSS_CipherType_EC_NIST_P, 256,
                                                kKeyObject_Mode_Transient);
        VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

        ChipLogDetail(Crypto, "Creating Nist256 key on SE05X !");

        status = sss_key_store_generate_key(&gex_sss_chip_ctx.ks, &keyObject, 256, 0);
        VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);
    }
    else
    {

        // if the key is provisioned already, only get the public key,
        // and set it in public key member of this class.
        ChipLogDetail(Crypto, "Provisioned key ! Not creating key in HSM");

        status = sss_key_object_get_handle(&keyObject, keyid);
        VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);
    }

    status = sss_key_store_get_key(&gex_sss_chip_ctx.ks, &keyObject, pubkey, &pubKeyLen, &pbKeyBitLen);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    {
        /* Set the public key */
        P256PublicKeyHSM & public_key = const_cast<P256PublicKeyHSM &>(Pubkey());
        VerifyOrReturnError(pubKeyLen > NIST256_HEADER_OFFSET, CHIP_ERROR_INTERNAL);
        VerifyOrReturnError((pubKeyLen - NIST256_HEADER_OFFSET) <= kP256_PublicKey_Length, CHIP_ERROR_INTERNAL);
        memcpy((void *) Uint8::to_const_uchar(public_key), pubkey + NIST256_HEADER_OFFSET, pubKeyLen - NIST256_HEADER_OFFSET);
        public_key.SetPublicKeyId(keyid);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR P256KeypairHSM::ECDSA_sign_msg(const uint8_t * msg, size_t msg_length, P256ECDSASignature & out_signature)
{
    CHIP_ERROR error                  = CHIP_ERROR_INTERNAL;
    sss_digest_t digest_ctx           = { 0 };
    sss_asymmetric_t asymm_ctx        = { 0 };
    uint8_t hash[kSHA256_Hash_Length] = {
        0,
    };
    size_t hashLen         = sizeof(hash);
    sss_status_t status    = kStatus_SSS_Success;
    sss_object_t keyObject = { 0 };
    size_t siglen          = out_signature.Capacity();

    VerifyOrExit(msg != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(msg_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(out_signature != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(keyid != kKeyId_NotInitialized, error = CHIP_ERROR_HSM);

    ChipLogDetail(Crypto, "ECDSA_sign_msg: Using SE05X for Ecc Sign!");

    se05x_sessionOpen();

    status = sss_digest_context_init(&digest_ctx, &gex_sss_chip_ctx.session, kAlgorithm_SSS_SHA256, kMode_SSS_Digest);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    if (msg_length <= MAX_SHA_ONE_SHOT_DATA_LEN)
    {
        status = sss_digest_one_go(&digest_ctx, msg, msg_length, hash, &hashLen);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);
    }
    else
    {
        /* Calculate SHA using multistep calls */
        size_t datalenTemp = 0;
        size_t rem_len     = msg_length;

        status = sss_digest_init(&digest_ctx);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

        while (rem_len > 0)
        {
            datalenTemp = (rem_len > MAX_SHA_ONE_SHOT_DATA_LEN) ? MAX_SHA_ONE_SHOT_DATA_LEN : rem_len;
            status      = sss_digest_update(&digest_ctx, (msg + (msg_length - rem_len)), datalenTemp);
            VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);
            rem_len = rem_len - datalenTemp;
        }

        status = sss_digest_finish(&digest_ctx, hash, &hashLen);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);
    }

    status = sss_key_object_init(&keyObject, &gex_sss_chip_ctx.ks);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_key_object_get_handle(&keyObject, keyid);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_asymmetric_context_init(&asymm_ctx, &gex_sss_chip_ctx.session, &keyObject, kAlgorithm_SSS_SHA256, kMode_SSS_Sign);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_asymmetric_sign_digest(&asymm_ctx, hash, hashLen, Uint8::to_uchar(out_signature), &siglen);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    SuccessOrExit(out_signature.SetLength(siglen));

    error = CHIP_NO_ERROR;
exit:
    if (asymm_ctx.session != nullptr)
    {
        sss_asymmetric_context_free(&asymm_ctx);
    }
    if (digest_ctx.session != nullptr)
    {
        sss_digest_context_free(&digest_ctx);
    }
    return error;
}

CHIP_ERROR P256KeypairHSM::ECDSA_sign_hash(const uint8_t * hash, size_t hash_length, P256ECDSASignature & out_signature)
{
    CHIP_ERROR error           = CHIP_ERROR_INTERNAL;
    sss_asymmetric_t asymm_ctx = { 0 };
    sss_status_t status        = kStatus_SSS_Success;
    sss_object_t keyObject     = { 0 };
    size_t siglen              = out_signature.Capacity();

    VerifyOrExit(hash != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(hash_length == kSHA256_Hash_Length, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(out_signature != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(keyid != kKeyId_NotInitialized, error = CHIP_ERROR_HSM);

    ChipLogDetail(Crypto, "ECDSA_sign_hash: Using SE05X for Ecc Sign!");

    se05x_sessionOpen();

    status = sss_key_object_init(&keyObject, &gex_sss_chip_ctx.ks);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_key_object_get_handle(&keyObject, keyid);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_asymmetric_context_init(&asymm_ctx, &gex_sss_chip_ctx.session, &keyObject, kAlgorithm_SSS_SHA256, kMode_SSS_Sign);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status =
        sss_asymmetric_sign_digest(&asymm_ctx, const_cast<uint8_t *>(hash), hash_length, Uint8::to_uchar(out_signature), &siglen);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    SuccessOrExit(out_signature.SetLength(siglen));

    error = CHIP_NO_ERROR;
exit:
    if (asymm_ctx.session != nullptr)
    {
        sss_asymmetric_context_free(&asymm_ctx);
    }

    return error;
}

CHIP_ERROR P256KeypairHSM::Serialize(P256SerializedKeypair & output)
{
    const size_t len = output.Length() == 0 ? output.Capacity() : output.Length();
    Encoding::BufferWriter bbuf(output, len);
    uint8_t privkey[kP256_PrivateKey_Length] = {
        0,
    };

    {
        /* Set the public key */
        P256PublicKeyHSM & public_key = const_cast<P256PublicKeyHSM &>(Pubkey());
        bbuf.Put(Uint8::to_uchar(public_key), public_key.Length());
    }

    VerifyOrReturnError(bbuf.Available() == sizeof(privkey), CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(sizeof(privkey) >= 4, CHIP_ERROR_INTERNAL);

    {
        /* When HSM is used for ECC key generation, store key info in private key buffer */
        Encoding::LittleEndian::BufferWriter privkey_bbuf(privkey, sizeof(privkey));
        privkey_bbuf.Put32(keyid);
    }

    bbuf.Put(privkey, sizeof(privkey));
    VerifyOrReturnError(bbuf.Fit(), CHIP_ERROR_BUFFER_TOO_SMALL);

    output.SetLength(bbuf.Needed());

    return CHIP_NO_ERROR;
}

CHIP_ERROR P256KeypairHSM::Deserialize(P256SerializedKeypair & input)
{
    /* Set the public key */
    P256PublicKeyHSM & public_key = const_cast<P256PublicKeyHSM &>(Pubkey());
    Encoding::BufferWriter bbuf((uint8_t *) Uint8::to_const_uchar(public_key), public_key.Length());

    VerifyOrReturnError(input.Length() == public_key.Length() + kP256_PrivateKey_Length, CHIP_ERROR_INVALID_ARGUMENT);
    bbuf.Put(static_cast<uint8_t *>(input), public_key.Length());

    /* Set private key info */
    VerifyOrReturnError(bbuf.Fit(), CHIP_ERROR_NO_MEMORY);
    {
        /* When HSM is used for ECC key generation, key info in stored in private key buffer */
        const uint8_t * privkey = Uint8::to_const_uchar(input) + public_key.Length();
        keyid                   = Encoding::LittleEndian::Get32(privkey);
        public_key.SetPublicKeyId(keyid);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR P256KeypairHSM::ECDH_derive_secret(const P256PublicKey & remote_public_key, P256ECDHDerivedSecret & out_secret) const
{
    size_t secret_length = (out_secret.Length() == 0) ? out_secret.Capacity() : out_secret.Length();

    VerifyOrReturnError(keyid != kKeyId_NotInitialized, CHIP_ERROR_HSM);

    ChipLogDetail(Crypto, "ECDH_derive_secret: Using SE05X for ECDH !");

    se05x_sessionOpen();

    const uint8_t * const rem_pubKey = Uint8::to_const_uchar(remote_public_key);
    const size_t rem_pubKeyLen       = remote_public_key.Length();

    VerifyOrReturnError(gex_sss_chip_ctx.ks.session != nullptr, CHIP_ERROR_INTERNAL);

    const smStatus_t smstatus = Se05x_API_ECGenSharedSecret(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx, keyid,
                                                            rem_pubKey, rem_pubKeyLen, Uint8::to_uchar(out_secret), &secret_length);
    VerifyOrReturnError(smstatus == SM_OK, CHIP_ERROR_INTERNAL);

    return out_secret.SetLength(secret_length);
}

/* EC Public key HSM implementation */

CHIP_ERROR SE05X_Set_ECDSA_Public_Key(sss_object_t * keyObject, const uint8_t * key, size_t keylen)
{
    uint8_t public_key[128] = {
        0,
    };

    /* ECC NIST-256 Public Key header */
    const uint8_t nist256_header[] = { 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01,
                                       0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00 };

    /* Set public key */
    sss_status_t status = sss_key_object_init(keyObject, &gex_sss_chip_ctx.ks);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    status = sss_key_object_allocate_handle(keyObject, kKeyId_sha256_ecc_pub_keyid, kSSS_KeyPart_Public, kSSS_CipherType_EC_NIST_P,
                                            256, kKeyObject_Mode_Transient);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    VerifyOrReturnError((sizeof(nist256_header) + keylen) <= sizeof(public_key), CHIP_ERROR_INTERNAL);

    memcpy(public_key, nist256_header, sizeof(nist256_header));
    size_t public_key_len = public_key_len + sizeof(nist256_header);
    memcpy(public_key + public_key_len, key, keylen);
    public_key_len = public_key_len + keylen;

    status = sss_key_store_set_key(&gex_sss_chip_ctx.ks, keyObject, public_key, public_key_len, 256, NULL, 0);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR P256PublicKeyHSM::ECDSA_validate_msg_signature(const uint8_t * msg, size_t msg_length,
                                                          const P256ECDSASignature & signature) const
{
    CHIP_ERROR error           = CHIP_ERROR_INTERNAL;
    sss_status_t status        = kStatus_SSS_Success;
    sss_asymmetric_t asymm_ctx = { 0 };
    sss_digest_t ctx_digest    = { 0 };
    uint8_t hash[32]           = {
        0,
    };
    size_t hash_length     = sizeof(hash);
    sss_object_t keyObject = { 0 };

    VerifyOrExit(msg != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(msg_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    ChipLogDetail(Crypto, "ECDSA_validate_msg_signature: Using SE05X for ECDSA verify (msg) !");

    se05x_sessionOpen();

    /* Create hash of input data */
    status = sss_digest_context_init(&ctx_digest, &gex_sss_chip_ctx.session, kAlgorithm_SSS_SHA256, kMode_SSS_Digest);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    if (msg_length <= MAX_SHA_ONE_SHOT_DATA_LEN)
    {
        status = sss_digest_one_go(&ctx_digest, msg, msg_length, hash, &hash_length);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);
    }
    else
    {
        /* Calculate SHA using multistep calls */
        size_t datalenTemp = 0;
        size_t rem_len     = msg_length;

        status = sss_digest_init(&ctx_digest);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

        while (rem_len > 0)
        {
            datalenTemp = (rem_len > MAX_SHA_ONE_SHOT_DATA_LEN) ? MAX_SHA_ONE_SHOT_DATA_LEN : rem_len;
            status      = sss_digest_update(&ctx_digest, (msg + (msg_length - rem_len)), datalenTemp);
            VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);
            rem_len = rem_len - datalenTemp;
        }

        status = sss_digest_finish(&ctx_digest, hash, &hash_length);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);
    }

    if (PublicKeyid == kKeyId_NotInitialized)
    {
        error = SE05X_Set_ECDSA_Public_Key(&keyObject, bytes, kP256_PublicKey_Length);
        SuccessOrExit(error);
        error = CHIP_ERROR_INTERNAL;
    }
    else
    {
        status = sss_key_object_init(&keyObject, &gex_sss_chip_ctx.ks);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

        status = sss_key_object_get_handle(&keyObject, PublicKeyid);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);
    }

    /* ECC Verify */
    status =
        sss_asymmetric_context_init(&asymm_ctx, &gex_sss_chip_ctx.session, &keyObject, kAlgorithm_SSS_SHA256, kMode_SSS_Verify);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_asymmetric_verify_digest(&asymm_ctx, hash, hash_length, (uint8_t *) Uint8::to_const_uchar(signature),
                                          signature.Length());
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INVALID_SIGNATURE);

    error = CHIP_NO_ERROR;
exit:
    if (asymm_ctx.session != NULL)
    {
        sss_asymmetric_context_free(&asymm_ctx);
    }

    if (ctx_digest.session != NULL)
    {
        sss_digest_context_free(&ctx_digest);
    }

    if (PublicKeyid == kKeyId_NotInitialized)
    {
        sss_key_store_erase_key(&gex_sss_chip_ctx.ks, &keyObject);
    }

    return error;
}

CHIP_ERROR P256PublicKeyHSM::ECDSA_validate_hash_signature(const uint8_t * hash, size_t hash_length,
                                                           const P256ECDSASignature & signature) const
{
    CHIP_ERROR error           = CHIP_ERROR_INTERNAL;
    sss_status_t status        = kStatus_SSS_Success;
    sss_asymmetric_t asymm_ctx = { 0 };
    sss_object_t keyObject     = { 0 };

    VerifyOrExit(hash != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(hash_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    ChipLogDetail(Crypto, "ECDSA_validate_hash_signature: Using SE05X for ECDSA verify (hash) !");

    se05x_sessionOpen();

    if (PublicKeyid == kKeyId_NotInitialized)
    {
        error = SE05X_Set_ECDSA_Public_Key(&keyObject, bytes, kP256_PublicKey_Length);
        SuccessOrExit(error);
        error = CHIP_ERROR_INTERNAL;
    }
    else
    {
        status = sss_key_object_init(&keyObject, &gex_sss_chip_ctx.ks);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

        status = sss_key_object_get_handle(&keyObject, PublicKeyid);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);
    }

    /* ECC Verify */
    status =
        sss_asymmetric_context_init(&asymm_ctx, &gex_sss_chip_ctx.session, &keyObject, kAlgorithm_SSS_SHA256, kMode_SSS_Verify);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_asymmetric_verify_digest(&asymm_ctx, const_cast<uint8_t *>(hash), hash_length,
                                          (uint8_t *) Uint8::to_const_uchar(signature), signature.Length());
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INVALID_SIGNATURE);

    error = CHIP_NO_ERROR;
exit:
    if (asymm_ctx.session != NULL)
    {
        sss_asymmetric_context_free(&asymm_ctx);
    }

    if (PublicKeyid == kKeyId_NotInitialized)
    {
        sss_key_store_erase_key(&gex_sss_chip_ctx.ks, &keyObject);
    }

    return error;
}

} // namespace Crypto
} // namespace chip

#endif //#if ENABLE_HSM_GENERATE_EC_KEY
