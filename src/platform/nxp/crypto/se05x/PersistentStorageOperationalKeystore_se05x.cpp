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

#include "PersistentStorageOperationalKeystore_se05x.h"
#include "CHIPCryptoPALHsm_se05x_utils.h"
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/SafeInt.h>

namespace chip {

using namespace chip::Crypto;

#define CHIP_SE05x_NODE_OP_KEY_INDEX (SE051H_NODE_OP_KEY_ID - 1)

#define CHIP_SE05x_NODE_OP_SLOT_B_OFFSET 0x2000 // Slot B: BASE + 0x2000 + fabricIndex
#define KEYPAIR_KEYID_OFFSET 8
#define NUM_NODE_OP_KEY_INDEXES (5)

#define CHIP_SE05x_NODE_OP_REF_KEY_TEMPLATE                                                                                        \
    {                                                                                                                              \
        0xA5, 0xA6, 0xB5, 0xB6, 0xA5, 0xA6, 0xB5, 0xB6, 0x7E, 0x00, 0x00, 0x00                                                     \
    }
#define CHIP_SE05x_NODE_OP_KEY_ID_INDEX 11

constexpr TLV::Tag kOpKeyVersionTag = TLV::ContextTag(0);
constexpr TLV::Tag kOpKeyDataTag    = TLV::ContextTag(1);
constexpr uint16_t kOpKeyVersion    = 1;

constexpr size_t OpKeyTLVMaxSize()
{
    // Version and serialized key
    return TLV::EstimateStructOverhead(sizeof(uint16_t), Crypto::P256SerializedKeypair::Capacity());
}

static uint32_t GetSlotAKeyId(FabricIndex fabricIndex)
{
    return CHIP_SE05x_NODE_OP_KEY_INDEX + fabricIndex;
}

static uint32_t GetSlotBKeyId(FabricIndex fabricIndex)
{
    return CHIP_SE05x_NODE_OP_KEY_INDEX + CHIP_SE05x_NODE_OP_SLOT_B_OFFSET + fabricIndex;
}

CHIP_ERROR
PersistentStorageOpKeystorese05x::ExtractKeyIdFromSerializedKeypair(const Crypto::P256SerializedKeypair & serializedKeypair,
                                                                    uint32_t & outKeyId) const
{

    const uint8_t * privKeyRef = serializedKeypair.ConstBytes() + kP256_PublicKey_Length;

    if (serializedKeypair.Length() < kP256_PublicKey_Length + KEYPAIR_KEYID_OFFSET + 4)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    // Verify magic number
    if (memcmp(privKeyRef, se05x_magic_no, sizeof(se05x_magic_no)) != 0)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Extract KeyID (big-endian, 4 bytes)
    outKeyId = (privKeyRef[KEYPAIR_KEYID_OFFSET] << 24) | (privKeyRef[KEYPAIR_KEYID_OFFSET + 1] << 16) |
        (privKeyRef[KEYPAIR_KEYID_OFFSET + 2] << 8) | (privKeyRef[KEYPAIR_KEYID_OFFSET + 3]);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PersistentStorageOpKeystorese05x::NewOpKeypairForFabric(FabricIndex fabricIndex,
                                                                   MutableByteSpan & outCertificateSigningRequest)
{
    P256SerializedKeypair serializedKeypair;
    uint8_t privatekey[32] = CHIP_SE05x_NODE_OP_REF_KEY_TEMPLATE;
    uint8_t publickey[65]  = { 0x00 };
    size_t privatekey_len  = sizeof(privatekey);
    size_t pubkey_len      = sizeof(publickey);
    uint32_t newKeyId      = 0;
    uint32_t existingKeyId = 0;
    bool hasExistingKey    = false;
    CHIP_ERROR err         = CHIP_NO_ERROR;
    uint32_t slotAKeyId    = 0;
    uint32_t slotBKeyId    = 0;
    uint16_t tlvBufSize    = 0;
    Crypto::SensitiveDataBuffer<OpKeyTLVMaxSize()> tlvBuf;
    CHIP_ERROR kvsErr = CHIP_NO_ERROR;

    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    // If a key is pending, we cannot generate for a different fabric index until we commit or revert.
    if ((mPendingFabricIndex != kUndefinedFabricIndex) && (fabricIndex != mPendingFabricIndex))
    {
        return CHIP_ERROR_INVALID_FABRIC_INDEX;
    }

    VerifyOrReturnError(outCertificateSigningRequest.size() >= Crypto::kMIN_CSR_Buffer_Size, CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(se05x_session_open() == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);

    // Replace previous pending key pair, if any was previously allocated
    ResetPendingKey();

    mPendingKeypair = Platform::New<Crypto::P256KeypairSE05x>();
    VerifyOrExit(mPendingKeypair != nullptr, err = CHIP_ERROR_NO_MEMORY);

    /* Determine the slot to use based on KVS */
    slotAKeyId = GetSlotAKeyId(fabricIndex);
    slotBKeyId = GetSlotBKeyId(fabricIndex);

    ChipLogProgress(Crypto, "Fabric %u: Slot A = 0x%" PRIx32 ", Slot B = 0x%" PRIx32, fabricIndex, slotAKeyId, slotBKeyId);

    // Check KVS to see if there's an existing committed key for this fabric
    tlvBufSize = static_cast<uint16_t>(tlvBuf.Capacity());

    kvsErr = mStorage->SyncGetKeyValue(DefaultStorageKeyAllocator::FabricOpKey(fabricIndex).KeyName(), tlvBuf.Bytes(), tlvBufSize);

    if (kvsErr == CHIP_NO_ERROR)
    {
        TLV::TLVReader reader;
        P256SerializedKeypair existingKeyRef;
        ByteSpan existingKeyData;

        reader.Init(tlvBuf.Bytes(), tlvBufSize);

        err = reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag());
        SuccessOrExit(err);

        TLV::TLVType containerType;
        err = reader.EnterContainer(containerType);
        SuccessOrExit(err);

        err = reader.Next(kOpKeyVersionTag);
        SuccessOrExit(err);
        uint16_t version;
        err = reader.Get(version);
        SuccessOrExit(err);
        VerifyOrExit(version == kOpKeyVersion, err = CHIP_ERROR_VERSION_MISMATCH);

        err = reader.Next(kOpKeyDataTag);
        SuccessOrExit(err);

        err = reader.Get(existingKeyData);
        SuccessOrExit(err);

        // Extract the key ID from the existing key reference
        VerifyOrExit(existingKeyData.size() <= existingKeyRef.Capacity(), err = CHIP_ERROR_BUFFER_TOO_SMALL);

        memcpy(existingKeyRef.Bytes(), existingKeyData.data(), existingKeyData.size());
        err = existingKeyRef.SetLength(existingKeyData.size());
        SuccessOrExit(err);

        err = ExtractKeyIdFromSerializedKeypair(existingKeyRef, existingKeyId);
        if (err == CHIP_NO_ERROR)
        {
            hasExistingKey = true;
            ChipLogProgress(Crypto, "Fabric %u: Found existing key in KVS at slot 0x%" PRIx32, fabricIndex, existingKeyId);
        }
        else
        {
            ChipLogError(Crypto, "Fabric %u: Failed to extract key ID from KVS: %" CHIP_ERROR_FORMAT, fabricIndex, err.Format());
            // Treat as no existing key - continue with first commission
            hasExistingKey = false;
            err            = CHIP_NO_ERROR;
        }
    }
    else if (kvsErr == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        ChipLogProgress(Crypto, "Fabric %u: No existing key found in KVS - first commission", fabricIndex);
        hasExistingKey = false;
    }
    else
    {
        ChipLogError(Crypto, "Fabric %u: Error reading from KVS: %" CHIP_ERROR_FORMAT, fabricIndex, kvsErr.Format());
        ExitNow(err = kvsErr);
    }

    if (!hasExistingKey)
    {
        // First time commissioning - use Slot A
        newKeyId = slotAKeyId;
        ChipLogProgress(Crypto, "Fabric %u: Using Slot A (0x%" PRIx32 ")", fabricIndex, newKeyId);
    }
    else
    {
        // Key update - use the alternate slot
        if (existingKeyId == slotAKeyId)
        {
            newKeyId = slotBKeyId;
            ChipLogProgress(Crypto, "Fabric %u: Using Slot B (0x%" PRIx32 ")", fabricIndex, newKeyId);
        }
        else if (existingKeyId == slotBKeyId)
        {
            newKeyId = slotAKeyId;
            ChipLogProgress(Crypto, "Fabric %u: Using Slot A (0x%" PRIx32 ")", fabricIndex, newKeyId);
        }
        else
        {
            // Unexpected key ID - this shouldn't happen, but handle it gracefully
            ChipLogError(Crypto,
                         "Fabric %u: Unexpected existing key ID 0x%" PRIx32 " (expected 0x%" PRIx32 " or 0x%" PRIx32
                         ") - using Slot A",
                         fabricIndex, existingKeyId, slotAKeyId, slotBKeyId);
            newKeyId = slotAKeyId;
        }
    }

    // Build the key reference with the new slot ID
    privatekey[CHIP_SE05x_NODE_OP_KEY_ID_INDEX - 3] = static_cast<uint8_t>((newKeyId >> 24) & 0xFF);
    privatekey[CHIP_SE05x_NODE_OP_KEY_ID_INDEX - 2] = static_cast<uint8_t>((newKeyId >> 16) & 0xFF);
    privatekey[CHIP_SE05x_NODE_OP_KEY_ID_INDEX - 1] = static_cast<uint8_t>((newKeyId >> 8) & 0xFF);
    privatekey[CHIP_SE05x_NODE_OP_KEY_ID_INDEX]     = static_cast<uint8_t>((newKeyId >> 0) & 0xFF);

    memcpy(serializedKeypair.Bytes(), publickey, pubkey_len);
    memcpy(serializedKeypair.Bytes() + pubkey_len, privatekey, privatekey_len);
    err = serializedKeypair.SetLength(privatekey_len + pubkey_len);
    SuccessOrExit(err);

    // This is required to ensure we pass the key id (mapping to fabric id) to CHIPCryptoPALHsm_se05x_p256.cpp NIST256 class.
    err = mPendingKeypair->Deserialize(serializedKeypair);
    SuccessOrExit(err);

    ChipLogProgress(Crypto, "Fabric %u: Generating NIST P-256 key in SE05x at slot 0x%" PRIx32, fabricIndex, newKeyId);

    err = mPendingKeypair->Initialize(Crypto::ECPKeyTarget::ECDSA);
    SuccessOrExit(err);

    {
        size_t csrLength = outCertificateSigningRequest.size();
        err              = mPendingKeypair->NewCertificateSigningRequest(outCertificateSigningRequest.data(), csrLength);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Crypto, "Fabric %u: Failed to generate CSR: %" CHIP_ERROR_FORMAT, fabricIndex, err.Format());
            se05x_delete_key(newKeyId);
            ExitNow();
        }
        outCertificateSigningRequest.reduce_size(csrLength);
    }

    mPendingFabricIndex = fabricIndex;

    ChipLogProgress(Crypto, "Fabric %u: Successfully generated new operational key at slot 0x%" PRIx32, fabricIndex, newKeyId);

    err = CHIP_NO_ERROR;

exit:
    if (se05x_close_session() != CHIP_NO_ERROR)
    {
        ChipLogError(Crypto, "SE05x: Error closing session during cleanup");
    }

    if (err != CHIP_NO_ERROR)
    {
        ResetPendingKey();
    }

    return err;
}

CHIP_ERROR PersistentStorageOpKeystorese05x::CommitOpKeypairForFabric(FabricIndex fabricIndex)
{
    CHIP_ERROR err        = CHIP_NO_ERROR;
    uint32_t slotAKeyId   = 0;
    uint32_t slotBKeyId   = 0;
    uint32_t pendingKeyId = 0;
    uint32_t oldKeyId     = 0;
    uint32_t opKeyLength  = 0;

    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mPendingKeypair != nullptr, CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex) && (fabricIndex == mPendingFabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(mIsPendingKeypairActive == true, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(se05x_session_open() == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);

    slotAKeyId = GetSlotAKeyId(fabricIndex);
    slotBKeyId = GetSlotBKeyId(fabricIndex);

    Crypto::SensitiveDataBuffer<OpKeyTLVMaxSize()> tlvBuf;
    TLV::TLVWriter writer;
    TLV::TLVType outerType;

    // Extract the new key ID from pending keypair
    P256SerializedKeypair newKeyRef;

    err = mPendingKeypair->Serialize(newKeyRef);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Crypto, "Fabric %u: Failed to serialize pending keypair: %" CHIP_ERROR_FORMAT, fabricIndex, err.Format());
        ExitNow();
    }

    err = ExtractKeyIdFromSerializedKeypair(newKeyRef, pendingKeyId);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Crypto, "Fabric %u: Failed to extract key ID from pending keypair: %" CHIP_ERROR_FORMAT, fabricIndex,
                     err.Format());
        ExitNow();
    }

    // Verify the new key exists in SE05x
    {
        bool pendingKeyExists = false;
        err                   = se05x_check_object_exists(pendingKeyId, &pendingKeyExists);
        if (err != CHIP_NO_ERROR || !pendingKeyExists)
        {
            ChipLogError(Crypto, "Fabric %u: New key does not exist at slot 0x%" PRIx32, fabricIndex, pendingKeyId);
            ExitNow(err = CHIP_ERROR_INVALID_FABRIC_INDEX);
        }
    }

    ChipLogProgress(Crypto, "Fabric %u: Committing new key at (0x%" PRIx32 ")", fabricIndex, pendingKeyId);

    writer.Init(tlvBuf.Bytes(), tlvBuf.Capacity());

    err = writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType);
    SuccessOrExit(err);

    // Write version tag
    err = writer.Put(kOpKeyVersionTag, kOpKeyVersion);
    SuccessOrExit(err);

    // Write the serialized keypair (SE05x key reference)
    err = writer.Put(kOpKeyDataTag, ByteSpan(newKeyRef.Bytes(), newKeyRef.Length()));
    SuccessOrExit(err);

    err = writer.EndContainer(outerType);
    SuccessOrExit(err);

    opKeyLength = writer.GetLengthWritten();

    if (!CanCastTo<uint16_t>(opKeyLength))
    {
        ChipLogError(Crypto, "Fabric %u: TLV data too large", fabricIndex);
        ExitNow(err = CHIP_ERROR_BUFFER_TOO_SMALL);
    }

    // Store the new key reference in persistent storage
    err = mStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::FabricOpKey(fabricIndex).KeyName(), tlvBuf.ConstBytes(),
                                    static_cast<uint16_t>(opKeyLength));
    SuccessOrExit(err);

    ChipLogProgress(Crypto, "Fabric %u: Successfully stored new key reference in KVS", fabricIndex);

    // Delete the OLD key from the other slot
    oldKeyId = (pendingKeyId == slotAKeyId) ? slotBKeyId : slotAKeyId;

    ChipLogProgress(Crypto, "Fabric %u: Deleting old key (0x%" PRIx32 ")", fabricIndex, oldKeyId);
    se05x_delete_key(oldKeyId);

    // Reset pending key state
    ResetPendingKey();

    err = CHIP_NO_ERROR;

exit:
    if (se05x_close_session() != CHIP_NO_ERROR)
    {
        ChipLogError(Crypto, "SE05x: Error closing session");
    }
    return err;
}

CHIP_ERROR PersistentStorageOpKeystorese05x::RemoveOpKeypairForFabric(FabricIndex fabricIndex)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(se05x_session_open() == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);

    uint32_t slotAKeyId = GetSlotAKeyId(fabricIndex);
    uint32_t slotBKeyId = GetSlotBKeyId(fabricIndex);

    ChipLogDetail(Crypto, "Fabric %u: Removing operational keys - Slot A: 0x%" PRIx32 ", Slot B: 0x%" PRIx32, fabricIndex,
                  slotAKeyId, slotBKeyId);

    // Delete both Slot A and Slot B keys
    se05x_delete_key(slotAKeyId);
    se05x_delete_key(slotBKeyId);

    // Remove pending keypair from memory if it matches this fabric
    if ((mPendingKeypair != nullptr) && (fabricIndex == mPendingFabricIndex))
    {
        RevertPendingKeypair();
    }

    // Remove from persistent storage
    err = mStorage->SyncDeleteKeyValue(DefaultStorageKeyAllocator::FabricOpKey(fabricIndex).KeyName());
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        ChipLogError(Crypto, "Fabric %u: Key not found in persistent storage", fabricIndex);
    }

    if (se05x_close_session() != CHIP_NO_ERROR)
    {
        ChipLogError(Crypto, "SE05x: Error closing session");
    }

    return err;
}

CHIP_ERROR PersistentStorageOpKeystorese05x::SignWithOpKeypair(FabricIndex fabricIndex, const ByteSpan & message,
                                                               Crypto::P256ECDSASignature & outSignature) const
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(se05x_session_open() == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);

    ChipLogDetail(Crypto, "PersistentStorageOpKeystorese05x::SignWithOpKeypair :: ECDSA Sign using SE05x ");

    if (mIsPendingKeypairActive && (fabricIndex == mPendingFabricIndex))
    {
        VerifyOrReturnError(mPendingKeypair != nullptr, CHIP_ERROR_INTERNAL);
        // We have an override key: sign with it!
        return mPendingKeypair->ECDSA_sign_msg(message.data(), message.size(), outSignature);
    }

    // Use ExportOpKeypairForFabric from base class directly
    auto transientOperationalKeypair = Platform::MakeUnique<P256KeypairSE05x>();
    if (!transientOperationalKeypair)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    P256SerializedKeypair serializedOpKey;
    // Call base class method directly using 'this'
    CHIP_ERROR err = const_cast<PersistentStorageOpKeystorese05x *>(this)->ExportOpKeypairForFabric(fabricIndex, serializedOpKey);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        return CHIP_ERROR_INVALID_FABRIC_INDEX;
    }
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Crypto, "Failed to export keypair for fabric %u: %" CHIP_ERROR_FORMAT, fabricIndex, err.Format());
    }
    ReturnErrorOnFailure(err);

    ReturnErrorOnFailure(transientOperationalKeypair->Deserialize(serializedOpKey));
    return transientOperationalKeypair->ECDSA_sign_msg(message.data(), message.size(), outSignature);
}

} // namespace chip
