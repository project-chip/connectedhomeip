/*
 *
 * Copyright (c) 2023 Project CHIP Authors
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

#if ENABLE_HSM_GENERATE_EC_KEY

namespace chip {
namespace Crypto {

P256KeypairHSM::~P256KeypairHSM() {}

CHIP_ERROR P256KeypairHSM::Initialize(ECPKeyTarget key_target)
{
    CHIP_ERROR error                  = CHIP_ERROR_INTERNAL;
    optiga_lib_status_t return_status = OPTIGA_LIB_BUSY;
    uint8_t pubkey[128]               = {
        0,
    };
    uint16_t pubKeyLen = sizeof(pubkey);

    if (keyid == 0)
    {
        ChipLogDetail(Crypto, "Keyid not set !. Set key id using 'SetKeyId' member class !");
        return CHIP_ERROR_INTERNAL;
    }

    // Trust M init
    trustm_Open();
    if (provisioned_key == false)
    {
        // Trust M ECC 256 Key Gen
        ChipLogDetail(Crypto, "Generating NIST256 key in Trust M !");
        uint8_t key_usage =
            (optiga_key_usage_t)(OPTIGA_KEY_USAGE_SIGN | OPTIGA_KEY_USAGE_AUTHENTICATION);
        return_status = trustm_ecc_keygen(OPTIGA_KEY_ID_E0F2, key_usage, OPTIGA_ECC_CURVE_NIST_P_256, pubkey, pubKeyLen);
        VerifyOrExit(return_status == OPTIGA_LIB_SUCCESS, error = CHIP_ERROR_INTERNAL);
    }
    else
    {
        // Read out the public Key stored
        ChipLogDetail(Crypto, "Provisioned_key - %lx !", keyid);
        trustmGetKey(TRUSTM_P256_PUBKEY_OID_KEY, pubkey, &pubKeyLen);

        /* Set the public key */
        P256PublicKeyHSM & public_key = const_cast<P256PublicKeyHSM &>(Pubkey());
        VerifyOrReturnError((size_t) pubKeyLen > NIST256_HEADER_OFFSET, CHIP_ERROR_INTERNAL);
        VerifyOrReturnError(((size_t) pubKeyLen - NIST256_HEADER_OFFSET) <= kP256_PublicKey_Length, CHIP_ERROR_INTERNAL);
        memcpy((void *) Uint8::to_const_uchar(public_key), pubkey, pubKeyLen);
        public_key.SetPublicKeyId(keyid);
    }
    error = CHIP_NO_ERROR;
exit:
    if (error != CHIP_NO_ERROR)
    {
        trustm_close();
    }
    return error;
}
CHIP_ERROR P256KeypairHSM::ECDSA_sign_msg(const uint8_t * msg, size_t msg_length, P256ECDSASignature & out_signature) const
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;

    uint8_t signature_trustm[kMax_ECDSA_Signature_Length_Der] = { 0 };
    uint16_t signature_trustm_len                             = (uint16_t) kMax_ECDSA_Signature_Length_Der;
    uint8_t digest[32];
    uint8_t digest_length = sizeof(digest);

    memset(&digest[0], 0, sizeof(digest));
    MutableByteSpan out_raw_sig_span(out_signature.Bytes(), out_signature.Capacity());

    optiga_lib_status_t return_status = OPTIGA_LIB_BUSY;

    VerifyOrReturnError(msg != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(msg_length > 0, CHIP_ERROR_INVALID_ARGUMENT);

    // Trust M Init
    trustm_Open();
    // Hash to get the digest
    Hash_SHA256(msg, msg_length, &digest[0]);
    // Api call to calculate the signature
    return_status = trustm_ecdsa_sign(OPTIGA_KEY_ID_E0F0, digest, digest_length, signature_trustm, &signature_trustm_len);

    VerifyOrExit(return_status == OPTIGA_LIB_SUCCESS, error = CHIP_ERROR_INTERNAL);

    error = EcdsaAsn1SignatureToRaw(kP256_FE_Length, ByteSpan{ signature_trustm, signature_trustm_len }, out_raw_sig_span);

    ChipLogError(NotSpecified, "EcdsaAsn1SignatureToRaw %" CHIP_ERROR_FORMAT, error.Format());

    SuccessOrExit(error);

    SuccessOrExit(out_signature.SetLength(2 * kP256_FE_Length));

    error = CHIP_NO_ERROR;

exit:
    if (error != CHIP_NO_ERROR)
    {
        trustm_close();
    }
    return error;
}

CHIP_ERROR P256KeypairHSM::ECDH_derive_secret(const P256PublicKey & remote_public_key, P256ECDHDerivedSecret & out_secret) const
{
    // Use the software based one
    return P256Keypair::ECDH_derive_secret(remote_public_key, out_secret);
}

CHIP_ERROR P256PublicKeyHSM::ECDSA_validate_hash_signature(const uint8_t * hash, size_t hash_length,
                                                           const P256ECDSASignature & signature) const
{
    CHIP_ERROR error                                          = CHIP_ERROR_INTERNAL;
    optiga_lib_status_t return_status                         = OPTIGA_LIB_BUSY;
    uint8_t signature_trustm[kMax_ECDSA_Signature_Length_Der] = { 0 };
    size_t signature_trustm_len                               = sizeof(signature_trustm);
    MutableByteSpan out_der_sig_span(signature_trustm, signature_trustm_len);

    uint8_t hash_length_u8            = static_cast<uint8_t>(hash_length);
    uint16_t signature_trustm_len_u16 = static_cast<uint16_t>(signature_trustm_len);

    VerifyOrReturnError(hash != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(hash_length > 0, CHIP_ERROR_INVALID_ARGUMENT);
    ChipLogDetail(Crypto, "ECDSA_validate_hash_signature: Using TrustM for TrustM verify (hash) !");

    // Trust M init
    trustm_Open();
    error = EcdsaRawSignatureToAsn1(kP256_FE_Length, ByteSpan{ Uint8::to_const_uchar(signature.ConstBytes()), signature.Length() },
                                    out_der_sig_span);
    SuccessOrExit(error);

    /* Set the public key */
    // P256PublicKeyHSM & public_key = const_cast<P256PublicKeyHSM &>(Pubkey());
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
}

CHIP_ERROR P256KeypairHSM::Serialize(P256SerializedKeypair & output) const
{
    const size_t len = output.Length() == 0 ? output.Capacity() : output.Length();
    Encoding::BufferWriter bbuf(output.Bytes(), len);
    uint8_t privkey[kP256_PrivateKey_Length] = {
        0,
    };

    /* Set the public key */
    P256PublicKeyHSM & public_key = const_cast<P256PublicKeyHSM &>(Pubkey());
    bbuf.Put(Uint8::to_uchar(public_key), public_key.Length());

    VerifyOrReturnError(bbuf.Available() == sizeof(privkey), CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(sizeof(privkey) >= 4, CHIP_ERROR_INTERNAL);

    /* When HSM is used for ECC key generation, store key info in private key buffer */
    Encoding::LittleEndian::BufferWriter privkey_bbuf(privkey, sizeof(privkey));
    privkey_bbuf.Put32(keyid);

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
    bbuf.Put(input.ConstBytes(), public_key.Length());

    /* Set private key info */
    VerifyOrReturnError(bbuf.Fit(), CHIP_ERROR_NO_MEMORY);

    /* When HSM is used for ECC key generation, key info in stored in private key buffer */
    const uint8_t * privkey = input.ConstBytes() + public_key.Length();
    keyid                   = Encoding::LittleEndian::Get32(privkey);
    public_key.SetPublicKeyId(keyid);

    return CHIP_NO_ERROR;
}

CHIP_ERROR P256PublicKeyHSM::ECDSA_validate_msg_signature(const uint8_t * msg, size_t msg_length,
                                                          const P256ECDSASignature & signature) const
{
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

    ChipLogDetail(Crypto, "ECDSA_validate_msg_signature: Using TrustM for TrustM verify (msg) !");

    // Trust M init
    trustm_Open();

    error = EcdsaRawSignatureToAsn1(kP256_FE_Length, ByteSpan{ Uint8::to_const_uchar(signature.ConstBytes()), signature.Length() },
                                    out_der_sig_span);
    SuccessOrExit(error);

    /* Set the public key */
    // P256PublicKeyHSM & public_key = const_cast<P256PublicKeyHSM &>(Pubkey());
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
}

CHIP_ERROR P256KeypairHSM::NewCertificateSigningRequest(uint8_t * csr, size_t & csr_length) const
{
    MutableByteSpan out_csr(csr, csr_length);
    CHIP_ERROR err = GenerateCertificateSigningRequest(this, out_csr);
    csr_length     = (CHIP_NO_ERROR == err) ? out_csr.size() : 0;
    return err;
}

} // namespace Crypto
} // namespace chip

#endif //#if ENABLE_HSM_GENERATE_EC_KEY
