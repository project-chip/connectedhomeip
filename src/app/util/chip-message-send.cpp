/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, softwarEchoe
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 * @file
 *   This file provides an implementation of functions for sending messages.
 */

#include <app/util/chip-message-send.h>

#include <assert.h>
#include <inet/InetLayer.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <protocols/Protocols.h>
#include <protocols/temp_zcl/TempZCL.h>
#include <transport/raw/MessageHeader.h>

using namespace chip;

// TODO: This is not ideal, but we're still sorting out how secure session
// managers end up working and whether they're singletons.  In the long term,
// there will be some sane API that lets us send a message to a given node id.
//
// https://github.com/project-chip/connectedhomeip/issues/2566 tracks that API.
namespace chip {
// TODO: This is a placeholder delegate for exchange context created in Device::SendMessage()
//       Delete this class when Device::SendMessage() is obsoleted.
class DeviceExchangeDelegate : public Messaging::ExchangeDelegate
{
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * ec, const PacketHeader & packetHeader,
                                 const PayloadHeader & payloadHeader, System::PacketBufferHandle && payload) override
    {
        return CHIP_NO_ERROR;
    }
    void OnResponseTimeout(Messaging::ExchangeContext * ec) override {}
};

extern Messaging::ExchangeManager * ExchangeManager();
} // namespace chip

EmberStatus chipSendUnicast(Messaging::ExchangeContext * exchange, EmberApsFrame * apsFrame, uint16_t messageLength,
                            uint8_t * message, Messaging::SendFlags sendFlags)
{
    uint16_t frameSize           = encodeApsFrame(nullptr, 0, apsFrame);
    uint32_t dataLengthUnchecked = uint32_t(frameSize) + uint32_t(messageLength);
    if (dataLengthUnchecked > UINT16_MAX)
    {
        // Definitely too long for a packet!
        return EMBER_MESSAGE_TOO_LONG;
    }
    uint16_t dataLength = static_cast<uint16_t>(dataLengthUnchecked);

    if (frameSize == 0)
    {
        ChipLogError(Zcl, "Error encoding APS frame");
        return EMBER_ERR_FATAL;
    }

    System::PacketBufferHandle buffer = MessagePacketBuffer::New(dataLength);
    if (buffer.IsNull())
    {
        // FIXME: Not quite right... what's the right way to indicate "out of
        // heap"?
        return EMBER_MESSAGE_TOO_LONG;
    }

    if (encodeApsFrame(buffer->Start(), dataLength, apsFrame) != frameSize)
    {
        // Something is very wrong here; our first call lied to us!
        ChipLogError(Zcl, "Something wrong happened trying to encode aps frame to respond with");
        return EMBER_ERR_FATAL;
    }

    memcpy(buffer->Start() + frameSize, message, messageLength);
    buffer->SetDataLength(dataLength);

    CHIP_ERROR err = exchange->SendMessage(Protocols::TempZCL::MsgType::TempZCLResponse, std::move(buffer), sendFlags);

    if (err != CHIP_NO_ERROR)
    {
        // FIXME: Figure out better translations between our error types?
        return EMBER_DELIVERY_FAILED;
    }

    return EMBER_SUCCESS;
}

EmberStatus chipSendUnicast(NodeId destination, EmberApsFrame * apsFrame, uint16_t messageLength, uint8_t * message)
{
    // TODO: temporary create a handle from node id, will be fix in PR 3602
    Messaging::ExchangeManager * exchangeMgr = ExchangeManager();
    if (exchangeMgr == nullptr)
    {
        return EMBER_DELIVERY_FAILED;
    }

    Messaging::ExchangeContext * exchange =
        exchangeMgr->NewContext(SessionHandle(destination, 0, Transport::kAnyKeyId, 0), nullptr);
    if (exchange == nullptr)
    {
        return EMBER_DELIVERY_FAILED;
    }

    // TODO(#5675): This code is temporary, and must be updated to use the IM API. Currently, we use a temporary Protocol
    // TempZCL to carry over legacy ZCL messages, use an ephemeral exchange to send message and use its unsolicited message
    // handler to receive messages. We need to set flag kFromInitiator to allow receiver to deliver message to corresponding
    // unsolicited message handler, and we also need to set flag kNoAutoRequestAck since there is no persistent exchange to
    // receive the ack message. This logic needs to be deleted after we convert all legacy ZCL messages to IM messages.
    DeviceExchangeDelegate delegate;
    exchange->SetDelegate(&delegate);

    Messaging::SendFlags sendFlags;

    sendFlags.Set(Messaging::SendMessageFlags::kFromInitiator).Set(Messaging::SendMessageFlags::kNoAutoRequestAck);

    EmberStatus err = chipSendUnicast(exchange, apsFrame, messageLength, message, sendFlags);

    // Make sure we always close the temporary exchange we just created, unless
    // we sent a message successfully.
    if (err != EMBER_SUCCESS)
    {
        exchange->Close();
    }

    return err;
}
