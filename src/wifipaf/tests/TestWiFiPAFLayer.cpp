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

#include <wifipaf/WiFiPAFError.h>
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
        InitialPafInfo();
    }

    void TearDown() override
    {
        mWiFiPAFTransport = nullptr;
        Shutdown([](uint32_t id, WiFiPAF::WiFiPafRole role) {});
    }

    CHIP_ERROR WiFiPAFMessageReceived(WiFiPAFSession & RxInfo, System::PacketBufferHandle && msg) override { return CHIP_NO_ERROR; }
    CHIP_ERROR WiFiPAFMessageSend(WiFiPAFSession & TxInfo, System::PacketBufferHandle && msg) override { return CHIP_NO_ERROR; }
    CHIP_ERROR WiFiPAFCloseSession(WiFiPAFSession & SessionInfo) override { return CHIP_NO_ERROR; }
    bool WiFiPAFResourceAvailable() override { return mResourceAvailable; }
    static constexpr size_t kTestPacketLength     = 100;
    static constexpr size_t kTestPacketLengthLong = 500;

    void SetEndPoint(WiFiPAFEndPoint * pEndPoint) { mEndPoint = pEndPoint; }
    void EpDoClose(uint8_t flags, CHIP_ERROR err) { return mEndPoint->DoClose(flags, err); }
    CHIP_ERROR EpDriveStandAloneAck() { return mEndPoint->DriveStandAloneAck(); }
    CHIP_ERROR EpDoSendStandAloneAck() { return mEndPoint->DoSendStandAloneAck(); }
    void EpSetRxNextSeqNum(SequenceNumber_t seq) { mEndPoint->mPafTP.mRxNextSeqNum = seq; }
    WiFiPAFTP::State_t EpGetTxState() { return mEndPoint->mPafTP.mTxState; }
    bool mResourceAvailable = true;
    bool isSendQueueNull() { return mEndPoint->mSendQueue.IsNull(); }
    uint8_t GetResourceWaitCount() { return mEndPoint->mResourceWaitCount; }

private:
    WiFiPAFEndPoint * mEndPoint;
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
    // Add the 1st session by giving node_id, discriminator
    WiFiPAF::WiFiPAFSession sessionInfo = { .role = kWiFiPafRole_Subscriber, .nodeId = 0x1, .discriminator = 0xF01 };
    EXPECT_EQ(AddPafSession(PafInfoAccess::kAccNodeInfo, sessionInfo), CHIP_NO_ERROR);

    // Add the 2nd session
    sessionInfo = { .role = WiFiPAF::WiFiPafRole::kWiFiPafRole_Subscriber, .nodeId = 0x2, .discriminator = 0xF02 };
    EXPECT_EQ(AddPafSession(PafInfoAccess::kAccNodeInfo, sessionInfo), CHIP_NO_ERROR);

    // Add the 3rd session => expect: no space
    sessionInfo.id = 0x3;
    EXPECT_EQ(AddPafSession(PafInfoAccess::kAccSessionId, sessionInfo), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);

    // Get the session info by giving node_id
    sessionInfo.nodeId          = 0x1;
    auto pPafSessionInfo_nodeid = GetPAFInfo(PafInfoAccess::kAccNodeId, sessionInfo);
    pPafSessionInfo_nodeid->id  = 0x1;
    EXPECT_EQ(pPafSessionInfo_nodeid->nodeId, 0x1u);
    EXPECT_EQ(pPafSessionInfo_nodeid->discriminator, 0xF01);

    // Get the session info by giving the discriminator
    sessionInfo.discriminator = 0xF01;
    auto pPafSessionInfo_disc = GetPAFInfo(PafInfoAccess::kAccDisc, sessionInfo);
    EXPECT_NE(pPafSessionInfo_disc, nullptr);
    EXPECT_EQ(pPafSessionInfo_disc->nodeId, 0x1u);
    pPafSessionInfo_disc->id = 1;

    sessionInfo.discriminator = 0xF02;
    pPafSessionInfo_disc      = GetPAFInfo(PafInfoAccess::kAccDisc, sessionInfo);
    EXPECT_NE(pPafSessionInfo_disc, nullptr);
    EXPECT_EQ(pPafSessionInfo_disc->nodeId, 0x2u);
    EXPECT_EQ(pPafSessionInfo_disc->discriminator, 0xF02);
    pPafSessionInfo_disc->id = 2;

    // Get the session info by giving the session id
    sessionInfo.id          = 0x1;
    auto pPafSessionInfo_id = GetPAFInfo(PafInfoAccess::kAccSessionId, sessionInfo);
    EXPECT_NE(pPafSessionInfo_id, nullptr);
    EXPECT_EQ(pPafSessionInfo_id->nodeId, 0x1u);
    EXPECT_EQ(pPafSessionInfo_id->discriminator, 0xF01);

    // Remove the session
    sessionInfo.id = 0x1;
    EXPECT_EQ(RmPafSession(PafInfoAccess::kAccSessionId, sessionInfo), CHIP_NO_ERROR);
    sessionInfo.id = 0x2;
    EXPECT_EQ(RmPafSession(PafInfoAccess::kAccSessionId, sessionInfo), CHIP_NO_ERROR);

    EXPECT_EQ(RmPafSession(PafInfoAccess::kAccNodeInfo, sessionInfo), CHIP_ERROR_NOT_IMPLEMENTED);
    EXPECT_EQ(RmPafSession(PafInfoAccess::kAccSessionId, sessionInfo), CHIP_ERROR_NOT_FOUND);
}

TEST_F(TestWiFiPAFLayer, CheckRunAsCommissioner)
{
    WiFiPAFSession sessionInfo = {
        .role          = kWiFiPafRole_Subscriber,
        .id            = 1,
        .peer_id       = 1,
        .peer_addr     = { 0xd0, 0x17, 0x69, 0xee, 0x7f, 0x3c },
        .nodeId        = 1,
        .discriminator = 0xF00,
    };

    WiFiPAFEndPoint * newEndPoint = nullptr;
    EXPECT_EQ(NewEndPoint(&newEndPoint, sessionInfo, sessionInfo.role), CHIP_NO_ERROR);
    EXPECT_NE(newEndPoint, nullptr);
    SetEndPoint(newEndPoint);
    newEndPoint->mState = WiFiPAFEndPoint::kState_Ready;
    SetWiFiPAFState(State::kInitialized);
    EXPECT_EQ(GetWiFiPAFState(), State::kInitialized);

    EXPECT_EQ(newEndPoint->StartConnect(), CHIP_NO_ERROR);
    EXPECT_EQ(AddPafSession(PafInfoAccess::kAccSessionId, sessionInfo), CHIP_NO_ERROR);
    newEndPoint->mState = WiFiPAFEndPoint::kState_Connected;

    // Send the capability request packet
    constexpr uint8_t bufCapReq[] = { 0x65, 0x6c, 0x04, 0x00, 0x00, 0x00, 0x5e, 0x01, 0x06 };
    auto packetCapReq             = System::PacketBufferHandle::NewWithData(bufCapReq, sizeof(bufCapReq));
    EXPECT_EQ(SendMessage(sessionInfo, std::move(packetCapReq)), CHIP_NO_ERROR);
    EXPECT_EQ(HandleWriteConfirmed(sessionInfo, true), CHIP_NO_ERROR);

    // Receive the capability response packet
    constexpr uint8_t bufCapResp[] = { 0x65, 0x6c, 0x04, 0x5b, 0x01, 0x06 };
    auto packetCapResp             = System::PacketBufferHandle::NewWithData(bufCapResp, sizeof(bufCapResp));
    newEndPoint->mState            = WiFiPAFEndPoint::kState_Connecting;
    EXPECT_EQ(OnWiFiPAFMessageReceived(sessionInfo, std::move(packetCapResp)), true);

    // Send a packet
    auto buf = System::PacketBufferHandle::New(kTestPacketLength);
    buf->SetDataLength(kTestPacketLength);
    memset(buf->Start(), 0, buf->DataLength());
    EXPECT_EQ(SendMessage(sessionInfo, std::move(buf)), CHIP_NO_ERROR);
    EXPECT_EQ(HandleWriteConfirmed(sessionInfo, true), CHIP_NO_ERROR);

    constexpr uint8_t buf_rx[] = {
        to_underlying(WiFiPAFTP::HeaderFlags::kStartMessage) | to_underlying(WiFiPAFTP::HeaderFlags::kEndMessage) |
            to_underlying(WiFiPAFTP::HeaderFlags::kFragmentAck),
        0x01,
        0x01,
        0x00,
        0x00, // payload
    };

    // Receive a pcaket
    auto packet_rx = System::PacketBufferHandle::NewWithData(buf_rx, sizeof(buf_rx));
    EXPECT_EQ(packet_rx->DataLength(), static_cast<size_t>(5));
    EXPECT_EQ(newEndPoint->Receive(std::move(packet_rx)), CHIP_NO_ERROR);

    EXPECT_EQ(EpDriveStandAloneAck(), CHIP_NO_ERROR);
    EXPECT_EQ(EpDoSendStandAloneAck(), CHIP_NO_ERROR);

    // Close the session
    EXPECT_EQ(RmPafSession(PafInfoAccess::kAccSessionId, sessionInfo), CHIP_NO_ERROR);
    EpDoClose(kWiFiPAFCloseFlag_AbortTransmission, WIFIPAF_ERROR_APP_CLOSED_CONNECTION);
}

TEST_F(TestWiFiPAFLayer, CheckRunAsCommissionee)
{
    WiFiPAFSession sessionInfo = {
        .role          = kWiFiPafRole_Publisher,
        .id            = 1,
        .peer_id       = 1,
        .peer_addr     = { 0xd0, 0x17, 0x69, 0xee, 0x7f, 0x3c },
        .nodeId        = 1,
        .discriminator = 0xF00,
    };

    WiFiPAFEndPoint * newEndPoint = nullptr;
    EXPECT_EQ(NewEndPoint(&newEndPoint, sessionInfo, sessionInfo.role), CHIP_NO_ERROR);
    EXPECT_NE(newEndPoint, nullptr);
    SetEndPoint(newEndPoint);
    EXPECT_EQ(AddPafSession(PafInfoAccess::kAccSessionId, sessionInfo), CHIP_NO_ERROR);
    newEndPoint->mState = WiFiPAFEndPoint::kState_Ready;

    // Receive the Capability_Request packet
    constexpr uint8_t bufCapReq[] = { 0x65, 0x6c, 0x04, 0x00, 0x00, 0x00, 0x5e, 0x01, 0x06 };
    auto packetCapReq             = System::PacketBufferHandle::NewWithData(bufCapReq, sizeof(bufCapReq));
    EXPECT_EQ(OnWiFiPAFMessageReceived(sessionInfo, std::move(packetCapReq)), true);

    // Reply the Capability Response packet
    constexpr uint8_t bufCapResp[] = { 0x65, 0x6c, 0x04, 0x5b, 0x01, 0x06 };
    auto packetCapResp             = System::PacketBufferHandle::NewWithData(bufCapResp, sizeof(bufCapResp));
    EXPECT_EQ(HandleWriteConfirmed(sessionInfo, true), CHIP_NO_ERROR);
    EXPECT_EQ(SendMessage(sessionInfo, std::move(packetCapResp)), CHIP_NO_ERROR);
    EXPECT_EQ(HandleWriteConfirmed(sessionInfo, true), CHIP_NO_ERROR);

    // Send a long packet
    auto buf = System::PacketBufferHandle::New(kTestPacketLengthLong);
    buf->SetDataLength(kTestPacketLengthLong);
    memset(buf->Start(), 0, buf->DataLength());
    EXPECT_EQ(SendMessage(sessionInfo, std::move(buf)), CHIP_NO_ERROR);
    EXPECT_EQ(EpGetTxState(), WiFiPAFTP::kState_InProgress);
    EXPECT_EQ(HandleWriteConfirmed(sessionInfo, true), CHIP_NO_ERROR);
    EXPECT_EQ(EpGetTxState(), WiFiPAFTP::kState_Complete);
    EXPECT_EQ(HandleWriteConfirmed(sessionInfo, true), CHIP_NO_ERROR);

    // Send a normal packet
    buf = System::PacketBufferHandle::New(kTestPacketLength);
    buf->SetDataLength(kTestPacketLength);
    memset(buf->Start(), 0, buf->DataLength());
    EXPECT_EQ(SendMessage(sessionInfo, std::move(buf)), CHIP_NO_ERROR);
    EXPECT_EQ(EpGetTxState(), WiFiPAFTP::kState_Complete);
    EXPECT_EQ(HandleWriteConfirmed(sessionInfo, true), CHIP_NO_ERROR);

    // Receive a packet, sn#1
    constexpr uint8_t buf_rx[] = {
        to_underlying(WiFiPAFTP::HeaderFlags::kStartMessage) | to_underlying(WiFiPAFTP::HeaderFlags::kEndMessage) |
            to_underlying(WiFiPAFTP::HeaderFlags::kFragmentAck),
        0x01,
        0x01, // sn
        0x00,
        0x00, // payload
    };
    auto packet_rx = System::PacketBufferHandle::NewWithData(buf_rx, sizeof(buf_rx));
    EXPECT_EQ(packet_rx->DataLength(), static_cast<size_t>(5));
    EpSetRxNextSeqNum(1);
    EXPECT_EQ(newEndPoint->Receive(std::move(packet_rx)), CHIP_NO_ERROR);

    // Receive the duplicate packet
    packet_rx = System::PacketBufferHandle::NewWithData(buf_rx, sizeof(buf_rx));
    EXPECT_EQ(packet_rx->DataLength(), static_cast<size_t>(5));
    EXPECT_EQ(newEndPoint->Receive(std::move(packet_rx)), CHIP_NO_ERROR);

    // Test Reordering
    // Receive pkt sn#3
    constexpr uint8_t buf_rx_sn3[] = {
        to_underlying(WiFiPAFTP::HeaderFlags::kStartMessage) | to_underlying(WiFiPAFTP::HeaderFlags::kEndMessage) |
            to_underlying(WiFiPAFTP::HeaderFlags::kFragmentAck),
        0x1,
        0x03, // sn
        0x00,
        0x00, // payload
    };
    packet_rx = System::PacketBufferHandle::NewWithData(buf_rx_sn3, sizeof(buf_rx_sn3));
    EXPECT_EQ(packet_rx->DataLength(), static_cast<size_t>(5));
    EXPECT_EQ(newEndPoint->Receive(std::move(packet_rx)), CHIP_NO_ERROR);

    // Receive pkt sn#2
    constexpr uint8_t buf_rx_sn2[] = {
        to_underlying(WiFiPAFTP::HeaderFlags::kStartMessage) | to_underlying(WiFiPAFTP::HeaderFlags::kEndMessage) |
            to_underlying(WiFiPAFTP::HeaderFlags::kFragmentAck),
        0x01,
        0x02, // sn
        0x00,
        0x00, // payload
    };
    packet_rx = System::PacketBufferHandle::NewWithData(buf_rx_sn2, sizeof(buf_rx_sn2));
    EXPECT_EQ(packet_rx->DataLength(), static_cast<size_t>(5));
    EXPECT_EQ(newEndPoint->Receive(std::move(packet_rx)), CHIP_NO_ERROR);

    // Test, send chained packet
    constexpr uint8_t buf_chain[] = {
        to_underlying(WiFiPAFTP::HeaderFlags::kStartMessage) | to_underlying(WiFiPAFTP::HeaderFlags::kEndMessage),
        0x01,
        0x01,
        0x00,
        0x00, // payload
    };
    auto packet_c1 = System::PacketBufferHandle::NewWithData(buf_chain, sizeof(buf_chain));
    auto packet_c2 = System::PacketBufferHandle::NewWithData(buf_chain, sizeof(buf_chain));
    packet_c1->AddToEnd(std::move(packet_c2));
    EXPECT_EQ(packet_c1->HasChainedBuffer(), true);
    EXPECT_EQ(newEndPoint->Send(std::move(packet_c1)), CHIP_NO_ERROR);
    EXPECT_EQ(HandleWriteConfirmed(sessionInfo, true), CHIP_NO_ERROR);

    // Test, Send the packet while resource is unavaialbe -> available
    mResourceAvailable   = false;
    auto packet_resource = System::PacketBufferHandle::NewWithData(buf_chain, sizeof(buf_chain));
    EXPECT_EQ(newEndPoint->Send(std::move(packet_resource)), CHIP_NO_ERROR);
    // break because resource is unavailable
    EXPECT_EQ(isSendQueueNull(), false);
    EXPECT_GT(GetResourceWaitCount(), 0);
    // Resource is available now
    mResourceAvailable = true;
    // PAF packets shoudl be sent within a second
    System::Clock::Internal::MockClock clock;
    System::Clock::ClockBase * realClock        = &System::SystemClock();
    constexpr System::Clock::Seconds64 pauseSec = System::Clock::Seconds64(2);
    clock.SetMonotonic(pauseSec);
    System::Clock::Internal::SetSystemClockForTesting(&clock);
    EXPECT_EQ(HandleWriteConfirmed(sessionInfo, true), CHIP_NO_ERROR);
    // PAF packet has been sent
    EXPECT_EQ(isSendQueueNull(), true);
    EXPECT_EQ(GetResourceWaitCount(), 0);

    // Close the session
    EXPECT_EQ(RmPafSession(PafInfoAccess::kAccSessionId, sessionInfo), CHIP_NO_ERROR);
    EpDoClose(kWiFiPAFCloseFlag_AbortTransmission, WIFIPAF_ERROR_APP_CLOSED_CONNECTION);

    System::Clock::Internal::SetSystemClockForTesting(realClock);
}
}; // namespace WiFiPAF
}; // namespace chip
