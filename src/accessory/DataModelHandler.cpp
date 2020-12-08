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
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 * @file
 *   This file implements the handler for data model messages.
 */

#include <accessory/DataModelHandler.h>

#include <app/chip-zcl-zpro-codec.h>
#include <app/util/af-types.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>
#include <lib/support/logging/CHIPLogging.h>
#include <support/logging/CHIPLogging.h>
#include <system/SystemPacketBuffer.h>

using namespace ::chip;

/**
 * A data model message has nonzero length and always has a first byte whose
 * value is one of: 0x00, 0x01, 0x02, 0x03.  See chipZclEncodeZclHeader for the
 * construction of the message and in particular the first byte.
 *
 * Echo messages should generally not have a first byte with those values, so we
 * can use that to try to distinguish between the two.
 */
bool ContentMayBeADataModelMessage(const System::PacketBufferHandle & buffer)
{
    const size_t data_len      = buffer->DataLength();
    const uint8_t * data       = buffer->Start();
    bool maybeDataModelMessage = true;

    // Has to have nonzero length.
    VerifyOrExit(data_len > 0, maybeDataModelMessage = false);

    // Has to have a valid first byte value.
    VerifyOrExit(data[0] < 0x04, maybeDataModelMessage = false);

exit:
    return maybeDataModelMessage;
}

/**
 * @brief implements something like "od -c", changes an arbitrary byte string
 *   into a single-line of ascii.  Destroys any byte-wise encoding that
 *   might be present, e.g. utf-8.
 *
 * @param bytes     potentially unprintable buffer
 * @param bytes_len length of bytes
 * @param out       where to put the printable string
 * @param out_len   length of out
 * @return size_t required size of output buffer, including null-termination
 */
static size_t BinaryBytesToPrintableString(const uint8_t * bytes, size_t bytes_len, char * out, size_t out_len)
{
    size_t required = 1; // always need null termination
    memset(out, 0, out_len);
    // count and print
    for (; bytes_len > 0; bytes_len--, bytes++)
    {
        uint8_t byte = *bytes;

        if ((byte >= '\t' && byte <= '\r') || byte == '\\')
        {
            static const char * kCodes = "tnvfr";
            char code                  = (byte == '\\') ? '\\' : kCodes[byte - '\t'];
            required += 2;
            if (out_len > 2)
            {
                *out++ = '\\';
                *out++ = code;
                out_len -= 2;
            }
        }
        else if (byte >= ' ' && byte <= '~')
        {
            required += 1;
            if (out_len > 1)
            {
                *out++ = byte;
                out_len--;
            }
        }
        else
        {
            static const size_t kBinCodeLen = sizeof("\\xFF") - 1;
            static const char * kCodes      = "0123456789ABCDEF";

            required += kBinCodeLen;
            if (out_len > kBinCodeLen)
            {
                *out++ = '\\';
                *out++ = 'x';
                *out++ = kCodes[(byte & 0xf0) >> 4];
                *out++ = kCodes[byte & 0xf];
                out_len -= kBinCodeLen;
            }
        }
    }

    return required;
}

void ProcessOthersMessage(const PacketHeader & header, System::PacketBufferHandle & buffer, SecureSessionMgr * mgr)
{
    CHIP_ERROR err;
    char logmsg[512];

    BinaryBytesToPrintableString(buffer->Start(), buffer->DataLength(), logmsg, sizeof(logmsg));

    ChipLogProgress(AppServer, "Client sent: %s", logmsg);

    // Attempt to echo back
    err = mgr->SendMessage(header.GetSourceNodeId().Value(), std::move(buffer));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(AppServer, "Unable to echo back to client: %s", ErrorStr(err));
    }
    else
    {
        ChipLogProgress(AppServer, "Echo sent");
    }
}

void ProcessDataModelMessage(const PacketHeader & header, System::PacketBufferHandle & buffer)
{
    EmberApsFrame frame;
    bool ok = extractApsFrame(buffer->Start(), buffer->DataLength(), &frame) > 0;
    if (ok)
    {
        ChipLogProgress(Zcl, "APS frame processing success!");
    }
    else
    {
        ChipLogProgress(Zcl, "APS frame processing failure!");
        return;
    }

    uint8_t * message;
    uint16_t messageLen = extractMessage(buffer->Start(), buffer->DataLength(), &message);
    ok                  = emberAfProcessMessage(&frame,
                               0, // type
                               message, messageLen,
                               header.GetSourceNodeId().Value(), // source identifier
                               NULL);

    if (ok)
    {
        ChipLogProgress(Zcl, "Data model processing success!");
    }
    else
    {
        ChipLogProgress(Zcl, "Data model processing failure!");
    }
}

/**
 * Handle a message that should be processed via our data model processing
 * codepath. This function will free the packet buffer.
 *
 * @param [in] buffer The buffer holding the message.  This function guarantees
 *                    that it will free the buffer before returning.
 */
void HandleDataModelMessage(const PacketHeader & header, System::PacketBufferHandle buffer, SecureSessionMgr * mgr)
{
    // FIXME: Long-term we shouldn't be guessing what sort of message this is
    // based on the message bytes.  We're doing this for now to support both
    // data model messages and text echo messages, but in the long term we
    // should either do echo via a data model command or do echo on a separate
    // port from data model processing.
    if (ContentMayBeADataModelMessage(buffer))
    {
        ProcessDataModelMessage(header, buffer);
    }
    else
    {
        ProcessOthersMessage(header, buffer, mgr);
    }
}

void InitDataModelHandler()
{
    emberAfEndpointConfigure();
    emberAfInit();
}
