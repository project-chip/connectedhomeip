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

/**
 *    @file
 *      This file implements unit tests for the ExchangeContext implementation.
 */

#include <lib/core/CHIPCore.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <messaging/tests/MessagingContext.h>
#include <protocols/Protocols.h>
#include <transport/SessionManager.h>
#include <transport/TransportMgr.h>

#include <nlbyteorder.h>
#include <nlunit-test.h>

#include <errno.h>
#include <utility>

#if CHIP_CRYPTO_PSA
#include "psa/crypto.h"
#endif

namespace {

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::Messaging;

struct TestContext : Test::LoopbackMessagingContext
{
    // TODO Add TearDown function when changing test framework to Pigweed to make it more clear how it works.
    // Currently, the TearDown function is from LoopbackMessagingContext
    void SetUp() override
    {
#if CHIP_CRYPTO_PSA
        // TODO: use ASSERT_EQ, once transition to pw_unit_test is complete
        VerifyOrDie(psa_crypto_init() == PSA_SUCCESS);
#endif
        chip::Test::LoopbackMessagingContext::SetUp();
    }
};

enum : uint8_t
{
    kMsgType_TEST1 = 0xf0,
    kMsgType_TEST2 = 0xf1,
};

class MockExchangeDelegate : public UnsolicitedMessageHandler, public ExchangeDelegate
{
public:
    CHIP_ERROR OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, ExchangeDelegate *& newDelegate) override
    {
        newDelegate = this;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR OnMessageReceived(ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && buffer) override
    {
        ++mReceivedMessageCount;
        if (mKeepExchangeAliveOnMessageReceipt)
        {
            ec->WillSendMessage();
            mExchange = ec;
        }
        else
        {
            // Exchange will be closing, so don't hold on to a reference to it.
            mExchange = nullptr;
        }
        return CHIP_NO_ERROR;
    }

    void OnResponseTimeout(ExchangeContext * ec) override {}

    ExchangeMessageDispatch & GetMessageDispatch() override
    {
        if (mMessageDispatch != nullptr)
        {
            return *mMessageDispatch;
        }

        return ExchangeDelegate::GetMessageDispatch();
    }

    uint32_t mReceivedMessageCount             = 0;
    bool mKeepExchangeAliveOnMessageReceipt    = true;
    ExchangeContext * mExchange                = nullptr;
    ExchangeMessageDispatch * mMessageDispatch = nullptr;
};

// Helper used by several tests.  Registers delegate2 as an unsolicited message
// handler, sends a message of type requestMessageType via an exchange that has
// delegate1 as delegate, responds with responseMessageType.
template <typename AfterRequestChecker, typename AfterResponseChecker>
void DoRoundTripTest(nlTestSuite * inSuite, void * inContext, MockExchangeDelegate & delegate1, MockExchangeDelegate & delegate2,
                     uint8_t requestMessageType, uint8_t responseMessageType, AfterRequestChecker && afterRequestChecker,
                     AfterResponseChecker && afterResponseChecker)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    ExchangeContext * ec1 = ctx.NewExchangeToBob(&delegate1);
    NL_TEST_ASSERT(inSuite, ec1 != nullptr);

    CHIP_ERROR err = ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::Id,
                                                                                       requestMessageType, &delegate2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // To simplify things, skip MRP for all our messages, and make sure we are
    // always expecting responses.
    constexpr auto sendFlags =
        SendFlags(Messaging::SendMessageFlags::kNoAutoRequestAck, Messaging::SendMessageFlags::kExpectResponse);

    err = ec1->SendMessage(Protocols::SecureChannel::Id, requestMessageType,
                           System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize), sendFlags);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();

    afterRequestChecker();

    ExchangeContext * ec2 = delegate2.mExchange;
    err                   = ec2->SendMessage(Protocols::SecureChannel::Id, responseMessageType,
                                             System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize), sendFlags);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();

    afterResponseChecker();

    ec1->Close();
    ec2->Close();

    err = ctx.GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::Id, kMsgType_TEST1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

void CheckBasicMessageRoundTrip(nlTestSuite * inSuite, void * inContext)
{
    MockExchangeDelegate delegate1;
    MockExchangeDelegate delegate2;
    DoRoundTripTest(
        inSuite, inContext, delegate1, delegate2, kMsgType_TEST1, kMsgType_TEST2,
        [&] {
            NL_TEST_ASSERT(inSuite, delegate1.mReceivedMessageCount == 0);
            NL_TEST_ASSERT(inSuite, delegate2.mReceivedMessageCount == 1);
        },
        [&] {
            NL_TEST_ASSERT(inSuite, delegate1.mReceivedMessageCount == 1);
            NL_TEST_ASSERT(inSuite, delegate2.mReceivedMessageCount == 1);
        });
}

void CheckBasicExchangeMessageDispatch(nlTestSuite * inSuite, void * inContext)
{
    class MockMessageDispatch : public ExchangeMessageDispatch
    {
        bool MessagePermitted(Protocols::Id protocol, uint8_t type) override
        {
            // Only allow TEST1 messages.
            return protocol == Protocols::SecureChannel::Id && type == kMsgType_TEST1;
        }
    };

    MockMessageDispatch dispatch;

    {
        // Allowed response.
        MockExchangeDelegate delegate1;
        delegate1.mMessageDispatch = &dispatch;
        MockExchangeDelegate delegate2;

        DoRoundTripTest(
            inSuite, inContext, delegate1, delegate2, kMsgType_TEST1, kMsgType_TEST1,
            [&] {
                NL_TEST_ASSERT(inSuite, delegate1.mReceivedMessageCount == 0);
                NL_TEST_ASSERT(inSuite, delegate2.mReceivedMessageCount == 1);
            },
            [&] {
                NL_TEST_ASSERT(inSuite, delegate1.mReceivedMessageCount == 1);
                NL_TEST_ASSERT(inSuite, delegate2.mReceivedMessageCount == 1);
            });
    }

    {
        // Disallowed response.
        MockExchangeDelegate delegate1;
        delegate1.mMessageDispatch = &dispatch;
        MockExchangeDelegate delegate2;

        DoRoundTripTest(
            inSuite, inContext, delegate1, delegate2, kMsgType_TEST1, kMsgType_TEST2,
            [&] {
                NL_TEST_ASSERT(inSuite, delegate1.mReceivedMessageCount == 0);
                NL_TEST_ASSERT(inSuite, delegate2.mReceivedMessageCount == 1);
            },
            [&] {
                NL_TEST_ASSERT(inSuite, delegate1.mReceivedMessageCount == 0);
                NL_TEST_ASSERT(inSuite, delegate2.mReceivedMessageCount == 1);
            });
    }
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("Test ExchangeContext::SendMessage", CheckBasicMessageRoundTrip),
    NL_TEST_DEF("Test ExchangeMessageDispatch", CheckBasicExchangeMessageDispatch),

    NL_TEST_SENTINEL()
};
// clang-format on

// clang-format off
nlTestSuite sSuite =
{
    "Test-Exchange",
    &sTests[0],
    TestContext::nlTestSetUpTestSuite,
    TestContext::nlTestTearDownTestSuite,
    TestContext::nlTestSetUp,
    TestContext::nlTestTearDown,
};
// clang-format on

} // namespace

/**
 *  Main
 */
int TestExchange()
{
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestExchange);
