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
#include <app/clusters/ota-requestor/DefaultOTARequestorDriver.h>
#include <messaging/tests/MessagingContext.h>
#include <platform/OTAImageProcessor.h>
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

class MockOTARequestorDriver : public DeviceLayer::DefaultOTARequestorDriver
{
public:
    uint16_t mMaxDownloadBlockSize = 1024;
    uint16_t GetMaxDownloadBlockSize() override { return mMaxDownloadBlockSize; }
};

class MockOTAImageProcessor : public OTAImageProcessorInterface
{
public:
    CHIP_ERROR PrepareDownload() override { return CHIP_NO_ERROR; }
    CHIP_ERROR Finalize() override { return CHIP_NO_ERROR; }
    CHIP_ERROR Apply() override { return CHIP_NO_ERROR; }
    CHIP_ERROR Abort() override { return CHIP_NO_ERROR; }
    CHIP_ERROR ProcessBlock(ByteSpan & block) override { return CHIP_NO_ERROR; }
    bool IsFirstImageRun() override { return false; }
    CHIP_ERROR ConfirmCurrentImage() override { return CHIP_NO_ERROR; }
};

class TestDefaultOTARequestor : public Testing::LoopbackMessagingContext
{
public:
    DefaultOTARequestor mRequestor;
    BDXDownloader mBdxDownloader;
    MockOTARequestorDriver mDriver;
    MockOTAImageProcessor mImageProcessor;

    void SetUp() override
    {
        Testing::LoopbackMessagingContext::SetUp();

        mRequestor.mBdxDownloader      = &mBdxDownloader;
        mRequestor.mOtaRequestorDriver = &mDriver;
        mBdxDownloader.SetImageProcessorDelegate(&mImageProcessor);
    }

    void TearDown() override { Testing::LoopbackMessagingContext::TearDown(); }

    CHIP_ERROR CallStartDownload(const SessionHandle & session)
    {
        CHIP_ERROR err = mRequestor.StartDownload(GetExchangeManager(), session);

        if (err == CHIP_NO_ERROR)
        {
            err = mBdxDownloader.OnPreparedForDownload(CHIP_NO_ERROR);
        }

        return err;
    }
};

TEST_F(TestDefaultOTARequestor, StartDownload_OrdinarySession)
{
    mRequestor.SetLargePayload(false);
    mDriver.mMaxDownloadBlockSize = 1024;

    MockBDXReceiver mockReceiver;
    EXPECT_EQ(GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(
                  Protocols::BDX::Id, static_cast<uint8_t>(bdx::MessageType::ReceiveInit), &mockReceiver),
              CHIP_NO_ERROR);

    SessionHandle session = GetSessionAliceToBob();

    CHIP_ERROR err = CallStartDownload(session);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DrainAndServiceIO();

    EXPECT_TRUE(mockReceiver.mMessageReceived);

    uint16_t expectedOrdinarySize = 1024;
    EXPECT_EQ(mockReceiver.mReceivedMaxBlockSize, expectedOrdinarySize);

    EXPECT_EQ(GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Protocols::BDX::Id,
                                                                              static_cast<uint8_t>(bdx::MessageType::ReceiveInit)),
              CHIP_NO_ERROR);
}

TEST_F(TestDefaultOTARequestor, StartDownload_LargePayloadSession)
{
    mRequestor.SetLargePayload(true);
    mDriver.mMaxDownloadBlockSize = 4096;

    MockBDXReceiver mockReceiver;
    EXPECT_EQ(GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(
                  Protocols::BDX::Id, static_cast<uint8_t>(bdx::MessageType::ReceiveInit), &mockReceiver),
              CHIP_NO_ERROR);

    SessionHandle session = GetSessionAliceToBob();

    Transport::PeerAddress currentAddr = session->AsSecureSession()->GetPeerAddress();
    Transport::PeerAddress tcpAddr     = Transport::PeerAddress::TCP(currentAddr.GetIPAddress(), currentAddr.GetPort());
    session->AsSecureSession()->SetPeerAddress(tcpAddr);

    CHIP_ERROR err = CallStartDownload(session);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DrainAndServiceIO();

    EXPECT_TRUE(mockReceiver.mMessageReceived);

    EXPECT_GT(mockReceiver.mReceivedMaxBlockSize, 1024);

    EXPECT_EQ(GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Protocols::BDX::Id,
                                                                              static_cast<uint8_t>(bdx::MessageType::ReceiveInit)),
              CHIP_NO_ERROR);
}

} // namespace chip
