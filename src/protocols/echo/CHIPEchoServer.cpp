/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file implements an object for a CHIP Echo unsolicitied
 *      responder (server).
 *
 */

#include "CHIPEcho.h"

namespace chip {
namespace Protocols {

ChipEchoServer::ChipEchoServer()
{
    ExchangeMgr           = nullptr;
    OnEchoRequestReceived = nullptr;
}

CHIP_ERROR ChipEchoServer::Init(ExchangeManager * exchangeMgr)
{
    // Error if already initialized.
    if (ExchangeMgr != nullptr)
        return CHIP_ERROR_INCORRECT_STATE;

    ExchangeMgr           = exchangeMgr;
    OnEchoRequestReceived = nullptr;

    // Register to receive unsolicited Echo Request messages from the exchange manager.
    ExchangeMgr->RegisterUnsolicitedMessageHandler(kChipProtocol_Echo, kEchoMessageType_EchoRequest, HandleEchoRequest, this);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipEchoServer::Shutdown()
{
    if (ExchangeMgr != nullptr)
    {
        ExchangeMgr->UnregisterUnsolicitedMessageHandler(kChipProtocol_Echo, kEchoMessageType_EchoRequest);
        ExchangeMgr = nullptr;
    }

    return CHIP_NO_ERROR;
}

void ChipEchoServer::HandleEchoRequest(ExchangeContext * ec, const PacketHeader & packetHeader, uint32_t protocolId,
                                       uint8_t msgType, System::PacketBuffer * payload)
{
    ChipEchoServer * echoApp = static_cast<ChipEchoServer *>(ec->GetAppState());

    // NOTE: we already know this is an Echo Request message because we explicitly registered with the
    // Exchange Manager for unsolicited Echo Requests.

    // Call the registered OnEchoRequestReceived handler, if any.
    if (echoApp->OnEchoRequestReceived != nullptr)
        echoApp->OnEchoRequestReceived(ec->GetPeerNodeId(), payload);

    // Since we are re-using the inbound EchoRequest buffer to send the EchoResponse, if necessary,
    // adjust the position of the payload within the buffer to ensure there is enough room for the
    // outgoing network headers.  This is necessary because in some network stack configurations,
    // the incoming header size may be smaller than the outgoing size.
    payload->EnsureReservedSize(CHIP_SYSTEM_CONFIG_HEADER_RESERVE_SIZE);

    // Send an Echo Response back to the sender.
    ec->SendMessage(kChipProtocol_Echo, kEchoMessageType_EchoResponse, payload);

    // Discard the exchange context.
    ec->Close();
}

} // namespace Protocols
} // namespace chip
