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

#include <credentials/PersistentStorageOpCertStore.h>
#include <credentials/tests/CHIPCert_unit_test_vectors.h>
#include <crypto/DefaultSessionKeystore.h>
#include <crypto/PersistentStorageOperationalKeystore.h>
#include <lib/core/CHIPCore.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <protocols/Protocols.h>
#include <protocols/echo/Echo.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/PASESession.h>
#include <transport/SessionManager.h>
#include <transport/TransportMgr.h>
#include <transport/tests/LoopbackTransportManager.h>

#include <nlbyteorder.h>
#include <nlunit-test.h>

#include <errno.h>

#undef CHIP_ENABLE_TEST_ENCRYPTED_BUFFER_API

namespace {

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::Test;
using namespace chip::TestCerts;

using TestContext = chip::Test::LoopbackTransportManager;

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

    FabricTableHolder fabricTableHolder;
    SessionManager sessionManager;
    secure_channel::MessageCounterManager gMessageCounterManager;
    chip::TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;

    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == fabricTableHolder.Init());
    NL_TEST_ASSERT(inSuite,
                   CHIP_NO_ERROR ==
                       sessionManager.Init(&ctx.GetSystemLayer(), &ctx.GetTransportMgr(), &gMessageCounterManager, &deviceStorage,
                                           &fabricTableHolder.GetFabricTable(), sessionKeystore));
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

    FabricTableHolder fabricTableHolder;
    SessionManager sessionManager;
    secure_channel::MessageCounterManager gMessageCounterManager;
    chip::TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    FabricTable & fabricTable    = fabricTableHolder.GetFabricTable();
    FabricIndex aliceFabricIndex = kUndefinedFabricIndex;
    FabricIndex bobFabricIndex   = kUndefinedFabricIndex;

    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == fabricTableHolder.Init());
    NL_TEST_ASSERT(inSuite,
                   CHIP_NO_ERROR ==
                       sessionManager.Init(&ctx.GetSystemLayer(), &ctx.GetTransportMgr(), &gMessageCounterManager, &deviceStorage,
                                           &fabricTableHolder.GetFabricTable(), sessionKeystore));

    callback.mSuite = inSuite;

    sessionManager.SetMessageDelegate(&callback);

    Transport::PeerAddress peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));

    err =
        fabricTable.AddNewFabricForTestIgnoringCollisions(GetRootACertAsset().mCert, GetIAA1CertAsset().mCert,
                                                          GetNodeA1CertAsset().mCert, GetNodeA1CertAsset().mKey, &aliceFabricIndex);
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == err);

    err = fabricTable.AddNewFabricForTestIgnoringCollisions(GetRootACertAsset().mCert, GetIAA1CertAsset().mCert,
                                                            GetNodeA2CertAsset().mCert, GetNodeA2CertAsset().mKey, &bobFabricIndex);
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == err);

    SessionHolder aliceToBobSession;
    err = sessionManager.InjectPaseSessionWithTestKey(aliceToBobSession, 2,
                                                      fabricTable.FindFabricWithIndex(bobFabricIndex)->GetNodeId(), 1,
                                                      aliceFabricIndex, peer, CryptoContext::SessionRole::kInitiator);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SessionHolder bobToAliceSession;
    err = sessionManager.InjectPaseSessionWithTestKey(bobToAliceSession, 1,
                                                      fabricTable.FindFabricWithIndex(aliceFabricIndex)->GetNodeId(), 2,
                                                      bobFabricIndex, peer, CryptoContext::SessionRole::kResponder);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Should be able to send a message to itself by just calling send.
    callback.ReceiveHandlerCallCount = 0;

    PayloadHeader payloadHeader;

    // Set the exchange ID for this header.
    payloadHeader.SetExchangeID(0);

    // Set the protocol ID and message type for this header.
    payloadHeader.SetMessageType(chip::Protocols::Echo::MsgType::EchoRequest);

    EncryptedPacketBufferHandle preparedMessage;
    err = sessionManager.PrepareMessage(aliceToBobSession.Get().Value(), payloadHeader, std::move(buffer), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get().Value(), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();
    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 1);

    // Let's send the max sized message and make sure it is received
    chip::System::PacketBufferHandle large_buffer = chip::MessagePacketBuffer::NewWithData(LARGE_PAYLOAD, kMaxAppMessageLen);
    NL_TEST_ASSERT(inSuite, !large_buffer.IsNull());

    callback.LargeMessageSent = true;

    err = sessionManager.PrepareMessage(aliceToBobSession.Get().Value(), payloadHeader, std::move(large_buffer), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get().Value(), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();
    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 2);

    uint16_t large_payload_len = sizeof(LARGE_PAYLOAD);

    // Let's send bigger message than supported and make sure it fails to send
    chip::System::PacketBufferHandle extra_large_buffer = chip::MessagePacketBuffer::NewWithData(LARGE_PAYLOAD, large_payload_len);
    NL_TEST_ASSERT(inSuite, !extra_large_buffer.IsNull());

    callback.LargeMessageSent = true;

    err = sessionManager.PrepareMessage(aliceToBobSession.Get().Value(), payloadHeader, std::move(extra_large_buffer),
                                        preparedMessage);
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

    FabricTableHolder fabricTableHolder;
    SessionManager sessionManager;
    secure_channel::MessageCounterManager gMessageCounterManager;
    chip::TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    FabricTable & fabricTable    = fabricTableHolder.GetFabricTable();
    FabricIndex aliceFabricIndex = kUndefinedFabricIndex;
    FabricIndex bobFabricIndex   = kUndefinedFabricIndex;

    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == fabricTableHolder.Init());
    NL_TEST_ASSERT(inSuite,
                   CHIP_NO_ERROR ==
                       sessionManager.Init(&ctx.GetSystemLayer(), &ctx.GetTransportMgr(), &gMessageCounterManager, &deviceStorage,
                                           &fabricTableHolder.GetFabricTable(), sessionKeystore));

    callback.mSuite = inSuite;

    sessionManager.SetMessageDelegate(&callback);

    Transport::PeerAddress peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));

    err =
        fabricTable.AddNewFabricForTestIgnoringCollisions(GetRootACertAsset().mCert, GetIAA1CertAsset().mCert,
                                                          GetNodeA1CertAsset().mCert, GetNodeA1CertAsset().mKey, &aliceFabricIndex);
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == err);

    err = fabricTable.AddNewFabricForTestIgnoringCollisions(GetRootACertAsset().mCert, GetIAA1CertAsset().mCert,
                                                            GetNodeA2CertAsset().mCert, GetNodeA2CertAsset().mKey, &bobFabricIndex);
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == err);

    SessionHolder aliceToBobSession;
    err = sessionManager.InjectPaseSessionWithTestKey(aliceToBobSession, 2,
                                                      fabricTable.FindFabricWithIndex(bobFabricIndex)->GetNodeId(), 1,
                                                      aliceFabricIndex, peer, CryptoContext::SessionRole::kInitiator);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SessionHolder bobToAliceSession;
    err = sessionManager.InjectPaseSessionWithTestKey(bobToAliceSession, 1,
                                                      fabricTable.FindFabricWithIndex(aliceFabricIndex)->GetNodeId(), 2,
                                                      bobFabricIndex, peer, CryptoContext::SessionRole::kResponder);
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

    err = sessionManager.PrepareMessage(aliceToBobSession.Get().Value(), payloadHeader, std::move(buffer), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get().Value(), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();
    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 1);

    // Reset receive side message counter, or duplicated message will be denied.
    Transport::SecureSession * session = bobToAliceSession.Get().Value()->AsSecureSession();
    session->GetSessionMessageCounter().GetPeerMessageCounter().SetCounter(1);

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get().Value(), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();
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

    FabricTableHolder fabricTableHolder;
    SessionManager sessionManager;
    secure_channel::MessageCounterManager gMessageCounterManager;
    chip::TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    FabricTable & fabricTable    = fabricTableHolder.GetFabricTable();
    FabricIndex aliceFabricIndex = kUndefinedFabricIndex;
    FabricIndex bobFabricIndex   = kUndefinedFabricIndex;

    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == fabricTableHolder.Init());
    NL_TEST_ASSERT(inSuite,
                   CHIP_NO_ERROR ==
                       sessionManager.Init(&ctx.GetSystemLayer(), &ctx.GetTransportMgr(), &gMessageCounterManager, &deviceStorage,
                                           &fabricTableHolder.GetFabricTable(), sessionKeystore));

    callback.mSuite = inSuite;

    sessionManager.SetMessageDelegate(&callback);

    Transport::PeerAddress peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));

    err =
        fabricTable.AddNewFabricForTestIgnoringCollisions(GetRootACertAsset().mCert, GetIAA1CertAsset().mCert,
                                                          GetNodeA1CertAsset().mCert, GetNodeA1CertAsset().mKey, &aliceFabricIndex);
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == err);

    err = fabricTable.AddNewFabricForTestIgnoringCollisions(GetRootACertAsset().mCert, GetIAA1CertAsset().mCert,
                                                            GetNodeA2CertAsset().mCert, GetNodeA2CertAsset().mKey, &bobFabricIndex);
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == err);

    SessionHolder aliceToBobSession;
    err = sessionManager.InjectPaseSessionWithTestKey(aliceToBobSession, 2,
                                                      fabricTable.FindFabricWithIndex(bobFabricIndex)->GetNodeId(), 1,
                                                      aliceFabricIndex, peer, CryptoContext::SessionRole::kInitiator);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SessionHolder bobToAliceSession;
    err = sessionManager.InjectPaseSessionWithTestKey(bobToAliceSession, 1,
                                                      fabricTable.FindFabricWithIndex(aliceFabricIndex)->GetNodeId(), 2,
                                                      bobFabricIndex, peer, CryptoContext::SessionRole::kResponder);
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

    err = sessionManager.PrepareMessage(aliceToBobSession.Get().Value(), payloadHeader, std::move(buffer), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get().Value(), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();
    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 1);

    /* -------------------------------------------------------------------------------------------*/
    // Reset receive side message counter, or duplicated message will be denied.
    Transport::SecureSession * session = bobToAliceSession.Get().Value()->AsSecureSession();
    session->GetSessionMessageCounter().GetPeerMessageCounter().SetCounter(1);

    PacketHeader packetHeader;

    // Change Message ID
    EncryptedPacketBufferHandle badMessageCounterMsg = preparedMessage.CloneData();
    NL_TEST_ASSERT(inSuite, badMessageCounterMsg.ExtractPacketHeader(packetHeader) == CHIP_NO_ERROR);

    uint32_t messageCounter = packetHeader.GetMessageCounter();
    packetHeader.SetMessageCounter(messageCounter + 1);
    NL_TEST_ASSERT(inSuite, badMessageCounterMsg.InsertPacketHeader(packetHeader) == CHIP_NO_ERROR);

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get().Value(), badMessageCounterMsg);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();
    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 1);

    /* -------------------------------------------------------------------------------------------*/
    session->GetSessionMessageCounter().GetPeerMessageCounter().SetCounter(1);

    // Change Key ID
    EncryptedPacketBufferHandle badKeyIdMsg = preparedMessage.CloneData();
    NL_TEST_ASSERT(inSuite, badKeyIdMsg.ExtractPacketHeader(packetHeader) == CHIP_NO_ERROR);

    // the secure channel is setup to use key ID 1, and 2. So let's use 3 here.
    packetHeader.SetSessionId(3);
    NL_TEST_ASSERT(inSuite, badKeyIdMsg.InsertPacketHeader(packetHeader) == CHIP_NO_ERROR);

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get().Value(), badKeyIdMsg);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();
    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 1);

    /* -------------------------------------------------------------------------------------------*/
    session->GetSessionMessageCounter().GetPeerMessageCounter().SetCounter(1);

    // Send the correct encrypted msg
    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get().Value(), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();
    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 2);

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

    FabricTableHolder fabricTableHolder;
    SessionManager sessionManager;
    secure_channel::MessageCounterManager gMessageCounterManager;
    chip::TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    FabricTable & fabricTable    = fabricTableHolder.GetFabricTable();
    FabricIndex aliceFabricIndex = kUndefinedFabricIndex;
    FabricIndex bobFabricIndex   = kUndefinedFabricIndex;

    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == fabricTableHolder.Init());
    NL_TEST_ASSERT(inSuite,
                   CHIP_NO_ERROR ==
                       sessionManager.Init(&ctx.GetSystemLayer(), &ctx.GetTransportMgr(), &gMessageCounterManager, &deviceStorage,
                                           &fabricTableHolder.GetFabricTable(), sessionKeystore));

    callback.mSuite = inSuite;

    sessionManager.SetMessageDelegate(&callback);

    Transport::PeerAddress peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));

    err =
        fabricTable.AddNewFabricForTestIgnoringCollisions(GetRootACertAsset().mCert, GetIAA1CertAsset().mCert,
                                                          GetNodeA1CertAsset().mCert, GetNodeA1CertAsset().mKey, &aliceFabricIndex);
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == err);

    err = fabricTable.AddNewFabricForTestIgnoringCollisions(GetRootACertAsset().mCert, GetIAA1CertAsset().mCert,
                                                            GetNodeA2CertAsset().mCert, GetNodeA2CertAsset().mKey, &bobFabricIndex);
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == err);

    SessionHolder aliceToBobSession;
    err = sessionManager.InjectPaseSessionWithTestKey(aliceToBobSession, 2,
                                                      fabricTable.FindFabricWithIndex(bobFabricIndex)->GetNodeId(), 1,
                                                      aliceFabricIndex, peer, CryptoContext::SessionRole::kInitiator);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SessionHolder bobToAliceSession;
    err = sessionManager.InjectPaseSessionWithTestKey(bobToAliceSession, 1,
                                                      fabricTable.FindFabricWithIndex(aliceFabricIndex)->GetNodeId(), 2,
                                                      bobFabricIndex, peer, CryptoContext::SessionRole::kResponder);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    callback.ReceiveHandlerCallCount = 0;

    PayloadHeader payloadHeader;
    EncryptedPacketBufferHandle preparedMessage;

    // Set the exchange ID for this header.
    payloadHeader.SetExchangeID(0);

    // Set the protocol ID and message type for this header.
    payloadHeader.SetMessageType(chip::Protocols::Echo::MsgType::EchoRequest);

    payloadHeader.SetInitiator(true);

    err = sessionManager.PrepareMessage(aliceToBobSession.Get().Value(), payloadHeader, std::move(buffer), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get().Value(), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();
    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 1);

    // Now advance our message counter by 5.
    EncryptedPacketBufferHandle newMessage;
    for (size_t i = 0; i < 5; ++i)
    {
        buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, payload_len);
        NL_TEST_ASSERT(inSuite, !buffer.IsNull());

        err = sessionManager.PrepareMessage(aliceToBobSession.Get().Value(), payloadHeader, std::move(buffer), newMessage);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    }

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get().Value(), newMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();
    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 2);

    // Now resend our original message.  It should be rejected as a duplicate.

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get().Value(), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();
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

    FabricTableHolder fabricTableHolder;
    SessionManager sessionManager;
    secure_channel::MessageCounterManager gMessageCounterManager;
    chip::TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    FabricTable & fabricTable    = fabricTableHolder.GetFabricTable();
    FabricIndex aliceFabricIndex = kUndefinedFabricIndex;
    FabricIndex bobFabricIndex   = kUndefinedFabricIndex;

    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == fabricTableHolder.Init());
    NL_TEST_ASSERT(inSuite,
                   CHIP_NO_ERROR ==
                       sessionManager.Init(&ctx.GetSystemLayer(), &ctx.GetTransportMgr(), &gMessageCounterManager, &deviceStorage,
                                           &fabricTableHolder.GetFabricTable(), sessionKeystore));
    callback.mSuite = inSuite;

    sessionManager.SetMessageDelegate(&callback);

    Transport::PeerAddress peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));

    err =
        fabricTable.AddNewFabricForTestIgnoringCollisions(GetRootACertAsset().mCert, GetIAA1CertAsset().mCert,
                                                          GetNodeA1CertAsset().mCert, GetNodeA1CertAsset().mKey, &aliceFabricIndex);
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == err);

    err = fabricTable.AddNewFabricForTestIgnoringCollisions(GetRootACertAsset().mCert, GetIAA1CertAsset().mCert,
                                                            GetNodeA2CertAsset().mCert, GetNodeA2CertAsset().mKey, &bobFabricIndex);
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == err);

    SessionHolder aliceToBobSession;
    err = sessionManager.InjectPaseSessionWithTestKey(aliceToBobSession, 2,
                                                      fabricTable.FindFabricWithIndex(bobFabricIndex)->GetNodeId(), 1,
                                                      aliceFabricIndex, peer, CryptoContext::SessionRole::kInitiator);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SessionHolder bobToAliceSession;
    err = sessionManager.InjectPaseSessionWithTestKey(bobToAliceSession, 1,
                                                      fabricTable.FindFabricWithIndex(aliceFabricIndex)->GetNodeId(), 2,
                                                      bobFabricIndex, peer, CryptoContext::SessionRole::kResponder);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    callback.ReceiveHandlerCallCount = 0;

    PayloadHeader payloadHeader;
    EncryptedPacketBufferHandle preparedMessage;

    // Set the exchange ID for this header.
    payloadHeader.SetExchangeID(0);

    // Set the protocol ID and message type for this header.
    payloadHeader.SetMessageType(chip::Protocols::Echo::MsgType::EchoRequest);

    payloadHeader.SetInitiator(true);

    err = sessionManager.PrepareMessage(aliceToBobSession.Get().Value(), payloadHeader, std::move(buffer), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get().Value(), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();
    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 1);

    // Now advance our message counter by at least
    // CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE + 2, so preparedMessage will be
    // out of the window.
    EncryptedPacketBufferHandle newMessage;
    for (size_t i = 0; i < CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE + 2; ++i)
    {
        buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, payload_len);
        NL_TEST_ASSERT(inSuite, !buffer.IsNull());

        err = sessionManager.PrepareMessage(aliceToBobSession.Get().Value(), payloadHeader, std::move(buffer), newMessage);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    }

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get().Value(), newMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();
    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 2);

    // Now resend our original message.  It should be rejected as a duplicate.

    err = sessionManager.SendPreparedMessage(aliceToBobSession.Get().Value(), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();
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
        auto handle = sessionManager.AllocateSession(
            Transport::SecureSession::Type::kPASE,
            ScopedNodeId(NodeIdFromPAKEKeyId(kDefaultCommissioningPasscodeId), kUndefinedFabricIndex));
        NL_TEST_ASSERT(inSuite, handle.HasValue());
        handle.Value()->AsSecureSession()->MarkForEviction();
    }
}

void SessionAllocationTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    FabricTableHolder fabricTableHolder;
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == fabricTableHolder.Init());

    secure_channel::MessageCounterManager messageCounterManager;
    TestPersistentStorageDelegate deviceStorage1, deviceStorage2;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;
    SessionManager sessionManager;

    NL_TEST_ASSERT(inSuite,
                   CHIP_NO_ERROR ==
                       sessionManager.Init(&ctx.GetSystemLayer(), &ctx.GetTransportMgr(), &messageCounterManager, &deviceStorage1,
                                           &fabricTableHolder.GetFabricTable(), sessionKeystore));

    // Allocate a session.
    uint16_t sessionId1;
    {
        auto handle = sessionManager.AllocateSession(
            Transport::SecureSession::Type::kPASE,
            ScopedNodeId(NodeIdFromPAKEKeyId(kDefaultCommissioningPasscodeId), kUndefinedFabricIndex));
        NL_TEST_ASSERT(inSuite, handle.HasValue());
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
        NL_TEST_ASSERT(inSuite, sessionId - prevSessionId == 1 || (sessionId == 1 && prevSessionId == 65535));
        NL_TEST_ASSERT(inSuite, sessionId != 0);
        prevSessionId = sessionId;
    }

    // Reconstruct the Session Manager to reset state.
    sessionManager.Shutdown();
    sessionManager.~SessionManager();
    new (&sessionManager) SessionManager();
    NL_TEST_ASSERT(inSuite,
                   CHIP_NO_ERROR ==
                       sessionManager.Init(&ctx.GetSystemLayer(), &ctx.GetTransportMgr(), &messageCounterManager, &deviceStorage2,
                                           &fabricTableHolder.GetFabricTable(), sessionKeystore));

    // Allocate a single session so we know what random id we are starting at.
    {
        auto handle = sessionManager.AllocateSession(
            Transport::SecureSession::Type::kPASE,
            ScopedNodeId(NodeIdFromPAKEKeyId(kDefaultCommissioningPasscodeId), kUndefinedFabricIndex));
        NL_TEST_ASSERT(inSuite, handle.HasValue());
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
        NL_TEST_ASSERT(inSuite, handle.HasValue());
        auto sessionId = handle.Value()->AsSecureSession()->GetLocalSessionId();
        NL_TEST_ASSERT(inSuite, sessionId - prevSessionId == 1 || (sessionId == 1 && prevSessionId == 65535));
        NL_TEST_ASSERT(inSuite, sessionId != 0);
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
            auto handle = sessionManager.AllocateSession(
                Transport::SecureSession::Type::kPASE,
                ScopedNodeId(NodeIdFromPAKEKeyId(kDefaultCommissioningPasscodeId), kUndefinedFabricIndex));
            NL_TEST_ASSERT(inSuite, handle.HasValue());
            auto potentialCollision = handle.Value()->AsSecureSession()->GetLocalSessionId();
            for (uint16_t sessionId : sessionIds)
            {
                NL_TEST_ASSERT(inSuite, potentialCollision != sessionId);
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

void SessionCounterExhaustedTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

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

    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == fabricTableHolder.Init());
    NL_TEST_ASSERT(inSuite,
                   CHIP_NO_ERROR ==
                       sessionManager.Init(&ctx.GetSystemLayer(), &ctx.GetTransportMgr(), &gMessageCounterManager, &deviceStorage,
                                           &fabricTableHolder.GetFabricTable(), sessionKeystore));

    Transport::PeerAddress peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));

    err =
        fabricTable.AddNewFabricForTestIgnoringCollisions(GetRootACertAsset().mCert, GetIAA1CertAsset().mCert,
                                                          GetNodeA1CertAsset().mCert, GetNodeA1CertAsset().mKey, &aliceFabricIndex);
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == err);

    err = fabricTable.AddNewFabricForTestIgnoringCollisions(GetRootACertAsset().mCert, GetIAA1CertAsset().mCert,
                                                            GetNodeA2CertAsset().mCert, GetNodeA2CertAsset().mKey, &bobFabricIndex);
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == err);

    SessionHolder aliceToBobSession;
    err = sessionManager.InjectPaseSessionWithTestKey(aliceToBobSession, 2,
                                                      fabricTable.FindFabricWithIndex(bobFabricIndex)->GetNodeId(), 1,
                                                      aliceFabricIndex, peer, CryptoContext::SessionRole::kInitiator);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SessionHolder bobToAliceSession;
    err = sessionManager.InjectPaseSessionWithTestKey(bobToAliceSession, 1,
                                                      fabricTable.FindFabricWithIndex(aliceFabricIndex)->GetNodeId(), 2,
                                                      bobFabricIndex, peer, CryptoContext::SessionRole::kResponder);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // ==== Set counter value to max ====
    LocalSessionMessageCounter & counter = static_cast<LocalSessionMessageCounter &>(
        aliceToBobSession.Get().Value()->AsSecureSession()->GetSessionMessageCounter().GetLocalMessageCounter());
    counter.TestSetCounter(LocalSessionMessageCounter::kMessageCounterMax - 1);

    // ==== Build a valid message with max counter value ====
    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    PayloadHeader payloadHeader;

    // Set the exchange ID for this header.
    payloadHeader.SetExchangeID(0);

    // Set the protocol ID and message type for this header.
    payloadHeader.SetMessageType(chip::Protocols::Echo::MsgType::EchoRequest);

    EncryptedPacketBufferHandle preparedMessage;
    err = sessionManager.PrepareMessage(aliceToBobSession.Get().Value(), payloadHeader, std::move(buffer), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // ==== Build another message which will fail becuase message counter is exhausted ====
    chip::System::PacketBufferHandle buffer2 = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer2.IsNull());

    EncryptedPacketBufferHandle preparedMessage2;
    err = sessionManager.PrepareMessage(aliceToBobSession.Get().Value(), payloadHeader, std::move(buffer2), preparedMessage2);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_MESSAGE_COUNTER_EXHAUSTED);

    sessionManager.Shutdown();
}

static void SessionShiftingTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

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

    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == fabricTableHolder.Init());
    NL_TEST_ASSERT(inSuite,
                   CHIP_NO_ERROR ==
                       sessionManager.Init(&ctx.GetSystemLayer(), &ctx.GetTransportMgr(), &messageCounterManager, &deviceStorage,
                                           &fabricTableHolder.GetFabricTable(), sessionKeystore));

    Transport::PeerAddress peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));

    SessionHolder aliceToBobSession;
    CHIP_ERROR err = sessionManager.InjectCaseSessionWithTestKey(aliceToBobSession, 2, 1, aliceNodeId, bobNodeId, aliceFabricIndex,
                                                                 peer, CryptoContext::SessionRole::kInitiator);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    class StickySessionDelegate : public SessionDelegate
    {
    public:
        NewSessionHandlingPolicy GetNewSessionHandlingPolicy() override { return NewSessionHandlingPolicy::kStayAtOldSession; }
        void OnSessionReleased() override {}
    } delegate;

    SessionHolderWithDelegate stickyAliceToBobSession(aliceToBobSession.Get().Value(), delegate);
    NL_TEST_ASSERT(inSuite, aliceToBobSession.Contains(stickyAliceToBobSession.Get().Value()));

    SessionHolder bobToAliceSession;
    err = sessionManager.InjectCaseSessionWithTestKey(bobToAliceSession, 1, 2, bobNodeId, aliceNodeId, bobFabricIndex, peer,
                                                      CryptoContext::SessionRole::kResponder);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SessionHolder newAliceToBobSession;
    err = sessionManager.InjectCaseSessionWithTestKey(newAliceToBobSession, 3, 4, aliceNodeId, bobNodeId, aliceFabricIndex, peer,
                                                      CryptoContext::SessionRole::kInitiator);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Here we got 3 sessions, and 4 holders:
    // 1. alice -> bob: aliceToBobSession, stickyAliceToBobSession
    // 2. alice <- bob: bobToAliceSession
    // 3. alice -> bob: newAliceToBobSession

    SecureSession * session1 = aliceToBobSession->AsSecureSession();
    SecureSession * session2 = bobToAliceSession->AsSecureSession();
    SecureSession * session3 = newAliceToBobSession->AsSecureSession();

    NL_TEST_ASSERT(inSuite, session1 != session3);
    NL_TEST_ASSERT(inSuite, stickyAliceToBobSession->AsSecureSession() == session1);

    // Now shift the 1st session to the 3rd one, after shifting, holders should be:
    // 1. alice -> bob: stickyAliceToBobSession
    // 2. alice <- bob: bobToAliceSession
    // 3. alice -> bob: aliceToBobSession, newAliceToBobSession
    sessionManager.GetSecureSessions().NewerSessionAvailable(newAliceToBobSession.Get().Value()->AsSecureSession());

    NL_TEST_ASSERT(inSuite, aliceToBobSession);
    NL_TEST_ASSERT(inSuite, stickyAliceToBobSession);
    NL_TEST_ASSERT(inSuite, newAliceToBobSession);

    NL_TEST_ASSERT(inSuite, stickyAliceToBobSession->AsSecureSession() == session1);
    NL_TEST_ASSERT(inSuite, bobToAliceSession->AsSecureSession() == session2);
    NL_TEST_ASSERT(inSuite, aliceToBobSession->AsSecureSession() == session3);
    NL_TEST_ASSERT(inSuite, newAliceToBobSession->AsSecureSession() == session3);

    sessionManager.Shutdown();
}

static void TestFindSecureSessionForNode(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

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

    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == fabricTableHolder.Init());
    NL_TEST_ASSERT(inSuite,
                   CHIP_NO_ERROR ==
                       sessionManager.Init(&ctx.GetSystemLayer(), &ctx.GetTransportMgr(), &messageCounterManager, &deviceStorage,
                                           &fabricTableHolder.GetFabricTable(), sessionKeystore));

    Transport::PeerAddress peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));

    SessionHolder aliceToBobSession;
    CHIP_ERROR err = sessionManager.InjectCaseSessionWithTestKey(aliceToBobSession, 2, 1, aliceNodeId, bobNodeId, aliceFabricIndex,
                                                                 peer, CryptoContext::SessionRole::kInitiator);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    aliceToBobSession->AsSecureSession()->MarkActive();

    SessionHolder newAliceToBobSession;
    err = sessionManager.InjectCaseSessionWithTestKey(newAliceToBobSession, 3, 4, aliceNodeId, bobNodeId, aliceFabricIndex, peer,
                                                      CryptoContext::SessionRole::kInitiator);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    while (System::SystemClock().GetMonotonicTimestamp() <= aliceToBobSession->AsSecureSession()->GetLastActivityTime())
    {
        // Wait for the clock to advance so the new session is
        // more-recently-active.
    }
    newAliceToBobSession->AsSecureSession()->MarkActive();

    auto foundSession = sessionManager.FindSecureSessionForNode(ScopedNodeId(bobNodeId, aliceFabricIndex),
                                                                MakeOptional(SecureSession::Type::kCASE));
    NL_TEST_ASSERT(inSuite, foundSession.HasValue());
    NL_TEST_ASSERT(inSuite, newAliceToBobSession.Contains(foundSession.Value()));
    NL_TEST_ASSERT(inSuite, !aliceToBobSession.Contains(foundSession.Value()));

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
    NL_TEST_DEF("Old counter Test",               SendPacketWithOldCounterTest),
    NL_TEST_DEF("Too-old counter Test",           SendPacketWithTooOldCounterTest),
    NL_TEST_DEF("Session Allocation Test",        SessionAllocationTest),
    NL_TEST_DEF("Session Counter Exhausted Test", SessionCounterExhaustedTest),
    NL_TEST_DEF("SessionShiftingTest",            SessionShiftingTest),
    NL_TEST_DEF("TestFindSecureSessionForNode",   TestFindSecureSessionForNode),

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
    reinterpret_cast<TestContext *>(aContext)->Shutdown();
    return SUCCESS;
}

} // namespace

/**
 *  Main
 */
int TestSessionManager()
{
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestSessionManager);
