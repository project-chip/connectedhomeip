/*
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

// BdxOtaSender serves a single transfer that InitializeTransfer() arms for one
// requester. These tests arm it for one peer and then deliver a BDX ReceiveInit
// over an exchange bound to a different peer, asserting the sender does not treat
// the foreign peer as the armed requester.
//
// Both peers live on the same fabric: a peer on a different fabric is already
// separated by fabric-scoped session state and so does not exercise the identity
// comparison. The check under test is at the OnMessageReceived seam, so the tests
// observe its return value directly rather than driving a full transfer.

#include <ota-provider-common/BdxOtaSender.h>

#include <lib/support/tests/ExtraPwTestMacros.h>
#include <messaging/tests/MessagingContext.h>
#include <optional>
#include <protocols/Protocols.h>
#include <protocols/bdx/BdxMessages.h>
#include <pw_unit_test/framework.h>
#include <utility>
#include <vector>

using namespace ::chip;
using namespace ::chip::bdx;

namespace {

constexpr uint16_t kArmedLocalSessionId = 21;
constexpr uint16_t kArmedPeerSessionId  = 11;
constexpr uint16_t kOtherLocalSessionId = 22;
constexpr uint16_t kOtherPeerSessionId  = 12;
constexpr uint16_t kXFabLocalSessionId  = 23;
constexpr uint16_t kXFabPeerSessionId   = 13;

constexpr NodeId kArmedRequesterNodeId = 0x1111;
constexpr NodeId kOtherRequesterNodeId = 0x2222;

constexpr uint16_t kMaxBlockSize = 1024;
constexpr char kFileDesignator[] = "0";

constexpr System::Clock::Timeout kBdxTimeout      = System::Clock::Seconds16(300);
constexpr System::Clock::Timeout kBdxPollInterval = System::Clock::Milliseconds32(20);

// Exposes the protected OnMessageReceived so a test can deliver one message and
// read the sender's verdict without a running transfer.
class TestableBdxOtaSender : public BdxOtaSender
{
public:
    using BdxOtaSender::OnMessageReceived;
    using BdxOtaSender::PollForOutput;
};

class TestBdxOtaSenderPeerBinding : public chip::Testing::LoopbackMessagingContext
{
public:
    void SetUp() override
    {
        LoopbackMessagingContext::SetUp();

        const NodeId providerNodeId = GetBobFabric()->GetNodeId();
        const FabricIndex fabric    = GetBobFabricIndex();
        const auto address          = Transport::PeerAddress::UDP(Inet::IPAddress::Any);

        // Provider-side sessions for two requesters on the same fabric, differing
        // only in peer node id. GetPeer() on each returns the requester identity.
        ASSERT_SUCCESS(GetSecureSessionManager().InjectCaseSessionWithTestKey(
            mArmedSession, kArmedLocalSessionId, kArmedPeerSessionId, providerNodeId, kArmedRequesterNodeId, fabric, address,
            CryptoContext::SessionRole::kResponder));
        ASSERT_SUCCESS(GetSecureSessionManager().InjectCaseSessionWithTestKey(
            mOtherSession, kOtherLocalSessionId, kOtherPeerSessionId, providerNodeId, kOtherRequesterNodeId, fabric, address,
            CryptoContext::SessionRole::kResponder));

        // Same node id as the armed requester, but on a different fabric. BDX has no
        // ACL, so this peer can reach the provider; only the fabric-index half of the
        // comparison separates it.
        ASSERT_SUCCESS(GetSecureSessionManager().InjectCaseSessionWithTestKey(
            mCrossFabricSession, kXFabLocalSessionId, kXFabPeerSessionId, GetAliceFabric()->GetNodeId(), kArmedRequesterNodeId,
            GetAliceFabricIndex(), address, CryptoContext::SessionRole::kResponder));

        mSender.emplace();
        mSender->SetFileDesignatorMap({ { kFileDesignator, "/dev/null" } });
    }

    void TearDown() override
    {
        // Destroy the sender first (its destructor only resets the transfer and never
        // touches its raw mExchangeCtx), then abort the exchanges it referenced, then
        // the sessions, all before the messaging context that owns them.
        mSender.reset();
        for (auto * exchange : mExchanges)
        {
            exchange->Abort();
        }
        mExchanges.clear();
        mArmedSession.Release();
        mOtherSession.Release();
        mCrossFabricSession.Release();
        LoopbackMessagingContext::TearDown();
    }

protected:
    // Arms the transfer for kArmedRequesterNodeId, as a successful QueryImage would.
    void ArmForArmedRequester()
    {
        ASSERT_SUCCESS(mSender->InitializeTransfer(GetBobFabricIndex(), kArmedRequesterNodeId));
        ASSERT_SUCCESS(mSender->PrepareForTransfer(&GetSystemLayer(), TransferRole::kSender,
                                                   BitFlags<TransferControlFlags>(TransferControlFlags::kReceiverDrive),
                                                   kMaxBlockSize, kBdxTimeout, kBdxPollInterval));
    }

    System::PacketBufferHandle MakeReceiveInit()
    {
        TransferInit init;
        init.TransferCtlOptions.ClearAll().Set(TransferControlFlags::kReceiverDrive, true);
        init.Version        = 1;
        init.MaxBlockSize   = kMaxBlockSize;
        init.StartOffset    = 0;
        init.MaxLength      = 0;
        init.FileDesLength  = static_cast<uint16_t>(strlen(kFileDesignator));
        init.FileDesignator = reinterpret_cast<const uint8_t *>(kFileDesignator);

        Encoding::LittleEndian::PacketBufferWriter writer(System::PacketBufferHandle::New(init.MessageSize()));
        init.WriteToBuffer(writer);
        return writer.Finalize();
    }

    // Opens an exchange bound to `session`, tracked so teardown can abort it.
    Messaging::ExchangeContext * NewExchangeFor(const SessionHandle & session)
    {
        Messaging::ExchangeContext * exchange = GetExchangeManager().NewContext(session, &*mSender);
        VerifyOrDie(exchange != nullptr);
        mExchanges.push_back(exchange);
        return exchange;
    }

    // Delivers a ReceiveInit to the sender over `exchange` and returns its verdict.
    CHIP_ERROR DeliverReceiveInitOn(Messaging::ExchangeContext * exchange)
    {
        PayloadHeader payloadHeader;
        payloadHeader.SetMessageType(Protocols::BDX::Id, to_underlying(MessageType::ReceiveInit));
        return mSender->OnMessageReceived(exchange, payloadHeader, MakeReceiveInit());
    }

    CHIP_ERROR DeliverReceiveInitFrom(const SessionHandle & session) { return DeliverReceiveInitOn(NewExchangeFor(session)); }

    std::optional<TestableBdxOtaSender> mSender;
    std::vector<Messaging::ExchangeContext *> mExchanges;
    SessionHolder mArmedSession;
    SessionHolder mOtherSession;
    SessionHolder mCrossFabricSession;
};

// A requester other than the one the transfer was armed for must be rejected.
TEST_F(TestBdxOtaSenderPeerBinding, RejectsARequesterItWasNotArmedFor)
{
    ArmForArmedRequester();

    EXPECT_EQ(DeliverReceiveInitFrom(mOtherSession.Get().Value()), CHIP_ERROR_INVALID_DESTINATION_NODE_ID);
}

// Control: the armed requester is still accepted, so the check is not rejecting
// every peer.
TEST_F(TestBdxOtaSenderPeerBinding, AcceptsTheRequesterItWasArmedFor)
{
    ArmForArmedRequester();

    EXPECT_SUCCESS(DeliverReceiveInitFrom(mArmedSession.Get().Value()));
}

// A peer sharing the armed node id but on a different fabric must be rejected: the
// fabric-index half of the comparison is what separates it, since BDX consults no ACL.
TEST_F(TestBdxOtaSenderPeerBinding, RejectsTheSameNodeIdOnADifferentFabric)
{
    ArmForArmedRequester();

    EXPECT_EQ(DeliverReceiveInitFrom(mCrossFabricSession.Get().Value()), CHIP_ERROR_INVALID_DESTINATION_NODE_ID);
}

// The override must not reject a later message on the exchange already driving the transfer:
// an early return there would skip the base's WillSendMessage() and free that exchange in the
// real message path. Guards against a future gate reintroducing that.
TEST_F(TestBdxOtaSenderPeerBinding, DoesNotRejectALaterMessageOnTheDrivingExchange)
{
    ArmForArmedRequester();

    Messaging::ExchangeContext * driving = NewExchangeFor(mArmedSession.Get().Value());
    ASSERT_SUCCESS(DeliverReceiveInitOn(driving));

    // Drain the pending output the init produced: the first poll accepts the transfer, the second
    // sends the ReceiveAccept. Without this the transfer session rejects any further message before
    // its own state machine sees it, which would hide what this test is asserting.
    mSender->PollForOutput();
    mSender->PollForOutput();
    DrainAndServiceIO();

    // The base now handles the second init and answers it with an unexpected-message status report,
    // so any early return in the override - whatever code it picks - turns this red.
    EXPECT_SUCCESS(DeliverReceiveInitOn(driving));
}

} // namespace
