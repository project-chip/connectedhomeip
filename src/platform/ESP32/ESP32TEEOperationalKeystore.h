/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
 *      OperationalKeystore backed by ESP-TEE secure storage. Each fabric's operational
 *      (NOC) private key is generated and used entirely inside the secure world; it is
 *      never present in application RAM or flash in usable form.
 *
 *      Because a TEE-generated key is persisted the moment it is created (it cannot be
 *      held RAM-only like the software keystore's pending key), fail-safe semantics are
 *      implemented with two secure-storage slots per fabric ("A"/"B") plus a small
 *      persisted pointer selecting the committed slot. NewOpKeypairForFabric generates
 *      into the currently-inactive slot; CommitOpKeypairForFabric atomically flips the
 *      pointer (and removes the superseded slot); RevertPendingKeypair clears the
 *      inactive (provisional) slot, leaving the committed key untouched.
 */
#pragma once

#include <crypto/CHIPCryptoPAL.h>
#include <crypto/OperationalKeystore.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/Span.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

class ESP32TEEOperationalKeystore : public Crypto::OperationalKeystore
{
public:
    ESP32TEEOperationalKeystore()           = default;
    ~ESP32TEEOperationalKeystore() override = default;

    // Non-copyable
    ESP32TEEOperationalKeystore(const ESP32TEEOperationalKeystore &) = delete;
    void operator=(const ESP32TEEOperationalKeystore &)              = delete;

    /**
     * @brief Bind the keystore to a persistent storage delegate used only for the
     *        per-fabric active-slot pointer (never for key material).
     */
    CHIP_ERROR Init(PersistentStorageDelegate * storage);

    void Finish();

    bool HasPendingOpKeypair() const override;
    bool HasOpKeypairForFabric(FabricIndex fabricIndex) const override;
    CHIP_ERROR NewOpKeypairForFabric(FabricIndex fabricIndex, MutableByteSpan & outCertificateSigningRequest) override;
    CHIP_ERROR ActivateOpKeypairForFabric(FabricIndex fabricIndex, const Crypto::P256PublicKey & nocPublicKey) override;
    CHIP_ERROR CommitOpKeypairForFabric(FabricIndex fabricIndex) override;
    CHIP_ERROR RemoveOpKeypairForFabric(FabricIndex fabricIndex) override;
    void RevertPendingKeypair() override;
    CHIP_ERROR SignWithOpKeypair(FabricIndex fabricIndex, const ByteSpan & message,
                                 Crypto::P256ECDSASignature & outSignature) const override;

    // TEE private keys are non-exportable by design.
    CHIP_ERROR ExportOpKeypairForFabric(FabricIndex, Crypto::P256SerializedKeypair &) override
    {
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }

    // CASE ephemeral keys are short-lived session keys and stay in software.
    Crypto::P256Keypair * AllocateEphemeralKeypairForCASE() override;
    void ReleaseEphemeralKeypair(Crypto::P256Keypair * keypair) override;

private:
    // Returns 'A' or 'B' for the committed slot, or 0 if none is stored.
    // Reads the committed slot for @p fabricIndex into @p outSlot ('A'/'B', or 0 if none).
    // Returns an error only on an actual storage failure, not for a missing pointer.
    CHIP_ERROR ReadActiveSlot(FabricIndex fabricIndex, char & outSlot) const;
    void ResetPending();

    PersistentStorageDelegate * mStorage = nullptr;

    FabricIndex mPendingFabricIndex = kUndefinedFabricIndex;
    char mPendingSlot               = 0;
    bool mHasPending                = false;
    bool mIsPendingKeypairActive    = false;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
