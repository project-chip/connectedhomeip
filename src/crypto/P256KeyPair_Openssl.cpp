/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
 *      openSSL based implementation of CHIP crypto primitives
 */

#include "CHIPCryptoPAL.h"
#include "CHIPCryptoPALOpenSSL.h"

#include <type_traits>

#if CHIP_CRYPTO_BORINGSSL
#include <openssl/aead.h>
#endif // CHIP_CRYPTO_BORINGSSL

#include <openssl/err.h>
#include <openssl/evp.h>

#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>

namespace chip {
namespace Crypto {

static inline void from_EC_KEY(EC_KEY * key, P256KeypairContext * context)
{
    *SafePointerCast<EC_KEY **>(context) = key;
}

static inline EC_KEY * to_EC_KEY(P256KeypairContext * context)
{
    return *SafePointerCast<EC_KEY **>(context);
}

static inline const EC_KEY * to_const_EC_KEY(const P256KeypairContext * context)
{
    return *SafePointerCast<const EC_KEY * const *>(context);
}

CHIP_ERROR _create_evp_key_from_binary_p256_key(const P256PublicKey & key, EVP_PKEY ** out_evp_pkey)
{

    CHIP_ERROR error = CHIP_NO_ERROR;
    EC_KEY * ec_key  = nullptr;
    int result       = -1;
    EC_POINT * point = nullptr;
    EC_GROUP * group = nullptr;
    int nid          = NID_undef;

    VerifyOrExit(*out_evp_pkey == nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);

    nid = CryptoPAL_nidForCurve(CryptoPAL_MapECName(key.Type()));
    VerifyOrExit(nid != NID_undef, error = CHIP_ERROR_INTERNAL);

    ec_key = EC_KEY_new_by_curve_name(nid);
    VerifyOrExit(ec_key != nullptr, error = CHIP_ERROR_INTERNAL);

    group = EC_GROUP_new_by_curve_name(nid);
    VerifyOrExit(group != nullptr, error = CHIP_ERROR_INTERNAL);

    point = EC_POINT_new(group);
    VerifyOrExit(point != nullptr, error = CHIP_ERROR_INTERNAL);

    result = EC_POINT_oct2point(group, point, Uint8::to_const_uchar(key), key.Length(), nullptr);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    result = EC_KEY_set_public_key(ec_key, point);

    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    *out_evp_pkey = EVP_PKEY_new();
    VerifyOrExit(*out_evp_pkey != nullptr, error = CHIP_ERROR_INTERNAL);

    result = EVP_PKEY_set1_EC_KEY(*out_evp_pkey, ec_key);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

exit:
    if (ec_key != nullptr)
    {
        EC_KEY_free(ec_key);
        ec_key = nullptr;
    }

    if (error != CHIP_NO_ERROR && *out_evp_pkey)
    {
        EVP_PKEY_free(*out_evp_pkey);
        out_evp_pkey = nullptr;
    }

    if (point != nullptr)
    {
        EC_POINT_free(point);
        point = nullptr;
    }

    if (group != nullptr)
    {
        EC_GROUP_free(group);
        group = nullptr;
    }

    return error;
}

CHIP_ERROR P256Keypair::ECDSA_sign_msg(const uint8_t * msg, const size_t msg_length, P256ECDSASignature & out_signature) const
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int nid          = NID_undef;
    EC_KEY * ec_key  = nullptr;
    ECDSA_SIG * sig  = nullptr;
    const BIGNUM * r = nullptr;
    const BIGNUM * s = nullptr;

    VerifyOrReturnError((msg != nullptr) && (msg_length > 0), CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t digest[kSHA256_Hash_Length];
    memset(&digest[0], 0, sizeof(digest));

    ReturnErrorOnFailure(Hash_SHA256(msg, msg_length, &digest[0]));

    ERR_clear_error();

    static_assert(P256ECDSASignature::Capacity() >= kP256_ECDSA_Signature_Length_Raw, "P256ECDSASignature must be large enough");
    VerifyOrExit(mInitialized, error = CHIP_ERROR_UNINITIALIZED);
    nid = CryptoPAL_nidForCurve(CryptoPAL_MapECName(mPublicKey.Type()));
    VerifyOrExit(nid != NID_undef, error = CHIP_ERROR_INVALID_ARGUMENT);

    ec_key = to_EC_KEY(&mKeypair);
    VerifyOrExit(ec_key != nullptr, error = CHIP_ERROR_INTERNAL);

    sig = ECDSA_do_sign(Uint8::to_const_uchar(&digest[0]), static_cast<boringssl_size_t_openssl_int>(sizeof(digest)), ec_key);
    VerifyOrExit(sig != nullptr, error = CHIP_ERROR_INTERNAL);
    ECDSA_SIG_get0(sig, &r, &s);
    VerifyOrExit((r != nullptr) && (s != nullptr), error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(CanCastTo<size_t>(BN_num_bytes(r)) && CanCastTo<size_t>(BN_num_bytes(s)), error = CHIP_ERROR_INTERNAL);
    VerifyOrExit((static_cast<size_t>(BN_num_bytes(r)) <= kP256_FE_Length) &&
                     (static_cast<size_t>(BN_num_bytes(s)) <= kP256_FE_Length),
                 error = CHIP_ERROR_INTERNAL);

    // Concatenate r and s to output. Sizes were checked above.
    VerifyOrExit(out_signature.SetLength(kP256_ECDSA_Signature_Length_Raw) == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(BN_bn2binpad(r, out_signature.Bytes() + 0u, kP256_FE_Length) == kP256_FE_Length, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(BN_bn2binpad(s, out_signature.Bytes() + kP256_FE_Length, kP256_FE_Length) == kP256_FE_Length,
                 error = CHIP_ERROR_INTERNAL);

exit:
    if (sig != nullptr)
    {
        // SIG owns the memory of r, s
        ECDSA_SIG_free(sig);
    }

    if (error != CHIP_NO_ERROR)
    {
        CryptoPAL_logSSLError();
    }

    return error;
}

CHIP_ERROR P256Keypair::ECDH_derive_secret(const P256PublicKey & remote_public_key, P256ECDHDerivedSecret & out_secret) const
{
    ERR_clear_error();
    CHIP_ERROR error      = CHIP_NO_ERROR;
    int result            = -1;
    EVP_PKEY * local_key  = nullptr;
    EVP_PKEY * remote_key = nullptr;

    EVP_PKEY_CTX * context = nullptr;
    size_t out_buf_length  = 0;

    EC_KEY * ec_key = EC_KEY_dup(to_const_EC_KEY(&mKeypair));
    VerifyOrExit(ec_key != nullptr, error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(mInitialized, error = CHIP_ERROR_UNINITIALIZED);

    local_key = EVP_PKEY_new();
    VerifyOrExit(local_key != nullptr, error = CHIP_ERROR_INTERNAL);

    result = EVP_PKEY_set1_EC_KEY(local_key, ec_key);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    error = _create_evp_key_from_binary_p256_key(remote_public_key, &remote_key);
    SuccessOrExit(error);

    context = EVP_PKEY_CTX_new(local_key, nullptr);
    VerifyOrExit(context != nullptr, error = CHIP_ERROR_INTERNAL);

    result = EVP_PKEY_derive_init(context);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    result = EVP_PKEY_derive_set_peer(context, remote_key);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    out_buf_length = (out_secret.Length() == 0) ? out_secret.Capacity() : out_secret.Length();
    result         = EVP_PKEY_derive(context, out_secret.Bytes(), &out_buf_length);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
    SuccessOrExit(error = out_secret.SetLength(out_buf_length));

exit:
    if (ec_key != nullptr)
    {
        EC_KEY_free(ec_key);
        ec_key = nullptr;
    }

    if (local_key != nullptr)
    {
        EVP_PKEY_free(local_key);
        local_key = nullptr;
    }

    if (remote_key != nullptr)
    {
        EVP_PKEY_free(remote_key);
        remote_key = nullptr;
    }

    if (context != nullptr)
    {
        EVP_PKEY_CTX_free(context);
        context = nullptr;
    }

    CryptoPAL_logSSLError();
    return error;
}

CHIP_ERROR P256Keypair::Initialize(ECPKeyTarget key_target)
{
    ERR_clear_error();

    Clear();

    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;
    EC_KEY * ec_key  = nullptr;
    ECName curve     = CryptoPAL_MapECName(mPublicKey.Type());

    int nid = CryptoPAL_nidForCurve(curve);
    VerifyOrExit(nid != NID_undef, error = CHIP_ERROR_INVALID_ARGUMENT);

    ec_key = EC_KEY_new_by_curve_name(nid);
    VerifyOrExit(ec_key != nullptr, error = CHIP_ERROR_INTERNAL);

    result = EC_KEY_generate_key(ec_key);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    error = P256PublicKeyFromECKey(ec_key, mPublicKey);
    SuccessOrExit(error);

    from_EC_KEY(ec_key, &mKeypair);
    mInitialized = true;
    ec_key       = nullptr;

exit:
    if (ec_key != nullptr)
    {
        EC_KEY_free(ec_key);
        ec_key = nullptr;
    }

    CryptoPAL_logSSLError();
    return error;
}

CHIP_ERROR P256Keypair::Serialize(P256SerializedKeypair & output) const
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    const EC_KEY * ec_key = to_const_EC_KEY(&mKeypair);
    uint8_t privkey[kP256_PrivateKey_Length];

    int privkey_size          = 0;
    const BIGNUM * privkey_bn = EC_KEY_get0_private_key(ec_key);
    VerifyOrExit(privkey_bn != nullptr, error = CHIP_ERROR_INTERNAL);

    privkey_size = BN_bn2binpad(privkey_bn, privkey, sizeof(privkey));
    privkey_bn   = nullptr;

    VerifyOrExit(privkey_size > 0, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit((size_t) privkey_size == sizeof(privkey), error = CHIP_ERROR_INTERNAL);

    {
        size_t len = output.Length() == 0 ? output.Capacity() : output.Length();
        Encoding::BufferWriter bbuf(output.Bytes(), len);
        bbuf.Put(mPublicKey, mPublicKey.Length());
        bbuf.Put(privkey, sizeof(privkey));
        VerifyOrExit(bbuf.Fit(), error = CHIP_ERROR_NO_MEMORY);
        output.SetLength(bbuf.Needed());
    }

exit:
    ClearSecretData(privkey, sizeof(privkey));
    CryptoPAL_logSSLError();
    return error;
}

CHIP_ERROR P256Keypair::Deserialize(P256SerializedKeypair & input)
{
    Encoding::BufferWriter bbuf(mPublicKey, mPublicKey.Length());

    Clear();

    BIGNUM * pvt_key     = nullptr;
    EC_GROUP * group     = nullptr;
    EC_POINT * key_point = nullptr;

    EC_KEY * ec_key = nullptr;
    ECName curve    = CryptoPAL_MapECName(mPublicKey.Type());

    ERR_clear_error();
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;
    int nid          = NID_undef;

    const uint8_t * privkey = input.ConstBytes() + mPublicKey.Length();

    VerifyOrExit(input.Length() == mPublicKey.Length() + kP256_PrivateKey_Length, error = CHIP_ERROR_INVALID_ARGUMENT);
    bbuf.Put(input.ConstBytes(), mPublicKey.Length());
    VerifyOrExit(bbuf.Fit(), error = CHIP_ERROR_NO_MEMORY);

    nid = CryptoPAL_nidForCurve(curve);
    VerifyOrExit(nid != NID_undef, error = CHIP_ERROR_INVALID_ARGUMENT);

    group = EC_GROUP_new_by_curve_name(nid);
    VerifyOrExit(group != nullptr, error = CHIP_ERROR_INTERNAL);

    key_point = EC_POINT_new(group);
    VerifyOrExit(key_point != nullptr, error = CHIP_ERROR_INTERNAL);

    result = EC_POINT_oct2point(group, key_point, Uint8::to_const_uchar(mPublicKey), mPublicKey.Length(), nullptr);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    ec_key = EC_KEY_new_by_curve_name(nid);
    VerifyOrExit(ec_key != nullptr, error = CHIP_ERROR_INTERNAL);

    result = EC_KEY_set_public_key(ec_key, key_point);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    pvt_key = BN_bin2bn(privkey, kP256_PrivateKey_Length, nullptr);
    VerifyOrExit(pvt_key != nullptr, error = CHIP_ERROR_INTERNAL);

    result = EC_KEY_set_private_key(ec_key, pvt_key);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    from_EC_KEY(ec_key, &mKeypair);
    mInitialized = true;
    ec_key       = nullptr;

exit:
    if (ec_key != nullptr)
    {
        EC_KEY_free(ec_key);
        ec_key = nullptr;
    }

    if (group != nullptr)
    {
        EC_GROUP_free(group);
        group = nullptr;
    }

    if (pvt_key != nullptr)
    {
        BN_free(pvt_key);
        pvt_key = nullptr;
    }

    if (key_point != nullptr)
    {
        EC_POINT_free(key_point);
        key_point = nullptr;
    }
    CryptoPAL_logSSLError();
    return error;
}

void P256Keypair::Clear()
{
    if (mInitialized)
    {
        EC_KEY * ec_key = to_EC_KEY(&mKeypair);
        EC_KEY_free(ec_key);
        mInitialized = false;
    }
}

P256Keypair::~P256Keypair()
{
    Clear();
}

CHIP_ERROR P256Keypair::NewCertificateSigningRequest(uint8_t * out_csr, size_t & csr_length) const
{
    ERR_clear_error();
    CHIP_ERROR error     = CHIP_NO_ERROR;
    int result           = 0;
    int csr_length_local = 0;

    X509_REQ * x509_req = X509_REQ_new();
    EVP_PKEY * evp_pkey = nullptr;

    EC_KEY * ec_key = to_EC_KEY(&mKeypair);

    X509_NAME * subject = X509_NAME_new();
    VerifyOrExit(subject != nullptr, error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(mInitialized, error = CHIP_ERROR_UNINITIALIZED);

    result = X509_REQ_set_version(x509_req, 0);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    result = EC_KEY_check_key(ec_key);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    evp_pkey = EVP_PKEY_new();
    VerifyOrExit(evp_pkey != nullptr, error = CHIP_ERROR_INTERNAL);

    result = EVP_PKEY_set1_EC_KEY(evp_pkey, ec_key);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    result = X509_REQ_set_pubkey(x509_req, evp_pkey);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // TODO: mbedTLS CSR parser fails if the subject name is not set (or if empty).
    //       CHIP Spec doesn't specify the subject name that can be used.
    //       Figure out the correct value and update this code.
    result = X509_NAME_add_entry_by_txt(subject, "O", MBSTRING_ASC, Uint8::from_const_char("CSR"), -1, -1, 0);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    result = X509_REQ_set_subject_name(x509_req, subject);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    result = X509_REQ_sign(x509_req, evp_pkey, EVP_sha256());
    VerifyOrExit(result > 0, error = CHIP_ERROR_INTERNAL);

    csr_length_local = i2d_X509_REQ(x509_req, nullptr);
    VerifyOrExit(csr_length_local >= 0, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(CanCastTo<size_t>(csr_length_local), error = CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrExit(static_cast<size_t>(csr_length_local) <= csr_length, error = CHIP_ERROR_BUFFER_TOO_SMALL);
    csr_length = static_cast<size_t>(i2d_X509_REQ(x509_req, &out_csr));

exit:
    ec_key = nullptr;

    if (evp_pkey != nullptr)
    {
        EVP_PKEY_free(evp_pkey);
        evp_pkey = nullptr;
    }

    X509_NAME_free(subject);
    subject = nullptr;

    X509_REQ_free(x509_req);

    CryptoPAL_logSSLError();
    return error;
}

} // namespace Crypto
} // namespace chip
