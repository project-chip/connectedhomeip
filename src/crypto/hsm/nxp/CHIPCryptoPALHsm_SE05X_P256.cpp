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
    size_t pubKeyLen   = sizeof(pubkey);
    size_t pbKeyBitLen = sizeof(pubkey) * 8;

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

    VerifyOrReturnError(msg != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(msg_length > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_signature != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(keyid != kKeyId_NotInitialized, CHIP_ERROR_HSM);

    ChipLogDetail(Crypto, "ECDSA_sign_msg: Using SE05X for Ecc Sign!");

    se05x_sessionOpen();
    VerifyOrReturnError(gex_sss_chip_ctx.ks.session != NULL, CHIP_ERROR_INTERNAL);

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

    VerifyOrReturnError(hash != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(hash_length == kSHA256_Hash_Length, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_signature != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(keyid != kKeyId_NotInitialized, CHIP_ERROR_HSM);

    ChipLogDetail(Crypto, "ECDSA_sign_hash: Using SE05X for Ecc Sign!");

    se05x_sessionOpen();
    VerifyOrReturnError(gex_sss_chip_ctx.ks.session != NULL, CHIP_ERROR_INTERNAL);

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

CHIP_ERROR P256KeypairHSM::Serialize(P256SerializedKeypair & output) const
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
    VerifyOrReturnError(gex_sss_chip_ctx.ks.session != NULL, CHIP_ERROR_INTERNAL);

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
    size_t public_key_len = 0;

    /* ECC NIST-256 Public Key header */
    const uint8_t nist256_header[] = { 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01,
                                       0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00 };

    se05x_sessionOpen();
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

    VerifyOrReturnError(msg != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(msg_length > 0, CHIP_ERROR_INVALID_ARGUMENT);

    ChipLogDetail(Crypto, "ECDSA_validate_msg_signature: Using SE05X for ECDSA verify (msg) !");

    se05x_sessionOpen();
    VerifyOrReturnError(gex_sss_chip_ctx.ks.session != NULL, CHIP_ERROR_INTERNAL);

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

    VerifyOrReturnError(hash != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(hash_length > 0, CHIP_ERROR_INVALID_ARGUMENT);

    ChipLogDetail(Crypto, "ECDSA_validate_hash_signature: Using SE05X for ECDSA verify (hash) !");

    se05x_sessionOpen();
    VerifyOrReturnError(gex_sss_chip_ctx.ks.session != NULL, CHIP_ERROR_INTERNAL);

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

static void add_tlv(uint8_t * buf, size_t buf_index, uint8_t tag, size_t len, uint8_t * val)
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

CHIP_ERROR P256KeypairHSM::NewCertificateSigningRequest(uint8_t * csr, size_t & csr_length)
{
    CHIP_ERROR error           = CHIP_ERROR_INTERNAL;
    sss_status_t status        = kStatus_SSS_Success;
    sss_asymmetric_t asymm_ctx = { 0 };
    sss_object_t keyObject     = { 0 };
    sss_digest_t digest_ctx    = { 0 };

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

    ChipLogDetail(Crypto, "NewCertificateSigningRequest: Using SE05X for creating CSR !");

    // No extensions are copied
    buffer_index -= kTlvHeader;
    add_tlv(data_to_hash, buffer_index, (ASN1_CONSTRUCTED | ASN1_CONTEXT_SPECIFIC), 0, NULL);

    // Copy public key (with header)
    {
        P256PublicKeyHSM & public_key = const_cast<P256PublicKeyHSM &>(Pubkey());

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

    /* Create hash of `data_to_hash` buffer */
    status = sss_digest_context_init(&digest_ctx, &gex_sss_chip_ctx.session, kAlgorithm_SSS_SHA256, kMode_SSS_Digest);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_digest_one_go(&digest_ctx, data_to_hash, data_to_hash_len, hash, &hash_length);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

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
    csr[csr_index++] = (uint8_t)(data_to_hash_len + 14 + kTlvHeader + signature_len);

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
    csr[csr_index++] = (uint8_t)((signature[0] != 0) ? (signature_len + 1) : (signature_len));

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

    if (digest_ctx.session != NULL)
    {
        sss_digest_context_free(&digest_ctx);
    }

    return error;
}

} // namespace Crypto
} // namespace chip

#endif //#if ENABLE_HSM_GENERATE_EC_KEY
