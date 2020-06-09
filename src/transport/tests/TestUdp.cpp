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

#include <core/CHIPCore.h>
#include <support/CodeUtils.h>
#include <transport/Udp.h>

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

struct TestContext
{
    nlTestSuite * mSuite;
    System::Layer mSystemLayer;
    InetLayer mInetLayer;
};

struct TestContext sContext;

const char PAYLOAD[]        = "Hello!";
int ReceiveHandlerCallCount = 0;

void MessageReceiveHandler(const MessageHeader & header, const Inet::IPPacketInfo & source, System::PacketBuffer * msgBuf,
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

void DriveIO(TestContext & ctx)
{
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    // Set the select timeout to 100ms
    struct timeval aSleepTime;
    aSleepTime.tv_sec  = 0;
    aSleepTime.tv_usec = 100 * 1000;

    fd_set readFDs, writeFDs, exceptFDs;
    int numFDs = 0;

    FD_ZERO(&readFDs);
    FD_ZERO(&writeFDs);
    FD_ZERO(&exceptFDs);

    if (ctx.mSystemLayer.State() == System::kLayerState_Initialized)
        ctx.mSystemLayer.PrepareSelect(numFDs, &readFDs, &writeFDs, &exceptFDs, aSleepTime);

    if (ctx.mInetLayer.State == Inet::InetLayer::kState_Initialized)
        ctx.mInetLayer.PrepareSelect(numFDs, &readFDs, &writeFDs, &exceptFDs, aSleepTime);

    int selectRes = select(numFDs, &readFDs, &writeFDs, &exceptFDs, &aSleepTime);
    if (selectRes < 0)
    {
        printf("select failed: %s\n", ErrorStr(System::MapErrorPOSIX(errno)));
        NL_TEST_ASSERT(ctx.mSuite, false);
        return;
    }

    if (ctx.mSystemLayer.State() == System::kLayerState_Initialized)
    {
        ctx.mSystemLayer.HandleSelectResult(selectRes, &readFDs, &writeFDs, &exceptFDs);
    }

    if (ctx.mInetLayer.State == Inet::InetLayer::kState_Initialized)
    {
        ctx.mInetLayer.HandleSelectResult(selectRes, &readFDs, &writeFDs, &exceptFDs);
    }
#endif
}

} // namespace

CHIP_ERROR InitLayers(System::Layer & systemLayer, InetLayer & inetLayer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // Initialize the CHIP System Layer.
    err = systemLayer.Init(NULL);
    SuccessOrExit(err);

    // Initialize the CHIP Inet layer.
    err = inetLayer.Init(systemLayer, NULL);
    SuccessOrExit(err);

exit:
    return err;
}

void CheckSimpleInitTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    Transport::Udp udp;

    CHIP_ERROR err = udp.Init(&ctx.mInetLayer, kIPAddressType_IPv4, CHIP_PORT, CHIP_PORT);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

void CheckMessageTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    size_t payload_len = sizeof(PAYLOAD);

    chip::System::PacketBuffer * buffer = chip::System::PacketBuffer::NewWithAvailableSize(payload_len);
    memmove(buffer->Start(), PAYLOAD, payload_len);
    buffer->SetDataLength(payload_len);

    IPAddress addr;
    IPAddress::FromString("127.0.0.1", addr);
    CHIP_ERROR err = CHIP_NO_ERROR;

    Transport::Udp udp;

    err = udp.Init(&ctx.mInetLayer, kIPAddressType_IPv4, CHIP_PORT, CHIP_PORT);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    udp.SetMessageReceiveHandler(MessageReceiveHandler, inSuite);
    ReceiveHandlerCallCount = 0;

    MessageHeader header;
    header.SetSourceNodeId(kSourceNodeId).SetDestinationNodeId(kDestinationNodeId).SetMessageId(kMessageId);

    // Should be able to send a message to itself by just calling send.
    err = udp.SendMessage(header, addr, buffer);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // allow the send and recv enough time
    DriveIO(ctx);
    sleep(1);
    DriveIO(ctx);

    NL_TEST_ASSERT(inSuite, ReceiveHandlerCallCount == 1);
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("Simple Init Test",              CheckSimpleInitTest),
    NL_TEST_DEF("Message Self Test",             CheckMessageTest),

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
    TestContext & lContext = *reinterpret_cast<TestContext *>(aContext);

    CHIP_ERROR err = InitLayers(lContext.mSystemLayer, lContext.mInetLayer);
    if (err != CHIP_NO_ERROR)
    {
        return FAILURE;
    }
    lContext.mSuite = &sSuite;

    return SUCCESS;
}

/**
 *  Finalize the test suite.
 */
static int Finalize(void * aContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    TestContext & lContext = *reinterpret_cast<TestContext *>(aContext);

    lContext.mSuite = NULL;

    err = lContext.mSystemLayer.Shutdown();
    if (err != CHIP_NO_ERROR)
    {
        return FAILURE;
    }
    err = lContext.mInetLayer.Shutdown();
    if (err != CHIP_NO_ERROR)
    {
        return FAILURE;
    }
    return SUCCESS;
}

/**
 *  Main
 */
int TestUdp()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, &sContext);

    return (nlTestRunnerStats(&sSuite));
}
