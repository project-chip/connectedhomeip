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

#include <errno.h>

#include <pw_unit_test/framework.h>

#include <lib/core/CHIPCore.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CodeUtils.h>
#include <transport/TransportMgr.h>
#include <transport/raw/UDP.h>

using namespace chip;
using namespace chip::Test;
using namespace chip::Inet;

namespace {

constexpr NodeId kSourceNodeId      = 123654;
constexpr NodeId kDestinationNodeId = 111222333;
constexpr uint32_t kMessageCounter  = 18;

const char PAYLOAD[]        = "Hello!";
int ReceiveHandlerCallCount = 0;

class MockTransportMgrDelegate : public TransportMgrDelegate
{
public:
    MockTransportMgrDelegate() {}
    ~MockTransportMgrDelegate() override {}

    void OnMessageReceived(const Transport::PeerAddress & source, System::PacketBufferHandle && msgBuf,
                           Transport::MessageTransportContext * transCtxt = nullptr) override
    {
        PacketHeader packetHeader;

        CHIP_ERROR err = packetHeader.DecodeAndConsume(msgBuf);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        EXPECT_EQ(packetHeader.GetSourceNodeId(), Optional<NodeId>::Value(kSourceNodeId));
        EXPECT_EQ(packetHeader.GetDestinationNodeId(), Optional<NodeId>::Value(kDestinationNodeId));
        EXPECT_EQ(packetHeader.GetMessageCounter(), kMessageCounter);

        size_t data_len = msgBuf->DataLength();
        EXPECT_EQ(0, memcmp(msgBuf->Start(), PAYLOAD, data_len));

        ReceiveHandlerCallCount++;
    }
};

} // namespace

class TestUDP : public ::testing::Test
{
public:
    // Performs shared setup for all tests in the test suite
    static void SetUpTestSuite()
    {
        if (mIOContext == nullptr)
        {
            mIOContext = new IOContext();
            ASSERT_NE(mIOContext, nullptr);
        }
        ASSERT_EQ(mIOContext->Init(), CHIP_NO_ERROR);
    }

    // Performs shared teardown for all tests in the test suite
    static void TearDownTestSuite()
    {
        if (mIOContext != nullptr)
        {
            mIOContext->Shutdown();
            delete mIOContext;
            mIOContext = nullptr;
        }
    }

protected:
    static IOContext * mIOContext;

    void CheckSimpleInitTest(IPAddressType type)
    {
        Transport::UDP udp;

        CHIP_ERROR err =
            udp.Init(Transport::UdpListenParameters(mIOContext->GetUDPEndPointManager()).SetAddressType(type).SetListenPort(0));

        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    void CheckMessageTest(const IPAddress & addr)
    {
        uint16_t payload_len = sizeof(PAYLOAD);

        chip::System::PacketBufferHandle buffer = chip::System::PacketBufferHandle::NewWithData(PAYLOAD, payload_len);
        EXPECT_FALSE(buffer.IsNull());

        CHIP_ERROR err = CHIP_NO_ERROR;

        Transport::UDP udp;

        err = udp.Init(
            Transport::UdpListenParameters(mIOContext->GetUDPEndPointManager()).SetAddressType(addr.Type()).SetListenPort(0));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        MockTransportMgrDelegate gMockTransportMgrDelegate;
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

        mIOContext->DriveIOUntil(chip::System::Clock::Seconds16(1), []() { return ReceiveHandlerCallCount != 0; });

        EXPECT_EQ(ReceiveHandlerCallCount, 1);
    }
};

IOContext * TestUDP::mIOContext = nullptr;

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
