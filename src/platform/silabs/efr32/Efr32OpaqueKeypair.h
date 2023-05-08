/*
 *    Copyright (c) 2022 Project CHIP Authors
 *    All rights reserved.
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

#pragma once

#include <crypto/CHIPCryptoPAL.h>
#include <platform/CHIPDeviceError.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

typedef uint16_t EFR32OpaqueKeyId;
constexpr EFR32OpaqueKeyId kEFR32OpaqueKeyIdUnknown       = 0xFFFFU; // Do not modify, will impact existing deployments
constexpr EFR32OpaqueKeyId kEFR32OpaqueKeyIdVolatile      = 0xFFFEU; // Do not modify, will impact existing deployments
constexpr EFR32OpaqueKeyId kEFR32OpaqueKeyIdPersistentMin = 0x0U;    // Do not modify, will impact existing deployments
constexpr EFR32OpaqueKeyId kEFR32OpaqueKeyIdPersistentMax = 0x1FFU;  // Do not decrease, will impact existing deployments

enum class EFR32OpaqueKeyUsages : uint8_t
{
    ECDSA_P256_SHA256 = 0,
    ECDH_P256         = 1,
};

/**
 * @brief Base class for opaque keys
 *
 * Deriving from this class allows using it as a base class for operations
 * which don't expose the private key independant of key size.
 **/
class EFR32OpaqueKeypair
{
public:
    EFR32OpaqueKeypair();
    virtual ~EFR32OpaqueKeypair();

    /**
     * @brief Load a keypair with given key ID
     *
     * If no key exists under the given ID, an error is returned
     * and the object is unusable for operations.
     *
     * @param key_id    key ID under which this key was created
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR Load(EFR32OpaqueKeyId key_id);

    /**
     * @brief Create a new keypair with given ID and usage
     *
     * If a key already exists under the given ID, an error is returned
     * and no new key is created.
     *
     * @param key_id    key ID under which to store this key. Set to 0
     *                  for a non-persistent key which gets destructed
     *                  when the lifetime of this object ends, or set
     *                  to any other value to store the key under that
     *                  ID.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR Create(EFR32OpaqueKeyId key_id, EFR32OpaqueKeyUsages usage);

    /**
     * @brief Get the public key for this keypair
     *
     * @param output        Output buffer to put public key (in 0x04 || X || Y format)
     * @param output_size   Size of \p output
     * @param output_length Amount of bytes put in \p output on success
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR GetPublicKey(uint8_t * output, size_t output_size, size_t * output_length) const;

    /**
     * @brief Get the key ID for this keypair
     *
     * @return Returns kEFR32OpaqueKeyIdUnknown for an uninitialised/invalid
     *         key, kEFR32OpaqueKeyIdVolatile for a volatile key, and a key
     *         ID in the range [kEFR32OpaqueKeyIdPersistentMin, kEFR32OpaqueKeyIdPersistentMax]
     *         for valid persistent keys.
     **/
    EFR32OpaqueKeyId GetKeyId() const;

    /**
     * @brief Use this keypair to sign a message using the ECDSA-SHA256 algorithm
     *
     * @param msg           Message buffer to sign
     * @param msg_len       Size of \p msg in bytes
     * @param output        Output buffer to write signature to. Signature
     *                      is in raw format (i.e. binary concatenation of
     *                      r and s)
     * @param output_size   Size of output buffer
     * @param output_length Amount of bytes written into output buffer
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR Sign(const uint8_t * msg, size_t msg_len, uint8_t * output, size_t output_size, size_t * output_length) const;

    /**
     * @brief Use this keypair to derive a key using the raw ECDH algorithm
     *
     * @param their_key     Buffer containing raw uncompressed public key
     *                      of party to derive with
     * @param their_key_len Size of \p their_key in bytes
     * @param output        Output buffer to write derived bytes to
     * @param output_size   Size of output buffer
     * @param output_length Amount of bytes written into output buffer
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR Derive(const uint8_t * their_key, size_t their_key_len, uint8_t * output, size_t output_size,
                      size_t * output_length) const;

    /**
     * @brief Delete the keypair from storage
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR Delete();

protected:
    void * mContext      = nullptr;
    bool mHasKey         = false;
    bool mIsPersistent   = false;
    uint8_t * mPubkeyRef = nullptr;
    size_t mPubkeySize   = 0;
    size_t mPubkeyLength = 0;
};

/**
 * @brief Derived class of P256Keypair for using opaque keys
 *
 * The signature and compiled form of this class is suboptimal due to how
 * the P256Keypair isn't really an abstract interface, but rather partly
 * interface / partly implementation. Future optimisation should look at
 * converting P256Keypair to a fully abstract interface.
 **/
class EFR32OpaqueP256Keypair : public chip::Crypto::P256Keypair, public EFR32OpaqueKeypair
{
public:
    EFR32OpaqueP256Keypair();
    ~EFR32OpaqueP256Keypair() override;

    /**
     * @brief Initialize the keypair.
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR Initialize(chip::Crypto::ECPKeyTarget key_target) override;

    /**
     * @brief Serialize the keypair (unsupported on opaque keys)
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR Serialize(chip::Crypto::P256SerializedKeypair & output) const override;

    /**
     * @brief Deserialize the keypair (unsupported on opaque keys)
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR Deserialize(chip::Crypto::P256SerializedKeypair & input) override;

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
    CHIP_ERROR ECDSA_sign_msg(const uint8_t * msg, size_t msg_length,
                              chip::Crypto::P256ECDSASignature & out_signature) const override;

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
    CHIP_ERROR ECDH_derive_secret(const chip::Crypto::P256PublicKey & remote_public_key,
                                  chip::Crypto::P256ECDHDerivedSecret & out_secret) const override;

    /** @brief Return public key for the keypair.
     **/
    const chip::Crypto::P256PublicKey & Pubkey() const override;

private:
    chip::Crypto::P256PublicKey mPubKey;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
