/* See Project CHIP LICENSE file for licensing information. */


/**
 *    @file
 *      This file implements an object for a CHIP Echo unsolicitied
 *      responder (server).
 *
 */

#include "Echo.h"

namespace chip {
namespace Protocols {
namespace Echo {

CHIP_ERROR EchoServer::Init(Messaging::ExchangeManager * exchangeMgr)
{
    // Error if already initialized.
    if (mExchangeMgr != nullptr)
        return CHIP_ERROR_INCORRECT_STATE;

    mExchangeMgr          = exchangeMgr;
    OnEchoRequestReceived = nullptr;

    // Register to receive unsolicited Echo Request messages from the exchange manager.
    mExchangeMgr->RegisterUnsolicitedMessageHandlerForType(MsgType::EchoRequest, this);

    return CHIP_NO_ERROR;
}

void EchoServer::Shutdown()
{
    if (mExchangeMgr != nullptr)
    {
        mExchangeMgr->UnregisterUnsolicitedMessageHandlerForType(MsgType::EchoRequest);
        mExchangeMgr = nullptr;
    }
}

void EchoServer::OnMessageReceived(Messaging::ExchangeContext * ec, const PacketHeader & packetHeader,
                                   const PayloadHeader & payloadHeader, System::PacketBufferHandle payload)
{
    System::PacketBufferHandle response;

    // NOTE: we already know this is an Echo Request message because we explicitly registered with the
    // Exchange Manager for unsolicited Echo Requests.

    // Call the registered OnEchoRequestReceived handler, if any.
    if (OnEchoRequestReceived != nullptr)
    {
        OnEchoRequestReceived(ec, payload.Retain());
    }

    // Since we are re-using the inbound EchoRequest buffer to send the EchoResponse, if necessary,
    // adjust the position of the payload within the buffer to ensure there is enough room for the
    // outgoing network headers.  This is necessary because in some network stack configurations,
    // the incoming header size may be smaller than the outgoing size.
    if (payload->EnsureReservedSize(CHIP_SYSTEM_CONFIG_HEADER_RESERVE_SIZE) && MessagePacketBuffer::HasFooterSpace(payload))
    {
        response = std::move(payload);
    }
    else
    {
        response = MessagePacketBuffer::NewWithData(payload->Start(), payload->DataLength());
    }

    // Send an Echo Response back to the sender.
    ec->SendMessage(MsgType::EchoResponse, std::move(response), Messaging::SendFlags(Messaging::SendMessageFlags::kNone));

    // Discard the exchange context.
    ec->Close();
}

} // namespace Echo
} // namespace Protocols
} // namespace chip
