/*
 *    Copyright (c) 2023 Project CHIP Authors
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
#include <lib/support/Span.h>

namespace chip {
namespace Crypto {

/**
 * @brief Interface for deriving session keys and managing their lifetime.
 *
 * The session keystore interface provides an abstraction that allows the application to store
 * session keys in a secure environment. It uses the concept of key handles that isolate the
 * application from the actual key material.
 */
class SessionKeystore
{
public:
    virtual ~SessionKeystore() {}

    /**
     * @brief Import raw key material and return a key handle.
     *
     * @note This method should only be used when using the raw key material in the Matter stack
     * cannot be avoided. Ideally, crypto interfaces should allow platforms to perform all the
     * cryptographic operations in a secure environment.
     *
     * If the method returns no error, the application is responsible for destroying the handle
     * using DestroyKey() method when the key is no longer needed.
     */
    virtual CHIP_ERROR CreateKey(const Aes128KeyByteArray & keyMaterial, Aes128KeyHandle & key) = 0;

    /**
     * @brief Derive key from a shared secret.
     *
     * Use HKDF as defined in the Matter specification to derive an AES key from the shared secret.
     *
     * If the method returns no error, the application is responsible for destroying the handle
     * using DestroyKey() method when the key is no longer needed.
     */
    virtual CHIP_ERROR DeriveKey(const P256ECDHDerivedSecret & secret, const ByteSpan & salt, const ByteSpan & info,
                                 Aes128KeyHandle & key) = 0;

    /**
     * @brief Derive session keys from a shared secret.
     *
     * Use HKDF as defined in the Matter specification to derive AES keys for both directions, and
     * the attestation challenge from the shared secret.
     *
     * If the method returns no error, the application is responsible for destroying the handles
     * using DestroyKey() method when the keys are no longer needed. On failure, the method must
     * release all handles that it allocated so far.
     */
    virtual CHIP_ERROR DeriveSessionKeys(const ByteSpan & secret, const ByteSpan & salt, const ByteSpan & info,
                                         Aes128KeyHandle & i2rKey, Aes128KeyHandle & r2iKey,
                                         AttestationChallenge & attestationChallenge) = 0;

    /**
     * @brief Destroy key.
     *
     * The method can take an uninitialized handle in which case it is a no-op.
     * As a result of calling this method, the handle is put in the uninitialized state.
     */
    virtual void DestroyKey(Aes128KeyHandle & key) = 0;
};

/**
 * @brief RAII class to hold a temporary key handle that is destroyed on scope exit.
 */
class AutoReleaseSessionKey
{
public:
    explicit AutoReleaseSessionKey(SessionKeystore & keystore) : mKeystore(keystore) {}
    ~AutoReleaseSessionKey() { mKeystore.DestroyKey(mKeyHandle); }

    Aes128KeyHandle & KeyHandle() { return mKeyHandle; }

private:
    SessionKeystore & mKeystore;
    Aes128KeyHandle mKeyHandle;
};

} // namespace Crypto
} // namespace chip
