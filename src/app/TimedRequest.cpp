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
