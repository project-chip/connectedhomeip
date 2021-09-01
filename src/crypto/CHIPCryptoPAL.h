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

#if CHIP_HAVE_CONFIG_H
#include <crypto/CryptoBuildConfig.h>
#endif

#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>

#include <stddef.h>
#include <string.h>

namespace chip {
namespace Crypto {

constexpr size_t kMax_x509_Certificate_Length = 600;

constexpr size_t kP256_FE_Length                  = 32;
constexpr size_t kP256_ECDSA_Signature_Length_Raw = (2 * kP256_FE_Length);
constexpr size_t kP256_Point_Length               = (2 * kP256_FE_Length + 1);
constexpr size_t kSHA256_Hash_Length              = 32;
constexpr size_t kSHA1_Hash_Length                = 20;

constexpr size_t CHIP_CRYPTO_GROUP_SIZE_BYTES      = kP256_FE_Length;
constexpr size_t CHIP_CRYPTO_PUBLIC_KEY_SIZE_BYTES = kP256_Point_Length;

constexpr size_t kMax_ECDH_Secret_Length     = kP256_FE_Length;
constexpr size_t kMax_ECDSA_Signature_Length = kP256_ECDSA_Signature_Length_Raw;
constexpr size_t kMAX_FE_Length              = kP256_FE_Length;
constexpr size_t kMAX_Point_Length           = kP256_Point_Length;
constexpr size_t kMAX_Hash_Length            = kSHA256_Hash_Length;
constexpr size_t kMAX_CSR_Length             = 512;

constexpr size_t CHIP_CRYPTO_HASH_LEN_BYTES = kSHA256_Hash_Length;

constexpr size_t kMin_Salt_Length = 8;
constexpr size_t kMax_Salt_Length = 16;

constexpr size_t kP256_PrivateKey_Length = CHIP_CRYPTO_GROUP_SIZE_BYTES;
constexpr size_t kP256_PublicKey_Length  = CHIP_CRYPTO_PUBLIC_KEY_SIZE_BYTES;

/* These sizes are hardcoded here to remove header dependency on underlying crypto library
 * in a public interface file. The validity of these sizes is verified by static_assert in
 * the implementation files.
 */
constexpr size_t kMAX_Spake2p_Context_Size     = 1024;
constexpr size_t kMAX_P256Keypair_Context_Size = 512;

constexpr size_t kEmitDerIntegerWithoutTagOverhead = 1; // 1 sign stuffer
constexpr size_t kEmitDerIntegerOverhead           = 3; // Tag + Length byte + 1 sign stuffer

/*
 * Worst case is OpenSSL, so let's use its worst case and let static assert tell us if
 * we are wrong, since `typedef SHA_LONG unsigned int` is default.
 *   SHA_LONG h[8];
 *   SHA_LONG Nl, Nh;
 *   SHA_LONG data[SHA_LBLOCK]; // SHA_LBLOCK is 16 for SHA256
 *   unsigned int num, md_len;
 *
 * We also have to account for possibly some custom extensions on some targets,
 * especially for mbedTLS, so an extra sizeof(uint64_t) is added to account.
 */
constexpr size_t kMAX_Hash_SHA256_Context_Size = ((sizeof(unsigned int) * (8 + 2 + 16 + 2)) + sizeof(uint64_t));

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
constexpr size_t kMax_ECDSA_X9Dot62_Asn1_Overhead = 9;
constexpr size_t kMax_ECDSA_Signature_Length_Der  = kMax_ECDSA_Signature_Length + kMax_ECDSA_X9Dot62_Asn1_Overhead;

static_assert(kMax_ECDH_Secret_Length >= kP256_FE_Length, "ECDH shared secret is too short for crypto suite");
static_assert(kMax_ECDSA_Signature_Length >= kP256_ECDSA_Signature_Length_Raw,
              "ECDSA signature buffer length is too short for crypto suite");

constexpr size_t kCompressedFabricIdentifierSize = 8;

/**
 * Spake2+ parameters for P256
 * Defined in https://www.ietf.org/id/draft-bar-cfrg-spake2plus-01.html#name-ciphersuites
 */
const uint8_t spake2p_M_p256[65] = {
    0x04, 0x88, 0x6e, 0x2f, 0x97, 0xac, 0xe4, 0x6e, 0x55, 0xba, 0x9d, 0xd7, 0x24, 0x25, 0x79, 0xf2, 0x99,
    0x3b, 0x64, 0xe1, 0x6e, 0xf3, 0xdc, 0xab, 0x95, 0xaf, 0xd4, 0x97, 0x33, 0x3d, 0x8f, 0xa1, 0x2f, 0x5f,
    0xf3, 0x55, 0x16, 0x3e, 0x43, 0xce, 0x22, 0x4e, 0x0b, 0x0e, 0x65, 0xff, 0x02, 0xac, 0x8e, 0x5c, 0x7b,
    0xe0, 0x94, 0x19, 0xc7, 0x85, 0xe0, 0xca, 0x54, 0x7d, 0x55, 0xa1, 0x2e, 0x2d, 0x20,
};
const uint8_t spake2p_N_p256[65] = {
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

template <typename Sig>
class ECPKey
{
public:
    virtual ~ECPKey() {}
    virtual SupportedECPKeyTypes Type() const  = 0;
    virtual size_t Length() const              = 0;
    virtual bool IsUncompressed() const        = 0;
    virtual operator const uint8_t *() const   = 0;
    virtual operator uint8_t *()               = 0;
    virtual const uint8_t * ConstBytes() const = 0;
    virtual uint8_t * Bytes()                  = 0;

    virtual CHIP_ERROR ECDSA_validate_msg_signature(const uint8_t * msg, const size_t msg_length, const Sig & signature) const = 0;
    virtual CHIP_ERROR ECDSA_validate_hash_signature(const uint8_t * hash, const size_t hash_length,
                                                     const Sig & signature) const                                              = 0;
};

template <size_t Cap>
class CapacityBoundBuffer
{
public:
    /** @brief Set current length of the buffer that's being used
     * @return Returns error if new length is > capacity
     **/
    CHIP_ERROR SetLength(size_t len)
    {
        VerifyOrReturnError(len <= sizeof(bytes), CHIP_ERROR_INVALID_ARGUMENT);
        length = len;
        return CHIP_NO_ERROR;
    }

    /** @brief Returns current length of the buffer that's being used
     * @return Returns 0 if SetLength() was never called
     **/
    size_t Length() const { return length; }

    /** @brief Returns max capacity of the buffer
     **/
    static constexpr size_t Capacity() { return sizeof(bytes); }

    /** @brief Returns pointer to start of underlying buffer
     **/
    uint8_t * Bytes() { return &bytes[0]; }

    /** @brief Returns const pointer to start of underlying buffer
     **/
    const uint8_t * ConstBytes() const { return &bytes[0]; }

    /** @brief Returns buffer pointer
     **/
    operator uint8_t *() { return bytes; }
    operator const uint8_t *() const { return bytes; }

private:
    uint8_t bytes[Cap];
    size_t length = 0;
};

typedef CapacityBoundBuffer<kMax_ECDSA_Signature_Length> P256ECDSASignature;

typedef CapacityBoundBuffer<kMax_ECDH_Secret_Length> P256ECDHDerivedSecret;

class P256PublicKey : public ECPKey<P256ECDSASignature>
{
public:
    P256PublicKey() {}

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
public:
    virtual ~ECPKeypair() {}

    /** @brief Generate a new Certificate Signing Request (CSR).
     * @param csr Newly generated CSR in DER format
     * @param csr_length The caller provides the length of input buffer (csr). The function returns the actual length of generated
     *CSR.
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    virtual CHIP_ERROR NewCertificateSigningRequest(uint8_t * csr, size_t & csr_length) = 0;

    /**
     * @brief A function to sign a msg using ECDSA
     * @param msg Message that needs to be signed
     * @param msg_length Length of message
     * @param out_signature Buffer that will hold the output signature. The signature consists of: 2 EC elements (r and s),
     * in raw <r,s> point form (see SEC1).
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    virtual CHIP_ERROR ECDSA_sign_msg(const uint8_t * msg, size_t msg_length, Sig & out_signature) = 0;

    /**
     * @brief A function to sign a hash using ECDSA
     * @param hash Hash that needs to be signed
     * @param hash_length Length of hash
     * @param out_signature Buffer that will hold the output signature. The signature consists of: 2 EC elements (r and s),
     * in raw <r,s> point form (see SEC1).
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    virtual CHIP_ERROR ECDSA_sign_hash(const uint8_t * hash, size_t hash_length, Sig & out_signature) = 0;

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

typedef CapacityBoundBuffer<kP256_PublicKey_Length + kP256_PrivateKey_Length> P256SerializedKeypair;

class P256KeypairBase : public ECPKeypair<P256PublicKey, P256ECDHDerivedSecret, P256ECDSASignature>
{
public:
    /**
     * @brief Initialize the keypair.
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    virtual CHIP_ERROR Initialize() = 0;

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
    P256Keypair() {}
    ~P256Keypair();

    /**
     * @brief Initialize the keypair.
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR Initialize() override;

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
    CHIP_ERROR NewCertificateSigningRequest(uint8_t * csr, size_t & csr_length) override;

    /**
     * @brief A function to sign a msg using ECDSA
     * @param msg Message that needs to be signed
     * @param msg_length Length of message
     * @param out_signature Buffer that will hold the output signature. The signature consists of: 2 EC elements (r and s),
     * in raw <r,s> point form (see SEC1).
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR ECDSA_sign_msg(const uint8_t * msg, size_t msg_length, P256ECDSASignature & out_signature) override;

    /**
     * @brief A function to sign a hash using ECDSA
     * @param hash Hash that needs to be signed
     * @param hash_length Length of hash
     * @param out_signature Buffer that will hold the output signature. The signature consists of: 2 EC elements (r and s),
     * in raw <r,s> point form (see SEC1).
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR ECDSA_sign_hash(const uint8_t * hash, size_t hash_length, P256ECDSASignature & out_signature) override;

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

private:
    P256PublicKey mPublicKey;
    P256KeypairContext mKeypair;
    bool mInitialized = false;

    void Clear();
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
 * least >= `2 * fe_length_bytes`. On CHIP_NO_ERROR, the out_asn1_sig buffer will be re-assigned
 * to have the correct size based on variable-length output.
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 */
CHIP_ERROR EcdsaAsn1SignatureToRaw(size_t fe_length_bytes, const ByteSpan & asn1_sig, MutableByteSpan & out_raw_sig);

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
 * from the specification.
 *
 * @param plaintext Plaintext to encrypt
 * @param plaintext_length Length of plain_text
 * @param aad Additional authentication data
 * @param aad_length Length of additional authentication data
 * @param key Encryption key
 * @param key_length Length of encryption key (in bytes)
 * @param iv Initial vector
 * @param iv_length Length of initial vector
 * @param ciphertext Buffer to write ciphertext into. Caller must ensure this is large enough to hold the ciphertext
 * @param tag Buffer to write tag into. Caller must ensure this is large enough to hold the tag
 * @param tag_length Expected length of tag
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 * */
CHIP_ERROR AES_CCM_encrypt(const uint8_t * plaintext, size_t plaintext_length, const uint8_t * aad, size_t aad_length,
                           const uint8_t * key, size_t key_length, const uint8_t * iv, size_t iv_length, uint8_t * ciphertext,
                           uint8_t * tag, size_t tag_length);

/**
 * @brief A function that implements AES-CCM decryption
 *
 * This implements the CHIP_Crypto_AEAD_DecryptVerify() cryptographic primitive
 * from the specification.
 *
 * @param ciphertext Ciphertext to decrypt
 * @param ciphertext_length Length of ciphertext
 * @param aad Additional authentical data.
 * @param aad_length Length of additional authentication data
 * @param tag Tag to use to decrypt
 * @param tag_length Length of tag
 * @param key Decryption key
 * @param key_length Length of Decryption key (in bytes)
 * @param iv Initial vector
 * @param iv_length Length of initial vector
 * @param plaintext Buffer to write plaintext into
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/

CHIP_ERROR AES_CCM_decrypt(const uint8_t * ciphertext, size_t ciphertext_length, const uint8_t * aad, size_t aad_length,
                           const uint8_t * tag, size_t tag_length, const uint8_t * key, size_t key_length, const uint8_t * iv,
                           size_t iv_length, uint8_t * plaintext);

/**
 * @brief Verify the Certificate Signing Request (CSR). If successfully verified, it outputs the public key from the CSR.
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

struct alignas(size_t) HashSHA256OpaqueContext
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
    HKDF_sha() {}
    virtual ~HKDF_sha() {}

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
    HMAC_sha() {}
    virtual ~HMAC_sha() {}

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
 * @param fn_source Function pointer to the entropy source
 * @param p_source  Data that should be provided when fn_source is called
 * @param threshold Minimum required from source before entropy is released
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/
CHIP_ERROR add_entropy_source(entropy_source fn_source, void * p_source, size_t threshold);

class PBKDF2_sha256
{
public:
    PBKDF2_sha256() {}
    virtual ~PBKDF2_sha256() {}

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
    virtual ~Spake2p() {}

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
     * @brief Start the Spake2+ process as a verifier (i.e. an accessory being provisioned).
     *
     * @param my_identity       The verifier identity. May be NULL if identities are not established.
     * @param my_identity_len   The verifier identity length.
     * @param peer_identity     The peer identity. May be NULL if identities are not established.
     * @param peer_identity_len The peer identity length.
     * @param w0in              The input w0 (an output from the PBKDF).
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
     * @brief Start the Spake2+ process as a prover (i.e. a commisioner).
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
     * @brief Return the shared secret.
     *
     * @param out     The output secret.
     * @param out_len The output secret length.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR GetKeys(uint8_t * out, size_t * out_len);

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
     * @param out    Output buffer. The size is implicit and is determined by the hash used.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    virtual CHIP_ERROR HashFinalize(uint8_t * out) = 0;

    /**
     * @brief Generate a message authentication code.
     *
     * @param key     The MAC key buffer.
     * @param key_len The size of the MAC key in bytes.
     * @param in      The input buffer.
     * @param in_len  The size of the input data to MAC in bytes.
     * @param out     The output MAC buffer. Size is implicit and is determined by the hash used.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    virtual CHIP_ERROR Mac(const uint8_t * key, size_t key_len, const uint8_t * in, size_t in_len, uint8_t * out) = 0;

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
    CHIP_SPAKE2P_STATE state;
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

    ~Spake2p_P256_SHA256_HKDF_HMAC() override { FreeImpl(); }

    CHIP_ERROR Mac(const uint8_t * key, size_t key_len, const uint8_t * in, size_t in_len, uint8_t * out) override;
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
    CHIP_ERROR ComputeL(uint8_t * Lout, size_t * L_len, const uint8_t * w1in, size_t w1in_len) override;

protected:
    CHIP_ERROR InitImpl() override;
    CHIP_ERROR Hash(const uint8_t * in, size_t in_len) override;
    CHIP_ERROR HashFinalize(uint8_t * out) override;
    CHIP_ERROR KDF(const uint8_t * secret, size_t secret_length, const uint8_t * salt, size_t salt_length, const uint8_t * info,
                   size_t info_length, uint8_t * out, size_t out_length) override;

private:
    /**
     * @brief Free any underlying implementation curve, points, field elements, etc.
     **/
    void FreeImpl();

    CHIP_ERROR InitInternal();
    Hash_SHA256_stream sha256_hash_ctx;

    Spake2pOpaqueContext mSpake2pContext;
};

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

/** @brief Safely clears the first `len` bytes of memory area `buf`.
 * @param buf Pointer to a memory buffer holding secret data that must be cleared.
 * @param len Specifies secret data size in bytes.
 **/
void ClearSecretData(uint8_t * buf, size_t len);

typedef CapacityBoundBuffer<kMax_x509_Certificate_Length> X509DerCertificate;

CHIP_ERROR LoadCertsFromPKCS7(const char * pkcs7, X509DerCertificate * x509list, uint32_t * max_certs);

CHIP_ERROR LoadCertFromPKCS7(const char * pkcs7, X509DerCertificate * x509list, uint32_t n_cert);

CHIP_ERROR GetNumberOfCertsFromPKCS7(const char * pkcs7, uint32_t * n_certs);

CHIP_ERROR ValidateCertificateChain(const uint8_t * rootCertificate, size_t rootCertificateLen, const uint8_t * caCertificate,
                                    size_t caCertificateLen, const uint8_t * leafCertificate, size_t leafCertificateLen);

CHIP_ERROR ExtractPubkeyFromX509Cert(const ByteSpan & certificate, Crypto::P256PublicKey & pubkey);

} // namespace Crypto
} // namespace chip
