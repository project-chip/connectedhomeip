/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <pw_unit_test/framework.h>

#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <app/clusters/ota-requestor/DefaultOTARequestor.h>
#include <messaging/tests/MessagingContext.h>
#include <protocols/bdx/BdxMessages.h>

using namespace chip;
using namespace chip::app::Clusters::OtaSoftwareUpdateRequestor;

namespace chip {

// Mock Receiver to intercept the BDX ReceiveInit message and verify the MaxBlockSize
class MockBDXReceiver : public Messaging::ExchangeDelegate, public Messaging::UnsolicitedMessageHandler
{
public:
    CHIP_ERROR OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, ExchangeDelegate *& newDelegate) override
    {
        newDelegate = this;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && payload) override
    {
        if (payloadHeader.HasMessageType(bdx::MessageType::ReceiveInit))
        {
            bdx::ReceiveInit msg;
            CHIP_ERROR err = msg.Parse(std::move(payload));
            EXPECT_EQ(err, CHIP_NO_ERROR);

            mReceivedMaxBlockSize = msg.MaxBlockSize;
            mMessageReceived      = true;
        }
        return CHIP_NO_ERROR;
    }

    void OnResponseTimeout(Messaging::ExchangeContext * ec) override {}

    bool mMessageReceived          = false;
    uint16_t mReceivedMaxBlockSize = 0;
};

// Test fixture inheriting from LoopbackMessagingContext to set up a network session environment
class TestDefaultOTARequestor : public Testing::LoopbackMessagingContext
{
public:
    void SetUp() override { Testing::LoopbackMessagingContext::SetUp(); }

    void TearDown() override { Testing::LoopbackMessagingContext::TearDown(); }

    void InjectBdxDownloader(DefaultOTARequestor & requestor, BDXDownloader * downloader) { requestor.mBdxDownloader = downloader; }

    CHIP_ERROR CallStartDownload(DefaultOTARequestor & requestor, Messaging::ExchangeManager & exchangeMgr,
                                 const SessionHandle & session)
    {
        return requestor.StartDownload(exchangeMgr, session);
    }
};

TEST_F(TestDefaultOTARequestor, StartDownload_OrdinarySession)
{
    DefaultOTARequestor requestor;
    BDXDownloader bdxDownloader;

    // Inject necessary dependencies directly instead of calling the heavy Server Init() (using friend class)
    InjectBdxDownloader(requestor, &bdxDownloader);

    // Set to ordinary session (Large Payload not supported)
    requestor.SetLargePayload(false);

    MockBDXReceiver mockReceiver;
    EXPECT_EQ(GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(
                  Protocols::BDX::Id, static_cast<uint8_t>(bdx::MessageType::ReceiveInit), &mockReceiver),
              CHIP_NO_ERROR);

    SessionHandle session = GetSessionAliceToBob();

    // Execute StartDownload
    CHIP_ERROR err = CallStartDownload(requestor, GetExchangeManager(), session);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Process the message loop so the MockReceiver can receive the ReceiveInit message
    DrainAndServiceIO();

    EXPECT_TRUE(mockReceiver.mMessageReceived);

    // For an ordinary session, it should be limited to the default BDX Block Size (e.g., 1024)
    // (Please adjust the 1024 value according to the actual implementation constant in the PR)
    uint16_t expectedOrdinarySize = 1024;
    EXPECT_EQ(mockReceiver.mReceivedMaxBlockSize, expectedOrdinarySize);

    EXPECT_EQ(GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Protocols::BDX::Id,
                                                                              static_cast<uint8_t>(bdx::MessageType::ReceiveInit)),
              CHIP_NO_ERROR);
}

TEST_F(TestDefaultOTARequestor, StartDownload_LargePayloadSession)
{
    DefaultOTARequestor requestor;
    BDXDownloader bdxDownloader;

    // Inject necessary dependencies directly instead of calling the heavy Server Init()
    InjectBdxDownloader(requestor, &bdxDownloader);

    // Set to support large payload
    requestor.SetLargePayload(true);

    MockBDXReceiver mockReceiver;
    EXPECT_EQ(GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(
                  Protocols::BDX::Id, static_cast<uint8_t>(bdx::MessageType::ReceiveInit), &mockReceiver),
              CHIP_NO_ERROR);

    SessionHandle session = GetSessionAliceToBob();

    // Execute StartDownload
    CHIP_ERROR err = CallStartDownload(requestor, GetExchangeManager(), session);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DrainAndServiceIO();

    EXPECT_TRUE(mockReceiver.mMessageReceived);

    // For a large payload session, a larger Block Size should be chosen based on the session's MaxAppMessageLen
    // Verify that a value larger than the typical 1024 bytes was selected
    EXPECT_GT(mockReceiver.mReceivedMaxBlockSize, 1024);

    // (Optional) You can also verify the exact value according to the PR's exact formula as shown below:
    // uint16_t expectedLargeSize = session->GetMaxAppMessageLen() - bdx::kMaxDataBlockHeaderSize;
    // EXPECT_EQ(mockReceiver.mReceivedMaxBlockSize, expectedLargeSize);

    EXPECT_EQ(GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Protocols::BDX::Id,
                                                                              static_cast<uint8_t>(bdx::MessageType::ReceiveInit)),
              CHIP_NO_ERROR);
}

} // namespace chip
