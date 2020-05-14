/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2015-2019 The HomeKit ADK Contributors
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

#ifndef CHIP_CRYPTO_H
#define CHIP_CRYPTO_H

#include "CHIPBase+CompilerAbstraction.h"

#ifndef CHIP_IP
#define CHIP_IP 1
#endif

//----------------------------------------------------------------------------------------------------------------------

/**
 * Defines an opaque type that is 8-byte aligned.
 *
 * @param      numBytes             Size of opaque type.
 *
 * **Example**

   @code{.c}

   // Public header.
   typedef CHIP_OPAQUE(80) CHIPFooRef;

   // Private header.
   typedef struct {
       // ...
   } CHIPFoo;
   CHIP_STATIC_ASSERT(sizeof (CHIPFooRef) >= sizeof (CHIPFoo), CHIPFoo);

   // Private sources.
   CHIPFooRef *foo_;
   CHIPFoo *foo = (CHIPFoo *) foo_;

   @endcode
 */
#if defined(_MSC_VER)
#define CHIP_OPAQUE(numBytes)                                                                                                      \
    struct                                                                                                                         \
    {                                                                                                                              \
        CHIP_ALIGNAS(8) char privateData[numBytes];                                                                                \
    }
#else
#define CHIP_OPAQUE(numBytes)                                                                                                      \
    struct                                                                                                                         \
    {                                                                                                                              \
        char privateData[numBytes] CHIP_ALIGNAS(8);                                                                                \
    }
#endif

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

void CHIP_store_bigendian(uint8_t x[4], uint32_t u);

#define ED25519_PUBLIC_KEY_BYTES 32
#define ED25519_SECRET_KEY_BYTES 32
#define ED25519_BYTES 64

void CHIP_ed25519_public_key(uint8_t pk[ED25519_PUBLIC_KEY_BYTES], const uint8_t sk[ED25519_SECRET_KEY_BYTES]);
void CHIP_ed25519_sign(uint8_t sig[ED25519_BYTES], const uint8_t * m, size_t m_len, const uint8_t sk[ED25519_SECRET_KEY_BYTES],
                       const uint8_t pk[ED25519_PUBLIC_KEY_BYTES]);
int CHIP_ed25519_verify(const uint8_t sig[ED25519_BYTES], const uint8_t * m, size_t m_len,
                        const uint8_t pk[ED25519_PUBLIC_KEY_BYTES]);

#define X25519_SCALAR_BYTES 32
#define X25519_BYTES 32

void CHIP_X25519_scalarmult_base(uint8_t r[X25519_BYTES], const uint8_t n[X25519_SCALAR_BYTES]);
void CHIP_X25519_scalarmult(uint8_t r[X25519_BYTES], const uint8_t n[X25519_SCALAR_BYTES], const uint8_t p[X25519_BYTES]);

#define CHACHA20_POLY1305_KEY_BYTES 32
#define CHACHA20_POLY1305_NONCE_BYTES_MAX 12
#define CHACHA20_POLY1305_TAG_BYTES 16

typedef CHIP_OPAQUE(8) CHIP_chacha20_poly1305_ctx;

void CHIP_chacha20_poly1305_init(CHIP_chacha20_poly1305_ctx * ctx, const uint8_t * n, size_t n_len,
                                 const uint8_t k[CHACHA20_POLY1305_KEY_BYTES]);
void CHIP_chacha20_poly1305_update_enc(CHIP_chacha20_poly1305_ctx * ctx, uint8_t * c, const uint8_t * m, size_t m_len,
                                       const uint8_t * n, size_t n_len, const uint8_t k[CHACHA20_POLY1305_KEY_BYTES]);
void CHIP_chacha20_poly1305_update_enc_aad(CHIP_chacha20_poly1305_ctx * ctx, const uint8_t * a, size_t a_len, const uint8_t * n,
                                           size_t n_len, const uint8_t k[CHACHA20_POLY1305_KEY_BYTES]);
void CHIP_chacha20_poly1305_final_enc(CHIP_chacha20_poly1305_ctx * ctx, uint8_t tag[CHACHA20_POLY1305_TAG_BYTES]);
void CHIP_chacha20_poly1305_update_dec(CHIP_chacha20_poly1305_ctx * ctx, uint8_t * m, const uint8_t * c, size_t c_len,
                                       const uint8_t * n, size_t n_len, const uint8_t k[CHACHA20_POLY1305_KEY_BYTES]);
void CHIP_chacha20_poly1305_update_dec_aad(CHIP_chacha20_poly1305_ctx * ctx, const uint8_t * a, size_t a_len, const uint8_t * n,
                                           size_t n_len, const uint8_t k[CHACHA20_POLY1305_KEY_BYTES]);
int CHIP_chacha20_poly1305_final_dec(CHIP_chacha20_poly1305_ctx * ctx, const uint8_t tag[CHACHA20_POLY1305_TAG_BYTES]);

void CHIP_chacha20_poly1305_encrypt(uint8_t tag[CHACHA20_POLY1305_TAG_BYTES], uint8_t * c, const uint8_t * m, size_t m_len,
                                    const uint8_t * n, size_t n_len, const uint8_t k[CHACHA20_POLY1305_KEY_BYTES]);
int CHIP_chacha20_poly1305_decrypt(const uint8_t tag[CHACHA20_POLY1305_TAG_BYTES], uint8_t * m, const uint8_t * c, size_t c_len,
                                   const uint8_t * n, size_t n_len, const uint8_t k[CHACHA20_POLY1305_KEY_BYTES]);
void CHIP_chacha20_poly1305_encrypt_aad(uint8_t tag[CHACHA20_POLY1305_TAG_BYTES], uint8_t * c, const uint8_t * m, size_t m_len,
                                        const uint8_t * a, size_t a_len, const uint8_t * n, size_t n_len,
                                        const uint8_t k[CHACHA20_POLY1305_KEY_BYTES]);
int CHIP_chacha20_poly1305_decrypt_aad(const uint8_t tag[CHACHA20_POLY1305_TAG_BYTES], uint8_t * m, const uint8_t * c, size_t c_len,
                                       const uint8_t * a, size_t a_len, const uint8_t * n, size_t n_len,
                                       const uint8_t k[CHACHA20_POLY1305_KEY_BYTES]);

#define SRP_PRIME_BYTES 384
#define SRP_SALT_BYTES 16
#define SRP_VERIFIER_BYTES 384
#define SRP_SECRET_KEY_BYTES 32
#define SRP_PUBLIC_KEY_BYTES 384
#define SRP_SCRAMBLING_PARAMETER_BYTES 64
#define SRP_PREMASTER_SECRET_BYTES 384
#define SRP_SESSION_KEY_BYTES 64
#define SRP_PROOF_BYTES 64

void CHIP_srp_verifier(uint8_t v[SRP_VERIFIER_BYTES], const uint8_t salt[SRP_SALT_BYTES], const uint8_t * user, size_t user_len,
                       const uint8_t * pass, size_t pass_len);
void CHIP_srp_public_key(uint8_t pub_b[SRP_PUBLIC_KEY_BYTES], const uint8_t priv_b[SRP_SECRET_KEY_BYTES],
                         const uint8_t v[SRP_VERIFIER_BYTES]);
void CHIP_srp_scrambling_parameter(uint8_t u[SRP_SCRAMBLING_PARAMETER_BYTES], const uint8_t pub_a[SRP_PUBLIC_KEY_BYTES],
                                   const uint8_t pub_b[SRP_PUBLIC_KEY_BYTES]);
int CHIP_srp_premaster_secret(uint8_t s[SRP_PREMASTER_SECRET_BYTES], const uint8_t pub_a[SRP_PUBLIC_KEY_BYTES],
                              const uint8_t priv_b[SRP_SECRET_KEY_BYTES], const uint8_t u[SRP_SCRAMBLING_PARAMETER_BYTES],
                              const uint8_t v[SRP_VERIFIER_BYTES]);
void CHIP_srp_session_key(uint8_t k[SRP_SESSION_KEY_BYTES], const uint8_t s[SRP_PREMASTER_SECRET_BYTES]);
void CHIP_srp_proof_m1(uint8_t m1[SRP_PROOF_BYTES], const uint8_t * user, size_t user_len, const uint8_t salt[SRP_SALT_BYTES],
                       const uint8_t pub_a[SRP_PUBLIC_KEY_BYTES], const uint8_t pub_b[SRP_PUBLIC_KEY_BYTES],
                       const uint8_t k[SRP_SESSION_KEY_BYTES]);
void CHIP_srp_proof_m2(uint8_t m2[SRP_PROOF_BYTES], const uint8_t pub_a[SRP_PUBLIC_KEY_BYTES], const uint8_t m1[SRP_PROOF_BYTES],
                       const uint8_t k[SRP_SESSION_KEY_BYTES]);

#define SHA1_BYTES 20

void CHIP_sha1(uint8_t md[SHA1_BYTES], const uint8_t * data, size_t size);

#define SHA256_BYTES 32

void CHIP_sha256(uint8_t md[SHA256_BYTES], const uint8_t * data, size_t size);

#define SHA512_BYTES 64

void CHIP_sha512(uint8_t md[SHA512_BYTES], const uint8_t * data, size_t size);

#define HMAC_SHA1_BYTES SHA1_BYTES

void CHIP_hmac_sha1_aad(uint8_t r[HMAC_SHA1_BYTES], const uint8_t * key, size_t key_len, const uint8_t * in, size_t in_len,
                        const uint8_t * aad, size_t aad_len);

void CHIP_hkdf_sha512(uint8_t * r, size_t r_len, const uint8_t * key, size_t key_len, const uint8_t * salt, size_t salt_len,
                      const uint8_t * info, size_t info_len);

void CHIP_pbkdf2_hmac_sha1(uint8_t * key, size_t key_len, const uint8_t * password, size_t password_len, const uint8_t * salt,
                           size_t salt_len, uint32_t count);

typedef CHIP_OPAQUE(8) CHIP_aes_ctr_ctx;

#define AES128_KEY_BYTES 16

void CHIP_aes_ctr_init(CHIP_aes_ctr_ctx * ctx, const uint8_t * key, int size, const uint8_t iv[16]);
void CHIP_aes_ctr_encrypt(CHIP_aes_ctr_ctx * ctx, uint8_t * ct, const uint8_t * pt, size_t pt_len);
void CHIP_aes_ctr_decrypt(CHIP_aes_ctr_ctx * ctx, uint8_t * pt, const uint8_t * ct, size_t ct_len);
void CHIP_aes_ctr_done(CHIP_aes_ctr_ctx * ctx);

int CHIP_constant_time_equal(const void * x, const void * y, size_t length);
int CHIP_constant_time_is_zero(const void * x, size_t length);
void CHIP_constant_time_fill_zero(void * x, size_t length);
void CHIP_constant_time_copy(void * x, const void * y, size_t length);

#ifdef __cplusplus
}
#endif

#endif
