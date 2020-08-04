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
 *      This file implements unit tests for the UdpTransport implementation.
 */

#include "TestTransportLayer.h"

#include "NetworkTestHelpers.h"

#include <core/CHIPCore.h>
#include <support/CodeUtils.h>
#include <transport/UDP.h>

#include <nlbyteorder.h>
#include <nlunit-test.h>

#include <errno.h>

using namespace chip;

static int Initialize(void * aContext);
static int Finalize(void * aContext);

namespace {

constexpr NodeId kSourceNodeId      = 123654;
constexpr NodeId kDestinationNodeId = 111222333;
constexpr uint32_t kMessageId       = 18;

using TestContext = chip::Test::IOContext;
TestContext sContext;

const char PAYLOAD[]        = "Hello!";
int ReceiveHandlerCallCount = 0;

void MessageReceiveHandler(MessageHeader & header, const Transport::PeerAddress & source, System::PacketBuffer * msgBuf,
                           nlTestSuite * inSuite)
{
    NL_TEST_ASSERT(inSuite, header.GetSourceNodeId() == Optional<NodeId>::Value(kSourceNodeId));
    NL_TEST_ASSERT(inSuite, header.GetDestinationNodeId() == Optional<NodeId>::Value(kDestinationNodeId));
    NL_TEST_ASSERT(inSuite, header.GetMessageId() == kMessageId);

    size_t data_len = msgBuf->DataLength();
    int compare     = memcmp(msgBuf->Start(), PAYLOAD, data_len);
    NL_TEST_ASSERT(inSuite, compare == 0);

    ReceiveHandlerCallCount++;
}

} // namespace

/////////////////////////// Init test

void CheckSimpleInitTest(nlTestSuite * inSuite, void * inContext, Inet::IPAddressType type)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    Transport::UDP udp;

    CHIP_ERROR err = udp.Init(Transport::UdpListenParameters(&ctx.GetInetLayer()).SetAddressType(type));

    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

#if INET_CONFIG_ENABLE_IPV4
void CheckSimpleInitTest4(nlTestSuite * inSuite, void * inContext)
{
    CheckSimpleInitTest(inSuite, inContext, kIPAddressType_IPv4);
}
#endif

void CheckSimpleInitTest6(nlTestSuite * inSuite, void * inContext)
{
    CheckSimpleInitTest(inSuite, inContext, kIPAddressType_IPv6);
}

/////////////////////////// Messaging test

void CheckMessageTest(nlTestSuite * inSuite, void * inContext, const IPAddress & addr)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    size_t payload_len = sizeof(PAYLOAD);

    chip::System::PacketBuffer * buffer = chip::System::PacketBuffer::NewWithAvailableSize(payload_len);
    memmove(buffer->Start(), PAYLOAD, payload_len);
    buffer->SetDataLength(payload_len);

    CHIP_ERROR err = CHIP_NO_ERROR;

    Transport::UDP udp;

    err = udp.Init(Transport::UdpListenParameters(&ctx.GetInetLayer()).SetAddressType(addr.Type()));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    udp.SetMessageReceiveHandler(MessageReceiveHandler, inSuite);
    ReceiveHandlerCallCount = 0;

    MessageHeader header;
    header.SetSourceNodeId(kSourceNodeId).SetDestinationNodeId(kDestinationNodeId).SetMessageId(kMessageId);

    // Should be able to send a message to itself by just calling send.
    err = udp.SendMessage(header, Transport::PeerAddress::UDP(addr), buffer);
    if (err == System::MapErrorPOSIX(EADDRNOTAVAIL))
    {
        // TODO: the underlying system does not support IPV6. This early return should
        // be removed and error should be made fatal.
        printf("%s:%u: System does NOT support IPV6.\n", __FILE__, __LINE__);
        return;
    }

    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ctx.DriveIOUntil(1000 /* ms */, []() { return ReceiveHandlerCallCount != 0; });

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
    CHIP_ERROR err = reinterpret_cast<TestContext *>(aContext)->Init(&sSuite);
    return (err == CHIP_NO_ERROR) ? SUCCESS : FAILURE;
}

/**
 *  Finalize the test suite.
 */
static int Finalize(void * aContext)
{
    CHIP_ERROR err = reinterpret_cast<TestContext *>(aContext)->Shutdown();
    return (err == CHIP_NO_ERROR) ? SUCCESS : FAILURE;
}

/**
 *  Main
 */
int TestUDP()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, &sContext);

    return (nlTestRunnerStats(&sSuite));
}
