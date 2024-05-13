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

#include <lib/support/Span.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>
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

void TestClientInfoCount(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err       = CHIP_NO_ERROR;
    FabricIndex fabricId = 1;
    NodeId nodeId1       = 6666;
    NodeId nodeId2       = 6667;
    TestPersistentStorageDelegate clientInfoStorage;
    TestSessionKeystoreImpl keystore;

    {
        DefaultICDClientStorage manager;
        err = manager.Init(&clientInfoStorage, &keystore);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        err = manager.UpdateFabricList(fabricId);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        // Write some ClientInfos and see the counts are correct
        ICDClientInfo clientInfo1;
        clientInfo1.peer_node = ScopedNodeId(nodeId1, fabricId);
        ICDClientInfo clientInfo2;
        clientInfo2.peer_node = ScopedNodeId(nodeId2, fabricId);
        ICDClientInfo clientInfo3;
        clientInfo3.peer_node = ScopedNodeId(nodeId1, fabricId);
        err                   = manager.SetKey(clientInfo1, ByteSpan(kKeyBuffer1));
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        err = manager.StoreEntry(clientInfo1);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = manager.SetKey(clientInfo2, ByteSpan(kKeyBuffer2));
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        err = manager.StoreEntry(clientInfo2);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = manager.SetKey(clientInfo3, ByteSpan(kKeyBuffer3));
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        err = manager.StoreEntry(clientInfo3);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ICDClientInfo clientInfo;
        // Make sure iterator counts correctly
        auto * iterator = manager.IterateICDClientInfo();
        // same nodeId for clientInfo2 and clientInfo3, so the new one replace old one
        NL_TEST_ASSERT(apSuite, iterator->Count() == 2);

        NL_TEST_ASSERT(apSuite, iterator->Next(clientInfo));
        NL_TEST_ASSERT(apSuite, clientInfo.peer_node.GetNodeId() == nodeId2);
        NL_TEST_ASSERT(apSuite, iterator->Next(clientInfo));
        NL_TEST_ASSERT(apSuite, clientInfo.peer_node.GetNodeId() == nodeId1);

        iterator->Release();

        // Delete all and verify iterator counts 0
        err = manager.DeleteAllEntries(fabricId);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        iterator = manager.IterateICDClientInfo();
        NL_TEST_ASSERT(apSuite, iterator->Count() == 0);

        // Verify ClientInfos manually count correctly
        size_t count = 0;
        while (iterator->Next(clientInfo))
        {
            count++;
        }
        iterator->Release();
        NL_TEST_ASSERT(apSuite, count == 0);
    }

    {
        DefaultICDClientStorage manager;
        err = manager.Init(&clientInfoStorage, &keystore);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        err = manager.UpdateFabricList(fabricId);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    }
}

void TestClientInfoCountMultipleFabric(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err        = CHIP_NO_ERROR;
    FabricIndex fabricId1 = 1;
    FabricIndex fabricId2 = 2;
    NodeId nodeId1        = 6666;
    NodeId nodeId2        = 6667;
    NodeId nodeId3        = 6668;
    DefaultICDClientStorage manager;
    TestPersistentStorageDelegate clientInfoStorage;
    TestSessionKeystoreImpl keystore;
    err = manager.Init(&clientInfoStorage, &keystore);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    err = manager.UpdateFabricList(fabricId1);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    err = manager.UpdateFabricList(fabricId2);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    // Write some ClientInfos and see the counts are correct
    ICDClientInfo clientInfo1;
    clientInfo1.peer_node = ScopedNodeId(nodeId1, fabricId1);
    ICDClientInfo clientInfo2;
    clientInfo2.peer_node = ScopedNodeId(nodeId2, fabricId1);
    ICDClientInfo clientInfo3;
    clientInfo3.peer_node = ScopedNodeId(nodeId3, fabricId2);

    err = manager.SetKey(clientInfo1, ByteSpan(kKeyBuffer1));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    err = manager.StoreEntry(clientInfo1);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = manager.SetKey(clientInfo2, ByteSpan(kKeyBuffer2));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    err = manager.StoreEntry(clientInfo2);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = manager.SetKey(clientInfo3, ByteSpan(kKeyBuffer3));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    err = manager.StoreEntry(clientInfo3);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    // Make sure iterator counts correctly
    auto * iterator = manager.IterateICDClientInfo();
    NL_TEST_ASSERT(apSuite, iterator->Count() == 3);
    iterator->Release();

    // Delete all and verify iterator counts 0
    err = manager.DeleteEntry(ScopedNodeId(nodeId1, fabricId1));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    iterator = manager.IterateICDClientInfo();
    NL_TEST_ASSERT(apSuite, iterator != nullptr);
    DefaultICDClientStorage::ICDClientInfoIteratorWrapper clientInfoIteratorWrapper(iterator);
    NL_TEST_ASSERT(apSuite, iterator->Count() == 2);

    err = manager.DeleteEntry(ScopedNodeId(nodeId2, fabricId1));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, iterator->Count() == 1);

    err = manager.DeleteEntry(ScopedNodeId(nodeId3, fabricId2));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, iterator->Count() == 0);

    // Verify ClientInfos manually count correctly
    size_t count = 0;
    ICDClientInfo clientInfo;
    while (iterator->Next(clientInfo))
    {
        count++;
    }

    NL_TEST_ASSERT(apSuite, count == 0);
}

void TestProcessCheckInPayload(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err       = CHIP_NO_ERROR;
    FabricIndex fabricId = 1;
    NodeId nodeId        = 6666;
    TestPersistentStorageDelegate clientInfoStorage;
    TestSessionKeystoreImpl keystore;

    DefaultICDClientStorage manager;
    err = manager.Init(&clientInfoStorage, &keystore);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    err = manager.UpdateFabricList(fabricId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    // Populate clientInfo
    ICDClientInfo clientInfo;
    clientInfo.peer_node = ScopedNodeId(nodeId, fabricId);

    err = manager.SetKey(clientInfo, ByteSpan(kKeyBuffer1));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    err = manager.StoreEntry(clientInfo);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    uint32_t counter                  = 1;
    System::PacketBufferHandle buffer = MessagePacketBuffer::New(chip::Protocols::SecureChannel::CheckinMessage::kMinPayloadSize);
    MutableByteSpan output{ buffer->Start(), buffer->MaxDataLength() };
    err = chip::Protocols::SecureChannel::CheckinMessage::GenerateCheckinMessagePayload(
        clientInfo.aes_key_handle, clientInfo.hmac_key_handle, counter, ByteSpan(), output);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    buffer->SetDataLength(static_cast<uint16_t>(output.size()));
    ICDClientInfo decodeClientInfo;
    uint32_t checkInCounter = 0;
    ByteSpan payload{ buffer->Start(), buffer->DataLength() };
    err = manager.ProcessCheckInPayload(payload, decodeClientInfo, checkInCounter);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    // 2. Use a key not available in the storage for encoding
    err = manager.SetKey(clientInfo, ByteSpan(kKeyBuffer2));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    err = chip::Protocols::SecureChannel::CheckinMessage::GenerateCheckinMessagePayload(
        clientInfo.aes_key_handle, clientInfo.hmac_key_handle, counter, ByteSpan(), output);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    buffer->SetDataLength(static_cast<uint16_t>(output.size()));
    ByteSpan payload1{ buffer->Start(), buffer->DataLength() };
    err = manager.ProcessCheckInPayload(payload1, decodeClientInfo, checkInCounter);
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_NOT_FOUND);
}

/**
 *  Set up the test suite.
 */
int TestClientInfo_Setup(void * apContext)
{
    VerifyOrReturnError(CHIP_NO_ERROR == Platform::MemoryInit(), FAILURE);

    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
int TestClientInfo_Teardown(void * apContext)
{
    Platform::MemoryShutdown();
    return SUCCESS;
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("TestClientInfoCount", TestClientInfoCount),
    NL_TEST_DEF("TestClientInfoCountMultipleFabric", TestClientInfoCountMultipleFabric),
    NL_TEST_DEF("TestProcessCheckInPayload", TestProcessCheckInPayload),

    NL_TEST_SENTINEL()
};
// clang-format on

// clang-format off
static nlTestSuite sSuite =
{
    "TestDefaultICDClientStorage",
    &sTests[0],
    &TestClientInfo_Setup, &TestClientInfo_Teardown
};
// clang-format on

/**
 *  Main
 */
int TestDefaultICDClientStorage()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, nullptr);

    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestDefaultICDClientStorage)
