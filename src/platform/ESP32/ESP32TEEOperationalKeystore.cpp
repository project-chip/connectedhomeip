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

#include <platform/ESP32/ESP32TEEOperationalKeystore.h>

#include <platform/ESP32/ESP32TEEOpKey.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <cstdio>

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {

// Per-fabric secure-storage key id, one per slot: "opk-<idx>-<slot>".
void MakeKeyId(FabricIndex fabricIndex, char slot, char (&out)[16])
{
    snprintf(out, sizeof(out), "opk-%u-%c", static_cast<unsigned>(fabricIndex), slot);
}

// Per-fabric KVS key holding the committed slot char. Kept short for NVS limits.
void MakeSlotStorageKey(FabricIndex fabricIndex, char (&out)[16])
{
    snprintf(out, sizeof(out), "tso/%x", static_cast<unsigned>(fabricIndex));
}

} // namespace

CHIP_ERROR ESP32TEEOperationalKeystore::Init(PersistentStorageDelegate * storage)
{
    VerifyOrReturnError(mStorage == nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(storage != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    mStorage = storage;
    ResetPending();
    return CHIP_NO_ERROR;
}

void ESP32TEEOperationalKeystore::Finish()
{
    VerifyOrReturn(mStorage != nullptr);
    RevertPendingKeypair();
    mStorage = nullptr;
}

void ESP32TEEOperationalKeystore::ResetPending()
{
    mPendingFabricIndex     = kUndefinedFabricIndex;
    mPendingSlot            = 0;
    mHasPending             = false;
    mIsPendingKeypairActive = false;
}

CHIP_ERROR ESP32TEEOperationalKeystore::ReadActiveSlot(FabricIndex fabricIndex, char & outSlot) const
{
    outSlot = 0; // 0 == no committed slot
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    char storageKey[16];
    MakeSlotStorageKey(fabricIndex, storageKey);

    char slot      = 0;
    uint16_t size  = sizeof(slot);
    CHIP_ERROR err = mStorage->SyncGetKeyValue(storageKey, &slot, size);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        return CHIP_NO_ERROR; // no committed key for this fabric yet
    }
    ReturnErrorOnFailure(err); // propagate real storage errors instead of masking them as "no slot"
    VerifyOrReturnError(size == sizeof(slot) && (slot == 'A' || slot == 'B'), CHIP_ERROR_INTEGRITY_CHECK_FAILED);
    outSlot = slot;
    return CHIP_NO_ERROR;
}

bool ESP32TEEOperationalKeystore::HasPendingOpKeypair() const
{
    return mHasPending;
}

bool ESP32TEEOperationalKeystore::HasOpKeypairForFabric(FabricIndex fabricIndex) const
{
    VerifyOrReturnValue(mStorage != nullptr, false);
    VerifyOrReturnValue(IsValidFabricIndex(fabricIndex), false);

    if (mIsPendingKeypairActive && (fabricIndex == mPendingFabricIndex) && mHasPending)
    {
        return true;
    }

    // Invariant: the active-slot pointer is written only after a key is committed to that
    // slot, and cleared by RemoveOpKeypairForFabric, so its presence implies a usable key.
    char slot = 0;
    return (ReadActiveSlot(fabricIndex, slot) == CHIP_NO_ERROR) && (slot != 0);
}

CHIP_ERROR ESP32TEEOperationalKeystore::NewOpKeypairForFabric(FabricIndex fabricIndex,
                                                              MutableByteSpan & outCertificateSigningRequest)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);
    // Only one pending keypair at a time, and only for a single fabric.
    VerifyOrReturnError(!mHasPending || (fabricIndex == mPendingFabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(outCertificateSigningRequest.size() >= Crypto::kMIN_CSR_Buffer_Size, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Discard any prior pending key for this fabric before generating a fresh one.
    RevertPendingKeypair();

    // Generate into the slot that is NOT currently committed, so the committed key
    // stays intact and usable until (and unless) this one is committed.
    char activeSlot = 0;
    ReturnErrorOnFailure(ReadActiveSlot(fabricIndex, activeSlot));
    const char pendingSlot = (activeSlot == 'A') ? 'B' : 'A';

    char keyId[16];
    MakeKeyId(fabricIndex, pendingSlot, keyId);

    // A fail-safe aborted by a reboot (power loss before Commit/Revert) can leave a stale key in
    // this inactive slot. esp_tee_sec_storage_gen_key rejects an existing id, so clear it first.
    LogErrorOnFailure(ESP32TEEOpKeyRemove(keyId));
    ReturnErrorOnFailure(ESP32TEEOpKeyGenerate(keyId));

    // NewCSR resizes outCertificateSigningRequest in place to the actual CSR length.
    CHIP_ERROR err = ESP32TEEOpKeyNewCSR(keyId, outCertificateSigningRequest);
    if (err != CHIP_NO_ERROR)
    {
        LogErrorOnFailure(ESP32TEEOpKeyRemove(keyId));
        return err;
    }

    mPendingFabricIndex     = fabricIndex;
    mPendingSlot            = pendingSlot;
    mHasPending             = true;
    mIsPendingKeypairActive = false;
    ChipLogProgress(Crypto, "TEE opkey: generated NOC keypair for fabric 0x%x in TEE secure storage (slot %c), CSR signed in TEE",
                    static_cast<unsigned>(fabricIndex), pendingSlot);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32TEEOperationalKeystore::ActivateOpKeypairForFabric(FabricIndex fabricIndex,
                                                                   const Crypto::P256PublicKey & nocPublicKey)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mHasPending, CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex) && (fabricIndex == mPendingFabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    char keyId[16];
    MakeKeyId(fabricIndex, mPendingSlot, keyId);

    Crypto::P256PublicKey pendingPubkey;
    ReturnErrorOnFailure(ESP32TEEOpKeyGetPublicKey(keyId, pendingPubkey));
    VerifyOrReturnError(pendingPubkey.Matches(nocPublicKey), CHIP_ERROR_INVALID_PUBLIC_KEY);

    mIsPendingKeypairActive = true;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32TEEOperationalKeystore::CommitOpKeypairForFabric(FabricIndex fabricIndex)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mHasPending, CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex) && (fabricIndex == mPendingFabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(mIsPendingKeypairActive, CHIP_ERROR_INCORRECT_STATE);

    char oldSlot = 0;
    ReturnErrorOnFailure(ReadActiveSlot(fabricIndex, oldSlot));

    // The pointer write is the atomic commit point. On failure, leave everything pending.
    char storageKey[16];
    MakeSlotStorageKey(fabricIndex, storageKey);
    ReturnErrorOnFailure(mStorage->SyncSetKeyValue(storageKey, &mPendingSlot, sizeof(mPendingSlot)));

    // Rotation: drop the superseded slot's key (best-effort; a leftover is cleared by the next
    // NewOpKeypairForFabric, which targets and clears the now-inactive slot before generating).
    if (oldSlot != 0 && oldSlot != mPendingSlot)
    {
        char oldKeyId[16];
        MakeKeyId(fabricIndex, oldSlot, oldKeyId);
        LogErrorOnFailure(ESP32TEEOpKeyRemove(oldKeyId));
    }

    ChipLogProgress(Crypto, "TEE opkey: committed NOC keypair for fabric 0x%x to TEE secure storage (slot %c)",
                    static_cast<unsigned>(fabricIndex), mPendingSlot);
    ResetPending();
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32TEEOperationalKeystore::RemoveOpKeypairForFabric(FabricIndex fabricIndex)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    if (mHasPending && (fabricIndex == mPendingFabricIndex))
    {
        RevertPendingKeypair();
    }

    // Remove both slots' keys; either may hold a committed or orphaned key.
    for (const char slot : { 'A', 'B' })
    {
        char keyId[16];
        MakeKeyId(fabricIndex, slot, keyId);
        LogErrorOnFailure(ESP32TEEOpKeyRemove(keyId));
    }

    char storageKey[16];
    MakeSlotStorageKey(fabricIndex, storageKey);
    CHIP_ERROR err = mStorage->SyncDeleteKeyValue(storageKey);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        err = CHIP_ERROR_INVALID_FABRIC_INDEX;
    }
    ChipLogProgress(Crypto, "TEE opkey: cleared TEE secure-storage NOC key slots for fabric 0x%x",
                    static_cast<unsigned>(fabricIndex));
    return err;
}

void ESP32TEEOperationalKeystore::RevertPendingKeypair()
{
    VerifyOrReturn(mStorage != nullptr);

    if (mHasPending && mPendingSlot != 0)
    {
        char keyId[16];
        MakeKeyId(mPendingFabricIndex, mPendingSlot, keyId);
        LogErrorOnFailure(ESP32TEEOpKeyRemove(keyId));
    }
    ResetPending();
}

CHIP_ERROR ESP32TEEOperationalKeystore::SignWithOpKeypair(FabricIndex fabricIndex, const ByteSpan & message,
                                                          Crypto::P256ECDSASignature & outSignature) const
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    char slot = 0;
    if (mIsPendingKeypairActive && (fabricIndex == mPendingFabricIndex))
    {
        slot = mPendingSlot;
    }
    else
    {
        ReturnErrorOnFailure(ReadActiveSlot(fabricIndex, slot));
        VerifyOrReturnError(slot != 0, CHIP_ERROR_INVALID_FABRIC_INDEX);
    }

    char keyId[16];
    MakeKeyId(fabricIndex, slot, keyId);
    ChipLogDetail(Crypto, "TEE opkey: signing (CASE) for fabric 0x%x with TEE key slot %c", static_cast<unsigned>(fabricIndex),
                  slot);
    return ESP32TEEOpKeySign(keyId, message, outSignature);
}

Crypto::P256Keypair * ESP32TEEOperationalKeystore::AllocateEphemeralKeypairForCASE()
{
    return Platform::New<Crypto::P256Keypair>();
}

void ESP32TEEOperationalKeystore::ReleaseEphemeralKeypair(Crypto::P256Keypair * keypair)
{
    Platform::Delete<Crypto::P256Keypair>(keypair);
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
