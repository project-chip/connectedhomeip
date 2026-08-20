/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
 *      Unit tests for the Proxy transport, which tunnels Matter packets over
 *      the ProxyMessageRequest / ProxyMessageResponse command path instead of
 *      a real network interface.
 */

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMem.h>
#include <system/SystemPacketBuffer.h>
#include <transport/raw/Base.h>
#include <transport/raw/PeerAddress.h>
#include <transport/raw/ProxyTransport.h>

#include <vector>

namespace {

using namespace chip;
using chip::Transport::PeerAddress;
using chip::Transport::ProxyTransportBase;
using chip::Transport::ProxyTransportDelegate;

constexpr uint16_t kSessionId      = 0x1234;
constexpr uint16_t kOtherSessionId = 0x5678;

const uint8_t kPayload[] = { 0xde, 0xad, 0xbe, 0xef };

/**
 * Records the packets the transport asks to be forwarded to the proxy, and can
 * be told to fail so the error propagates back through SendMessage().
 */
class MockProxyDelegate : public ProxyTransportDelegate
{
public:
    CHIP_ERROR SendProxyMessage(uint16_t sessionId, chip::ByteSpan message) override
    {
        mCallCount++;
        mLastSessionId = sessionId;
        mLastMessage.assign(message.begin(), message.end());
        return mSendResult;
    }

    unsigned mCallCount     = 0;
    uint16_t mLastSessionId = 0;
    std::vector<uint8_t> mLastMessage;
    CHIP_ERROR mSendResult = CHIP_NO_ERROR;
};

/** Captures packets the transport injects back into the Matter stack. */
class MockRawTransportDelegate : public chip::Transport::RawTransportDelegate
{
public:
    void HandleMessageReceived(const PeerAddress & peerAddress, System::PacketBufferHandle && msg,
                               chip::Transport::MessageTransportContext * ctxt = nullptr) override
    {
        mCallCount++;
        mLastPeerAddress = peerAddress;
        mLastLength      = msg->DataLength();
    }

    unsigned mCallCount = 0;
    PeerAddress mLastPeerAddress;
    size_t mLastLength = 0;
};

class TestProxyTransport : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

protected:
    void SetUp() override
    {
        mTransport.SetDelegate(&mRawDelegate);
        ASSERT_EQ(mTransport.Init(chip::Transport::ProxyListenParameters(nullptr)), CHIP_NO_ERROR);
    }

    System::PacketBufferHandle NewPayload() { return System::PacketBufferHandle::NewWithData(kPayload, sizeof(kPayload)); }

    ProxyTransportBase mTransport;
    MockProxyDelegate mProxyDelegate;
    MockRawTransportDelegate mRawDelegate;
};

TEST_F(TestProxyTransport, InactiveTransportClaimsNoAddress)
{
    EXPECT_FALSE(mTransport.IsActive());
    EXPECT_FALSE(mTransport.CanSendToPeer(PeerAddress::Proxy(kSessionId)));
}

TEST_F(TestProxyTransport, ActivateAndDeactivate)
{
    mTransport.Activate(kSessionId, &mProxyDelegate);
    EXPECT_TRUE(mTransport.IsActive());
    EXPECT_EQ(mTransport.GetSessionId(), kSessionId);
    EXPECT_TRUE(mTransport.CanSendToPeer(PeerAddress::Proxy(kSessionId)));

    mTransport.Deactivate();
    EXPECT_FALSE(mTransport.IsActive());
    EXPECT_FALSE(mTransport.CanSendToPeer(PeerAddress::Proxy(kSessionId)));
}

TEST_F(TestProxyTransport, CloseDeactivates)
{
    mTransport.Activate(kSessionId, &mProxyDelegate);
    mTransport.Close();
    EXPECT_FALSE(mTransport.IsActive());
}

TEST_F(TestProxyTransport, ActiveTransportOnlyClaimsProxyAddresses)
{
    mTransport.Activate(kSessionId, &mProxyDelegate);
    EXPECT_FALSE(mTransport.CanSendToPeer(PeerAddress::BLE()));
    EXPECT_FALSE(mTransport.CanSendToPeer(PeerAddress::WiFiPAF(1)));
}

TEST_F(TestProxyTransport, SendMessageForwardsToDelegate)
{
    mTransport.Activate(kSessionId, &mProxyDelegate);

    EXPECT_EQ(mTransport.SendMessage(PeerAddress::Proxy(kSessionId), NewPayload()), CHIP_NO_ERROR);
    EXPECT_EQ(mProxyDelegate.mCallCount, 1u);
    EXPECT_EQ(mProxyDelegate.mLastSessionId, kSessionId);
    ASSERT_EQ(mProxyDelegate.mLastMessage.size(), sizeof(kPayload));
    EXPECT_EQ(memcmp(mProxyDelegate.mLastMessage.data(), kPayload, sizeof(kPayload)), 0);
}

TEST_F(TestProxyTransport, SendMessageUsesTheAddressSessionId)
{
    mTransport.Activate(kSessionId, &mProxyDelegate);

    // The session id travels in the PeerAddress, so it is what gets forwarded.
    EXPECT_EQ(mTransport.SendMessage(PeerAddress::Proxy(kOtherSessionId), NewPayload()), CHIP_NO_ERROR);
    EXPECT_EQ(mProxyDelegate.mLastSessionId, kOtherSessionId);
}

TEST_F(TestProxyTransport, SendMessagePropagatesDelegateError)
{
    mTransport.Activate(kSessionId, &mProxyDelegate);
    mProxyDelegate.mSendResult = CHIP_ERROR_NO_MEMORY;

    EXPECT_EQ(mTransport.SendMessage(PeerAddress::Proxy(kSessionId), NewPayload()), CHIP_ERROR_NO_MEMORY);
}

TEST_F(TestProxyTransport, SendMessageRejectsNonProxyAddress)
{
    mTransport.Activate(kSessionId, &mProxyDelegate);

    EXPECT_EQ(mTransport.SendMessage(PeerAddress::BLE(), NewPayload()), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(mProxyDelegate.mCallCount, 0u);
}

TEST_F(TestProxyTransport, SendMessageRejectsNullBuffer)
{
    mTransport.Activate(kSessionId, &mProxyDelegate);

    EXPECT_EQ(mTransport.SendMessage(PeerAddress::Proxy(kSessionId), System::PacketBufferHandle()), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(mProxyDelegate.mCallCount, 0u);
}

TEST_F(TestProxyTransport, SendMessageRejectedWhenInactive)
{
    EXPECT_EQ(mTransport.SendMessage(PeerAddress::Proxy(kSessionId), NewPayload()), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(mProxyDelegate.mCallCount, 0u);
}

TEST_F(TestProxyTransport, SendMessageRejectedAfterDeactivate)
{
    mTransport.Activate(kSessionId, &mProxyDelegate);
    mTransport.Deactivate();

    EXPECT_EQ(mTransport.SendMessage(PeerAddress::Proxy(kSessionId), NewPayload()), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(mProxyDelegate.mCallCount, 0u);
}

TEST_F(TestProxyTransport, ReceivedMessageIsInjectedIntoTheStack)
{
    mTransport.Activate(kSessionId, &mProxyDelegate);
    mTransport.OnProxyMessageReceived(kSessionId, kPayload, sizeof(kPayload));

    EXPECT_EQ(mRawDelegate.mCallCount, 1u);
    EXPECT_EQ(mRawDelegate.mLastLength, sizeof(kPayload));
    EXPECT_TRUE(mRawDelegate.mLastPeerAddress == PeerAddress::Proxy(kSessionId));
}

TEST_F(TestProxyTransport, ReceivedMessageForOtherSessionIsDropped)
{
    mTransport.Activate(kSessionId, &mProxyDelegate);
    mTransport.OnProxyMessageReceived(kOtherSessionId, kPayload, sizeof(kPayload));

    EXPECT_EQ(mRawDelegate.mCallCount, 0u);
}

TEST_F(TestProxyTransport, ReceivedMessageWhileInactiveIsDropped)
{
    mTransport.OnProxyMessageReceived(kSessionId, kPayload, sizeof(kPayload));
    EXPECT_EQ(mRawDelegate.mCallCount, 0u);
}

} // namespace
