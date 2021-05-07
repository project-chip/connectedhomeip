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

#include <core/CHIPCore.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <messaging/tests/MessagingContext.h>
#include <protocols/Protocols.h>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/UnitTestRegistration.h>
#include <transport/SecureSessionMgr.h>
#include <transport/TransportMgr.h>

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

class LoopbackTransport : public Transport::Base
{
public:
    /// Transports are required to have a constructor that takes exactly one argument
    CHIP_ERROR Init(const char * unused) { return CHIP_NO_ERROR; }

    CHIP_ERROR SendMessage(const PacketHeader & header, const PeerAddress & address, System::PacketBufferHandle msgBuf) override
    {
        HandleMessageReceived(header, address, std::move(msgBuf));
        return CHIP_NO_ERROR;
    }

    bool CanSendToPeer(const PeerAddress & address) override { return true; }
};

TransportMgr<LoopbackTransport> gTransportMgr;

class MockAppDelegate : public ExchangeDelegate
{
public:
    void OnMessageReceived(ExchangeContext * ec, const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
                           System::PacketBufferHandle buffer) override
    {
        IsOnMessageReceivedCalled = true;
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
    ec1->SendMessage(0x0001, 0x0002, System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize),
                     SendFlags(Messaging::SendMessageFlags::kNone));
    NL_TEST_ASSERT(inSuite, !mockUnsolicitedAppDelegate.IsOnMessageReceivedCalled);

    // send a good packet
    ec1->SendMessage(0x0001, 0x0001, System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize),
                     SendFlags(Messaging::SendMessageFlags::kNone));
    NL_TEST_ASSERT(inSuite, mockUnsolicitedAppDelegate.IsOnMessageReceivedCalled);

    ec1->Close();
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
    CHIP_ERROR err = chip::Platform::MemoryInit();
    if (err != CHIP_NO_ERROR)
        return FAILURE;

    err = gTransportMgr.Init("LOOPBACK");
    if (err != CHIP_NO_ERROR)
        return FAILURE;

    err = reinterpret_cast<TestContext *>(aContext)->Init(&sSuite, &gTransportMgr);
    return (err == CHIP_NO_ERROR) ? SUCCESS : FAILURE;
}

/**
 *  Finalize the test suite.
 */
int Finalize(void * aContext)
{
    CHIP_ERROR err = reinterpret_cast<TestContext *>(aContext)->Shutdown();
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

CHIP_REGISTER_TEST_SUITE(TestChannel)
