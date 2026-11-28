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

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/Span.h>
#include <pw_unit_test/framework.h>
#include <system/SystemPacketBuffer.h>

#include <app/icd/client/DefaultICDClientStorage.h>
#include <crypto/DefaultSessionKeystore.h>
#include <lib/support/AutoRelease.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <protocols/secure_channel/CheckinMessage.h>
#include <transport/SessionManager.h>

#if CHIP_CRYPTO_KEYSTORE_PSA
#include <crypto/PSAKeyAllocator.h>
#include <psa/crypto.h>
#endif

using namespace chip;
using namespace app;
using namespace System;
using TestSessionKeystoreImpl = Crypto::DefaultSessionKeystore;

constexpr uint8_t kKeyBuffer1[] = {
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
};

constexpr uint8_t kKeyBuffer2[] = {
    0xf1, 0xe1, 0xd1, 0xc1, 0xb1, 0xa1, 0x91, 0x81, 0x71, 0x61, 0x51, 0x14, 0x31, 0x21, 0x11, 0x01
};
constexpr uint8_t kKeyBuffer3[] = {
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f
};

struct TestClientInfo : public ICDClientInfo
{
    bool operator==(const ICDClientInfo & that) const
    {
        if ((peer_node != that.peer_node))
        {
            return false;
        }
        return true;
    }
};

class TestDefaultICDClientStorage : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void TearDown() override
    {
#if CHIP_CRYPTO_KEYSTORE_PSA
        (void) psa_crypto_init();
        for (psa_key_id_t id = static_cast<psa_key_id_t>(chip::Crypto::KeyIdBase::ICDKeyRangeStart);
             id < static_cast<psa_key_id_t>(chip::Crypto::KeyIdBase::Maximum); id++)
        {
            psa_destroy_key(id);
        }
#endif
    }
};

TEST_F(TestDefaultICDClientStorage, TestClientInfoCount)
{
    FabricIndex fabricId = 1;
    NodeId nodeId1       = 6666;
    NodeId nodeId2       = 6667;
    TestPersistentStorageDelegate clientInfoStorage;
    TestSessionKeystoreImpl keystore;

    {
        DefaultICDClientStorage manager;
        EXPECT_EQ(manager.Init(&clientInfoStorage, &keystore), CHIP_NO_ERROR);
        EXPECT_EQ(manager.UpdateFabricList(fabricId), CHIP_NO_ERROR);
        // Write some ClientInfos and see the counts are correct
        ICDClientInfo clientInfo1;
        clientInfo1.peer_node = ScopedNodeId(nodeId1, fabricId);
        ICDClientInfo clientInfo2;
        clientInfo2.peer_node = ScopedNodeId(nodeId2, fabricId);
        ICDClientInfo clientInfo3;
        clientInfo3.peer_node = ScopedNodeId(nodeId1, fabricId);
        EXPECT_EQ(manager.SetKey(clientInfo1, ByteSpan(kKeyBuffer1)), CHIP_NO_ERROR);
        EXPECT_EQ(manager.StoreEntry(clientInfo1), CHIP_NO_ERROR);

        EXPECT_EQ(manager.SetKey(clientInfo2, ByteSpan(kKeyBuffer2)), CHIP_NO_ERROR);
        EXPECT_EQ(manager.StoreEntry(clientInfo2), CHIP_NO_ERROR);

        EXPECT_EQ(manager.SetKey(clientInfo3, ByteSpan(kKeyBuffer3)), CHIP_NO_ERROR);
        EXPECT_EQ(manager.StoreEntry(clientInfo3), CHIP_NO_ERROR);

        ICDClientInfo clientInfo;
        // Make sure iterator counts correctly
        auto * iterator = manager.IterateICDClientInfo();
        // same nodeId for clientInfo2 and clientInfo3, so the new one replace old one
        EXPECT_EQ(iterator->Count(), 2u);

        EXPECT_TRUE(iterator->Next(clientInfo));
        EXPECT_EQ(clientInfo.peer_node.GetNodeId(), nodeId2);
        EXPECT_TRUE(iterator->Next(clientInfo));
        EXPECT_EQ(clientInfo.peer_node.GetNodeId(), nodeId1);

        iterator->Release();

        EXPECT_TRUE(manager.GetClientInfoStore()->SyncDoesKeyExist(DefaultStorageKeyAllocator::ICDFabricList().KeyName()));
        EXPECT_TRUE(manager.GetClientInfoStore()->SyncDoesKeyExist(
            DefaultStorageKeyAllocator::FabricICDClientInfoCounter(fabricId).KeyName()));
        EXPECT_TRUE(
            manager.GetClientInfoStore()->SyncDoesKeyExist(DefaultStorageKeyAllocator::ICDClientInfoKey(fabricId).KeyName()));

        // Delete all and verify iterator counts 0
        EXPECT_EQ(manager.DeleteAllEntries(fabricId), CHIP_NO_ERROR);
        iterator = manager.IterateICDClientInfo();
        EXPECT_EQ(iterator->Count(), 0u);

        // Verify ClientInfos manually count correctly
        size_t count = 0;
        while (iterator->Next(clientInfo))
        {
            count++;
        }
        iterator->Release();
        EXPECT_EQ(count, 0u);
        EXPECT_EQ(manager.GetFabricListSize(), 0u);
        EXPECT_FALSE(manager.GetClientInfoStore()->SyncDoesKeyExist(DefaultStorageKeyAllocator::ICDFabricList().KeyName()));
        EXPECT_FALSE(manager.GetClientInfoStore()->SyncDoesKeyExist(
            DefaultStorageKeyAllocator::FabricICDClientInfoCounter(fabricId).KeyName()));
        EXPECT_FALSE(
            manager.GetClientInfoStore()->SyncDoesKeyExist(DefaultStorageKeyAllocator::ICDClientInfoKey(fabricId).KeyName()));

        EXPECT_EQ(manager.DeleteAllEntries(fabricId), CHIP_NO_ERROR);
    }

    {
        DefaultICDClientStorage manager;
        EXPECT_EQ(manager.Init(&clientInfoStorage, &keystore), CHIP_NO_ERROR);
        EXPECT_EQ(manager.UpdateFabricList(fabricId), CHIP_NO_ERROR);
    }
}

TEST_F(TestDefaultICDClientStorage, TestClientInfoCountMultipleFabric)
{

    FabricIndex fabricId1 = 1;
    FabricIndex fabricId2 = 2;
    NodeId nodeId1        = 6666;
    NodeId nodeId2        = 6667;
    NodeId nodeId3        = 6668;
    DefaultICDClientStorage manager;
    TestPersistentStorageDelegate clientInfoStorage;
    TestSessionKeystoreImpl keystore;
    EXPECT_EQ(manager.Init(&clientInfoStorage, &keystore), CHIP_NO_ERROR);
    EXPECT_EQ(manager.UpdateFabricList(fabricId1), CHIP_NO_ERROR);
    EXPECT_EQ(manager.UpdateFabricList(fabricId2), CHIP_NO_ERROR);

    // Write some ClientInfos and see the counts are correct
    ICDClientInfo clientInfo1;
    clientInfo1.peer_node = ScopedNodeId(nodeId1, fabricId1);
    ICDClientInfo clientInfo2;
    clientInfo2.peer_node = ScopedNodeId(nodeId2, fabricId1);
    ICDClientInfo clientInfo3;
    clientInfo3.peer_node = ScopedNodeId(nodeId3, fabricId2);

    EXPECT_EQ(manager.SetKey(clientInfo1, ByteSpan(kKeyBuffer1)), CHIP_NO_ERROR);
    EXPECT_EQ(manager.StoreEntry(clientInfo1), CHIP_NO_ERROR);

    EXPECT_EQ(manager.SetKey(clientInfo2, ByteSpan(kKeyBuffer2)), CHIP_NO_ERROR);
    EXPECT_EQ(manager.StoreEntry(clientInfo2), CHIP_NO_ERROR);

    EXPECT_EQ(manager.SetKey(clientInfo3, ByteSpan(kKeyBuffer3)), CHIP_NO_ERROR);
    EXPECT_EQ(manager.StoreEntry(clientInfo3), CHIP_NO_ERROR);
    // Make sure iterator counts correctly
    AutoRelease iterator(manager.IterateICDClientInfo());
    ASSERT_FALSE(iterator.IsNull());
    EXPECT_EQ(iterator->Count(), 3u);
    iterator.Release();

    // Delete all and verify iterator counts 0
    EXPECT_EQ(manager.DeleteEntry(ScopedNodeId(nodeId1, fabricId1)), CHIP_NO_ERROR);
    iterator.Set(manager.IterateICDClientInfo());
    ASSERT_FALSE(iterator.IsNull());
    EXPECT_EQ(iterator->Count(), 2u);

    EXPECT_EQ(manager.DeleteEntry(ScopedNodeId(nodeId2, fabricId1)), CHIP_NO_ERROR);
    EXPECT_EQ(iterator->Count(), 1u);

    EXPECT_EQ(manager.DeleteEntry(ScopedNodeId(nodeId3, fabricId2)), CHIP_NO_ERROR);
    EXPECT_EQ(iterator->Count(), 0u);

    EXPECT_EQ(manager.GetFabricListSize(), 2u);
    EXPECT_TRUE(manager.GetClientInfoStore()->SyncDoesKeyExist(DefaultStorageKeyAllocator::ICDFabricList().KeyName()));
    EXPECT_TRUE(manager.GetClientInfoStore()->SyncDoesKeyExist(
        DefaultStorageKeyAllocator::FabricICDClientInfoCounter(fabricId1).KeyName()));
    EXPECT_TRUE(manager.GetClientInfoStore()->SyncDoesKeyExist(DefaultStorageKeyAllocator::ICDClientInfoKey(fabricId1).KeyName()));
    EXPECT_TRUE(manager.GetClientInfoStore()->SyncDoesKeyExist(
        DefaultStorageKeyAllocator::FabricICDClientInfoCounter(fabricId2).KeyName()));
    EXPECT_TRUE(manager.GetClientInfoStore()->SyncDoesKeyExist(DefaultStorageKeyAllocator::ICDClientInfoKey(fabricId2).KeyName()));

    // Verify ClientInfos manually count correctly
    size_t count = 0;
    ICDClientInfo clientInfo;
    while (iterator->Next(clientInfo))
    {
        count++;
    }

    EXPECT_FALSE(count);

    EXPECT_EQ(manager.DeleteEntry(ScopedNodeId(nodeId3, fabricId2)), CHIP_NO_ERROR);
}

TEST_F(TestDefaultICDClientStorage, TestClientInfoCountMultipleFabricWithRemovingFabric)
{

    FabricIndex fabricId1 = 1;
    FabricIndex fabricId2 = 2;
    NodeId nodeId1        = 6666;
    NodeId nodeId2        = 6667;
    NodeId nodeId3        = 6668;
    DefaultICDClientStorage manager;
    TestPersistentStorageDelegate clientInfoStorage;
    TestSessionKeystoreImpl keystore;
    EXPECT_EQ(manager.Init(&clientInfoStorage, &keystore), CHIP_NO_ERROR);
    EXPECT_EQ(manager.UpdateFabricList(fabricId1), CHIP_NO_ERROR);
    EXPECT_EQ(manager.UpdateFabricList(fabricId2), CHIP_NO_ERROR);

    // Write some ClientInfos and see the counts are correct
    ICDClientInfo clientInfo1;
    clientInfo1.peer_node = ScopedNodeId(nodeId1, fabricId1);
    ICDClientInfo clientInfo2;
    clientInfo2.peer_node = ScopedNodeId(nodeId2, fabricId1);
    ICDClientInfo clientInfo3;
    clientInfo3.peer_node = ScopedNodeId(nodeId3, fabricId2);

    EXPECT_EQ(manager.SetKey(clientInfo1, ByteSpan(kKeyBuffer1)), CHIP_NO_ERROR);
    EXPECT_EQ(manager.StoreEntry(clientInfo1), CHIP_NO_ERROR);

    EXPECT_EQ(manager.SetKey(clientInfo2, ByteSpan(kKeyBuffer2)), CHIP_NO_ERROR);
    EXPECT_EQ(manager.StoreEntry(clientInfo2), CHIP_NO_ERROR);

    EXPECT_EQ(manager.SetKey(clientInfo3, ByteSpan(kKeyBuffer3)), CHIP_NO_ERROR);
    EXPECT_EQ(manager.StoreEntry(clientInfo3), CHIP_NO_ERROR);
    // Make sure iterator counts correctly
    AutoRelease iterator(manager.IterateICDClientInfo());
    ASSERT_FALSE(iterator.IsNull());
    EXPECT_EQ(iterator->Count(), 3u);
    iterator.Release();

    EXPECT_EQ(manager.DeleteAllEntries(fabricId1), CHIP_NO_ERROR);

    iterator.Set(manager.IterateICDClientInfo());
    ASSERT_FALSE(iterator.IsNull());
    EXPECT_EQ(iterator->Count(), 1u);

    EXPECT_EQ(manager.DeleteAllEntries(fabricId2), CHIP_NO_ERROR);
    EXPECT_EQ(iterator->Count(), 0u);

    EXPECT_EQ(manager.StoreEntry(clientInfo1), CHIP_ERROR_INVALID_FABRIC_INDEX);
    EXPECT_EQ(manager.StoreEntry(clientInfo2), CHIP_ERROR_INVALID_FABRIC_INDEX);
    EXPECT_EQ(manager.StoreEntry(clientInfo3), CHIP_ERROR_INVALID_FABRIC_INDEX);
}

TEST_F(TestDefaultICDClientStorage, TestProcessCheckInPayload)
{
    FabricIndex fabricId = 1;
    NodeId nodeId        = 6666;
    TestPersistentStorageDelegate clientInfoStorage;
    TestSessionKeystoreImpl keystore;

    DefaultICDClientStorage manager;
    EXPECT_EQ(manager.Init(&clientInfoStorage, &keystore), CHIP_NO_ERROR);
    EXPECT_EQ(manager.UpdateFabricList(fabricId), CHIP_NO_ERROR);
    // Populate clientInfo
    ICDClientInfo clientInfo;
    clientInfo.peer_node = ScopedNodeId(nodeId, fabricId);

    EXPECT_EQ(manager.SetKey(clientInfo, ByteSpan(kKeyBuffer1)), CHIP_NO_ERROR);
    EXPECT_EQ(manager.StoreEntry(clientInfo), CHIP_NO_ERROR);

    uint32_t counter                  = 1;
    System::PacketBufferHandle buffer = MessagePacketBuffer::New(chip::Protocols::SecureChannel::CheckinMessage::kMinPayloadSize);
    MutableByteSpan output{ buffer->Start(), buffer->MaxDataLength() };
    EXPECT_EQ(chip::Protocols::SecureChannel::CheckinMessage::GenerateCheckinMessagePayload(
                  clientInfo.aes_key_handle, clientInfo.hmac_key_handle, counter, ByteSpan(), output),
              CHIP_NO_ERROR);

    buffer->SetDataLength(static_cast<uint16_t>(output.size()));
    ICDClientInfo decodeClientInfo;
    uint32_t checkInCounter = 0;
    ByteSpan payload{ buffer->Start(), buffer->DataLength() };
    EXPECT_EQ(manager.ProcessCheckInPayload(payload, decodeClientInfo, checkInCounter), CHIP_NO_ERROR);
    EXPECT_EQ(checkInCounter, counter);

    // Validate second check-in message with increased counter
    counter++;
    EXPECT_EQ(chip::Protocols::SecureChannel::CheckinMessage::GenerateCheckinMessagePayload(
                  clientInfo.aes_key_handle, clientInfo.hmac_key_handle, counter, ByteSpan(), output),
              CHIP_NO_ERROR);
    buffer->SetDataLength(static_cast<uint16_t>(output.size()));
    ByteSpan payload1{ buffer->Start(), buffer->DataLength() };
    EXPECT_EQ(manager.ProcessCheckInPayload(payload1, decodeClientInfo, checkInCounter), CHIP_NO_ERROR);
    EXPECT_EQ(checkInCounter, counter);

    // Use a key not available in the storage for encoding
    ICDClientInfo clientInfo2;
    clientInfo2.peer_node = ScopedNodeId(nodeId, fabricId);
    EXPECT_EQ(manager.SetKey(clientInfo2, ByteSpan(kKeyBuffer2)), CHIP_NO_ERROR);
    EXPECT_EQ(chip::Protocols::SecureChannel::CheckinMessage::GenerateCheckinMessagePayload(
                  clientInfo2.aes_key_handle, clientInfo2.hmac_key_handle, counter, ByteSpan(), output),
              CHIP_NO_ERROR);

    buffer->SetDataLength(static_cast<uint16_t>(output.size()));
    ByteSpan payload2{ buffer->Start(), buffer->DataLength() };
    EXPECT_EQ(manager.ProcessCheckInPayload(payload2, decodeClientInfo, checkInCounter), CHIP_ERROR_NOT_FOUND);

    manager.RemoveKey(clientInfo2);
    EXPECT_SUCCESS(manager.DeleteAllEntries(fabricId));
}

TEST_F(TestDefaultICDClientStorage, TestProcessCheckInPayloadWithRemovedKey)
{
    FabricIndex fabricId = 1;
    NodeId nodeId        = 6666;
    TestPersistentStorageDelegate clientInfoStorage;
    TestSessionKeystoreImpl keystore;

    DefaultICDClientStorage manager;
    EXPECT_EQ(manager.Init(&clientInfoStorage, &keystore), CHIP_NO_ERROR);
    EXPECT_EQ(manager.UpdateFabricList(fabricId), CHIP_NO_ERROR);
    // Populate clientInfo
    ICDClientInfo clientInfo;
    clientInfo.peer_node = ScopedNodeId(nodeId, fabricId);

    EXPECT_EQ(manager.SetKey(clientInfo, ByteSpan(kKeyBuffer1)), CHIP_NO_ERROR);
    EXPECT_EQ(manager.StoreEntry(clientInfo), CHIP_NO_ERROR);

    uint32_t counter                  = 1;
    System::PacketBufferHandle buffer = MessagePacketBuffer::New(chip::Protocols::SecureChannel::CheckinMessage::kMinPayloadSize);
    MutableByteSpan output{ buffer->Start(), buffer->MaxDataLength() };
    EXPECT_EQ(chip::Protocols::SecureChannel::CheckinMessage::GenerateCheckinMessagePayload(
                  clientInfo.aes_key_handle, clientInfo.hmac_key_handle, counter, ByteSpan(), output),
              CHIP_NO_ERROR);

    buffer->SetDataLength(static_cast<uint16_t>(output.size()));
    ICDClientInfo decodeClientInfo;
    uint32_t checkInCounter = 0;
    ByteSpan payload{ buffer->Start(), buffer->DataLength() };
    EXPECT_EQ(manager.ProcessCheckInPayload(payload, decodeClientInfo, checkInCounter), CHIP_NO_ERROR);
    EXPECT_EQ(checkInCounter, counter);

    // Generate payload before removing the key
    EXPECT_EQ(chip::Protocols::SecureChannel::CheckinMessage::GenerateCheckinMessagePayload(
                  clientInfo.aes_key_handle, clientInfo.hmac_key_handle, counter, ByteSpan(), output),
              CHIP_NO_ERROR);

    buffer->SetDataLength(static_cast<uint16_t>(output.size()));
    ByteSpan payload1{ buffer->Start(), buffer->DataLength() };

    // Use a removed key in the storage for encoding
    manager.RemoveKey(clientInfo);
    EXPECT_EQ(manager.DeleteEntry(clientInfo.peer_node), CHIP_NO_ERROR);

    EXPECT_EQ(manager.ProcessCheckInPayload(payload1, decodeClientInfo, checkInCounter), CHIP_ERROR_NOT_FOUND);
}

TEST_F(TestDefaultICDClientStorage, TestProcessCheckInPayloadWithEmptyIcdStorage)
{
    FabricIndex fabricId = 1;
    NodeId nodeId        = 6666;
    TestPersistentStorageDelegate clientInfoStorage;
    TestSessionKeystoreImpl keystore;

    DefaultICDClientStorage manager;
    EXPECT_EQ(manager.Init(&clientInfoStorage, &keystore), CHIP_NO_ERROR);
    EXPECT_EQ(manager.UpdateFabricList(fabricId), CHIP_NO_ERROR);
    // Populate clientInfo
    ICDClientInfo clientInfo;
    clientInfo.peer_node = ScopedNodeId(nodeId, fabricId);

    EXPECT_EQ(manager.SetKey(clientInfo, ByteSpan(kKeyBuffer1)), CHIP_NO_ERROR);
    EXPECT_EQ(manager.StoreEntry(clientInfo), CHIP_NO_ERROR);

    uint32_t counter                  = 1;
    System::PacketBufferHandle buffer = MessagePacketBuffer::New(chip::Protocols::SecureChannel::CheckinMessage::kMinPayloadSize);
    MutableByteSpan output{ buffer->Start(), buffer->MaxDataLength() };
    EXPECT_EQ(chip::Protocols::SecureChannel::CheckinMessage::GenerateCheckinMessagePayload(
                  clientInfo.aes_key_handle, clientInfo.hmac_key_handle, counter, ByteSpan(), output),
              CHIP_NO_ERROR);

    buffer->SetDataLength(static_cast<uint16_t>(output.size()));
    ICDClientInfo decodeClientInfo;
    uint32_t checkInCounter = 0;
    ByteSpan payload{ buffer->Start(), buffer->DataLength() };
    EXPECT_EQ(manager.ProcessCheckInPayload(payload, decodeClientInfo, checkInCounter), CHIP_NO_ERROR);
    EXPECT_EQ(checkInCounter, counter);
    // Generate payload before removing the key
    EXPECT_EQ(chip::Protocols::SecureChannel::CheckinMessage::GenerateCheckinMessagePayload(
                  clientInfo.aes_key_handle, clientInfo.hmac_key_handle, counter, ByteSpan(), output),
              CHIP_NO_ERROR);

    buffer->SetDataLength(static_cast<uint16_t>(output.size()));
    ByteSpan payload1{ buffer->Start(), buffer->DataLength() };

    EXPECT_SUCCESS(manager.DeleteAllEntries(fabricId));

    EXPECT_EQ(manager.ProcessCheckInPayload(payload1, decodeClientInfo, checkInCounter), CHIP_ERROR_NOT_FOUND);
}

TEST_F(TestDefaultICDClientStorage, TestPersistentKeysReload)
{
    FabricIndex fabricId = 1;
    NodeId nodeId        = 6666;
    TestPersistentStorageDelegate clientInfoStorage;
    TestSessionKeystoreImpl keystore;

    {
        DefaultICDClientStorage manager;
        EXPECT_EQ(manager.Init(&clientInfoStorage, &keystore), CHIP_NO_ERROR);
        EXPECT_EQ(manager.UpdateFabricList(fabricId), CHIP_NO_ERROR);

        ICDClientInfo clientInfo;
        clientInfo.peer_node = ScopedNodeId(nodeId, fabricId);

        EXPECT_EQ(manager.SetKey(clientInfo, ByteSpan(kKeyBuffer1)), CHIP_NO_ERROR);
        EXPECT_EQ(manager.StoreEntry(clientInfo), CHIP_NO_ERROR);

#if CHIP_CRYPTO_KEYSTORE_PSA
        psa_key_attributes_t attr = PSA_KEY_ATTRIBUTES_INIT;
        EXPECT_EQ(psa_get_key_attributes(clientInfo.aes_key_handle.As<psa_key_id_t>(), &attr), PSA_SUCCESS);
        EXPECT_FALSE(PSA_KEY_LIFETIME_IS_VOLATILE(psa_get_key_lifetime(&attr)));
        EXPECT_GE(clientInfo.aes_key_handle.As<psa_key_id_t>(), chip::to_underlying(chip::Crypto::KeyIdBase::ICDKeyRangeStart));
        EXPECT_LT(clientInfo.aes_key_handle.As<psa_key_id_t>(),
                  chip::to_underlying(chip::Crypto::KeyIdBase::ICDKeyRangeStart) + chip::Crypto::kMaxICDClientKeys);
        psa_reset_key_attributes(&attr);

        EXPECT_EQ(psa_get_key_attributes(clientInfo.hmac_key_handle.As<psa_key_id_t>(), &attr), PSA_SUCCESS);
        EXPECT_FALSE(PSA_KEY_LIFETIME_IS_VOLATILE(psa_get_key_lifetime(&attr)));
        EXPECT_GE(clientInfo.hmac_key_handle.As<psa_key_id_t>(), chip::to_underlying(chip::Crypto::KeyIdBase::ICDKeyRangeStart));
        EXPECT_LT(clientInfo.hmac_key_handle.As<psa_key_id_t>(),
                  chip::to_underlying(chip::Crypto::KeyIdBase::ICDKeyRangeStart) + chip::Crypto::kMaxICDClientKeys);
        psa_reset_key_attributes(&attr);
#endif

        uint32_t counter = 1;
        System::PacketBufferHandle buffer =
            MessagePacketBuffer::New(chip::Protocols::SecureChannel::CheckinMessage::kMinPayloadSize);
        MutableByteSpan output{ buffer->Start(), buffer->MaxDataLength() };
        EXPECT_EQ(chip::Protocols::SecureChannel::CheckinMessage::GenerateCheckinMessagePayload(
                      clientInfo.aes_key_handle, clientInfo.hmac_key_handle, counter, ByteSpan(), output),
                  CHIP_NO_ERROR);
        buffer->SetDataLength(static_cast<uint16_t>(output.size()));

        DefaultICDClientStorage manager2;
        EXPECT_EQ(manager2.Init(&clientInfoStorage, &keystore), CHIP_NO_ERROR);
        EXPECT_EQ(manager2.UpdateFabricList(fabricId), CHIP_NO_ERROR);

        ICDClientInfo decodeClientInfo;
        uint32_t checkInCounter = 0;
        ByteSpan payload{ buffer->Start(), buffer->DataLength() };
        EXPECT_EQ(manager2.ProcessCheckInPayload(payload, decodeClientInfo, checkInCounter), CHIP_NO_ERROR);

        EXPECT_SUCCESS(manager2.DeleteAllEntries(fabricId));

#if CHIP_CRYPTO_KEYSTORE_PSA
        EXPECT_EQ(psa_get_key_attributes(clientInfo.aes_key_handle.As<psa_key_id_t>(), &attr), PSA_ERROR_INVALID_HANDLE);
        EXPECT_EQ(psa_get_key_attributes(clientInfo.hmac_key_handle.As<psa_key_id_t>(), &attr), PSA_ERROR_INVALID_HANDLE);
#endif

        EXPECT_EQ(manager2.UpdateFabricList(fabricId), CHIP_NO_ERROR);

        // Re-registering the same peer_node with a new key destroys the old persistent key IDs
        ICDClientInfo newClientInfo;
        newClientInfo.peer_node = ScopedNodeId(nodeId, fabricId);
        EXPECT_EQ(manager2.SetKey(newClientInfo, ByteSpan(kKeyBuffer2)), CHIP_NO_ERROR);
        EXPECT_EQ(manager2.StoreEntry(newClientInfo), CHIP_NO_ERROR);

        ICDClientInfo newClientInfo2;
        newClientInfo2.peer_node = ScopedNodeId(nodeId, fabricId);
        EXPECT_EQ(manager2.SetKey(newClientInfo2, ByteSpan(kKeyBuffer1)), CHIP_NO_ERROR);
        EXPECT_EQ(manager2.StoreEntry(newClientInfo2), CHIP_NO_ERROR);

#if CHIP_CRYPTO_KEYSTORE_PSA
        EXPECT_EQ(psa_get_key_attributes(newClientInfo.aes_key_handle.As<psa_key_id_t>(), &attr), PSA_ERROR_INVALID_HANDLE);
        EXPECT_EQ(psa_get_key_attributes(newClientInfo.hmac_key_handle.As<psa_key_id_t>(), &attr), PSA_ERROR_INVALID_HANDLE);
#endif
        EXPECT_SUCCESS(manager2.DeleteAllEntries(fabricId));
    }
}

#if CHIP_CRYPTO_KEYSTORE_PSA
TEST_F(TestDefaultICDClientStorage, TestLegacyVolatileKeyInvalidatedOnLoad)
{
    FabricIndex fabricId = 1;
    NodeId nodeId        = 6666;
    TestPersistentStorageDelegate clientInfoStorage;
    TestSessionKeystoreImpl keystore;

    {
        DefaultICDClientStorage manager;
        EXPECT_EQ(manager.Init(&clientInfoStorage, &keystore), CHIP_NO_ERROR);
        EXPECT_EQ(manager.UpdateFabricList(fabricId), CHIP_NO_ERROR);

        // Encode a valid Check-In payload using temporary volatile keys, then destroy them to
        // simulate a restart that wiped the pre-reboot volatile key table.
        chip::Crypto::Aes128KeyHandle tempAesKey;
        chip::Crypto::Hmac128KeyHandle tempHmacKey;
        ASSERT_EQ(keystore.CreateKey(kKeyBuffer1, tempAesKey), CHIP_NO_ERROR);
        ASSERT_EQ(keystore.CreateKey(kKeyBuffer1, tempHmacKey), CHIP_NO_ERROR);

        uint32_t counter = 1;
        System::PacketBufferHandle buffer =
            MessagePacketBuffer::New(chip::Protocols::SecureChannel::CheckinMessage::kMinPayloadSize);
        MutableByteSpan output{ buffer->Start(), buffer->MaxDataLength() };
        EXPECT_EQ(chip::Protocols::SecureChannel::CheckinMessage::GenerateCheckinMessagePayload(tempAesKey, tempHmacKey, counter,
                                                                                                ByteSpan(), output),
                  CHIP_NO_ERROR);
        buffer->SetDataLength(static_cast<uint16_t>(output.size()));
        keystore.DestroyKey(tempAesKey);
        keystore.DestroyKey(tempHmacKey);

        // Create active volatile keys in the post-restart process first, then store a legacy
        // entry carrying those exact volatile IDs so the collision does not depend on PSA
        // volatile-ID reuse order.
        chip::Crypto::Aes128KeyHandle unrelatedAesKey;
        chip::Crypto::Hmac128KeyHandle unrelatedHmacKey;
        ASSERT_EQ(keystore.CreateKey(kKeyBuffer2, unrelatedAesKey), CHIP_NO_ERROR);
        ASSERT_EQ(keystore.CreateKey(kKeyBuffer2, unrelatedHmacKey), CHIP_NO_ERROR);

        ICDClientInfo clientInfo;
        clientInfo.peer_node                                 = ScopedNodeId(nodeId, fabricId);
        clientInfo.aes_key_handle.AsMutable<psa_key_id_t>()  = unrelatedAesKey.As<psa_key_id_t>();
        clientInfo.hmac_key_handle.AsMutable<psa_key_id_t>() = unrelatedHmacKey.As<psa_key_id_t>();
        EXPECT_EQ(manager.StoreEntry(clientInfo), CHIP_NO_ERROR);

        // Reload into a fresh manager and verify Load() cleared both volatile handles to null
        DefaultICDClientStorage manager2;
        EXPECT_EQ(manager2.Init(&clientInfoStorage, &keystore), CHIP_NO_ERROR);
        EXPECT_EQ(manager2.UpdateFabricList(fabricId), CHIP_NO_ERROR);

        AutoRelease iterator(manager2.IterateICDClientInfo());
        ASSERT_FALSE(iterator.IsNull());
        ICDClientInfo loadedInfo;
        ASSERT_TRUE(iterator->Next(loadedInfo));
        EXPECT_EQ(loadedInfo.aes_key_handle.As<psa_key_id_t>(), static_cast<psa_key_id_t>(PSA_KEY_ID_NULL));
        EXPECT_EQ(loadedInfo.hmac_key_handle.As<psa_key_id_t>(), static_cast<psa_key_id_t>(PSA_KEY_ID_NULL));
        iterator.Release();

        ICDClientInfo decodeClientInfo;
        uint32_t checkInCounter = 0;
        ByteSpan payload{ buffer->Start(), buffer->DataLength() };
        // ProcessCheckInPayload should fail because the keys are treated as invalid
        EXPECT_NE(manager2.ProcessCheckInPayload(payload, decodeClientInfo, checkInCounter), CHIP_NO_ERROR);

        // Deleting the entry must NOT destroy the unrelated volatile keys
        EXPECT_EQ(manager2.DeleteEntry(ScopedNodeId(nodeId, fabricId)), CHIP_NO_ERROR);

        psa_key_attributes_t attr = PSA_KEY_ATTRIBUTES_INIT;
        EXPECT_EQ(psa_get_key_attributes(unrelatedAesKey.As<psa_key_id_t>(), &attr), PSA_SUCCESS);
        psa_reset_key_attributes(&attr);
        EXPECT_EQ(psa_get_key_attributes(unrelatedHmacKey.As<psa_key_id_t>(), &attr), PSA_SUCCESS);
        psa_reset_key_attributes(&attr);

        keystore.DestroyKey(unrelatedAesKey);
        keystore.DestroyKey(unrelatedHmacKey);
    }
}
#endif

TEST_F(TestDefaultICDClientStorage, TestStorageWriteFailurePreservesKeys)
{
    FabricIndex fabricId = 1;
    NodeId nodeId        = 6666;
    TestPersistentStorageDelegate clientInfoStorage;
    TestSessionKeystoreImpl keystore;

    DefaultICDClientStorage manager;
    ASSERT_EQ(manager.Init(&clientInfoStorage, &keystore), CHIP_NO_ERROR);
    ASSERT_EQ(manager.UpdateFabricList(fabricId), CHIP_NO_ERROR);

    const std::string counterKey(DefaultStorageKeyAllocator::FabricICDClientInfoCounter(fabricId).KeyName());
    const std::string blobKey(DefaultStorageKeyAllocator::ICDClientInfoKey(fabricId).KeyName());

    // 1. New-entry StoreEntry failure on counter write or blob write rolls back cleanly.
    ICDClientInfo stagedNew;
    stagedNew.peer_node = ScopedNodeId(nodeId, fabricId);
    ASSERT_EQ(manager.SetKey(stagedNew, ByteSpan(kKeyBuffer1)), CHIP_NO_ERROR);

    clientInfoStorage.AddPoisonKey(counterKey, /*readPattern=*/-1, /*writePattern=*/0);
    EXPECT_EQ(manager.StoreEntry(stagedNew), CHIP_ERROR_PERSISTED_STORAGE_FAILED);
    clientInfoStorage.ClearPoisonKeys();

    {
        AutoRelease iterator(manager.IterateICDClientInfo());
        ASSERT_FALSE(iterator.IsNull());
        EXPECT_EQ(iterator->Count(), 0u);
    }

    clientInfoStorage.AddPoisonKey(blobKey, /*readPattern=*/-1, /*writePattern=*/0);
    EXPECT_EQ(manager.StoreEntry(stagedNew), CHIP_ERROR_PERSISTED_STORAGE_FAILED);
    clientInfoStorage.ClearPoisonKeys();

    {
        AutoRelease iterator(manager.IterateICDClientInfo());
        ASSERT_FALSE(iterator.IsNull());
        EXPECT_EQ(iterator->Count(), 0u);
    }
    manager.RemoveKey(stagedNew);

    // 2. Persist an initial entry and capture a Check-In payload for it.
    ICDClientInfo initialEntry;
    initialEntry.peer_node = ScopedNodeId(nodeId, fabricId);
    ASSERT_EQ(manager.SetKey(initialEntry, ByteSpan(kKeyBuffer1)), CHIP_NO_ERROR);
    ASSERT_EQ(manager.StoreEntry(initialEntry), CHIP_NO_ERROR);

    uint32_t counter = 1;
    System::PacketBufferHandle initialBuf =
        MessagePacketBuffer::New(chip::Protocols::SecureChannel::CheckinMessage::kMinPayloadSize);
    MutableByteSpan initialOut{ initialBuf->Start(), initialBuf->MaxDataLength() };
    ASSERT_EQ(chip::Protocols::SecureChannel::CheckinMessage::GenerateCheckinMessagePayload(
                  initialEntry.aes_key_handle, initialEntry.hmac_key_handle, counter, ByteSpan(), initialOut),
              CHIP_NO_ERROR);
    initialBuf->SetDataLength(static_cast<uint16_t>(initialOut.size()));
    ByteSpan payloadInitial{ initialBuf->Start(), initialBuf->DataLength() };

    // 3. Replacing an existing entry does not write the fabric counter, so poisoning the counter
    //    cannot cause a partial-commit error after the replacement blob write.
    ICDClientInfo replacement1;
    replacement1.peer_node = ScopedNodeId(nodeId, fabricId);
    ASSERT_EQ(manager.SetKey(replacement1, ByteSpan(kKeyBuffer2)), CHIP_NO_ERROR);

    System::PacketBufferHandle repl1Buf = MessagePacketBuffer::New(chip::Protocols::SecureChannel::CheckinMessage::kMinPayloadSize);
    MutableByteSpan repl1Out{ repl1Buf->Start(), repl1Buf->MaxDataLength() };
    ASSERT_EQ(chip::Protocols::SecureChannel::CheckinMessage::GenerateCheckinMessagePayload(
                  replacement1.aes_key_handle, replacement1.hmac_key_handle, counter, ByteSpan(), repl1Out),
              CHIP_NO_ERROR);
    repl1Buf->SetDataLength(static_cast<uint16_t>(repl1Out.size()));
    ByteSpan payloadRepl1{ repl1Buf->Start(), repl1Buf->DataLength() };

    clientInfoStorage.AddPoisonKey(counterKey, /*readPattern=*/-1, /*writePattern=*/0);
    EXPECT_EQ(manager.StoreEntry(replacement1), CHIP_NO_ERROR);
    clientInfoStorage.ClearPoisonKeys();

#if CHIP_CRYPTO_KEYSTORE_PSA
    psa_key_attributes_t attr = PSA_KEY_ATTRIBUTES_INIT;
    EXPECT_EQ(psa_get_key_attributes(initialEntry.aes_key_handle.As<psa_key_id_t>(), &attr), PSA_ERROR_INVALID_HANDLE);
    EXPECT_EQ(psa_get_key_attributes(initialEntry.hmac_key_handle.As<psa_key_id_t>(), &attr), PSA_ERROR_INVALID_HANDLE);
#endif

    ICDClientInfo decodeClientInfo;
    uint32_t checkInCounter = 0;
    EXPECT_EQ(manager.ProcessCheckInPayload(payloadInitial, decodeClientInfo, checkInCounter), CHIP_ERROR_NOT_FOUND);
    EXPECT_EQ(manager.ProcessCheckInPayload(payloadRepl1, decodeClientInfo, checkInCounter), CHIP_NO_ERROR);

    // 4. Failed replacement blob write leaves replacement1 intact while allowing caller cleanup of staged keys.
    ICDClientInfo replacement2;
    replacement2.peer_node = ScopedNodeId(nodeId, fabricId);
    ASSERT_EQ(manager.SetKey(replacement2, ByteSpan(kKeyBuffer3)), CHIP_NO_ERROR);

    clientInfoStorage.AddPoisonKey(blobKey, /*readPattern=*/-1, /*writePattern=*/0);
    EXPECT_EQ(manager.StoreEntry(replacement2), CHIP_ERROR_PERSISTED_STORAGE_FAILED);
    manager.RemoveKey(replacement2);

    // 5. Failed DeleteEntry and DeleteAllEntries do not retire replacement1's keys while the KVS record remains.
    EXPECT_EQ(manager.DeleteEntry(ScopedNodeId(nodeId, fabricId)), CHIP_ERROR_PERSISTED_STORAGE_FAILED);
    EXPECT_EQ(manager.DeleteAllEntries(fabricId), CHIP_ERROR_PERSISTED_STORAGE_FAILED);
    clientInfoStorage.ClearPoisonKeys();

#if CHIP_CRYPTO_KEYSTORE_PSA
    EXPECT_EQ(psa_get_key_attributes(replacement2.aes_key_handle.As<psa_key_id_t>(), &attr), PSA_ERROR_INVALID_HANDLE);
    EXPECT_EQ(psa_get_key_attributes(replacement2.hmac_key_handle.As<psa_key_id_t>(), &attr), PSA_ERROR_INVALID_HANDLE);
    EXPECT_EQ(psa_get_key_attributes(replacement1.aes_key_handle.As<psa_key_id_t>(), &attr), PSA_SUCCESS);
    psa_reset_key_attributes(&attr);
    EXPECT_EQ(psa_get_key_attributes(replacement1.hmac_key_handle.As<psa_key_id_t>(), &attr), PSA_SUCCESS);
    psa_reset_key_attributes(&attr);
#endif

    DefaultICDClientStorage manager2;
    ASSERT_EQ(manager2.Init(&clientInfoStorage, &keystore), CHIP_NO_ERROR);
    ASSERT_EQ(manager2.UpdateFabricList(fabricId), CHIP_NO_ERROR);
    EXPECT_EQ(manager2.ProcessCheckInPayload(payloadRepl1, decodeClientInfo, checkInCounter), CHIP_NO_ERROR);
    EXPECT_EQ(manager2.DeleteAllEntries(fabricId), CHIP_NO_ERROR);

#if CHIP_CRYPTO_KEYSTORE_PSA
    EXPECT_EQ(psa_get_key_attributes(replacement1.aes_key_handle.As<psa_key_id_t>(), &attr), PSA_ERROR_INVALID_HANDLE);
    EXPECT_EQ(psa_get_key_attributes(replacement1.hmac_key_handle.As<psa_key_id_t>(), &attr), PSA_ERROR_INVALID_HANDLE);
#endif
}

#if CHIP_CRYPTO_KEYSTORE_PSA && defined(MBEDTLS_PSA_CRYPTO_C)
TEST_F(TestDefaultICDClientStorage, TestLoadPropagatesOperationalPsaError)
{
    FabricIndex fabricId = 1;
    NodeId nodeB         = 6666;
    NodeId nodeA         = 6667;
    TestPersistentStorageDelegate clientInfoStorage;
    TestSessionKeystoreImpl keystore;

    DefaultICDClientStorage manager;
    ASSERT_EQ(manager.Init(&clientInfoStorage, &keystore), CHIP_NO_ERROR);
    ASSERT_EQ(manager.UpdateFabricList(fabricId), CHIP_NO_ERROR);

    ICDClientInfo clientB;
    clientB.peer_node = ScopedNodeId(nodeB, fabricId);
    ASSERT_EQ(manager.SetKey(clientB, ByteSpan(kKeyBuffer1)), CHIP_NO_ERROR);
    ASSERT_EQ(manager.StoreEntry(clientB), CHIP_NO_ERROR);

    uint32_t counter                  = 1;
    System::PacketBufferHandle buffer = MessagePacketBuffer::New(chip::Protocols::SecureChannel::CheckinMessage::kMinPayloadSize);
    MutableByteSpan output{ buffer->Start(), buffer->MaxDataLength() };
    ASSERT_EQ(chip::Protocols::SecureChannel::CheckinMessage::GenerateCheckinMessagePayload(
                  clientB.aes_key_handle, clientB.hmac_key_handle, counter, ByteSpan(), output),
              CHIP_NO_ERROR);
    buffer->SetDataLength(static_cast<uint16_t>(output.size()));
    ByteSpan payloadB{ buffer->Start(), buffer->DataLength() };

    ICDClientInfo clientA;
    clientA.peer_node = ScopedNodeId(nodeA, fabricId);
    ASSERT_EQ(manager.SetKey(clientA, ByteSpan(kKeyBuffer2)), CHIP_NO_ERROR);

    // Deinitializing PSA crypto forces psa_get_key_attributes() to return PSA_ERROR_BAD_STATE
    // (an operational failure rather than INVALID_HANDLE / DOES_NOT_EXIST).
    mbedtls_psa_crypto_free();

    EXPECT_EQ(manager.StoreEntry(clientA), CHIP_ERROR_INTERNAL);
    EXPECT_EQ(manager.DeleteEntry(clientB.peer_node), CHIP_ERROR_INTERNAL);
    EXPECT_EQ(manager.DeleteAllEntries(fabricId), CHIP_ERROR_INTERNAL);

    ASSERT_EQ(psa_crypto_init(), PSA_SUCCESS);
    manager.RemoveKey(clientA);

    DefaultICDClientStorage manager2;
    ASSERT_EQ(manager2.Init(&clientInfoStorage, &keystore), CHIP_NO_ERROR);
    ASSERT_EQ(manager2.UpdateFabricList(fabricId), CHIP_NO_ERROR);

    ICDClientInfo decodeClientInfo;
    uint32_t checkInCounter = 0;
    EXPECT_EQ(manager2.ProcessCheckInPayload(payloadB, decodeClientInfo, checkInCounter), CHIP_NO_ERROR);
    EXPECT_EQ(checkInCounter, counter);
    EXPECT_EQ(manager2.DeleteAllEntries(fabricId), CHIP_NO_ERROR);
}
#endif
