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

namespace {

constexpr uint16_t kHceTestPort1      = 5540;
constexpr uint16_t kHceTestPort2      = 5541;
constexpr NodeId kHceLocalNodeId      = 0xAAAA'AAAA'AAAA'0001ull;
constexpr NodeId kHcePeerNodeId       = 0xBBBB'BBBB'BBBB'0001ull;
constexpr FabricIndex kHceFabricIndex = 1;

Transport::PeerAddress MakeUdpPeer(const char * addrStr, uint16_t port)
{
    IPAddress addr;
    IPAddress::FromString(addrStr, addr);
    return Transport::PeerAddress::UDP(addr, port);
}

SecureSession * InjectActiveCaseSession(SessionManager & mgr, SessionHolder & holder, const Transport::PeerAddress & peer,
                                        uint16_t localSessionId)
{
    CHIP_ERROR err = mgr.InjectCaseSessionWithTestKey(holder, localSessionId, /*peerSessionId*/ 1, kHceLocalNodeId, kHcePeerNodeId,
                                                      kHceFabricIndex, peer, CryptoContext::SessionRole::kInitiator);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Stand in for the send path, which is what makes a session eligible to be retired.
    holder->AsSecureSession()->MarkActiveTx();
    return holder->AsSecureSession();
}

// A non-null InterfaceId has no portable literal form: an index under sockets, a netif pointer
// under LwIP. Which interface it is does not matter here.
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

TEST_F(TestSessionManager, HandleConnectionExpired_MarksMatchingSessionDefunct)
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

    Transport::PeerAddress peer = MakeUdpPeer("::1", kHceTestPort1);
    SessionHolder holder;
    auto * session = InjectActiveCaseSession(sessionManager, holder, peer, /*lsid*/ 100);
    ASSERT_TRUE(session->IsActiveSession());

    sessionManager.HandleConnectionExpired(peer);

    EXPECT_TRUE(session->IsDefunct());
    EXPECT_FALSE(session->IsActiveSession());

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, HandleConnectionExpired_Idempotent)
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

    Transport::PeerAddress peer = MakeUdpPeer("::1", kHceTestPort1);
    SessionHolder holder;
    auto * session = InjectActiveCaseSession(sessionManager, holder, peer, /*lsid*/ 200);

    sessionManager.HandleConnectionExpired(peer);
    ASSERT_TRUE(session->IsDefunct());

    sessionManager.HandleConnectionExpired(peer);
    EXPECT_TRUE(session->IsDefunct());

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, HandleConnectionExpired_DistinguishesByPort)
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

    Transport::PeerAddress peerA = MakeUdpPeer("::1", kHceTestPort1);
    Transport::PeerAddress peerB = MakeUdpPeer("::1", kHceTestPort2);
    SessionHolder holderA, holderB;
    auto * a = InjectActiveCaseSession(sessionManager, holderA, peerA, /*lsid*/ 300);
    auto * b = InjectActiveCaseSession(sessionManager, holderB, peerB, /*lsid*/ 301);

    sessionManager.HandleConnectionExpired(peerA);

    EXPECT_TRUE(a->IsDefunct());
    EXPECT_TRUE(b->IsActiveSession()); // different port — untouched

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, HandleConnectionExpired_AllSessionsToSamePeer)
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

    Transport::PeerAddress peer = MakeUdpPeer("::1", kHceTestPort1);
    SessionHolder h1, h2;
    auto * s1 = InjectActiveCaseSession(sessionManager, h1, peer, /*lsid*/ 400);
    auto * s2 = InjectActiveCaseSession(sessionManager, h2, peer, /*lsid*/ 401);

    sessionManager.HandleConnectionExpired(peer);

    EXPECT_TRUE(s1->IsDefunct());
    EXPECT_TRUE(s2->IsDefunct());

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, HandleConnectionExpired_DistinguishesByAddress)
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

    Transport::PeerAddress sessionPeer = MakeUdpPeer("::1", kHceTestPort1);
    Transport::PeerAddress otherPeer   = MakeUdpPeer("::2", kHceTestPort1);
    SessionHolder holder;
    auto * session = InjectActiveCaseSession(sessionManager, holder, sessionPeer, /*lsid*/ 500);

    sessionManager.HandleConnectionExpired(otherPeer);

    EXPECT_TRUE(session->IsActiveSession()); // different address — untouched
    EXPECT_FALSE(session->IsDefunct());

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, HandleConnectionExpired_StaleAfterRecovery)
{
    // A late error for the old port must not disturb the session on the new one.
    FabricTableHolder fabricTableHolder;
    secure_channel::MessageCounterManager messageCounterManager;
    TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    SessionManager sessionManager;
    EXPECT_EQ(CHIP_NO_ERROR, fabricTableHolder.Init());
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.Init(&mContext.GetSystemLayer(), &mContext.GetTransportMgr(), &messageCounterManager, &deviceStorage,
                                  &fabricTableHolder.GetFabricTable(), sessionKeystore));

    Transport::PeerAddress oldPeer = MakeUdpPeer("::1", kHceTestPort1);
    Transport::PeerAddress newPeer = MakeUdpPeer("::1", kHceTestPort2);
    SessionHolder oldHolder, newHolder;
    auto * oldSession = InjectActiveCaseSession(sessionManager, oldHolder, oldPeer, /*lsid*/ 600);

    sessionManager.HandleConnectionExpired(oldPeer);
    ASSERT_TRUE(oldSession->IsDefunct());

    auto * newSession = InjectActiveCaseSession(sessionManager, newHolder, newPeer, /*lsid*/ 601);
    ASSERT_TRUE(newSession->IsActiveSession());

    sessionManager.HandleConnectionExpired(oldPeer);

    EXPECT_TRUE(newSession->IsActiveSession());
    EXPECT_TRUE(oldSession->IsDefunct());

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, HandleConnectionExpired_DoesNotClaimAnAlreadyDefunctSession)
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

    Transport::PeerAddress peer = MakeUdpPeer("::1", kHceTestPort1);
    SessionHolder holder;
    auto * session = InjectActiveCaseSession(sessionManager, holder, peer, /*lsid*/ 700);

    session->MarkAsDefunct();
    ASSERT_TRUE(session->IsDefunct());
    ASSERT_FALSE(session->PeerReportedUnreachable());

    sessionManager.HandleConnectionExpired(peer);

    // Defunct, but not attributed to the peer, so its exchanges keep their retry budget.
    EXPECT_TRUE(session->IsDefunct());
    EXPECT_FALSE(session->PeerReportedUnreachable());

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, HandleConnectionExpired_IgnoresAnIdleSession)
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

    Transport::PeerAddress peer = MakeUdpPeer("::1", kHceTestPort1);
    SessionHolder holder;
    auto * session = InjectActiveCaseSession(sessionManager, holder, peer, /*lsid*/ 960);

    // Collapse the MRP window, then let the clock pass it.
    session->SetRemoteSessionParameters(
        ReliableMessageProtocolConfig({ System::Clock::Timestamp(0), System::Clock::Timestamp(0) }));
    const System::Clock::Timestamp sent = session->GetLastTxTime();
    while (System::SystemClock().GetMonotonicTimestamp() <= sent)
    {
    }

    sessionManager.HandleConnectionExpired(peer);

    // Nothing in flight, so the error cannot be answering us.
    EXPECT_TRUE(session->IsActiveSession());
    EXPECT_FALSE(session->PeerReportedUnreachable());

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, HandleConnectionExpired_InboundTrafficDoesNotCount)
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

    Transport::PeerAddress peer = MakeUdpPeer("::1", kHceTestPort1);
    SessionHolder holder;
    auto * session = InjectActiveCaseSession(sessionManager, holder, peer, /*lsid*/ 970);

    session->SetRemoteSessionParameters(
        ReliableMessageProtocolConfig({ System::Clock::Timestamp(0), System::Clock::Timestamp(0) }));
    const System::Clock::Timestamp sent = session->GetLastTxTime();
    while (System::SystemClock().GetMonotonicTimestamp() <= sent)
    {
    }
    // Receiving refreshes MarkActive, so a check against general activity would wrongly requalify
    // this session even though nothing was sent.
    session->MarkActiveRx();

    sessionManager.HandleConnectionExpired(peer);

    EXPECT_TRUE(session->IsActiveSession());
    EXPECT_FALSE(session->PeerReportedUnreachable());

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, HandleConnectionExpired_LeavesPaseAlone)
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

    Transport::PeerAddress peer = MakeUdpPeer("::1", kHceTestPort1);
    SessionHolder holder;
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.InjectPaseSessionWithTestKey(holder, /*lsid*/ 950, kHcePeerNodeId, /*peerSessionId*/ 1,
                                                          kHceFabricIndex, peer, CryptoContext::SessionRole::kInitiator));
    auto * session = holder->AsSecureSession();
    ASSERT_TRUE(session->IsActiveSession());

    sessionManager.HandleConnectionExpired(peer);

    // PASE cannot re-establish itself, so an unauthenticated error must not abort commissioning.
    EXPECT_TRUE(session->IsActiveSession());
    EXPECT_FALSE(session->PeerReportedUnreachable());

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, OnConnectionExpired_RoutesToHandleConnectionExpired)
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

    Transport::PeerAddress peer = MakeUdpPeer("::1", kHceTestPort1);
    SessionHolder holder;
    auto * session = InjectActiveCaseSession(sessionManager, holder, peer, /*lsid*/ 800);
    ASSERT_TRUE(session->IsActiveSession());

    // Through the delegate base, the path Transport::UDP::OnUdpError takes.
    static_cast<TransportMgrDelegate &>(sessionManager).OnConnectionExpired(peer);

    EXPECT_TRUE(session->IsDefunct());

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, HandleConnectionExpired_IgnoresInterfaceMismatch)
{
    // Whole-PeerAddress equality includes the interface, which would never match a link-local peer.
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
    Transport::PeerAddress sessionPeer = Transport::PeerAddress::UDP(addr, kHceTestPort1, ifid);
    Transport::PeerAddress icmpPeer    = Transport::PeerAddress::UDP(addr, kHceTestPort1);
    ASSERT_NE(sessionPeer, icmpPeer); // they differ only by interface

    SessionHolder holder;
    auto * session = InjectActiveCaseSession(sessionManager, holder, sessionPeer, /*lsid*/ 900);
    ASSERT_TRUE(session->IsActiveSession());

    sessionManager.HandleConnectionExpired(icmpPeer);

    EXPECT_TRUE(session->IsDefunct());

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, HandleConnectionExpired_IgnoresNeverTransmittedSession)
{
    // The monotonic clock is time since boot, so a zero TX timestamp is inside the correlation
    // window early in uptime. Such a session has sent nothing that could have drawn the error.
    FabricTableHolder fabricTableHolder;
    secure_channel::MessageCounterManager messageCounterManager;
    TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    SessionManager sessionManager;
    EXPECT_EQ(CHIP_NO_ERROR, fabricTableHolder.Init());
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.Init(&mContext.GetSystemLayer(), &mContext.GetTransportMgr(), &messageCounterManager, &deviceStorage,
                                  &fabricTableHolder.GetFabricTable(), sessionKeystore));

    Transport::PeerAddress peer = MakeUdpPeer("::1", kHceTestPort1);
    SessionHolder holder;
    // Early uptime, where a zero TX timestamp still falls inside the correlation window. Without
    // the explicit zero check this session is retired; on a host whose uptime already exceeds the
    // window the bug is invisible, so the clock has to be pinned for the test to mean anything.
    System::Clock::Internal::RAIIMockClock clock;
    clock.SetMonotonic(System::Clock::Milliseconds64(100));
    // Deliberately not InjectActiveCaseSession: no MarkActiveTx, so mLastTxTime stays zero.
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.InjectCaseSessionWithTestKey(holder, /*lsid*/ 1000, /*peerSessionId*/ 1, kHceLocalNodeId,
                                                          kHcePeerNodeId, kHceFabricIndex, peer,
                                                          CryptoContext::SessionRole::kInitiator));
    auto * session = holder->AsSecureSession();
    ASSERT_TRUE(session->IsActiveSession());
    ASSERT_EQ(session->GetLastTxTime(), System::Clock::kZero);
    ASSERT_LE(System::SystemClock().GetMonotonicTimestamp(), session->GetMRPBaseTimeout());

    sessionManager.HandleConnectionExpired(peer);

    EXPECT_FALSE(session->IsDefunct());
    EXPECT_TRUE(session->IsActiveSession());
    EXPECT_FALSE(session->PeerReportedUnreachable());

    sessionManager.Shutdown();
}

TEST_F(TestSessionManager, HandleConnectionExpired_IgnoresEstablishingSession)
{
    // A session still being established has no confirmed peer to retire, and tearing it down here
    // would abort the very handshake that would recover it.
    FabricTableHolder fabricTableHolder;
    secure_channel::MessageCounterManager messageCounterManager;
    TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    SessionManager sessionManager;
    EXPECT_EQ(CHIP_NO_ERROR, fabricTableHolder.Init());
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.Init(&mContext.GetSystemLayer(), &mContext.GetTransportMgr(), &messageCounterManager, &deviceStorage,
                                  &fabricTableHolder.GetFabricTable(), sessionKeystore));

    Transport::PeerAddress peer = MakeUdpPeer("::1", kHceTestPort1);
    Optional<SessionHandle> pending =
        sessionManager.AllocateSession(Transport::SecureSession::Type::kCASE, ScopedNodeId(kHcePeerNodeId, kHceFabricIndex));
    ASSERT_TRUE(pending.HasValue());
    auto * session = pending.Value()->AsSecureSession();
    session->SetPeerAddress(peer);
    // Fresh TX timestamp, so only the state guard can exclude this session.
    session->MarkActiveTx();
    ASSERT_TRUE(session->IsEstablishing());

    sessionManager.HandleConnectionExpired(peer);

    EXPECT_TRUE(session->IsEstablishing());
    EXPECT_FALSE(session->IsDefunct());
    EXPECT_FALSE(session->PeerReportedUnreachable());

    sessionManager.Shutdown();
}

} // namespace
