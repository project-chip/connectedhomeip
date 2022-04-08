/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

/**
 *    @file
 *      This file implements unit tests for the SessionManager implementation.
 */

#define CHIP_ENABLE_TEST_ENCRYPTED_BUFFER_API // Up here in case some other header
                                              // includes SessionManager.h indirectly

#include <credentials/tests/CHIPCert_test_vectors.h>
#include <lib/core/CHIPCore.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/UnitTestRegistration.h>
#include <protocols/Protocols.h>
#include <protocols/echo/Echo.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/PASESession.h>
#include <transport/SessionManager.h>
#include <transport/TransportMgr.h>
#include <transport/raw/tests/NetworkTestHelpers.h>

#include <nlbyteorder.h>
#include <nlunit-test.h>

#include <errno.h>

#undef CHIP_ENABLE_TEST_ENCRYPTED_BUFFER_API

namespace {

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::Test;

using TestContext = chip::Test::IOContext;

TestContext sContext;

const char PAYLOAD[] = "Hello!";

const char LARGE_PAYLOAD[kMaxAppMessageLen + 1] = "test message";

class TestSessionReleaseCallback : public SessionReleaseDelegate
{
public:
    void OnSessionReleased() override { mOldConnectionDropped = true; }
    bool mOldConnectionDropped = false;
};

class TestSessMgrCallback : public SessionMessageDelegate
{
public:
    void OnMessageReceived(const PacketHeader & header, const PayloadHeader & payloadHeader, const SessionHandle & session,
                           const Transport::PeerAddress & source, DuplicateMessage isDuplicate,
                           System::PacketBufferHandle && msgBuf) override
    {
        size_t data_len = msgBuf->DataLength();

        if (LargeMessageSent)
        {
            int compare = memcmp(msgBuf->Start(), LARGE_PAYLOAD, data_len);
            NL_TEST_ASSERT(mSuite, compare == 0);
        }
        else
        {
            int compare = memcmp(msgBuf->Start(), PAYLOAD, data_len);
            NL_TEST_ASSERT(mSuite, compare == 0);
        }

        ReceiveHandlerCallCount++;
    }

    nlTestSuite * mSuite        = nullptr;
    int ReceiveHandlerCallCount = 0;
    bool LargeMessageSent       = false;
};

void CheckSimpleInitTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    TransportMgr<LoopbackTransport> transportMgr;
    FabricTable fabricTable;
    SessionManager sessionManager;
    secure_channel::MessageCounterManager gMessageCounterManager;
    chip::TestPersistentStorageDelegate deviceStorage;

    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == transportMgr.Init("LOOPBACK"));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == fabricTable.Init(&deviceStorage));
    NL_TEST_ASSERT(
        inSuite,
        CHIP_NO_ERROR ==
            sessionManager.Init(&ctx.GetSystemLayer(), &transportMgr, &gMessageCounterManager, &deviceStorage, &fabricTable));
}

void CheckMessageTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    uint16_t payload_len = sizeof(PAYLOAD);

    TestSessMgrCallback callback;
    callback.LargeMessageSent = false;

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, payload_len);
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    IPAddress addr;
    IPAddress::FromString("::1", addr);
    CHIP_ERROR err = CHIP_NO_ERROR;

    TransportMgr<LoopbackTransport> transportMgr;
    FabricTable fabricTable;
    SessionManager sessionManager;
    secure_channel::MessageCounterManager gMessageCounterManager;
    chip::TestPersistentStorageDelegate deviceStorage;

    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == transportMgr.Init("LOOPBACK"));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == fabricTable.Init(&deviceStorage));
    NL_TEST_ASSERT(
        inSuite,
        CHIP_NO_ERROR ==
            sessionManager.Init(&ctx.GetSystemLayer(), &transportMgr, &gMessageCounterManager, &deviceStorage, &fabricTable));

    callback.mSuite = inSuite;

    sessionManager.SetMessageDelegate(&callback);

    Optional<Transport::PeerAddress> peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));

    FabricIndex aliceFabricIndex;
    FabricInfo aliceFabric;
    aliceFabric.TestOnlyBuildFabric(
        ByteSpan(TestCerts::sTestCert_Root01_Chip, TestCerts::sTestCert_Root01_Chip_Len),
        ByteSpan(TestCerts::sTestCert_ICA01_Chip, TestCerts::sTestCert_ICA01_Chip_Len),
        ByteSpan(TestCerts::sTestCert_Node01_01_Chip, TestCerts::sTestCert_Node01_01_Chip_Len),
        ByteSpan(TestCerts::sTestCert_Node01_01_PublicKey, TestCerts::sTestCert_Node01_01_PublicKey_Len),
        ByteSpan(TestCerts::sTestCert_Node01_01_PrivateKey, TestCerts::sTestCert_Node01_01_PrivateKey_Len));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == fabricTable.AddNewFabric(aliceFabric, &aliceFabricIndex));

    FabricIndex bobFabricIndex;
    FabricInfo bobFabric;
    bobFabric.TestOnlyBuildFabric(
        ByteSpan(TestCerts::sTestCert_Root02_Chip, TestCerts::sTestCert_Root02_Chip_Len),
        ByteSpan(TestCerts::sTestCert_ICA02_Chip, TestCerts::sTestCert_ICA02_Chip_Len),
        ByteSpan(TestCerts::sTestCert_Node02_01_Chip, TestCerts::sTestCert_Node02_01_Chip_Len),
        ByteSpan(TestCerts::sTestCert_Node02_01_PublicKey, TestCerts::sTestCert_Node02_01_PublicKey_Len),
        ByteSpan(TestCerts::sTestCert_Node02_01_PrivateKey, TestCerts::sTestCert_Node02_01_PrivateKey_Len));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == fabricTable.AddNewFabric(bobFabric, &bobFabricIndex));

    SessionHolder aliceToBobSession;
    SecurePairingUsingTestSecret aliceToBobPairing(1, 2, sessionManager);
    err = sessionManager.NewPairing(aliceToBobSession, peer, fabricTable.FindFabricWithIndex(bobFabricIndex)->GetNodeId(),
                                    &aliceToBobPairing, CryptoContext::SessionRole::kInitiator, aliceFabricIndex);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SecurePairingUsingTestSecret bobToAlicePairing(2, 1, sessionManager);
    SessionHolder bobToAliceSession;
    err = sessionManager.NewPairing(bobToAliceSession, peer, fabricTable.FindFabricWithIndex(aliceFabricIndex)->GetNodeId(),
                                    &bobToAlicePairing, CryptoContext::SessionRole::kResponder, bobFabricIndex);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Should be able to send a message to itself by just calling send.
    callback.ReceiveHandlerCallCount = 0;

    PayloadHeader payloadHeader;

    // Set the exchange ID for this header.
    payloadHeader.SetExchangeID(0);

    // Set the protocol ID and message type for this header.
    payloadHeader.SetMessageType(chip::Protocols::Echo::MsgType::EchoRequest);

    EncryptedPacketBufferHandle preparedMessage;
    err = sessionManager.PrepareMessage(aliceToBobSession.Get(), payloadHeader, std::move(buffer), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get(), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 1);

    // Let's send the max sized message and make sure it is received
    chip::System::PacketBufferHandle large_buffer = chip::MessagePacketBuffer::NewWithData(LARGE_PAYLOAD, kMaxAppMessageLen);
    NL_TEST_ASSERT(inSuite, !large_buffer.IsNull());

    callback.LargeMessageSent = true;

    err = sessionManager.PrepareMessage(aliceToBobSession.Get(), payloadHeader, std::move(large_buffer), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get(), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 2);

    uint16_t large_payload_len = sizeof(LARGE_PAYLOAD);

    // Let's send bigger message than supported and make sure it fails to send
    chip::System::PacketBufferHandle extra_large_buffer = chip::MessagePacketBuffer::NewWithData(LARGE_PAYLOAD, large_payload_len);
    NL_TEST_ASSERT(inSuite, !extra_large_buffer.IsNull());

    callback.LargeMessageSent = true;

    err = sessionManager.PrepareMessage(aliceToBobSession.Get(), payloadHeader, std::move(extra_large_buffer), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_MESSAGE_TOO_LONG);

    sessionManager.Shutdown();
}

void SendEncryptedPacketTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    uint16_t payload_len = sizeof(PAYLOAD);

    TestSessMgrCallback callback;
    callback.LargeMessageSent = false;

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, payload_len);
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    IPAddress addr;
    IPAddress::FromString("::1", addr);
    CHIP_ERROR err = CHIP_NO_ERROR;

    TransportMgr<LoopbackTransport> transportMgr;
    FabricTable fabricTable;
    SessionManager sessionManager;
    secure_channel::MessageCounterManager gMessageCounterManager;
    chip::TestPersistentStorageDelegate deviceStorage;

    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == transportMgr.Init("LOOPBACK"));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == fabricTable.Init(&deviceStorage));
    NL_TEST_ASSERT(
        inSuite,
        CHIP_NO_ERROR ==
            sessionManager.Init(&ctx.GetSystemLayer(), &transportMgr, &gMessageCounterManager, &deviceStorage, &fabricTable));

    callback.mSuite = inSuite;

    sessionManager.SetMessageDelegate(&callback);

    Optional<Transport::PeerAddress> peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));

    FabricIndex aliceFabricIndex;
    FabricInfo aliceFabric;
    aliceFabric.TestOnlyBuildFabric(
        ByteSpan(TestCerts::sTestCert_Root01_Chip, TestCerts::sTestCert_Root01_Chip_Len),
        ByteSpan(TestCerts::sTestCert_ICA01_Chip, TestCerts::sTestCert_ICA01_Chip_Len),
        ByteSpan(TestCerts::sTestCert_Node01_01_Chip, TestCerts::sTestCert_Node01_01_Chip_Len),
        ByteSpan(TestCerts::sTestCert_Node01_01_PublicKey, TestCerts::sTestCert_Node01_01_PublicKey_Len),
        ByteSpan(TestCerts::sTestCert_Node01_01_PrivateKey, TestCerts::sTestCert_Node01_01_PrivateKey_Len));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == fabricTable.AddNewFabric(aliceFabric, &aliceFabricIndex));

    FabricIndex bobFabricIndex;
    FabricInfo bobFabric;
    bobFabric.TestOnlyBuildFabric(
        ByteSpan(TestCerts::sTestCert_Root02_Chip, TestCerts::sTestCert_Root02_Chip_Len),
        ByteSpan(TestCerts::sTestCert_ICA02_Chip, TestCerts::sTestCert_ICA02_Chip_Len),
        ByteSpan(TestCerts::sTestCert_Node02_01_Chip, TestCerts::sTestCert_Node02_01_Chip_Len),
        ByteSpan(TestCerts::sTestCert_Node02_01_PublicKey, TestCerts::sTestCert_Node02_01_PublicKey_Len),
        ByteSpan(TestCerts::sTestCert_Node02_01_PrivateKey, TestCerts::sTestCert_Node02_01_PrivateKey_Len));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == fabricTable.AddNewFabric(bobFabric, &bobFabricIndex));

    SessionHolder aliceToBobSession;
    SecurePairingUsingTestSecret aliceToBobPairing(1, 2, sessionManager);
    err = sessionManager.NewPairing(aliceToBobSession, peer, fabricTable.FindFabricWithIndex(bobFabricIndex)->GetNodeId(),
                                    &aliceToBobPairing, CryptoContext::SessionRole::kInitiator, aliceFabricIndex);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SecurePairingUsingTestSecret bobToAlicePairing(2, 1, sessionManager);
    SessionHolder bobToAliceSession;
    err = sessionManager.NewPairing(bobToAliceSession, peer, fabricTable.FindFabricWithIndex(aliceFabricIndex)->GetNodeId(),
                                    &bobToAlicePairing, CryptoContext::SessionRole::kResponder, bobFabricIndex);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Should be able to send a message to itself by just calling send.
    callback.ReceiveHandlerCallCount = 0;

    PayloadHeader payloadHeader;
    EncryptedPacketBufferHandle preparedMessage;

    // Set the exchange ID for this header.
    payloadHeader.SetExchangeID(0);

    // Set the protocol ID and message type for this header.
    payloadHeader.SetMessageType(chip::Protocols::Echo::MsgType::EchoRequest);

    payloadHeader.SetInitiator(true);

    err = sessionManager.PrepareMessage(aliceToBobSession.Get(), payloadHeader, std::move(buffer), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get(), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Reset receive side message counter, or duplicated message will be denied.
    Transport::SecureSession * session = bobToAliceSession.Get()->AsSecureSession();
    session->GetSessionMessageCounter().GetPeerMessageCounter().SetCounter(1);

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 1);

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get(), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 2);

    sessionManager.Shutdown();
}

void SendBadEncryptedPacketTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    uint16_t payload_len = sizeof(PAYLOAD);

    TestSessMgrCallback callback;
    callback.LargeMessageSent = false;

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, payload_len);
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    IPAddress addr;
    IPAddress::FromString("::1", addr);
    CHIP_ERROR err = CHIP_NO_ERROR;

    TransportMgr<LoopbackTransport> transportMgr;
    FabricTable fabricTable;
    SessionManager sessionManager;
    secure_channel::MessageCounterManager gMessageCounterManager;
    chip::TestPersistentStorageDelegate deviceStorage;

    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == transportMgr.Init("LOOPBACK"));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == fabricTable.Init(&deviceStorage));
    NL_TEST_ASSERT(
        inSuite,
        CHIP_NO_ERROR ==
            sessionManager.Init(&ctx.GetSystemLayer(), &transportMgr, &gMessageCounterManager, &deviceStorage, &fabricTable));

    callback.mSuite = inSuite;

    sessionManager.SetMessageDelegate(&callback);

    Optional<Transport::PeerAddress> peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));

    FabricIndex aliceFabricIndex;
    FabricInfo aliceFabric;
    aliceFabric.TestOnlyBuildFabric(
        ByteSpan(TestCerts::sTestCert_Root01_Chip, TestCerts::sTestCert_Root01_Chip_Len),
        ByteSpan(TestCerts::sTestCert_ICA01_Chip, TestCerts::sTestCert_ICA01_Chip_Len),
        ByteSpan(TestCerts::sTestCert_Node01_01_Chip, TestCerts::sTestCert_Node01_01_Chip_Len),
        ByteSpan(TestCerts::sTestCert_Node01_01_PublicKey, TestCerts::sTestCert_Node01_01_PublicKey_Len),
        ByteSpan(TestCerts::sTestCert_Node01_01_PrivateKey, TestCerts::sTestCert_Node01_01_PrivateKey_Len));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == fabricTable.AddNewFabric(aliceFabric, &aliceFabricIndex));

    FabricIndex bobFabricIndex;
    FabricInfo bobFabric;
    bobFabric.TestOnlyBuildFabric(
        ByteSpan(TestCerts::sTestCert_Root02_Chip, TestCerts::sTestCert_Root02_Chip_Len),
        ByteSpan(TestCerts::sTestCert_ICA02_Chip, TestCerts::sTestCert_ICA02_Chip_Len),
        ByteSpan(TestCerts::sTestCert_Node02_01_Chip, TestCerts::sTestCert_Node02_01_Chip_Len),
        ByteSpan(TestCerts::sTestCert_Node02_01_PublicKey, TestCerts::sTestCert_Node02_01_PublicKey_Len),
        ByteSpan(TestCerts::sTestCert_Node02_01_PrivateKey, TestCerts::sTestCert_Node02_01_PrivateKey_Len));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == fabricTable.AddNewFabric(bobFabric, &bobFabricIndex));

    SessionHolder aliceToBobSession;
    SecurePairingUsingTestSecret aliceToBobPairing(1, 2, sessionManager);
    err = sessionManager.NewPairing(aliceToBobSession, peer, fabricTable.FindFabricWithIndex(bobFabricIndex)->GetNodeId(),
                                    &aliceToBobPairing, CryptoContext::SessionRole::kInitiator, aliceFabricIndex);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SecurePairingUsingTestSecret bobToAlicePairing(2, 1, sessionManager);
    SessionHolder bobToAliceSession;
    err = sessionManager.NewPairing(bobToAliceSession, peer, fabricTable.FindFabricWithIndex(aliceFabricIndex)->GetNodeId(),
                                    &bobToAlicePairing, CryptoContext::SessionRole::kResponder, bobFabricIndex);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Should be able to send a message to itself by just calling send.
    callback.ReceiveHandlerCallCount = 0;

    PayloadHeader payloadHeader;
    EncryptedPacketBufferHandle preparedMessage;

    // Set the exchange ID for this header.
    payloadHeader.SetExchangeID(0);

    // Set the protocol ID and message type for this header.
    payloadHeader.SetMessageType(chip::Protocols::Echo::MsgType::EchoRequest);

    payloadHeader.SetInitiator(true);

    err = sessionManager.PrepareMessage(aliceToBobSession.Get(), payloadHeader, std::move(buffer), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get(), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 1);

    /* -------------------------------------------------------------------------------------------*/
    // Reset receive side message counter, or duplicated message will be denied.
    Transport::SecureSession * session = bobToAliceSession.Get()->AsSecureSession();
    session->GetSessionMessageCounter().GetPeerMessageCounter().SetCounter(1);

    PacketHeader packetHeader;

    // Change Message ID
    EncryptedPacketBufferHandle badMessageCounterMsg = preparedMessage.CloneData();
    NL_TEST_ASSERT(inSuite, badMessageCounterMsg.ExtractPacketHeader(packetHeader) == CHIP_NO_ERROR);

    uint32_t messageCounter = packetHeader.GetMessageCounter();
    packetHeader.SetMessageCounter(messageCounter + 1);
    NL_TEST_ASSERT(inSuite, badMessageCounterMsg.InsertPacketHeader(packetHeader) == CHIP_NO_ERROR);

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get(), badMessageCounterMsg);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 1);

    /* -------------------------------------------------------------------------------------------*/
    session->GetSessionMessageCounter().GetPeerMessageCounter().SetCounter(1);

    // Change Key ID
    EncryptedPacketBufferHandle badKeyIdMsg = preparedMessage.CloneData();
    NL_TEST_ASSERT(inSuite, badKeyIdMsg.ExtractPacketHeader(packetHeader) == CHIP_NO_ERROR);

    // the secure channel is setup to use key ID 1, and 2. So let's use 3 here.
    packetHeader.SetSessionId(3);
    NL_TEST_ASSERT(inSuite, badKeyIdMsg.InsertPacketHeader(packetHeader) == CHIP_NO_ERROR);

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get(), badKeyIdMsg);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    /* -------------------------------------------------------------------------------------------*/
    session->GetSessionMessageCounter().GetPeerMessageCounter().SetCounter(1);

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 1);

    // Send the correct encrypted msg
    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get(), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 2);

    sessionManager.Shutdown();
}

void StaleConnectionDropTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    constexpr NodeId kSourceNodeId = 123654;

    IPAddress addr;
    IPAddress::FromString("::1", addr);
    CHIP_ERROR err = CHIP_NO_ERROR;

    TransportMgr<LoopbackTransport> transportMgr;
    FabricTable fabricTable;
    SessionManager sessionManager;
    secure_channel::MessageCounterManager gMessageCounterManager;
    chip::TestPersistentStorageDelegate deviceStorage;

    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == transportMgr.Init("LOOPBACK"));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == fabricTable.Init(&deviceStorage));
    NL_TEST_ASSERT(
        inSuite,
        CHIP_NO_ERROR ==
            sessionManager.Init(&ctx.GetSystemLayer(), &transportMgr, &gMessageCounterManager, &deviceStorage, &fabricTable));

    Optional<Transport::PeerAddress> peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));
    TestSessionReleaseCallback callback;
    SessionHolderWithDelegate session1(callback);
    SessionHolderWithDelegate session2(callback);
    SessionHolderWithDelegate session3(callback);
    SessionHolderWithDelegate session4(callback);
    SessionHolderWithDelegate session5(callback);

    // First pairing
    callback.mOldConnectionDropped = false;
    SecurePairingUsingTestSecret pairing1(1, 1, sessionManager);
    err = sessionManager.NewPairing(session1, peer, kSourceNodeId, &pairing1, CryptoContext::SessionRole::kInitiator, 1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !callback.mOldConnectionDropped);

    // New pairing with different peer node ID and different local key ID (same peer key ID)
    callback.mOldConnectionDropped = false;
    SecurePairingUsingTestSecret pairing2(1, 2, sessionManager);
    err = sessionManager.NewPairing(session2, peer, kSourceNodeId, &pairing2, CryptoContext::SessionRole::kResponder, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !callback.mOldConnectionDropped);

    // New pairing with undefined node ID and different local key ID (same peer key ID)
    callback.mOldConnectionDropped = false;
    SecurePairingUsingTestSecret pairing3(1, 3, sessionManager);
    err = sessionManager.NewPairing(session3, peer, kUndefinedNodeId, &pairing3, CryptoContext::SessionRole::kResponder, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !callback.mOldConnectionDropped);

    // New pairing with same local key ID, and a given node ID
    callback.mOldConnectionDropped = false;
    SecurePairingUsingTestSecret pairing4(1, 2, sessionManager);
    err = sessionManager.NewPairing(session4, peer, kSourceNodeId, &pairing4, CryptoContext::SessionRole::kResponder, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, callback.mOldConnectionDropped);

    // New pairing with same local key ID, and undefined node ID
    callback.mOldConnectionDropped = false;
    SecurePairingUsingTestSecret pairing5(1, 1, sessionManager);
    err = sessionManager.NewPairing(session5, peer, kUndefinedNodeId, &pairing5, CryptoContext::SessionRole::kResponder, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, callback.mOldConnectionDropped);

    sessionManager.Shutdown();
}

void SendPacketWithOldCounterTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    uint16_t payload_len = sizeof(PAYLOAD);

    TestSessMgrCallback callback;
    callback.LargeMessageSent = false;

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, payload_len);
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    IPAddress addr;
    IPAddress::FromString("::1", addr);
    CHIP_ERROR err = CHIP_NO_ERROR;

    TransportMgr<LoopbackTransport> transportMgr;
    FabricTable fabricTable;
    SessionManager sessionManager;
    secure_channel::MessageCounterManager gMessageCounterManager;
    chip::TestPersistentStorageDelegate deviceStorage;

    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == transportMgr.Init("LOOPBACK"));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == fabricTable.Init(&deviceStorage));
    NL_TEST_ASSERT(
        inSuite,
        CHIP_NO_ERROR ==
            sessionManager.Init(&ctx.GetSystemLayer(), &transportMgr, &gMessageCounterManager, &deviceStorage, &fabricTable));

    callback.mSuite = inSuite;

    sessionManager.SetMessageDelegate(&callback);

    Optional<Transport::PeerAddress> peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));

    FabricIndex aliceFabricIndex;
    FabricInfo aliceFabric;
    aliceFabric.TestOnlyBuildFabric(
        ByteSpan(TestCerts::sTestCert_Root01_Chip, TestCerts::sTestCert_Root01_Chip_Len),
        ByteSpan(TestCerts::sTestCert_ICA01_Chip, TestCerts::sTestCert_ICA01_Chip_Len),
        ByteSpan(TestCerts::sTestCert_Node01_01_Chip, TestCerts::sTestCert_Node01_01_Chip_Len),
        ByteSpan(TestCerts::sTestCert_Node01_01_PublicKey, TestCerts::sTestCert_Node01_01_PublicKey_Len),
        ByteSpan(TestCerts::sTestCert_Node01_01_PrivateKey, TestCerts::sTestCert_Node01_01_PrivateKey_Len));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == fabricTable.AddNewFabric(aliceFabric, &aliceFabricIndex));

    FabricIndex bobFabricIndex;
    FabricInfo bobFabric;
    bobFabric.TestOnlyBuildFabric(
        ByteSpan(TestCerts::sTestCert_Root02_Chip, TestCerts::sTestCert_Root02_Chip_Len),
        ByteSpan(TestCerts::sTestCert_ICA02_Chip, TestCerts::sTestCert_ICA02_Chip_Len),
        ByteSpan(TestCerts::sTestCert_Node02_01_Chip, TestCerts::sTestCert_Node02_01_Chip_Len),
        ByteSpan(TestCerts::sTestCert_Node02_01_PublicKey, TestCerts::sTestCert_Node02_01_PublicKey_Len),
        ByteSpan(TestCerts::sTestCert_Node02_01_PrivateKey, TestCerts::sTestCert_Node02_01_PrivateKey_Len));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == fabricTable.AddNewFabric(bobFabric, &bobFabricIndex));

    SessionHolder aliceToBobSession;
    SecurePairingUsingTestSecret aliceToBobPairing(1, 2, sessionManager);
    err = sessionManager.NewPairing(aliceToBobSession, peer, fabricTable.FindFabricWithIndex(bobFabricIndex)->GetNodeId(),
                                    &aliceToBobPairing, CryptoContext::SessionRole::kInitiator, aliceFabricIndex);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SecurePairingUsingTestSecret bobToAlicePairing(2, 1, sessionManager);
    SessionHolder bobToAliceSession;
    err = sessionManager.NewPairing(bobToAliceSession, peer, fabricTable.FindFabricWithIndex(aliceFabricIndex)->GetNodeId(),
                                    &bobToAlicePairing, CryptoContext::SessionRole::kResponder, bobFabricIndex);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    callback.ReceiveHandlerCallCount = 0;

    PayloadHeader payloadHeader;
    EncryptedPacketBufferHandle preparedMessage;

    // Set the exchange ID for this header.
    payloadHeader.SetExchangeID(0);

    // Set the protocol ID and message type for this header.
    payloadHeader.SetMessageType(chip::Protocols::Echo::MsgType::EchoRequest);

    payloadHeader.SetInitiator(true);

    err = sessionManager.PrepareMessage(aliceToBobSession.Get(), payloadHeader, std::move(buffer), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get(), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 1);

    // Now advance our message counter by 5.
    EncryptedPacketBufferHandle newMessage;
    for (size_t i = 0; i < 5; ++i)
    {
        buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, payload_len);
        NL_TEST_ASSERT(inSuite, !buffer.IsNull());

        err = sessionManager.PrepareMessage(aliceToBobSession.Get(), payloadHeader, std::move(buffer), newMessage);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    }

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get(), newMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 2);

    // Now resend our original message.  It should be rejected as a duplicate.

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get(), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 2);

    sessionManager.Shutdown();
}

void SendPacketWithTooOldCounterTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    uint16_t payload_len = sizeof(PAYLOAD);

    TestSessMgrCallback callback;
    callback.LargeMessageSent = false;

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, payload_len);
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    IPAddress addr;
    IPAddress::FromString("::1", addr);
    CHIP_ERROR err = CHIP_NO_ERROR;

    TransportMgr<LoopbackTransport> transportMgr;
    FabricTable fabricTable;
    SessionManager sessionManager;
    secure_channel::MessageCounterManager gMessageCounterManager;
    chip::TestPersistentStorageDelegate deviceStorage;

    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == transportMgr.Init("LOOPBACK"));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == fabricTable.Init(&deviceStorage));
    NL_TEST_ASSERT(
        inSuite,
        CHIP_NO_ERROR ==
            sessionManager.Init(&ctx.GetSystemLayer(), &transportMgr, &gMessageCounterManager, &deviceStorage, &fabricTable));

    callback.mSuite = inSuite;

    sessionManager.SetMessageDelegate(&callback);

    Optional<Transport::PeerAddress> peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));

    FabricIndex aliceFabricIndex;
    FabricInfo aliceFabric;
    aliceFabric.TestOnlyBuildFabric(
        ByteSpan(TestCerts::sTestCert_Root01_Chip, TestCerts::sTestCert_Root01_Chip_Len),
        ByteSpan(TestCerts::sTestCert_ICA01_Chip, TestCerts::sTestCert_ICA01_Chip_Len),
        ByteSpan(TestCerts::sTestCert_Node01_01_Chip, TestCerts::sTestCert_Node01_01_Chip_Len),
        ByteSpan(TestCerts::sTestCert_Node01_01_PublicKey, TestCerts::sTestCert_Node01_01_PublicKey_Len),
        ByteSpan(TestCerts::sTestCert_Node01_01_PrivateKey, TestCerts::sTestCert_Node01_01_PrivateKey_Len));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == fabricTable.AddNewFabric(aliceFabric, &aliceFabricIndex));

    FabricIndex bobFabricIndex;
    FabricInfo bobFabric;
    bobFabric.TestOnlyBuildFabric(
        ByteSpan(TestCerts::sTestCert_Root02_Chip, TestCerts::sTestCert_Root02_Chip_Len),
        ByteSpan(TestCerts::sTestCert_ICA02_Chip, TestCerts::sTestCert_ICA02_Chip_Len),
        ByteSpan(TestCerts::sTestCert_Node02_01_Chip, TestCerts::sTestCert_Node02_01_Chip_Len),
        ByteSpan(TestCerts::sTestCert_Node02_01_PublicKey, TestCerts::sTestCert_Node02_01_PublicKey_Len),
        ByteSpan(TestCerts::sTestCert_Node02_01_PrivateKey, TestCerts::sTestCert_Node02_01_PrivateKey_Len));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == fabricTable.AddNewFabric(bobFabric, &bobFabricIndex));

    SessionHolder aliceToBobSession;
    SecurePairingUsingTestSecret aliceToBobPairing(1, 2, sessionManager);
    err = sessionManager.NewPairing(aliceToBobSession, peer, fabricTable.FindFabricWithIndex(bobFabricIndex)->GetNodeId(),
                                    &aliceToBobPairing, CryptoContext::SessionRole::kInitiator, aliceFabricIndex);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SecurePairingUsingTestSecret bobToAlicePairing(2, 1, sessionManager);
    SessionHolder bobToAliceSession;
    err = sessionManager.NewPairing(bobToAliceSession, peer, fabricTable.FindFabricWithIndex(aliceFabricIndex)->GetNodeId(),
                                    &bobToAlicePairing, CryptoContext::SessionRole::kResponder, bobFabricIndex);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    callback.ReceiveHandlerCallCount = 0;

    PayloadHeader payloadHeader;
    EncryptedPacketBufferHandle preparedMessage;

    // Set the exchange ID for this header.
    payloadHeader.SetExchangeID(0);

    // Set the protocol ID and message type for this header.
    payloadHeader.SetMessageType(chip::Protocols::Echo::MsgType::EchoRequest);

    payloadHeader.SetInitiator(true);

    err = sessionManager.PrepareMessage(aliceToBobSession.Get(), payloadHeader, std::move(buffer), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get(), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 1);

    // Now advance our message counter by at least
    // CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE + 2, so preparedMessage will be
    // out of the window.
    EncryptedPacketBufferHandle newMessage;
    for (size_t i = 0; i < CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE + 2; ++i)
    {
        buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, payload_len);
        NL_TEST_ASSERT(inSuite, !buffer.IsNull());

        err = sessionManager.PrepareMessage(aliceToBobSession.Get(), payloadHeader, std::move(buffer), newMessage);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    }

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get(), newMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 2);

    // Now resend our original message.  It should be rejected as a duplicate.

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get(), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 2);

    sessionManager.Shutdown();
}

static void RandomSessionIdAllocatorOffset(nlTestSuite * inSuite, SessionManager & sessionManager, int max)
{
    // Allocate + free a pseudo-random number of sessions to create a
    // pseudo-random offset in mNextSessionId.
    const int bound = rand() % max;
    for (int i = 0; i < bound; ++i)
    {
        auto handle = sessionManager.AllocateSession();
        NL_TEST_ASSERT(inSuite, handle.HasValue());
        sessionManager.ExpirePairing(handle.Value());
    }
}

void SessionAllocationTest(nlTestSuite * inSuite, void * inContext)
{
    SessionManager sessionManager;
    TestSessionReleaseCallback callback;

    // Allocate a session.
    uint16_t sessionId1;
    {
        auto handle = sessionManager.AllocateSession();
        NL_TEST_ASSERT(inSuite, handle.HasValue());
        SessionHolderWithDelegate session(handle.Value(), callback);
        sessionId1 = session->AsSecureSession()->GetLocalSessionId();
    }

    // Allocate a session at a colliding ID, verify eviction.
    {
        callback.mOldConnectionDropped = false;
        auto handle                    = sessionManager.AllocateSession(sessionId1);
        NL_TEST_ASSERT(inSuite, handle.HasValue());
        SessionHolderWithDelegate session(handle.Value(), callback);
    }

    // Verify that we increment session ID by 1 for each allocation, except for
    // the wraparound case where we skip session ID 0.
    auto prevSessionId = sessionId1;
    for (uint32_t i = 0; i < 10; ++i)
    {
        auto handle = sessionManager.AllocateSession();
        if (!handle.HasValue())
        {
            break;
        }
        auto sessionId = handle.Value()->AsSecureSession()->GetLocalSessionId();
        NL_TEST_ASSERT(inSuite, sessionId - prevSessionId == 1 || (sessionId == 1 && prevSessionId == 65535));
        NL_TEST_ASSERT(inSuite, sessionId != 0);
        prevSessionId = sessionId;
    }

    // Reconstruct the Session Manager to reset state.
    sessionManager.~SessionManager();
    new (&sessionManager) SessionManager();

    prevSessionId = 0;
    // Verify that we increment session ID by 1 for each allocation (except for
    // the wraparound case where we skip session ID 0), even when allocated
    // sessions are immediately freed.
    for (uint32_t i = 0; i < UINT16_MAX + 10; ++i)
    {
        auto handle = sessionManager.AllocateSession();
        NL_TEST_ASSERT(inSuite, handle.HasValue());
        auto sessionId = handle.Value()->AsSecureSession()->GetLocalSessionId();
        NL_TEST_ASSERT(inSuite, sessionId - prevSessionId == 1 || (sessionId == 1 && prevSessionId == 65535));
        NL_TEST_ASSERT(inSuite, sessionId != 0);
        prevSessionId = sessionId;
        sessionManager.ExpirePairing(handle.Value());
    }

    // Verify that the allocator does not give colliding IDs.
    constexpr int collisionTestIterations = 1;
    for (int i = 0; i < collisionTestIterations; ++i)
    {
        // Allocate some session handles at pseudo-random offsets in the session
        // ID space.
        constexpr size_t numHandles = CHIP_CONFIG_PEER_CONNECTION_POOL_SIZE - 1;
        Optional<SessionHandle> handles[numHandles];
        uint16_t sessionIds[numHandles];
        for (size_t h = 0; h < numHandles; ++h)
        {
            constexpr int maxOffset = 5000;
            handles[h]              = sessionManager.AllocateSession();
            NL_TEST_ASSERT(inSuite, handles[h].HasValue());
            sessionIds[h] = handles[h].Value()->AsSecureSession()->GetLocalSessionId();
            RandomSessionIdAllocatorOffset(inSuite, sessionManager, maxOffset);
        }

        // Verify that none collide each other.
        for (size_t h = 0; h < numHandles; ++h)
        {
            NL_TEST_ASSERT(inSuite, sessionIds[h] != sessionIds[(h + 1) % numHandles]);
        }

        // Allocate through the entire session ID space and verify that none of
        // these collide either.
        for (int j = 0; j < UINT16_MAX; ++j)
        {
            auto handle = sessionManager.AllocateSession();
            NL_TEST_ASSERT(inSuite, handle.HasValue());
            auto potentialCollision = handle.Value()->AsSecureSession()->GetLocalSessionId();
            for (size_t h = 0; h < numHandles; ++h)
            {
                NL_TEST_ASSERT(inSuite, potentialCollision != sessionIds[h]);
            }
            sessionManager.ExpirePairing(handle.Value());
        }

        // Free our allocated sessions.
        for (size_t h = 0; h < numHandles; ++h)
        {
            sessionManager.ExpirePairing(handles[h].Value());
        }
    }

    sessionManager.Shutdown();
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("Simple Init Test",               CheckSimpleInitTest),
    NL_TEST_DEF("Message Self Test",              CheckMessageTest),
    NL_TEST_DEF("Send Encrypted Packet Test",     SendEncryptedPacketTest),
    NL_TEST_DEF("Send Bad Encrypted Packet Test", SendBadEncryptedPacketTest),
    NL_TEST_DEF("Drop stale connection Test",     StaleConnectionDropTest),
    NL_TEST_DEF("Old counter Test",               SendPacketWithOldCounterTest),
    NL_TEST_DEF("Too-old counter Test",           SendPacketWithTooOldCounterTest),
    NL_TEST_DEF("Session Allocation Test",        SessionAllocationTest),

    NL_TEST_SENTINEL()
};
// clang-format on

int Initialize(void * aContext);
int Finalize(void * aContext);

// clang-format off
nlTestSuite sSuite =
{
    "Test-CHIP-SessionManager",
    &sTests[0],
    Initialize,
    Finalize
};
// clang-format on

/**
 *  Initialize the test suite.
 */
int Initialize(void * aContext)
{
    CHIP_ERROR err = reinterpret_cast<TestContext *>(aContext)->Init();
    return (err == CHIP_NO_ERROR) ? SUCCESS : FAILURE;
}

/**
 *  Finalize the test suite.
 */
int Finalize(void * aContext)
{
    CHIP_ERROR err = reinterpret_cast<TestContext *>(aContext)->Shutdown();
    return (err == CHIP_NO_ERROR) ? SUCCESS : FAILURE;
}

} // namespace

/**
 *  Main
 */
int TestSessionManager()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, &sContext);

    int r = (nlTestRunnerStats(&sSuite));
    return r;
}

CHIP_REGISTER_TEST_SUITE(TestSessionManager);
