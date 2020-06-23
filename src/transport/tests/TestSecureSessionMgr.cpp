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

#include <core/CHIPCore.h>
#include <support/CodeUtils.h>
#include <transport/SecureSessionMgr.h>

#include <nlbyteorder.h>
#include <nlunit-test.h>

#include <errno.h>

using namespace chip;

static int Initialize(void * aContext);
static int Finalize(void * aContext);

struct TestContext
{
    nlTestSuite * mSuite;
    System::Layer mSystemLayer;
    InetLayer mInetLayer;
};

struct TestContext sContext;

static const unsigned char local_private_key[] = { 0x00, 0xd1, 0x90, 0xd9, 0xb3, 0x95, 0x1c, 0x5f, 0xa4, 0xe7, 0x47,
                                                   0x92, 0x5b, 0x0a, 0xa9, 0xa7, 0xc1, 0x1c, 0xe7, 0x06, 0x10, 0xe2,
                                                   0xdd, 0x16, 0x41, 0x52, 0x55, 0xb7, 0xb8, 0x80, 0x8d, 0x87, 0xa1 };

static const unsigned char remote_public_key[] = { 0x04, 0xe2, 0x07, 0x64, 0xff, 0x6f, 0x6a, 0x91, 0xd9, 0xc2, 0xc3, 0x0a, 0xc4,
                                                   0x3c, 0x56, 0x4b, 0x42, 0x8a, 0xf3, 0xb4, 0x49, 0x29, 0x39, 0x95, 0xa2, 0xf7,
                                                   0x02, 0x8c, 0xa5, 0xce, 0xf3, 0xc9, 0xca, 0x24, 0xc5, 0xd4, 0x5c, 0x60, 0x79,
                                                   0x48, 0x30, 0x3c, 0x53, 0x86, 0xd9, 0x23, 0xe6, 0x61, 0x1f, 0x5a, 0x3d, 0xdf,
                                                   0x9f, 0xdc, 0x35, 0xea, 0xd0, 0xde, 0x16, 0x7e, 0x64, 0xde, 0x7f, 0x3c, 0xa6 };

static const char PAYLOAD[]         = "Hello!";
constexpr NodeId kSourceNodeId      = 123654;
constexpr NodeId kDestinationNodeId = 111222333;

int ReceiveHandlerCallCount = 0;

static void MessageReceiveHandler(const MessageHeader & header, Transport::PeerConnectionState * state,
                                  System::PacketBuffer * msgBuf, nlTestSuite * inSuite)
{
    NL_TEST_ASSERT(inSuite, header.GetSourceNodeId() == Optional<NodeId>::Value(kSourceNodeId));
    NL_TEST_ASSERT(inSuite, header.GetDestinationNodeId() == Optional<NodeId>::Value(kDestinationNodeId));
    NL_TEST_ASSERT(inSuite, state->GetPeerNodeId() == kDestinationNodeId);

    size_t data_len = msgBuf->DataLength();

    int compare = memcmp(msgBuf->Start(), PAYLOAD, data_len);
    NL_TEST_ASSERT(inSuite, compare == 0);

    ReceiveHandlerCallCount++;
}

int NewConnectionHandlerCallCount = 0;
static void NewConnectionHandler(Transport::PeerConnectionState * state, nlTestSuite * inSuite)
{
    CHIP_ERROR err;

    NewConnectionHandlerCallCount++;

    err = state->GetSecureSession().TemporaryManualKeyExchange(remote_public_key, sizeof(remote_public_key), local_private_key,
                                                               sizeof(local_private_key));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

static void DriveIO(TestContext & ctx)
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

static CHIP_ERROR InitLayers(System::Layer & systemLayer, InetLayer & inetLayer)
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

    SecureSessionMgr conn;
    CHIP_ERROR err;

    err = conn.Init(kSourceNodeId, &ctx.mInetLayer, Transport::UdpListenParameters());
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

    SecureSessionMgr conn;

    err = conn.Init(kSourceNodeId, &ctx.mInetLayer, Transport::UdpListenParameters().SetAddressType(addr.Type()));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    conn.SetNewConnectionHandler(NewConnectionHandler, inSuite);
    conn.SetMessageReceiveHandler(MessageReceiveHandler, inSuite);

    NewConnectionHandlerCallCount = 0;
    err                           = conn.Connect(kDestinationNodeId, Transport::PeerAddress::UDP(addr));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, NewConnectionHandlerCallCount == 1);

    // Should be able to send a message to itself by just calling send.
    ReceiveHandlerCallCount = 0;
    err                     = conn.SendMessage(kDestinationNodeId, buffer);
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
    "Test-CHIP-Connection",
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
int TestSecureSessionMgr()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, &sContext);

    return (nlTestRunnerStats(&sSuite));
}
