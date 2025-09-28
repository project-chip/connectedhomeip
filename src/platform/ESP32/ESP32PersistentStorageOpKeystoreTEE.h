/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <crypto/OperationalKeystore.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/Span.h>

#if defined(CONFIG_USE_ESP32_ECDSA_PERIPHERAL) || defined(CONFIG_USE_ESP32_TEE_SECURE_STORAGE)
#include <platform/ESP32/ESP32CHIPCryptoPAL.h>
#endif // CONFIG_USE_ESP32_ECDSA_PERIPHERAL || CONFIG_USE_ESP32_TEE_SECURE_STORAGE
#include "esp_tee_sec_storage.h"

namespace chip {
namespace Crypto {

class PersistentStorageOpKeystoreTEE : public OperationalKeystore
{
public:
    PersistentStorageOpKeystoreTEE() = default;
    ~PersistentStorageOpKeystoreTEE() override { Finish(); }

    // Non-copyable
    PersistentStorageOpKeystoreTEE(PersistentStorageOpKeystoreTEE const &) = delete;
    void operator=(PersistentStorageOpKeystoreTEE const &)                 = delete;

    CHIP_ERROR Init();
    void Finish();
    bool IsInitialized() const { return mInitialized; }

    bool HasPendingOpKeypair() const override { return mIsPendingKeypairActive; }
    bool HasOpKeypairForFabric(FabricIndex fabricIndex) const override;

    CHIP_ERROR NewOpKeypairForFabric(FabricIndex fabricIndex, MutableByteSpan & outCSR) override;
    CHIP_ERROR ActivateOpKeypairForFabric(FabricIndex fabricIndex, const Crypto::P256PublicKey & nocPublicKey) override;
    CHIP_ERROR CommitOpKeypairForFabric(FabricIndex fabricIndex) override;
    CHIP_ERROR RemoveOpKeypairForFabric(FabricIndex fabricIndex) override;
    void RevertPendingKeypair() override;
    CHIP_ERROR SignWithOpKeypair(FabricIndex fabricIndex, const ByteSpan & message,
                                 Crypto::P256ECDSASignature & outSignature) const override;

    // Not implemented for now
    Crypto::P256Keypair * AllocateEphemeralKeypairForCASE() override { return nullptr; }
    void ReleaseEphemeralKeypair(Crypto::P256Keypair * keypair) override {}

private:
    void ResetPendingKey();

    std::string GetKeyId(FabricIndex fabricIndex) const;

    bool mInitialized                                  = false;
    mutable Crypto::ESP32P256Keypair * mPendingKeypair = nullptr;
    mutable bool mIsPendingKeypairActive               = false;
    mutable FabricIndex mPendingFabricIndex            = kUndefinedFabricIndex;
};

} // namespace Crypto
} // namespace chip
