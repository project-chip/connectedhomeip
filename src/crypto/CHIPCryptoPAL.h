/*
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
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

#if CHIP_HAVE_CONFIG_H
#include <crypto/CryptoBuildConfig.h>
#endif // CHIP_HAVE_CONFIG_H

#include <system/SystemConfig.h>

#include <lib/core/CHIPError.h>
#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <lib/core/Optional.h>
#include <lib/support/BufferReader.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafePointerCast.h>
#include <lib/support/Span.h>

#include <stddef.h>
#include <string.h>

namespace chip {
namespace Crypto {

inline constexpr size_t kMax_x509_Certificate_Length = 600;

inline constexpr size_t kP256_FE_Length                        = 32;
inline constexpr size_t kP256_ECDSA_Signature_Length_Raw       = (2 * kP256_FE_Length);
inline constexpr size_t kP256_Point_Length                     = (2 * kP256_FE_Length + 1);
inline constexpr size_t kSHA256_Hash_Length                    = 32;
inline constexpr size_t kSHA1_Hash_Length                      = 20;
inline constexpr size_t kSubjectKeyIdentifierLength            = kSHA1_Hash_Length;
inline constexpr size_t kAuthorityKeyIdentifierLength          = kSHA1_Hash_Length;
inline constexpr size_t kMaxCertificateSerialNumberLength      = 20;
inline constexpr size_t kMaxCertificateDistinguishedNameLength = 200;
inline constexpr size_t kMaxCRLDistributionPointURLLength      = 100;

inline constexpr char kValidCDPURIHttpPrefix[]  = "http://";
inline constexpr char kValidCDPURIHttpsPrefix[] = "https://";

inline constexpr size_t CHIP_CRYPTO_GROUP_SIZE_BYTES      = kP256_FE_Length;
inline constexpr size_t CHIP_CRYPTO_PUBLIC_KEY_SIZE_BYTES = kP256_Point_Length;

inline constexpr size_t CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES      = 16;
inline constexpr size_t CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES = 16;

inline constexpr size_t kMax_ECDH_Secret_Length     = kP256_FE_Length;
inline constexpr size_t kMax_ECDSA_Signature_Length = kP256_ECDSA_Signature_Length_Raw;
inline constexpr size_t kMAX_FE_Length              = kP256_FE_Length;
inline constexpr size_t kMAX_Point_Length           = kP256_Point_Length;
inline constexpr size_t kMAX_Hash_Length            = kSHA256_Hash_Length;

// Minimum required CSR length buffer length is relatively small since it's a single
// P256 key and no metadata/extensions are expected to be honored by the CA.
inline constexpr size_t kMIN_CSR_Buffer_Size = 255;

[[deprecated("This constant is no longer used by common code and should be replaced by kMIN_CSR_Buffer_Size. Checks that a CSR is "
             "<= kMAX_CSR_Buffer_size must be updated. This remains to keep valid buffers working from previous public API "
             "usage.")]] constexpr size_t kMAX_CSR_Buffer_Size = 255;

inline constexpr size_t CHIP_CRYPTO_HASH_LEN_BYTES = kSHA256_Hash_Length;

inline constexpr size_t kSpake2p_Min_PBKDF_Salt_Length  = 16;
inline constexpr size_t kSpake2p_Max_PBKDF_Salt_Length  = 32;
inline constexpr uint32_t kSpake2p_Min_PBKDF_Iterations = 1000;
inline constexpr uint32_t kSpake2p_Max_PBKDF_Iterations = 100000;

inline constexpr size_t kP256_PrivateKey_Length = CHIP_CRYPTO_GROUP_SIZE_BYTES;
inline constexpr size_t kP256_PublicKey_Length  = CHIP_CRYPTO_PUBLIC_KEY_SIZE_BYTES;

inline constexpr size_t kAES_CCM128_Key_Length   = 128u / 8u;
inline constexpr size_t kAES_CCM128_Block_Length = kAES_CCM128_Key_Length;
inline constexpr size_t kAES_CCM128_Nonce_Length = 13;
inline constexpr size_t kAES_CCM128_Tag_Length   = 16;
inline constexpr size_t kHMAC_CCM128_Key_Length  = 128u / 8u;

inline constexpr size_t CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES = kAES_CCM128_Nonce_Length;

/* These sizes are hardcoded here to remove header dependency on underlying crypto library
 * in a public interface file. The validity of these sizes is verified by static_assert in
 * the implementation files.
 */
inline constexpr size_t kMAX_Spake2p_Context_Size     = 1024;
inline constexpr size_t kMAX_P256Keypair_Context_Size = 512;

inline constexpr size_t kEmitDerIntegerWithoutTagOverhead = 1; // 1 sign stuffer
inline constexpr size_t kEmitDerIntegerOverhead           = 3; // Tag + Length byte + 1 sign stuffer

inline constexpr size_t kMAX_Hash_SHA256_Context_Size = CHIP_CONFIG_SHA256_CONTEXT_SIZE;

inline constexpr size_t kSpake2p_WS_Length                 = kP256_FE_Length + 8;
inline constexpr size_t kSpake2p_VerifierSerialized_Length = kP256_FE_Length + kP256_Point_Length;

inline constexpr char kVIDPrefixForCNEncoding[]    = "Mvid:";
inline constexpr char kPIDPrefixForCNEncoding[]    = "Mpid:";
inline constexpr size_t kVIDandPIDHexLength        = sizeof(uint16_t) * 2;
inline constexpr size_t kMax_CommonNameAttr_Length = 64;

/*
 * Overhead to encode a raw ECDSA signature in X9.62 format in ASN.1 DER
 *
 * Ecdsa-Sig-Value ::= SEQUENCE {
 *     r       INTEGER,
 *     s       INTEGER
 * }
 *
 * --> SEQUENCE, universal constructed tag (0x30), length over 2 bytes, up to 255 (to support future larger sizes up to 512 bits)
 *   -> SEQ_OVERHEAD = 3 bytes
 * --> INTEGER, universal primitive tag (0x02), length over 1 byte, one extra byte worst case
 *     over max for 0x00 when MSB is set.
 *       -> INT_OVERHEAD = 3 bytes
 *
 * There is 1 sequence of 2 integers. Overhead is SEQ_OVERHEAD + (2 * INT_OVERHEAD) = 3 + (2 * 3) = 9.
 */
inline constexpr size_t kMax_ECDSA_X9Dot62_Asn1_Overhead = 9;
inline constexpr size_t kMax_ECDSA_Signature_Length_Der  = kMax_ECDSA_Signature_Length + kMax_ECDSA_X9Dot62_Asn1_Overhead;

static_assert(kMax_ECDH_Secret_Length >= kP256_FE_Length, "ECDH shared secret is too short for crypto suite");
static_assert(kMax_ECDSA_Signature_Length >= kP256_ECDSA_Signature_Length_Raw,
              "ECDSA signature buffer length is too short for crypto suite");

inline constexpr size_t kCompressedFabricIdentifierSize = 8;

/**
 * Spake2+ parameters for P256
 * Defined in https://www.ietf.org/id/draft-bar-cfrg-spake2plus-01.html#name-ciphersuites
 */
const uint8_t spake2p_M_p256[] = {
    0x04, 0x88, 0x6e, 0x2f, 0x97, 0xac, 0xe4, 0x6e, 0x55, 0xba, 0x9d, 0xd7, 0x24, 0x25, 0x79, 0xf2, 0x99,
    0x3b, 0x64, 0xe1, 0x6e, 0xf3, 0xdc, 0xab, 0x95, 0xaf, 0xd4, 0x97, 0x33, 0x3d, 0x8f, 0xa1, 0x2f, 0x5f,
    0xf3, 0x55, 0x16, 0x3e, 0x43, 0xce, 0x22, 0x4e, 0x0b, 0x0e, 0x65, 0xff, 0x02, 0xac, 0x8e, 0x5c, 0x7b,
    0xe0, 0x94, 0x19, 0xc7, 0x85, 0xe0, 0xca, 0x54, 0x7d, 0x55, 0xa1, 0x2e, 0x2d, 0x20,
};
const uint8_t spake2p_N_p256[] = {
    0x04, 0xd8, 0xbb, 0xd6, 0xc6, 0x39, 0xc6, 0x29, 0x37, 0xb0, 0x4d, 0x99, 0x7f, 0x38, 0xc3, 0x77, 0x07,
    0x19, 0xc6, 0x29, 0xd7, 0x01, 0x4d, 0x49, 0xa2, 0x4b, 0x4f, 0x98, 0xba, 0xa1, 0x29, 0x2b, 0x49, 0x07,
    0xd6, 0x0a, 0xa6, 0xbf, 0xad, 0xe4, 0x50, 0x08, 0xa6, 0x36, 0x33, 0x7f, 0x51, 0x68, 0xc6, 0x4d, 0x9b,
    0xd3, 0x60, 0x34, 0x80, 0x8c, 0xd5, 0x64, 0x49, 0x0b, 0x1e, 0x65, 0x6e, 0xdb, 0xe7,
};

/**
 * Spake2+ state machine to ensure proper execution of the protocol.
 */
enum class CHIP_SPAKE2P_STATE : uint8_t
{
    PREINIT = 0, // Before any initialization
    INIT,        // First initialization
    STARTED,     // Prover & Verifier starts
    R1,          // Round one complete
    R2,          // Round two complete
    KC,          // Key confirmation complete
};

/**
 * Spake2+ role.
 */
enum class CHIP_SPAKE2P_ROLE : uint8_t
{
    VERIFIER = 0, // Accessory
    PROVER   = 1, // Commissioner
};

enum class SupportedECPKeyTypes : uint8_t
{
    ECP256R1 = 0,
};

enum class ECPKeyTarget : uint8_t
{
    ECDH  = 0,
    ECDSA = 1,
};

/** @brief Safely clears the first `len` bytes of memory area `buf`.
 * @param buf Pointer to a memory buffer holding secret data that must be cleared.
 * @param len Specifies secret data size in bytes.
 **/
void ClearSecretData(uint8_t * buf, size_t len);

/**
 * Helper for clearing a C array which auto-deduces the size.
 */
template <size_t N>
void ClearSecretData(uint8_t (&buf)[N])
{
    ClearSecretData(buf, N);
}

/**
 * @brief Constant-time buffer comparison
 *
 * This function implements constant time memcmp. It's good practice
 * to use constant time functions for cryptographic functions.
 *
 * @param a Pointer to first buffer
 * @param b Pointer to Second buffer
 * @param n Number of bytes to compare
 * @return true if `n` first bytes of both buffers are equal, false otherwise
 */
bool IsBufferContentEqualConstantTime(const void * a, const void * b, size_t n);

template <typename Sig>
class ECPKey
{
protected:
    // This base type can't be copied / assigned directly.
    // Sub-types should be either uncopyable or final.
    ECPKey()                           = default;
    ECPKey(const ECPKey &)             = default;
    ECPKey & operator=(const ECPKey &) = default;

public:
    virtual ~ECPKey() = default;

    virtual SupportedECPKeyTypes Type() const  = 0;
    virtual size_t Length() const              = 0;
    virtual bool IsUncompressed() const        = 0;
    virtual operator const uint8_t *() const   = 0;
    virtual operator uint8_t *()               = 0;
    virtual const uint8_t * ConstBytes() const = 0;
    virtual uint8_t * Bytes()                  = 0;

    virtual bool Matches(const ECPKey<Sig> & other) const
    {
        return (this->Length() == other.Length()) &&
            IsBufferContentEqualConstantTime(this->ConstBytes(), other.ConstBytes(), this->Length());
    }

    virtual CHIP_ERROR ECDSA_validate_msg_signature(const uint8_t * msg, const size_t msg_length, const Sig & signature) const = 0;
    virtual CHIP_ERROR ECDSA_validate_hash_signature(const uint8_t * hash, const size_t hash_length,
                                                     const Sig & signature) const                                              = 0;
};

/**
 * @brief Helper class for holding sensitive data that should be erased from memory after use.
 *
 * The sensitive data buffer is a variable-length, fixed-capacity buffer class that securely erases
 * the contents of a buffer when the buffer is destroyed.
 */
template <size_t kCapacity>
class SensitiveDataBuffer
{
public:
    ~SensitiveDataBuffer()
    {
        // Sanitize after use
        ClearSecretData(mBytes);
    }
    SensitiveDataBuffer() {}
    SensitiveDataBuffer(const SensitiveDataBuffer & other) { *this = other; }
    SensitiveDataBuffer & operator=(const SensitiveDataBuffer & other)
    {
        // Guard self assignment
        if (this == &other)
            return *this;

        ClearSecretData(mBytes);
        SetLength(other.Length());
        ::memcpy(Bytes(), other.ConstBytes(), other.Length());
        return *this;
    }

    /**
     * @brief Set current length of the buffer
     * @return Error if new length is exceeds capacity of the buffer
     */
    CHIP_ERROR SetLength(size_t length)
    {
        VerifyOrReturnError(length <= kCapacity, CHIP_ERROR_INVALID_ARGUMENT);
        mLength = length;
        return CHIP_NO_ERROR;
    }

    /**
     * @brief Returns current length of the buffer
     */
    size_t Length() const { return mLength; }

    /**
     * @brief Returns non-const pointer to start of the underlying buffer
     */
    uint8_t * Bytes() { return &mBytes[0]; }

    /**
     * @brief Returns const pointer to start of the underlying buffer
     */
    const uint8_t * ConstBytes() const { return &mBytes[0]; }

    /**
     * @brief Constructs span from the underlying buffer
     */
    ByteSpan Span() const { return ByteSpan(ConstBytes(), Length()); }

    /**
     * @brief Returns capacity of the buffer
     */
    static constexpr size_t Capacity() { return kCapacity; }

private:
    uint8_t mBytes[kCapacity];
    size_t mLength = 0;
};

/**
 * @brief Helper class for holding fixed-sized sensitive data that should be erased from memory after use.
 *
 * The sensitive data buffer is a fixed-length, fixed-capacity buffer class that securely erases
 * the contents of a buffer when the buffer is destroyed.
 */
template <size_t kCapacity>
class SensitiveDataFixedBuffer
{
public:
    SensitiveDataFixedBuffer() = default;

    constexpr explicit SensitiveDataFixedBuffer(const uint8_t (&rawValue)[kCapacity])
    {
        memcpy(&mBytes[0], &rawValue[0], kCapacity);
    }

    constexpr explicit SensitiveDataFixedBuffer(const FixedByteSpan<kCapacity> & value)
    {
        memcpy(&mBytes[0], value.data(), kCapacity);
    }

    ~SensitiveDataFixedBuffer()
    {
        // Sanitize after use
        ClearSecretData(mBytes);
    }

    /**
     * @brief Returns fixed length of the buffer
     */
    constexpr size_t Length() const { return kCapacity; }

    /**
     * @brief Returns non-const pointer to start of the underlying buffer
     */
    uint8_t * Bytes() { return &mBytes[0]; }

    /**
     * @brief Returns const pointer to start of the underlying buffer
     */
    const uint8_t * ConstBytes() const { return &mBytes[0]; }

    /**
     * @brief Constructs fixed span from the underlying buffer
     */
    FixedByteSpan<kCapacity> Span() const { return FixedByteSpan<kCapacity>(mBytes); }

    /**
     * @brief Returns capacity of the buffer
     */
    static constexpr size_t Capacity() { return kCapacity; }

private:
    uint8_t mBytes[kCapacity];
};

using P256ECDSASignature    = SensitiveDataBuffer<kMax_ECDSA_Signature_Length>;
using P256ECDHDerivedSecret = SensitiveDataBuffer<kMax_ECDH_Secret_Length>;

using IdentityProtectionKey     = SensitiveDataFixedBuffer<CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES>;
using IdentityProtectionKeySpan = FixedByteSpan<Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES>;

using AttestationChallenge = SensitiveDataFixedBuffer<CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES>;

class P256PublicKey final // final due to being copyable
    : public ECPKey<P256ECDSASignature>
{
public:
    P256PublicKey() = default;

    template <size_t N>
    constexpr P256PublicKey(const uint8_t (&raw_value)[N])
    {
        static_assert(N == kP256_PublicKey_Length, "Can only array-initialize from proper bounds");
        memcpy(&bytes[0], &raw_value[0], N);
    }

    template <size_t N>
    constexpr P256PublicKey(const FixedByteSpan<N> & value)
    {
        static_assert(N == kP256_PublicKey_Length, "Can only initialize from proper sized byte span");
        memcpy(&bytes[0], value.data(), N);
    }

    template <size_t N>
    P256PublicKey & operator=(const FixedByteSpan<N> & value)
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

template <typename PK, typename Secret, typename Sig>
class ECPKeypair
{
protected:
    // This base type can't be copied / assigned directly.
    // Sub-types should be either uncopyable or final.
    ECPKeypair()                               = default;
    ECPKeypair(const ECPKeypair &)             = default;
    ECPKeypair & operator=(const ECPKeypair &) = default;

public:
    virtual ~ECPKeypair() = default;

    /** @brief Generate a new Certificate Signing Request (CSR).
     * @param csr Newly generated CSR in DER format
     * @param csr_length The caller provides the length of input buffer (csr). The function returns the actual length of generated
     *CSR.
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    virtual CHIP_ERROR NewCertificateSigningRequest(uint8_t * csr, size_t & csr_length) const = 0;

    /**
     * @brief A function to sign a msg using ECDSA
     * @param msg Message that needs to be signed
     * @param msg_length Length of message
     * @param out_signature Buffer that will hold the output signature. The signature consists of: 2 EC elements (r and s),
     * in raw <r,s> point form (see SEC1).
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    virtual CHIP_ERROR ECDSA_sign_msg(const uint8_t * msg, size_t msg_length, Sig & out_signature) const = 0;

    /** @brief A function to derive a shared secret using ECDH
     * @param remote_public_key Public key of remote peer with which we are trying to establish secure channel. remote_public_key is
     * ASN.1 DER encoded as padded big-endian field elements as described in SEC 1: Elliptic Curve Cryptography
     * [https://www.secg.org/sec1-v2.pdf]
     * @param out_secret Buffer to write out secret into. This is a byte array representing the x coordinate of the shared secret.
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    virtual CHIP_ERROR ECDH_derive_secret(const PK & remote_public_key, Secret & out_secret) const = 0;

    virtual const PK & Pubkey() const = 0;
};

struct alignas(size_t) P256KeypairContext
{
    uint8_t mBytes[kMAX_P256Keypair_Context_Size];
};

/**
 * A serialized P256 key pair is the concatenation of the public and private keys, in that order.
 */
using P256SerializedKeypair = SensitiveDataBuffer<kP256_PublicKey_Length + kP256_PrivateKey_Length>;

class P256KeypairBase : public ECPKeypair<P256PublicKey, P256ECDHDerivedSecret, P256ECDSASignature>
{
protected:
    // This base type can't be copied / assigned directly.
    // Sub-types should be either uncopyable or final.
    P256KeypairBase()                                    = default;
    P256KeypairBase(const P256KeypairBase &)             = default;
    P256KeypairBase & operator=(const P256KeypairBase &) = default;

public:
    /**
     * @brief Initialize the keypair.
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    virtual CHIP_ERROR Initialize(ECPKeyTarget key_target) = 0;

    /**
     * @brief Serialize the keypair.
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    virtual CHIP_ERROR Serialize(P256SerializedKeypair & output) const = 0;

    /**
     * @brief Deserialize the keypair.
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    virtual CHIP_ERROR Deserialize(P256SerializedKeypair & input) = 0;
};

class P256Keypair : public P256KeypairBase
{
public:
    P256Keypair() = default;
    ~P256Keypair() override;

    // P256Keypair can't be copied / assigned.
    P256Keypair(const P256Keypair &)             = delete;
    P256Keypair & operator=(const P256Keypair &) = delete;

    /**
     * @brief Initialize the keypair.
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR Initialize(ECPKeyTarget key_target) override;

    /**
     * @brief Serialize the keypair.
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR Serialize(P256SerializedKeypair & output) const override;

    /**
     * @brief Deserialize the keypair.
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR Deserialize(P256SerializedKeypair & input) override;

    /**
     * @brief Generate a new Certificate Signing Request (CSR).
     * @param csr Newly generated CSR in DER format
     * @param csr_length The caller provides the length of input buffer (csr). The function returns the actual length of generated
     *CSR.
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR NewCertificateSigningRequest(uint8_t * csr, size_t & csr_length) const override;

    /**
     * @brief A function to sign a msg using ECDSA
     * @param msg Message that needs to be signed
     * @param msg_length Length of message
     * @param out_signature Buffer that will hold the output signature. The signature consists of: 2 EC elements (r and s),
     * in raw <r,s> point form (see SEC1).
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR ECDSA_sign_msg(const uint8_t * msg, size_t msg_length, P256ECDSASignature & out_signature) const override;

    /**
     * @brief A function to derive a shared secret using ECDH
     *
     * This implements the CHIP_Crypto_ECDH(PrivateKey myPrivateKey, PublicKey theirPublicKey) cryptographic primitive
     * from the specification, using this class's private key from `mKeypair` as `myPrivateKey` and the remote
     * public key from `remote_public_key` as `theirPublicKey`.
     *
     * @param remote_public_key Public key of remote peer with which we are trying to establish secure channel. remote_public_key is
     * ASN.1 DER encoded as padded big-endian field elements as described in SEC 1: Elliptic Curve Cryptography
     * [https://www.secg.org/sec1-v2.pdf]
     * @param out_secret Buffer to write out secret into. This is a byte array representing the x coordinate of the shared secret.
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR ECDH_derive_secret(const P256PublicKey & remote_public_key, P256ECDHDerivedSecret & out_secret) const override;

    /** @brief Return public key for the keypair.
     **/
    const P256PublicKey & Pubkey() const override { return mPublicKey; }

    /** Release resources associated with this key pair */
    void Clear();

protected:
    P256PublicKey mPublicKey;
    mutable P256KeypairContext mKeypair;
    bool mInitialized = false;
};

/**
 * @brief Platform-specific symmetric key handle
 *
 * The class represents a key used by the Matter stack either in the form of raw key material or key
 * reference, depending on the platform. To achieve that, it contains an opaque context that can be
 * cast to a concrete representation used by the given platform.
 *
 * @note SymmetricKeyHandle is an abstract class to force child classes for each key handle type.
 *       SymmetricKeyHandle class implements all the necessary components for handles.
 */
template <size_t ContextSize>
class SymmetricKeyHandle
{
public:
    SymmetricKeyHandle(const SymmetricKeyHandle &) = delete;
    SymmetricKeyHandle(SymmetricKeyHandle &&)      = delete;
    void operator=(const SymmetricKeyHandle &)     = delete;
    void operator=(SymmetricKeyHandle &&)          = delete;

    /**
     * @brief Get internal context cast to the desired key representation
     */
    template <class T>
    const T & As() const
    {
        return *SafePointerCast<const T *>(&mContext);
    }

    /**
     * @brief Get internal context cast to the desired, mutable key representation
     */
    template <class T>
    T & AsMutable()
    {
        return *SafePointerCast<T *>(&mContext);
    }

protected:
    SymmetricKeyHandle() = default;
    ~SymmetricKeyHandle() { ClearSecretData(mContext.mOpaque); }

private:
    struct alignas(uintptr_t) OpaqueContext
    {
        uint8_t mOpaque[ContextSize] = {};
    } mContext;
};

using Symmetric128BitsKeyByteArray = uint8_t[CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES];

/**
 * @brief Platform-specific 128-bit symmetric key handle
 */
class Symmetric128BitsKeyHandle : public SymmetricKeyHandle<CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES>
{
};

/**
 * @brief Platform-specific 128-bit AES key handle
 */
class Aes128KeyHandle final : public Symmetric128BitsKeyHandle
{
};

/**
 * @brief Platform-specific 128-bit HMAC key handle
 */
class Hmac128KeyHandle final : public Symmetric128BitsKeyHandle
{
};

/**
 * @brief Platform-specific HKDF key handle
 */
class HkdfKeyHandle final : public SymmetricKeyHandle<CHIP_CONFIG_HKDF_KEY_HANDLE_CONTEXT_SIZE>
{
};

/**
 * @brief Convert a raw ECDSA signature to ASN.1 signature (per X9.62) as used by TLS libraries.
 *
 * Errors are:
 *   - CHIP_ERROR_INVALID_ARGUMENT on any argument being invalid (e.g. nullptr), wrong sizes,
 *     wrong or unsupported format,
 *   - CHIP_ERROR_BUFFER_TOO_SMALL on running out of space at runtime.
 *   - CHIP_ERROR_INTERNAL on any unexpected processing error.
 *
 * @param[in] fe_length_bytes Field Element length in bytes (e.g. 32 for P256 curve)
 * @param[in] raw_sig Raw signature of <r,s> concatenated
 * @param[out] out_asn1_sig ASN.1 DER signature format output buffer. Size must have space for at least
 * kMax_ECDSA_X9Dot62_Asn1_Overhead. On CHIP_NO_ERROR, the out_asn1_sig buffer will be re-assigned
 * to have the correct size based on variable-length output.
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 */
CHIP_ERROR EcdsaRawSignatureToAsn1(size_t fe_length_bytes, const ByteSpan & raw_sig, MutableByteSpan & out_asn1_sig);

/**
 * @brief Convert an ASN.1 DER signature (per X9.62) as used by TLS libraries to SEC1 raw format
 *
 * Errors are:
 *   - CHIP_ERROR_INVALID_ARGUMENT on any argument being invalid (e.g. nullptr), wrong sizes,
 *     wrong or unsupported format,
 *   - CHIP_ERROR_BUFFER_TOO_SMALL on running out of space at runtime.
 *   - CHIP_ERROR_INTERNAL on any unexpected processing error.
 *
 * @param[in] fe_length_bytes Field Element length in bytes (e.g. 32 for P256 curve)
 * @param[in] asn1_sig ASN.1 DER signature input
 * @param[out] out_raw_sig Raw signature of <r,s> concatenated format output buffer. Size must be at
 * least >= `2 * fe_length_bytes`. On CHIP_NO_ERROR, the out_raw_sig buffer will be re-assigned
 * to have the correct size (2 * fe_length_bytes).
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 */
CHIP_ERROR EcdsaAsn1SignatureToRaw(size_t fe_length_bytes, const ByteSpan & asn1_sig, MutableByteSpan & out_raw_sig);

/**
 * @brief Utility to read a length field after a tag in a DER-encoded stream.
 * @param[in] reader Reader instance from which the input will be read
 * @param[out] length Length of the following element read from the stream
 * @return CHIP_ERROR_INVALID_ARGUMENT or CHIP_ERROR_BUFFER_TOO_SMALL on error, CHIP_NO_ERROR otherwise
 */
CHIP_ERROR ReadDerLength(chip::Encoding::LittleEndian::Reader & reader, size_t & length);

/**
 * @brief Utility to emit a DER-encoded INTEGER given a raw unsigned large integer
 *        in big-endian order. The `out_der_integer` span is updated to reflect the final
 *        variable length, including tag and length, and must have at least `kEmitDerIntegerOverhead`
 *        extra space in addition to the `raw_integer.size()`.
 * @param[in] raw_integer Bytes of a large unsigned integer in big-endian, possibly including leading zeroes
 * @param[out] out_der_integer Buffer to receive the DER-encoded integer
 * @return Returns CHIP_ERROR_INVALID_ARGUMENT or CHIP_ERROR_BUFFER_TOO_SMALL on error, CHIP_NO_ERROR otherwise.
 */
CHIP_ERROR ConvertIntegerRawToDer(const ByteSpan & raw_integer, MutableByteSpan & out_der_integer);

/**
 * @brief Utility to emit a DER-encoded INTEGER given a raw unsigned large integer
 *        in big-endian order. The `out_der_integer` span is updated to reflect the final
 *        variable length, excluding tag and length, and must have at least `kEmitDerIntegerWithoutTagOverhead`
 *        extra space in addition to the `raw_integer.size()`.
 * @param[in] raw_integer Bytes of a large unsigned integer in big-endian, possibly including leading zeroes
 * @param[out] out_der_integer Buffer to receive the DER-encoded integer
 * @return Returns CHIP_ERROR_INVALID_ARGUMENT or CHIP_ERROR_BUFFER_TOO_SMALL on error, CHIP_NO_ERROR otherwise.
 */
CHIP_ERROR ConvertIntegerRawToDerWithoutTag(const ByteSpan & raw_integer, MutableByteSpan & out_der_integer);

/**
 * @brief A function that implements AES-CCM encryption
 *
 * This implements the CHIP_Crypto_AEAD_GenerateEncrypt() cryptographic primitive
 * from the specification. For an empty plaintext, the user of the API can provide
 * an empty string, or a nullptr, and provide plaintext_length as 0. The output buffer,
 * ciphertext can also be an empty string, or a nullptr for this case.
 *
 * @param plaintext Plaintext to encrypt
 * @param plaintext_length Length of plain_text
 * @param aad Additional authentication data
 * @param aad_length Length of additional authentication data
 * @param key Encryption key
 * @param nonce Encryption nonce
 * @param nonce_length Length of encryption nonce
 * @param ciphertext Buffer to write ciphertext into. Caller must ensure this is large enough to hold the ciphertext
 * @param tag Buffer to write tag into. Caller must ensure this is large enough to hold the tag
 * @param tag_length Expected length of tag
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 * */
CHIP_ERROR AES_CCM_encrypt(const uint8_t * plaintext, size_t plaintext_length, const uint8_t * aad, size_t aad_length,
                           const Aes128KeyHandle & key, const uint8_t * nonce, size_t nonce_length, uint8_t * ciphertext,
                           uint8_t * tag, size_t tag_length);

/**
 * @brief A function that implements AES-CCM decryption
 *
 * This implements the CHIP_Crypto_AEAD_DecryptVerify() cryptographic primitive
 * from the specification. For an empty ciphertext, the user of the API can provide
 * an empty string, or a nullptr, and provide ciphertext_length as 0. The output buffer,
 * plaintext can also be an empty string, or a nullptr for this case.
 *
 * @param ciphertext Ciphertext to decrypt
 * @param ciphertext_length Length of ciphertext
 * @param aad Additional authentical data.
 * @param aad_length Length of additional authentication data
 * @param tag Tag to use to decrypt
 * @param tag_length Length of tag
 * @param key Decryption key
 * @param nonce Encryption nonce
 * @param nonce_length Length of encryption nonce
 * @param plaintext Buffer to write plaintext into
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/
CHIP_ERROR AES_CCM_decrypt(const uint8_t * ciphertext, size_t ciphertext_length, const uint8_t * aad, size_t aad_length,
                           const uint8_t * tag, size_t tag_length, const Aes128KeyHandle & key, const uint8_t * nonce,
                           size_t nonce_length, uint8_t * plaintext);

/**
 * @brief A function that implements AES-CTR encryption/decryption
 *
 * This implements the AES-CTR-Encrypt/Decrypt() cryptographic primitives per sections
 * 3.7.1 and 3.7.2 of the specification. For an empty input, the user of the API
 * can provide an empty string, or a nullptr, and provide input as 0.
 * The output buffer can also be an empty string, or a nullptr for this case.
 *
 * @param input Input text to encrypt/decrypt
 * @param input_length Length of ciphertext
 * @param key Decryption key
 * @param nonce Encryption nonce
 * @param nonce_length Length of encryption nonce
 * @param output Buffer to write output into
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/
CHIP_ERROR AES_CTR_crypt(const uint8_t * input, size_t input_length, const Aes128KeyHandle & key, const uint8_t * nonce,
                         size_t nonce_length, uint8_t * output);

/**
 * @brief Generate a PKCS#10 CSR, usable for Matter, from a P256Keypair.
 *
 * This uses first principles ASN.1 encoding to avoid relying on the CHIPCryptoPAL backend
 * itself, other than to provide an implementation of a P256Keypair * that supports
 * at least `::Pubkey()` and `::ECDSA_sign_msg`. This allows using it with
 * OS/Platform-bridged private key handling, without requiring a specific
 * implementation of other bits like ASN.1.
 *
 * The CSR will have subject OU set to `CSA`. This is needed since omiting
 * subject altogether often trips CSR parsing code. The profile at the CA can
 * be configured to ignore CSR requested subject.
 *
 * @param keypair The key pair for which a CSR should be generated. Must not be null.
 * @param csr_span Span to hold the resulting CSR. Must have size at least kMIN_CSR_Buffer_Size.
 *                 Otherwise returns CHIP_ERROR_BUFFER_TOO_SMALL. It will get resized to
 *                 actual size needed on success.

 * @return Returns a CHIP_ERROR from P256Keypair or ASN.1 backend on error, CHIP_NO_ERROR otherwise
 **/
CHIP_ERROR GenerateCertificateSigningRequest(const P256Keypair * keypair, MutableByteSpan & csr_span);

/**
 * @brief Common code to validate ASN.1 format/size of a CSR, used by VerifyCertificateSigningRequest.
 *
 * Ensures it's not obviously malformed and doesn't have trailing garbage.
 *
 * @param csr CSR in DER format
 * @param csr_length The length of the CSR buffer
 * @return CHIP_ERROR_UNSUPPORTED_CERT_FORMAT on invalid format, CHIP_NO_ERROR otherwise.
 */
CHIP_ERROR VerifyCertificateSigningRequestFormat(const uint8_t * csr, size_t csr_length);

/**
 * @brief Verify the Certificate Signing Request (CSR). If successfully verified, it outputs the public key from the CSR.
 *
 * The CSR is valid if the format is correct, the signature validates with the embedded public
 * key, and there is no trailing garbage data.
 *
 * @param csr CSR in DER format
 * @param csr_length The length of the CSR
 * @param pubkey The public key from the verified CSR
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/
CHIP_ERROR VerifyCertificateSigningRequest(const uint8_t * csr, size_t csr_length, P256PublicKey & pubkey);

/**
 * @brief A function that implements SHA-256 hash
 *
 * This implements the CHIP_Crypto_Hash() cryptographic primitive
 * in the the specification.
 *
 * @param data The data to hash
 * @param data_length Length of the data
 * @param out_buffer Pointer to buffer to write output into
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/

CHIP_ERROR Hash_SHA256(const uint8_t * data, size_t data_length, uint8_t * out_buffer);

/**
 * @brief A function that implements SHA-1 hash
 * @param data The data to hash
 * @param data_length Length of the data
 * @param out_buffer Pointer to buffer to write output into
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/

CHIP_ERROR Hash_SHA1(const uint8_t * data, size_t data_length, uint8_t * out_buffer);

/**
 * @brief A class that defines stream based implementation of SHA-256 hash
 *        It's expected that the object of this class can be safely copied.
 *        All implementations must check for std::is_trivially_copyable.
 **/

struct alignas(CHIP_CONFIG_SHA256_CONTEXT_ALIGN) HashSHA256OpaqueContext
{
    uint8_t mOpaque[kMAX_Hash_SHA256_Context_Size];
};

class Hash_SHA256_stream
{
public:
    Hash_SHA256_stream();
    ~Hash_SHA256_stream();

    /**
     * @brief Re-initialize digest computation to an empty context.
     *
     * @return CHIP_ERROR_INTERNAL on failure to initialize the context,
     *         CHIP_NO_ERROR otherwise.
     */
    CHIP_ERROR Begin();

    /**
     * @brief Add some data to the digest computation, updating internal state.
     *
     * @param[in] data The span of bytes to include in the digest update process.
     *
     * @return CHIP_ERROR_INTERNAL on failure to ingest the data, CHIP_NO_ERROR otherwise.
     */
    CHIP_ERROR AddData(const ByteSpan data);

    /**
     * @brief Get the intermediate padded digest for the current state of the stream.
     *
     * More data can be added before finish is called.
     *
     * @param[in,out] out_buffer Output buffer to receive the digest. `out_buffer` must
     * be at least `kSHA256_Hash_Length` bytes long. The `out_buffer` size
     * will be set to `kSHA256_Hash_Length` on success.
     *
     * @return CHIP_ERROR_INTERNAL on failure to compute the digest, CHIP_ERROR_BUFFER_TOO_SMALL
     *         if out_buffer is too small, CHIP_NO_ERROR otherwise.
     */
    CHIP_ERROR GetDigest(MutableByteSpan & out_buffer);

    /**
     * @brief Finalize the stream digest computation, getting the final digest.
     *
     * @param[in,out] out_buffer Output buffer to receive the digest. `out_buffer` must
     * be at least `kSHA256_Hash_Length` bytes long. The `out_buffer` size
     * will be set to `kSHA256_Hash_Length` on success.
     *
     * @return CHIP_ERROR_INTERNAL on failure to compute the digest, CHIP_ERROR_BUFFER_TOO_SMALL
     *         if out_buffer is too small, CHIP_NO_ERROR otherwise.
     */
    CHIP_ERROR Finish(MutableByteSpan & out_buffer);

    /**
     * @brief Clear-out internal digest data to avoid lingering the state.
     */
    void Clear();

private:
    HashSHA256OpaqueContext mContext;
};

class HKDF_sha
{
public:
    HKDF_sha()          = default;
    virtual ~HKDF_sha() = default;

    /**
     * @brief A function that implements SHA-256 based HKDF
     *
     * This implements the CHIP_Crypto_KDF() cryptographic primitive
     * in the the specification.
     *
     *  Error values are:
     *   - CHIP_ERROR_INVALID_ARGUMENT: for any bad arguments or nullptr input on
     *     any pointer.
     *   - CHIP_ERROR_INTERNAL: for any unexpected error arising in the underlying
     *     cryptographic layers.
     *
     * @param secret The secret to use as the key to the HKDF
     * @param secret_length Length of the secret
     * @param salt Optional salt to use as input to the HKDF
     * @param salt_length Length of the salt
     * @param info Optional info to use as input to the HKDF
     * @param info_length Length of the info
     * @param out_buffer Pointer to buffer to write output into.
     * @param out_length Size of the underlying out_buffer. That length of output key material will be generated in out_buffer.
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/

    virtual CHIP_ERROR HKDF_SHA256(const uint8_t * secret, size_t secret_length, const uint8_t * salt, size_t salt_length,
                                   const uint8_t * info, size_t info_length, uint8_t * out_buffer, size_t out_length);
};

class HMAC_sha
{
public:
    HMAC_sha()          = default;
    virtual ~HMAC_sha() = default;

    /**
     * @brief A function that implements SHA-256 based HMAC per FIPS1981.
     *
     * This implements the CHIP_Crypto_HMAC() cryptographic primitive
     * in the the specification.
     *
     * The `out_length` must be at least kSHA256_Hash_Length, and only
     * kSHA256_Hash_Length bytes are written to out_buffer.
     *
     * Error values are:
     *   - CHIP_ERROR_INVALID_ARGUMENT: for any bad arguments or nullptr input on
     *     any pointer.
     *   - CHIP_ERROR_INTERNAL: for any unexpected error arising in the underlying
     *     cryptographic layers.
     *
     * @param key The key to use for the HMAC operation
     * @param key_length Length of the key
     * @param message Message over which to compute the HMAC
     * @param message_length Length of the message over which to compute the HMAC
     * @param out_buffer Pointer to buffer into which to write the output.
     * @param out_length Underlying size of the `out_buffer`.
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/

    virtual CHIP_ERROR HMAC_SHA256(const uint8_t * key, size_t key_length, const uint8_t * message, size_t message_length,
                                   uint8_t * out_buffer, size_t out_length);

    /**
     * @brief A function that implements SHA-256 based HMAC per FIPS1981.
     *
     * This implements the CHIP_Crypto_HMAC() cryptographic primitive
     * in the the specification.
     *
     * The `out_length` must be at least kSHA256_Hash_Length, and only
     * kSHA256_Hash_Length bytes are written to out_buffer.
     *
     * Error values are:
     *   - CHIP_ERROR_INVALID_ARGUMENT: for any bad arguments or nullptr input on
     *     any pointer.
     *   - CHIP_ERROR_INTERNAL: for any unexpected error arising in the underlying
     *     cryptographic layers.
     *
     * @param key The HMAC Key handle to use for the HMAC operation
     * @param message Message over which to compute the HMAC
     * @param message_length Length of the message over which to compute the HMAC
     * @param out_buffer Pointer to buffer into which to write the output.
     * @param out_length Underlying size of the `out_buffer`.
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    virtual CHIP_ERROR HMAC_SHA256(const Hmac128KeyHandle & key, const uint8_t * message, size_t message_length,
                                   uint8_t * out_buffer, size_t out_length);
};

/**
 * @brief A cryptographically secure random number generator based on NIST SP800-90A
 * @param out_buffer Buffer into which to write random bytes
 * @param out_length Number of random bytes to generate
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/
CHIP_ERROR DRBG_get_bytes(uint8_t * out_buffer, size_t out_length);

/** @brief Entropy callback function
 * @param data Callback-specific data pointer
 * @param output Output data to fill
 * @param len Length of output buffer
 * @param olen The actual amount of data that was written to output buffer
 * @return 0 if success
 */
typedef int (*entropy_source)(void * data, uint8_t * output, size_t len, size_t * olen);

/** @brief A function to add entropy sources to crypto library
 *
 * This function can be called multiple times to add multiple entropy sources. However,
 * once the entropy source is added, it cannot be removed. Please make sure that the
 * entropy source is valid for the lifetime of the application. Also, make sure that the
 * same entropy source is not added multiple times, e.g.: by calling this function
 * in class constructor or initialization function.
 *
 * @param fn_source Function pointer to the entropy source
 * @param p_source  Data that should be provided when fn_source is called
 * @param threshold Minimum required from source before entropy is released
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/
CHIP_ERROR add_entropy_source(entropy_source fn_source, void * p_source, size_t threshold);

class PBKDF2_sha256
{
public:
    PBKDF2_sha256()          = default;
    virtual ~PBKDF2_sha256() = default;

    /** @brief Function to derive key using password. SHA256 hashing algorithm is used for calculating hmac.
     * @param password password used for key derivation
     * @param plen length of buffer containing password
     * @param salt salt to use as input to the KDF
     * @param slen length of salt
     * @param iteration_count number of iterations to run
     * @param key_length length of output key
     * @param output output buffer where the key will be written
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    virtual CHIP_ERROR pbkdf2_sha256(const uint8_t * password, size_t plen, const uint8_t * salt, size_t slen,
                                     unsigned int iteration_count, uint32_t key_length, uint8_t * output);
};

// TODO: Extract Spake2p to a separate header and replace the forward declaration with #include SessionKeystore.h
class SessionKeystore;

/**
 * The below class implements the draft 01 version of the Spake2+ protocol as
 * defined in https://www.ietf.org/id/draft-bar-cfrg-spake2plus-01.html.
 *
 * The following describes the protocol flows:
 *
 *     Commissioner                     Accessory
 *     ------------                     ---------
 *
 *     Init
 *     BeginProver
 *     ComputeRoundOne  ------------->
 *                                      Init
 *                                      BeginVerifier
 *                                  /-  ComputeRoundOne
 *                      <-------------  ComputeRoundTwo
 *     ComputeRoundTwo  ------------->
 *     KeyConfirm                       KeyConfirm
 *     GetKeys                          GetKeys
 *
 **/
class Spake2p
{
public:
    Spake2p(size_t fe_size, size_t point_size, size_t hash_size);
    virtual ~Spake2p() = default;

    /**
     * @brief Initialize Spake2+ with some context specific information.
     *
     * @param context     The context is arbitrary but should include information about the
     *                    protocol being run, contain the transcript for negotiation, include
     *                    the PKBDF parameters, etc.
     * @param context_len The length of the context.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    virtual CHIP_ERROR Init(const uint8_t * context, size_t context_len);

    /**
     * @brief Free Spake2+ underlying objects.
     **/
    virtual void Clear() = 0;

    /**
     * @brief Start the Spake2+ process as a verifier (i.e. an accessory being provisioned).
     *
     * @param my_identity       The verifier identity. May be NULL if identities are not established.
     * @param my_identity_len   The verifier identity length.
     * @param peer_identity     The peer identity. May be NULL if identities are not established.
     * @param peer_identity_len The peer identity length.
     * @param w0in              The input w0 (a parameter baked into the device or computed with ComputeW0).
     * @param w0in_len          The input w0 length.
     * @param Lin               The input L (a parameter baked into the device or computed with ComputeL).
     * @param Lin_len           The input L length.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    virtual CHIP_ERROR BeginVerifier(const uint8_t * my_identity, size_t my_identity_len, const uint8_t * peer_identity,
                                     size_t peer_identity_len, const uint8_t * w0in, size_t w0in_len, const uint8_t * Lin,
                                     size_t Lin_len);

    /**
     * @brief Start the Spake2+ process as a prover (i.e. a commissioner).
     *
     * @param my_identity       The prover identity. May be NULL if identities are not established.
     * @param my_identity_len   The prover identity length.
     * @param peer_identity     The peer identity. May be NULL if identities are not established.
     * @param peer_identity_len The peer identity length.
     * @param w0in              The input w0 (an output from the PBKDF).
     * @param w0in_len          The input w0 length.
     * @param w1in              The input w1 (an output from the PBKDF).
     * @param w1in_len          The input w1 length.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    virtual CHIP_ERROR BeginProver(const uint8_t * my_identity, size_t my_identity_len, const uint8_t * peer_identity,
                                   size_t peer_identity_len, const uint8_t * w0in, size_t w0in_len, const uint8_t * w1in,
                                   size_t w1in_len);

    /**
     * @brief Compute the first round of the protocol.
     *
     * @param pab      X value from commissioner.
     * @param pab_len  X length.
     * @param out     The output first round Spake2+ contribution.
     * @param out_len The output first round Spake2+ contribution length.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    virtual CHIP_ERROR ComputeRoundOne(const uint8_t * pab, size_t pab_len, uint8_t * out, size_t * out_len);

    /**
     * @brief Compute the second round of the protocol.
     *
     * @param in      The peer first round Spake2+ contribution.
     * @param in_len  The peer first round Spake2+ contribution length.
     * @param out     The output second round Spake2+ contribution.
     * @param out_len The output second round Spake2+ contribution length.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    virtual CHIP_ERROR ComputeRoundTwo(const uint8_t * in, size_t in_len, uint8_t * out, size_t * out_len);

    /**
     * @brief Confirm that each party computed the same keys.
     *
     * @param in     The peer second round Spake2+ contribution.
     * @param in_len The peer second round Spake2+ contribution length.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    virtual CHIP_ERROR KeyConfirm(const uint8_t * in, size_t in_len);

    /**
     * @brief Return the shared HKDF key.
     *
     * Returns the shared key established during the Spake2+ process, which can be used
     * to derive application-specific keys using HKDF.
     *
     * @param keystore The session keystore for managing the HKDF key lifetime.
     * @param key The output HKDF key.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR GetKeys(SessionKeystore & keystore, HkdfKeyHandle & key);

    CHIP_ERROR InternalHash(const uint8_t * in, size_t in_len);
    CHIP_ERROR WriteMN();
    CHIP_ERROR GenerateKeys();

    /**
     * @brief Load a field element.
     *
     *  @param in     The input big endian field element.
     *  @param in_len The size of the input buffer in bytes.
     *  @param fe     A pointer to an initialized implementation dependant field element.
     *
     *  @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    virtual CHIP_ERROR FELoad(const uint8_t * in, size_t in_len, void * fe) = 0;

    /**
     * @brief Write a field element in big-endian format.
     *
     *  @param fe      The field element to write.
     *  @param out     The output buffer.
     *  @param out_len The length of the output buffer.
     *
     *  @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    virtual CHIP_ERROR FEWrite(const void * fe, uint8_t * out, size_t out_len) = 0;

    /**
     * @brief Generate a field element.
     *
     *  @param fe  A pointer to an initialized implementation dependant field element.
     *
     *  @note The implementation must generate a random element from [0, q) where q is the curve order.
     *
     *  @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    virtual CHIP_ERROR FEGenerate(void * fe) = 0;

    /**
     * @brief Multiply two field elements, fer = fe1 * fe2.
     *
     *  @param fer   A pointer to an initialized implementation dependant field element.
     *  @param fe1  A pointer to an initialized implementation dependant field element.
     *  @param fe2  A pointer to an initialized implementation dependant field element.
     *
     *  @note The result must be a field element (i.e. reduced by the curve order).
     *
     *  @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    virtual CHIP_ERROR FEMul(void * fer, const void * fe1, const void * fe2) = 0;

    /**
     * @brief Load a point from 0x04 || X || Y format
     *
     * @param in     Input buffer
     * @param in_len Input buffer length
     * @param R      A pointer to an initialized implementation dependant point.
     *
     *  @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    virtual CHIP_ERROR PointLoad(const uint8_t * in, size_t in_len, void * R) = 0;

    /**
     * @brief Write a point in 0x04 || X || Y format
     *
     * @param R       A pointer to an initialized implementation dependant point.
     * @param out     Output buffer
     * @param out_len Length of the output buffer
     *
     *  @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    virtual CHIP_ERROR PointWrite(const void * R, uint8_t * out, size_t out_len) = 0;

    /**
     * @brief Scalar multiplication, R = fe1 * P1.
     *
     * @param R   Resultant point
     * @param P1  Input point
     * @param fe1  Input field element.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    virtual CHIP_ERROR PointMul(void * R, const void * P1, const void * fe1) = 0;

    /**
     * @brief Scalar multiplication with addition, R = fe1 * P1 + fe2 * P2.
     *
     * @param R   Resultant point
     * @param P1  Input point
     * @param fe1  Input field element.
     * @param P2  Input point
     * @param fe2  Input field element.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    virtual CHIP_ERROR PointAddMul(void * R, const void * P1, const void * fe1, const void * P2, const void * fe2) = 0;

    /**
     * @brief Point inversion.
     *
     * @param R   Input/Output point to point_invert
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    virtual CHIP_ERROR PointInvert(void * R) = 0;

    /**
     * @brief Multiply a point by the curve cofactor.
     *
     * @param R   Input/Output point to point_invert
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    virtual CHIP_ERROR PointCofactorMul(void * R) = 0;

    /*
     *   @synopsis Check if a point is on the curve.
     *
     *   @param R   Input point to check.
     *
     *   @return CHIP_NO_ERROR if the point is valid, CHIP_ERROR otherwise.
     */
    virtual CHIP_ERROR PointIsValid(void * R) = 0;

    /*
     *   @synopsis Compute w0sin mod p
     *
     *   @param w0out       Output field element (modulo p)
     *   @param w0_len      Output field element length
     *   @param w1sin       Input field element
     *   @param w1sin_len   Input field element length
     *
     *   @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    virtual CHIP_ERROR ComputeW0(uint8_t * w0out, size_t * w0_len, const uint8_t * w0sin, size_t w0sin_len) = 0;

    /*
     *   @synopsis Compute w1in*G
     *
     *   @param Lout        Output point in 0x04 || X || Y format.
     *   @param L_len       Output point length
     *   @param w1in        Input field element
     *   @param w1in_len    Input field element size
     *
     *   @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    virtual CHIP_ERROR ComputeL(uint8_t * Lout, size_t * L_len, const uint8_t * w1in, size_t w1in_len) = 0;

    void * M;
    void * N;
    const void * G;
    void * X;
    void * Y;
    void * L;
    void * Z;
    void * V;
    void * w0;
    void * w1;
    void * xy;
    void * order;
    void * tempbn;

protected:
    /**
     * @brief Initialize underlying implementation curve, points, field elements, etc.
     *
     * @details The implementation needs to:
     *     1. Initialize each of the points below and set the relevant pointers on the class:
     *        a. M
     *        b. N
     *        c. G
     *        d. X
     *        e. Y
     *        f. L
     *        g. Z
     *        h. V
     *
     *        As an example:
     *           this.M = implementation_alloc_point();
     *     2. Initialize each of the field elements below and set the relevant pointers on the class:
     *        a. w0
     *        b. w1
     *        c. xy
     *        d. tempbn
     *     3. The hashing context should be initialized
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    virtual CHIP_ERROR InitImpl() = 0;

    /**
     * @brief Hash in_len bytes of in into the internal hash context.
     *
     * @param in     The input buffer.
     * @param in_len Size of the input buffer in bytes.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    virtual CHIP_ERROR Hash(const uint8_t * in, size_t in_len) = 0;

    /**
     * @brief Return the hash.
     *
     * @param out_span Output buffer. The size available must be >= the hash size. It gets resized
     *                 to hash size on success.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    virtual CHIP_ERROR HashFinalize(MutableByteSpan & out_span) = 0;

    /**
     * @brief Generate a message authentication code.
     *
     * @param key      The MAC key buffer.
     * @param key_len  The size of the MAC key in bytes.
     * @param in       The input buffer.
     * @param in_len   The size of the input data to MAC in bytes.
     * @param out_span The output MAC buffer span. Size must be >= the hash_size. Output size is updated to fit on success.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    virtual CHIP_ERROR Mac(const uint8_t * key, size_t key_len, const uint8_t * in, size_t in_len, MutableByteSpan & out_span) = 0;

    /**
     * @brief Verify a message authentication code.
     *
     *  @param key     The MAC key buffer.
     *  @param key_len The size of the MAC key in bytes.
     *  @param mac     The input MAC buffer.
     *  @param mac_len The size of the MAC in bytes.
     *  @param in      The input buffer to verify.
     *  @param in_len  The size of the input data to verify in bytes.
     *
     *  @return Returns a CHIP_ERROR when the MAC doesn't validate, CHIP_NO_ERROR otherwise.
     **/
    virtual CHIP_ERROR MacVerify(const uint8_t * key, size_t key_len, const uint8_t * mac, size_t mac_len, const uint8_t * in,
                                 size_t in_len) = 0;

    /**
     * @brief Derive an key of length out_len.
     *
     * @param ikm      The input key material buffer.
     * @param ikm_len  The input key material length.
     * @param salt     The optional salt. This may be NULL.
     * @param salt_len The size of the salt in bytes.
     * @param info     The info.
     * @param info_len The size of the info in bytes.
     * @param out      The output key
     * @param out_len  The output key length
     *
     * @return Returns a CHIP_ERROR when the MAC doesn't validate, CHIP_NO_ERROR otherwise.
     **/
    virtual CHIP_ERROR KDF(const uint8_t * ikm, size_t ikm_len, const uint8_t * salt, size_t salt_len, const uint8_t * info,
                           size_t info_len, uint8_t * out, size_t out_len) = 0;

    CHIP_SPAKE2P_ROLE role;
    CHIP_SPAKE2P_STATE state = CHIP_SPAKE2P_STATE::PREINIT;
    size_t fe_size;
    size_t hash_size;
    size_t point_size;
    uint8_t Kcab[kMAX_Hash_Length];
    uint8_t Kae[kMAX_Hash_Length];
    uint8_t * Kca;
    uint8_t * Kcb;
    uint8_t * Ka;
    uint8_t * Ke;
};

struct alignas(size_t) Spake2pOpaqueContext
{
    uint8_t mOpaque[kMAX_Spake2p_Context_Size];
};

class Spake2p_P256_SHA256_HKDF_HMAC : public Spake2p
{
public:
    Spake2p_P256_SHA256_HKDF_HMAC() : Spake2p(kP256_FE_Length, kP256_Point_Length, kSHA256_Hash_Length)
    {
        memset(&mSpake2pContext, 0, sizeof(mSpake2pContext));
    }

    ~Spake2p_P256_SHA256_HKDF_HMAC() override { Spake2p_P256_SHA256_HKDF_HMAC::Clear(); }

    void Clear() override;
    CHIP_ERROR Mac(const uint8_t * key, size_t key_len, const uint8_t * in, size_t in_len, MutableByteSpan & out_span) override;
    CHIP_ERROR MacVerify(const uint8_t * key, size_t key_len, const uint8_t * mac, size_t mac_len, const uint8_t * in,
                         size_t in_len) override;
    CHIP_ERROR FELoad(const uint8_t * in, size_t in_len, void * fe) override;
    CHIP_ERROR FEWrite(const void * fe, uint8_t * out, size_t out_len) override;
    CHIP_ERROR FEGenerate(void * fe) override;
    CHIP_ERROR FEMul(void * fer, const void * fe1, const void * fe2) override;

    CHIP_ERROR PointLoad(const uint8_t * in, size_t in_len, void * R) override;
    CHIP_ERROR PointWrite(const void * R, uint8_t * out, size_t out_len) override;
    CHIP_ERROR PointMul(void * R, const void * P1, const void * fe1) override;
    CHIP_ERROR PointAddMul(void * R, const void * P1, const void * fe1, const void * P2, const void * fe2) override;
    CHIP_ERROR PointInvert(void * R) override;
    CHIP_ERROR PointCofactorMul(void * R) override;
    CHIP_ERROR PointIsValid(void * R) override;

    CHIP_ERROR ComputeW0(uint8_t * w0out, size_t * w0_len, const uint8_t * w0sin, size_t w0sin_len) override;
    CHIP_ERROR ComputeL(uint8_t * Lout, size_t * L_len, const uint8_t * w1in, size_t w1in_len) override;

protected:
    CHIP_ERROR InitImpl() override;
    CHIP_ERROR Hash(const uint8_t * in, size_t in_len) override;
    CHIP_ERROR HashFinalize(MutableByteSpan & out_span) override;
    CHIP_ERROR KDF(const uint8_t * secret, size_t secret_length, const uint8_t * salt, size_t salt_length, const uint8_t * info,
                   size_t info_length, uint8_t * out, size_t out_length) override;

private:
    CHIP_ERROR InitInternal();
    Hash_SHA256_stream sha256_hash_ctx;

    Spake2pOpaqueContext mSpake2pContext;
};

/**
 * @brief Class used for verifying PASE secure sessions.
 **/
class Spake2pVerifier
{
public:
    uint8_t mW0[kP256_FE_Length];
    uint8_t mL[kP256_Point_Length];

    CHIP_ERROR Serialize(MutableByteSpan & outSerialized) const;
    CHIP_ERROR Deserialize(const ByteSpan & inSerialized);

    /**
     * @brief Generate the Spake2+ verifier.
     *
     * @param pbkdf2IterCount Iteration count for PBKDF2 function
     * @param salt            Salt to be used for Spake2+ operation
     * @param setupPin        Provided setup PIN (passcode)
     *
     * @return CHIP_ERROR     The result of Spake2+ verifier generation
     */
    CHIP_ERROR Generate(uint32_t pbkdf2IterCount, const ByteSpan & salt, uint32_t setupPin);

    /**
     * @brief Compute the initiator values (w0, w1) used for PAKE input.
     *
     * @param pbkdf2IterCount Iteration count for PBKDF2 function
     * @param salt            Salt to be used for Spake2+ operation
     * @param setupPin        Provided setup PIN (passcode)
     * @param ws              The output pair (w0, w1) stored sequentially
     * @param ws_len          The output length
     *
     * @return CHIP_ERROR     The result from running PBKDF2
     */
    static CHIP_ERROR ComputeWS(uint32_t pbkdf2IterCount, const ByteSpan & salt, uint32_t setupPin, uint8_t * ws, uint32_t ws_len);
};

/**
 * @brief Serialized format of the Spake2+ Verifier components.
 *
 *  This is used when the Verifier should be presented in a serialized form.
 *  For example, when it is generated using PBKDF function, when stored in the
 *  memory or when sent over the wire.
 *  The serialized format is concatentation of 'W0' and 'L' verifier components:
 *      { Spake2pVerifier.mW0[kP256_FE_Length], Spake2pVerifier.mL[kP256_Point_Length] }
 **/
typedef uint8_t Spake2pVerifierSerialized[kSpake2p_VerifierSerialized_Length];

/**
 * @brief Compute the compressed fabric identifier used for operational discovery service
 *        records from a Node's root public key and Fabric ID. On success, out_compressed_fabric_id
 *        will have a size of exactly kCompressedFabricIdentifierSize.
 *
 * Errors are:
 *   - CHIP_ERROR_INVALID_ARGUMENT if root_public_key is invalid
 *   - CHIP_ERROR_BUFFER_TOO_SMALL if out_compressed_fabric_id is too small for serialization
 *   - CHIP_ERROR_INTERNAL on any unexpected crypto or data conversion errors.
 *
 * @param[in] root_public_key The root public key associated with the node's fabric
 * @param[in] fabric_id The fabric ID associated with the node's fabric
 * @param[out] out_compressed_fabric_id Span where output will be written. Its size must be >= kCompressedFabricIdentifierSize.
 * @returns a CHIP_ERROR (see above) on failure or CHIP_NO_ERROR otherwise.
 */
CHIP_ERROR GenerateCompressedFabricId(const Crypto::P256PublicKey & root_public_key, uint64_t fabric_id,
                                      MutableByteSpan & out_compressed_fabric_id);

/**
 * @brief Compute the compressed fabric identifier used for operational discovery service
 *        records from a Node's root public key and Fabric ID.  This is a conveniance
 *        overload that writes to a uint64_t (CompressedFabricId) type.
 *
 * @param[in] rootPublicKey The root public key associated with the node's fabric
 * @param[in] fabricId The fabric ID associated with the node's fabric
 * @param[out] compressedFabricId output location for compressed fabric ID
 * @returns a CHIP_ERROR on failure or CHIP_NO_ERROR otherwise.
 */
CHIP_ERROR GenerateCompressedFabricId(const Crypto::P256PublicKey & rootPublicKey, uint64_t fabricId,
                                      uint64_t & compressedFabricId);

enum class CertificateChainValidationResult
{
    kSuccess = 0,

    kRootFormatInvalid   = 100,
    kRootArgumentInvalid = 101,

    kICAFormatInvalid   = 200,
    kICAArgumentInvalid = 201,

    kLeafFormatInvalid   = 300,
    kLeafArgumentInvalid = 301,

    kChainInvalid = 400,

    kNoMemory = 500,

    kInternalFrameworkError = 600,
};

CHIP_ERROR ValidateCertificateChain(const uint8_t * rootCertificate, size_t rootCertificateLen, const uint8_t * caCertificate,
                                    size_t caCertificateLen, const uint8_t * leafCertificate, size_t leafCertificateLen,
                                    CertificateChainValidationResult & result);

enum class AttestationCertType
{
    kPAA = 0,
    kPAI = 1,
    kDAC = 2,
};

CHIP_ERROR VerifyAttestationCertificateFormat(const ByteSpan & cert, AttestationCertType certType);

/**
 * @brief Validate notBefore timestamp of a certificate (candidateCertificate) against validity period of the
 *        issuer certificate (issuerCertificate).
 *
 * Errors are:
 *   - CHIP_ERROR_CERT_EXPIRED if the candidateCertificate timestamp does not satisfy the issuerCertificate's timestamp.
 *   - CHIP_ERROR_INVALID_ARGUMENT when passing an invalid argument.
 *   - CHIP_ERROR_INTERNAL on any unexpected crypto or data conversion errors.
 *
 *  @param candidateCertificate     A DER Certificate ByteSpan those notBefore timestamp to be evaluated.
 *  @param issuerCertificate        A DER Certificate ByteSpan used to evaluate validity timestamp of the candidateCertificate.
 *
 *  @returns a CHIP_ERROR (see above) on failure or CHIP_NO_ERROR otherwise.
 **/
CHIP_ERROR IsCertificateValidAtIssuance(const ByteSpan & candidateCertificate, const ByteSpan & issuerCertificate);

/**
 * @brief Validate a certificate's validity date against current time.
 *
 * Errors are:
 *   - CHIP_ERROR_CERT_EXPIRED if the certificate has expired.
 *   - CHIP_ERROR_INVALID_ARGUMENT when passing an invalid argument.
 *   - CHIP_ERROR_INTERNAL on any unexpected crypto or data conversion errors.
 *
 *  @param certificate A DER Certificate ByteSpan used as the validity reference to be checked against current time.
 *
 *  @returns a CHIP_ERROR (see above) on failure or CHIP_NO_ERROR otherwise.
 **/
CHIP_ERROR IsCertificateValidAtCurrentTime(const ByteSpan & certificate);

CHIP_ERROR ExtractPubkeyFromX509Cert(const ByteSpan & certificate, Crypto::P256PublicKey & pubkey);

/**
 * @brief Extracts the Subject Key Identifier from an X509 Certificate.
 **/
CHIP_ERROR ExtractSKIDFromX509Cert(const ByteSpan & certificate, MutableByteSpan & skid);

/**
 * @brief Extracts the Authority Key Identifier from an X509 Certificate.
 **/
CHIP_ERROR ExtractAKIDFromX509Cert(const ByteSpan & certificate, MutableByteSpan & akid);

/**
 * @brief Extracts the CRL Distribution Point (CDP) extension from an X509 ASN.1 Encoded Certificate.
 *        The returned value only covers the URI of the CDP. Only a single URI distribution point
 *        GeneralName is supported, and only those that start with "http://" and "https://".
 *
 * @returns CHIP_ERROR_NOT_FOUND if not found or wrong format.
 *          CHIP_NO_ERROR otherwise.
 **/
CHIP_ERROR ExtractCRLDistributionPointURIFromX509Cert(const ByteSpan & certificate, MutableCharSpan & cdpurl);

/**
 * @brief Extracts the CRL Distribution Point (CDP) extension's cRLIssuer Name from an X509 ASN.1 Encoded Certificate.
 *        The value is copied into buffer in a raw ASN.1 X.509 format. This format should be directly comparable
 *        with the result of ExtractSubjectFromX509Cert().
 *
 * @returns CHIP_ERROR_NOT_FOUND if not found or wrong format.
 *          CHIP_NO_ERROR otherwise.
 **/
CHIP_ERROR ExtractCDPExtensionCRLIssuerFromX509Cert(const ByteSpan & certificate, MutableByteSpan & crlIssuer);

/**
 * @brief Extracts Serial Number from X509 Certificate.
 **/
CHIP_ERROR ExtractSerialNumberFromX509Cert(const ByteSpan & certificate, MutableByteSpan & serialNumber);

/**
 * @brief Extracts Subject Distinguished Name from X509 Certificate. The value is copied into buffer in a raw ASN.1 X.509 format.
 **/
CHIP_ERROR ExtractSubjectFromX509Cert(const ByteSpan & certificate, MutableByteSpan & subject);

/**
 * @brief Extracts Issuer Distinguished Name from X509 Certificate. The value is copied into buffer in a raw ASN.1 X.509 format.
 **/
CHIP_ERROR ExtractIssuerFromX509Cert(const ByteSpan & certificate, MutableByteSpan & issuer);

/**
 * @brief Checks for resigned version of the certificate in the list and returns it.
 *
 * The following conditions SHOULD be satisfied for the certificate to qualify as
 * a resigned version of a reference certificate:
 *   - SKID of the candidate and the reference certificate should match.
 *   - SubjectDN of the candidate and the reference certificate should match.
 *
 * If no resigned version is found then reference certificate itself is returned.
 *
 *  @param referenceCertificate       A DER certificate.
 *  @param candidateCertificates      A pointer to the list of DER Certificates, which should be searched
 *                                    for the resigned version of `referenceCertificate`.
 *  @param candidateCertificatesCount Number of certificates in the `candidateCertificates` list.
 *  @param outCertificate             A reference to the certificate or it's resigned version if found.
 *                                    Note that it points to either `referenceCertificate` or one of
 *                                    `candidateCertificates`, but it doesn't copy data.
 *
 *  @returns error if there is certificate parsing/format issue or CHIP_NO_ERROR otherwise.
 **/
CHIP_ERROR ReplaceCertIfResignedCertFound(const ByteSpan & referenceCertificate, const ByteSpan * candidateCertificates,
                                          size_t candidateCertificatesCount, ByteSpan & outCertificate);

/**
 * Defines DN attribute types that can include endocing of VID/PID parameters.
 */
enum class DNAttrType
{
    kUnspecified = 0,
    kCommonName  = 1,
    kMatterVID   = 2,
    kMatterPID   = 3,
};

/**
 *  @struct AttestationCertVidPid
 *
 *  @brief
 *    A data structure representing Attestation Certificate VID and PID attributes.
 */
struct AttestationCertVidPid
{
    Optional<VendorId> mVendorId;
    Optional<uint16_t> mProductId;

    bool Initialized() const { return (mVendorId.HasValue() || mProductId.HasValue()); }
};

/**
 * @brief Extracts VID and PID attributes from the DN Attribute string.
 *        If attribute is not present the corresponding output value stays uninitialized.
 *
 * @return CHIP_ERROR_INVALID_ARGUMENT if wrong input is provided.
 *         CHIP_ERROR_WRONG_CERT_DN if encoding of kMatterVID and kMatterPID attributes is wrong.
 *         CHIP_NO_ERROR otherwise.
 **/
CHIP_ERROR ExtractVIDPIDFromAttributeString(DNAttrType attrType, const ByteSpan & attr,
                                            AttestationCertVidPid & vidpidFromMatterAttr, AttestationCertVidPid & vidpidFromCNAttr);

/**
 * @brief Extracts VID and PID attributes from the Subject DN of an X509 Certificate.
 *        If attribute is not present the corresponding output value stays uninitialized.
 **/
CHIP_ERROR ExtractVIDPIDFromX509Cert(const ByteSpan & x509Cert, AttestationCertVidPid & vidpid);

/**
 * @brief The set of credentials needed to operate group message security with symmetric keys.
 */
typedef struct GroupOperationalCredentials
{
    /// Validity start time in microseconds since 2000-01-01T00:00:00 UTC ("the Epoch")
    uint64_t start_time;
    /// Session Id
    uint16_t hash;
    /// Operational group key
    uint8_t encryption_key[Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES];
    /// Privacy key
    uint8_t privacy_key[Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES];
} GroupOperationalCredentials;

/**
 * @brief Opaque context used to protect a symmetric key. The key operations must
 *        be performed without exposing the protected key value.
 */
class SymmetricKeyContext
{
public:
    /**
     * @brief Returns the symmetric key hash
     *
     * TODO: Replace GetKeyHash() with DeriveGroupSessionId(SymmetricKeyContext &, uint16_t & session_id)
     *
     * @return Group Key Hash
     */
    virtual uint16_t GetKeyHash() = 0;

    virtual ~SymmetricKeyContext() = default;
    /**
     * @brief Perform the message encryption as described in 4.7.2. (Security Processing of Outgoing Messages)
     * @param[in] plaintext     Outgoing message payload.
     * @param[in] aad           Additional data (message header contents)
     * @param[in] nonce         Nonce (Security Flags | Message Counter | Source Node ID)
     * @param[out] mic          Outgoing Message Integrity Check
     * @param[out] ciphertext   Outgoing encrypted payload. Must be at least as big as plaintext. The same buffer may be used both
     * for ciphertext, and plaintext.
     * @return CHIP_ERROR
     */
    virtual CHIP_ERROR MessageEncrypt(const ByteSpan & plaintext, const ByteSpan & aad, const ByteSpan & nonce,
                                      MutableByteSpan & mic, MutableByteSpan & ciphertext) const = 0;
    /**
     * @brief Perform the message decryption as described in 4.7.3.(Security Processing of Incoming Messages)
     * @param[in] ciphertext    Incoming encrypted payload
     * @param[in] aad           Additional data (message header contents)
     * @param[in] nonce         Nonce (Security Flags | Message Counter | Source Node ID)
     * @param[in] mic           Incoming Message Integrity Check
     * @param[out] plaintext     Incoming message payload. Must be at least as big as ciphertext. The same buffer may be used both
     * for plaintext, and ciphertext.
     * @return CHIP_ERROR
     */
    virtual CHIP_ERROR MessageDecrypt(const ByteSpan & ciphertext, const ByteSpan & aad, const ByteSpan & nonce,
                                      const ByteSpan & mic, MutableByteSpan & plaintext) const = 0;

    /**
     * @brief Perform privacy encoding as described in 4.8.2. (Privacy Processing of Outgoing Messages)
     * @param[in] input         Message header to privacy encrypt
     * @param[in] nonce         Privacy Nonce = session_id | mic
     * @param[out] output       Message header obfuscated
     * @return CHIP_ERROR
     */
    virtual CHIP_ERROR PrivacyEncrypt(const ByteSpan & input, const ByteSpan & nonce, MutableByteSpan & output) const = 0;

    /**
     * @brief Perform privacy decoding as described in 4.8.3. (Privacy Processing of Incoming Messages)
     * @param[in] input         Message header to privacy decrypt
     * @param[in] nonce         Privacy Nonce = session_id | mic
     * @param[out] output       Message header deobfuscated
     * @return CHIP_ERROR
     */
    virtual CHIP_ERROR PrivacyDecrypt(const ByteSpan & input, const ByteSpan & nonce, MutableByteSpan & output) const = 0;

    /**
     * @brief Release resources such as dynamic memory used to allocate this instance of the SymmetricKeyContext
     */
    virtual void Release() = 0;
};

/**
 *  @brief Derives the Operational Group Key using the Key Derivation Function (KDF) from the given epoch key.
 * @param[in] epoch_key  The epoch key. Must be CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES bytes length.
 * @param[in] compressed_fabric_id The compressed fabric ID for the fabric (big endian byte string)
 * @param[out] out_key  Symmetric key used as the encryption key during message processing for group communication.
 The buffer size must be at least CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES bytes length.
 * @return Returns a CHIP_NO_ERROR on succcess, or CHIP_ERROR_INTERNAL if the provided key is invalid.
 **/
CHIP_ERROR DeriveGroupOperationalKey(const ByteSpan & epoch_key, const ByteSpan & compressed_fabric_id, MutableByteSpan & out_key);

/**
 *  @brief Derives the Group Session ID from a given operational group key using
 *         the Key Derivation Function (Group Key Hash)
 * @param[in] operational_key  The operational group key. Must be CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES bytes length.
 * @param[out] session_id  Output of the Group Key Hash
 * @return Returns a CHIP_NO_ERROR on succcess, or CHIP_ERROR_INVALID_ARGUMENT if the provided key is invalid.
 **/
CHIP_ERROR DeriveGroupSessionId(const ByteSpan & operational_key, uint16_t & session_id);

/**
 *  @brief Derives the Privacy Group Key using the Key Derivation Function (KDF) from the given epoch key.
 * @param[in] epoch_key  The epoch key. Must be CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES bytes length.
 * @param[out] out_key  Symmetric key used as the privacy key during message processing for group communication.
 *                      The buffer size must be at least CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES bytes length.
 * @return Returns a CHIP_NO_ERROR on succcess, or CHIP_ERROR_INTERNAL if the provided key is invalid.
 **/
CHIP_ERROR DeriveGroupPrivacyKey(const ByteSpan & epoch_key, MutableByteSpan & out_key);

/**
 *  @brief Derives the complete set of credentials needed for group security.
 *
 * This function will derive the Encryption Key, Group Key Hash (Session Id), and Privacy Key
 * for the given Epoch Key and Compressed Fabric Id.
 * @param[in] epoch_key  The epoch key. Must be CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES bytes length.
 * @param[in] compressed_fabric_id The compressed fabric ID for the fabric (big endian byte string)
 * @param[out] operational_credentials The set of Symmetric keys used during message processing for group communication.
 * @return Returns a CHIP_NO_ERROR on succcess, or CHIP_ERROR_INTERNAL if the provided key is invalid.
 **/
CHIP_ERROR DeriveGroupOperationalCredentials(const ByteSpan & epoch_key, const ByteSpan & compressed_fabric_id,
                                             GroupOperationalCredentials & operational_credentials);
} // namespace Crypto
} // namespace chip
