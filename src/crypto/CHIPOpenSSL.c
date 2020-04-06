// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "CHIPCrypto.h"

#include <nlassert.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/kdf.h>
#include <openssl/srp.h>
#include <openssl/rand.h>
#include <stdbool.h>

static void hash_init(EVP_MD_CTX ** ctx, const EVP_MD * type)
{
    *ctx    = EVP_MD_CTX_create();
    int ret = EVP_DigestInit_ex(*ctx, type, NULL);
    nlASSERT(ret == 1);
}

static void hash_update(EVP_MD_CTX ** ctx, const uint8_t * data, size_t size)
{
    int ret = EVP_DigestUpdate(*ctx, data, size);
    nlASSERT(ret == 1);
}

static void hash_final(EVP_MD_CTX ** ctx, uint8_t * md)
{
    int ret = EVP_DigestFinal_ex(*ctx, md, NULL);
    nlASSERT(ret == 1);
    EVP_MD_CTX_destroy(*ctx);
    *ctx = NULL;
}

#define WITH(type, name, init, free, X)                                                                                            \
    do                                                                                                                             \
    {                                                                                                                              \
        type * name = init;                                                                                                        \
        X;                                                                                                                         \
        free(name);                                                                                                                \
    } while (0)

#define WITH_PKEY(name, init, X) WITH(EVP_PKEY, name, init, EVP_PKEY_free, X)

void CHIP_ed25519_public_key(uint8_t pk[ED25519_PUBLIC_KEY_BYTES], const uint8_t sk[ED25519_SECRET_KEY_BYTES])
{
    WITH_PKEY(key, EVP_PKEY_new_raw_private_key(EVP_PKEY_ED25519, NULL, sk, ED25519_SECRET_KEY_BYTES), {
        size_t len = ED25519_PUBLIC_KEY_BYTES;
        int ret    = EVP_PKEY_get_raw_public_key(key, pk, &len);
        nlASSERT(ret == 1 && len == ED25519_SECRET_KEY_BYTES);
    });
}

#define WITH_CTX(type, init, X) WITH(type, ctx, init, type##_free, X)

void CHIP_ed25519_sign(uint8_t sig[ED25519_BYTES], const uint8_t * m, size_t m_len, const uint8_t sk[ED25519_SECRET_KEY_BYTES],
                       const uint8_t pk[ED25519_PUBLIC_KEY_BYTES])
{
    WITH_PKEY(key, EVP_PKEY_new_raw_private_key(EVP_PKEY_ED25519, NULL, sk, ED25519_SECRET_KEY_BYTES), {
        WITH_CTX(EVP_MD_CTX, EVP_MD_CTX_new(), {
            int ret = EVP_DigestSignInit(ctx, NULL, NULL, NULL, key);
            nlASSERT(ret == 1);
            size_t len = ED25519_BYTES;
            EVP_DigestSign(ctx, sig, &len, m, m_len);
            nlASSERT(len == ED25519_BYTES);
        });
    });
}

int CHIP_ed25519_verify(const uint8_t sig[ED25519_BYTES], const uint8_t * m, size_t m_len,
                        const uint8_t pk[ED25519_PUBLIC_KEY_BYTES])
{
    int ret;
    WITH_PKEY(key, EVP_PKEY_new_raw_public_key(EVP_PKEY_ED25519, NULL, pk, ED25519_PUBLIC_KEY_BYTES), {
        WITH_CTX(EVP_MD_CTX, EVP_MD_CTX_new(), {
            ret = EVP_DigestVerifyInit(ctx, NULL, NULL, NULL, key);
            nlASSERT(ret == 1);
            ret = EVP_DigestVerify(ctx, sig, ED25519_BYTES, m, m_len);
        });
    });
    return (ret == 1) ? 0 : -1;
}

void CHIP_X25519_scalarmult_base(uint8_t r[X25519_BYTES], const uint8_t n[X25519_SCALAR_BYTES])
{
    WITH_PKEY(key, EVP_PKEY_new_raw_private_key(EVP_PKEY_X25519, NULL, n, X25519_SCALAR_BYTES), {
        size_t len = X25519_BYTES;
        int ret    = EVP_PKEY_get_raw_public_key(key, r, &len);
        nlASSERT(ret == 1 && len == X25519_BYTES);
    });
}

void CHIP_X25519_scalarmult(uint8_t r[X25519_BYTES], const uint8_t n[X25519_SCALAR_BYTES], const uint8_t p[X25519_BYTES])
{
    WITH_PKEY(pkey, EVP_PKEY_new_raw_private_key(EVP_PKEY_X25519, NULL, n, X25519_SCALAR_BYTES), {
        WITH_PKEY(peer, EVP_PKEY_new_raw_public_key(EVP_PKEY_X25519, NULL, p, X25519_BYTES), {
            WITH_CTX(EVP_PKEY_CTX, EVP_PKEY_CTX_new(pkey, NULL), {
                int ret = EVP_PKEY_derive_init(ctx);
                nlASSERT(ret == 1);
                ret = EVP_PKEY_derive_set_peer(ctx, peer);
                nlASSERT(ret == 1);
                size_t r_len;
                ret = EVP_PKEY_derive(ctx, r, &r_len);
                nlASSERT(ret == 1 && r_len == X25519_BYTES);
            });
        });
    });
}

// OpenSSL doesn't support SRP6a with SHA512 so we have to do this on foot

static void Calc_x(uint8_t x[SHA512_BYTES], const uint8_t salt[SRP_SALT_BYTES], const uint8_t * user, size_t user_len,
                   const uint8_t * pass, size_t pass_len)
{
    EVP_MD_CTX * ctx;

    hash_init(&ctx, EVP_sha512());
    hash_update(&ctx, user, user_len);
    hash_update(&ctx, (const uint8_t *) ":", 1);
    hash_update(&ctx, pass, pass_len);
    hash_final(&ctx, x);

    hash_init(&ctx, EVP_sha512());
    hash_update(&ctx, salt, SRP_SALT_BYTES);
    hash_update(&ctx, x, SHA512_BYTES);
    hash_final(&ctx, x);
}

static SRP_gN * Get_gN_3072()
{
    static SRP_gN * gN = NULL;
    if (!gN)
    {
        gN = SRP_get_default_gN("3072");
    }
    return gN;
}

#define WITH_BN(name, init, X) WITH(BIGNUM, name, init, BN_clear_free, X)

void CHIP_srp_verifier(uint8_t v[SRP_VERIFIER_BYTES], const uint8_t salt[SRP_SALT_BYTES], const uint8_t * user, size_t user_len,
                       const uint8_t * pass, size_t pass_len)
{
    uint8_t h[SHA512_BYTES];
    Calc_x(h, salt, user, user_len, pass, pass_len);
    WITH_BN(x, BN_bin2bn(h, sizeof h, NULL), {
        WITH_BN(verifier, BN_new(), {
            SRP_gN * gN = Get_gN_3072();
            WITH_CTX(BN_CTX, BN_CTX_new(), {
                int ret = BN_mod_exp(verifier, gN->g, x, gN->N, ctx);
                nlASSERT(!!ret);
            });
            int ret = BN_bn2binpad(verifier, v, SRP_VERIFIER_BYTES);
            nlASSERT(ret == SRP_VERIFIER_BYTES);
        });
    });
}

static BIGNUM * Calc_k(SRP_gN * gN)
{
    uint8_t N[SRP_PRIME_BYTES];
    int ret = BN_bn2binpad(gN->N, N, sizeof N);
    nlASSERT(ret == sizeof N);
    uint8_t g[SRP_PRIME_BYTES];
    ret = BN_bn2binpad(gN->g, g, sizeof g);
    nlASSERT(ret == sizeof g);
    uint8_t k[SHA512_BYTES];
    EVP_MD_CTX * ctx;
    hash_init(&ctx, EVP_sha512());
    hash_update(&ctx, N, SRP_PRIME_BYTES);
    hash_update(&ctx, g, SRP_PRIME_BYTES);
    hash_final(&ctx, k);
    return BN_bin2bn(k, sizeof k, NULL);
}

static BIGNUM * Calc_B(BIGNUM * b, BIGNUM * v)
{
    SRP_gN * gN = Get_gN_3072();
    BIGNUM * B  = BN_new();
    WITH_CTX(BN_CTX, BN_CTX_new(), {
        WITH_BN(gb, BN_new(), {
            int ret = BN_mod_exp(gb, gN->g, b, gN->N, ctx);
            nlASSERT(!!ret);
            WITH_BN(k, Calc_k(gN), {
                WITH_BN(kv, BN_new(), {
                    ret = BN_mod_mul(kv, v, k, gN->N, ctx);
                    nlASSERT(!!ret);
                    ret = BN_mod_add(B, gb, kv, gN->N, ctx);
                    nlASSERT(!!ret);
                });
            });
        });
    });
    return B;
}

void CHIP_srp_public_key(uint8_t pub_b[SRP_PUBLIC_KEY_BYTES], const uint8_t priv_b[SRP_SECRET_KEY_BYTES],
                         const uint8_t v[SRP_VERIFIER_BYTES])
{
    WITH_BN(b, BN_bin2bn(priv_b, SRP_SECRET_KEY_BYTES, NULL), {
        WITH_BN(verifier, BN_bin2bn(v, SRP_VERIFIER_BYTES, NULL), {
            WITH_BN(B, Calc_B(b, verifier), {
                int ret = BN_bn2binpad(B, pub_b, SRP_PUBLIC_KEY_BYTES);
                nlASSERT(ret == SRP_PUBLIC_KEY_BYTES);
            });
        });
    });
}

void CHIP_srp_scrambling_parameter(uint8_t u[SRP_SCRAMBLING_PARAMETER_BYTES], const uint8_t pub_a[SRP_PUBLIC_KEY_BYTES],
                                   const uint8_t pub_b[SRP_PUBLIC_KEY_BYTES])
{
    EVP_MD_CTX * ctx;
    hash_init(&ctx, EVP_sha512());
    hash_update(&ctx, pub_a, SRP_PUBLIC_KEY_BYTES);
    hash_update(&ctx, pub_b, SRP_PUBLIC_KEY_BYTES);
    hash_final(&ctx, u);
}

int CHIP_srp_premaster_secret(uint8_t s[SRP_PREMASTER_SECRET_BYTES], const uint8_t pub_a[SRP_PUBLIC_KEY_BYTES],
                              const uint8_t priv_b[SRP_SECRET_KEY_BYTES], const uint8_t u[SRP_SCRAMBLING_PARAMETER_BYTES],
                              const uint8_t v[SRP_VERIFIER_BYTES])
{
    bool isAValid = false;
    WITH_BN(A, BN_bin2bn(pub_a, SRP_PUBLIC_KEY_BYTES, NULL), {
        // Refer RFC 5054: https://tools.ietf.org/html/rfc5054
        // Section 2.5.4
        // Fail if A%N == 0
        WITH_CTX(BN_CTX, BN_CTX_new(), {
            WITH_BN(rem, BN_new(), {
                int ret = BN_nnmod(rem, A, Get_gN_3072()->N, ctx);
                nlASSERT(!!ret);
                if (BN_is_zero(rem) == 0)
                {
                    isAValid = true;
                }
            });
        });

        WITH_BN(b, BN_bin2bn(priv_b, SRP_SECRET_KEY_BYTES, NULL), {
            WITH_BN(u_, BN_bin2bn(u, SRP_SCRAMBLING_PARAMETER_BYTES, NULL), {
                WITH_BN(v_, BN_bin2bn(v, SRP_VERIFIER_BYTES, NULL), {
                    WITH_BN(s_, SRP_Calc_server_key(A, v_, u_, b, Get_gN_3072()->N), {
                        int ret = BN_bn2binpad(s_, s, SRP_PREMASTER_SECRET_BYTES);
                        nlASSERT(ret == SRP_PREMASTER_SECRET_BYTES);
                    });
                });
            });
        });
    });
    return (isAValid) ? 0 : 1;
}

static size_t Count_Leading_Zeroes(const uint8_t * start, size_t n)
{
    const uint8_t * p    = start;
    const uint8_t * stop = start + n;
    while (p < stop && !*p)
    {
        p++;
    }
    return p - start;
}

void CHIP_srp_session_key(uint8_t k[SRP_SESSION_KEY_BYTES], const uint8_t s[SRP_PREMASTER_SECRET_BYTES])
{
    size_t z = Count_Leading_Zeroes(s, SRP_PREMASTER_SECRET_BYTES);
    CHIP_sha512(k, s + z, SRP_PREMASTER_SECRET_BYTES - z);
}

static void Xor(int * x, const int * a, const int * b, size_t n)
{
    while (n-- > 0)
    {
        *x++ = *a++ ^ *b++;
    }
}

void CHIP_srp_proof_m1(uint8_t m1[SRP_PROOF_BYTES], const uint8_t * user, size_t user_len, const uint8_t salt[SRP_SALT_BYTES],
                       const uint8_t pub_a[SRP_PUBLIC_KEY_BYTES], const uint8_t pub_b[SRP_PUBLIC_KEY_BYTES],
                       const uint8_t k[SRP_SESSION_KEY_BYTES])
{
    SRP_gN * gN = Get_gN_3072();
    uint8_t N[SRP_PRIME_BYTES];
    int ret = BN_bn2binpad(gN->N, N, SRP_PRIME_BYTES);
    nlASSERT(ret == SRP_PRIME_BYTES);
    uint8_t g[1];
    ret = BN_bn2binpad(gN->g, g, sizeof g);
    nlASSERT(ret == sizeof g);
    uint8_t H_N[SHA512_BYTES];
    CHIP_sha512(H_N, N, sizeof N);
    uint8_t H_g[SHA512_BYTES];
    CHIP_sha512(H_g, g, sizeof g);
    uint8_t H_Ng[SHA512_BYTES];
    Xor((int *) H_Ng, (const int *) H_N, (const int *) H_g, SHA512_BYTES / sizeof(int));
    uint8_t H_U[SHA512_BYTES];
    CHIP_sha512(H_U, user, user_len);
    size_t z_A = Count_Leading_Zeroes(pub_a, SRP_PUBLIC_KEY_BYTES);
    size_t z_B = Count_Leading_Zeroes(pub_b, SRP_PUBLIC_KEY_BYTES);
    EVP_MD_CTX * ctx;
    hash_init(&ctx, EVP_sha512());
    hash_update(&ctx, H_Ng, sizeof H_Ng);
    hash_update(&ctx, H_U, sizeof H_U);
    hash_update(&ctx, salt, SRP_SALT_BYTES);
    hash_update(&ctx, pub_a + z_A, SRP_PUBLIC_KEY_BYTES - z_A);
    hash_update(&ctx, pub_b + z_B, SRP_PUBLIC_KEY_BYTES - z_B);
    hash_update(&ctx, k, SRP_SESSION_KEY_BYTES);
    hash_final(&ctx, m1);
}

void CHIP_srp_proof_m2(uint8_t m2[SRP_PROOF_BYTES], const uint8_t pub_a[SRP_PUBLIC_KEY_BYTES], const uint8_t m1[SRP_PROOF_BYTES],
                       const uint8_t k[SRP_SESSION_KEY_BYTES])
{
    EVP_MD_CTX * ctx;
    hash_init(&ctx, EVP_sha512());
    hash_update(&ctx, pub_a, SRP_PUBLIC_KEY_BYTES);
    hash_update(&ctx, m1, SRP_PROOF_BYTES);
    hash_update(&ctx, k, SRP_SESSION_KEY_BYTES);
    hash_final(&ctx, m2);
}

static void hash(const EVP_MD * type, uint8_t * md, const uint8_t * data, size_t size)
{
    EVP_MD_CTX * ctx;
    hash_init(&ctx, type);
    hash_update(&ctx, data, size);
    hash_final(&ctx, md);
}

void CHIP_sha1(uint8_t md[SHA1_BYTES], const uint8_t * data, size_t size)
{
    hash(EVP_sha1(), md, data, size);
}

void CHIP_sha256(uint8_t md[SHA256_BYTES], const uint8_t * data, size_t size)
{
    hash(EVP_sha256(), md, data, size);
}

void CHIP_sha512(uint8_t md[SHA512_BYTES], const uint8_t * data, size_t size)
{
    hash(EVP_sha512(), md, data, size);
}

void CHIP_hmac_sha1_aad(uint8_t r[HMAC_SHA1_BYTES], const uint8_t * key, size_t key_len, const uint8_t * in, size_t in_len,
                        const uint8_t * aad, size_t aad_len)
{
    WITH_CTX(HMAC_CTX, HMAC_CTX_new(), {
        int ret = HMAC_Init_ex(ctx, key, key_len, EVP_sha1(), NULL);
        nlASSERT(ret == 1);
        ret = HMAC_Update(ctx, in, in_len);
        nlASSERT(ret == 1);
        ret = HMAC_Update(ctx, aad, aad_len);
        nlASSERT(ret == 1);
        unsigned int r_len = HMAC_SHA1_BYTES;
        ret                = HMAC_Final(ctx, r, &r_len);
        nlASSERT(ret == 1 && r_len == HMAC_SHA1_BYTES);
    });
}

void CHIP_hkdf_sha512(uint8_t * r, size_t r_len, const uint8_t * key, size_t key_len, const uint8_t * salt, size_t salt_len,
                      const uint8_t * info, size_t info_len)
{
    WITH_CTX(EVP_PKEY_CTX, EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, NULL), {
        int ret = EVP_PKEY_derive_init(ctx);
        nlASSERT(ret == 1);
        ret = EVP_PKEY_CTX_set_hkdf_md(ctx, EVP_sha512());
        nlASSERT(ret == 1);
        ret = EVP_PKEY_CTX_set1_hkdf_salt(ctx, salt, salt_len);
        nlASSERT(ret == 1);
        ret = EVP_PKEY_CTX_set1_hkdf_key(ctx, key, key_len);
        nlASSERT(ret == 1);
        ret = EVP_PKEY_CTX_add1_hkdf_info(ctx, info, info_len);
        nlASSERT(ret == 1);
        size_t out_len = r_len;
        ret            = EVP_PKEY_derive(ctx, r, &out_len);
        nlASSERT(ret == 1 && out_len == r_len);
    });
}

void CHIP_pbkdf2_hmac_sha1(uint8_t * key, size_t key_len, const uint8_t * password, size_t password_len, const uint8_t * salt,
                           size_t salt_len, uint32_t count)
{
    PKCS5_PBKDF2_HMAC_SHA1((const char *) password, password_len, salt, salt_len, count, key_len, key);
}

typedef struct
{
    EVP_CIPHER_CTX * ctx;
} EVP_CIPHER_CTX_Handle;

CHIP_STATIC_ASSERT(sizeof(CHIP_chacha20_poly1305_ctx) >= sizeof(EVP_CIPHER_CTX_Handle), CHIP_chacha20_poly1305_ctx);

// OpenSSL doesn't like overlapping in/out buffers in EVP_EncryptUpdate/EVP_DecryptUpdate
static bool is_overlapping(const uint8_t * a, const uint8_t * b, size_t n)
{
    return (a < b && a + n > b) || (b < a && b + n > a);
}

static uint8_t * use_temporary_if_overlapping(uint8_t ** tmp, const uint8_t * in, uint8_t * out, size_t n)
{
    *tmp = NULL;
    if (is_overlapping(in, out, n))
    {
        return *tmp = malloc(n);
    }
    return out;
}

static void copy_and_free_if_overlapping(uint8_t ** tmp, uint8_t ** out, size_t n)
{
    if (*tmp)
    {
        memcpy(*out, *tmp, n);
        free(*tmp);
        *tmp = NULL;
    }
}

// CHIP:
void CHIP_chacha20_poly1305_init(CHIP_chacha20_poly1305_ctx * ctx, const uint8_t * n, size_t n_len,
                                 const uint8_t k[CHACHA20_POLY1305_KEY_BYTES])
{
    EVP_CIPHER_CTX_Handle * handle = (EVP_CIPHER_CTX_Handle *) ctx;
    handle->ctx                    = NULL;
}

void CHIP_chacha20_poly1305_update_enc(CHIP_chacha20_poly1305_ctx * ctx, uint8_t * c, const uint8_t * m, size_t m_len,
                                       const uint8_t * n, size_t n_len, const uint8_t k[CHACHA20_POLY1305_KEY_BYTES])
{
    EVP_CIPHER_CTX_Handle * handle = (EVP_CIPHER_CTX_Handle *) ctx;
    int ret;
    if (!handle->ctx)
    {
        handle->ctx = EVP_CIPHER_CTX_new();
        ret         = EVP_EncryptInit_ex(handle->ctx, EVP_chacha20_poly1305(), 0, 0, 0);
        nlASSERT(ret == 1);
        ret = EVP_CIPHER_CTX_ctrl(handle->ctx, EVP_CTRL_AEAD_SET_TAG, CHACHA20_POLY1305_TAG_BYTES, NULL);
        nlASSERT(ret == 1);
        ret = EVP_CIPHER_CTX_ctrl(handle->ctx, EVP_CTRL_AEAD_SET_IVLEN, n_len, NULL);
        nlASSERT(ret == 1);
        ret = EVP_EncryptInit_ex(handle->ctx, NULL, NULL, k, n);
        nlASSERT(ret == 1);
    }
    if (m_len > 0)
    {
        uint8_t * tmp;
        int c_len;
        ret = EVP_EncryptUpdate(handle->ctx, use_temporary_if_overlapping(&tmp, m, c, m_len), &c_len, m, m_len);
        copy_and_free_if_overlapping(&tmp, &c, m_len);
        nlASSERT(ret == 1 && c_len == m_len);
    }
}

void CHIP_chacha20_poly1305_update_enc_aad(CHIP_chacha20_poly1305_ctx * ctx, const uint8_t * a, size_t a_len, const uint8_t * n,
                                           size_t n_len, const uint8_t k[CHACHA20_POLY1305_KEY_BYTES])
{
    CHIP_chacha20_poly1305_update_enc(ctx, NULL, NULL, 0, n, n_len, k);
    EVP_CIPHER_CTX_Handle * handle = (EVP_CIPHER_CTX_Handle *) ctx;
    int a_out;
    int ret = EVP_EncryptUpdate(handle->ctx, NULL, &a_out, a, a_len);
    nlASSERT(ret == 1 && a_out == a_len);
}

void CHIP_chacha20_poly1305_final_enc(CHIP_chacha20_poly1305_ctx * ctx, uint8_t tag[CHACHA20_POLY1305_TAG_BYTES])
{
    EVP_CIPHER_CTX_Handle * handle = (EVP_CIPHER_CTX_Handle *) ctx;
    int c_len;
    int ret = EVP_EncryptFinal_ex(handle->ctx, NULL, &c_len);
    nlASSERT(ret == 1 && !c_len);
    ret = EVP_CIPHER_CTX_ctrl(handle->ctx, EVP_CTRL_AEAD_GET_TAG, CHACHA20_POLY1305_TAG_BYTES, tag);
    nlASSERT(ret == 1);
    EVP_CIPHER_CTX_free(handle->ctx);
    handle->ctx = NULL;
}

void CHIP_chacha20_poly1305_update_dec(CHIP_chacha20_poly1305_ctx * ctx, uint8_t * m, const uint8_t * c, size_t c_len,
                                       const uint8_t * n, size_t n_len, const uint8_t k[CHACHA20_POLY1305_KEY_BYTES])
{
    EVP_CIPHER_CTX_Handle * handle = (EVP_CIPHER_CTX_Handle *) ctx;
    int ret;
    if (!handle->ctx)
    {
        handle->ctx = EVP_CIPHER_CTX_new();
        ret         = EVP_DecryptInit_ex(handle->ctx, EVP_chacha20_poly1305(), 0, 0, 0);
        nlASSERT(ret == 1);
        ret = EVP_CIPHER_CTX_ctrl(handle->ctx, EVP_CTRL_AEAD_SET_IVLEN, n_len, NULL);
        nlASSERT(ret == 1);
        ret = EVP_DecryptInit_ex(handle->ctx, NULL, NULL, k, n);
        nlASSERT(ret == 1);
    }
    if (c_len > 0)
    {
        uint8_t * tmp;
        int m_len;
        ret = EVP_DecryptUpdate(handle->ctx, use_temporary_if_overlapping(&tmp, c, m, c_len), &m_len, c, c_len);
        copy_and_free_if_overlapping(&tmp, &m, c_len);
        nlASSERT(ret == 1);
    }
}

void CHIP_chacha20_poly1305_update_dec_aad(CHIP_chacha20_poly1305_ctx * ctx, const uint8_t * a, size_t a_len, const uint8_t * n,
                                           size_t n_len, const uint8_t k[CHACHA20_POLY1305_KEY_BYTES])
{
    CHIP_chacha20_poly1305_update_dec(ctx, NULL, NULL, 0, n, n_len, k);
    EVP_CIPHER_CTX_Handle * handle = (EVP_CIPHER_CTX_Handle *) ctx;
    int a_out;
    int ret = EVP_DecryptUpdate(handle->ctx, NULL, &a_out, a, a_len);
    nlASSERT(ret == 1 && a_out == a_len);
}

int CHIP_chacha20_poly1305_final_dec(CHIP_chacha20_poly1305_ctx * ctx, const uint8_t tag[CHACHA20_POLY1305_TAG_BYTES])
{
    EVP_CIPHER_CTX_Handle * handle = (EVP_CIPHER_CTX_Handle *) ctx;
    int ret = EVP_CIPHER_CTX_ctrl(handle->ctx, EVP_CTRL_AEAD_SET_TAG, CHACHA20_POLY1305_TAG_BYTES, (void *) tag);
    nlASSERT(ret == 1);
    int m_len;
    ret = EVP_DecryptFinal_ex(handle->ctx, NULL, &m_len);
    nlASSERT(m_len == 0);
    EVP_CIPHER_CTX_free(handle->ctx);
    handle->ctx = NULL;
    return (ret == 1) ? 0 : -1;
}

CHIP_STATIC_ASSERT(sizeof(CHIP_aes_ctr_ctx) >= sizeof(EVP_CIPHER_CTX_Handle), CHIP_aes_ctr_ctx);

void CHIP_aes_ctr_init(CHIP_aes_ctr_ctx * ctx, const uint8_t * key, int size, const uint8_t iv[16])
{
    EVP_CIPHER_CTX_Handle * handle = (EVP_CIPHER_CTX_Handle *) ctx;
    nlASSERT(size == 16 || size == 32);
    handle->ctx = EVP_CIPHER_CTX_new();
    int ret     = EVP_EncryptInit_ex(handle->ctx, (size == 16) ? EVP_aes_128_ctr() : EVP_aes_256_ctr(), NULL, key, iv);
    nlASSERT(ret == 1);
    EVP_CIPHER_CTX_set_padding(handle->ctx, 0);
}

void CHIP_aes_ctr_encrypt(CHIP_aes_ctr_ctx * ctx, uint8_t * ct, const uint8_t * pt, size_t pt_len)
{
    EVP_CIPHER_CTX_Handle * handle = (EVP_CIPHER_CTX_Handle *) ctx;
    int ct_len;
    int ret = EVP_EncryptUpdate(handle->ctx, ct, &ct_len, pt, pt_len);
    nlASSERT(ret == 1 && ct_len == pt_len);
}

void CHIP_aes_ctr_decrypt(CHIP_aes_ctr_ctx * ctx, uint8_t * pt, const uint8_t * ct, size_t ct_len)
{
    CHIP_aes_ctr_encrypt(ctx, pt, ct, ct_len);
}

void CHIP_aes_ctr_done(CHIP_aes_ctr_ctx * ctx)
{
    EVP_CIPHER_CTX_Handle * handle = (EVP_CIPHER_CTX_Handle *) ctx;
    EVP_CIPHER_CTX_free(handle->ctx);
    handle->ctx = NULL;
}
