/**
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

#include "UnsolicitedStatusHandler.h"
#include <lib/support/TypeTraits.h>
#include <protocols/secure_channel/Constants.h>
#include <protocols/secure_channel/StatusReport.h>

using namespace chip;
using namespace chip::Protocols;
using namespace chip::Protocols::SecureChannel;
using namespace chip::Messaging;

CHIP_ERROR UnsolicitedStatusHandler::Init(ExchangeManager * exchangeManager)
{
    return exchangeManager->RegisterUnsolicitedMessageHandlerForType(SecureChannel::Id, to_underlying(MsgType::StatusReport), this);
}

CHIP_ERROR UnsolicitedStatusHandler::OnMessageReceived(ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                                       System::PacketBufferHandle && payload)
{
    if (!payloadHeader.HasMessageType(MsgType::StatusReport))
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    auto session = ec->GetSessionHandle();
    if (!session->IsSecureSession())
    {
        // Nothing to do here.  We only care about CloseSession on secure
        // sessions.
        return CHIP_NO_ERROR;
    }

    StatusReport report;
    ReturnErrorOnFailure(report.Parse(std::move(payload)));

    if (report.GetGeneralCode() == GeneralStatusCode::kSuccess && report.GetProtocolCode() == kProtocolCodeCloseSession)
    {
        ChipLogProgress(ExchangeManager, "Received CloseSession status message, closing session");
        session->AsSecureSession()->MarkForEviction();
        return CHIP_NO_ERROR;
    }

    // Just ignore this message.
    return CHIP_NO_ERROR;
}

void UnsolicitedStatusHandler::OnResponseTimeout(ExchangeContext * ec)
{
    // We should never get here, since we never send messages ourselves.
}

CHIP_ERROR UnsolicitedStatusHandler::OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader,
                                                                  ExchangeDelegate *& newDelegate)
{
    newDelegate = this;
    return CHIP_NO_ERROR;
}
