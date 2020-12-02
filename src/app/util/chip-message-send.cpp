/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, softwarEchoe
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 * @file
 *   This file provides an implementation of functions for sending messages.
 */

#include "chip-message-send.h"

#include <app/util/common.h>
#include <assert.h>
#include <inet/InetLayer.h> // PacketBuffer and the like
#include <messaging/ExchangeContext.h>
#include <protocols/Protocols.h>
#include <support/logging/CHIPLogging.h>
#include <transport/SecureSessionMgr.h> // For SecureSessionMgr

using namespace chip;

EmberStatus chipSendUnicast(Messaging::ExchangeContext & exchangeContext, EmberApsFrame * apsFrame, uint16_t messageLength, uint8_t * message)
{
    uint16_t frameSize           = encodeApsFrame(nullptr, 0, apsFrame);
    uint32_t dataLengthUnchecked = uint32_t(frameSize) + uint32_t(messageLength);
    if (dataLengthUnchecked > UINT16_MAX)
    {
        // Definitely too long for a packet!
        return EMBER_MESSAGE_TOO_LONG;
    }
    uint16_t dataLength = static_cast<uint16_t>(dataLengthUnchecked);

    if (frameSize == 0)
    {
        ChipLogError(Zcl, "Error encoding APS frame");
        return EMBER_ERR_FATAL;
    }

    System::PacketBufferHandle buffer = System::PacketBuffer::NewWithAvailableSize(dataLength);
    if (buffer.IsNull())
    {
        // FIXME: Not quite right... what's the right way to indicate "out of
        // heap"?
        return EMBER_MESSAGE_TOO_LONG;
    }

    if (encodeApsFrame(buffer->Start(), dataLength, apsFrame) != frameSize)
    {
        // Something is very wrong here; our first call lied to us!
        ChipLogError(Zcl, "Something wrong happened trying to encode aps frame to respond with");
        return EMBER_ERR_FATAL;
    }

    memcpy(buffer->Start() + frameSize, message, messageLength);
    buffer->SetDataLength(dataLength);

    CHIP_ERROR err = exchangeContext.SendMessage(Protocols::kProtocol_InteractionModel, 0, std::move(buffer), Messaging::SendFlags(), 0);
    if (err != CHIP_NO_ERROR)
    {
        // FIXME: Figure out better translations between our error types?
        return EMBER_DELIVERY_FAILED;
    }

    return EMBER_SUCCESS;
}
