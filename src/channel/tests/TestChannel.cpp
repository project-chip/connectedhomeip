/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      This file implements unit tests for the ExchangeManager implementation.
 */

#include "TestMessagingLayer.h"

#include <lib/core/CHIPCore.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <messaging/tests/MessagingContext.h>
#include <protocols/Protocols.h>
#include <transport/SecureSessionMgr.h>
#include <transport/TransportMgr.h>
#include <transport/raw/tests/NetworkTestHelpers.h>

#include <nlbyteorder.h>
#include <nlunit-test.h>

#include <errno.h>
#include <utility>

namespace {

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::Messaging;

using TestContext = chip::Test::MessagingContext;

TestContext sContext;

TransportMgr<Test::LoopbackTransport> gTransportMgr;
chip::Test::IOContext gIOContext;

class MockAppDelegate : public ExchangeDelegate
{
public:
    CHIP_ERROR OnMessageReceived(ExchangeContext * ec, const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && buffer) override
    {
        IsOnMessageReceivedCalled = true;
        return CHIP_NO_ERROR;
    }

    void OnResponseTimeout(ExchangeContext * ec) override {}

    bool IsOnMessageReceivedCalled = false;
};

class MockChannelDelegate : public ChannelDelegate
{
public:
    ~MockChannelDelegate() override {}

    void OnEstablished() override {}
    void OnClosed() override {}
    void OnFail(CHIP_ERROR err) override {}
};

void CheckExchangeChannels(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    // create unsolicited exchange
    MockAppDelegate mockUnsolicitedAppDelegate;
    CHIP_ERROR err = ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Protocols::Id(VendorId::Common, 0x0001),
                                                                                       0x0001, &mockUnsolicitedAppDelegate);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // create the channel
    ChannelBuilder channelBuilder;
    channelBuilder.SetPeerNodeId(ctx.GetDestinationNodeId())
        .SetForcePeerAddress(ctx.GetAddress())
        .SetOperationalCredentialSet(&ctx.GetOperationalCredentialSet());
    MockChannelDelegate channelDelegate;
    auto channelHandle = ctx.GetExchangeManager().EstablishChannel(channelBuilder, &channelDelegate);
    return;

#if 0
    // TODO: complete test when CASESession is completed
    // wait for channel establishment
    ctx.DriveIOUntil(1000, [&] { return channelHandle.GetState() == ChannelState::kReady; });
    NL_TEST_ASSERT(inSuite, channelHandle.GetState() == ChannelState::kReady);

    MockAppDelegate mockAppDelegate;
    ExchangeContext * ec1 = channelHandle.NewExchange(&mockAppDelegate);

    // send a malicious packet
    ec1->SendMessage(0x0001, 0x0002, System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize));
    NL_TEST_ASSERT(inSuite, !mockUnsolicitedAppDelegate.IsOnMessageReceivedCalled);

    // Need to sort out what this test should really be testing and how; sending
    // two messages in a row on an exchange is not something that really
    // happens.

    // send a good packet
    ec1->SendMessage(0x0001, 0x0001, System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize));
    NL_TEST_ASSERT(inSuite, mockUnsolicitedAppDelegate.IsOnMessageReceivedCalled);

    channelHandle.Release();
#endif
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("Test Channel/Exchange",               CheckExchangeChannels),

    NL_TEST_SENTINEL()
};
// clang-format on

int Initialize(void * aContext);
int Finalize(void * aContext);

// clang-format off
nlTestSuite sSuite =
{
    "Test-CHIP-ExchangeManager",
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
    // Initialize System memory and resources
    VerifyOrReturnError(chip::Platform::MemoryInit() == CHIP_NO_ERROR, FAILURE);
    VerifyOrReturnError(gIOContext.Init(&sSuite) == CHIP_NO_ERROR, FAILURE);
    VerifyOrReturnError(gTransportMgr.Init("LOOPBACK") == CHIP_NO_ERROR, FAILURE);

    auto * ctx = static_cast<TestContext *>(aContext);
    VerifyOrReturnError(ctx->Init(&sSuite, &gTransportMgr, &gIOContext) == CHIP_NO_ERROR, FAILURE);

    return CHIP_NO_ERROR;
}

/**
 *  Finalize the test suite.
 */
int Finalize(void * aContext)
{
    CHIP_ERROR err = reinterpret_cast<TestContext *>(aContext)->Shutdown();
    gIOContext.Shutdown();
    chip::Platform::MemoryShutdown();
    return (err == CHIP_NO_ERROR) ? SUCCESS : FAILURE;
}

} // namespace

/**
 *  Main
 */
int TestChannel()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, &sContext);

    return (nlTestRunnerStats(&sSuite));
}
