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

#include <assert.h>
#include <inet/InetLayer.h>
#include <support/logging/CHIPLogging.h>
#include <transport/SecureSessionMgr.h> // For SecureSessionMgr

using namespace chip;

// TODO: This is not ideal, but we're still sorting out how secure session
// managers end up working and whether they're singletons.  In the long term,
// there will be some sane API that lets us send a message to a given node id.
//
// https://github.com/project-chip/connectedhomeip/issues/2566 tracks that API.
namespace chip {
extern SecureSessionMgr & SessionManager();
}

EmberStatus chipSendUnicast(NodeId destination, EmberApsFrame * apsFrame, uint16_t messageLength, uint8_t * message)
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

    // TODO: temprary create a handle from node id, will be fix in PR 3602
    CHIP_ERROR err = SessionManager().SendMessage({ destination, Transport::kAnyKeyId }, std::move(buffer));
    if (err != CHIP_NO_ERROR)
    {
        // FIXME: Figure out better translations between our error types?
        return EMBER_DELIVERY_FAILED;
    }

    return EMBER_SUCCESS;
}
