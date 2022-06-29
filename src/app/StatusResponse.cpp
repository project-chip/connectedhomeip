/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app/InteractionModelTimeout.h>
#include <app/MessageDef/StatusResponseMessage.h>
#include <app/StatusResponse.h>

namespace chip {
namespace app {
CHIP_ERROR StatusResponse::Send(Protocols::InteractionModel::Status aStatus, Messaging::ExchangeContext * apExchangeContext,
                                bool aExpectResponse)
{
    VerifyOrReturnError(apExchangeContext != nullptr, CHIP_ERROR_INCORRECT_STATE);
    System::PacketBufferHandle msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    VerifyOrReturnError(!msgBuf.IsNull(), CHIP_ERROR_NO_MEMORY);

    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(msgBuf));

    StatusResponseMessage::Builder response;
    ReturnErrorOnFailure(response.Init(&writer));
    response.Status(aStatus);
    ReturnErrorOnFailure(response.GetError());
    ReturnErrorOnFailure(writer.Finalize(&msgBuf));
    apExchangeContext->UseSuggestedResponseTimeout(app::kExpectedIMProcessingTime);
    ReturnErrorOnFailure(apExchangeContext->SendMessage(Protocols::InteractionModel::MsgType::StatusResponse, std::move(msgBuf),
                                                        aExpectResponse ? Messaging::SendMessageFlags::kExpectResponse
                                                                        : Messaging::SendMessageFlags::kNone));
    return CHIP_NO_ERROR;
}

CHIP_ERROR StatusResponse::ProcessStatusResponse(System::PacketBufferHandle && aPayload, CHIP_ERROR & aStatusError)
{
    StatusResponseMessage::Parser response;
    System::PacketBufferTLVReader reader;
    reader.Init(std::move(aPayload));
    ReturnErrorOnFailure(response.Init(reader));
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    ReturnErrorOnFailure(response.CheckSchemaValidity());
#endif
    StatusIB status;
    ReturnErrorOnFailure(response.GetStatus(status.mStatus));
    ChipLogProgress(InteractionModel, "Received status response, status is " ChipLogFormatIMStatus,
                    ChipLogValueIMStatus(status.mStatus));
    ReturnErrorOnFailure(response.ExitContainer());

    aStatusError = status.ToChipError();
    return CHIP_NO_ERROR;
}
} // namespace app
} // namespace chip
