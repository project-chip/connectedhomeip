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
#include <messaging/ExchangeContext.h>
#include <protocols/interaction_model/Constants.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <transport/raw/MessageHeader.h>

namespace chip {
namespace app {
static constexpr size_t kMaxSecureSduLengthBytes      = kMaxAppMessageLen + kMaxTagLen;
static constexpr size_t kMaxLargeSecureSduLengthBytes = kMaxLargeAppMessageLen + kMaxTagLen;

class StatusResponse
{
public:
    static CHIP_ERROR Send(Protocols::InteractionModel::Status aStatus, Messaging::ExchangeContext * apExchangeContext,
                           bool aExpectResponse);

    // The return value indicates whether the StatusResponse was parsed properly, and if it is CHIP_NO_ERROR
    // then aStatus has been set to the actual status, which might be success or failure.
    static CHIP_ERROR ProcessStatusResponse(System::PacketBufferHandle && aPayload, CHIP_ERROR & aStatus);
};
} // namespace app
} // namespace chip
