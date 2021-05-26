/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include "UserDirectedCommissioning.h"

namespace chip {
namespace Protocols {
namespace UserDirectedCommissioning {

CHIP_ERROR UserDirectedCommissioningServer::Init(Messaging::ExchangeManager * exchangeMgr)
{
    // Error if already initialized.
    if (mExchangeMgr != nullptr)
        return CHIP_ERROR_INCORRECT_STATE;

    mExchangeMgr         = exchangeMgr;
    OnUDCRequestReceived = nullptr;

    // Register to receive unsolicited Echo Request messages from the exchange manager.
    mExchangeMgr->RegisterUnsolicitedMessageHandlerForType(MsgType::IdentificationDeclaration, this);

    return CHIP_NO_ERROR;
}

void UserDirectedCommissioningServer::Shutdown()
{
    if (mExchangeMgr != nullptr)
    {
        mExchangeMgr->UnregisterUnsolicitedMessageHandlerForType(MsgType::IdentificationDeclaration);
        mExchangeMgr = nullptr;
    }
}

void UserDirectedCommissioningServer::OnMessageReceived(Messaging::ExchangeContext * ec, const PacketHeader & packetHeader,
                                                        const PayloadHeader & payloadHeader, System::PacketBufferHandle payload)
{
    payload->DebugDump("UserDirectedCommissioningServer::OnMessageReceive");

    // TODO:
    // - parse message
    // - check recent cache to ignore duplicates
    // - perform DNS-SD discovery on specified instance
    // - callback to higher level to get user confirmation

    // Discard the exchange context.
    ec->Close();
}

} // namespace UserDirectedCommissioning
} // namespace Protocols
} // namespace chip
