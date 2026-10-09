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

#include <algorithm>
#include <errno.h>

#include <pw_unit_test/framework.h>

#define CHIP_ENABLE_TEST_ENCRYPTED_BUFFER_API // Up here in case some other header
                                              // includes SessionManager.h indirectly

#include <access/SubjectDescriptor.h>
#include <credentials/PersistentStorageOpCertStore.h>
#include <credentials/tests/CHIPCert_unit_test_vectors.h>
#include <crypto/DefaultSessionKeystore.h>
#include <crypto/PersistentStorageOperationalKeystore.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <protocols/Protocols.h>
#include <protocols/echo/Echo.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/PASESession.h>
#include <system/RAIIMockClock.h>
#include <transport/MessageStats.h>
#include <transport/SessionManager.h>
#include <transport/TransportMgr.h>
#include <transport/raw/MessageHeader.h>
#include <transport/tests/LoopbackTransportManager.h>

#undef CHIP_ENABLE_TEST_ENCRYPTED_BUFFER_API

namespace {

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::Testing;
using namespace chip::TestCerts;

using TestContext = LoopbackTransportManager;

const char PAYLOAD[] = "Hello!";

const char LARGE_PAYLOAD[kMaxAppMessageLen + 1] = "test message";

// Just enough init to replace a ton of boilerplate
class FabricTableHolder
{
public:
    FabricTableHolder() {}
    ~FabricTableHolder()
    {
        mFabricTable.Shutdown();
        mOpKeyStore.Finish();
        mOpCertStore.Finish();
    }

    CHIP_ERROR Init()
    {
        ReturnErrorOnFailure(mOpKeyStore.Init(&mStorage));
        ReturnErrorOnFailure(mOpCertStore.Init(&mStorage));

        chip::FabricTable::InitParams initParams;
        initParams.storage             = &mStorage;
        initParams.operationalKeystore = &mOpKeyStore;
        initParams.opCertStore         = &mOpCertStore;

        return mFabricTable.Init(initParams);
    }

    FabricTable & GetFabricTable() { return mFabricTable; }

private:
    chip::FabricTable mFabricTable;
    chip::TestPersistentStorageDelegate mStorage;
    chip::PersistentStorageOperationalKeystore mOpKeyStore;
    chip::Credentials::PersistentStorageOpCertStore mOpCertStore;
};

class TestSessMgrCallback : public SessionMessageDelegate
{
public:
    void OnMessageReceived(const PacketHeader & header, const PayloadHeader & payloadHeader, const SessionHandle & session,
                           DuplicateMessage isDuplicate, System::PacketBufferHandle && msgBuf) override
    {
        size_t data_len = msgBuf->DataLength();

        if (LargeMessageSent)
        {
            EXPECT_EQ(0, memcmp(msgBuf->Start(), LARGE_PAYLOAD, data_len));
        }
        else
        {
            EXPECT_EQ(0, memcmp(msgBuf->Start(), PAYLOAD, data_len));
        }

        ReceiveHandlerCallCount++;
        lastSubjectDescriptor = session->GetSubjectDescriptor();
    }

    int ReceiveHandlerCallCount = 0;
    bool LargeMessageSent       = false;
    Access::SubjectDescriptor lastSubjectDescriptor{};
};

class TestSessionManager : public ::testing::Test
{
protected:
    void SetUp() { ASSERT_EQ(mContext.Init(), CHIP_NO_ERROR); }
    void TearDown() { mContext.Shutdown(); }

    TestContext mContext;
};

TEST_F(TestSessionManager, CheckSimpleInitTest)
{
    FabricTableHolder fabricTableHolder;
    SessionManager sessionManager;
    secure_channel::MessageCounterManager gMessageCounterManager;
    chip::TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;

    EXPECT_EQ(CHIP_NO_ERROR, fabricTableHolder.Init());
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.Init(&mContext.GetSystemLayer(), &mContext.GetTransportMgr(), &gMessageCounterManager, &deviceStorage,
                                  &fabricTableHolder.GetFabricTable(), sessionKeystore));
}

TEST_F(TestSessionManager, CheckMessageOverPaseTest)
{
    uint16_t payload_len = sizeof(PAYLOAD);

    TestSessMgrCallback callback;
    callback.LargeMessageSent = false;

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, payload_len);
    EXPECT_FALSE(buffer.IsNull());

    IPAddress addr;
    IPAddress::FromString("::1", addr);
    CHIP_ERROR err = CHIP_NO_ERROR;

    FabricTableHolder fabricTableHolder;
    SessionManager sessionManager;
    secure_channel::MessageCounterManager gMessageCounterManager;
    chip::TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    FabricTable & fabricTable    = fabricTableHolder.GetFabricTable();
    FabricIndex aliceFabricIndex = kUndefinedFabricIndex;
    FabricIndex bobFabricIndex   = kUndefinedFabricIndex;

    EXPECT_EQ(CHIP_NO_ERROR, fabricTableHolder.Init());
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.Init(&mContext.GetSystemLayer(), &mContext.GetTransportMgr(), &gMessageCounterManager, &deviceStorage,
                                  &fabricTableHolder.GetFabricTable(), sessionKeystore));

    sessionManager.SetMessageDelegate(&callback);

    Transport::PeerAddress peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));

    err =
        fabricTable.AddNewFabricForTestIgnoringCollisions(GetRootACertAsset().mCert, GetIAA1CertAsset().mCert,
                                                          GetNodeA1CertAsset().mCert, GetNodeA1CertAsset().mKey, &aliceFabricIndex);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    err = fabricTable.AddNewFabricForTestIgnoringCollisions(GetRootACertAsset().mCert, GetIAA1CertAsset().mCert,
                                                            GetNodeA2CertAsset().mCert, GetNodeA2CertAsset().mKey, &bobFabricIndex);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    SessionHolder aliceToBobSession;
    err = sessionManager.InjectPaseSessionWithTestKey(aliceToBobSession, 2,
                                                      fabricTable.FindFabricWithIndex(bobFabricIndex)->GetNodeId(), 1,
                                                      aliceFabricIndex, peer, CryptoContext::SessionRole::kInitiator);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    SessionHolder bobToAliceSession;
    err = sessionManager.InjectPaseSessionWithTestKey(bobToAliceSession, 1,
                                                      fabricTable.FindFabricWithIndex(aliceFabricIndex)->GetNodeId(), 2,
                                                      bobFabricIndex, peer, CryptoContext::SessionRole::kResponder);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Should be able to send a message to itself by just calling send.
    callback.ReceiveHandlerCallCount = 0;

    PayloadHeader payloadHeader;

    // Set the exchange ID for this header.
    payloadHeader.SetExchangeID(0);

    // Set the protocol ID and message type for this header.
    payloadHeader.SetMessageType(chip::Protocols::Echo::MsgType::EchoRequest);

    EncryptedPacketBufferHandle preparedMessage;
    err = sessionManager.PrepareMessage(aliceToBobSession.Get().Value(), payloadHeader, std::move(buffer), preparedMessage);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get().Value(), preparedMessage);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    mContext.DrainAndServiceIO();
    ASSERT_EQ(callback.ReceiveHandlerCallCount, 1);

    // This was a PASE session so we expect the subject descriptor to indicate it's for commissioning.
    EXPECT_TRUE(callback.lastSubjectDescriptor.isCommissioning);

    // Let's send the max sized message and make sure it is received
    chip::System::PacketBufferHandle large_buffer = chip::MessagePacketBuffer::NewWithData(LARGE_PAYLOAD, kMaxAppMessageLen);
    EXPECT_FALSE(large_buffer.IsNull());

    callback.LargeMessageSent = true;

    err = sessionManager.PrepareMessage(aliceToBobSession.Get().Value(), payloadHeader, std::move(large_buffer), preparedMessage);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get().Value(), preparedMessage);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    mContext.DrainAndServiceIO();
    EXPECT_EQ(callback.ReceiveHandlerCallCount, 2);

    uint16_t large_payload_len = sizeof(LARGE_PAYLOAD);

    // Let's send bigger message than supported and make sure it fails to send
    chip::System::PacketBufferHandle extra_large_buffer = chip::MessagePacketBuffer::NewWithData(LARGE_PAYLOAD, large_payload_len);
    EXPECT_FALSE(extra_large_buffer.IsNull());

    callback.LargeMessageSent = true;

    err = sessionManager.PrepareMessage(aliceToBobSession.Get().Value(), payloadHeader, std::move(extra_large_buffer),
                                        preparedMessage);
    EXPECT_EQ(err, CHIP_ERROR_MESSAGE_TOO_LONG);

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, SendEncryptedPacketTest)
{
    uint16_t payload_len = sizeof(PAYLOAD);

    TestSessMgrCallback callback;
    callback.LargeMessageSent = false;

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, payload_len);
    EXPECT_FALSE(buffer.IsNull());

    IPAddress addr;
    IPAddress::FromString("::1", addr);
    CHIP_ERROR err = CHIP_NO_ERROR;

    FabricTableHolder fabricTableHolder;
    SessionManager sessionManager;
    secure_channel::MessageCounterManager gMessageCounterManager;
    chip::TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    FabricTable & fabricTable    = fabricTableHolder.GetFabricTable();
    FabricIndex aliceFabricIndex = kUndefinedFabricIndex;
    FabricIndex bobFabricIndex   = kUndefinedFabricIndex;

    EXPECT_EQ(CHIP_NO_ERROR, fabricTableHolder.Init());
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.Init(&mContext.GetSystemLayer(), &mContext.GetTransportMgr(), &gMessageCounterManager, &deviceStorage,
                                  &fabricTableHolder.GetFabricTable(), sessionKeystore));

    sessionManager.SetMessageDelegate(&callback);

    Transport::PeerAddress peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));

    err =
        fabricTable.AddNewFabricForTestIgnoringCollisions(GetRootACertAsset().mCert, GetIAA1CertAsset().mCert,
                                                          GetNodeA1CertAsset().mCert, GetNodeA1CertAsset().mKey, &aliceFabricIndex);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    err = fabricTable.AddNewFabricForTestIgnoringCollisions(GetRootACertAsset().mCert, GetIAA1CertAsset().mCert,
                                                            GetNodeA2CertAsset().mCert, GetNodeA2CertAsset().mKey, &bobFabricIndex);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    SessionHolder aliceToBobSession;
    err = sessionManager.InjectPaseSessionWithTestKey(aliceToBobSession, 2,
                                                      fabricTable.FindFabricWithIndex(bobFabricIndex)->GetNodeId(), 1,
                                                      aliceFabricIndex, peer, CryptoContext::SessionRole::kInitiator);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    SessionHolder bobToAliceSession;
    err = sessionManager.InjectPaseSessionWithTestKey(bobToAliceSession, 1,
                                                      fabricTable.FindFabricWithIndex(aliceFabricIndex)->GetNodeId(), 2,
                                                      bobFabricIndex, peer, CryptoContext::SessionRole::kResponder);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Should be able to send a message to itself by just calling send.
    callback.ReceiveHandlerCallCount = 0;

    PayloadHeader payloadHeader;
    EncryptedPacketBufferHandle preparedMessage;

    // Set the exchange ID for this header.
    payloadHeader.SetExchangeID(0);

    // Set the protocol ID and message type for this header.
    payloadHeader.SetMessageType(chip::Protocols::Echo::MsgType::EchoRequest);

    payloadHeader.SetInitiator(true);

    err = sessionManager.PrepareMessage(aliceToBobSession.Get().Value(), payloadHeader, std::move(buffer), preparedMessage);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get().Value(), preparedMessage);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    mContext.DrainAndServiceIO();
    EXPECT_EQ(callback.ReceiveHandlerCallCount, 1);

    // Reset receive side message counter, or duplicated message will be denied.
    Transport::SecureSession * session = bobToAliceSession.Get().Value()->AsSecureSession();
    session->GetSessionMessageCounter().GetPeerMessageCounter().SetCounter(1);

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get().Value(), preparedMessage);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    mContext.DrainAndServiceIO();
    EXPECT_EQ(callback.ReceiveHandlerCallCount, 2);

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, SendBadEncryptedPacketTest)
{
    uint16_t payload_len = sizeof(PAYLOAD);

    TestSessMgrCallback callback;
    callback.LargeMessageSent = false;

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, payload_len);
    EXPECT_FALSE(buffer.IsNull());

    IPAddress addr;
    IPAddress::FromString("::1", addr);
    CHIP_ERROR err = CHIP_NO_ERROR;

    FabricTableHolder fabricTableHolder;
    SessionManager sessionManager;
    secure_channel::MessageCounterManager gMessageCounterManager;
    chip::TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    FabricTable & fabricTable    = fabricTableHolder.GetFabricTable();
    FabricIndex aliceFabricIndex = kUndefinedFabricIndex;
    FabricIndex bobFabricIndex   = kUndefinedFabricIndex;

    EXPECT_EQ(CHIP_NO_ERROR, fabricTableHolder.Init());
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.Init(&mContext.GetSystemLayer(), &mContext.GetTransportMgr(), &gMessageCounterManager, &deviceStorage,
                                  &fabricTableHolder.GetFabricTable(), sessionKeystore));

    sessionManager.SetMessageDelegate(&callback);

    Transport::PeerAddress peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));

    err =
        fabricTable.AddNewFabricForTestIgnoringCollisions(GetRootACertAsset().mCert, GetIAA1CertAsset().mCert,
                                                          GetNodeA1CertAsset().mCert, GetNodeA1CertAsset().mKey, &aliceFabricIndex);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    err = fabricTable.AddNewFabricForTestIgnoringCollisions(GetRootACertAsset().mCert, GetIAA1CertAsset().mCert,
                                                            GetNodeA2CertAsset().mCert, GetNodeA2CertAsset().mKey, &bobFabricIndex);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    SessionHolder aliceToBobSession;
    err = sessionManager.InjectPaseSessionWithTestKey(aliceToBobSession, 2,
                                                      fabricTable.FindFabricWithIndex(bobFabricIndex)->GetNodeId(), 1,
                                                      aliceFabricIndex, peer, CryptoContext::SessionRole::kInitiator);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    SessionHolder bobToAliceSession;
    err = sessionManager.InjectPaseSessionWithTestKey(bobToAliceSession, 1,
                                                      fabricTable.FindFabricWithIndex(aliceFabricIndex)->GetNodeId(), 2,
                                                      bobFabricIndex, peer, CryptoContext::SessionRole::kResponder);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Should be able to send a message to itself by just calling send.
    callback.ReceiveHandlerCallCount = 0;

    PayloadHeader payloadHeader;
    EncryptedPacketBufferHandle preparedMessage;

    // Set the exchange ID for this header.
    payloadHeader.SetExchangeID(0);

    // Set the protocol ID and message type for this header.
    payloadHeader.SetMessageType(chip::Protocols::Echo::MsgType::EchoRequest);

    payloadHeader.SetInitiator(true);

    err = sessionManager.PrepareMessage(aliceToBobSession.Get().Value(), payloadHeader, std::move(buffer), preparedMessage);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get().Value(), preparedMessage);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    mContext.DrainAndServiceIO();
    EXPECT_EQ(callback.ReceiveHandlerCallCount, 1);

    /* -------------------------------------------------------------------------------------------*/
    // Reset receive side message counter, or duplicated message will be denied.
    Transport::SecureSession * session = bobToAliceSession.Get().Value()->AsSecureSession();
    session->GetSessionMessageCounter().GetPeerMessageCounter().SetCounter(1);

    PacketHeader packetHeader;

    // Change Message ID
    EncryptedPacketBufferHandle badMessageCounterMsg = preparedMessage.CloneData();
    EXPECT_EQ(badMessageCounterMsg.ExtractPacketHeader(packetHeader), CHIP_NO_ERROR);

    uint32_t messageCounter = packetHeader.GetMessageCounter();
    packetHeader.SetMessageCounter(messageCounter + 1);
    EXPECT_EQ(badMessageCounterMsg.InsertPacketHeader(packetHeader), CHIP_NO_ERROR);

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get().Value(), badMessageCounterMsg);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    mContext.DrainAndServiceIO();
    EXPECT_EQ(callback.ReceiveHandlerCallCount, 1);

    /* -------------------------------------------------------------------------------------------*/
    session->GetSessionMessageCounter().GetPeerMessageCounter().SetCounter(1);

    // Change Key ID
    EncryptedPacketBufferHandle badKeyIdMsg = preparedMessage.CloneData();
    EXPECT_EQ(badKeyIdMsg.ExtractPacketHeader(packetHeader), CHIP_NO_ERROR);

    // the secure channel is setup to use key ID 1, and 2. So let's use 3 here.
    packetHeader.SetSessionId(3);
    EXPECT_EQ(badKeyIdMsg.InsertPacketHeader(packetHeader), CHIP_NO_ERROR);

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get().Value(), badKeyIdMsg);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    mContext.DrainAndServiceIO();
    EXPECT_EQ(callback.ReceiveHandlerCallCount, 1);

    /* -------------------------------------------------------------------------------------------*/
    session->GetSessionMessageCounter().GetPeerMessageCounter().SetCounter(1);

    // Send the correct encrypted msg
    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get().Value(), preparedMessage);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    mContext.DrainAndServiceIO();
    EXPECT_EQ(callback.ReceiveHandlerCallCount, 2);

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, SendPacketWithOldCounterTest)
{
    uint16_t payload_len = sizeof(PAYLOAD);

    TestSessMgrCallback callback;
    callback.LargeMessageSent = false;

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, payload_len);
    EXPECT_FALSE(buffer.IsNull());

    IPAddress addr;
    IPAddress::FromString("::1", addr);
    CHIP_ERROR err = CHIP_NO_ERROR;

    FabricTableHolder fabricTableHolder;
    SessionManager sessionManager;
    secure_channel::MessageCounterManager gMessageCounterManager;
    chip::TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    FabricTable & fabricTable    = fabricTableHolder.GetFabricTable();
    FabricIndex aliceFabricIndex = kUndefinedFabricIndex;
    FabricIndex bobFabricIndex   = kUndefinedFabricIndex;

    EXPECT_EQ(CHIP_NO_ERROR, fabricTableHolder.Init());
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.Init(&mContext.GetSystemLayer(), &mContext.GetTransportMgr(), &gMessageCounterManager, &deviceStorage,
                                  &fabricTableHolder.GetFabricTable(), sessionKeystore));

    sessionManager.SetMessageDelegate(&callback);

    Transport::PeerAddress peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));

    err =
        fabricTable.AddNewFabricForTestIgnoringCollisions(GetRootACertAsset().mCert, GetIAA1CertAsset().mCert,
                                                          GetNodeA1CertAsset().mCert, GetNodeA1CertAsset().mKey, &aliceFabricIndex);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    err = fabricTable.AddNewFabricForTestIgnoringCollisions(GetRootACertAsset().mCert, GetIAA1CertAsset().mCert,
                                                            GetNodeA2CertAsset().mCert, GetNodeA2CertAsset().mKey, &bobFabricIndex);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    SessionHolder aliceToBobSession;
    err = sessionManager.InjectPaseSessionWithTestKey(aliceToBobSession, 2,
                                                      fabricTable.FindFabricWithIndex(bobFabricIndex)->GetNodeId(), 1,
                                                      aliceFabricIndex, peer, CryptoContext::SessionRole::kInitiator);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    SessionHolder bobToAliceSession;
    err = sessionManager.InjectPaseSessionWithTestKey(bobToAliceSession, 1,
                                                      fabricTable.FindFabricWithIndex(aliceFabricIndex)->GetNodeId(), 2,
                                                      bobFabricIndex, peer, CryptoContext::SessionRole::kResponder);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    callback.ReceiveHandlerCallCount = 0;

    PayloadHeader payloadHeader;
    EncryptedPacketBufferHandle preparedMessage;

    // Set the exchange ID for this header.
    payloadHeader.SetExchangeID(0);

    // Set the protocol ID and message type for this header.
    payloadHeader.SetMessageType(chip::Protocols::Echo::MsgType::EchoRequest);

    payloadHeader.SetInitiator(true);

    err = sessionManager.PrepareMessage(aliceToBobSession.Get().Value(), payloadHeader, std::move(buffer), preparedMessage);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get().Value(), preparedMessage);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    mContext.DrainAndServiceIO();
    EXPECT_EQ(callback.ReceiveHandlerCallCount, 1);

    // Now advance our message counter by 5.
    EncryptedPacketBufferHandle newMessage;
    for (size_t i = 0; i < 5; ++i)
    {
        buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, payload_len);
        EXPECT_FALSE(buffer.IsNull());

        err = sessionManager.PrepareMessage(aliceToBobSession.Get().Value(), payloadHeader, std::move(buffer), newMessage);
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get().Value(), newMessage);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    mContext.DrainAndServiceIO();
    EXPECT_EQ(callback.ReceiveHandlerCallCount, 2);

    // Now resend our original message.  It should be rejected as a duplicate.

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get().Value(), preparedMessage);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    mContext.DrainAndServiceIO();
    EXPECT_EQ(callback.ReceiveHandlerCallCount, 2);

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, SendPacketWithTooOldCounterTest)
{
    uint16_t payload_len = sizeof(PAYLOAD);

    TestSessMgrCallback callback;
    callback.LargeMessageSent = false;

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, payload_len);
    EXPECT_FALSE(buffer.IsNull());

    IPAddress addr;
    IPAddress::FromString("::1", addr);
    CHIP_ERROR err = CHIP_NO_ERROR;

    FabricTableHolder fabricTableHolder;
    SessionManager sessionManager;
    secure_channel::MessageCounterManager gMessageCounterManager;
    chip::TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    FabricTable & fabricTable    = fabricTableHolder.GetFabricTable();
    FabricIndex aliceFabricIndex = kUndefinedFabricIndex;
    FabricIndex bobFabricIndex   = kUndefinedFabricIndex;

    EXPECT_EQ(CHIP_NO_ERROR, fabricTableHolder.Init());
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.Init(&mContext.GetSystemLayer(), &mContext.GetTransportMgr(), &gMessageCounterManager, &deviceStorage,
                                  &fabricTableHolder.GetFabricTable(), sessionKeystore));
    sessionManager.SetMessageDelegate(&callback);

    Transport::PeerAddress peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));

    err =
        fabricTable.AddNewFabricForTestIgnoringCollisions(GetRootACertAsset().mCert, GetIAA1CertAsset().mCert,
                                                          GetNodeA1CertAsset().mCert, GetNodeA1CertAsset().mKey, &aliceFabricIndex);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    err = fabricTable.AddNewFabricForTestIgnoringCollisions(GetRootACertAsset().mCert, GetIAA1CertAsset().mCert,
                                                            GetNodeA2CertAsset().mCert, GetNodeA2CertAsset().mKey, &bobFabricIndex);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    SessionHolder aliceToBobSession;
    err = sessionManager.InjectPaseSessionWithTestKey(aliceToBobSession, 2,
                                                      fabricTable.FindFabricWithIndex(bobFabricIndex)->GetNodeId(), 1,
                                                      aliceFabricIndex, peer, CryptoContext::SessionRole::kInitiator);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    SessionHolder bobToAliceSession;
    err = sessionManager.InjectPaseSessionWithTestKey(bobToAliceSession, 1,
                                                      fabricTable.FindFabricWithIndex(aliceFabricIndex)->GetNodeId(), 2,
                                                      bobFabricIndex, peer, CryptoContext::SessionRole::kResponder);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    callback.ReceiveHandlerCallCount = 0;

    PayloadHeader payloadHeader;
    EncryptedPacketBufferHandle preparedMessage;

    // Set the exchange ID for this header.
    payloadHeader.SetExchangeID(0);

    // Set the protocol ID and message type for this header.
    payloadHeader.SetMessageType(chip::Protocols::Echo::MsgType::EchoRequest);

    payloadHeader.SetInitiator(true);

    err = sessionManager.PrepareMessage(aliceToBobSession.Get().Value(), payloadHeader, std::move(buffer), preparedMessage);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get().Value(), preparedMessage);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    mContext.DrainAndServiceIO();
    EXPECT_EQ(callback.ReceiveHandlerCallCount, 1);

    // Now advance our message counter by at least
    // CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE + 2, so preparedMessage will be
    // out of the window.
    EncryptedPacketBufferHandle newMessage;
    for (size_t i = 0; i < CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE + 2; ++i)
    {
        buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, payload_len);
        EXPECT_FALSE(buffer.IsNull());

        err = sessionManager.PrepareMessage(aliceToBobSession.Get().Value(), payloadHeader, std::move(buffer), newMessage);
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get().Value(), newMessage);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    mContext.DrainAndServiceIO();
    EXPECT_EQ(callback.ReceiveHandlerCallCount, 2);

    // Now resend our original message.  It should be rejected as a duplicate.

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get().Value(), preparedMessage);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    mContext.DrainAndServiceIO();
    EXPECT_EQ(callback.ReceiveHandlerCallCount, 2);

    sessionManager.Shutdown();
}

static void RandomSessionIdAllocatorOffset(SessionManager & sessionManager, int max)
{
    // Allocate + free a pseudo-random number of sessions to create a
    // pseudo-random offset in mNextSessionId.
    const int bound = rand() % max;
    for (int i = 0; i < bound; ++i)
    {
        auto handle = sessionManager.AllocateSession(
            Transport::SecureSession::Type::kPASE,
            ScopedNodeId(NodeIdFromPAKEKeyId(kDefaultCommissioningPasscodeId), kUndefinedFabricIndex));
        EXPECT_TRUE(handle.HasValue());
        handle.Value()->AsSecureSession()->MarkForEviction();
    }
}

TEST_F(TestSessionManager, SessionAllocationTest)
{
    FabricTableHolder fabricTableHolder;
    EXPECT_EQ(CHIP_NO_ERROR, fabricTableHolder.Init());

    secure_channel::MessageCounterManager messageCounterManager;
    TestPersistentStorageDelegate deviceStorage1, deviceStorage2;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    SessionManager sessionManager;

    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.Init(&mContext.GetSystemLayer(), &mContext.GetTransportMgr(), &messageCounterManager, &deviceStorage1,
                                  &fabricTableHolder.GetFabricTable(), sessionKeystore));

    // Allocate a session.
    uint16_t sessionId1;
    {
        auto handle = sessionManager.AllocateSession(
            Transport::SecureSession::Type::kPASE,
            ScopedNodeId(NodeIdFromPAKEKeyId(kDefaultCommissioningPasscodeId), kUndefinedFabricIndex));
        EXPECT_TRUE(handle.HasValue());
        SessionHolder session;
        session.GrabPairingSession(handle.Value());
        sessionId1 = session->AsSecureSession()->GetLocalSessionId();
    }

    // Verify that we increment session ID by 1 for each allocation, except for
    // the wraparound case where we skip session ID 0.
    auto prevSessionId = sessionId1;
    for (uint32_t i = 0; i < 10; ++i)
    {
        auto handle = sessionManager.AllocateSession(
            Transport::SecureSession::Type::kPASE,
            ScopedNodeId(NodeIdFromPAKEKeyId(kDefaultCommissioningPasscodeId), kUndefinedFabricIndex));
        if (!handle.HasValue())
        {
            break;
        }
        auto sessionId = handle.Value()->AsSecureSession()->GetLocalSessionId();
        EXPECT_TRUE(sessionId - prevSessionId == 1 || (sessionId == 1 && prevSessionId == 65535));
        EXPECT_NE(sessionId, 0);
        prevSessionId = sessionId;
    }

    // Reconstruct the Session Manager to reset state.
    sessionManager.Shutdown();
    sessionManager.~SessionManager();
    new (&sessionManager) SessionManager();
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.Init(&mContext.GetSystemLayer(), &mContext.GetTransportMgr(), &messageCounterManager, &deviceStorage2,
                                  &fabricTableHolder.GetFabricTable(), sessionKeystore));

    // Allocate a single session so we know what random id we are starting at.
    {
        auto handle = sessionManager.AllocateSession(
            Transport::SecureSession::Type::kPASE,
            ScopedNodeId(NodeIdFromPAKEKeyId(kDefaultCommissioningPasscodeId), kUndefinedFabricIndex));
        EXPECT_TRUE(handle.HasValue());
        prevSessionId = handle.Value()->AsSecureSession()->GetLocalSessionId();
        handle.Value()->AsSecureSession()->MarkForEviction();
    }

    // Verify that we increment session ID by 1 for each allocation (except for
    // the wraparound case where we skip session ID 0), even when allocated
    // sessions are immediately freed.
    for (uint32_t i = 0; i < UINT16_MAX + 10; ++i)
    {
        auto handle = sessionManager.AllocateSession(
            Transport::SecureSession::Type::kPASE,
            ScopedNodeId(NodeIdFromPAKEKeyId(kDefaultCommissioningPasscodeId), kUndefinedFabricIndex));
        EXPECT_TRUE(handle.HasValue());
        auto sessionId = handle.Value()->AsSecureSession()->GetLocalSessionId();
        EXPECT_TRUE(sessionId - prevSessionId == 1 || (sessionId == 1 && prevSessionId == 65535));
        EXPECT_NE(sessionId, 0);
        prevSessionId = sessionId;
        handle.Value()->AsSecureSession()->MarkForEviction();
    }

    // Verify that the allocator does not give colliding IDs.
    constexpr int collisionTestIterations = 1;
    for (int i = 0; i < collisionTestIterations; ++i)
    {
        // Allocate some session handles at pseudo-random offsets in the session
        // ID space.
        constexpr size_t numHandles = CHIP_CONFIG_SECURE_SESSION_POOL_SIZE - 1;
        Optional<SessionHandle> handles[numHandles];
        uint16_t sessionIds[numHandles];
        for (size_t h = 0; h < numHandles; ++h)
        {
            constexpr int maxOffset = 100;
            handles[h]              = sessionManager.AllocateSession(
                Transport::SecureSession::Type::kPASE,
                ScopedNodeId(NodeIdFromPAKEKeyId(kDefaultCommissioningPasscodeId), kUndefinedFabricIndex));
            EXPECT_TRUE(handles[h].HasValue());
            sessionIds[h] = handles[h].Value()->AsSecureSession()->GetLocalSessionId();
            RandomSessionIdAllocatorOffset(sessionManager, maxOffset);
        }

        // Verify that none collide each other.
        for (size_t h = 0; h < numHandles; ++h)
        {
            EXPECT_NE(sessionIds[h], sessionIds[(h + 1) % numHandles]);
        }

        // Allocate through the entire session ID space and verify that none of
        // these collide either.
        for (int j = 0; j < UINT16_MAX; ++j)
        {
            auto handle = sessionManager.AllocateSession(
                Transport::SecureSession::Type::kPASE,
                ScopedNodeId(NodeIdFromPAKEKeyId(kDefaultCommissioningPasscodeId), kUndefinedFabricIndex));
            EXPECT_TRUE(handle.HasValue());
            auto potentialCollision = handle.Value()->AsSecureSession()->GetLocalSessionId();
            for (uint16_t sessionId : sessionIds)
            {
                EXPECT_NE(potentialCollision, sessionId);
            }
            handle.Value()->AsSecureSession()->MarkForEviction();
        }

        // Free our allocated sessions.
        for (auto & handle : handles)
        {
            handle.Value()->AsSecureSession()->MarkForEviction();
        }
    }

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, SessionCounterExhaustedTest)
{
    IPAddress addr;
    IPAddress::FromString("::1", addr);
    CHIP_ERROR err = CHIP_NO_ERROR;

    FabricTableHolder fabricTableHolder;
    SessionManager sessionManager;
    secure_channel::MessageCounterManager gMessageCounterManager;
    chip::TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    FabricTable & fabricTable    = fabricTableHolder.GetFabricTable();
    FabricIndex aliceFabricIndex = kUndefinedFabricIndex;
    FabricIndex bobFabricIndex   = kUndefinedFabricIndex;

    EXPECT_EQ(CHIP_NO_ERROR, fabricTableHolder.Init());
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.Init(&mContext.GetSystemLayer(), &mContext.GetTransportMgr(), &gMessageCounterManager, &deviceStorage,
                                  &fabricTableHolder.GetFabricTable(), sessionKeystore));

    Transport::PeerAddress peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));

    err =
        fabricTable.AddNewFabricForTestIgnoringCollisions(GetRootACertAsset().mCert, GetIAA1CertAsset().mCert,
                                                          GetNodeA1CertAsset().mCert, GetNodeA1CertAsset().mKey, &aliceFabricIndex);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    err = fabricTable.AddNewFabricForTestIgnoringCollisions(GetRootACertAsset().mCert, GetIAA1CertAsset().mCert,
                                                            GetNodeA2CertAsset().mCert, GetNodeA2CertAsset().mKey, &bobFabricIndex);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    SessionHolder aliceToBobSession;
    err = sessionManager.InjectPaseSessionWithTestKey(aliceToBobSession, 2,
                                                      fabricTable.FindFabricWithIndex(bobFabricIndex)->GetNodeId(), 1,
                                                      aliceFabricIndex, peer, CryptoContext::SessionRole::kInitiator);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    SessionHolder bobToAliceSession;
    err = sessionManager.InjectPaseSessionWithTestKey(bobToAliceSession, 1,
                                                      fabricTable.FindFabricWithIndex(aliceFabricIndex)->GetNodeId(), 2,
                                                      bobFabricIndex, peer, CryptoContext::SessionRole::kResponder);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // ==== Set counter value to max ====
    LocalSessionMessageCounter & counter = static_cast<LocalSessionMessageCounter &>(
        aliceToBobSession.Get().Value()->AsSecureSession()->GetSessionMessageCounter().GetLocalMessageCounter());
    counter.TestSetCounter(LocalSessionMessageCounter::kMessageCounterMax - 1);

    // ==== Build a valid message with max counter value ====
    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    EXPECT_FALSE(buffer.IsNull());

    PayloadHeader payloadHeader;

    // Set the exchange ID for this header.
    payloadHeader.SetExchangeID(0);

    // Set the protocol ID and message type for this header.
    payloadHeader.SetMessageType(chip::Protocols::Echo::MsgType::EchoRequest);

    EncryptedPacketBufferHandle preparedMessage;
    err = sessionManager.PrepareMessage(aliceToBobSession.Get().Value(), payloadHeader, std::move(buffer), preparedMessage);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // ==== Build another message which will fail becuase message counter is exhausted ====
    chip::System::PacketBufferHandle buffer2 = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    EXPECT_FALSE(buffer2.IsNull());

    EncryptedPacketBufferHandle preparedMessage2;
    err = sessionManager.PrepareMessage(aliceToBobSession.Get().Value(), payloadHeader, std::move(buffer2), preparedMessage2);
    EXPECT_EQ(err, CHIP_ERROR_MESSAGE_COUNTER_EXHAUSTED);

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, SessionShiftingTest)
{
    IPAddress addr;
    IPAddress::FromString("::1", addr);

    NodeId aliceNodeId           = 0x11223344ull;
    NodeId bobNodeId             = 0x12344321ull;
    FabricIndex aliceFabricIndex = 1;
    FabricIndex bobFabricIndex   = 1;

    FabricTableHolder fabricTableHolder;
    secure_channel::MessageCounterManager messageCounterManager;
    TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    SessionManager sessionManager;

    EXPECT_EQ(CHIP_NO_ERROR, fabricTableHolder.Init());
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.Init(&mContext.GetSystemLayer(), &mContext.GetTransportMgr(), &messageCounterManager, &deviceStorage,
                                  &fabricTableHolder.GetFabricTable(), sessionKeystore));

    Transport::PeerAddress peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));

    SessionHolder aliceToBobSession;
    CHIP_ERROR err = sessionManager.InjectCaseSessionWithTestKey(aliceToBobSession, 2, 1, aliceNodeId, bobNodeId, aliceFabricIndex,
                                                                 peer, CryptoContext::SessionRole::kInitiator);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    class StickySessionDelegate : public SessionDelegate
    {
    public:
        NewSessionHandlingPolicy GetNewSessionHandlingPolicy() override { return NewSessionHandlingPolicy::kStayAtOldSession; }
        void OnSessionReleased() override {}
    } delegate;

    SessionHolderWithDelegate stickyAliceToBobSession(aliceToBobSession.Get().Value(), delegate);
    EXPECT_TRUE(aliceToBobSession.Contains(stickyAliceToBobSession.Get().Value()));

    SessionHolder bobToAliceSession;
    err = sessionManager.InjectCaseSessionWithTestKey(bobToAliceSession, 1, 2, bobNodeId, aliceNodeId, bobFabricIndex, peer,
                                                      CryptoContext::SessionRole::kResponder);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    SessionHolder newAliceToBobSession;
    err = sessionManager.InjectCaseSessionWithTestKey(newAliceToBobSession, 3, 4, aliceNodeId, bobNodeId, aliceFabricIndex, peer,
                                                      CryptoContext::SessionRole::kInitiator);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Here we got 3 sessions, and 4 holders:
    // 1. alice -> bob: aliceToBobSession, stickyAliceToBobSession
    // 2. alice <- bob: bobToAliceSession
    // 3. alice -> bob: newAliceToBobSession

    SecureSession * session1 = aliceToBobSession->AsSecureSession();
    SecureSession * session2 = bobToAliceSession->AsSecureSession();
    SecureSession * session3 = newAliceToBobSession->AsSecureSession();

    EXPECT_NE(session1, session3);
    EXPECT_EQ(stickyAliceToBobSession->AsSecureSession(), session1);

    // Now shift the 1st session to the 3rd one, after shifting, holders should be:
    // 1. alice -> bob: stickyAliceToBobSession
    // 2. alice <- bob: bobToAliceSession
    // 3. alice -> bob: aliceToBobSession, newAliceToBobSession
    sessionManager.GetSecureSessions().NewerSessionAvailable(newAliceToBobSession.Get().Value()->AsSecureSession());

    EXPECT_TRUE(aliceToBobSession);
    EXPECT_TRUE(stickyAliceToBobSession);
    EXPECT_TRUE(newAliceToBobSession);

    EXPECT_EQ(stickyAliceToBobSession->AsSecureSession(), session1);
    EXPECT_EQ(bobToAliceSession->AsSecureSession(), session2);
    EXPECT_EQ(aliceToBobSession->AsSecureSession(), session3);
    EXPECT_EQ(newAliceToBobSession->AsSecureSession(), session3);

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, TestFindSecureSessionForNode)
{
    IPAddress addr;
    IPAddress::FromString("::1", addr);

    NodeId aliceNodeId           = 0x11223344ull;
    NodeId bobNodeId             = 0x12344321ull;
    FabricIndex aliceFabricIndex = 1;

    FabricTableHolder fabricTableHolder;
    secure_channel::MessageCounterManager messageCounterManager;
    TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    SessionManager sessionManager;

    EXPECT_EQ(CHIP_NO_ERROR, fabricTableHolder.Init());
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.Init(&mContext.GetSystemLayer(), &mContext.GetTransportMgr(), &messageCounterManager, &deviceStorage,
                                  &fabricTableHolder.GetFabricTable(), sessionKeystore));

    Transport::PeerAddress peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));

    SessionHolder aliceToBobSession;
    CHIP_ERROR err = sessionManager.InjectCaseSessionWithTestKey(aliceToBobSession, 2, 1, aliceNodeId, bobNodeId, aliceFabricIndex,
                                                                 peer, CryptoContext::SessionRole::kInitiator);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    aliceToBobSession->AsSecureSession()->MarkActiveRx();

    SessionHolder newAliceToBobSession;
    err = sessionManager.InjectCaseSessionWithTestKey(newAliceToBobSession, 3, 4, aliceNodeId, bobNodeId, aliceFabricIndex, peer,
                                                      CryptoContext::SessionRole::kInitiator);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    while (System::SystemClock().GetMonotonicTimestamp() <= aliceToBobSession->AsSecureSession()->GetLastPeerActivityTime())
    {
        // Wait for the clock to advance so the new session is
        // more-recently-active.
    }
    newAliceToBobSession->AsSecureSession()->MarkActiveRx();

    auto foundSession = sessionManager.FindSecureSessionForNode(ScopedNodeId(bobNodeId, aliceFabricIndex),
                                                                MakeOptional(SecureSession::Type::kCASE));
    EXPECT_TRUE(foundSession.HasValue());
    EXPECT_TRUE(newAliceToBobSession.Contains(foundSession.Value()));
    EXPECT_FALSE(aliceToBobSession.Contains(foundSession.Value()));

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, TestMessageStats)
{
    uint16_t payload_len = sizeof(PAYLOAD);

    TestSessMgrCallback callback;
    callback.LargeMessageSent = false;

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, payload_len);
    EXPECT_FALSE(buffer.IsNull());

    IPAddress addr;
    IPAddress::FromString("::1", addr);
    CHIP_ERROR err = CHIP_NO_ERROR;

    FabricTableHolder fabricTableHolder;
    SessionManager sessionManager;
    secure_channel::MessageCounterManager gMessageCounterManager;
    chip::TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    FabricTable & fabricTable    = fabricTableHolder.GetFabricTable();
    FabricIndex aliceFabricIndex = kUndefinedFabricIndex;
    FabricIndex bobFabricIndex   = kUndefinedFabricIndex;

    EXPECT_EQ(CHIP_NO_ERROR, fabricTableHolder.Init());
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.Init(&mContext.GetSystemLayer(), &mContext.GetTransportMgr(), &gMessageCounterManager, &deviceStorage,
                                  &fabricTableHolder.GetFabricTable(), sessionKeystore));

    sessionManager.SetMessageDelegate(&callback);

    Transport::PeerAddress peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));

    err =
        fabricTable.AddNewFabricForTestIgnoringCollisions(GetRootACertAsset().mCert, GetIAA1CertAsset().mCert,
                                                          GetNodeA1CertAsset().mCert, GetNodeA1CertAsset().mKey, &aliceFabricIndex);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    err = fabricTable.AddNewFabricForTestIgnoringCollisions(GetRootACertAsset().mCert, GetIAA1CertAsset().mCert,
                                                            GetNodeA2CertAsset().mCert, GetNodeA2CertAsset().mKey, &bobFabricIndex);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    SessionHolder aliceToBobSession;
    err = sessionManager.InjectPaseSessionWithTestKey(aliceToBobSession, 2,
                                                      fabricTable.FindFabricWithIndex(bobFabricIndex)->GetNodeId(), 1,
                                                      aliceFabricIndex, peer, CryptoContext::SessionRole::kInitiator);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    SessionHolder bobToAliceSession;
    err = sessionManager.InjectPaseSessionWithTestKey(bobToAliceSession, 1,
                                                      fabricTable.FindFabricWithIndex(aliceFabricIndex)->GetNodeId(), 2,
                                                      bobFabricIndex, peer, CryptoContext::SessionRole::kResponder);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    callback.ReceiveHandlerCallCount = 0;

    // Ensure base case, counts start at 0
    MessageStats messageStatistics = sessionManager.GetMessageStats();
    EXPECT_EQ(messageStatistics.interactionModelMessagesSent, static_cast<uint32_t>(0));
    EXPECT_EQ(messageStatistics.interactionModelMessagesReceived, static_cast<uint32_t>(0));

    PayloadHeader payloadHeader;

    // Set the exchange ID for this header.
    payloadHeader.SetExchangeID(0);

    // Set the protocol ID and message type for this header.
    payloadHeader.SetMessageType(Protocols::InteractionModel::Id, 0);

    // Prepare message
    EncryptedPacketBufferHandle preparedMessage;
    err = sessionManager.PrepareMessage(aliceToBobSession.Get().Value(), payloadHeader, std::move(buffer), preparedMessage);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Send message to itself, this means a message is both sent and received
    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get().Value(), preparedMessage);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Verify final stats results
    mContext.DrainAndServiceIO();
    messageStatistics = sessionManager.GetMessageStats();
    EXPECT_EQ(messageStatistics.interactionModelMessagesSent, static_cast<uint32_t>(1));
    EXPECT_EQ(messageStatistics.interactionModelMessagesReceived, static_cast<uint32_t>(1));

    // Shutdown
    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, GetMaxAppMessageLenTest)
{
    FabricTableHolder fabricTableHolder;
    SessionManager sessionManager;
    secure_channel::MessageCounterManager gMessageCounterManager;
    chip::TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    FabricTable & fabricTable    = fabricTableHolder.GetFabricTable();
    FabricIndex aliceFabricIndex = kUndefinedFabricIndex;
    FabricIndex bobFabricIndex   = kUndefinedFabricIndex;

    IPAddress addr;
    IPAddress::FromString("::1", addr);

    EXPECT_EQ(CHIP_NO_ERROR, fabricTableHolder.Init());
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.Init(&mContext.GetSystemLayer(), &mContext.GetTransportMgr(), &gMessageCounterManager, &deviceStorage,
                                  &fabricTableHolder.GetFabricTable(), sessionKeystore));

    EXPECT_EQ(CHIP_NO_ERROR,
              fabricTable.AddNewFabricForTestIgnoringCollisions(GetRootACertAsset().mCert, GetIAA1CertAsset().mCert,
                                                                GetNodeA1CertAsset().mCert, GetNodeA1CertAsset().mKey,
                                                                &aliceFabricIndex));
    EXPECT_EQ(CHIP_NO_ERROR,
              fabricTable.AddNewFabricForTestIgnoringCollisions(GetRootACertAsset().mCert, GetIAA1CertAsset().mCert,
                                                                GetNodeA2CertAsset().mCert, GetNodeA2CertAsset().mKey,
                                                                &bobFabricIndex));

    // 1. Non-TCP (UDP) session should return kMaxAppMessageLen.
    Transport::PeerAddress udpPeer(Transport::PeerAddress::UDP(addr, CHIP_PORT));
    SessionHolder udpSession;
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.InjectPaseSessionWithTestKey(udpSession, 1,
                                                          fabricTable.FindFabricWithIndex(bobFabricIndex)->GetNodeId(), 2,
                                                          aliceFabricIndex, udpPeer, CryptoContext::SessionRole::kInitiator));
    EXPECT_FALSE(udpSession->AllowsLargePayload());
    EXPECT_EQ(udpSession->GetMaxAppMessageLen(), kMaxAppMessageLen);

    // 2. TCP session with unadvertised MaxTCPPayloadSize (0) should fall back to kLegacyDefaultMaxLargeAppMessageLen.
    Transport::PeerAddress tcpPeer(Transport::PeerAddress::TCP(addr, CHIP_PORT));
    SessionHolder tcpSessionLegacy;
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.InjectPaseSessionWithTestKey(tcpSessionLegacy, 3,
                                                          fabricTable.FindFabricWithIndex(bobFabricIndex)->GetNodeId(), 4,
                                                          aliceFabricIndex, tcpPeer, CryptoContext::SessionRole::kInitiator));
    EXPECT_TRUE(tcpSessionLegacy->AllowsLargePayload());
    EXPECT_EQ(tcpSessionLegacy->GetRemoteSessionParameters().GetMaxTCPPayloadSize(), 0u);
    EXPECT_EQ(tcpSessionLegacy->GetMaxAppMessageLen(), kLegacyDefaultMaxLargeAppMessageLen);

    // 3. TCP session with explicitly negotiated MaxTCPPayloadSize (e.g. 4096).
    SessionHolder tcpSessionNegotiated;
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.InjectPaseSessionWithTestKey(tcpSessionNegotiated, 5,
                                                          fabricTable.FindFabricWithIndex(bobFabricIndex)->GetNodeId(), 6,
                                                          aliceFabricIndex, tcpPeer, CryptoContext::SessionRole::kInitiator));
    SessionParameters paramsNegotiated;
    paramsNegotiated.SetMaxTCPPayloadSize(4096);
    tcpSessionNegotiated->AsSecureSession()->SetRemoteSessionParameters(paramsNegotiated);
    EXPECT_EQ(tcpSessionNegotiated->GetRemoteSessionParameters().GetMaxTCPPayloadSize(), 4096u);
    EXPECT_EQ(tcpSessionNegotiated->GetMaxAppMessageLen(), static_cast<size_t>(4096));

    // 4. TCP session with negotiated MaxTCPPayloadSize exceeding kMaxLargeAppMessageLen should be clamped.
    SessionHolder tcpSessionClamped;
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.InjectPaseSessionWithTestKey(tcpSessionClamped, 7,
                                                          fabricTable.FindFabricWithIndex(bobFabricIndex)->GetNodeId(), 8,
                                                          aliceFabricIndex, tcpPeer, CryptoContext::SessionRole::kInitiator));
    SessionParameters paramsLarge;
    paramsLarge.SetMaxTCPPayloadSize(kMaxLargeAppMessageLen + 1000);
    tcpSessionClamped->AsSecureSession()->SetRemoteSessionParameters(paramsLarge);
    EXPECT_EQ(tcpSessionClamped->GetMaxAppMessageLen(), kMaxLargeAppMessageLen);

    sessionManager.Shutdown();
}

#if INET_CONFIG_ENABLE_UDP_PORT_UNREACHABLE
namespace {

constexpr uint16_t kUnreachableTestPort1      = 5540;
constexpr uint16_t kUnreachableTestPort2      = 5541;
constexpr NodeId kUnreachableLocalNodeId      = 0xAAAA'AAAA'AAAA'0001ull;
constexpr NodeId kUnreachablePeerNodeId       = 0xBBBB'BBBB'BBBB'0001ull;
constexpr FabricIndex kUnreachableFabricIndex = 1;

Transport::PeerAddress MakeUdpPeer(const char * addrStr, uint16_t port)
{
    IPAddress addr;
    IPAddress::FromString(addrStr, addr);
    return Transport::PeerAddress::UDP(addr, port);
}

SecureSession * InjectActiveCaseSession(SessionManager & mgr, SessionHolder & holder, const Transport::PeerAddress & peer,
                                        uint16_t localSessionId, uint16_t peerSessionId = 1)
{
    CHIP_ERROR err =
        mgr.InjectCaseSessionWithTestKey(holder, localSessionId, peerSessionId, kUnreachableLocalNodeId, kUnreachablePeerNodeId,
                                         kUnreachableFabricIndex, peer, CryptoContext::SessionRole::kInitiator);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    holder->AsSecureSession()->MarkActive();
    return holder->AsSecureSession();
}

struct QuotedMessage
{
    uint8_t bytes[32] = {};
    uint16_t length   = 0;
    ByteSpan Span() const { return ByteSpan(bytes, length); }
};

QuotedMessage QuoteHeader(uint16_t sessionId, uint32_t counter,
                          Header::SessionType sessionType = Header::SessionType::kUnicastSession)
{
    QuotedMessage quoted;
    PacketHeader header;
    header.SetSessionType(sessionType).SetSessionId(sessionId).SetMessageCounter(counter);
    EXPECT_EQ(header.Encode(quoted.bytes, sizeof(quoted.bytes), &quoted.length), CHIP_NO_ERROR);
    return quoted;
}

uint32_t NextSendCounter(SecureSession & session)
{
    uint32_t counter = 0;
    EXPECT_EQ(session.GetSessionMessageCounter().GetLocalMessageCounter().AdvanceAndConsume(counter), CHIP_NO_ERROR);
    return counter;
}

QuotedMessage QuoteNextSend(SecureSession & session)
{
    return QuoteHeader(session.GetPeerSessionId(), NextSendCounter(session));
}

Inet::InterfaceId FirstPresentInterface()
{
    Inet::InterfaceIterator it;
    while (it.Next())
    {
        if (it.GetInterfaceId().IsPresent())
        {
            return it.GetInterfaceId();
        }
    }
    return Inet::InterfaceId::Null();
}

} // namespace

TEST_F(TestSessionManager, PortUnreachable_MarksMatchingSessionDefunct)
{
    FabricTableHolder fabricTableHolder;
    secure_channel::MessageCounterManager messageCounterManager;
    TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    SessionManager sessionManager;
    EXPECT_EQ(CHIP_NO_ERROR, fabricTableHolder.Init());
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.Init(&mContext.GetSystemLayer(), &mContext.GetTransportMgr(), &messageCounterManager, &deviceStorage,
                                  &fabricTableHolder.GetFabricTable(), sessionKeystore));

    Transport::PeerAddress peer = MakeUdpPeer("::1", kUnreachableTestPort1);
    SessionHolder holder;
    auto * session = InjectActiveCaseSession(sessionManager, holder, peer, /*lsid*/ 100);
    ASSERT_TRUE(session->IsActiveSession());

    sessionManager.OnPortUnreachable(peer, QuoteNextSend(*session).Span());

    EXPECT_TRUE(session->IsDefunct());
    EXPECT_FALSE(session->IsActiveSession());

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, PortUnreachable_DistinguishesByPort)
{
    FabricTableHolder fabricTableHolder;
    secure_channel::MessageCounterManager messageCounterManager;
    TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    SessionManager sessionManager;
    EXPECT_EQ(CHIP_NO_ERROR, fabricTableHolder.Init());
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.Init(&mContext.GetSystemLayer(), &mContext.GetTransportMgr(), &messageCounterManager, &deviceStorage,
                                  &fabricTableHolder.GetFabricTable(), sessionKeystore));

    Transport::PeerAddress peerA = MakeUdpPeer("::1", kUnreachableTestPort1);
    Transport::PeerAddress peerB = MakeUdpPeer("::1", kUnreachableTestPort2);
    SessionHolder holderA, holderB;
    auto * a               = InjectActiveCaseSession(sessionManager, holderA, peerA, /*lsid*/ 300);
    auto * b               = InjectActiveCaseSession(sessionManager, holderB, peerB, /*lsid*/ 301);
    const uint32_t counter = std::max(NextSendCounter(*a), NextSendCounter(*b));
    a->GetSessionMessageCounter().GetLocalMessageCounter().TestSetCounter(counter);
    b->GetSessionMessageCounter().GetLocalMessageCounter().TestSetCounter(counter);

    sessionManager.OnPortUnreachable(peerA, QuoteHeader(b->GetPeerSessionId(), counter).Span());

    EXPECT_TRUE(a->IsDefunct());
    EXPECT_TRUE(b->IsActiveSession()); // different port: untouched

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, PortUnreachable_DistinguishesByTransport)
{
    FabricTableHolder fabricTableHolder;
    secure_channel::MessageCounterManager messageCounterManager;
    TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    SessionManager sessionManager;
    EXPECT_EQ(CHIP_NO_ERROR, fabricTableHolder.Init());
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.Init(&mContext.GetSystemLayer(), &mContext.GetTransportMgr(), &messageCounterManager, &deviceStorage,
                                  &fabricTableHolder.GetFabricTable(), sessionKeystore));

    Transport::PeerAddress udpPeer = MakeUdpPeer("::1", kUnreachableTestPort1);
    Transport::PeerAddress tcpPeer = Transport::PeerAddress::TCP(udpPeer.GetIPAddress(), kUnreachableTestPort1);
    SessionHolder holder;
    auto * session = InjectActiveCaseSession(sessionManager, holder, tcpPeer, /*lsid*/ 310);

    sessionManager.OnPortUnreachable(udpPeer, QuoteNextSend(*session).Span());

    EXPECT_TRUE(session->IsActiveSession()); // different transport: untouched
    EXPECT_FALSE(session->IsDefunct());

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, PortUnreachable_RetiresOnlyTheQuotedSession)
{
    FabricTableHolder fabricTableHolder;
    secure_channel::MessageCounterManager messageCounterManager;
    TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    SessionManager sessionManager;
    EXPECT_EQ(CHIP_NO_ERROR, fabricTableHolder.Init());
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.Init(&mContext.GetSystemLayer(), &mContext.GetTransportMgr(), &messageCounterManager, &deviceStorage,
                                  &fabricTableHolder.GetFabricTable(), sessionKeystore));

    Transport::PeerAddress peer = MakeUdpPeer("::1", kUnreachableTestPort1);
    SessionHolder h1, h2;
    auto * s1 = InjectActiveCaseSession(sessionManager, h1, peer, /*lsid*/ 400);
    auto * s2 = InjectActiveCaseSession(sessionManager, h2, peer, /*lsid*/ 401);

    const QuotedMessage quoted = QuoteNextSend(*s1);
    PacketHeader header;
    uint16_t headerSize = 0;
    ASSERT_EQ(header.Decode(quoted.bytes, quoted.length, &headerSize), CHIP_NO_ERROR);
    s2->GetSessionMessageCounter().GetLocalMessageCounter().TestSetCounter(header.GetMessageCounter() + 1000);

    sessionManager.OnPortUnreachable(peer, quoted.Span());

    EXPECT_TRUE(s1->IsDefunct());
    EXPECT_TRUE(s2->IsActiveSession());

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, PortUnreachable_DistinguishesByPeerSessionId)
{
    FabricTableHolder fabricTableHolder;
    secure_channel::MessageCounterManager messageCounterManager;
    TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    SessionManager sessionManager;
    EXPECT_EQ(CHIP_NO_ERROR, fabricTableHolder.Init());
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.Init(&mContext.GetSystemLayer(), &mContext.GetTransportMgr(), &messageCounterManager, &deviceStorage,
                                  &fabricTableHolder.GetFabricTable(), sessionKeystore));

    Transport::PeerAddress peer = MakeUdpPeer("::1", kUnreachableTestPort1);
    SessionHolder h1, h2;
    auto * s1              = InjectActiveCaseSession(sessionManager, h1, peer, /*lsid*/ 410, /*peerSessionId*/ 7);
    auto * s2              = InjectActiveCaseSession(sessionManager, h2, peer, /*lsid*/ 411, /*peerSessionId*/ 8);
    const uint32_t counter = std::max(NextSendCounter(*s1), NextSendCounter(*s2));
    s1->GetSessionMessageCounter().GetLocalMessageCounter().TestSetCounter(counter);
    s2->GetSessionMessageCounter().GetLocalMessageCounter().TestSetCounter(counter);

    sessionManager.OnPortUnreachable(peer, QuoteHeader(s2->GetPeerSessionId(), counter).Span());

    EXPECT_TRUE(s1->IsActiveSession());
    EXPECT_TRUE(s2->IsDefunct());

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, PortUnreachable_DistinguishesByAddress)
{
    FabricTableHolder fabricTableHolder;
    secure_channel::MessageCounterManager messageCounterManager;
    TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    SessionManager sessionManager;
    EXPECT_EQ(CHIP_NO_ERROR, fabricTableHolder.Init());
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.Init(&mContext.GetSystemLayer(), &mContext.GetTransportMgr(), &messageCounterManager, &deviceStorage,
                                  &fabricTableHolder.GetFabricTable(), sessionKeystore));

    Transport::PeerAddress sessionPeer = MakeUdpPeer("::1", kUnreachableTestPort1);
    Transport::PeerAddress otherPeer   = MakeUdpPeer("::2", kUnreachableTestPort1);
    SessionHolder holder;
    auto * session = InjectActiveCaseSession(sessionManager, holder, sessionPeer, /*lsid*/ 500);

    sessionManager.OnPortUnreachable(otherPeer, QuoteNextSend(*session).Span());

    EXPECT_TRUE(session->IsActiveSession()); // different address: untouched
    EXPECT_FALSE(session->IsDefunct());

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, PortUnreachable_IgnoresAnIdleSession)
{
    FabricTableHolder fabricTableHolder;
    secure_channel::MessageCounterManager messageCounterManager;
    TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    SessionManager sessionManager;
    EXPECT_EQ(CHIP_NO_ERROR, fabricTableHolder.Init());
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.Init(&mContext.GetSystemLayer(), &mContext.GetTransportMgr(), &messageCounterManager, &deviceStorage,
                                  &fabricTableHolder.GetFabricTable(), sessionKeystore));

    System::Clock::Internal::RAIIMockClock clock;
    Transport::PeerAddress peer = MakeUdpPeer("::1", kUnreachableTestPort1);
    SessionHolder holder;
    auto * session = InjectActiveCaseSession(sessionManager, holder, peer, /*lsid*/ 960);
    session->SetRemoteSessionParameters(
        ReliableMessageProtocolConfig({ System::Clock::Timestamp(300), System::Clock::Timestamp(300) }));
    const QuotedMessage quoted = QuoteNextSend(*session);

    clock.AdvanceMonotonic(session->GetMRPBaseTimeout() + System::Clock::Milliseconds64(1));
    sessionManager.OnPortUnreachable(peer, quoted.Span());
    EXPECT_TRUE(session->IsActiveSession());

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, PortUnreachable_AcceptsReportAtMrpIntervalEdge)
{
    FabricTableHolder fabricTableHolder;
    secure_channel::MessageCounterManager messageCounterManager;
    TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    SessionManager sessionManager;
    EXPECT_EQ(CHIP_NO_ERROR, fabricTableHolder.Init());
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.Init(&mContext.GetSystemLayer(), &mContext.GetTransportMgr(), &messageCounterManager, &deviceStorage,
                                  &fabricTableHolder.GetFabricTable(), sessionKeystore));

    System::Clock::Internal::RAIIMockClock clock;
    Transport::PeerAddress peer = MakeUdpPeer("::1", kUnreachableTestPort1);
    SessionHolder holder;
    auto * session = InjectActiveCaseSession(sessionManager, holder, peer, /*lsid*/ 961);
    session->SetRemoteSessionParameters(
        ReliableMessageProtocolConfig({ System::Clock::Timestamp(300), System::Clock::Timestamp(300) }));
    const QuotedMessage quoted = QuoteNextSend(*session);

    clock.AdvanceMonotonic(session->GetMRPBaseTimeout());
    sessionManager.OnPortUnreachable(peer, quoted.Span());
    EXPECT_TRUE(session->IsDefunct());

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, PortUnreachable_LeavesPaseAlone)
{
    FabricTableHolder fabricTableHolder;
    secure_channel::MessageCounterManager messageCounterManager;
    TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    SessionManager sessionManager;
    EXPECT_EQ(CHIP_NO_ERROR, fabricTableHolder.Init());
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.Init(&mContext.GetSystemLayer(), &mContext.GetTransportMgr(), &messageCounterManager, &deviceStorage,
                                  &fabricTableHolder.GetFabricTable(), sessionKeystore));

    Transport::PeerAddress peer = MakeUdpPeer("::1", kUnreachableTestPort1);
    SessionHolder holder;
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.InjectPaseSessionWithTestKey(holder, /*lsid*/ 950, kUnreachablePeerNodeId, /*peerSessionId*/ 1,
                                                          kUnreachableFabricIndex, peer, CryptoContext::SessionRole::kInitiator));
    auto * session = holder->AsSecureSession();
    ASSERT_TRUE(session->IsActiveSession());

    sessionManager.OnPortUnreachable(peer, QuoteNextSend(*session).Span());

    EXPECT_TRUE(session->IsActiveSession());

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, PortUnreachable_MatchesAcrossInterfaces)
{
    FabricTableHolder fabricTableHolder;
    secure_channel::MessageCounterManager messageCounterManager;
    TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    SessionManager sessionManager;
    EXPECT_EQ(CHIP_NO_ERROR, fabricTableHolder.Init());
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.Init(&mContext.GetSystemLayer(), &mContext.GetTransportMgr(), &messageCounterManager, &deviceStorage,
                                  &fabricTableHolder.GetFabricTable(), sessionKeystore));

    IPAddress addr;
    IPAddress::FromString("fe80::1", addr);
    Inet::InterfaceId ifid = FirstPresentInterface();
    ASSERT_TRUE(ifid.IsPresent());
    Transport::PeerAddress sessionPeer = Transport::PeerAddress::UDP(addr, kUnreachableTestPort1, ifid);
    Transport::PeerAddress icmpPeer    = Transport::PeerAddress::UDP(addr, kUnreachableTestPort1);
    ASSERT_NE(sessionPeer, icmpPeer); // they differ only by interface

    SessionHolder holder;
    auto * session = InjectActiveCaseSession(sessionManager, holder, sessionPeer, /*lsid*/ 900);
    ASSERT_TRUE(session->IsActiveSession());

    sessionManager.OnPortUnreachable(icmpPeer, QuoteNextSend(*session).Span());

    EXPECT_TRUE(session->IsDefunct());

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, PortUnreachable_IgnoresEstablishingSession)
{
    FabricTableHolder fabricTableHolder;
    secure_channel::MessageCounterManager messageCounterManager;
    TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    SessionManager sessionManager;
    EXPECT_EQ(CHIP_NO_ERROR, fabricTableHolder.Init());
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.Init(&mContext.GetSystemLayer(), &mContext.GetTransportMgr(), &messageCounterManager, &deviceStorage,
                                  &fabricTableHolder.GetFabricTable(), sessionKeystore));

    Transport::PeerAddress peer     = MakeUdpPeer("::1", kUnreachableTestPort1);
    Optional<SessionHandle> pending = sessionManager.AllocateSession(Transport::SecureSession::Type::kCASE,
                                                                     ScopedNodeId(kUnreachablePeerNodeId, kUnreachableFabricIndex));
    ASSERT_TRUE(pending.HasValue());
    auto * session = pending.Value()->AsSecureSession();
    session->SetPeerAddress(peer);
    session->MarkActive();
    ASSERT_TRUE(session->IsEstablishing());

    sessionManager.OnPortUnreachable(peer, QuoteNextSend(*session).Span());

    EXPECT_TRUE(session->IsEstablishing());
    EXPECT_FALSE(session->IsDefunct());

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, PortUnreachable_IgnoresWrongQuotedSessionId)
{
    FabricTableHolder fabricTableHolder;
    secure_channel::MessageCounterManager messageCounterManager;
    TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    SessionManager sessionManager;
    EXPECT_EQ(CHIP_NO_ERROR, fabricTableHolder.Init());
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.Init(&mContext.GetSystemLayer(), &mContext.GetTransportMgr(), &messageCounterManager, &deviceStorage,
                                  &fabricTableHolder.GetFabricTable(), sessionKeystore));

    Transport::PeerAddress peer = MakeUdpPeer("::1", kUnreachableTestPort1);
    SessionHolder holder;
    auto * session = InjectActiveCaseSession(sessionManager, holder, peer, /*lsid*/ 1100);
    sessionManager.OnPortUnreachable(
        peer, QuoteHeader(static_cast<uint16_t>(session->GetPeerSessionId() + 1), NextSendCounter(*session)).Span());
    EXPECT_TRUE(session->IsActiveSession());

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, PortUnreachable_AcceptsCounterAtWindowEdge)
{
    FabricTableHolder fabricTableHolder;
    secure_channel::MessageCounterManager messageCounterManager;
    TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    SessionManager sessionManager;
    EXPECT_EQ(CHIP_NO_ERROR, fabricTableHolder.Init());
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.Init(&mContext.GetSystemLayer(), &mContext.GetTransportMgr(), &messageCounterManager, &deviceStorage,
                                  &fabricTableHolder.GetFabricTable(), sessionKeystore));

    Transport::PeerAddress peer = MakeUdpPeer("::1", kUnreachableTestPort1);
    SessionHolder holder;
    auto * session      = InjectActiveCaseSession(sessionManager, holder, peer, /*lsid*/ 1101);
    const uint32_t sent = NextSendCounter(*session);
    for (uint32_t i = 0; i < CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE - 1; i++)
    {
        NextSendCounter(*session);
    }
    sessionManager.OnPortUnreachable(peer, QuoteHeader(session->GetPeerSessionId(), sent).Span());
    EXPECT_TRUE(session->IsDefunct());

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, PortUnreachable_IgnoresCounterBeyondWindow)
{
    FabricTableHolder fabricTableHolder;
    secure_channel::MessageCounterManager messageCounterManager;
    TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    SessionManager sessionManager;
    EXPECT_EQ(CHIP_NO_ERROR, fabricTableHolder.Init());
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.Init(&mContext.GetSystemLayer(), &mContext.GetTransportMgr(), &messageCounterManager, &deviceStorage,
                                  &fabricTableHolder.GetFabricTable(), sessionKeystore));

    Transport::PeerAddress peer = MakeUdpPeer("::1", kUnreachableTestPort1);
    SessionHolder holder;
    auto * session      = InjectActiveCaseSession(sessionManager, holder, peer, /*lsid*/ 1102);
    const uint32_t sent = NextSendCounter(*session);
    for (uint32_t i = 0; i < CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE; i++)
    {
        NextSendCounter(*session);
    }
    sessionManager.OnPortUnreachable(peer, QuoteHeader(session->GetPeerSessionId(), sent).Span());
    EXPECT_TRUE(session->IsActiveSession());

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, PortUnreachable_IgnoresUnsentCounter)
{
    FabricTableHolder fabricTableHolder;
    secure_channel::MessageCounterManager messageCounterManager;
    TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    SessionManager sessionManager;
    EXPECT_EQ(CHIP_NO_ERROR, fabricTableHolder.Init());
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.Init(&mContext.GetSystemLayer(), &mContext.GetTransportMgr(), &messageCounterManager, &deviceStorage,
                                  &fabricTableHolder.GetFabricTable(), sessionKeystore));

    Transport::PeerAddress peer = MakeUdpPeer("::1", kUnreachableTestPort1);
    SessionHolder holder;
    auto * session        = InjectActiveCaseSession(sessionManager, holder, peer, /*lsid*/ 1103);
    const uint32_t unsent = NextSendCounter(*session) + 1;
    sessionManager.OnPortUnreachable(peer, QuoteHeader(session->GetPeerSessionId(), unsent).Span());
    EXPECT_TRUE(session->IsActiveSession());

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, PortUnreachable_IgnoresCounterBeforeFirstSend)
{
    FabricTableHolder fabricTableHolder;
    secure_channel::MessageCounterManager messageCounterManager;
    TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    SessionManager sessionManager;
    EXPECT_EQ(CHIP_NO_ERROR, fabricTableHolder.Init());
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.Init(&mContext.GetSystemLayer(), &mContext.GetTransportMgr(), &messageCounterManager, &deviceStorage,
                                  &fabricTableHolder.GetFabricTable(), sessionKeystore));

    Transport::PeerAddress peer = MakeUdpPeer("::1", kUnreachableTestPort1);
    SessionHolder holder;
    auto * session       = InjectActiveCaseSession(sessionManager, holder, peer, /*lsid*/ 1106);
    const uint32_t first = NextSendCounter(*session);
    for (uint32_t below = 1; below < CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE && below < first; below++)
    {
        sessionManager.OnPortUnreachable(peer, QuoteHeader(session->GetPeerSessionId(), first - below).Span());
        EXPECT_TRUE(session->IsActiveSession());
    }

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, PortUnreachable_IgnoresUnsentCounterAcrossWrap)
{
    FabricTableHolder fabricTableHolder;
    secure_channel::MessageCounterManager messageCounterManager;
    TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    SessionManager sessionManager;
    EXPECT_EQ(CHIP_NO_ERROR, fabricTableHolder.Init());
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.Init(&mContext.GetSystemLayer(), &mContext.GetTransportMgr(), &messageCounterManager, &deviceStorage,
                                  &fabricTableHolder.GetFabricTable(), sessionKeystore));

    Transport::PeerAddress peer = MakeUdpPeer("::1", kUnreachableTestPort1);
    SessionHolder holder;
    auto * session = InjectActiveCaseSession(sessionManager, holder, peer, /*lsid*/ 1107);
    session->GetSessionMessageCounter().GetLocalMessageCounter().TestSetCounter(0);
    sessionManager.OnPortUnreachable(
        peer, QuoteHeader(session->GetPeerSessionId(), LocalSessionMessageCounter::kMessageCounterMax).Span());
    EXPECT_TRUE(session->IsActiveSession());

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, PortUnreachable_IgnoresGroupSessionQuote)
{
    FabricTableHolder fabricTableHolder;
    secure_channel::MessageCounterManager messageCounterManager;
    TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    SessionManager sessionManager;
    EXPECT_EQ(CHIP_NO_ERROR, fabricTableHolder.Init());
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.Init(&mContext.GetSystemLayer(), &mContext.GetTransportMgr(), &messageCounterManager, &deviceStorage,
                                  &fabricTableHolder.GetFabricTable(), sessionKeystore));

    Transport::PeerAddress peer = MakeUdpPeer("::1", kUnreachableTestPort1);
    SessionHolder holder;
    auto * session = InjectActiveCaseSession(sessionManager, holder, peer, /*lsid*/ 1104);
    sessionManager.OnPortUnreachable(
        peer, QuoteHeader(session->GetPeerSessionId(), NextSendCounter(*session), Header::SessionType::kGroupSession).Span());
    EXPECT_TRUE(session->IsActiveSession());

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, PortUnreachable_IgnoresUndecodableQuote)
{
    FabricTableHolder fabricTableHolder;
    secure_channel::MessageCounterManager messageCounterManager;
    TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    SessionManager sessionManager;
    EXPECT_EQ(CHIP_NO_ERROR, fabricTableHolder.Init());
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.Init(&mContext.GetSystemLayer(), &mContext.GetTransportMgr(), &messageCounterManager, &deviceStorage,
                                  &fabricTableHolder.GetFabricTable(), sessionKeystore));

    Transport::PeerAddress peer = MakeUdpPeer("::1", kUnreachableTestPort1);
    SessionHolder holder;
    auto * session = InjectActiveCaseSession(sessionManager, holder, peer, /*lsid*/ 1105);
    PacketHeader header;
    header.SetSessionType(Header::SessionType::kUnicastSession)
        .SetSessionId(session->GetPeerSessionId())
        .SetMessageCounter(NextSendCounter(*session))
        .SetSourceNodeId(kUnreachablePeerNodeId);
    uint8_t encoded[32];
    uint16_t encodedLength = 0;
    ASSERT_EQ(header.Encode(encoded, sizeof(encoded), &encodedLength), CHIP_NO_ERROR);
    sessionManager.OnPortUnreachable(peer, ByteSpan(encoded, encodedLength - sizeof(NodeId)));
    EXPECT_TRUE(session->IsActiveSession());

    sessionManager.Shutdown();
}

#endif // INET_CONFIG_ENABLE_UDP_PORT_UNREACHABLE

} // namespace
