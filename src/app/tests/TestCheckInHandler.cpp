/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/InteractionModelEngine.h>
#include <app/icd/client/CheckInHandler.h>
#include <app/icd/client/DefaultCheckInDelegate.h>
#include <app/icd/client/DefaultICDClientStorage.h>
#include <app/reporting/tests/MockReportScheduler.h>
#include <app/tests/AppTestContext.h>
#include <crypto/DefaultSessionKeystore.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/Span.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <protocols/secure_channel/CheckinMessage.h>
#include <pw_unit_test/framework.h>
#include <system/SystemPacketBuffer.h>
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

class TestCheckInHandler : public chip::Test::AppContext
{
};

class CheckInHandlerWrapper : public chip::app::CheckInHandler
{
public:
    CHIP_ERROR ValidateOnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                         System::PacketBufferHandle && payload)
    {
        return OnMessageReceived(ec, payloadHeader, std::move(payload));
    }
};

TEST_F(TestCheckInHandler, TestOnMessageReceived)
{
    InteractionModelEngine * engine = InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), app::reporting::GetDefaultReportScheduler()), CHIP_NO_ERROR);
    TestPersistentStorageDelegate clientInfoStorage;
    TestSessionKeystoreImpl keystore;

    DefaultICDClientStorage manager;
    EXPECT_EQ(manager.Init(&clientInfoStorage, &keystore), CHIP_NO_ERROR);

    DefaultCheckInDelegate checkInDelegate;
    EXPECT_EQ(checkInDelegate.Init(&manager, engine), CHIP_NO_ERROR);

    CheckInHandlerWrapper checkInHandler;
    EXPECT_EQ(checkInHandler.Init(&GetExchangeManager(), &manager, &checkInDelegate, engine), CHIP_NO_ERROR);

    FabricIndex fabricIdA = 1;
    NodeId nodeIdA        = 6666;

    ICDClientInfo clientInfoA;
    clientInfoA.peer_node         = ScopedNodeId(nodeIdA, fabricIdA);
    clientInfoA.start_icd_counter = 0;
    clientInfoA.offset            = 0;
    EXPECT_EQ(manager.UpdateFabricList(fabricIdA), CHIP_NO_ERROR);
    EXPECT_EQ(manager.SetKey(clientInfoA, ByteSpan(kKeyBuffer1)), CHIP_NO_ERROR);
    EXPECT_EQ(manager.StoreEntry(clientInfoA), CHIP_NO_ERROR);

    FabricIndex fabricIdB = 2;
    NodeId nodeIdB        = 6667;

    ICDClientInfo clientInfoB;
    clientInfoB.peer_node = ScopedNodeId(nodeIdB, fabricIdB);
    clientInfoB.offset    = 0;
    EXPECT_EQ(manager.UpdateFabricList(fabricIdB), CHIP_NO_ERROR);
    EXPECT_EQ(manager.SetKey(clientInfoB, ByteSpan(kKeyBuffer2)), CHIP_NO_ERROR);
    EXPECT_EQ(manager.StoreEntry(clientInfoB), CHIP_NO_ERROR);

    PayloadHeader payloadHeader;
    payloadHeader.SetExchangeID(0);
    payloadHeader.SetMessageType(chip::Protocols::SecureChannel::MsgType::ICD_CheckIn);

    uint32_t counter                   = 1;
    System::PacketBufferHandle buffer1 = MessagePacketBuffer::New(chip::Protocols::SecureChannel::CheckinMessage::kMinPayloadSize);
    MutableByteSpan output1{ buffer1->Start(), buffer1->MaxDataLength() };
    EXPECT_EQ(chip::Protocols::SecureChannel::CheckinMessage::GenerateCheckinMessagePayload(
                  clientInfoA.aes_key_handle, clientInfoA.hmac_key_handle, counter, ByteSpan(), output1),
              CHIP_NO_ERROR);

    buffer1->SetDataLength(static_cast<uint16_t>(output1.size()));
    EXPECT_EQ(checkInHandler.ValidateOnMessageReceived(nullptr, payloadHeader, std::move(buffer1)), CHIP_NO_ERROR);

    ICDClientInfo clientInfo1;
    auto * iterator = manager.IterateICDClientInfo();
    ASSERT_NE(iterator, nullptr);
    while (iterator->Next(clientInfo1))
    {
        if (clientInfo1.peer_node.GetNodeId() == nodeIdA && clientInfo1.peer_node.GetFabricIndex() == fabricIdA)
        {
            break;
        }
    }
    iterator->Release();

    EXPECT_EQ(clientInfo1.offset, counter - clientInfoA.start_icd_counter);

    // Validate duplicate check-in message handling
    chip::System::PacketBufferHandle buffer2 =
        MessagePacketBuffer::New(chip::Protocols::SecureChannel::CheckinMessage::kMinPayloadSize);
    MutableByteSpan output2{ buffer2->Start(), buffer2->MaxDataLength() };
    EXPECT_EQ(chip::Protocols::SecureChannel::CheckinMessage::GenerateCheckinMessagePayload(
                  clientInfoA.aes_key_handle, clientInfoA.hmac_key_handle, counter, ByteSpan(), output2),
              CHIP_NO_ERROR);

    buffer2->SetDataLength(static_cast<uint16_t>(output2.size()));
    EXPECT_EQ(checkInHandler.ValidateOnMessageReceived(nullptr, payloadHeader, std::move(buffer2)), CHIP_NO_ERROR);

    ICDClientInfo clientInfo2;
    iterator = manager.IterateICDClientInfo();
    ASSERT_NE(iterator, nullptr);
    while (iterator->Next(clientInfo2))
    {
        if (clientInfo2.peer_node.GetNodeId() == nodeIdA && clientInfo2.peer_node.GetFabricIndex() == fabricIdA)
        {
            break;
        }
    }
    iterator->Release();
    EXPECT_EQ(clientInfo2.offset, counter - clientInfoA.start_icd_counter);

    // Validate second check-in message with increased counter
    counter++;
    System::PacketBufferHandle buffer3 = MessagePacketBuffer::New(chip::Protocols::SecureChannel::CheckinMessage::kMinPayloadSize);
    MutableByteSpan output3{ buffer3->Start(), buffer3->MaxDataLength() };
    EXPECT_EQ(chip::Protocols::SecureChannel::CheckinMessage::GenerateCheckinMessagePayload(
                  clientInfoA.aes_key_handle, clientInfoA.hmac_key_handle, counter, ByteSpan(), output3),
              CHIP_NO_ERROR);
    buffer3->SetDataLength(static_cast<uint16_t>(output3.size()));
    EXPECT_EQ(checkInHandler.ValidateOnMessageReceived(nullptr, payloadHeader, std::move(buffer3)), CHIP_NO_ERROR);
    ICDClientInfo clientInfo3;
    iterator = manager.IterateICDClientInfo();
    ASSERT_NE(iterator, nullptr);
    while (iterator->Next(clientInfo3))
    {
        if (clientInfo3.peer_node.GetNodeId() == nodeIdA && clientInfo3.peer_node.GetFabricIndex() == fabricIdA)
        {
            break;
        }
    }
    iterator->Release();
    EXPECT_EQ(clientInfo3.offset, counter - clientInfoA.start_icd_counter);

    // Validate check-in message from fabricB
    counter++;
    System::PacketBufferHandle buffer4 = MessagePacketBuffer::New(chip::Protocols::SecureChannel::CheckinMessage::kMinPayloadSize);
    MutableByteSpan output4{ buffer4->Start(), buffer4->MaxDataLength() };
    EXPECT_EQ(chip::Protocols::SecureChannel::CheckinMessage::GenerateCheckinMessagePayload(
                  clientInfoB.aes_key_handle, clientInfoB.hmac_key_handle, counter, ByteSpan(), output4),
              CHIP_NO_ERROR);
    buffer4->SetDataLength(static_cast<uint16_t>(output4.size()));
    EXPECT_EQ(checkInHandler.ValidateOnMessageReceived(nullptr, payloadHeader, std::move(buffer4)), CHIP_NO_ERROR);
    ICDClientInfo clientInfo4;
    iterator = manager.IterateICDClientInfo();
    ASSERT_NE(iterator, nullptr);
    while (iterator->Next(clientInfo4))
    {
        if (clientInfo4.peer_node.GetNodeId() == nodeIdB && clientInfo4.peer_node.GetFabricIndex() == fabricIdB)
        {
            break;
        }
    }
    iterator->Release();
    EXPECT_EQ(clientInfo4.offset, counter - clientInfoB.start_icd_counter);

    // Validate check-in message from removed fabricB
    counter++;
    System::PacketBufferHandle buffer5 = MessagePacketBuffer::New(chip::Protocols::SecureChannel::CheckinMessage::kMinPayloadSize);
    MutableByteSpan output5{ buffer5->Start(), buffer5->MaxDataLength() };
    EXPECT_EQ(chip::Protocols::SecureChannel::CheckinMessage::GenerateCheckinMessagePayload(
                  clientInfoB.aes_key_handle, clientInfoB.hmac_key_handle, counter, ByteSpan(), output5),
              CHIP_NO_ERROR);

    EXPECT_EQ(manager.DeleteAllEntries(fabricIdB), CHIP_NO_ERROR);
    buffer5->SetDataLength(static_cast<uint16_t>(output5.size()));
    EXPECT_EQ(checkInHandler.ValidateOnMessageReceived(nullptr, payloadHeader, std::move(buffer5)), CHIP_NO_ERROR);
    ICDClientInfo clientInfo5;
    iterator = manager.IterateICDClientInfo();
    ASSERT_NE(iterator, nullptr);
    bool located = false;
    while (iterator->Next(clientInfo5))
    {
        if (clientInfo5.peer_node.GetFabricIndex() == fabricIdB)
        {
            located = true;
            break;
        }
    }
    iterator->Release();
    EXPECT_FALSE(located);

    // Add back fabricB and validate check-in message again
    EXPECT_EQ(manager.UpdateFabricList(fabricIdB), CHIP_NO_ERROR);
    EXPECT_EQ(manager.SetKey(clientInfoB, ByteSpan(kKeyBuffer2)), CHIP_NO_ERROR);
    EXPECT_EQ(manager.StoreEntry(clientInfoB), CHIP_NO_ERROR);
    counter++;
    System::PacketBufferHandle buffer6 = MessagePacketBuffer::New(chip::Protocols::SecureChannel::CheckinMessage::kMinPayloadSize);
    MutableByteSpan output6{ buffer6->Start(), buffer6->MaxDataLength() };
    EXPECT_EQ(chip::Protocols::SecureChannel::CheckinMessage::GenerateCheckinMessagePayload(
                  clientInfoB.aes_key_handle, clientInfoB.hmac_key_handle, counter, ByteSpan(), output6),
              CHIP_NO_ERROR);
    buffer6->SetDataLength(static_cast<uint16_t>(output4.size()));
    EXPECT_EQ(checkInHandler.ValidateOnMessageReceived(nullptr, payloadHeader, std::move(buffer6)), CHIP_NO_ERROR);
    ICDClientInfo clientInfo6;
    iterator = manager.IterateICDClientInfo();
    ASSERT_NE(iterator, nullptr);
    while (iterator->Next(clientInfo6))
    {
        if (clientInfo6.peer_node.GetNodeId() == nodeIdB && clientInfo6.peer_node.GetFabricIndex() == fabricIdB)
        {
            break;
        }
    }
    iterator->Release();
    EXPECT_EQ(clientInfo6.offset, counter - clientInfoB.start_icd_counter);

    // Clear fabric table
    EXPECT_EQ(manager.DeleteAllEntries(fabricIdA), CHIP_NO_ERROR);
    EXPECT_EQ(manager.DeleteAllEntries(fabricIdB), CHIP_NO_ERROR);
    // Add back fabricA and validate check-in message again
    EXPECT_EQ(manager.UpdateFabricList(fabricIdA), CHIP_NO_ERROR);
    EXPECT_EQ(manager.SetKey(clientInfoA, ByteSpan(kKeyBuffer1)), CHIP_NO_ERROR);
    EXPECT_EQ(manager.StoreEntry(clientInfoA), CHIP_NO_ERROR);
    counter++;
    System::PacketBufferHandle buffer7 = MessagePacketBuffer::New(chip::Protocols::SecureChannel::CheckinMessage::kMinPayloadSize);
    MutableByteSpan output7{ buffer7->Start(), buffer7->MaxDataLength() };
    EXPECT_EQ(chip::Protocols::SecureChannel::CheckinMessage::GenerateCheckinMessagePayload(
                  clientInfoA.aes_key_handle, clientInfoA.hmac_key_handle, counter, ByteSpan(), output7),
              CHIP_NO_ERROR);
    buffer7->SetDataLength(static_cast<uint16_t>(output7.size()));
    EXPECT_EQ(checkInHandler.ValidateOnMessageReceived(nullptr, payloadHeader, std::move(buffer7)), CHIP_NO_ERROR);
    ICDClientInfo clientInfo7;
    iterator = manager.IterateICDClientInfo();
    ASSERT_NE(iterator, nullptr);
    while (iterator->Next(clientInfo7))
    {
        if (clientInfo7.peer_node.GetNodeId() == nodeIdA && clientInfo7.peer_node.GetFabricIndex() == fabricIdA)
        {
            break;
        }
    }
    iterator->Release();
    EXPECT_EQ(clientInfo7.offset, counter - clientInfoA.start_icd_counter);

    // Validate IcdclientInfo is not updated when handling overlimited counter and fail to create case session
    uint32_t old_start_icd_counter     = clientInfo7.start_icd_counter;
    uint32_t old_counter               = counter;
    counter                            = (1U << 31) + 100U + clientInfo7.start_icd_counter;
    System::PacketBufferHandle buffer8 = MessagePacketBuffer::New(chip::Protocols::SecureChannel::CheckinMessage::kMinPayloadSize);
    MutableByteSpan output8{ buffer8->Start(), buffer8->MaxDataLength() };
    EXPECT_EQ(chip::Protocols::SecureChannel::CheckinMessage::GenerateCheckinMessagePayload(
                  clientInfoA.aes_key_handle, clientInfoA.hmac_key_handle, counter, ByteSpan(), output8),
              CHIP_NO_ERROR);

    buffer8->SetDataLength(static_cast<uint16_t>(output8.size()));
    EXPECT_EQ(checkInHandler.ValidateOnMessageReceived(nullptr, payloadHeader, std::move(buffer8)), CHIP_NO_ERROR);
    ICDClientInfo clientInfo8;
    iterator = manager.IterateICDClientInfo();
    ASSERT_NE(iterator, nullptr);
    while (iterator->Next(clientInfo8))
    {
        if (clientInfo8.peer_node.GetNodeId() == nodeIdA && clientInfo8.peer_node.GetFabricIndex() == fabricIdA)
        {
            break;
        }
    }
    iterator->Release();
    EXPECT_EQ(clientInfo8.offset, old_counter - clientInfoA.start_icd_counter);
    EXPECT_EQ(clientInfo8.start_icd_counter, old_start_icd_counter);

    checkInHandler.Shutdown();
}
