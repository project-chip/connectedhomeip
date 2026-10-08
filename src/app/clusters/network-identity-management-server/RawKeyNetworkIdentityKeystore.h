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

#include <app/clusters/network-identity-management-server/NetworkIdentityKeystore.h>

namespace chip::Crypto {

/**
 * @brief Raw key implementation of NetworkIdentityKeystore.
 *
 * This implementation stores key material directly in the handles:
 * - HkdfKeyHandle contains the raw 32-byte NASS secret
 * - P256KeypairHandle contains the serialized keypair (97 bytes)
 *
 * Identity derivation via `DeriveECDSANetworkIdentity` requires Deterministic
 * ECDSA (`P256KeyPair::ECDSA_sign_msg_det()`) to be supported by the crypto backend.
 *
 * WARNING: This implementation stores raw key material in handles that may be
 * persisted to storage. For production use, consider PSA-backed implementations
 * that use secure key storage.
 */
class RawKeyNetworkIdentityKeystore : public NetworkIdentityKeystore
{
public:
    CHIP_ERROR ImportNetworkAdministratorSecret(const NetworkAdministratorRawSecret & secret, HkdfKeyHandle & outHandle) override;
    CHIP_ERROR ExportNetworkAdministratorSecret(const HkdfKeyHandle & handle,
                                                NetworkAdministratorRawSecret & outRawSecret) override;
    void DestroyNetworkAdministratorSecret(HkdfKeyHandle & handle) override;

    CHIP_ERROR DeriveECDSANetworkIdentity(const HkdfKeyHandle & nassHandle, P256KeypairHandle & outKeypairHandle,
                                          MutableByteSpan & outIdentity) override;
    void DestroyNetworkIdentityKeypair(P256KeypairHandle & handle) override;
};

} // namespace chip::Crypto
