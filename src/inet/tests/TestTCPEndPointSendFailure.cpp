/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <pw_unit_test/framework.h>

#include <inet/IPAddress.h>
#include <inet/InetLayer.h>
#include <inet/TCPEndPoint.h>
#include <inet/TCPEndPointImpl.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <system/SystemLayerImpl.h>
#include <system/SystemPacketBuffer.h>

#include <errno.h>
#include <random>

using namespace chip;
using namespace chip::Inet;

namespace {

class WriteRequestFailingLayer : public System::LayerImpl
{
public:
    CHIP_ERROR RequestCallbackOnPendingWrite(System::SocketWatchToken token) override
    {
        if (mFailNextWriteRequest)
        {
            mFailNextWriteRequest = false;
            return CHIP_ERROR_NO_MEMORY;
        }
        return System::LayerImpl::RequestCallbackOnPendingWrite(token);
    }

    bool mFailNextWriteRequest = false;
};

WriteRequestFailingLayer gLayer;
TCPEndPointManagerImpl gTCPManager;
TCPEndPointHandle gAccepted;
bool gConnectComplete   = false;
bool gClosed            = false;
CHIP_ERROR gClosedError = CHIP_NO_ERROR;
size_t gReceivedLength  = 0;

void ServiceEvents()
{
#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    gLayer.HandleDispatchQueueEvents(System::Clock::Milliseconds32(10));
#else
    SuccessOrDie(gLayer.StartTimer(
        System::Clock::Milliseconds32(10), [](System::Layer *, void *) {}, nullptr));
    gLayer.PrepareEvents();
    gLayer.WaitForEvents();
    gLayer.HandleEvents();
#endif
}

template <typename Predicate>
bool ServiceEventsUntil(Predicate done)
{
    for (int i = 0; i < 500 && !done(); i++)
    {
        ServiceEvents();
    }
    return done();
}

class TestTCPEndPointSendFailure : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
        ASSERT_EQ(gLayer.Init(), CHIP_NO_ERROR);
        ASSERT_EQ(gTCPManager.Init(gLayer), CHIP_NO_ERROR);
    }

    static void TearDownTestSuite()
    {
        gTCPManager.Shutdown();
        gLayer.Shutdown();
        Platform::MemoryShutdown();
    }

protected:
    void SetUp() override
    {
        gAccepted        = TCPEndPointHandle();
        gConnectComplete = false;
        gClosed          = false;
        gClosedError     = CHIP_NO_ERROR;
        gReceivedLength  = 0;

        IPAddress loopback;
        ASSERT_TRUE(IPAddress::FromString("::1", loopback));

        ASSERT_EQ(gTCPManager.NewEndPoint(mListener), CHIP_NO_ERROR);
        mListener->OnConnectionReceived = [](const TCPEndPointHandle &, const TCPEndPointHandle & conEndPoint, const IPAddress &,
                                             uint16_t) { gAccepted = conEndPoint; };
        uint16_t port                   = static_cast<uint16_t>(49152 + std::random_device{}() % 8192);
        CHIP_ERROR bindErr              = mListener->Bind(IPAddressType::kIPv6, loopback, port);
        for (int attempt = 0; attempt < 64 && bindErr == CHIP_ERROR_POSIX(EADDRINUSE); attempt++)
        {
            bindErr = mListener->Bind(IPAddressType::kIPv6, loopback, ++port);
        }
        ASSERT_EQ(bindErr, CHIP_NO_ERROR);
        ASSERT_EQ(mListener->Listen(1), CHIP_NO_ERROR);

        ASSERT_EQ(gTCPManager.NewEndPoint(mClient), CHIP_NO_ERROR);
        mClient->OnConnectComplete  = [](const TCPEndPointHandle &, CHIP_ERROR err) { gConnectComplete = (err == CHIP_NO_ERROR); };
        mClient->OnConnectionClosed = [](const TCPEndPointHandle &, CHIP_ERROR err) {
            gClosed      = true;
            gClosedError = err;
        };
        ASSERT_EQ(mClient->Connect(loopback, port), CHIP_NO_ERROR);
        ASSERT_TRUE(ServiceEventsUntil([] { return gConnectComplete && !gAccepted.IsNull(); }));
    }

    void TearDown() override
    {
        gLayer.mFailNextWriteRequest = false;
        for (TCPEndPointHandle * endPoint : { &mClient, &gAccepted, &mListener })
        {
            if (!endPoint->IsNull())
            {
                (*endPoint)->Abort();
            }
            *endPoint = TCPEndPointHandle();
        }
    }

    static System::PacketBufferHandle MakePayload() { return System::PacketBufferHandle::NewWithData("ping", 4); }

    TCPEndPointHandle mListener;
    TCPEndPointHandle mClient;
};

TEST_F(TestTCPEndPointSendFailure, FailedWriteCallbackRequestClosesConnection)
{
    ASSERT_EQ(mClient->PendingSendLength(), 0u);

    gLayer.mFailNextWriteRequest = true;
    EXPECT_EQ(mClient->Send(MakePayload()), CHIP_ERROR_NO_MEMORY);

    EXPECT_FALSE(mClient->IsConnected());
    EXPECT_EQ(mClient->PendingSendLength(), 0u);
    EXPECT_TRUE(gClosed);
    EXPECT_EQ(gClosedError, CHIP_ERROR_NO_MEMORY);
}

TEST_F(TestTCPEndPointSendFailure, FailedWriteCallbackRequestWithoutPushClosesConnection)
{
    ASSERT_EQ(mClient->PendingSendLength(), 0u);

    gLayer.mFailNextWriteRequest = true;
    EXPECT_EQ(mClient->Send(MakePayload(), /* push = */ false), CHIP_ERROR_NO_MEMORY);

    EXPECT_FALSE(mClient->IsConnected());
    EXPECT_EQ(mClient->PendingSendLength(), 0u);
    EXPECT_TRUE(gClosed);
    EXPECT_EQ(mClient->Send(MakePayload()), CHIP_ERROR_INCORRECT_STATE);
}

TEST_F(TestTCPEndPointSendFailure, SendWithSuccessfulRequestDeliversData)
{
    gAccepted->OnDataReceived = [](const TCPEndPointHandle &, System::PacketBufferHandle && data) {
        gReceivedLength += data->TotalLength();
        return CHIP_NO_ERROR;
    };

    ASSERT_EQ(mClient->Send(MakePayload()), CHIP_NO_ERROR);
    EXPECT_TRUE(ServiceEventsUntil([] { return gReceivedLength == 4; }));
    EXPECT_EQ(mClient->PendingSendLength(), 0u);
    EXPECT_TRUE(mClient->IsConnected());
    EXPECT_FALSE(gClosed);
}

} // namespace
