/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/MessageDef/StatusIB.h>
#include <messaging/ExchangeContext.h>
#include <protocols/interaction_model/Constants.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <transport/raw/MessageHeader.h>

namespace chip {
namespace app {
static constexpr size_t kMaxSecureSduLengthBytes = kMaxAppMessageLen + kMaxTagLen;

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
