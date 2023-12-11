/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <lib/core/CHIPCore.h>
#include <lib/support/CHIPFaultInjection.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <messaging/tests/MessagingContext.h>
#include <protocols/Protocols.h>
#include <protocols/echo/Echo.h>
#include <transport/SessionManager.h>
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
using namespace chip::Protocols;
using namespace chip::System::Clock::Literals;

using TestContext = Test::UDPMessagingContext;

// The message timeout value in milliseconds.
constexpr System::Clock::Timeout kMessageTimeout = System::Clock::Milliseconds32(100);

class MockAppDelegate : public UnsolicitedMessageHandler, public ExchangeDelegate
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
        IsOnMessageReceivedCalled = true;
        return CHIP_NO_ERROR;
    }

    void OnResponseTimeout(ExchangeContext * ec) override { IsOnResponseTimeoutCalled = true; }

    bool IsOnMessageReceivedCalled = false;
    bool IsOnResponseTimeoutCalled = false;
};

/**
 * Tests sending exchange message with Success:
 *
 *      DUT = sender, PEER = remote device
 *
 * 1) DUT sends message w/o MRP to PEER
 *      - Confirm the message is sent successfully
 *      - Observe DUT response timeout with no response
 */
void CheckExchangeOutgoingMessagesSuccess(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    // create solicited exchange
    MockAppDelegate mockSolicitedAppDelegate;
    ExchangeContext * ec = ctx.NewExchangeToAlice(&mockSolicitedAppDelegate);

    NL_TEST_ASSERT(inSuite, ec != nullptr);
    ec->SetResponseTimeout(kMessageTimeout);

    CHIP_ERROR err = ec->SendMessage(Echo::MsgType::EchoRequest, System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize),
                                     SendFlags(SendMessageFlags::kExpectResponse).Set(SendMessageFlags::kNoAutoRequestAck));

    // Wait for the initial message to fail (should take 330-413ms)
    ctx.GetIOContext().DriveIOUntil(500_ms32, [&] { return mockSolicitedAppDelegate.IsOnMessageReceivedCalled; });

    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, mockSolicitedAppDelegate.IsOnResponseTimeoutCalled);
}

/**
 * Tests sending exchange message with Failure:
 *
 *      DUT = sender, PEER = remote device
 *
 * 1) DUT configured to drop the outgoing UDP packet
 * 2) DUT sends message w/o MRP to PEER
 *      - Confirm the message is sent with failure
 *      - Confirm the DUT response timeout timer is cancelled
 */
void CheckExchangeOutgoingMessagesFail(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    // create solicited exchange
    MockAppDelegate mockSolicitedAppDelegate;
    ExchangeContext * ec = ctx.NewExchangeToAlice(&mockSolicitedAppDelegate);

    NL_TEST_ASSERT(inSuite, ec != nullptr);
    ec->SetResponseTimeout(kMessageTimeout);

    chip::FaultInjection::GetManager().FailAtFault(chip::FaultInjection::kFault_DropOutgoingUDPMsg, 0, 1);

    CHIP_ERROR err = ec->SendMessage(Echo::MsgType::EchoRequest, System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize),
                                     SendFlags(SendMessageFlags::kExpectResponse).Set(SendMessageFlags::kNoAutoRequestAck));

    // Wait for the initial message to fail (should take 330-413ms)
    ctx.GetIOContext().DriveIOUntil(500_ms32, [&] { return mockSolicitedAppDelegate.IsOnMessageReceivedCalled; });

    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !mockSolicitedAppDelegate.IsOnResponseTimeoutCalled);
    ec->Close();
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("Test MessagingLayer::ExchangeOutgoingMessagesSuccess", CheckExchangeOutgoingMessagesSuccess),
    NL_TEST_DEF("Test MessagingLayer::ExchangeOutgoingMessagesFail", CheckExchangeOutgoingMessagesFail),

    NL_TEST_SENTINEL()
};
// clang-format on

// clang-format off
nlTestSuite sSuite =
{
    "Test-CHIP-MessagingLayer",
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
int TestMessagingLayer()
{
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestMessagingLayer);
