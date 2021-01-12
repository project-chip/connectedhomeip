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

#include "Echo.h"

namespace chip {
namespace Protocols {

CHIP_ERROR EchoClient::Init(Messaging::ExchangeManager * exchangeMgr, SecureSessionHandle session)
{
    // Error if already initialized.
    if (mExchangeMgr != nullptr)
        return CHIP_ERROR_INCORRECT_STATE;

    mExchangeMgr           = exchangeMgr;
    mSecureSession         = session;
    OnEchoResponseReceived = nullptr;
    mExchangeCtx           = nullptr;

    return CHIP_NO_ERROR;
}

void EchoClient::Shutdown()
{
    if (mExchangeCtx != nullptr)
    {
        mExchangeCtx->Abort();
        mExchangeCtx = nullptr;
    }
}

CHIP_ERROR EchoClient::SendEchoRequest(System::PacketBufferHandle payload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Discard any existing exchange context. Effectively we can only have one Echo exchange with
    // a single node at any one time.
    if (mExchangeCtx != nullptr)
    {
        mExchangeCtx->Abort();
        mExchangeCtx = nullptr;
    }

    // Create a new exchange context.
    mExchangeCtx = mExchangeMgr->NewContext(mSecureSession, this);
    if (mExchangeCtx == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    // Send an Echo Request message.  Discard the exchange context if the send fails.
    err = mExchangeCtx->SendMessage(kProtocol_Echo, kEchoMessageType_EchoRequest, std::move(payload),
                                    Messaging::SendFlags(Messaging::SendMessageFlags::kNone));

    if (err != CHIP_NO_ERROR)
    {
        mExchangeCtx->Abort();
        mExchangeCtx = nullptr;
    }

    return err;
}

void EchoClient::OnMessageReceived(Messaging::ExchangeContext * ec, const PacketHeader & packetHeader, uint32_t protocolId,
                                   uint8_t msgType, System::PacketBufferHandle payload)
{
    // Assert that the exchange context matches the client's current context.
    // This should never fail because even if SendEchoRequest is called
    // back-to-back, the second call will call Close() on the first exchange,
    // which clears the OnMessageReceived callback.
    VerifyOrDie(ec == mExchangeCtx);

    // Verify that the message is an Echo Response.
    // If not, close the exchange and free the payload.
    if (protocolId != kProtocol_Echo || msgType != kEchoMessageType_EchoResponse)
    {
        ec->Close();
        mExchangeCtx = nullptr;
        return;
    }

    // Remove the EC from the app state now. OnEchoResponseReceived can call
    // SendEchoRequest and install a new one. We abort rather than close
    // because we no longer care whether the echo request message has been
    // acknowledged at the transport layer.
    mExchangeCtx->Abort();
    mExchangeCtx = nullptr;

    // Call the registered OnEchoResponseReceived handler, if any.
    if (OnEchoResponseReceived != nullptr)
    {
        OnEchoResponseReceived(ec, std::move(payload));
    }
}

void EchoClient::OnResponseTimeout(Messaging::ExchangeContext * ec)
{
    ChipLogProgress(Echo, "Time out! failed to receive echo response from Exchange: %p", ec);
}

} // namespace Protocols
} // namespace chip
