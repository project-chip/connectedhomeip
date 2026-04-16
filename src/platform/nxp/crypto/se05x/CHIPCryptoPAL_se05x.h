/*
 *
 *    Copyright (c) 2020, 2025 Project CHIP Authors
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

#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/DataModelTypes.h>
#include <platform/nxp/crypto/se05x/CHIPCryptoPALHsm_se05x_config.h>

#if ((ENABLE_SE05X_SPAKE_VERIFIER) || (ENABLE_SE05X_SPAKE_PROVER))
typedef struct hsm_pake_context_s
{
    uint8_t spake_context[32];
    size_t spake_context_len;
    uint8_t spake_objId;
} hsm_pake_context_t;
#endif // #if ((ENABLE_SE05X_SPAKE_VERIFIER) || (ENABLE_SE05X_SPAKE_PROVER))

namespace chip {
namespace Crypto {

#define NXP_CRYPTO_KEY_MAGIC                                                                                                       \
    {                                                                                                                              \
        0xA5, 0xA6, 0xB5, 0xB6, 0xA5, 0xA6, 0xB5, 0xB6                                                                             \
    }

static const uint8_t se05x_magic_no[] = NXP_CRYPTO_KEY_MAGIC;

#if ((ENABLE_SE05X_SPAKE_VERIFIER) || (ENABLE_SE05X_SPAKE_PROVER))
/* Spake HSM class */

class Spake2pHSM_P256_SHA256_HKDF_HMAC : public Spake2p_P256_SHA256_HKDF_HMAC
{
public:
    Spake2pHSM_P256_SHA256_HKDF_HMAC() {}

    ~Spake2pHSM_P256_SHA256_HKDF_HMAC() {}

    CHIP_ERROR Init(const uint8_t * context, size_t context_len) override;

#if ENABLE_SE05X_SPAKE_VERIFIER
    CHIP_ERROR BeginVerifier(const uint8_t * my_identity, size_t my_identity_len, const uint8_t * peer_identity,
                             size_t peer_identity_len, const uint8_t * w0in, size_t w0in_len, const uint8_t * Lin,
                             size_t Lin_len) override;
#endif

#if ENABLE_SE05X_SPAKE_PROVER
    CHIP_ERROR BeginProver(const uint8_t * my_identity, size_t my_identity_len, const uint8_t * peer_identity,
                           size_t peer_identity_len, const uint8_t * w0in, size_t w0in_len, const uint8_t * w1in,
                           size_t w1in_len) override;
#endif

    CHIP_ERROR ComputeRoundOne(const uint8_t * pab, size_t pab_len, uint8_t * out, size_t * out_len) override;

    CHIP_ERROR ComputeRoundTwo(const uint8_t * in, size_t in_len, uint8_t * out, size_t * out_len) override;

    CHIP_ERROR KeyConfirm(const uint8_t * in, size_t in_len) override;

    hsm_pake_context_t hsm_pake_context;
};

#endif // #if ((ENABLE_SE05X_SPAKE_VERIFIER) || (ENABLE_SE05X_SPAKE_PROVER))

/**
 * @brief P256 derived class to use SE05x secure element for P256 Operations.
 *
 */
class P256KeypairSE05x : public P256Keypair
{
public:
    ~P256KeypairSE05x() override;
    CHIP_ERROR Initialize(ECPKeyTarget key_target) override;
    CHIP_ERROR Serialize(P256SerializedKeypair & output) const override;
    CHIP_ERROR Deserialize(P256SerializedKeypair & input) override;
    CHIP_ERROR NewCertificateSigningRequest(uint8_t * csr, size_t & csr_length) const override;
    CHIP_ERROR ECDSA_sign_msg(const uint8_t * msg, size_t msg_length, P256ECDSASignature & out_signature) const override;
    CHIP_ERROR ECDH_derive_secret(const P256PublicKey & remote_public_key, P256ECDHDerivedSecret & out_secret) const override;
};

class P256PublicKeySE05x final // final due to being copyable
    : public ECPKey<P256ECDSASignature>
{
public:
    P256PublicKeySE05x() = default;

    template <size_t N>
    constexpr P256PublicKeySE05x(const uint8_t (&raw_value)[N])
    {
        static_assert(N == kP256_PublicKey_Length, "Can only array-initialize from proper bounds");
        memcpy(&bytes[0], &raw_value[0], N);
    }

    template <size_t N>
    constexpr P256PublicKeySE05x(const FixedByteSpan<N> & value)
    {
        static_assert(N == kP256_PublicKey_Length, "Can only initialize from proper sized byte span");
        memcpy(&bytes[0], value.data(), N);
    }

    template <size_t N>
    P256PublicKeySE05x & operator=(const FixedByteSpan<N> & value)
    {
        static_assert(N == kP256_PublicKey_Length, "Can only initialize from proper sized byte span");
        memcpy(&bytes[0], value.data(), N);
        return *this;
    }

    SupportedECPKeyTypes Type() const override { return SupportedECPKeyTypes::ECP256R1; }
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

    CHIP_ERROR ECDSA_validate_msg_signature(const uint8_t * msg, size_t msg_length,
                                            const P256ECDSASignature & signature) const override;
    CHIP_ERROR ECDSA_validate_hash_signature(const uint8_t * hash, size_t hash_length,
                                             const P256ECDSASignature & signature) const override;

private:
    uint8_t bytes[kP256_PublicKey_Length];
};

class HKDF_sha_SE05x : public HKDF_sha
{
public:
    CHIP_ERROR HKDF_SHA256(const uint8_t * secret, size_t secret_length, const uint8_t * salt, size_t salt_length,
                           const uint8_t * info, size_t info_length, uint8_t * out_buffer, size_t out_length) override;
};

class HMAC_sha_SE05x : public HMAC_sha
{
public:
    virtual CHIP_ERROR HMAC_SHA256(const uint8_t * key, size_t key_length, const uint8_t * message, size_t message_length,
                                   uint8_t * out_buffer, size_t out_length) override;

    virtual CHIP_ERROR HMAC_SHA256(const Hmac128KeyHandle & key, const uint8_t * message, size_t message_length,
                                   uint8_t * out_buffer, size_t out_length) override;
};

class PBKDF2_sha256_SE05x : public PBKDF2_sha256
{
public:
    virtual CHIP_ERROR pbkdf2_sha256(const uint8_t * password, size_t plen, const uint8_t * salt, size_t slen,
                                     unsigned int iteration_count, uint32_t key_length, uint8_t * output) override;
};

} // namespace Crypto
} // namespace chip
