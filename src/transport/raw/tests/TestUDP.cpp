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
 *      This file implements unit tests for the UdpTransport implementation.
 */

#include "NetworkTestHelpers.h"

#include <lib/core/CHIPCore.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <transport/TransportMgr.h>
#include <transport/raw/UDP.h>

#include <nlunit-test.h>

#include <errno.h>

using namespace chip;
using namespace chip::Inet;

static int Initialize(void * aContext);
static int Finalize(void * aContext);

namespace {

constexpr NodeId kSourceNodeId      = 123654;
constexpr NodeId kDestinationNodeId = 111222333;
constexpr uint32_t kMessageCounter  = 18;

using TestContext = chip::Test::IOContext;

const char PAYLOAD[]        = "Hello!";
int ReceiveHandlerCallCount = 0;

class MockTransportMgrDelegate : public TransportMgrDelegate
{
public:
    MockTransportMgrDelegate(nlTestSuite * inSuite) : mSuite(inSuite) {}
    ~MockTransportMgrDelegate() override {}

    void OnMessageReceived(const Transport::PeerAddress & source, System::PacketBufferHandle && msgBuf) override
    {
        PacketHeader packetHeader;

        CHIP_ERROR err = packetHeader.DecodeAndConsume(msgBuf);
        NL_TEST_ASSERT(mSuite, err == CHIP_NO_ERROR);

        NL_TEST_ASSERT(mSuite, packetHeader.GetSourceNodeId() == Optional<NodeId>::Value(kSourceNodeId));
        NL_TEST_ASSERT(mSuite, packetHeader.GetDestinationNodeId() == Optional<NodeId>::Value(kDestinationNodeId));
        NL_TEST_ASSERT(mSuite, packetHeader.GetMessageCounter() == kMessageCounter);

        size_t data_len = msgBuf->DataLength();
        int compare     = memcmp(msgBuf->Start(), PAYLOAD, data_len);
        NL_TEST_ASSERT(mSuite, compare == 0);

        ReceiveHandlerCallCount++;
    }

private:
    nlTestSuite * mSuite;
};

} // namespace

/////////////////////////// Init test

void CheckSimpleInitTest(nlTestSuite * inSuite, void * inContext, Inet::IPAddressType type)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    Transport::UDP udp;

    CHIP_ERROR err = udp.Init(Transport::UdpListenParameters(ctx.GetUDPEndPointManager()).SetAddressType(type).SetListenPort(0));

    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

#if INET_CONFIG_ENABLE_IPV4
void CheckSimpleInitTest4(nlTestSuite * inSuite, void * inContext)
{
    CheckSimpleInitTest(inSuite, inContext, IPAddressType::kIPv4);
}
#endif

void CheckSimpleInitTest6(nlTestSuite * inSuite, void * inContext)
{
    CheckSimpleInitTest(inSuite, inContext, IPAddressType::kIPv6);
}

/////////////////////////// Messaging test

void CheckMessageTest(nlTestSuite * inSuite, void * inContext, const IPAddress & addr)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    uint16_t payload_len = sizeof(PAYLOAD);

    chip::System::PacketBufferHandle buffer = chip::System::PacketBufferHandle::NewWithData(PAYLOAD, payload_len);
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    Transport::UDP udp;

    err = udp.Init(Transport::UdpListenParameters(ctx.GetUDPEndPointManager()).SetAddressType(addr.Type()).SetListenPort(0));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    MockTransportMgrDelegate gMockTransportMgrDelegate(inSuite);
    TransportMgrBase gTransportMgrBase;
    gTransportMgrBase.SetSessionManager(&gMockTransportMgrDelegate);
    gTransportMgrBase.Init(&udp);

    ReceiveHandlerCallCount = 0;

    PacketHeader header;
    header.SetSourceNodeId(kSourceNodeId).SetDestinationNodeId(kDestinationNodeId).SetMessageCounter(kMessageCounter);

    err = header.EncodeBeforeData(buffer);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Should be able to send a message to itself by just calling send.
    err = udp.SendMessage(Transport::PeerAddress::UDP(addr, udp.GetBoundPort()), std::move(buffer));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ctx.DriveIOUntil(chip::System::Clock::Seconds16(1), []() { return ReceiveHandlerCallCount != 0; });

    NL_TEST_ASSERT(inSuite, ReceiveHandlerCallCount == 1);
}

void CheckMessageTest4(nlTestSuite * inSuite, void * inContext)
{
    IPAddress addr;
    IPAddress::FromString("127.0.0.1", addr);
    CheckMessageTest(inSuite, inContext, addr);
}

void CheckMessageTest6(nlTestSuite * inSuite, void * inContext)
{
    IPAddress addr;
    IPAddress::FromString("::1", addr);
    CheckMessageTest(inSuite, inContext, addr);
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
#if INET_CONFIG_ENABLE_IPV4
    NL_TEST_DEF("Simple Init Test IPV4",   CheckSimpleInitTest4),
    NL_TEST_DEF("Message Self Test IPV4",  CheckMessageTest4),
#endif

    NL_TEST_DEF("Simple Init Test IPV6",   CheckSimpleInitTest6),
    NL_TEST_DEF("Message Self Test IPV6",  CheckMessageTest6),

    NL_TEST_SENTINEL()
};
// clang-format on

// clang-format off
static nlTestSuite sSuite =
{
    "Test-CHIP-Udp",
    &sTests[0],
    Initialize,
    Finalize
};
// clang-format on

/**
 *  Initialize the test suite.
 */
static int Initialize(void * aContext)
{
    CHIP_ERROR err = reinterpret_cast<TestContext *>(aContext)->Init();
    return (err == CHIP_NO_ERROR) ? SUCCESS : FAILURE;
}

/**
 *  Finalize the test suite.
 */
static int Finalize(void * aContext)
{
    reinterpret_cast<TestContext *>(aContext)->Shutdown();
    return SUCCESS;
}

int TestUDP()
{
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestUDP);
