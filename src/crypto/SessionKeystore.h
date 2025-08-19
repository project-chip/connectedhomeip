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
 *
 * @note Refactor has begun to refactor this API into two disctinct APIs : SymmetrycKeyStore & SessionKeyDerivation
 *       Work has not been completed so the  SessionKeystore has APIs that shouldn't go together for the time being
 *       The SessionKeystore APIs are split into two sections, one for each futur API.
 */
class SessionKeystore
{
public:
    virtual ~SessionKeystore() {}

    /****************************
     * SymmetricKeyStore APIs
     *****************************/

    /**
     * @brief Import raw key material and return a key handle for a key that be used to do AES 128 encryption.
     *
     * @note This method should only be used when using the raw key material in the Matter stack
     * cannot be avoided. Ideally, crypto interfaces should allow platforms to perform all the
     * cryptographic operations in a secure environment.
     *
     * If the method returns no error, the application is responsible for destroying the handle
     * using the DestroyKey() method when the key is no longer needed.
     */
    virtual CHIP_ERROR CreateKey(const Symmetric128BitsKeyByteArray & keyMaterial, Aes128KeyHandle & key) = 0;

    /**
     * @brief Import raw key material and return a key handle for a key that can be used to do 128-bit HMAC.
     *
     * @note This method should only be used when using the raw key material in the Matter stack
     * cannot be avoided. Ideally, crypto interfaces should allow platforms to perform all the
     * cryptographic operations in a secure environment.
     *
     * If the method returns no error, the application is responsible for destroying the handle
     * using the DestroyKey() method when the key is no longer needed.
     */
    virtual CHIP_ERROR CreateKey(const Symmetric128BitsKeyByteArray & keyMaterial, Hmac128KeyHandle & key) = 0;

    /**
     * @brief Import raw key material and return a key handle for an HKDF key.
     *
     * @note This method should only be used when using the raw key material in the Matter stack
     * cannot be avoided. Ideally, crypto interfaces should allow platforms to perform all the
     * cryptographic operations in a secure environment.
     *
     * If the method returns no error, the application is responsible for destroying the handle
     * using the DestroyKey() method when the key is no longer needed.
     */
    virtual CHIP_ERROR CreateKey(const ByteSpan & keyMaterial, HkdfKeyHandle & key) = 0;

    /**
     * @brief Destroy key.
     *
     * The method can take an uninitialized handle in which case it is a no-op.
     * As a result of calling this method, the handle is put in the uninitialized state.
     */
    virtual void DestroyKey(Symmetric128BitsKeyHandle & key) = 0;

    /**
     * @brief Destroy key.
     *
     * The method can take an uninitialized handle in which case it is a no-op.
     * As a result of calling this method, the handle is put in the uninitialized state.
     */
    virtual void DestroyKey(HkdfKeyHandle & key) = 0;

    /****************************
     * SessionKeyDerivation APIs
     *****************************/

    /**
     * @brief Derive key from a session establishment's `SharedSecret`.
     *
     * Use `Crypto_KDF` (HKDF) primitive as defined in the Matter specification to derive
     * a symmetric (AES) key from the session establishment's `SharedSecret`.
     *
     * If the method returns no error, the caller is responsible for destroying the symmetric key
     * using the DestroyKey() method when the key is no longer needed.
     */
    virtual CHIP_ERROR DeriveKey(const P256ECDHDerivedSecret & secret, const ByteSpan & salt, const ByteSpan & info,
                                 Aes128KeyHandle & key) = 0;

    /**
     * @brief Derive session keys from a session establishment's `SharedSecret`.
     *
     * Use `Crypto_KDF` (HKDF) primitive as defined in the Matter specification to derive symmetric
     * (AES) session keys for both directions, and the attestation challenge from the session
     * establishment's `SharedSecret`.
     *
     * If the method returns no error, the caller is responsible for destroying the symmetric keys
     * using the DestroyKey() method when the keys are no longer needed. On failure, the method is
     * responsible for releasing all keys that it allocated so far.
     */
    virtual CHIP_ERROR DeriveSessionKeys(const ByteSpan & secret, const ByteSpan & salt, const ByteSpan & info,
                                         Aes128KeyHandle & i2rKey, Aes128KeyHandle & r2iKey,
                                         AttestationChallenge & attestationChallenge) = 0;

    /**
     * @brief Derive session keys from a session establishment's `SharedSecret`.
     *
     * Use Crypto_KDF (HKDF) primitive as defined in the Matter specification to derive symmetric
     * (AES) session keys for both directions, and the attestation challenge from the session
     * establishment's `SharedSecret`, represented as the key handle.
     *
     * If the method returns no error, the caller is responsible for destroying the symmetric keys
     * using the DestroyKey() method when the keys are no longer needed. On failure, the method is
     * responsible for releasing all keys that it allocated so far.
     */
    virtual CHIP_ERROR DeriveSessionKeys(const HkdfKeyHandle & secretKey, const ByteSpan & salt, const ByteSpan & info,
                                         Aes128KeyHandle & i2rKey, Aes128KeyHandle & r2iKey,
                                         AttestationChallenge & attestationChallenge) = 0;

    /**
     * @brief Persistently store an ICD key.
     *
     * If input is already a persistent key handle, the function is a no-op and the original handle is returned.
     * If input is a volatile key handle, key is persisted and the handle may be updated.
     *
     * If the method returns no error, the application is responsible for destroying the handle
     * using the DestroyKey() method when the key is no longer needed.
     */
    virtual CHIP_ERROR PersistICDKey(Symmetric128BitsKeyHandle & key) { return CHIP_NO_ERROR; }
};

/**
 * @brief RAII class to hold a temporary key handle that is destroyed on scope exit.
 */
template <class KeyHandleType>
class AutoReleaseSymmetricKey
{
public:
    explicit AutoReleaseSymmetricKey(SessionKeystore & keystore) : mKeystore(keystore) {}
    ~AutoReleaseSymmetricKey() { mKeystore.DestroyKey(mKeyHandle); }

    KeyHandleType & KeyHandle() { return mKeyHandle; }

private:
    SessionKeystore & mKeystore;
    KeyHandleType mKeyHandle;
};

} // namespace Crypto
} // namespace chip
