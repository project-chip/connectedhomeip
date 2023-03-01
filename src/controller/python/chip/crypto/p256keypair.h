/*
 *
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

#include <inttypes.h>

#include <controller/python/chip/native/PyChipError.h>
#include <crypto/CHIPCryptoPAL.h>

typedef bool (*pychip_P256Keypair_ECDSA_sign_msg)(void * pyObject, const uint8_t * msg, size_t msg_length, uint8_t * out_signature,
                                                  size_t * signature_length);

typedef bool (*pychip_P256Keypair_ECDH_derive_secret)(void * pyObject, const uint8_t * remote_public_key, uint8_t * out_secret,
                                                      size_t * out_secret_length);

namespace chip {
namespace python {

class pychip_P256Keypair : public Crypto::P256Keypair
{
public:
    pychip_P256Keypair(void * aPyContext, pychip_P256Keypair_ECDSA_sign_msg aSignMsgFunct,
                       pychip_P256Keypair_ECDH_derive_secret aDeriveSecretFunct);
    ~pychip_P256Keypair() override;

    CHIP_ERROR Initialize(Crypto::ECPKeyTarget key_target) override;

    /**
     * @brief Serialize the keypair.
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR Serialize(Crypto::P256SerializedKeypair & output) const override;

    /**
     * @brief Deserialize the keypair.
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR Deserialize(Crypto::P256SerializedKeypair & input) override;

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
    CHIP_ERROR ECDSA_sign_msg(const uint8_t * msg, size_t msg_length, Crypto::P256ECDSASignature & out_signature) const override;

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
    CHIP_ERROR ECDH_derive_secret(const Crypto::P256PublicKey & remote_public_key,
                                  Crypto::P256ECDHDerivedSecret & out_secret) const override;

    /**
     * @brief A function to update the public key recorded in the keypair for C++ interface.
     *
     * @param publicKey A buffer of publicKey, should have exactly `kP256_PublicKey_Length` bytes.
     *
     **/
    void UpdatePubkey(const FixedByteSpan<Crypto::kP256_PublicKey_Length> & aPublicKey);

    /** @brief Return public key for the keypair.
     **/
    const Crypto::P256PublicKey & Pubkey() const override
    {
        // The mPublicKey is a member of Crypto::P256Keypair and is set in Initialize
        return mPublicKey;
    }

private:
    void * mPyContext;

    pychip_P256Keypair_ECDSA_sign_msg mSignMsgFunct;
    pychip_P256Keypair_ECDH_derive_secret mDeriveSecretFunct;
};

} // namespace python
} // namespace chip

extern "C" {

chip::python::pychip_P256Keypair * pychip_NewP256Keypair(void * pyObject, pychip_P256Keypair_ECDSA_sign_msg aSignMsgFunct,
                                                         pychip_P256Keypair_ECDH_derive_secret aDeriveSecretFunct);

PyChipError pychip_P256Keypair_UpdatePubkey(chip::python::pychip_P256Keypair * this_, uint8_t * aPubKey, size_t aPubKeyLen);
void pychip_DeleteP256Keypair(chip::python::pychip_P256Keypair * this_);
}
