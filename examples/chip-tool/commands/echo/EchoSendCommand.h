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

#pragma once

#include "../common/CHIPCommand.h"

#include <protocols/echo/Echo.h>

class EchoSendCommand : public CHIPCommand, public chip::SessionMessageDelegate, public chip::Messaging::ExchangeDelegate
{
public:
    EchoSendCommand(CredentialIssuerCommands * credsIssuerConfig) :
        CHIPCommand("send", credsIssuerConfig), mOnDeviceConnectedCallback(OnDeviceConnectedFn, this),
        mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this)
    {
        AddArgument("message-size", 0, UINT16_MAX, &mMessageSize);
        AddArgument("node-id", 0, UINT64_MAX, &mNodeId);

        // Optional arguments
        AddArgument("suppress-response", 0, 1, &mSuppressResponse);
        AddArgument("needs-ack", 0, 1, &mNeedsAck);
        AddArgument("skip-incoming-acks", 0, 1, &mSkipIncomingAcks);
        AddArgument("skip-outgoing-acks", 0, 1, &mSkipOutgoingAcks);
    }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(30); }

    static void OnDeviceConnectedFn(void * context, chip::OperationalDeviceProxy * device);
    static void OnDeviceConnectionFailureFn(void * context, PeerId peerId, CHIP_ERROR error);

    /////////// SessionMessageDelegate Interface /////////
    void OnMessageReceived(const chip::PacketHeader & packetHeader, const chip::PayloadHeader & payloadHeader,
                           const chip::SessionHandle & session, const chip::Transport::PeerAddress & source,
                           chip::SessionMessageDelegate::DuplicateMessage isDuplicate,
                           chip::System::PacketBufferHandle && msgBuf) override;

    /////////// ExchangeDelegate Interface /////////
    CHIP_ERROR OnMessageReceived(chip::Messaging::ExchangeContext * ec, const chip::PayloadHeader & payloadHeader,
                                 chip::System::PacketBufferHandle && payload) override;
    void OnResponseTimeout(chip::Messaging::ExchangeContext * ec) override;

private:
    CHIP_ERROR SendMessage(chip::OperationalDeviceProxy * device);
    chip::Messaging::SendFlags GetSendFlags() const;
    chip::Protocols::Echo::MsgType GetSendMessageType() const;

    bool IsIncomingAck(const chip::PayloadHeader & payloadHeader) const;

    chip::NodeId mNodeId;
    uint16_t mMessageSize;
    chip::Optional<bool> mSuppressResponse;
    chip::Optional<bool> mNeedsAck;
    chip::Optional<bool> mSkipIncomingAcks;
    chip::Optional<bool> mSkipOutgoingAcks;

    bool NeedsResponse() const { return mSuppressResponse.HasValue() ? !mSuppressResponse.Value() : true; }
    bool NeedsAck() const { return mNeedsAck.HasValue() ? mNeedsAck.Value() : true; }
    bool ShouldSkipIncomingAcks() const { return NeedsAck() && (mSkipIncomingAcks.HasValue() ? mSkipIncomingAcks.Value() : false); }
    bool ShouldSkipOutgoingAcks() const { return NeedsAck() && (mSkipOutgoingAcks.HasValue() ? mSkipOutgoingAcks.Value() : false); }

    chip::SessionMessageDelegate * mSessionMessageDelegate;

    chip::Callback::Callback<chip::OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;
};
