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
 *      Header that exposes the platform agnostic CHIP crypto primitives
 */

#pragma once

#include "CHIPCryptoPALHsm_config.h"
#include <lib/core/DataModelTypes.h>

#if CHIP_CRYPTO_HSM_NXP
#include <fsl_sss_se05x_apis.h>
#endif

#if ((ENABLE_HSM_SPAKE_VERIFIER) || (ENABLE_HSM_SPAKE_PROVER))
typedef struct hsm_pake_context_s
{
    uint8_t spake_context[32];
    size_t spake_context_len;
#if CHIP_CRYPTO_HSM_NXP
    SE05x_CryptoObjectID_t spake_objId;
#endif
} hsm_pake_context_t;
#endif //#if ((ENABLE_HSM_SPAKE_VERIFIER) || (ENABLE_HSM_SPAKE_PROVER))

namespace chip {
namespace Crypto {

#if ((ENABLE_HSM_SPAKE_VERIFIER) || (ENABLE_HSM_SPAKE_PROVER))
/* Spake HSM class */

class Spake2pHSM_P256_SHA256_HKDF_HMAC : public Spake2p_P256_SHA256_HKDF_HMAC
{
public:
    Spake2pHSM_P256_SHA256_HKDF_HMAC() {}

    ~Spake2pHSM_P256_SHA256_HKDF_HMAC() {}

    CHIP_ERROR Init(const uint8_t * context, size_t context_len) override;

#if ENABLE_HSM_SPAKE_VERIFIER
    CHIP_ERROR BeginVerifier(const uint8_t * my_identity, size_t my_identity_len, const uint8_t * peer_identity,
                             size_t peer_identity_len, const uint8_t * w0in, size_t w0in_len, const uint8_t * Lin,
                             size_t Lin_len) override;
#endif

#if ENABLE_HSM_SPAKE_PROVER
    CHIP_ERROR BeginProver(const uint8_t * my_identity, size_t my_identity_len, const uint8_t * peer_identity,
                           size_t peer_identity_len, const uint8_t * w0in, size_t w0in_len, const uint8_t * w1in,
                           size_t w1in_len) override;
#endif

    CHIP_ERROR ComputeRoundOne(const uint8_t * pab, size_t pab_len, uint8_t * out, size_t * out_len) override;

    CHIP_ERROR ComputeRoundTwo(const uint8_t * in, size_t in_len, uint8_t * out, size_t * out_len) override;

    CHIP_ERROR KeyConfirm(const uint8_t * in, size_t in_len) override;

    hsm_pake_context_t hsm_pake_context;
};

#endif //#if ((ENABLE_HSM_SPAKE_VERIFIER) || (ENABLE_HSM_SPAKE_PROVER))

#if ENABLE_HSM_GENERATE_EC_KEY
/* Nist256 Key pair HSM class */

class P256PublicKeyHSM : public P256PublicKey
{
public:
    P256PublicKeyHSM() { PublicKeyid = 0; }

    size_t Length() const override { return kP256_PublicKey_Length; }
    operator uint8_t *() override { return bytes; }
    operator const uint8_t *() const override { return bytes; }

    const uint8_t * ConstBytes() const override { return &bytes[0]; }
    uint8_t * Bytes() override { return &bytes[0]; }
    bool IsUncompressed() const override
    {
        constexpr uint8_t kUncompressedPointMarker = 0x04;
        // SEC1 definition of an uncompressed point is (0x04 || X || Y) where X and Y are
        // raw zero-padded big-endian large integers of the group size.
        return (Length() == ((kP256_FE_Length * 2) + 1)) && (ConstBytes()[0] == kUncompressedPointMarker);
    }

    void SetPublicKeyId(uint32_t id) { PublicKeyid = id; }

    CHIP_ERROR ECDSA_validate_msg_signature(const uint8_t * msg, size_t msg_length,
                                            const P256ECDSASignature & signature) const override;

    CHIP_ERROR ECDSA_validate_hash_signature(const uint8_t * hash, size_t hash_length,
                                             const P256ECDSASignature & signature) const override;

private:
    uint8_t bytes[kP256_PublicKey_Length];
    uint32_t PublicKeyid;
};

class P256KeypairHSM : public P256Keypair
{
public:
    P256KeypairHSM()
    {
        provisioned_key = false;
        keyid           = 0;
    }

    ~P256KeypairHSM();

    virtual CHIP_ERROR Initialize(ECPKeyTarget key_target) override;

    virtual CHIP_ERROR Serialize(P256SerializedKeypair & output) const override;

    virtual CHIP_ERROR Deserialize(P256SerializedKeypair & input) override;

    virtual CHIP_ERROR ECDSA_sign_msg(const uint8_t * msg, size_t msg_length, P256ECDSASignature & out_signature) const override;

    virtual CHIP_ERROR ECDH_derive_secret(const P256PublicKey & remote_public_key,
                                          P256ECDHDerivedSecret & out_secret) const override;

    CHIP_ERROR NewCertificateSigningRequest(uint8_t * csr, size_t & csr_length) const override;

    const P256PublicKeyHSM & Pubkey() const override { return mPublicKeyHSM; }

    bool provisioned_key;

    void SetKeyId(uint32_t id) { keyid = id; }

    uint32_t GetKeyId(void) { return keyid; }

private:
    uint32_t keyid;
    P256PublicKeyHSM mPublicKeyHSM;
};

#endif //#if ENABLE_HSM_GENERATE_EC_KEY

#if ENABLE_HSM_PBKDF2_SHA256

class PBKDF2_sha256HSM : public PBKDF2_sha256
{
public:
    PBKDF2_sha256HSM();
    ~PBKDF2_sha256HSM();

    virtual CHIP_ERROR pbkdf2_sha256(const uint8_t * password, size_t plen, const uint8_t * salt, size_t slen,
                                     unsigned int iteration_count, uint32_t key_length, uint8_t * output) override;

    void SetKeyId(uint32_t id) { keyid = id; }

    uint32_t GetKeyId() { return keyid; }

private:
    uint32_t keyid;
};

#endif //#if ENABLE_HSM_PBKDF2_SHA256

#if ENABLE_HSM_HKDF_SHA256

class HKDF_shaHSM : public HKDF_sha
{
public:
    HKDF_shaHSM();
    ~HKDF_shaHSM();

    virtual CHIP_ERROR HKDF_SHA256(const uint8_t * secret, const size_t secret_length, const uint8_t * salt,
                                   const size_t salt_length, const uint8_t * info, const size_t info_length, uint8_t * out_buffer,
                                   size_t out_length) override;

private:
    uint32_t keyid;
};

#endif //#if ENABLE_HSM_HKDF_SHA256

#if ENABLE_HSM_HMAC_SHA256

class HMAC_shaHSM : public HMAC_sha
{
public:
    HMAC_shaHSM();
    ~HMAC_shaHSM();

    virtual CHIP_ERROR HMAC_SHA256(const uint8_t * key, size_t key_length, const uint8_t * message, size_t message_length,
                                   uint8_t * out_buffer, size_t out_length) override;

private:
    uint32_t keyid;
};

#endif //#if ENABLE_HSM_HMAC_SHA256

} // namespace Crypto
} // namespace chip
