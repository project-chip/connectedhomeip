/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/network-identity-management-server/NetworkAdministratorSecret.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>

namespace chip::Crypto {

/**
 * @brief Cryptographic keystore interface for the Network Identity Management cluster.
 *
 * Handles returned by Import* and Derive* methods may represent live keystore allocations.
 * Callers are responsible for ensuring handles are either:
 *   (a) persisted (via NetworkIdentityStorage) and later destroyed via the corresponding Destroy*, or
 *   (b) destroyed immediately on any failure that prevents persistence.
 * Failing to do so may leak keystore resources. On hardware-backed keystores this
 * may be unrecoverable across reboots.
 */
class NetworkIdentityKeystore
{
public:
    virtual ~NetworkIdentityKeystore() = default;

    /**
     * Stores the raw Network Administrator Shared Secret and returns a persistent handle to it.
     * The keystore must support storing at least 2 NASS handles at any one time to allow
     * a new NASS to be imported and keys derived from it before the old NASS is deleted.
     */
    virtual CHIP_ERROR ImportNetworkAdministratorSecret(const NetworkAdministratorRawSecret & secret,
                                                        HkdfKeyHandle & outHandle) = 0;

    /**
     * Exports the raw Network Administrator Shared Secret given its handle.
     */
    virtual CHIP_ERROR ExportNetworkAdministratorSecret(const HkdfKeyHandle & handle,
                                                        NetworkAdministratorRawSecret & outRawSecret) = 0;

    /**
     * Destroys the Network Administrator Shared Secret with the specified handle.
     */
    virtual void DestroyNetworkAdministratorSecret(HkdfKeyHandle & handle) = 0;

    /**
     * Derives the ECDSA Network Identity from the Network Administrator Shared Secret.
     * Returns a persistent handle for the associated keypair and the serialized identity.
     * The keystore must support storing at least 4 ECDSA keypairs at any one time.
     * See Matter Specification, "Network Identity Derivation for ECDSA".
     */
    virtual CHIP_ERROR DeriveECDSANetworkIdentity(const HkdfKeyHandle & nassHandle, P256KeypairHandle & outKeypairHandle,
                                                  MutableByteSpan & outIdentity) = 0;

    /**
     * Destroys the Network Identity keypair with the specified handle.
     */
    virtual void DestroyNetworkIdentityKeypair(P256KeypairHandle & handle) = 0;
};

} // namespace chip::Crypto
