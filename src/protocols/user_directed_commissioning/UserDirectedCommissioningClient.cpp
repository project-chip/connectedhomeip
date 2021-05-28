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

    mExchangeMgr          = exchangeMgr;
    mSecureSession        = session;
    OnUDCResponseReceived = nullptr;
    mExchangeCtx          = nullptr;

    return CHIP_NO_ERROR;
}

void UserDirectedCommissioningClient::Shutdown()
{
    if (mExchangeCtx != nullptr)
    {
        mExchangeCtx->Abort();
        mExchangeCtx = nullptr;
    }

    OnUDCResponseReceived = nullptr;
    mExchangeMgr          = nullptr;
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

    printf("UserDirectedCommissioningClient::SendUDCRequest message DataLength=%d\n", payload->DataLength());
    uint8_t startOfBuf[10];
    payload->Read(startOfBuf, sizeof(startOfBuf));
    for (int i = 0; i < (int) sizeof(startOfBuf); i++)
    {
        printf("%d ", startOfBuf[i]);
    }
    printf("\n");

    // Create a new exchange context.
    mExchangeCtx = mExchangeMgr->NewContext(mSecureSession, this);
    if (mExchangeCtx == nullptr)
    {
        printf("UserDirectedCommissioningClient::SendUDCRequest no exchange\n");
        return CHIP_ERROR_NO_MEMORY;
    }

    printf("UserDirectedCommissioningClient::SendUDCRequest exchange\n");

    // Send an Echo Request message.  Discard the exchange context if the send fails.
    err = mExchangeCtx->SendMessage(MsgType::IdentificationDeclaration, std::move(payload), sendFlags);

    printf("UserDirectedCommissioningClient::SendUDCRequest returned err=%d\n", (int) err);

    if (err != CHIP_NO_ERROR)
    {
        mExchangeCtx->Abort();
        mExchangeCtx = nullptr;
        return err;
    }

    return err;
}

void UserDirectedCommissioningClient::OnMessageReceived(Messaging::ExchangeContext * ec, const PacketHeader & packetHeader,
                                                        const PayloadHeader & payloadHeader, System::PacketBufferHandle && payload)
{
    // There is no response to UDC at this time
    ChipLogProgress(Echo, "Unexpected UDC response from Exchange: %p", ec);
    ec->Close();
    mExchangeCtx->Abort();
    mExchangeCtx = nullptr;
}

void UserDirectedCommissioningClient::OnResponseTimeout(Messaging::ExchangeContext * ec)
{
    ChipLogProgress(Echo, "Time out! No UDC response from Exchange: %p", ec);
}

} // namespace UserDirectedCommissioning
} // namespace Protocols
} // namespace chip
