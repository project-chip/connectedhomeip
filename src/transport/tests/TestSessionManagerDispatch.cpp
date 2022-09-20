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
#include <crypto/PersistentStorageOperationalKeystore.h>
#include <lib/core/CHIPCore.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <protocols/secure_channel/MessageCounterManager.h>
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

using TestContext = chip::Test::LoopbackTransportManager;

struct MessageTestEntry
{
    const char * name;

    const char * peerAddr;

    const char * payload;
    const char * plain;
    const char * encrypted;
    const char * privacy;

    size_t payloadLength;
    size_t plainLength;
    size_t encryptedLength;
    size_t privacyLength;

    const char * encryptKey;
    const char * privacyKey;
    const char * epochKey;

    const char * nonce;
    const char * privacyNonce;
    const char * compressedFabricId;

    const char * mic;

    uint16_t sessionId;
    NodeId peerNodeId;
    FabricIndex fabricIndex;
};

struct MessageTestEntry theMessageTestVector[] = {
    {
        .name     = "secure pase message",
        .peerAddr = "::1",

        .payload   = "",
        .plain     = "\x00\xb8\x0b\x00\x39\x30\x00\x00\x05\x64\xee\x0e\x20\x7d",
        .encrypted = "\x00\xb8\x0b\x00\x39\x30\x00\x00\x5a\x98\x9a\xe4\x2e\x8d"
                     "\x84\x7f\x53\x5c\x30\x07\xe6\x15\x0c\xd6\x58\x67\xf2\xb8\x17\xdb", // Includes MIC
        .privacy = "\x00\xb8\x0b\x00\x39\x30\x00\x00\x5a\x98\x9a\xe4\x2e\x8d"
                   "\x84\x7f\x53\x5c\x30\x07\xe6\x15\x0c\xd6\x58\x67\xf2\xb8\x17\xdb", // Includes MIC

        .payloadLength   = 0,
        .plainLength     = 14,
        .encryptedLength = 30,
        .privacyLength   = 30,

        .encryptKey = "\x5e\xde\xd2\x44\xe5\x53\x2b\x3c\xdc\x23\x40\x9d\xba\xd0\x52\xd2",

        .nonce = "\x00\x39\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",

        .sessionId   = 0x0bb8, // 3000
        .peerNodeId  = 0x0000000000000000ULL,
        .fabricIndex = 1,
    },
};

const uint16_t theMessageTestVectorLength = sizeof(theMessageTestVector) / sizeof(theMessageTestVector[0]);

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

class TestSessionManagerCallback : public SessionMessageDelegate
{
public:
    void OnMessageReceived(const PacketHeader & header, const PayloadHeader & payloadHeader, const SessionHandle & session,
                           DuplicateMessage isDuplicate, System::PacketBufferHandle && msgBuf) override
    {
        mReceivedCount++;

        MessageTestEntry & testEntry = theMessageTestVector[mTestVectorIndex];

        ChipLogProgress(Test, "OnMessageReceived: sessionId=0x%04x", testEntry.sessionId);
        NL_TEST_ASSERT(mSuite, header.GetSessionId() == testEntry.sessionId);

        size_t dataLength   = msgBuf->DataLength();
        size_t expectLength = testEntry.payloadLength;

        NL_TEST_ASSERT(mSuite, dataLength == expectLength);
        NL_TEST_ASSERT(mSuite, memcmp(msgBuf->Start(), testEntry.payload, dataLength) == 0);

        ChipLogProgress(Test, "TestSessionManagerDispatch[%d] PASS", mTestVectorIndex);
    }

    void ResetTest(unsigned testVectorIndex)
    {
        mTestVectorIndex = testVectorIndex;
        mReceivedCount   = 0;
    }

    unsigned NumMessagesReceived() { return mReceivedCount; }

    nlTestSuite * mSuite      = nullptr;
    unsigned mTestVectorIndex = 0;
    unsigned mReceivedCount   = 0;
};

PeerAddress AddressFromString(const char * str)
{
    Inet::IPAddress addr;

    VerifyOrDie(Inet::IPAddress::FromString(str, addr));

    return PeerAddress::UDP(addr);
}

void TestSessionManagerInit(nlTestSuite * inSuite, TestContext & ctx, SessionManager & sessionManager)
{
    static FabricTableHolder fabricTableHolder;
    static secure_channel::MessageCounterManager gMessageCounterManager;
    static chip::TestPersistentStorageDelegate deviceStorage;

    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == fabricTableHolder.Init());
    NL_TEST_ASSERT(inSuite,
                   CHIP_NO_ERROR ==
                       sessionManager.Init(&ctx.GetSystemLayer(), &ctx.GetTransportMgr(), &gMessageCounterManager, &deviceStorage,
                                           &fabricTableHolder.GetFabricTable()));
}

void TestSessionManagerDispatch(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);
    SessionManager sessionManager;
    TestSessionManagerCallback callback;

    TestSessionManagerInit(inSuite, ctx, sessionManager);
    sessionManager.SetMessageDelegate(&callback);

    IPAddress addr;
    IPAddress::FromString("::1", addr);
    Transport::PeerAddress peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));

    SessionHolder aliceToBobSession;

    callback.mSuite = inSuite;
    for (unsigned i = 0; i < theMessageTestVectorLength; i++)
    {
        MessageTestEntry & testEntry = theMessageTestVector[i];
        callback.ResetTest(i);

        ChipLogProgress(Test, "===> TestSessionManagerDispatch[%d] '%s': sessionId=0x%04x", i, testEntry.name, testEntry.sessionId);

        // Inject Sessions
        err = sessionManager.InjectPaseSessionWithTestKey(aliceToBobSession, testEntry.sessionId, testEntry.peerNodeId,
                                                          testEntry.sessionId, testEntry.fabricIndex, peer,
                                                          CryptoContext::SessionRole::kResponder);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        const char * plain = testEntry.plain;
        const ByteSpan expectedPlain(reinterpret_cast<const uint8_t *>(plain), testEntry.plainLength);
        const char * privacy = testEntry.privacy;
        chip::System::PacketBufferHandle msg =
            chip::MessagePacketBuffer::NewWithData(reinterpret_cast<const uint8_t *>(privacy), testEntry.privacyLength);

        // TODO: inject raw keys rather than always defaulting to test key

        const PeerAddress peerAddress = AddressFromString(testEntry.peerAddr);
        sessionManager.OnMessageReceived(peerAddress, std::move(msg));
        NL_TEST_ASSERT(inSuite, callback.NumMessagesReceived() > 0);
    }

    sessionManager.Shutdown();
}

// ============================================================================
//              Test Suite Instrumenation
// ============================================================================

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

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("Test Session Manager Dispatch",  TestSessionManagerDispatch),

    NL_TEST_SENTINEL()
};

nlTestSuite sSuite =
{
    "TestSessionManagerDispatch",
    &sTests[0],
    Initialize,
    Finalize
};
// clang-format on

} // namespace

/**
 *  Main
 */
int TestSessionManagerDispatchSuite()
{
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestSessionManagerDispatchSuite);
