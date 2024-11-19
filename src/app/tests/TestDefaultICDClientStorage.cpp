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
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <protocols/secure_channel/CheckinMessage.h>
#include <transport/SessionManager.h>

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
    auto * iterator = manager.IterateICDClientInfo();
    EXPECT_EQ(iterator->Count(), 3u);
    iterator->Release();

    // Delete all and verify iterator counts 0
    EXPECT_EQ(manager.DeleteEntry(ScopedNodeId(nodeId1, fabricId1)), CHIP_NO_ERROR);
    iterator = manager.IterateICDClientInfo();
    ASSERT_NE(iterator, nullptr);
    DefaultICDClientStorage::ICDClientInfoIteratorWrapper clientInfoIteratorWrapper(iterator);
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
    auto * iterator = manager.IterateICDClientInfo();
    EXPECT_EQ(iterator->Count(), 3u);
    iterator->Release();

    EXPECT_EQ(manager.DeleteAllEntries(fabricId1), CHIP_NO_ERROR);

    iterator = manager.IterateICDClientInfo();
    ASSERT_NE(iterator, nullptr);
    DefaultICDClientStorage::ICDClientInfoIteratorWrapper clientInfoIteratorWrapper(iterator);
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
    EXPECT_EQ(manager.SetKey(clientInfo, ByteSpan(kKeyBuffer2)), CHIP_NO_ERROR);
    EXPECT_EQ(chip::Protocols::SecureChannel::CheckinMessage::GenerateCheckinMessagePayload(
                  clientInfo.aes_key_handle, clientInfo.hmac_key_handle, counter, ByteSpan(), output),
              CHIP_NO_ERROR);

    buffer->SetDataLength(static_cast<uint16_t>(output.size()));
    ByteSpan payload2{ buffer->Start(), buffer->DataLength() };
    EXPECT_EQ(manager.ProcessCheckInPayload(payload2, decodeClientInfo, checkInCounter), CHIP_ERROR_NOT_FOUND);
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

    // Use a removed key in the storage for encoding
    manager.RemoveKey(clientInfo), CHIP_NO_ERROR;
    EXPECT_EQ(chip::Protocols::SecureChannel::CheckinMessage::GenerateCheckinMessagePayload(
                  clientInfo.aes_key_handle, clientInfo.hmac_key_handle, counter, ByteSpan(), output),
              CHIP_NO_ERROR);

    buffer->SetDataLength(static_cast<uint16_t>(output.size()));
    ByteSpan payload1{ buffer->Start(), buffer->DataLength() };
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
    manager.DeleteAllEntries(fabricId);

    EXPECT_EQ(chip::Protocols::SecureChannel::CheckinMessage::GenerateCheckinMessagePayload(
                  clientInfo.aes_key_handle, clientInfo.hmac_key_handle, counter, ByteSpan(), output),
              CHIP_NO_ERROR);

    buffer->SetDataLength(static_cast<uint16_t>(output.size()));
    ByteSpan payload1{ buffer->Start(), buffer->DataLength() };
    EXPECT_EQ(manager.ProcessCheckInPayload(payload1, decodeClientInfo, checkInCounter), CHIP_ERROR_NOT_FOUND);
}
