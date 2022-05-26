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

namespace chip {
namespace Crypto {

/**
 * Bridging implementation of P256Keypair to allow delegation of signing and
 * key generation to the JVM layer (through the KeypairDelegate Java interface).
 *
 * This implementation explicitly does not support serialization or
 * deserialization and will always return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if
 * either of them are invoked (as this bridge is not expected to ever have
 * access to the raw key bits).
 */
class CHIPP256KeypairBridge : public P256Keypair
{
public:
    ~CHIPP256KeypairBridge() override;

    /**
     * Sets a reference to the Java implementation of the KeypairDelegate
     * interface that will be called whenever this P256Keypair is used.
     */
    CHIP_ERROR SetDelegate(jobject delegate);

    bool HasKeypair() const { return mDelegate != nullptr; }

    CHIP_ERROR Initialize() override;

    CHIP_ERROR Serialize(P256SerializedKeypair & output) const override;

    CHIP_ERROR Deserialize(P256SerializedKeypair & input) override;

    CHIP_ERROR NewCertificateSigningRequest(uint8_t * csr, size_t & csr_length) const override;

    CHIP_ERROR ECDSA_sign_msg(const uint8_t * msg, size_t msg_length, P256ECDSASignature & out_signature) const override;

    CHIP_ERROR ECDSA_sign_hash(const uint8_t * hash, size_t hash_length, P256ECDSASignature & out_signature) const override;

    CHIP_ERROR ECDH_derive_secret(const P256PublicKey & remote_public_key,
                                  P256ECDHDerivedSecret & out_secret) const override;

    const P256PublicKey & Pubkey() const override { return mPublicKey; };

private:
    jobject mDelegate;
    jclass mKeypairDelegateClass;
    jmethodID mGetPublicKeyMethod;
    jmethodID mCreateCertificateSigningRequestMethod;
    jmethodID mEcdsaSignMessageMethod;

    P256PublicKey mPublicKey;

    CHIP_ERROR SetPubkey();
};

} // namespace Crypto
} // namespace chip
