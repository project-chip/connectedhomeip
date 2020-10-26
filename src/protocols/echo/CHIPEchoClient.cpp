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
 *      initiator (client).
 *
 */

#include "CHIPEcho.h"

namespace chip {
namespace Protocols {

ChipEchoClient::ChipEchoClient()
{
    ExchangeMgr            = nullptr;
    OnEchoResponseReceived = nullptr;
    ExchangeCtx            = nullptr;
}

CHIP_ERROR ChipEchoClient::Init(ExchangeManager * exchangeMgr)
{
    // Error if already initialized.
    if (ExchangeMgr != nullptr)
        return CHIP_ERROR_INCORRECT_STATE;

    ExchangeMgr            = exchangeMgr;
    OnEchoResponseReceived = nullptr;
    ExchangeCtx            = nullptr;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipEchoClient::Shutdown()
{
    if (ExchangeCtx != nullptr)
    {
        ExchangeCtx->Abort();
        ExchangeCtx = nullptr;
    }

    ExchangeMgr = nullptr;

    return CHIP_NO_ERROR;
}

/**
 * Send an echo request to a CHIP node.
 *
 * @param nodeId        The destination's nodeId
 * @param payload       A System::PacketBuffer with the payload. This function takes ownership of the System::PacketBuffer
 *
 * @return CHIP_ERROR_NO_MEMORY if no ExchangeContext is available.
 *         Other CHIP_ERROR codes as returned by the lower layers.
 */
CHIP_ERROR ChipEchoClient::SendEchoRequest(uint64_t nodeId, System::PacketBuffer * payload)
{
    // Discard any existing exchange context. Effectively we can only have one Echo exchange with
    // a single node at any one time.
    if (ExchangeCtx != nullptr)
    {
        ExchangeCtx->Abort();
        ExchangeCtx = nullptr;
    }

    // Create a new exchange context.
    ExchangeCtx = ExchangeMgr->NewContext(nodeId, this);
    if (ExchangeCtx == nullptr)
    {
        System::PacketBuffer::Free(payload);
        return CHIP_ERROR_NO_MEMORY;
    }

    return SendEchoRequest(payload);
}

CHIP_ERROR ChipEchoClient::SendEchoRequest(System::PacketBuffer * payload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Send an Echo Request message.  Discard the exchange context if the send fails.
    err = ExchangeCtx->SendMessage(kChipProtocol_Echo, kEchoMessageType_EchoRequest, payload);

    payload = nullptr;
    if (err != CHIP_NO_ERROR)
    {
        ExchangeCtx->Abort();
        ExchangeCtx = nullptr;
    }

    return err;
}

void ChipEchoClient::OnMessageReceived(ExchangeContext * ec, const PacketHeader & packetHeader, uint32_t protocolId,
                                       uint8_t msgType, System::PacketBuffer * payload)
{
    ChipEchoClient * echoApp = static_cast<ChipEchoClient *>(ec->GetAppState());

    // Assert that the exchange context matches the client's current context.
    // This should never fail because even if SendEchoRequest is called
    // back-to-back, the second call will call Close() on the first exchange,
    // which clears the OnMessageReceived callback.
    VerifyOrDie(echoApp && ec == echoApp->ExchangeCtx);

    // Verify that the message is an Echo Response.
    // If not, close the exchange and free the payload.
    if (protocolId != kChipProtocol_Echo || msgType != kEchoMessageType_EchoResponse)
    {
        ec->Close();
        echoApp->ExchangeCtx = nullptr;
        ExitNow();
    }

    // Remove the EC from the app state now. OnEchoResponseReceived can call
    // SendEchoRequest and install a new one. We abort rather than close
    // because we no longer care whether the echo request message has been
    // acknowledged at the transport layer.
    echoApp->ExchangeCtx->Abort();
    echoApp->ExchangeCtx = nullptr;

    // Call the registered OnEchoResponseReceived handler, if any.
    if (echoApp->OnEchoResponseReceived != nullptr)
    {
        echoApp->OnEchoResponseReceived(packetHeader.GetSourceNodeId().HasValue() ? packetHeader.GetSourceNodeId().Value() : 0,
                                        payload);
    }

exit:
    // Free the payload buffer.
    System::PacketBuffer::Free(payload);
}

} // namespace Protocols
} // namespace chip
