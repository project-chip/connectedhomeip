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

/**
 * @brief Reset binary data in SE05x
 *
 * This function checks if an object with the given keyid already exists in the secure element.
 * If it exists and the size differs from the new data, it deletes the old object before writing.
 * This prevents errors when trying to overwrite objects with different sizes.
 *
 * @param[in] keyid - Key id of the object
 * @param[in] buf - Buffer containing binary data
 * @param[in] buflen - Buffer length
 * @return CHIP_ERROR_INTERNAL on error, CHIP_NO_ERROR otherwise
 */
static CHIP_ERROR se05x_reset_binary_data(uint32_t keyid, const uint8_t * buf, size_t buflen)
{
    uint16_t seObjectSize = 0;
    CHIP_ERROR err = se05x_read_object_size(keyid, &seObjectSize);

    if (err == CHIP_NO_ERROR)
    {
        if (seObjectSize != buflen)
        {
            ChipLogProgress(Crypto, "SE05x: Size mismatch for keyid 0x%" PRIx32 " (SE: %" PRIu16 " bytes, new: %" PRIu16 " bytes) - deleting old object",
                keyid, seObjectSize, static_cast<uint16_t>(buflen));
            se05x_delete_key(keyid);
        }
    }
    else
    {
        ChipLogDetail(Crypto, "SE05x: No existing object found for keyid 0x%" PRIx32, keyid);
    }

    err = se05x_set_binary_data(keyid, buf, buflen);
    return err;
}


static CHIP_ERROR generate_node_oper_key()
{
    sss_object_t keyObject = { 0 };
    sss_status_t status    = kStatus_SSS_Fail;

    status = sss_key_object_init(&keyObject, &gex_sss_chip_ctx.ks);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    status = sss_key_object_allocate_handle(&keyObject, CHIP_SE05x_NODE_OP_KEY_INDEX + 1, kSSS_KeyPart_Pair,
                                            kSSS_CipherType_EC_NIST_P, 256, kKeyObject_Mode_Persistent);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    status = sss_key_store_generate_key(&gex_sss_chip_ctx.ks, &keyObject, 256, 0);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

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

    err = se05x_disable_nfc_commision();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Crypto,
            "Fabric %u: Failed to disable NFC commissioning: %" CHIP_ERROR_FORMAT,
            fabricIndex, err.Format());
    }
    else
    {
        ChipLogProgress(Crypto,
            "Fabric %u: NFC commissioning disabled after successful commit",
            fabricIndex);
    }

    se05x_host_gpio_notification_monitor_deinit();

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

static CHIP_ERROR se05xResetNFCCommData();

CHIP_ERROR PersistentStorageOpKeystorese05x::RemoveOpKeypairForFabric(FabricIndex fabricIndex)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(se05x_session_open() == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);

    uint32_t slotAKeyId = GetSlotAKeyId(fabricIndex);
    uint32_t slotBKeyId = GetSlotBKeyId(fabricIndex);
    bool allDeleted = true;
    uint8_t kvsFabricCount = 0;

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

    // To check if there are any fabrics in KVS
    for (FabricIndex i = kMinValidFabricIndex; i <= kMaxValidFabricIndex; i++)
    {
        uint8_t kvs_node_oper_key[256] = { 0 };
        uint16_t kvs_key_len                         = sizeof(kvs_node_oper_key);
        CHIP_ERROR kvErr = mStorage->SyncGetKeyValue(DefaultStorageKeyAllocator::FabricOpKey(i).KeyName(), kvs_node_oper_key, kvs_key_len);
        if (kvErr == CHIP_NO_ERROR)
        {
            allDeleted = false;
            kvsFabricCount++;
        }
    }

    ChipLogProgress(Crypto, "Fabrics in KVS =: %u", kvsFabricCount);

    if (allDeleted)
    {
        err = se05x_enable_nfc_commision();
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Crypto, "Failed to enable NFC commissioning: %" CHIP_ERROR_FORMAT, err.Format());
        }
        else
        {
            ChipLogProgress(Crypto, "NOTE: All node OP keys deleted — NFC commissioning enabled.");
        }

        err = generate_node_oper_key();
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Crypto, "Failed to dummy key pair (at id = 0x%" PRIx32 ")", (uint32_t) (CHIP_SE05x_NODE_OP_KEY_INDEX + 1));
            goto exit;
        }
        else
        {
            ChipLogProgress(Crypto, "Created dummy key pair (at id = 0x%" PRIx32 ")",
                            (uint32_t) (CHIP_SE05x_NODE_OP_KEY_INDEX + 1));
        }

        /* Create a dummy key pair at node operational key location,
        so that if the NFC commissioned data is provisioned, user can do the NFC commission. */
        err = se05xResetNFCCommData();
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Crypto, "Failed to reset nfc commissioned data");
            goto exit;
        }
    }

    err = CHIP_NO_ERROR;
exit:
    if (se05x_close_session() != CHIP_NO_ERROR)
    {
        ChipLogError(Crypto, "se05x::Error in se05x_close_session.");
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

#define SE05X_SET_BIN_DATA_TEMPLATE(keyid, buf)                                                                                    \
    {                                                                                                                              \
        const uint8_t buffer[] = { buf };                                                                                          \
        err                    = se05x_reset_binary_data(keyid, buffer, sizeof(buffer));                                             \
        VerifyOrReturnError(err == CHIP_NO_ERROR, err);                                                                            \
    }

#define SE05X_SET_EC_KEY(keyid, buf)                                \
    {                                                               \
        const uint8_t buffer[] = {buf};                             \
        err = se05x_set_ec_key(keyid, buffer, sizeof(buffer));      \
        VerifyOrReturnError(err == CHIP_NO_ERROR, err);             \
    }

static CHIP_ERROR se05xResetNFCCommData()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    /* Delete NFC commissioned data also, like Operational Credential cluster,
    Root CA, ICA, IPK, Access control cluster, Wi-fi / thread credentials,  */
    SE05X_SET_BIN_DATA_TEMPLATE(SE051H_OP_CRED_CLUSTER_ID, OCC);
    SE05X_SET_BIN_DATA_TEMPLATE(SE051H_ROOT_CER_ID, ROOT_CERTIFICATE);
    SE05X_SET_BIN_DATA_TEMPLATE(SE051H_IPK_ID, IPK);
    SE05X_SET_BIN_DATA_TEMPLATE(SE051H_ACL_ID, ACL);
    SE05X_SET_BIN_DATA_TEMPLATE(SE051H_WIFI_CRED_ID_APP_8_4, WIFI_CRED_DATA);
    SE05X_SET_BIN_DATA_TEMPLATE(SE051H_WIFI_CRED_ID_APP_8_8, WIFI_CRED_DATA);
    SE05X_SET_BIN_DATA_TEMPLATE(SE051H_VR_ID, VENDOR_RESERVED);
    SE05X_SET_BIN_DATA_TEMPLATE(SE051H_NOC_ID, NOC);
    SE05X_SET_BIN_DATA_TEMPLATE(SE051H_PAI_ID, PAI_CERTIFICATE);
    SE05X_SET_BIN_DATA_TEMPLATE(SE051H_DAC_ID, DAC_CERTIFICATE);
    SE05X_SET_EC_KEY(SE051H_DA_KEY_PAIR_ID, DA_KEY_PAIR_DATA);

    {
        uint8_t attest_tbs[] = {STRUCTURE_START,   CERTIFICATE_DECLARATION,
                      ATTESTATION_NONCE, SE_TIMESTAMP,
                      STRUCTURE_END,     ATTESTATION_CHALLENGE};

        err = se05x_reset_binary_data(SE051H_ATTEST_TBS, attest_tbs, sizeof(attest_tbs));
        VerifyOrReturnError(err == CHIP_NO_ERROR, err);
    }

    {
        uint8_t acl_data[] = {
            DATA_VERSION_ACC,
            CLUSTER_REVISION_ACC,
            FEATUREMAP_ACC,
            ATTRIBUTE_LIST_ACC,
            ACCEPTED_COMMAND_LIST_ACC,
            GENERATED_COMMAND_LIST_ACC,
            ACLS,
            EXTENSION,
            EXTENSION_FILLER,
            SUBJECTS_PER_ACCESS_CONTROL_ENTRY,
            TARGETS_PER_ACCESS_CONTROL_ENTRY,
            ACCESS_CONTROL_ENTRIES_PER_FABRIC,
            COMMISSIONING_ARL,
            COMMISSIONING_ARL_FILLER,
            ARL,
            ARL_FILLER,
        };

        err = se05x_reset_binary_data(SE051H_ACC_ID, acl_data, sizeof(acl_data));
        VerifyOrReturnError(err == CHIP_NO_ERROR, err);

    }

    {
        uint8_t ncc_buf[] = {
            DATA_VERSION_NCC,
            CLUSTER_REVISION_NCC,
            FEATUREMAP_NCC,
            ATTRIBUTE_LIST_NCC,
            ACCEPTED_COMMAND_LIST_NCC,
            GENERATED_COMMAND_LIST_NCC,
            MAX_NETWORKS,
            NETWORKS,
            NETWORKS_FILLER,
            SCAN_MAX_TIME_SECONDS,
            CONNECT_MAX_TIME_SECONDS,
            INTERFACE_ENABLED,
            LAST_NETWORKING_STATUS,
            LAST_NETWORK_ID,
            LAST_NETWORK_ID_FILLER,
            LAST_CONNECT_ERROR_VALUE_NCC,
            LAST_CONNECT_ERROR_VALUE_FILLER_NCC,
            SUPPORTED_WIFI_BANDS_NCC,
            SUPPORTED_THREAD_FEATURES_NCC,
            THREAD_VERSION_NCC,
        };

        uint8_t buffer[512] = {0};
        size_t buf_len = sizeof(buffer);

        // read NCC to check what was the last network interface type set for NFC comm
        smStatus_t smstatus = SM_NOT_OK;
        SE05x_Result_t exists = kSE05x_Result_NA;

        smstatus = Se05x_API_CheckObjectExists(
        &((sss_se05x_session_t *)&gex_sss_chip_ctx.session)->s_ctx, SE051H_NCC_ID,
        &exists);
        if (smstatus == SM_OK) {
            if (exists == kSE05x_Result_SUCCESS) {
                err = se05x_get_certificate(SE051H_NCC_ID, buffer, &buf_len);
                VerifyOrReturnError(err == CHIP_NO_ERROR, err);

                VerifyOrReturnError(buf_len > NETWORK_INTERFACE_BTYE_OFFSET, CHIP_ERROR_BUFFER_TOO_SMALL);

                ncc_buf[NETWORK_INTERFACE_BTYE_OFFSET] = buffer[NETWORK_INTERFACE_BTYE_OFFSET];
            }
        }
        else {
            ChipLogError(Crypto, "Se05x_API_CheckObjectExists Failed");
        }

        err = se05x_reset_binary_data(SE051H_NCC_ID, ncc_buf, sizeof(ncc_buf));
        VerifyOrReturnError(err == CHIP_NO_ERROR, err);
    }

    {
        uint8_t Genaral_comm_cluster_data[] = {
            DATA_VERSION_GCC,
            CLUSTER_REVISION_GCC,
            FEATUREMAP_GCC,
            ATTRIBUTE_LIST_GCC,
            ACCEPTED_COMMAND_LIST_GCC,
            GENERATED_COMMAND_LIST_GCC,
            BREADCRUMB,
            BASIC_COMMISSIONING_INFO,
            REGULATORY_CONFIG,
            LOCATION_CAPABILITY,
            SUPPORTS_CONCURRENT_CONNECTION,
            TC_ACCEPTED_VERSION,
            TC_MIN_REQUIRED_VERSION,
            TC_ACKNOWLEDGEMENTS,
            TC_ACKNOWLEDGEMENTS_REQUIRED,
            TC_UPDATE_DEADLINE,
            IS_COMM_WITHOUT_POWER,
        };
        err =
            se05x_reset_binary_data(SE051H_GENERAL_COMM_CLUSTER_ID, Genaral_comm_cluster_data, sizeof(Genaral_comm_cluster_data));
        VerifyOrReturnError(err == CHIP_NO_ERROR, err);
    }

    {
        uint8_t Basic_info_cluster_data[] = {
            DATA_VERSION_BIC,
            CLUSTER_REVISION_BIC,
            FEATUREMAP_BIC,
            ATTRIBUTE_LIST_BIC,
            ACCEPTED_COMMAND_LIST_BIC,
            GENERATED_COMMAND_LIST_BIC,
            DATA_MODEL_REVISION,
            VENDOR_NAME,
            VENDOR_NAME_FILLER,
            VENDOR_ID,
            PRODUCT_NAME,
            PRODUCT_NAME_FILLER,
            PRODUCT_ID,
            NODE_LABEL,
            NODE_LABEL_FILLER,
            LOCATION,
            HARDWARE_VERSION,
            HARDWARE_VERSIONING,
            HARDWARE_VERSIONING_FILLER,
            SOFTWARE_VERSION,
            SOFTWARE_VERSIONING,
            SOFTWARE_VERSIONING_FILLER,
            UNIQUE_ID,
            UNIQUE_ID_FILLER,
            CAPABILITY_MINIMA,
            SPECIFICATION_VERSION,
            MAX_PATH_PER_INVOKE,
            CONFIGURATION_VERSION};

        err = se05x_reset_binary_data(SE051H_BASIC_INFO_CLUSTER_ID, Basic_info_cluster_data, sizeof(Basic_info_cluster_data));
        VerifyOrReturnError(err == CHIP_NO_ERROR, err);
    }

    return CHIP_NO_ERROR;
}

} // namespace chip
