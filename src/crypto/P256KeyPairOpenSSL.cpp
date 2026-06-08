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

#include <limits>
#include <type_traits>

#if !CHIP_CRYPTO_BORINGSSL && defined(OPENSSL_VERSION_NUMBER) && OPENSSL_VERSION_NUMBER >= 0x30200000L
#include <openssl/core_names.h>
#include <openssl/params.h>
#elif CHIP_CRYPTO_BORINGSSL
#include <openssl/aead.h>
#endif

#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>

#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/BytesToHex.h>
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

// helper function to populate octet key into EVP_PKEY out_evp_pkey. Caller must free out_evp_pkey
static CHIP_ERROR _create_evp_key_from_binary_p256_key(const P256PublicKey & key, EVP_PKEY ** out_evp_pkey)
{

    CHIP_ERROR error = CHIP_NO_ERROR;
    EC_KEY * ec_key  = nullptr;
    int result       = -1;
    EC_POINT * point = nullptr;
    EC_GROUP * group = nullptr;
    int nid          = NID_undef;

    VerifyOrExit(*out_evp_pkey == nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);

    nid = GetNidForCurve(MapECName(key.Type()));
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

// Encode an ECDSA_SIG (r, s) pair as a raw P256ECDSASignature (r || s, each zero-padded to kP256_FE_Length).
static CHIP_ERROR encode_signature(const ECDSA_SIG * sig, P256ECDSASignature & out_signature)
{
    const BIGNUM * r = nullptr;
    const BIGNUM * s = nullptr;
    ECDSA_SIG_get0(sig, &r, &s); // sig keeps ownership of r and s
    VerifyOrReturnError((r != nullptr) && (s != nullptr), CHIP_ERROR_INTERNAL);

    VerifyOrReturnError(BN_bn2binpad(r, out_signature.Bytes(), kP256_FE_Length) == kP256_FE_Length, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(BN_bn2binpad(s, out_signature.Bytes() + kP256_FE_Length, kP256_FE_Length) == kP256_FE_Length,
                        CHIP_ERROR_INTERNAL);
    return out_signature.SetLength(kP256_ECDSA_Signature_Length_Raw);
}

CHIP_ERROR P256Keypair::ECDSA_sign_msg(const uint8_t * msg, const size_t msg_length, P256ECDSASignature & out_signature) const
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int nid          = NID_undef;
    EC_KEY * ec_key  = nullptr;
    ECDSA_SIG * sig  = nullptr;

    VerifyOrReturnError((msg != nullptr) && (msg_length > 0), CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t digest[kSHA256_Hash_Length];
    SHA256(msg, msg_length, digest);

    ERR_clear_error();

    static_assert(P256ECDSASignature::Capacity() >= kP256_ECDSA_Signature_Length_Raw, "P256ECDSASignature must be large enough");
    VerifyOrExit(mInitialized, error = CHIP_ERROR_UNINITIALIZED);
    nid = GetNidForCurve(MapECName(mPublicKey.Type()));
    VerifyOrExit(nid != NID_undef, error = CHIP_ERROR_INVALID_ARGUMENT);

    ec_key = to_EC_KEY(&mKeypair);
    VerifyOrExit(ec_key != nullptr, error = CHIP_ERROR_INTERNAL);

    sig = ECDSA_do_sign(Uint8::to_const_uchar(&digest[0]), static_cast<boringssl_size_t_openssl_int>(sizeof(digest)), ec_key);
    VerifyOrExit(sig != nullptr, error = CHIP_ERROR_INTERNAL);
    error = encode_signature(sig, out_signature);

exit:
    if (sig != nullptr)
    {
        ECDSA_SIG_free(sig);
    }

    if (error != CHIP_NO_ERROR)
    {
        SSLErrorLog();
    }

    return error;
}

#if !CHIP_CRYPTO_BORINGSSL && defined(OPENSSL_VERSION_NUMBER) && OPENSSL_VERSION_NUMBER >= 0x30200000L

// OpenSSL 3.2+ natively supports deterministic ECDSA (RFC 6979) via the EVP signing API.
CHIP_ERROR P256Keypair::ECDSA_sign_msg_det(const uint8_t * msg, size_t msg_length, P256ECDSASignature & out_signature) const
{
    VerifyOrReturnError((msg != nullptr) && (msg_length > 0), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mInitialized, CHIP_ERROR_UNINITIALIZED);

    ERR_clear_error();

    CHIP_ERROR error    = CHIP_NO_ERROR;
    EVP_MD_CTX * md_ctx = nullptr;
    EVP_PKEY * evp_pkey = nullptr;
    EC_KEY * ec_key     = nullptr;

    ec_key = EC_KEY_dup(to_const_EC_KEY(&mKeypair));
    VerifyOrExit(ec_key != nullptr, error = CHIP_ERROR_INTERNAL);

    evp_pkey = EVP_PKEY_new();
    VerifyOrExit(evp_pkey != nullptr, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(EVP_PKEY_set1_EC_KEY(evp_pkey, ec_key) == 1, error = CHIP_ERROR_INTERNAL);

    md_ctx = EVP_MD_CTX_new();
    VerifyOrExit(md_ctx != nullptr, error = CHIP_ERROR_INTERNAL);

    {
        // EVP_DigestSignInit sets pkey_ctx; it is owned by md_ctx and must not be freed separately.
        EVP_PKEY_CTX * pkey_ctx = nullptr;
        VerifyOrExit(EVP_DigestSignInit(md_ctx, &pkey_ctx, EVP_sha256(), nullptr, evp_pkey) == 1, error = CHIP_ERROR_INTERNAL);

        // Request deterministic nonce generation (RFC 6979).
        unsigned int nonce_type = 1;
        OSSL_PARAM params[]     = {
            OSSL_PARAM_construct_uint(OSSL_SIGNATURE_PARAM_NONCE_TYPE, &nonce_type),
            OSSL_PARAM_END,
        };
        VerifyOrExit(EVP_PKEY_CTX_set_params(pkey_ctx, params) == 1, error = CHIP_ERROR_INTERNAL);
    }

    // EVP_DigestSign produces a DER-encoded ECDSA signature; decode it to raw (r || s).
    {
        uint8_t der_sig[kP256_ECDSA_Signature_Length_Raw + 16]; // DER overhead is at most ~8 bytes for P-256
        size_t der_sig_len = sizeof(der_sig);
        VerifyOrExit(EVP_DigestSign(md_ctx, der_sig, &der_sig_len, msg, msg_length) == 1, error = CHIP_ERROR_INTERNAL);

        const uint8_t * p = der_sig;
        ECDSA_SIG * sig   = d2i_ECDSA_SIG(nullptr, &p, static_cast<long>(der_sig_len));
        VerifyOrExit(sig != nullptr, error = CHIP_ERROR_INTERNAL);
        error = encode_signature(sig, out_signature);
        ECDSA_SIG_free(sig);
    }

exit:
    if (md_ctx != nullptr)
    {
        EVP_MD_CTX_free(md_ctx);
    }
    if (evp_pkey != nullptr)
    {
        EVP_PKEY_free(evp_pkey);
    }
    if (ec_key != nullptr)
    {
        EC_KEY_free(ec_key);
    }
    if (error != CHIP_NO_ERROR)
    {
        SSLErrorLog();
    }
    return error;
}

#elif CHIP_CRYPTO_BORINGSSL

// BoringSSL removed RFC 6979 support, so we implement it manually:
// derive k per RFC 6979 section 3.2, then sign with an explicit nonce.

// Constant-time helper: returns (v != 0) ? ~0 : 0
static inline unsigned int ct_nonzero_mask(unsigned int v)
{
    return -((v | -v) >> (std::numeric_limits<decltype(v)>::digits - 1));
}

// Constant-time buffer comparison: returns (a >= b) ? ~0 : 0
// where a and b are interpreted as big-endian unsigned integers.
static unsigned int ct_buffer_gte(const volatile uint8_t * a, const volatile uint8_t * b, size_t n)
{
    // Loosely based on mbedtls_ct_memcmp() from third_party/mbedtls/repo/library/constant_time.c
    unsigned int diff = 0;
    for (size_t i = 0; i < n; i++)
    {
        unsigned int x = a[i], y = b[i];
        diff |= ~ct_nonzero_mask(diff) & (x - y);
    }

    // If a < b, the first byte difference will be negative, i.e. have MSB == 1,
    // so extract the MSB, spread it to all bits via unary minus, and invert.
    return ~-(diff >> (std::numeric_limits<decltype(diff)>::digits - 1));
}

// HMAC_K(V || sep || x || h1_reduced) -> K, used in RFC 6979 steps d and f.
static bool rfc6979_hmac_update_kv(uint8_t K[kSHA256_Hash_Length], const uint8_t V[kSHA256_Hash_Length], uint8_t sep,
                                   const uint8_t x[kP256_FE_Length], const uint8_t h1_reduced[kP256_FE_Length])
{
    HMAC_CTX * ctx = HMAC_CTX_new();
    VerifyOrReturnValue(ctx != nullptr, false);
    unsigned int len = kSHA256_Hash_Length;
    bool ok          = HMAC_Init_ex(ctx, K, kSHA256_Hash_Length, EVP_sha256(), nullptr) && //
        HMAC_Update(ctx, V, kSHA256_Hash_Length) &&                                        //
        HMAC_Update(ctx, &sep, 1) &&                                                       //
        HMAC_Update(ctx, x, kP256_FE_Length) &&                                            //
        HMAC_Update(ctx, h1_reduced, kP256_FE_Length) &&                                   //
        HMAC_Final(ctx, K, &len);
    HMAC_CTX_free(ctx);
    return ok;
}

// RFC 6979 section 3.2 - Deterministic k generation for ECDSA with HMAC-SHA256.
// For P-256 with SHA-256, qlen == hlen == 256 bits, so int2octets is the identity
// on 32-byte values. bits2octets requires reducing the hash modulo the group order.
//
// Derives a valid nonce k in [1, order) and writes it as 32 big-endian bytes to out_k.
// The private key x and hash h1 must each be exactly kP256_FE_Length bytes.
static CHIP_ERROR rfc6979_derive_k(const EC_KEY * ec_key, const uint8_t x[kP256_FE_Length], const uint8_t h1[kSHA256_Hash_Length],
                                   uint8_t out_k[kP256_FE_Length])
{
    unsigned int success = 0;
    CHIP_ERROR error     = CHIP_NO_ERROR;
    memset(out_k, 0x00, kP256_FE_Length);

    // Serialize the group order for constant-time comparison in the candidate loop below.
    uint8_t order[kP256_FE_Length];
    const EC_GROUP * group  = EC_KEY_get0_group(ec_key);
    const BIGNUM * order_bn = EC_GROUP_get0_order(group);
    VerifyOrReturnError(group != nullptr && order_bn != nullptr, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(BN_bn2binpad(order_bn, order, sizeof(order)) == sizeof(order), CHIP_ERROR_INTERNAL);

    BN_CTX * bn_ctx = BN_CTX_new();
    VerifyOrReturnError(bn_ctx != nullptr, CHIP_ERROR_INTERNAL);
    BN_CTX_start(bn_ctx);

    // bits2octets(h1): reduce hash modulo the group order
    uint8_t h1_reduced[kP256_FE_Length];
    {
        BIGNUM * h1_bn = BN_CTX_get(bn_ctx);
        VerifyOrExit(h1_bn != nullptr, error = CHIP_ERROR_INTERNAL);
        VerifyOrExit(BN_bin2bn(h1, kSHA256_Hash_Length, h1_bn) != nullptr, error = CHIP_ERROR_INTERNAL);
        VerifyOrExit(BN_nnmod(h1_bn, h1_bn, order_bn, bn_ctx) == 1, error = CHIP_ERROR_INTERNAL);
        VerifyOrExit(BN_bn2binpad(h1_bn, h1_reduced, sizeof(h1_reduced)) == sizeof(h1_reduced), error = CHIP_ERROR_INTERNAL);
    }

    // Steps b-c: initialize V and K
    uint8_t V[kSHA256_Hash_Length];
    memset(V, 0x01, sizeof(V));

    uint8_t K[kSHA256_Hash_Length];
    memset(K, 0x00, sizeof(K));

    // Step d: K = HMAC_K(V || 0x00 || int2octets(x) || bits2octets(h1))
    VerifyOrExit(rfc6979_hmac_update_kv(K, V, 0x00, x, h1_reduced), error = CHIP_ERROR_INTERNAL);

    // Step e: V = HMAC_K(V)
    VerifyOrExit(HMAC(EVP_sha256(), K, sizeof(K), V, sizeof(V), V, nullptr) != nullptr, error = CHIP_ERROR_INTERNAL);

    // Step f: K = HMAC_K(V || 0x01 || int2octets(x) || bits2octets(h1))
    VerifyOrExit(rfc6979_hmac_update_kv(K, V, 0x01, x, h1_reduced), error = CHIP_ERROR_INTERNAL);

    // Step g: V = HMAC_K(V)
    VerifyOrExit(HMAC(EVP_sha256(), K, sizeof(K), V, sizeof(V), V, nullptr) != nullptr, error = CHIP_ERROR_INTERNAL);

    // Step h: Generate candidate k, retrying if k is not in [1, order).
    // For P-256/SHA-256, one HMAC output (256 bits) == qlen, so one iteration
    // of the inner loop suffices to produce a candidate. bits2int is the identity
    // since qlen == hlen.
    // This logic needs to be constant-time to avoid leaking information about k
    // from either the numeric comparison operation to the order, or via the
    // number of candidates that had to be tested. The probability of rejection is
    // about 1 in 2^32 per round, so with our limit of 8 rounds, the chance of
    // failure is approximately 1 in 2^256 (10^77).
    for (int attempt = 0; attempt < 8; attempt++)
    {
        // Generate candidate: V = HMAC_K(V)
        VerifyOrExit(HMAC(EVP_sha256(), K, sizeof(K), V, sizeof(V), V, nullptr) != nullptr, error = CHIP_ERROR_INTERNAL);

        // V is our result if it's in range [1, order) and we weren't already successful.
        // This is equivalent to !success && !(0 >= V) && !(V >= order)
        // Instead of a conditional memcpy(out_k, V) bitwise or everything together with the result mask.
        constexpr auto zero = FixedByteSpan<kP256_FE_Length>().data(); // shared buffer of zeroes
        unsigned int result = ~success & ~ct_buffer_gte(zero, V, kP256_FE_Length) & ~ct_buffer_gte(V, order, kP256_FE_Length);
        for (size_t b = 0; b < kP256_FE_Length; b++)
        {
            out_k[b] |= V[b] & result;
        }
        success |= result;

        // RFC 6979 step h.3: K = HMAC_K(V || 0x00), V = HMAC_K(V)
        {
            HMAC_CTX * ctx = HMAC_CTX_new();
            VerifyOrExit(ctx != nullptr, error = CHIP_ERROR_INTERNAL);
            unsigned int len = sizeof(K);
            uint8_t sep      = 0x00;
            bool ok          = HMAC_Init_ex(ctx, K, sizeof(K), EVP_sha256(), nullptr) && //
                HMAC_Update(ctx, V, sizeof(V)) &&                                        //
                HMAC_Update(ctx, &sep, 1) &&                                             //
                HMAC_Final(ctx, K, &len);
            HMAC_CTX_free(ctx);
            VerifyOrExit(ok, error = CHIP_ERROR_INTERNAL);
        }
        VerifyOrExit(HMAC(EVP_sha256(), K, sizeof(K), V, sizeof(V), V, nullptr) != nullptr, error = CHIP_ERROR_INTERNAL);
    }

    VerifyOrExit(success, error = CHIP_ERROR_INTERNAL);

exit:
    ClearSecretData(V);
    ClearSecretData(K);
    ClearSecretData(h1_reduced);
    BN_CTX_end(bn_ctx);
    BN_CTX_free(bn_ctx);
    return error;
}

CHIP_ERROR P256Keypair::ECDSA_sign_msg_det(const uint8_t * msg, size_t msg_length, P256ECDSASignature & out_signature) const
{
    VerifyOrReturnError((msg != nullptr) && (msg_length > 0), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mInitialized, CHIP_ERROR_UNINITIALIZED);

    ERR_clear_error();

    CHIP_ERROR error = CHIP_NO_ERROR;
    ECDSA_SIG * sig  = nullptr;

    // Step a: h1 = Hash(msg)
    uint8_t h1[kSHA256_Hash_Length];
    SHA256(msg, msg_length, h1);

    const EC_KEY * ec_key = to_const_EC_KEY(&mKeypair);
    VerifyOrExit(ec_key != nullptr, error = CHIP_ERROR_INTERNAL);

    // Extract private key as big-endian bytes: int2octets(x)
    uint8_t x[kP256_FE_Length];
    {
        const BIGNUM * privkey_bn = EC_KEY_get0_private_key(ec_key);
        VerifyOrExit(privkey_bn != nullptr, error = CHIP_ERROR_INTERNAL);
        VerifyOrExit(BN_bn2binpad(privkey_bn, x, sizeof(x)) == sizeof(x), error = CHIP_ERROR_INTERNAL);
    }

    // Derive deterministic nonce k via RFC 6979
    uint8_t k[kP256_FE_Length];
    SuccessOrExit((error = rfc6979_derive_k(ec_key, x, h1, k)));

    // Sign using the deterministic nonce k derived above.
    // Despite its alarming name, ECDSA_sign_with_nonce_and_leak_private_key_for_testing is just a
    // standard ECDSA sign that accepts a caller-supplied nonce instead of generating one internally.
    // This testing hook is the only way to supply a pre-computed nonce. The "leak_private_key" risk
    // it guards against are scenarios where a caller reuses a nonce; here we provide a nonce that is
    // securely derived from the private key and message as per RFC 6979.
    sig = ECDSA_sign_with_nonce_and_leak_private_key_for_testing(h1, sizeof(h1), ec_key, k, sizeof(k));
    VerifyOrExit(sig != nullptr, error = CHIP_ERROR_INTERNAL);
    error = encode_signature(sig, out_signature);

exit:
    ClearSecretData(x);
    ClearSecretData(k);
    if (sig != nullptr)
    {
        ECDSA_SIG_free(sig);
    }
    if (error != CHIP_NO_ERROR)
    {
        SSLErrorLog();
    }
    return error;
}
#endif // CHIP_CRYPTO_BORINGSSL || OPENSSL_VERSION_NUMBER >= 0x30200000L

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

    SSLErrorLog();
    return error;
}

CHIP_ERROR P256Keypair::Initialize(ECPKeyTarget key_target)
{
    ERR_clear_error();

    Clear();

    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;
    EC_KEY * ec_key  = nullptr;
    ECName curve     = MapECName(mPublicKey.Type());

    int nid = GetNidForCurve(curve);
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

    SSLErrorLog();
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
        SuccessOrExit(error = output.SetLength(bbuf.Needed()));
    }

exit:
    ClearSecretData(privkey, sizeof(privkey));
    SSLErrorLog();
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
    ECName curve    = MapECName(mPublicKey.Type());

    ERR_clear_error();
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;
    int nid          = NID_undef;

    const uint8_t * privkey = input.ConstBytes() + mPublicKey.Length();

    VerifyOrExit(input.Length() == mPublicKey.Length() + kP256_PrivateKey_Length, error = CHIP_ERROR_INVALID_ARGUMENT);
    bbuf.Put(input.ConstBytes(), mPublicKey.Length());
    VerifyOrExit(bbuf.Fit(), error = CHIP_ERROR_NO_MEMORY);

    nid = GetNidForCurve(curve);
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
        BN_clear_free(pvt_key); // wipe!
        pvt_key = nullptr;
    }

    if (key_point != nullptr)
    {
        EC_POINT_free(key_point);
        key_point = nullptr;
    }
    SSLErrorLog();
    return error;
}

CHIP_ERROR P256Keypair::InitializeFromBitsOrReject(FixedByteSpan<kP256_PrivateKey_Length> privateKeyBits)
{
    ERR_clear_error();
    Clear();

    const EC_GROUP * group;
    const BIGNUM * order;
    BIGNUM * pvt_key     = nullptr;
    EC_POINT * pub_point = nullptr;
    EC_KEY * ec_key      = nullptr;

    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    int nid = GetNidForCurve(MapECName(mPublicKey.Type()));
    VerifyOrExit(nid != NID_undef, error = CHIP_ERROR_INTERNAL);

    ec_key = EC_KEY_new_by_curve_name(nid);
    VerifyOrExit(ec_key != nullptr, error = CHIP_ERROR_INTERNAL);

    group = EC_KEY_get0_group(ec_key);
    order = EC_GROUP_get0_order(group);

    // Convert private key bits to BIGNUM x, then compute d = x + 1.
    // Checking that d < order ensures that x was in [0, order - 2].
    pvt_key = BN_bin2bn(privateKeyBits.data(), privateKeyBits.size(), nullptr);
    VerifyOrExit(pvt_key != nullptr, error = CHIP_ERROR_INTERNAL);
    result = BN_add(pvt_key, pvt_key, BN_value_one());
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(BN_cmp(pvt_key, order) < 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    // Compute public key: Q = d * G
    pub_point = EC_POINT_new(group);
    VerifyOrExit(pub_point != nullptr, error = CHIP_ERROR_INTERNAL);

    result = EC_POINT_mul(group, pub_point, pvt_key, nullptr, nullptr, nullptr);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    result = EC_KEY_set_private_key(ec_key, pvt_key);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    result = EC_KEY_set_public_key(ec_key, pub_point);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    SuccessOrExit(error = P256PublicKeyFromECKey(ec_key, mPublicKey));

    from_EC_KEY(ec_key, &mKeypair);
    mInitialized = true;
    ec_key       = nullptr;

exit:
    if (ec_key != nullptr)
    {
        EC_KEY_free(ec_key);
        ec_key = nullptr;
    }

    if (pub_point != nullptr)
    {
        EC_POINT_free(pub_point);
        pub_point = nullptr;
    }

    if (pvt_key != nullptr)
    {
        BN_clear_free(pvt_key); // wipe!
        pvt_key = nullptr;
    }

    SSLErrorLog();
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

    SSLErrorLog();
    return error;
}

} // namespace Crypto
} // namespace chip
