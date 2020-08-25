/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file implements unit tests for the SecureSessionMgr implementation.
 */

#include "TestTransportLayer.h"

#include "NetworkTestHelpers.h"

#include <core/CHIPCore.h>
#include <support/CodeUtils.h>
#include <transport/SecureSessionMgr.h>

#include <nlbyteorder.h>
#include <nlunit-test.h>

#include <errno.h>

namespace {

using namespace chip;
using namespace chip::Transport;

using TestContext = chip::Test::IOContext;

TestContext sContext;

static const char PAYLOAD[]         = "Hello!";
constexpr NodeId kSourceNodeId      = 123654;
constexpr NodeId kDestinationNodeId = 111222333;

class LoopbackTransport : public Transport::Base
{
public:
    /// Transports are required to have a constructor that takes exactly one argument
    CHIP_ERROR Init(const char * unused) { return CHIP_NO_ERROR; }

    CHIP_ERROR SendMessage(const MessageHeader & header, const PeerAddress & address, System::PacketBuffer * msgBuf) override
    {
        HandleMessageReceived(header, address, msgBuf);
        return CHIP_NO_ERROR;
    }

    bool CanSendToPeer(const PeerAddress & address) override { return true; }
};

class TestSessMgrCallback : public SecureSessionMgrCallback
{
public:
    virtual void OnMessageReceived(const MessageHeader & header, PeerConnectionState * state, System::PacketBuffer * msgBuf,
                                   SecureSessionMgrBase * mgr)
    {
        NL_TEST_ASSERT(mSuite, header.GetSourceNodeId() == Optional<NodeId>::Value(kSourceNodeId));
        NL_TEST_ASSERT(mSuite, header.GetDestinationNodeId() == Optional<NodeId>::Value(kDestinationNodeId));
        NL_TEST_ASSERT(mSuite, state->GetPeerNodeId() == kSourceNodeId);

        size_t data_len = msgBuf->DataLength();

        int compare = memcmp(msgBuf->Start(), PAYLOAD, data_len);
        NL_TEST_ASSERT(mSuite, compare == 0);

        ReceiveHandlerCallCount++;
    }

    virtual void OnNewConnection(PeerConnectionState * state, SecureSessionMgrBase * mgr) { NewConnectionHandlerCallCount++; }

    nlTestSuite * mSuite              = nullptr;
    int ReceiveHandlerCallCount       = 0;
    int NewConnectionHandlerCallCount = 0;
};

TestSessMgrCallback callback;

void CheckSimpleInitTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    SecureSessionMgr<LoopbackTransport> conn;
    CHIP_ERROR err;

    ctx.GetInetLayer().SystemLayer()->Init(NULL);

    err = conn.Init(kSourceNodeId, ctx.GetInetLayer().SystemLayer(), "LOOPBACK");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

void CheckMessageTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    size_t payload_len = sizeof(PAYLOAD);

    ctx.GetInetLayer().SystemLayer()->Init(NULL);

    chip::System::PacketBuffer * buffer = chip::System::PacketBuffer::NewWithAvailableSize(payload_len);
    memmove(buffer->Start(), PAYLOAD, payload_len);
    buffer->SetDataLength(payload_len);

    IPAddress addr;
    IPAddress::FromString("127.0.0.1", addr);
    CHIP_ERROR err = CHIP_NO_ERROR;

    SecureSessionMgr<LoopbackTransport> conn;

    err = conn.Init(kSourceNodeId, ctx.GetInetLayer().SystemLayer(), "LOOPBACK");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    callback.mSuite = inSuite;

    conn.SetDelegate(&callback);

    SecurePairingUsingTestSecret pairing1, pairing2;
    Optional<Transport::PeerAddress> peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));

    err = conn.NewPairing(Optional<NodeId>::Value(kSourceNodeId), peer, 1, 2, &pairing1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = conn.NewPairing(Optional<NodeId>::Value(kDestinationNodeId), peer, 2, 1, &pairing2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Should be able to send a message to itself by just calling send.
    callback.ReceiveHandlerCallCount = 0;

    err = conn.SendMessage(kDestinationNodeId, buffer);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ctx.DriveIOUntil(1000 /* ms */, []() { return callback.ReceiveHandlerCallCount != 0; });

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 1);
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("Simple Init Test",              CheckSimpleInitTest),
    NL_TEST_DEF("Message Self Test",             CheckMessageTest),

    NL_TEST_SENTINEL()
};
// clang-format on

int Initialize(void * aContext);
int Finalize(void * aContext);

// clang-format off
nlTestSuite sSuite =
{
    "Test-CHIP-Connection",
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
    CHIP_ERROR err = reinterpret_cast<TestContext *>(aContext)->Init(&sSuite);
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
int TestSecureSessionMgr()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, &sContext);

    return (nlTestRunnerStats(&sSuite));
}
