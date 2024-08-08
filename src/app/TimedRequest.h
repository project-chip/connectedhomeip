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

#pragma once

#include <app/MessageDef/StatusIB.h>
#include <lib/core/CHIPError.h>
#include <messaging/ExchangeContext.h>
#include <system/SystemPacketBuffer.h>
#include <transport/raw/MessageHeader.h>

#include <cstdint>

namespace chip {
namespace app {
class TimedRequest
{
public:
    // Send a timed request with the given timeout value on the given exchange.
    static CHIP_ERROR Send(Messaging::ExchangeContext * aExchangeContext, uint16_t aTimeoutMs);

    // Handle a response message (which may not actually be a StatusResponse,
    // but came in after we sent a timed request).
    //
    // If the response is a failure StatusResponse, its status will be
    // encapsulated in the CHIP_ERROR this returns.  In that case, constructing
    // a StatusIB from the error can be used to extract the status.
    static CHIP_ERROR HandleResponse(const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload);
};

} // namespace app
} // namespace chip
