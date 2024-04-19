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

// Define keyid
uint32_t keyid = 0;
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

#if (ENABLE_TRUSTM_GENERATE_EC_KEY || ENABLE_TRUSTM_ECDSA_VERIFY)
static CHIP_ERROR get_trustm_keyid_from_keypair(const P256KeypairContext mKeypair, uint32_t * key_id)
{
    if (0 != memcmp(&mKeypair.mBytes[0], trustm_magic_no, sizeof(trustm_magic_no)))
    {
        return CHIP_ERROR_INTERNAL;
    }

    *key_id += (mKeypair.mBytes[CRYPTO_KEYPAIR_KEYID_OFFSET]) | (mKeypair.mBytes[CRYPTO_KEYPAIR_KEYID_OFFSET + 1] << 8);

    return CHIP_NO_ERROR;
}
#endif // #if (ENABLE_TRUSTM_GENERATE_EC_KEY || ENABLE_TRUSTM_ECDSA_VERIFY)

P256Keypair::~P256Keypair()
{
    // Add method to get the keyid
    if (CHIP_NO_ERROR != get_trustm_keyid_from_keypair(mKeypair, &keyid))
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
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;

#if !ENABLE_TRUSTM_GENERATE_EC_KEY
    if (CHIP_NO_ERROR == Initialize_H(this, &mPublicKey, &mKeypair))
    {
        mInitialized = true;
    }
    error = CHIP_NO_ERROR;
    return error;
#else
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
        // Add the logic to use different keyid
        keyid = TRUSTM_NODE_OID_KEY_START;
        // Trust M ECC 256 Key Gen
        ChipLogDetail(Crypto, "Generating NIST256 key in TrustM !");
        key_usage = (optiga_key_usage_t) (OPTIGA_KEY_USAGE_SIGN | OPTIGA_KEY_USAGE_AUTHENTICATION);
    }
    // Trust M init
    trustm_Open();
    return_status = trustm_ecc_keygen(keyid, key_usage, OPTIGA_ECC_CURVE_NIST_P_256, pubkey, &pubKeyLen);

    // Add signature length
    VerifyOrExit(return_status == OPTIGA_LIB_SUCCESS, error = CHIP_ERROR_INTERNAL);

    /* Set the public key */
    VerifyOrReturnError((size_t) pubKeyLen > NIST256_HEADER_OFFSET, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(((size_t) pubKeyLen - NIST256_HEADER_OFFSET) <= kP256_PublicKey_Length, CHIP_ERROR_INTERNAL);
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
#endif
}

CHIP_ERROR P256Keypair::ECDSA_sign_msg(const uint8_t * msg, size_t msg_length, P256ECDSASignature & out_signature) const
{
#if !ENABLE_TRUSTM_GENERATE_EC_KEY
    return ECDSA_sign_msg_H(&mKeypair, msg, msg_length, out_signature);
#else
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
    ChipLogDetail(Crypto, "TrustM: ECDSA_sign_msg");
    // Trust M Init
    trustm_Open();
    // Hash to get the digest
    Hash_SHA256(msg, msg_length, &digest[0]);
    uint16_t keyid = (mKeypair.mBytes[CRYPTO_KEYPAIR_KEYID_OFFSET]) | (mKeypair.mBytes[CRYPTO_KEYPAIR_KEYID_OFFSET + 1] << 8);
    // Api call to calculate the signature
    if (keyid == OPTIGA_KEY_ID_E0F2)
    {
        return_status = trustm_ecdsa_sign(OPTIGA_KEY_ID_E0F2, digest, digest_length, signature_trustm, &signature_trustm_len);
    }
    else
    {
        return_status = trustm_ecdsa_sign(OPTIGA_KEY_ID_E0F0, digest, digest_length, signature_trustm, &signature_trustm_len);
    }

    VerifyOrExit(return_status == OPTIGA_LIB_SUCCESS, error = CHIP_ERROR_INTERNAL);

    error = EcdsaAsn1SignatureToRaw(kP256_FE_Length, ByteSpan{ signature_trustm, signature_trustm_len }, out_raw_sig_span);

    SuccessOrExit(error);

    out_signature.SetLength(2 * kP256_FE_Length);

    error = CHIP_NO_ERROR;

exit:
    if (error != CHIP_NO_ERROR)
    {
        trustm_close();
    }
    return error;
#endif
}

CHIP_ERROR P256Keypair::ECDH_derive_secret(const P256PublicKey & remote_public_key, P256ECDHDerivedSecret & out_secret) const
{
#if !ENABLE_TRUSTM_GENERATE_EC_KEY
    return ECDH_derive_secret_H(&mKeypair, remote_public_key, out_secret);
#else
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
    trustm_Open();

    const uint8_t * const rem_pubKey = Uint8::to_const_uchar(remote_public_key);
    const size_t rem_pubKeyLen       = remote_public_key.Length();

    uint8_t remote_key[68];
    uint8_t header[3] = { 0x03, 0x42, 0x00 };

    memcpy(remote_key, &header, 3);
    memcpy(remote_key + 3, rem_pubKey, rem_pubKeyLen);

    return_status = trustm_ecdh_derive_secret(OPTIGA_KEY_ID_E100, (uint8_t *) remote_key, (uint16_t) rem_pubKeyLen + 3,
                                              out_secret.Bytes(), (uint8_t) secret_length);

    VerifyOrExit(return_status == OPTIGA_LIB_SUCCESS, error = CHIP_ERROR_INTERNAL);

exit:
    if (error != CHIP_NO_ERROR)
    {
        trustm_close();
    }
    return out_secret.SetLength(secret_length);
#endif
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

    uint8_t hash_length_u8            = static_cast<uint8_t>(hash_length);
    uint16_t signature_trustm_len_u16 = static_cast<uint16_t>(signature_trustm_len);

    VerifyOrReturnError(hash != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(hash_length > 0, CHIP_ERROR_INVALID_ARGUMENT);
    ChipLogDetail(Crypto, "TrustM: ECDSA_validate_hash_signature");

    // Trust M init
    trustm_Open();
    error = EcdsaRawSignatureToAsn1(kP256_FE_Length, ByteSpan{ Uint8::to_const_uchar(signature.ConstBytes()), signature.Length() },
                                    out_der_sig_span);
    SuccessOrExit(error);

    signature_trustm_len = out_der_sig_span.size();
    // ECC verify
    return_status = trustm_ecdsa_verify((uint8_t *) hash, hash_length_u8, (uint8_t *) signature_trustm, signature_trustm_len_u16,
                                        (uint8_t *) bytes, (uint8_t) kP256_PublicKey_Length);

    VerifyOrExit(return_status == OPTIGA_LIB_SUCCESS, error = CHIP_ERROR_INTERNAL);

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

    /* Set the public key */
    P256PublicKey & public_key = const_cast<P256PublicKey &>(Pubkey());
    bbuf.Put(Uint8::to_uchar(public_key), public_key.Length());

    VerifyOrReturnError(bbuf.Available() == sizeof(privkey), CHIP_ERROR_INTERNAL);

    /* Set the private key trustm_magic_no */
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
#if !ENABLE_TRUSTM_KEY_IMPORT
        if (CHIP_NO_ERROR == (error = Deserialize_H(this, &mPublicKey, &mKeypair, input)))
        {
            mInitialized = true;
        }
        return error;
#else
        // Add in code for Trust M
        return CHIP_NO_ERROR;
#endif
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
    uint16_t signature_trustm_len_u16 = static_cast<uint16_t>(signature_trustm_len);

    VerifyOrReturnError(msg != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(msg_length > 0, CHIP_ERROR_INVALID_ARGUMENT);

    ChipLogDetail(Crypto, "TrustM: ECDSA_validate_msg_signature");

    // Trust M init
    trustm_Open();

    error = EcdsaRawSignatureToAsn1(kP256_FE_Length, ByteSpan{ Uint8::to_const_uchar(signature.ConstBytes()), signature.Length() },
                                    out_der_sig_span);
    SuccessOrExit(error);

    signature_trustm_len = out_der_sig_span.size();
    // Hash to get the digest
    memset(&digest[0], 0, sizeof(digest));
    Hash_SHA256(msg, msg_length, &digest[0]);
    // ECC verify
    return_status = trustm_ecdsa_verify(digest, digest_length, (uint8_t *) signature_trustm, signature_trustm_len_u16,
                                        (uint8_t *) bytes, (uint8_t) kP256_PublicKey_Length);

    VerifyOrExit(return_status == OPTIGA_LIB_SUCCESS, error = CHIP_ERROR_INTERNAL);

exit:
    if (error != CHIP_NO_ERROR)
    {
        trustm_close();
    }
    return error;
#endif
}

static void add_tlv(uint8_t * buf, size_t buf_index, uint8_t tag, size_t len, uint8_t * val)
{
    buf[buf_index++] = tag;
    buf[buf_index++] = (uint8_t) len;
    if (len > 0 && val != NULL)
    {
        memcpy(&buf[buf_index], val, len);
        buf_index = buf_index + len;
    }
}

CHIP_ERROR P256Keypair::NewCertificateSigningRequest(uint8_t * csr, size_t & csr_length) const
{
#if !ENABLE_TRUSTM_GENERATE_EC_KEY
    return NewCertificateSigningRequest_H(&mKeypair, csr, csr_length);
#else
    CHIP_ERROR error                  = CHIP_ERROR_INTERNAL;
    optiga_lib_status_t return_status = OPTIGA_LIB_BUSY;

    uint8_t data_to_hash[128]     = { 0 };
    size_t data_to_hash_len       = sizeof(data_to_hash);
    uint8_t pubkey[128]           = { 0 };
    size_t pubKeyLen              = 0;
    uint8_t digest[32]            = { 0 };
    uint8_t digest_length         = sizeof(digest);
    uint8_t signature_trustm[128] = { 0 };
    uint16_t signature_len        = sizeof(signature_trustm);

    size_t csr_index    = 0;
    size_t buffer_index = data_to_hash_len;

    // Dummy value
    uint8_t organisation_oid[3] = { 0x55, 0x04, 0x0a };

    // Version  ::=  INTEGER  {  v1(0), v2(1), v3(2)  }
    uint8_t version[3]       = { 0x02, 0x01, 0x00 };
    uint8_t signature_oid[8] = { 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02 };
    uint8_t nist256_header[] = { 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01,
                                 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00 };

    VerifyOrReturnError(mInitialized, CHIP_ERROR_UNINITIALIZED);

    if (CHIP_NO_ERROR != get_trustm_keyid_from_keypair(mKeypair, &keyid))
    {
        ChipLogDetail(Crypto, "NewCertificateSigningRequest : Host");
        return NewCertificateSigningRequest_H(&mKeypair, csr, csr_length);
    }
    ChipLogDetail(Crypto, "NewCertificateSigningRequest: TrustM");

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

    // Hash to get the digest
    memset(&digest[0], 0, sizeof(digest));
    error = Hash_SHA256(data_to_hash, data_to_hash_len, digest);
    SuccessOrExit(error);

    // Trust M Init
    trustm_Open();

    // Sign on hash
    return_status = trustm_ecdsa_sign(OPTIGA_KEY_ID_E0F2, digest, digest_length, signature_trustm, &signature_len);

    VerifyOrExit(return_status == OPTIGA_LIB_SUCCESS, error = CHIP_ERROR_INTERNAL);
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
    csr[csr_index++] = (uint8_t) ((signature_trustm[0] != 0) ? (signature_len + 1) : (signature_len));

    if (signature_trustm[0] != 0)
    {
        VerifyOrExit(csr_index <= csr_length, error = CHIP_ERROR_INTERNAL);
        csr[csr_index++] = 0x00;
        // Increament total count by 1
        csr[2]++;
    }
    VerifyOrExit((csr_index + signature_len) <= csr_length, error = CHIP_ERROR_INTERNAL);
    memcpy(&csr[csr_index], signature_trustm, signature_len);

    csr_length = (csr_index + signature_len);

    error = CHIP_NO_ERROR;
exit:
    if (error != CHIP_NO_ERROR)
    {
        trustm_close();
    }
    return error;

#endif
}

} // namespace Crypto
} // namespace chip
