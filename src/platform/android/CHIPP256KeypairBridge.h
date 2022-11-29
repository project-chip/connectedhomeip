/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <jni.h>

#include "crypto/CHIPCryptoPAL.h"
#include "lib/core/CHIPError.h"
#include "lib/support/logging/CHIPLogging.h"

/**
 * Bridging implementation of P256Keypair to allow delegation of signing and
 * key generation to the JVM layer (through the KeypairDelegate Java interface).
 *
 * This implementation explicitly does not support serialization or
 * deserialization and will always return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if
 * either of them are invoked (as this bridge is not expected to ever have
 * access to the raw key bits).
 */
class CHIPP256KeypairBridge : public chip::Crypto::P256Keypair
{
public:
    ~CHIPP256KeypairBridge() override;

    /**
     * Sets a reference to the Java implementation of the KeypairDelegate
     * interface that will be called whenever this P256Keypair is used.
     */
    CHIP_ERROR SetDelegate(jobject delegate);

    bool HasKeypair() const { return mDelegate != nullptr; }

    CHIP_ERROR Initialize(chip::Crypto::ECPKeyTarget key_target) override;

    CHIP_ERROR Serialize(chip::Crypto::P256SerializedKeypair & output) const override;

    CHIP_ERROR Deserialize(chip::Crypto::P256SerializedKeypair & input) override;

    CHIP_ERROR NewCertificateSigningRequest(uint8_t * csr, size_t & csr_length) const override;

    CHIP_ERROR ECDSA_sign_msg(const uint8_t * msg, size_t msg_length,
                              chip::Crypto::P256ECDSASignature & out_signature) const override;

    CHIP_ERROR ECDH_derive_secret(const chip::Crypto::P256PublicKey & remote_public_key,
                                  chip::Crypto::P256ECDHDerivedSecret & out_secret) const override;

    const chip::Crypto::P256PublicKey & Pubkey() const override { return mPublicKey; };

private:
    jobject mDelegate;
    jclass mKeypairDelegateClass;
    jmethodID mGetPublicKeyMethod;
    jmethodID mCreateCertificateSigningRequestMethod;
    jmethodID mEcdsaSignMessageMethod;

    chip::Crypto::P256PublicKey mPublicKey;

    CHIP_ERROR SetPubkey();
};
