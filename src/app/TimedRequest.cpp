/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "TimedRequest.h"

#include <app/MessageDef/TimedRequestMessage.h>
#include <app/StatusResponse.h>
#include <protocols/interaction_model/Constants.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <transport/SessionManager.h>

namespace chip {
namespace app {

using namespace Protocols::InteractionModel;
using namespace Messaging;

CHIP_ERROR TimedRequest::Send(ExchangeContext * aExchangeContext, uint16_t aTimeoutMs)
{
    // The payload is an anonymous struct (2 bytes) containing a single
    // 16-bit integer with two context tag (1 control byte, 1 byte tag, at
    // most 2 bytes for the timeout integer and 1 control byte, 1 byte tag, one byte for InteractionModelRevision).  Use
    // MessagePacketBuffer::New to account for other message-global overheads (MIC, etc).
    System::PacketBufferHandle payload = MessagePacketBuffer::New(9);
    VerifyOrReturnError(!payload.IsNull(), CHIP_ERROR_NO_MEMORY);

    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(payload));

    TimedRequestMessage::Builder builder;
    ReturnErrorOnFailure(builder.Init(&writer));

    builder.TimeoutMs(aTimeoutMs);
    ReturnErrorOnFailure(builder.GetError());

    ReturnErrorOnFailure(writer.Finalize(&payload));

    return aExchangeContext->SendMessage(MsgType::TimedRequest, std::move(payload), SendMessageFlags::kExpectResponse);
}

} // namespace app
} // namespace chip
