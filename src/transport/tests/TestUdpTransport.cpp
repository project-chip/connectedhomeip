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
#include <transport/UdpTransport.h>

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

static const char PAYLOAD[] = "Hello!";

static void MessageReceiveHandler(UdpTransport * con, PacketBuffer * msgBuf, const IPPacketInfo * pktInfo)
{
    size_t data_len = msgBuf->DataLength();

    int compare = memcmp(msgBuf->Start(), PAYLOAD, data_len);
    NL_TEST_ASSERT(reinterpret_cast<nlTestSuite *>(con->AppState), compare == 0);
};

static void ReceiveErrorHandler(UdpTransport * con, CHIP_ERROR err, const IPPacketInfo * pktInfo)
{
    NL_TEST_ASSERT(reinterpret_cast<nlTestSuite *>(con->AppState), false);
};

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

    UdpTransport conn;
    conn.Init(&ctx.mInetLayer);
    CHIP_ERROR err = conn.Close();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

void CheckSimpleConnectTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    IPAddress addr;
    IPAddress::FromString("127.0.0.1", addr);
    CHIP_ERROR err = CHIP_NO_ERROR;

    UdpTransport conn;
    conn.Init(&ctx.mInetLayer);
    err = conn.Connect(addr, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = conn.Connect(addr, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INCORRECT_STATE);

    err = conn.Close();
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

    UdpTransport conn;
    conn.Init(&ctx.mInetLayer);
    conn.AppState          = inSuite;
    conn.OnMessageReceived = MessageReceiveHandler;
    conn.OnReceiveError    = ReceiveErrorHandler;

    err = conn.Connect(addr, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Should be able to send a message to itself by just calling send.
    conn.SendMessage(buffer);

    // allow the send and recv enough time
    DriveIO(ctx);
    sleep(1);
    DriveIO(ctx);

    err = conn.Close();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("Simple Init Test",              CheckSimpleInitTest),
    NL_TEST_DEF("Simple Connect Test",           CheckSimpleConnectTest),
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
int TestUdpTransport()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, &sContext);

    return (nlTestRunnerStats(&sSuite));
}
