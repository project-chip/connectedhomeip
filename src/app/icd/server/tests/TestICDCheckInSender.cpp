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

#include <app/icd/server/ICDCheckInSender.h>
#include <app/icd/server/ICDMonitoringTable.h>
#include <crypto/DefaultSessionKeystore.h>
#include <lib/address_resolve/AddressResolve.h>
#include <lib/core/CHIPError.h>
#include <lib/core/NodeId.h>
#include <lib/core/PeerId.h>
#include <lib/core/StringBuilderAdapters.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/tests/MessagingContext.h>

namespace {

using namespace chip;

constexpr NodeId kCheckInNodeId     = 0x1122334455667788ULL;
constexpr FabricId kCheckInFabricId = 0x2906C908D115D362ULL;
constexpr uint32_t kICDCounter      = 0x1DU;

constexpr Crypto::Symmetric128BitsKeyByteArray kCheckInKeyMaterial = { 0x5a, 0x1e, 0x9c, 0x37, 0x84, 0x02, 0xf1, 0x6b,
                                                                       0xcd, 0x70, 0x33, 0xa9, 0x18, 0xe5, 0x4c, 0xd2 };

constexpr CHIP_ERROR kFatalSendErrorNotRemappedToSuccessByMapSendError = CHIP_ERROR_BAD_REQUEST;

class TestICDCheckInSender : public chip::Testing::LoopbackMessagingContext, private chip::Testing::LoopbackTransportDelegate
{
public:
    void SetUp() override
    {
        chip::Testing::LoopbackMessagingContext::SetUp();
        GetLoopback().SetLoopbackTransportDelegate(this);
    }

    void TearDown() override
    {
        GetExchangeManager().CloseAllContextsForDelegate(nullptr);

        if (mMonitoringEntry.keyHandleValid)
        {
            EXPECT_EQ(mMonitoringEntry.DeleteKey(), CHIP_NO_ERROR);
        }

        GetLoopback().SetLoopbackTransportDelegate(nullptr);
        GetLoopback().mNumMessagesToAllowBeforeError = 0;
        GetLoopback().mMessageSendError              = CHIP_NO_ERROR;

        chip::Testing::LoopbackMessagingContext::TearDown();
    }

protected:
    void WillSendMessage(const Transport::PeerAddress & peer, const System::PacketBufferHandle & message) override
    {
        mTransportSendAttempts++;
    }

    void ResolveAndSendCheckIn(app::ICDCheckInSender & sender)
    {
        mMonitoringEntry.fabricIndex = GetAliceFabricIndex();
        ASSERT_EQ(mMonitoringEntry.SetKey(ByteSpan(kCheckInKeyMaterial)), CHIP_NO_ERROR);

        ASSERT_TRUE(mMonitoringEntry.IsValid());
        ASSERT_NE(GetFabricTable().FindFabricWithIndex(mMonitoringEntry.fabricIndex), nullptr);

        (void) sender.RequestResolve(mMonitoringEntry, &GetFabricTable(), kICDCounter);

        AddressResolve::ResolveResult result;
        result.address = GetBobAddress();
        sender.OnNodeAddressResolved(PeerId().SetCompressedFabricId(kCheckInFabricId).SetNodeId(kCheckInNodeId), result);
    }

    Crypto::DefaultSessionKeystore mKeystore;
    ICDMonitoringEntry mMonitoringEntry{ &mKeystore, kUndefinedFabricIndex, kCheckInNodeId };
    uint32_t mTransportSendAttempts = 0;
};

TEST_F(TestICDCheckInSender, SuccessfulCheckInSendReleasesItsExchange)
{
    ASSERT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);

    auto & loopback            = GetLoopback();
    loopback.mSentMessageCount = 0;
    ASSERT_EQ(loopback.mMessageSendError, CHIP_NO_ERROR);

    app::ICDCheckInSender sender(&GetExchangeManager());
    ResolveAndSendCheckIn(sender);

    ASSERT_EQ(mTransportSendAttempts, 1u);
    EXPECT_EQ(loopback.mSentMessageCount, 1u);

    DrainAndServiceIO();

    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestICDCheckInSender, FailedCheckInSendReleasesItsExchange)
{
    ASSERT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);

    auto & loopback                         = GetLoopback();
    loopback.mSentMessageCount              = 0;
    loopback.mNumMessagesToAllowBeforeError = 0;
    loopback.mMessageSendError              = kFatalSendErrorNotRemappedToSuccessByMapSendError;

    app::ICDCheckInSender sender(&GetExchangeManager());
    ResolveAndSendCheckIn(sender);

    ASSERT_EQ(mTransportSendAttempts, 1u);
    EXPECT_EQ(loopback.mSentMessageCount, 0u);

    DrainAndServiceIO();

    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

} // namespace
