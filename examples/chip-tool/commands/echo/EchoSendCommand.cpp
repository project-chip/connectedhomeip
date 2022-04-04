/*
 *   Copyright (c) 2022 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "EchoSendCommand.h"

#include <app/InteractionModelEngine.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMessageDispatch.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>
#include <transport/SecureMessageCodec.h>
#include <transport/raw/MessageHeader.h>

using namespace chip;
using namespace chip::Messaging;

constexpr char kPayload[UINT16_MAX] = "ping";

/////////// CHIPCommand Interface /////////
CHIP_ERROR EchoSendCommand::RunCommand()
{
    ChipLogProgress(chipTool, "Sending echo request to node 0x%" PRIx64, mNodeId);
    return CurrentCommissioner().GetConnectedDevice(mNodeId, &mOnDeviceConnectedCallback, &mOnDeviceConnectionFailureCallback);
}

void EchoSendCommand::OnDeviceConnectedFn(void * context, OperationalDeviceProxy * device)
{
    auto command = static_cast<EchoSendCommand *>(context);
    LogErrorOnFailure(command->SendMessage(device));
}

void EchoSendCommand::OnDeviceConnectionFailureFn(void * context, PeerId peerId, CHIP_ERROR error)
{
    auto command = static_cast<EchoSendCommand *>(context);
    command->SetCommandExitStatus(error);
}

/////////// ExchangeDelegate Interface /////////
CHIP_ERROR EchoSendCommand::OnMessageReceived(ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                              System::PacketBufferHandle && payload)
{
    auto payloadLength = payload->DataLength();
    auto needsAck      = payloadHeader.NeedsAck();
    ChipLogProgress(chipTool, "Echo Response: \n\t length: %u\n\tneedsAck: %d", payloadLength, needsAck);

    return CHIP_NO_ERROR;
}

void EchoSendCommand::OnResponseTimeout(ExchangeContext * ec)
{
    ChipLogError(chipTool, "Timed out!");
    SetCommandExitStatus(CHIP_ERROR_INTERNAL);
}

/////////// SessionMessageDelegate Interface /////////
void EchoSendCommand::OnMessageReceived(const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
                                        const SessionHandle & session, const Transport::PeerAddress & source,
                                        DuplicateMessage isDuplicate, System::PacketBufferHandle && msgBuf)
{
    if (!ShouldIgnoreAcks() || !payloadHeader.GetAckMessageCounter().HasValue())
    {
        return mSessionMessageDelegate->OnMessageReceived(packetHeader, payloadHeader, session, source, isDuplicate,
                                                          std::move(msgBuf));
    }

    if (!payloadHeader.HasMessageType(Protocols::Echo::MsgType::EchoResponse) &&
        !payloadHeader.HasMessageType(Protocols::SecureChannel::MsgType::StandaloneAck))
    {
        return mSessionMessageDelegate->OnMessageReceived(packetHeader, payloadHeader, session, source, isDuplicate,
                                                          std::move(msgBuf));
    }

    if (mNeedsAckHandling)
    {
        mNeedsAckHandling = false;
        mSessionMessageDelegate->OnMessageReceived(packetHeader, payloadHeader, session, source, isDuplicate, std::move(msgBuf));
    }

    ChipLogProgress(chipTool, "=== Ignoring ack for message counter: %u", payloadHeader.GetAckMessageCounter().Value());
}

/////////// Internal /////////
CHIP_ERROR EchoSendCommand::SendMessage(OperationalDeviceProxy * device)
{
    auto payload = MessagePacketBuffer::NewWithData(kPayload, mMessageSize);
    VerifyOrReturnError(!payload.IsNull(), CHIP_ERROR_NO_MEMORY);

    auto exchangeMgr = device->GetExchangeManager();
    VerifyOrReturnError(nullptr != exchangeMgr, CHIP_ERROR_INCORRECT_STATE);

    auto ec = exchangeMgr->NewContext(device->GetSecureSession().Value(), this);
    VerifyOrReturnError(nullptr != ec, CHIP_ERROR_NO_MEMORY);

    auto sessionMgr = exchangeMgr->GetSessionManager();
    VerifyOrReturnError(nullptr != sessionMgr, CHIP_ERROR_INCORRECT_STATE);

    if (ShouldIgnoreAcks())
    {
        // If acks are ignored the following code configure itself as the recipient of all
        // session messages. On the first ack received, the message will follow the normal
        // dispatch route except that the ReliableMessageContext is configured to not send
        // back a ack. This way, the ReliableMessageContext knows that the message has been
        // dispatched properly and do not try to re-send the echo message.
        // On following messages, piggybagged acks and regular acks are simply ignored.

        mSessionMessageDelegate = exchangeMgr;
        sessionMgr->SetMessageDelegate(this);

        ReliableMessageContext * rc = ec->GetReliableMessageContext();
        VerifyOrReturnError(nullptr != rc, CHIP_ERROR_INCORRECT_STATE);

        rc->SetDropAckDebug(true);
    }

    CHIP_ERROR err = ec->SendMessage(GetSendMessageType(), std::move(payload), GetSendFlags());
    if (CHIP_NO_ERROR != err)
    {
        ec->Abort();
    }
    return err;
}

SendFlags EchoSendCommand::GetSendFlags() const
{
    SendFlags sendFlags = SendMessageFlags::kNone;

    bool needsResponse = NeedsResponse();
    if (needsResponse)
    {
        sendFlags.Set(SendMessageFlags::kExpectResponse);
    }

    bool needsHack = NeedsAck();
    if (!needsHack)
    {
        sendFlags.Set(SendMessageFlags::kNoAutoRequestAck);
    }

    return sendFlags;
}

Protocols::Echo::MsgType EchoSendCommand::GetSendMessageType() const
{
    bool needsResponse = NeedsResponse();
    if (needsResponse)
    {
        return Protocols::Echo::MsgType::EchoRequest;
    }

    return Protocols::Echo::MsgType::EchoRequestWithoutResponse;
}
