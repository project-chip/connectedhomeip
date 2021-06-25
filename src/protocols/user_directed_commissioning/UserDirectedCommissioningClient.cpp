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

#include "UserDirectedCommissioning.h"

namespace chip {
namespace Protocols {
namespace UserDirectedCommissioning {

CHIP_ERROR UserDirectedCommissioningClient::Init(Messaging::ExchangeManager * exchangeMgr, SecureSessionHandle session)
{
    // Error if already initialized.
    if (mExchangeMgr != nullptr)
        return CHIP_ERROR_INCORRECT_STATE;

    mExchangeMgr   = exchangeMgr;
    mSecureSession = session;
    mExchangeCtx   = nullptr;

    return CHIP_NO_ERROR;
}

void UserDirectedCommissioningClient::Shutdown()
{
    if (mExchangeCtx != nullptr)
    {
        mExchangeCtx->Abort();
        mExchangeCtx = nullptr;
    }

    mExchangeMgr = nullptr;
}

CHIP_ERROR UserDirectedCommissioningClient::SendUDCRequest(System::PacketBufferHandle && payload,
                                                           const Messaging::SendFlags & sendFlags)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Discard any existing exchange context. Effectively we can only have one Echo exchange with
    // a single node at any one time.
    if (mExchangeCtx != nullptr)
    {
        mExchangeCtx->Abort();
        mExchangeCtx = nullptr;
    }

    payload->DebugDump("UserDirectedCommissioningClient::SendUDCRequest");

    // Create a new exchange context.
    mExchangeCtx = mExchangeMgr->NewContext(mSecureSession, this);
    if (mExchangeCtx == nullptr)
    {
        ChipLogError(Inet, "UserDirectedCommissioningClient::SendUDCRequest no exchange");
        return CHIP_ERROR_NO_MEMORY;
    }

    // Send an Echo Request message.  Discard the exchange context if the send fails.
    err = mExchangeCtx->SendMessage(MsgType::IdentificationDeclaration, std::move(payload), sendFlags);

    if (err != CHIP_NO_ERROR)
    {
        mExchangeCtx->Abort();
        mExchangeCtx = nullptr;
        return err;
    }

    return err;
}

CHIP_ERROR UserDirectedCommissioningClient::OnMessageReceived(Messaging::ExchangeContext * ec, const PacketHeader & packetHeader,
                                                              const PayloadHeader & payloadHeader,
                                                              System::PacketBufferHandle && payload)
{
    // There is no response to UDC at this time
    ChipLogProgress(Echo, "Unexpected UDC response from Exchange: %p", ec);
    ec->Close();
    mExchangeCtx->Abort();
    mExchangeCtx = nullptr;
    return CHIP_NO_ERROR;
}

void UserDirectedCommissioningClient::OnResponseTimeout(Messaging::ExchangeContext * ec)
{
    ChipLogProgress(Echo, "Time out! No UDC response from Exchange: %p", ec);
}

} // namespace UserDirectedCommissioning
} // namespace Protocols
} // namespace chip
