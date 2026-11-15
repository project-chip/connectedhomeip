/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <protocols/bdx/BusyResponder.h>

#include <lib/support/BufferWriter.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TypeTraits.h>
#include <messaging/ExchangeContext.h>
#include <protocols/bdx/StatusCode.h>
#include <protocols/secure_channel/Constants.h>
#include <protocols/secure_channel/StatusReport.h>
#include <transport/SessionManager.h>

namespace chip {
namespace bdx {

CHIP_ERROR BusyResponder::OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                            System::PacketBufferHandle && payload)
{
    Protocols::SecureChannel::StatusReport report(Protocols::SecureChannel::GeneralStatusCode::kFailure, Protocols::BDX::Id,
                                                  to_underlying(StatusCode::kResponderBusy));
    const size_t size = report.Size();
    Encoding::LittleEndian::PacketBufferWriter writer(MessagePacketBuffer::New(size), size);
    VerifyOrReturnError(!writer.IsNull(), CHIP_ERROR_NO_MEMORY);
    report.WriteToBuffer(writer);
    System::PacketBufferHandle message = writer.Finalize();
    VerifyOrReturnError(!message.IsNull(), CHIP_ERROR_NO_MEMORY);

    return ec->SendMessage(Protocols::SecureChannel::MsgType::StatusReport, std::move(message));
}

} // namespace bdx
} // namespace chip
