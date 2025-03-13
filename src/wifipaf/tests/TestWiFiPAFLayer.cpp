/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <cstdint>
#include <cstring>
#include <string>
#include <type_traits>
#include <utility>

#include <pw_unit_test/framework.h>

#include <lib/core/CHIPError.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/Span.h>
#include <lib/support/TypeTraits.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemLayer.h>
#include <system/SystemPacketBuffer.h>

#include <wifipaf/WiFiPAFLayer.h>
#include <wifipaf/WiFiPAFLayerDelegate.h>

namespace chip {
namespace WiFiPAF {

class TestWiFiPAFLayer : public WiFiPAFLayer, private WiFiPAFLayerDelegate, public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
        ASSERT_EQ(DeviceLayer::SystemLayer().Init(), CHIP_NO_ERROR);
    }

    static void TearDownTestSuite()
    {
        DeviceLayer::SystemLayer().Shutdown();
        chip::Platform::MemoryShutdown();
    }

    void SetUp() override
    {
        ASSERT_EQ(Init(&DeviceLayer::SystemLayer()), CHIP_NO_ERROR);
        mWiFiPAFTransport = this;
    }

    void TearDown() override
    {
        mWiFiPAFTransport = nullptr;
        Shutdown(nullptr);
    }

    CHIP_ERROR WiFiPAFMessageReceived(WiFiPAFSession & RxInfo, System::PacketBufferHandle && msg) override { return CHIP_NO_ERROR; }
    CHIP_ERROR WiFiPAFMessageSend(WiFiPAFSession & TxInfo, System::PacketBufferHandle && msg) override { return CHIP_NO_ERROR; }
    CHIP_ERROR WiFiPAFCloseSession(WiFiPAFSession & SessionInfo) override { return CHIP_NO_ERROR; }
};

TEST_F(TestWiFiPAFLayer, CheckWiFiPAFTransportCapabilitiesRequestMessage)
{
    auto buf = System::PacketBufferHandle::New(100);
    ASSERT_FALSE(buf.IsNull());

    PAFTransportCapabilitiesRequestMessage msg{};
    msg.SetSupportedProtocolVersion(0, CHIP_PAF_TRANSPORT_PROTOCOL_MIN_SUPPORTED_VERSION);
    msg.mMtu        = CHIP_PAF_DEFAULT_MTU;
    msg.mWindowSize = PAF_MAX_RECEIVE_WINDOW_SIZE;

    ASSERT_EQ(msg.Encode(buf), CHIP_NO_ERROR);
    ChipLogByteSpan(Test, ByteSpan(buf->Start(), buf->DataLength()));

    PAFTransportCapabilitiesRequestMessage msgVerify;
    ASSERT_EQ(PAFTransportCapabilitiesRequestMessage::Decode(buf, msgVerify), CHIP_NO_ERROR);
    EXPECT_EQ(memcmp(msg.mSupportedProtocolVersions, msgVerify.mSupportedProtocolVersions, sizeof(msg.mSupportedProtocolVersions)),
              0);
    EXPECT_EQ(msg.mMtu, msgVerify.mMtu);
    EXPECT_EQ(msg.mWindowSize, msgVerify.mWindowSize);
}

TEST_F(TestWiFiPAFLayer, CheckWiFiPAFTransportCapabilitiesResponseMessage)
{
    auto buf = System::PacketBufferHandle::New(100);
    ASSERT_FALSE(buf.IsNull());

    PAFTransportCapabilitiesResponseMessage msg{};
    msg.mSelectedProtocolVersion = CHIP_PAF_TRANSPORT_PROTOCOL_MAX_SUPPORTED_VERSION;
    msg.mFragmentSize            = CHIP_PAF_DEFAULT_MTU;
    msg.mWindowSize              = PAF_MAX_RECEIVE_WINDOW_SIZE;

    EXPECT_EQ(msg.Encode(buf), CHIP_NO_ERROR);
    ChipLogByteSpan(Test, ByteSpan(buf->Start(), buf->DataLength()));

    PAFTransportCapabilitiesResponseMessage msgVerify;
    ASSERT_EQ(PAFTransportCapabilitiesResponseMessage::Decode(buf, msgVerify), CHIP_NO_ERROR);
    EXPECT_EQ(msg.mSelectedProtocolVersion, msgVerify.mSelectedProtocolVersion);
    EXPECT_EQ(msg.mFragmentSize, msgVerify.mFragmentSize);
    EXPECT_EQ(msg.mWindowSize, msgVerify.mWindowSize);
}

TEST_F(TestWiFiPAFLayer, CheckPafSession)
{
    // Add a session by giving node_id, discriminator
    WiFiPAF::WiFiPAFSession sessionInfo = { .nodeId = 0x1, .discriminator = 0xF00 };
    EXPECT_EQ(AddPafSession(PafInfoAccess::kAccNodeInfo, sessionInfo), CHIP_NO_ERROR);

    // Get the session info by giving node_id
    sessionInfo.nodeId          = 0x1;
    auto pPafSessionInfo_nodeid = GetPAFInfo(PafInfoAccess::kAccNodeId, sessionInfo);
    EXPECT_EQ(pPafSessionInfo_nodeid->nodeId, sessionInfo.nodeId);
    EXPECT_EQ(pPafSessionInfo_nodeid->discriminator, sessionInfo.discriminator);

    // Get the session info by giving the discriminator
    sessionInfo.discriminator = 0xF00;
    auto pPafSessionInfo_disc = GetPAFInfo(PafInfoAccess::kAccDisc, sessionInfo);
    EXPECT_EQ(pPafSessionInfo_disc->nodeId, sessionInfo.nodeId);
    EXPECT_EQ(pPafSessionInfo_disc->discriminator, sessionInfo.discriminator);

    // Set the session ID of the existing session
    pPafSessionInfo_disc->id = 0x1;

    // Add a new session, but no space
    sessionInfo.id = 0x2;
    EXPECT_EQ(AddPafSession(PafInfoAccess::kAccSessionId, sessionInfo), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);

    sessionInfo.id = 0x1;
    EXPECT_EQ(RmPafSession(PafInfoAccess::kAccSessionId, sessionInfo), CHIP_NO_ERROR);
}

TEST_F(TestWiFiPAFLayer, CheckNewEndpoint)
{
    WiFiPAFSession SessionInfo = {
        .role          = kWiFiPafRole_Subscriber,
        .id            = 1,
        .peer_id       = 1,
        .peer_addr     = { 0xd0, 0x17, 0x69, 0xee, 0x7f, 0x3c },
        .nodeId        = 1,
        .discriminator = 0xF00,
    };

    WiFiPAFEndPoint * newEndPoint = nullptr;
    EXPECT_EQ(NewEndPoint(&newEndPoint, SessionInfo, SessionInfo.role), CHIP_NO_ERROR);
    EXPECT_NE(newEndPoint, nullptr);
}
}; // namespace WiFiPAF
}; // namespace chip
