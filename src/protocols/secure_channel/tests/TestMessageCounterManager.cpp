/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      This file implements unit tests for the MessageCounterManager implementation.
 */

#include <errno.h>

#include <nlbyteorder.h>
#include <pw_unit_test/framework.h>

#include <lib/core/CHIPCore.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <messaging/tests/MessagingContext.h>
#include <protocols/Protocols.h>
#include <protocols/echo/Echo.h>
#include <transport/SessionManager.h>
#include <transport/TransportMgr.h>

namespace {

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::Messaging;
using namespace chip::Protocols;

const char PAYLOAD[] = "Hello!";

class MockAppDelegate : public ExchangeDelegate
{
public:
    CHIP_ERROR OnMessageReceived(ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && msgBuf) override
    {
        ++ReceiveHandlerCallCount;
        return CHIP_NO_ERROR;
    }

    void OnResponseTimeout(ExchangeContext * ec) override {}

    int ReceiveHandlerCallCount = 0;
};

using TestMessageCounterManager = chip::Test::LoopbackMessagingContext;

TEST_F(TestMessageCounterManager, MessageCounterSyncProcess)
{

    SessionHandle localSession = GetSessionBobToAlice();
    SessionHandle peerSession  = GetSessionAliceToBob();

    Transport::SecureSession * localState = GetSecureSessionManager().GetSecureSession(localSession);
    Transport::SecureSession * peerState  = GetSecureSessionManager().GetSecureSession(peerSession);

    localState->GetSessionMessageCounter().GetPeerMessageCounter().Reset();
    EXPECT_EQ(GetMessageCounterManager().SendMsgCounterSyncReq(localSession, localState), CHIP_NO_ERROR);

    MessageCounter & peerCounter      = peerState->GetSessionMessageCounter().GetLocalMessageCounter();
    PeerMessageCounter & localCounter = localState->GetSessionMessageCounter().GetPeerMessageCounter();
    EXPECT_TRUE(localCounter.IsSynchronized());
    EXPECT_EQ(localCounter.GetCounter(), peerCounter.Value());
}

TEST_F(TestMessageCounterManager, CheckReceiveMessage)
{
    SessionHandle peerSession            = GetSessionAliceToBob();
    Transport::SecureSession * peerState = GetSecureSessionManager().GetSecureSession(peerSession);
    peerState->GetSessionMessageCounter().GetPeerMessageCounter().Reset();

    MockAppDelegate callback;
    GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(chip::Protocols::Echo::MsgType::EchoRequest, &callback);

    uint16_t payload_len              = sizeof(PAYLOAD);
    System::PacketBufferHandle msgBuf = MessagePacketBuffer::NewWithData(PAYLOAD, payload_len);
    ASSERT_FALSE(msgBuf.IsNull());

    Messaging::ExchangeContext * ec = NewExchangeToAlice(nullptr);
    ASSERT_NE(ec, nullptr);

    EXPECT_EQ(ec->SendMessage(chip::Protocols::Echo::MsgType::EchoRequest, std::move(msgBuf),
                              Messaging::SendFlags{ Messaging::SendMessageFlags::kNoAutoRequestAck }),
              CHIP_NO_ERROR);
    EXPECT_TRUE(peerState->GetSessionMessageCounter().GetPeerMessageCounter().IsSynchronized());
    EXPECT_EQ(callback.ReceiveHandlerCallCount, 1);
}

// Test Suite
} // namespace
