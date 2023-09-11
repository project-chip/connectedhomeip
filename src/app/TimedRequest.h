/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
    // encapsulated in the CHIP_ERROR this returns.  In that case,
    // StatusIB::InitFromChipError can be used to extract the status.
    static CHIP_ERROR HandleResponse(const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload);
};

} // namespace app
} // namespace chip
