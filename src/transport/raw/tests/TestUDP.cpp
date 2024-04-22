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
#include <lib/support/UnitTestContext.h>  //++++
#include <transport/TransportMgr.h>
#include <transport/raw/UDP.h>

#include <gtest/gtest.h>

#include <errno.h>

using namespace chip;
using namespace chip::Inet;

namespace {

constexpr NodeId kSourceNodeId      = 123654;
constexpr NodeId kDestinationNodeId = 111222333;
constexpr uint32_t kMessageCounter  = 18;

using TestContext = chip::Test::IOContext;  //++++

const char PAYLOAD[]        = "Hello!";
int ReceiveHandlerCallCount = 0;

class MockTransportMgrDelegate : public TransportMgrDelegate
{
public:
    MockTransportMgrDelegate() {}
    ~MockTransportMgrDelegate() override {}

    void OnMessageReceived(const Transport::PeerAddress & source, System::PacketBufferHandle && msgBuf) override
    {
        PacketHeader packetHeader;

        CHIP_ERROR err = packetHeader.DecodeAndConsume(msgBuf);
        EXPECT_EQ(mSuite, err, CHIP_NO_ERROR);

        EXPECT_EQ(mSuite, packetHeader.GetSourceNodeId(), Optional<NodeId>::Value(kSourceNodeId));
        EXPECT_EQ(mSuite, packetHeader.GetDestinationNodeId(), Optional<NodeId>::Value(kDestinationNodeId));
        EXPECT_EQ(mSuite, packetHeader.GetMessageCounter(), kMessageCounter);

        size_t data_len = msgBuf->DataLength();
        int compare     = memcmp(msgBuf->Start(), PAYLOAD, data_len);
        EXPECT_EQ(mSuite, compare, 0);

        ReceiveHandlerCallCount++;
    }
};

} // namespace

/////////////////////////// Init test
struct TestUDP : public ::testing::Test
{
    static void SetUpTestSuite()
    {
        CHIP_ERROR err = reinterpret_cast<TestContext *>(aContext)->Init();  //++++
        EXPECT_EQ(error, CHIP_NO_ERROR);
    }

    static void TearDownTestSuite()
    {
        reinterpret_cast<TestContext *>(aContext)->Shutdown();  //++++
    }
};

void CheckSimpleInitTest(Inet::IPAddressType type)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);  //++++

    Transport::UDP udp;

    CHIP_ERROR err = udp.Init(Transport::UdpListenParameters(ctx.GetUDPEndPointManager()).SetAddressType(type).SetListenPort(0));

    EXPECT_EQ(err, CHIP_NO_ERROR);
}

void CheckMessageTest(const IPAddress & addr)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);  //++++

    uint16_t payload_len = sizeof(PAYLOAD);

    chip::System::PacketBufferHandle buffer = chip::System::PacketBufferHandle::NewWithData(PAYLOAD, payload_len);
    EXPECT_FALSE(buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    Transport::UDP udp;

    err = udp.Init(Transport::UdpListenParameters(ctx.GetUDPEndPointManager()).SetAddressType(addr.Type()).SetListenPort(0));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    MockTransportMgrDelegate gMockTransportMgrDelegate(inSuite);
    TransportMgrBase gTransportMgrBase;
    gTransportMgrBase.SetSessionManager(&gMockTransportMgrDelegate);
    gTransportMgrBase.Init(&udp);

    ReceiveHandlerCallCount = 0;

    PacketHeader header;
    header.SetSourceNodeId(kSourceNodeId).SetDestinationNodeId(kDestinationNodeId).SetMessageCounter(kMessageCounter);

    err = header.EncodeBeforeData(buffer);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Should be able to send a message to itself by just calling send.
    err = udp.SendMessage(Transport::PeerAddress::UDP(addr, udp.GetBoundPort()), std::move(buffer));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    ctx.DriveIOUntil(chip::System::Clock::Seconds16(1), []() { return ReceiveHandlerCallCount != 0; });

    EXPECT_EQ(ReceiveHandlerCallCount, 1);
}

#if INET_CONFIG_ENABLE_IPV4
TEST_F(TestUDP, CheckSimpleInitTest4)
{
    CheckSimpleInitTest(IPAddressType::kIPv4);
}

TEST_F(TestUDP, CheckMessageTest4)
{
    IPAddress addr;
    IPAddress::FromString("127.0.0.1", addr);
    CheckMessageTest(addr);
}
#endif

TEST_F(TestUDP, CheckSimpleInitTest6)
{
    CheckSimpleInitTest(IPAddressType::kIPv6);
}

TEST_F(TestUDP, CheckMessageTest6)
{
    IPAddress addr;
    IPAddress::FromString("::1", addr);
    CheckMessageTest(addr);
}
